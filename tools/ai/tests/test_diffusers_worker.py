import importlib.util
import json
import os
import tempfile
import unittest
from contextlib import contextmanager
from pathlib import Path

from PIL import Image, ImageDraw


WORKER_PATH = Path(__file__).resolve().parents[1] / "underpaint-diffusers-worker.py"
spec = importlib.util.spec_from_file_location("underpaint_diffusers_worker", WORKER_PATH)
worker = importlib.util.module_from_spec(spec)
assert spec.loader is not None
spec.loader.exec_module(worker)


@contextmanager
def isolated_env(**updates):
    keys = {
        "UNDERPAINT_MODEL_REGISTRY",
        "UNDERPAINT_INPAINT_MODEL_ID",
        "UNDERPAINT_INPAINT_MODEL",
        "UNDERPAINT_INPAINT_MODEL_FORMAT",
        "UNDERPAINT_SEGMENTATION_MODEL_ID",
        "UNDERPAINT_SEGMENTATION_BACKEND",
        "UNDERPAINT_SEGMENTATION_MODEL",
        "UNDERPAINT_SAM_MODEL",
        "UNDERPAINT_SAM_HQ_MODEL",
        "UNDERPAINT_REFINER_BACKEND",
        "UNDERPAINT_REFINER_MODEL_ID",
        "UNDERPAINT_REFINER_MODEL",
        "UNDERPAINT_REFINER_GGUF_MODEL",
        "UNDERPAINT_REFINER_MIN_RENDER_WIDTH",
        "UNDERPAINT_GGUF_REFINER_WORKER",
        "UNDERPAINT_DETAIL_UPSCALE_BACKEND",
        "UNDERPAINT_DETAIL_DETECTOR_DEVICE",
    }
    keys.update(updates)
    old_values = {key: os.environ.get(key) for key in keys}
    for key in keys:
        os.environ.pop(key, None)
    for key, value in updates.items():
        os.environ[key] = value
    try:
        yield
    finally:
        for key, value in old_values.items():
            if value is None:
                os.environ.pop(key, None)
            else:
                os.environ[key] = value


class DiffusersWorkerUnitTest(unittest.TestCase):
    def test_generation_model_uses_registry_entry(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            registry_path = Path(temp_dir) / "registry.json"
            registry_path.write_text(
                json.dumps(
                    {
                        "models": [
                            {
                                "id": "local-xl",
                                "displayName": "Local XL",
                                "backend": "diffusers",
                                "format": "single_file_sdxl",
                                "model": "$HOME/.underpaint/models/local-xl.safetensors",
                            }
                        ]
                    }
                ),
                encoding="utf-8",
            )
            with isolated_env(UNDERPAINT_MODEL_REGISTRY=str(registry_path)):
                model = worker.normalized_generation_model({"modelId": "local-xl"})

        self.assertEqual(model["modelId"], "local-xl")
        self.assertEqual(model["displayName"], "Local XL")
        self.assertEqual(model["format"], "single_file_sdxl")
        self.assertEqual(model["adapter"], "masked_img2img")
        self.assertTrue(model["model"].endswith(".underpaint/models/local-xl.safetensors"))

    def test_generation_model_rejects_unknown_format(self):
        with isolated_env():
            model = worker.normalized_generation_model(
                {"model": "custom-model", "modelFormat": "mystery-format"}
            )

        self.assertEqual(model["format"], "diffusers_repo")
        self.assertEqual(model["adapter"], "inpaint")

    def test_segmentation_backend_aliases_and_invalid_values(self):
        with isolated_env():
            hq = worker.normalized_segmentation_backend({"segmentationBackend": "hq-sam"})
            invalid = worker.normalized_segmentation_backend(
                {"segmentationBackend": "unknown-backend"}
            )

        self.assertEqual(hq["backend"], "sam-hq")
        self.assertEqual(invalid["backend"], "sam")

    def test_refiner_and_detail_settings_are_clamped(self):
        with isolated_env(
            UNDERPAINT_REFINER_BACKEND="invalid",
            UNDERPAINT_REFINER_MIN_RENDER_WIDTH="4096",
        ):
            refiner = worker.normalized_refiner(
                {
                    "refiner": {
                        "enabled": True,
                        "strength": 0.001,
                        "steps": 500,
                        "placement": "sideways",
                        "scheduler": "DPM++ 3M Karras",
                    }
                }
            )
            detail = worker.normalized_detail_pass(
                {
                    "detailPass": {
                        "enabled": True,
                        "faceEnabled": False,
                        "bodyEnabled": True,
                        "handsEnabled": True,
                        "detectionConfidence": -2,
                        "maxRegions": 999,
                        "maskPaddingPx": 999,
                        "detailRenderEdge": 4096,
                        "minCropEdge": 8,
                        "denoise": 0.001,
                        "steps": 999,
                        "scheduler": "Euler A",
                    }
                }
            )

        self.assertEqual(refiner["backend"], "diffusers")
        self.assertEqual(refiner["placement"], "before-detail")
        self.assertEqual(refiner["minRenderWidth"], 2048)
        self.assertEqual(refiner["strength"], 0.05)
        self.assertEqual(refiner["steps"], 200)
        self.assertEqual(refiner["scheduler"], "dpm++-3m-karras")
        self.assertEqual(detail["detectionConfidence"], 0.01)
        self.assertEqual(detail["maxRegions"], 16)
        self.assertEqual(detail["maskPaddingPx"], 256)
        self.assertEqual(detail["detailRenderEdge"], worker.MAX_DETAIL_RENDER_EDGE)
        self.assertEqual(detail["minCropEdge"], 64)
        self.assertEqual(detail["denoise"], 0.05)
        self.assertEqual(detail["steps"], 200)
        self.assertEqual(detail["scheduler"], "euler-a")
        diagnostics = worker.detail_pass_diagnostics(detail)
        self.assertEqual(diagnostics["regions"], ["body", "hands"])
        self.assertEqual(diagnostics["status"], "pending")

    def test_load_mask_prefers_alpha_for_opaque_black_rgba_masks(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            mask_path = Path(temp_dir) / "opaque-black-mask.png"
            Image.new("RGBA", (4, 3), (0, 0, 0, 255)).save(mask_path)

            mask = worker.load_mask(mask_path, (4, 3))

        self.assertEqual(mask.mode, "L")
        self.assertEqual(mask.getextrema(), (255, 255))

    def test_load_mask_uses_luminance_for_plain_rgb_masks(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            mask_path = Path(temp_dir) / "luma-mask.png"
            image = Image.new("RGB", (2, 1), (0, 0, 0))
            image.putpixel((1, 0), (255, 255, 255))
            image.save(mask_path)

            mask = worker.load_mask(mask_path, (2, 1))

        self.assertEqual(mask.mode, "L")
        self.assertEqual(mask.getpixel((0, 0)), 0)
        self.assertEqual(mask.getpixel((1, 0)), 255)

    def test_padding_source_replicates_edges_and_mask_padding_stays_empty(self):
        source = Image.new("RGB", (3, 5))
        for y in range(5):
            for x in range(3):
                source.putpixel((x, y), (x * 40, y * 30, 100))
        padded, original_size = worker.pad_source_to_multiple(source, 4)

        self.assertEqual(original_size, (3, 5))
        self.assertEqual(padded.size, (4, 8))
        self.assertEqual(padded.getpixel((3, 2)), source.getpixel((2, 2)))
        self.assertEqual(padded.getpixel((1, 7)), source.getpixel((1, 4)))
        self.assertEqual(padded.getpixel((3, 7)), source.getpixel((2, 4)))

        mask = Image.new("L", (3, 5), 255)
        padded_mask = worker.pad_mask_to_size(mask, padded.size)
        self.assertEqual(padded_mask.size, (4, 8))
        self.assertEqual(padded_mask.getpixel((2, 4)), 255)
        self.assertEqual(padded_mask.getpixel((3, 4)), 0)
        self.assertEqual(padded_mask.getpixel((2, 5)), 0)

    def test_detail_crop_and_render_size_upscale_small_regions(self):
        mask = Image.new("L", (100, 80), 0)
        draw = ImageDraw.Draw(mask)
        draw.rectangle((10, 12, 24, 26), fill=255)

        crop = worker.detail_crop_region(mask, mask.size, 32)
        self.assertIsNotNone(crop)
        assert crop is not None
        left, top, right, bottom = crop
        self.assertGreaterEqual(right - left, 32)
        self.assertGreaterEqual(bottom - top, 32)
        self.assertGreaterEqual(left, 0)
        self.assertGreaterEqual(top, 0)
        self.assertLessEqual(right, 100)
        self.assertLessEqual(bottom, 80)

        render_size = worker.detail_render_size((right - left, bottom - top), 1024)
        self.assertGreaterEqual(render_size[0], worker.MIN_DETAIL_RENDER_WIDTH)
        self.assertLessEqual(max(render_size), worker.MAX_DETAIL_RENDER_EDGE)
        self.assertEqual(render_size[0] % 8, 0)
        self.assertEqual(render_size[1] % 8, 0)


if __name__ == "__main__":
    unittest.main()
