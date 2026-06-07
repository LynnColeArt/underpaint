import importlib.util
import json
import os
import unittest
from contextlib import contextmanager
from pathlib import Path


HELPER_PATH = Path(__file__).resolve().parents[1] / "underpaint-prompt-helper.py"
spec = importlib.util.spec_from_file_location("underpaint_prompt_helper", HELPER_PATH)
helper = importlib.util.module_from_spec(spec)
assert spec.loader is not None
spec.loader.exec_module(helper)


@contextmanager
def isolated_env(**updates):
    keys = {
        "UNDERPAINT_PROMPT_HELPER_SYSTEM_PROMPT",
        "UNDERPAINT_PROMPT_HELPER_URL",
        "QWENCH_OPENAI_URL",
        "OPENAI_COMPAT_URL",
        "UNDERPAINT_USE_QWENCH_PROMPT_HELPER",
        "UNDERPAINT_PROMPT_HELPER_PORT",
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


class PromptHelperUnitTest(unittest.TestCase):
    def test_normalize_prompt_strips_common_prefixes_and_quotes(self):
        self.assertEqual(
            helper.normalize_prompt('  Rewritten prompt: "old brick wall"  '),
            "old brick wall",
        )
        self.assertEqual(
            helper.normalize_prompt("Improved Prompt: cracked plaster texture"),
            "cracked plaster texture",
        )

    def test_system_prompt_override_and_operation_specific_prompts(self):
        with isolated_env(UNDERPAINT_PROMPT_HELPER_SYSTEM_PROMPT="custom prompt"):
            self.assertEqual(helper.system_prompt("inpaint-prompt-improve"), "custom prompt")

        with isolated_env():
            self.assertIn("outpainting", helper.system_prompt("outpaint-prompt-improve"))
            self.assertIn("compact JSON", helper.system_prompt("decomposition-region-classify"))
            self.assertIn("inpainting prompt", helper.system_prompt("unknown-operation"))

    def test_fallback_rewrite_adds_contextual_detail(self):
        inpaint = helper.fallback_rewrite(
            {"operation": "inpaint-prompt-improve", "prompt": "repair torn paper"}
        )
        outpaint = helper.fallback_rewrite(
            {"operation": "outpaint-prompt-improve", "prompt": ""}
        )
        explain = helper.fallback_rewrite(
            {
                "operation": "inpaint-selection-explain",
                "selection": {"width": 123, "height": 45},
            }
        )

        self.assertIn("repair torn paper", inpaint)
        self.assertIn("lighting", inpaint)
        self.assertIn("texture", inpaint)
        self.assertIn("extend the image outward", outpaint)
        self.assertIn("perspective", outpaint)
        self.assertIn("123 x 45 px", explain)

    def test_json_value_from_text_extracts_fenced_or_embedded_json(self):
        self.assertEqual(
            helper.json_value_from_text('```json\n{"name": "Rose"}\n```'),
            {"name": "Rose"},
        )
        self.assertEqual(
            helper.json_value_from_text('result: [{"id": "a"}, {"id": "b"}] thanks'),
            [{"id": "a"}, {"id": "b"}],
        )

    def test_normalize_classification_clamps_and_defaults(self):
        classification = helper.normalize_classification(
            json.dumps(
                {
                    "label": "  Torn Photo Edge  ",
                    "description": "ragged border with paper fibers",
                    "depthRole": "nonsense",
                    "sceneRole": "",
                    "repairRole": "remove-from-base",
                    "confidence": 42,
                }
            )
        )

        self.assertEqual(classification["name"], "Torn Photo Edge")
        self.assertEqual(classification["promptPhrase"], "ragged border with paper fibers")
        self.assertEqual(classification["depthRole"], "ambiguous")
        self.assertEqual(classification["sceneRole"], "ambiguous")
        self.assertEqual(classification["repairRole"], "remove-from-base")
        self.assertEqual(classification["group"], "Foreground Objects")
        self.assertEqual(classification["confidence"], 1.0)

    def test_normalize_group_refinements_accepts_aliases_and_discards_junk(self):
        refinements = helper.normalize_group_refinements(
            json.dumps(
                {
                    "candidates": [
                        {
                            "candidateId": "region-a",
                            "index": "3",
                            "label": "wheel",
                            "parentObject": "Car",
                            "description": "front wheel",
                        },
                        "ignore me",
                        {"region": "not-an-int"},
                    ]
                }
            )
        )

        self.assertEqual(len(refinements), 1)
        self.assertEqual(refinements[0]["id"], "region-a")
        self.assertEqual(refinements[0]["regionIndex"], 3)
        self.assertEqual(refinements[0]["name"], "wheel")
        self.assertEqual(refinements[0]["group"], "Car")
        self.assertEqual(refinements[0]["promptPhrase"], "front wheel")

    def test_helper_url_precedence_and_qwench_fallback(self):
        with isolated_env(
            UNDERPAINT_PROMPT_HELPER_URL="http://localhost:8000/v1/",
            QWENCH_OPENAI_URL="http://localhost:9000/v1",
        ):
            self.assertEqual(helper.helper_url(), "http://localhost:8000/v1")

        with isolated_env(
            UNDERPAINT_USE_QWENCH_PROMPT_HELPER="1",
            UNDERPAINT_PROMPT_HELPER_PORT="18181",
        ):
            self.assertEqual(helper.helper_url(), "http://127.0.0.1:18181/v1")


if __name__ == "__main__":
    unittest.main()
