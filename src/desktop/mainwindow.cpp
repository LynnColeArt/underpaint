// SPDX-License-Identifier: GPL-3.0-or-later
extern "C" {
#include <dpengine/project.h>
}
#include "cmake-config/config.h"
#include "desktop/ai/aijobrunner.h"
#include "desktop/chat/chatbox.h"
#include "desktop/dialogs/abusereport.h"
#include "desktop/dialogs/aimodelmanagerdialog.h"
#include "desktop/dialogs/aipreferencesdialog.h"
#include "desktop/dialogs/animationexportdialog.h"
#include "desktop/dialogs/animationimportdialog.h"
#include "desktop/dialogs/brushsettingsdialog.h"
#include "desktop/dialogs/colordialog.h"
#include "desktop/dialogs/dumpplaybackdialog.h"
#include "desktop/dialogs/inputsettingsdialog.h"
#include "desktop/dialogs/invitedialog.h"
#include "desktop/dialogs/layoutsdialog.h"
#include "desktop/dialogs/logindialog.h"
#include "desktop/dialogs/playbackdialog.h"
#include "desktop/dialogs/projectrecordingsettingsdialog.h"
#include "desktop/dialogs/resetdialog.h"
#include "desktop/dialogs/resizedialog.h"
#include "desktop/dialogs/selectionalterdialog.h"
#include "desktop/dialogs/serverlogdialog.h"
#include "desktop/dialogs/sessionsettings.h"
#include "desktop/dialogs/sessionundodepthlimitdialog.h"
#include "desktop/dialogs/settingsdialog.h"
#include "desktop/dialogs/startdialog.h"
#include "desktop/dialogs/systeminfodialog.h"
#include "desktop/dialogs/tablettester.h"
#include "desktop/dialogs/toolbarconfigdialog.h"
#include "desktop/dialogs/touchtestdialog.h"
#include "desktop/dialogs/userinfodialog.h"
#include "desktop/docks/brushpalettedock.h"
#include "desktop/docks/colorcircle.h"
#include "desktop/docks/colorpalette.h"
#include "desktop/docks/colorsliders.h"
#include "desktop/docks/colorspinner.h"
#include "desktop/docks/layerlistdock.h"
#include "desktop/docks/navigator.h"
#include "desktop/docks/onionskins.h"
#include "desktop/docks/reference.h"
#include "desktop/docks/timeline.h"
#include "desktop/docks/titlewidget.h"
#include "desktop/docks/toolsettingsdock.h"
#include "desktop/filewrangler.h"
#include "desktop/main.h"
#include "desktop/mainwindow.h"
#include "desktop/notifications.h"
#include "desktop/scene/actionbaritem.h"
#include "desktop/scene/hudhandler.h"
#include "desktop/tabletinput.h"
#include "desktop/toolwidgets/annotationsettings.h"
#include "desktop/toolwidgets/brushsettings.h"
#include "desktop/toolwidgets/colorpickersettings.h"
#include "desktop/toolwidgets/fillsettings.h"
#include "desktop/toolwidgets/gradientsettings.h"
#include "desktop/toolwidgets/inspectorsettings.h"
#include "desktop/toolwidgets/lasersettings.h"
#include "desktop/toolwidgets/lassofillsettings.h"
#include "desktop/toolwidgets/rotationsettings.h"
#include "desktop/toolwidgets/selectionsettings.h"
#include "desktop/toolwidgets/transformsettings.h"
#include "desktop/utils/actionbuilder.h"
#include "desktop/utils/connections.h"
#include "desktop/utils/qtguicompat.h"
#include "desktop/utils/recents.h"
#include "desktop/utils/widgetutils.h"
#include "desktop/view/canvaswrapper.h"
#include "desktop/view/lock.h"
#include "desktop/widgets/canvasframe.h"
#include "desktop/widgets/dualcolorbutton.h"
#include "desktop/widgets/netstatus.h"
#include "desktop/widgets/nonaltstealingmenubar.h"
#include "desktop/widgets/projectrecordingstatusbutton.h"
#include "desktop/widgets/viewstatus.h"
#include "desktop/widgets/viewstatusbar.h"
#include "libclient/canvas/blendmodes.h"
#include "libclient/canvas/canvasmodel.h"
#include "libclient/canvas/documentmetadata.h"
#include "libclient/canvas/layerlist.h"
#include "libclient/canvas/paintengine.h"
#include "libclient/canvas/selectionmodel.h"
#include "libclient/canvas/transformmodel.h"
#include "libclient/canvas/userlist.h"
#include "libclient/config/config.h"
#include "libclient/document.h"
#include "libclient/drawdance/eventlog.h"
#include "libclient/drawdance/perf.h"
#include "libclient/drawdance/viewmode.h"
#include "libclient/export/animationsaverrunnable.h"
#include "libclient/import/canvasloaderrunnable.h"
#include "libclient/import/loadresult.h"
#include "libclient/net/client.h"
#include "libclient/net/login.h"
#include "libclient/parentalcontrols/parentalcontrols.h"
#include "libclient/tools/toolcontroller.h"
#include "libclient/utils/customshortcutmodel.h"
#include "libclient/utils/images.h"
#include "libclient/utils/logging.h"
#include "libclient/utils/scopedoverridecursor.h"
#include "libclient/utils/selectionalteration.h"
#include "libclient/utils/shortcutdetector.h"
#include "libclient/utils/wasmpersistence.h"
#include "libclient/view/enums.h"
#include "libshared/net/netutils.h"
#include "libshared/util/networkaccess.h"
#include "libshared/util/paths.h"
#include "libshared/util/whatismyip.h"
#include <QActionGroup>
#include <QApplication>
#include <QAbstractItemView>
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QDesktopServices>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QDockWidget>
#include <QDoubleSpinBox>
#include <QEasingCurve>
#include <QFile>
#include <QFormLayout>
#include <QGroupBox>
#include <QIcon>
#include <QImageReader>
#include <QImageWriter>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QListWidget>
#include <QMap>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QPainter>
#include <QPointer>
#include <QProgressDialog>
#include <QProcess>
#include <QProcessEnvironment>
#include <QPushButton>
#include <QPixmap>
#include <QScopedValueRollback>
#include <QScreen>
#include <QSettings>
#include <QShortcutEvent>
#include <QSignalBlocker>
#include <QSlider>
#include <QSplitter>
#include <QSpinBox>
#include <QStatusBar>
#include <QStyle>
#include <QStringList>
#include <QTabBar>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTextEdit>
#include <QThread>
#include <QThreadPool>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QUrl>
#include <QVariantAnimation>
#include <QVBoxLayout>
#include <QWindow>
#include <atomic>
#include <climits>
#include <functional>
#include <memory>
#include <utility>
#ifdef DRAWPILE_PROJECT_DIALOG
#	include "desktop/dialogs/projectdialog.h"
#endif
#ifdef DRAWPILE_PROJECT_INFO_DIALOG
#	include "desktop/dialogs/projectinfodialog.h"
#endif
#ifdef DRAWPILE_TIMELAPSE_DIALOG
#	include "desktop/dialogs/timelapsedialog.h"
#endif
#ifdef Q_OS_WIN
#	include "desktop/bundled/kis_tablet/kis_tablet_support_win.h"
#endif
#ifdef __EMSCRIPTEN__
#	include "libclient/wasmsupport.h"
#endif
#ifdef DP_HAVE_BUILTIN_SERVER
#	include "libclient/server/builtinserver.h"
#endif
#ifdef Q_OS_MACOS
static constexpr auto CTRL_KEY = Qt::META;
#	include "desktop/widgets/macmenu.h"
#else
static constexpr auto CTRL_KEY = Qt::CTRL;
#endif

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
static constexpr int DEBOUNCE_MS = 250;

// clang-format off

MainWindow::MainWindow(bool restoreWindowPosition, bool singleSession)
	: QMainWindow(),
	  m_singleSession(singleSession),
	  m_smallScreenMode(isInitialSmallScreenMode()),
	  m_updatingInterfaceMode(true),
	  m_splitter(nullptr),
	  m_dockToolSettings(nullptr),
	  m_dockBrushPalette(nullptr),
	  m_dockInput(nullptr),
	  m_dockLayers(nullptr),
	  m_dockColorPalette(nullptr),
	  m_dockColorCircle(nullptr),
	  m_dockNavigator(nullptr),
	  m_dockOnionSkins(nullptr),
	  m_dockTimeline(nullptr),
	  m_dockReference(nullptr),
	  m_chatbox(nullptr),
	  m_viewLock(nullptr),
	  m_canvasView(nullptr),
	  m_viewStatusBar(nullptr),
	  m_netstatus(nullptr),
	  m_viewstatus(nullptr),
	  m_statusChatButton(nullptr),
	  m_playbackDialog(nullptr),
	  m_dumpPlaybackDialog(nullptr),
	  m_sessionSettings(nullptr),
	  m_serverLogDialog(nullptr),
#ifndef __EMSCRIPTEN__
	  m_recentMenu(nullptr),
#endif
	  m_lastLayerViewMode(nullptr),
	  m_currentdoctools(nullptr),
	  m_admintools(nullptr),
	  m_canvasbgtools(nullptr),
	  m_resizetools(nullptr),
	  m_putimagetools(nullptr),
	  m_undotools(nullptr),
	  m_drawingtools(nullptr),
	  m_brushSlots(nullptr),
	  m_dockToggles(nullptr),
	  m_tempToolSwitchShortcut(nullptr),
	  m_wasSessionLocked(false),
	  m_notificationsMuted(false),
	  m_initialCatchup(false),
	  m_toolStateNormal(true),
	  m_doc(nullptr)
#ifndef __EMSCRIPTEN__
	  , m_exitAction(RUNNING)
#endif
{
	// Avoid flickering of intermediate states.
	setUpdatesEnabled(false);
	// Animations are slow, ugly and cause crashes on Android and the browser.
	setAnimated(false);

	m_saveSplitterDebounce.setSingleShot(true);
	m_saveWindowDebounce.setSingleShot(true);
	m_updateIntendedDockStateDebounce.setSingleShot(true);
	m_restoreIntendedDockStateDebounce.setSingleShot(true);
	m_saveSplitterDebounce.setInterval(DEBOUNCE_MS);
	m_saveWindowDebounce.setInterval(DEBOUNCE_MS);
	m_updateIntendedDockStateDebounce.setInterval(DEBOUNCE_MS);
	m_restoreIntendedDockStateDebounce.setInterval(50);
#ifdef SINGLE_MAIN_WINDOW
	m_refitWindowDebounce.setSingleShot(true);
	m_refitWindowDebounce.setInterval(DEBOUNCE_MS);
#endif

	// The document (initially empty)
	DrawpileApp *app = &dpApp();
	config::Config *cfg = app->config();
	m_doc = new Document(app->canvasImplementation(), app->config(), this);

	// Set up the main window widgets
	// The central widget consists of a custom status bar and a splitter
	// which includes the chat box and the main view.
	// We don't use the normal QMainWindow statusbar to save some vertical space for the docks.
	QWidget *centralwidget = new QWidget;
	QVBoxLayout *mainwinlayout = new QVBoxLayout(centralwidget);
	mainwinlayout->setContentsMargins(0, 0, 0 ,0);
	// clang-format on
	mainwinlayout->setSpacing(0);
	setCentralWidget(centralwidget);

	// Work area is split between the canvas view and the chatbox
	m_splitter = new QSplitter(Qt::Vertical, centralwidget);
	m_splitterOriginalHandleWidth = m_splitter->handleWidth();

	mainwinlayout->addWidget(m_splitter);

	// Create custom status bar
	m_viewStatusBar = new widgets::ViewStatusBar;
	m_viewStatusBar->setSizeGripEnabled(false);
	mainwinlayout->addWidget(m_viewStatusBar);

	// Create status indicator widgets
	m_viewstatus = new widgets::ViewStatus(this);
	m_viewstatus->setHidden(m_smallScreenMode);
	m_viewStatusBar->addPermanentWidget(m_viewstatus);

	m_netstatus = new widgets::NetStatus(this);
	m_viewStatusBar->addPermanentWidget(m_netstatus);

	// Statusbar chat button: this is normally hidden and only shown
	// when there are unread chat messages.
	m_statusChatButton = new QToolButton(this);
	m_statusChatButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
	m_statusChatButton->setAutoRaise(true);
	m_statusChatButton->setIcon(
		QIcon::fromTheme(QStringLiteral("drawpile_chat")));
	utils::setWidgetRetainSizeWhenHidden(m_statusChatButton, true);
	m_statusChatButton->hide();
	m_viewStatusBar->addPermanentWidget(m_statusChatButton);

	m_statusAutoRecordButton = new widgets::ProjectRecordingStatusButton(this);
	m_viewStatusBar->addPermanentWidget(m_statusAutoRecordButton);

	m_viewLock = new view::Lock(this);

	int SPLITTER_WIDGET_IDX = 0;

	// Create canvas view (first splitter item)
	m_canvasView =
		view::CanvasWrapper::instantiate(app->canvasImplementation(), this);
	m_canvasView->setShowToggleItems(m_smallScreenMode, m_leftyMode);

	m_canvasFrame = new widgets::CanvasFrame(m_canvasView->viewWidget());
	m_splitter->addWidget(m_canvasFrame);
	m_splitter->setCollapsible(SPLITTER_WIDGET_IDX++, false);

	// Create the chatbox
	m_chatbox = new widgets::ChatBox(m_doc, m_smallScreenMode, this);
	m_splitter->addWidget(m_chatbox);

	connect(
		m_chatbox, &widgets::ChatBox::reattachNowPlease, this, [this, cfg]() {
			m_splitter->addWidget(m_chatbox);
			QByteArray state = cfg->getLastWindowViewState();
			bool haveSplitterState =
				!state.isEmpty() && m_splitter->restoreState(state);
			if(!haveSplitterState || m_chatbox->isCollapsed()) {
				int h = height();
				m_splitter->setSizes({h * 2 / 3, h / 3});
			}
		});

	// Nice initial division between canvas and chat
	{
		const int h = height();
		m_splitter->setSizes(QList<int>() << (h * 2 / 3) << (h / 3));
	}

	m_dualColorButton = new widgets::DualColorButton(this);

	// Create docks
	createDocks();
	resetDefaultDocks();

	// Crete persistent dialogs
	m_sessionSettings = new dialogs::SessionSettingsDialog(m_doc, this);
	m_serverLogDialog = new dialogs::ServerLogDialog(this);
	m_serverLogDialog->setModel(m_doc->serverLog());
	// clang-format off

	// Document <-> Main window connections
	connect(m_doc, &Document::canvasChanged, this, &MainWindow::onCanvasChanged);
	connect(m_doc, &Document::canvasSaveStarted, this, &MainWindow::onCanvasSaveStarted);
	connect(m_doc, &Document::canvasSaved, this, &MainWindow::onCanvasSaved);
#ifdef __EMSCRIPTEN__
	connect(m_doc, &Document::canvasDownloadStarted, this, &MainWindow::onCanvasDownloadStarted);
	connect(m_doc, &Document::canvasDownloadReady, this, &MainWindow::onCanvasDownloadReady);
	connect(m_doc, &Document::canvasDownloadError, this, &MainWindow::onCanvasDownloadError);
#endif
	connect(m_doc, &Document::templateExported, this, &MainWindow::onTemplateExported);
	connect(m_doc, &Document::dirtyCanvas, this, &MainWindow::setWindowModified);
	connect(m_doc, &Document::sessionTitleChanged, this, &MainWindow::updateTitle);
	connect(m_doc, &Document::currentPathChanged, this, &MainWindow::updateTitle);
#if !defined(Q_OS_ANDROID) && !defined(__EMSCRIPTEN__)
	connect(
		m_doc, &Document::exportPathChanged, this,
		&MainWindow::updateExportPath);
#endif
	connect(
		m_doc, &Document::projectPathChanged, this,
		&MainWindow::updateProjectActions);
	connect(m_doc, &Document::recorderStateChanged, this, &MainWindow::setRecorderStatus);
	connect(m_doc, &Document::sessionResetState, this, &MainWindow::showResetNoticeDialog, Qt::QueuedConnection);
	// clang-format on
	connect(
		m_doc, &Document::permissionDenied, this,
		&MainWindow::showPermissionDeniedMessage);

	connect(
		m_doc, &Document::resetImageTooLarge, this,
		&MainWindow::showResetImageTooLargeErrorMessage, Qt::QueuedConnection);

	// Tool dock connections
	m_tempToolSwitchShortcut = new ShortcutDetector(this);

	m_canvasView->connectCanvasFrame(m_canvasFrame);
	m_canvasView->connectDocument(m_doc);
	m_canvasView->connectMainWindow(this);
	m_canvasView->connectNavigator(m_dockNavigator);
	m_canvasView->connectLock(m_viewLock);
	m_canvasView->connectViewStatus(m_viewstatus);
	m_canvasView->connectViewStatusBar(m_viewStatusBar);
	m_canvasView->connectToolSettings(m_dockToolSettings);
	// clang-format off

	connect(m_dockLayers, &docks::LayerList::layerSelected, this, &MainWindow::triggerUpdateLockState);
	connect(m_dockLayers, &docks::LayerList::activeLayerVisibilityChanged, this, &MainWindow::triggerUpdateLockState);

	connect(m_dockToolSettings, &docks::ToolSettings::toolChanged, this, &MainWindow::toolChanged);
	connect(m_dockToolSettings, &docks::ToolSettings::activeBrushChanged, this, &MainWindow::triggerUpdateLockState);
	connect(
		m_dockToolSettings, &docks::ToolSettings::showMessageRequested, this,
		[this](const QString &message) {
			m_canvasView->showPopupNotice(message);
		});

	// Color docks
	connect(m_dockToolSettings, &docks::ToolSettings::foregroundColorChanged, m_dockColorPalette, &docks::ColorPaletteDock::setColor);
	connect(m_dockToolSettings, &docks::ToolSettings::foregroundColorChanged, m_dockColorSpinner, &docks::ColorSpinnerDock::setColor);
	connect(m_dockToolSettings, &docks::ToolSettings::foregroundColorChanged, m_dockColorSliders, &docks::ColorSliderDock::setColor);
	connect(m_dockToolSettings, &docks::ToolSettings::foregroundColorChanged, m_dockColorCircle, &docks::ColorCircleDock::setColor);
	connect(m_dockToolSettings, &docks::ToolSettings::lastUsedColorsChanged, m_dockColorPalette, &docks::ColorPaletteDock::setLastUsedColors);
	connect(m_dockToolSettings, &docks::ToolSettings::lastUsedColorsChanged, m_dockColorSpinner, &docks::ColorSpinnerDock::setLastUsedColors);
	connect(m_dockToolSettings, &docks::ToolSettings::lastUsedColorsChanged, m_dockColorSliders, &docks::ColorSliderDock::setLastUsedColors);
	connect(m_dockToolSettings, &docks::ToolSettings::lastUsedColorsChanged, m_dockColorCircle, &docks::ColorCircleDock::setLastUsedColors);
	connect(m_dockColorPalette, &docks::ColorPaletteDock::colorSelected, m_dockToolSettings, &docks::ToolSettings::setForegroundColor);
	connect(m_dockColorSpinner, &docks::ColorSpinnerDock::colorSelected, m_dockToolSettings, &docks::ToolSettings::setForegroundColor);
	connect(m_dockColorSliders, &docks::ColorSliderDock::colorSelected, m_dockToolSettings, &docks::ToolSettings::setForegroundColor);
	connect(m_dockColorCircle, &docks::ColorCircleDock::colorSelected, m_dockToolSettings, &docks::ToolSettings::setForegroundColor);
	connect(m_dockReference, &docks::ReferenceDock::colorPicked, m_dockToolSettings, &docks::ToolSettings::setForegroundColor);
	connect(
		m_dockToolSettings, &docks::ToolSettings::colorAdjustRequested,
		m_dockColorSliders, &docks::ColorSliderDock::adjustColor);

	// Dual color button
	connect(
		m_dockToolSettings, &docks::ToolSettings::foregroundColorChanged,
		m_dualColorButton, &widgets::DualColorButton::setForegroundColor);
	connect(
		m_dockToolSettings, &docks::ToolSettings::backgroundColorChanged,
		m_dualColorButton, &widgets::DualColorButton::setBackgroundColor);
	connect(
		m_dualColorButton, &widgets::DualColorButton::foregroundClicked,
		m_dockToolSettings, &docks::ToolSettings::changeForegroundColor);
	connect(
		m_dualColorButton, &widgets::DualColorButton::backgroundClicked,
		m_dockToolSettings, &docks::ToolSettings::changeBackgroundColor);
	connect(
		m_dualColorButton, &widgets::DualColorButton::swapClicked,
		m_dockToolSettings, &docks::ToolSettings::swapColors);
	connect(
		m_dualColorButton, &widgets::DualColorButton::resetClicked,
		m_dockToolSettings, &docks::ToolSettings::resetColors);

	// Network client <-> UI connections
	connect(
		m_doc, &Document::catchupProgress, this,
		&MainWindow::updateCatchupProgress);
	connect(
		m_doc, &Document::catchupProgress, m_netstatus,
		&widgets::NetStatus::setCatchupProgress);
	connect(
		m_doc, &Document::streamResetProgress, this,
		&MainWindow::updateStreamResetProgress);

	connect(
		m_doc->client(), &net::Client::serverStatusUpdate, m_viewStatusBar,
		&widgets::ViewStatusBar::setSessionHistorySize);
	connect(
		m_doc->client(), &net::Client::lagMeasured, m_viewStatusBar,
		&widgets::ViewStatusBar::setLatency);

	connect(m_chatbox, &widgets::ChatBox::message, m_doc->client(), &net::Client::sendMessage);
	connect(m_dockTimeline, &docks::Timeline::timelineEditCommands, m_doc->client(), &net::Client::sendCommands);

	connect(m_serverLogDialog, &dialogs::ServerLogDialog::opCommand, m_doc->client(), &net::Client::sendMessage);
	connect(m_dockLayers, &docks::LayerList::layerCommands, m_doc->client(), &net::Client::sendCommands);

	connect(m_doc->client(), &net::Client::userInfoRequested, this, &MainWindow::sendUserInfo);
	connect(m_doc->client(), &net::Client::currentBrushRequested, this, &MainWindow::sendCurrentBrush);
	connect(m_doc->client(), &net::Client::currentBrushReceived, this, &MainWindow::receiveCurrentBrush);

	connect(m_doc->client(), &net::Client::bansImported, m_sessionSettings, &dialogs::SessionSettingsDialog::bansImported);
	connect(m_doc->client(), &net::Client::bansExported, m_sessionSettings, &dialogs::SessionSettingsDialog::bansExported);
	connect(m_doc->client(), &net::Client::bansImpExError, m_sessionSettings, &dialogs::SessionSettingsDialog::bansImpExError);
	connect(m_sessionSettings, &dialogs::SessionSettingsDialog::requestBanImport, m_doc->client(), &net::Client::requestBanImport);
	connect(m_sessionSettings, &dialogs::SessionSettingsDialog::requestBanExport, m_doc->client(), &net::Client::requestBanExport);
	connect(m_sessionSettings, &dialogs::SessionSettingsDialog::requestUpdateAuthList, m_doc->client(), &net::Client::requestUpdateAuthList);

	// Tool controller <-> UI connections
	connect(m_doc->toolCtrl(), &tools::ToolController::colorUsed, m_dockToolSettings, &docks::ToolSettings::addLastUsedColor);
	connect(m_doc->toolCtrl(), &tools::ToolController::actionCancelled, m_dockToolSettings->colorPickerSettings(), &tools::ColorPickerSettings::cancelPickFromScreen);
	// clang-format on
	connect(
		m_dockToolSettings, &docks::ToolSettings::foregroundColorChanged,
		m_dockToolSettings->colorPickerSettings(),
		&tools::ColorPickerSettings::setCurrentColor);
	connect(
		m_doc->toolCtrl(), &tools::ToolController::toolStateChanged, this,
		&MainWindow::setToolState, Qt::QueuedConnection);
	connect(
		m_doc->toolCtrl(), &tools::ToolController::statusTextRequested,
		m_viewStatusBar, &widgets::ViewStatusBar::showToolMessage);
	// clang-format off

	connect(m_dockLayers, &docks::LayerList::layerSelected, m_doc->toolCtrl(), &tools::ToolController::setActiveLayer);
	connect(
		m_dockLayers, &docks::LayerList::layerSelectionChanged,
		m_doc->toolCtrl(), &tools::ToolController::setSelectedLayers);
	connect(m_dockLayers, &docks::LayerList::layerSelected, m_dockTimeline, &docks::Timeline::setCurrentLayer);
	connect(m_dockTimeline, &docks::Timeline::layerSelected, m_dockLayers, &docks::LayerList::selectLayer);
	connect(
		m_dockTimeline, &docks::Timeline::blankLayerSelected, m_dockLayers,
		&docks::LayerList::clearLayerSelection);
	connect(m_dockTimeline, &docks::Timeline::trackSelected, m_dockLayers, &docks::LayerList::setTrackId);
	m_dockLayers->setTrackId(m_dockTimeline->currentTrackId());
	connect(m_dockTimeline, &docks::Timeline::frameSelected, m_dockLayers, &docks::LayerList::setFrame);
	m_dockLayers->setFrame(m_dockTimeline->currentFrame());
	connect(m_doc->toolCtrl(), &tools::ToolController::activeAnnotationChanged,
			m_dockToolSettings->annotationSettings(), &tools::AnnotationSettings::setSelectionId);
	connect(
		m_dockToolSettings->annotationSettings(),
		&tools::AnnotationSettings::selectionIdChanged, this,
		&MainWindow::updateSelectTransformActions);
	connect(
		m_dockToolSettings->annotationSettings(),
		&tools::AnnotationSettings::showAnnotationsRequested, [this] {
			QAction *showannotations = getAction("showannotations");
			if(!showannotations->isChecked()) {
				showannotations->toggle();
			}
		});
	connect(
		m_dockLayers, &docks::LayerList::layerSelected, m_chatbox,
		&widgets::ChatBox::setCurrentLayer);
	connect(
		m_dockToolSettings->brushSettings(),
		&tools::BrushSettings::editBrushRequested, this,
		&MainWindow::showBrushSettingsDialogBrush);
	connect(
		m_dockToolSettings->brushSettings(),
		&tools::BrushSettings::stabilizerSettingsRequested, this,
		&MainWindow::showInputSettingsDialogStabilizerPage);
	connect(
		m_dockToolSettings->selectionSettings(),
		&tools::SelectionSettings::stabilizerSettingsRequested, this,
		&MainWindow::showInputSettingsDialogStabilizerPage);
	connect(
		m_dockToolSettings->lassoFillSettings(),
		&tools::LassoFillSettings::stabilizerSettingsRequested, this,
		&MainWindow::showInputSettingsDialogStabilizerPage);
	connect(
		m_dockBrushPalette, &docks::BrushPalette::editBrushRequested, this,
		&MainWindow::showBrushSettingsDialogPreset);
	connect(
		m_dockToolSettings->laserPointerSettings(),
		&tools::LaserPointerSettings::showLaserTrailsRequested, [this] {
			QAction *showlasers = getAction("showlasers");
			if(!showlasers->isChecked()) {
				showlasers->toggle();
			}
		});

	// Network status changes
	connect(m_doc, &Document::serverConnected, this, &MainWindow::onServerConnected);
	connect(m_doc, &Document::serverLoggedIn, this, &MainWindow::onServerLogin);
	connect(m_doc, &Document::serverDisconnected, this, &MainWindow::onServerDisconnected);
	connect(m_doc, &Document::serverDisconnectedAgain, this, &MainWindow::onServerDisconnectedAgain);
	connect(m_doc, &Document::serverDisconnected, this, [this]() {
		m_viewStatusBar->setSessionHistorySize(-1);
		m_viewStatusBar->setLatency(-1);
	});
	connect(
		m_doc, &Document::compatibilityModeChanged, this,
		&MainWindow::onCompatibilityModeChanged);
	connect(
		m_doc, &Document::compatibilityModeChanged, m_dockToolSettings,
		&docks::ToolSettings::setCompatibilityMode);
	connect(m_doc, &Document::sessionNsfmChanged, this, &MainWindow::onNsfmChanged);

	connect(m_doc, &Document::serverConnected, m_netstatus, &widgets::NetStatus::connectingToHost);
	connect(m_doc, &Document::serverRedirected, m_netstatus, &widgets::NetStatus::connectingToHost);
	connect(m_doc, &Document::serverSocketTypeChanged, m_netstatus, &widgets::NetStatus::setSocketType);
	connect(m_doc->client(), &net::Client::serverDisconnecting, m_netstatus, &widgets::NetStatus::hostDisconnecting);
	connect(m_doc, &Document::serverDisconnected, m_netstatus, &widgets::NetStatus::hostDisconnected);
	connect(m_sessionSettings, &dialogs::SessionSettingsDialog::joinPasswordChanged, m_netstatus, &widgets::NetStatus::setJoinPassword);
	// clang-format on
	connect(
		m_doc, &Document::sessionHasPasswordChanged, m_netstatus,
		&widgets::NetStatus::setHaveJoinPassword);
	connect(
		m_doc, &Document::sessionPasswordChanged, m_netstatus,
		&widgets::NetStatus::setJoinPassword);
	connect(
		m_doc, &Document::sessionOutOfSpaceChanged, this,
		&MainWindow::triggerUpdateLockState);
	connect(
		m_doc, &Document::preparingResetChanged, this,
		&MainWindow::triggerUpdateLockState);
	connect(
		this, &MainWindow::lockStateUpdateRequested, this,
		&MainWindow::updateLockState, Qt::QueuedConnection);
	// clang-format off

	connect(m_doc->client(), SIGNAL(bytesReceived(int)), m_netstatus, SLOT(bytesReceived(int)));
	connect(m_doc->client(), &net::Client::bytesSent, m_netstatus, &widgets::NetStatus::bytesSent);
	connect(m_doc->client(), &net::Client::lagMeasured, m_netstatus, &widgets::NetStatus::lagMeasured);

	// clang-format on
	connect(app, &DrawpileApp::focusCanvas, this, [this, cfg] {
		if(cfg->getDoubleTapAltToFocusCanvas()) {
			m_canvasView->viewWidget()->setFocus();
		}
	});

	// Create actions and menus
	setupActions();
	setDrawingToolsEnabled(false);
#if !defined(Q_OS_ANDROID) && !defined(__EMSCRIPTEN__)
	updateExportPath(m_doc->exportPath());
#endif
	m_dockToolSettings->triggerUpdate();

	// Restore settings and show the window
	updateTitle();
	readSettings(restoreWindowPosition);
	setupBrushShortcuts();
	setupHud();

	// Set status indicators
	updateLockState();
	setRecorderStatus(false);

	// Actually paint the window
	reenableUpdates();

#ifdef Q_OS_MACOS
	MacMenu::instance()->addWindow(this);
#endif

	setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
	setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
	setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
	setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);

#ifdef SINGLE_MAIN_WINDOW
	app->deleteAllMainWindowsExcept(this);
#endif

	CFG_BIND_NOTIFY(cfg, InterfaceMode, this, MainWindow::updateInterfaceMode);
	CFG_BIND_SET(cfg, LeftyMode, this, MainWindow::setLeftyMode);
	CFG_BIND_SET(cfg, ToolBarConfig, this, MainWindow::setToolBarConfig);
	CFG_BIND_NOTIFY(
		cfg, TemporaryToolSwitch, this, MainWindow::updateTemporaryToolSwitch);
	CFG_BIND_NOTIFY(
		cfg, TemporaryToolSwitchMs, this,
		MainWindow::updateTemporaryToolSwitch);
	CFG_BIND_SET(cfg, ActionBar, this, MainWindow::setActionBarSetting);
	CFG_BIND_SET(
		cfg, ActionBarLocation, this, MainWindow::setActionBarLocation);
	CFG_BIND_ACTION(
		cfg, AutomaticAlphaPreserve, getAction("layerautomaticalphapreserve"));
	CFG_BIND_ACTION(cfg, LeftyMode, getAction("smallscreenleftymode"));
	CFG_BIND_SET(
		cfg, DonationLinksEnabled, this, MainWindow::setDonationLinkEnabled);
#ifndef __EMSCRIPTEN__
	CFG_BIND_SET(
		cfg, PreferredSaveFormat, this, MainWindow::setPreferredSaveFormat);
#endif
	CFG_BIND_SET(
		cfg, ShowViewModeNotices, m_viewLock,
		view::Lock::setShowViewModeNotices);
	CFG_BIND_ACTION(cfg, ShowViewModeNotices, getAction("layerviewnotices"));
	cfg->trySubmit();

	m_updatingInterfaceMode = false;
	updateInterfaceMode();

#ifdef SINGLE_MAIN_WINDOW
	connect(
		compat::widgetScreen(*this), &QScreen::availableGeometryChanged, this,
		&MainWindow::startRefitWindowDebounce);
#endif
	startRefitWindowDebounce();

	connect(
		this, &MainWindow::dockTabUpdateRequested, this,
		&MainWindow::updateDockTabs, Qt::QueuedConnection);
	emit dockTabUpdateRequested();

	connect(
		&m_updateIntendedDockStateDebounce, &QTimer::timeout, this,
		&MainWindow::updateIntendedDockState);
	connect(
		&m_restoreIntendedDockStateDebounce, &QTimer::timeout, this,
		&MainWindow::restoreIntendedDockState);
	connect(
		this, &MainWindow::resizeReactionRequested, this,
		&MainWindow::reactToResize, Qt::QueuedConnection);
#if defined(Q_OS_ANDROID) && defined(KRITA_QT_SCREEN_DENSITY_ADJUSTMENT)
	connect(
		app, &DrawpileApp::androidScalingDialogShown, this,
		&MainWindow::handleAndroidScalingDialogShown, Qt::QueuedConnection);
	connect(
		app, &DrawpileApp::androidScalingDialogDismissed, this,
		&MainWindow::handleAndroidScalingDialogDismissed, Qt::QueuedConnection);
#endif
	connect(
		this, &MainWindow::smallScreenPreviewRequested, this,
		&MainWindow::showSmallScreenModePreview, Qt::QueuedConnection);
	m_resizeReactionPending = false;
	m_updateIntendedDockStateDebounce.stop();
	m_restoreIntendedDockStateDebounce.stop();
	QTimer::singleShot(DEBOUNCE_MS, this, &MainWindow::updateIntendedDockState);

	int brushMode = int(m_dockToolSettings->brushSettings()->getBrushMode());
	updateFreehandToolButton(
		brushMode == int(tools::BrushSettings::UnknownMode)
			? int(tools::BrushSettings::NormalMode)
			: brushMode);

	updateProjectActions();

	if(m_smallScreenMode) {
		if(app->isAndroidScalingDialogShown()) {
			Q_EMIT smallScreenPreviewRequested();
		}
	} else if(!m_chatbox->isCollapsed()) {
		getAction("togglechat")->trigger();
	}

	DRAWPILE_FS_PERSIST();
}

MainWindow::~MainWindow()
{
#ifdef Q_OS_MACOS
	MacMenu::instance()->removeWindow(this);
#endif

	// Clear this out first so there will be no weird signals emitted
	// while the document is being torn down.
	m_canvasView->disposeScene();
	// clang-format off

	// Make sure all child dialogs are closed
	for(auto *child : findChildren<QDialog *>(QString(), Qt::FindDirectChildrenOnly)) {
		delete child;
	}

	dpAppConfig()->trySubmit();
	DRAWPILE_FS_PERSIST();
}

// clang-format on
QMenu *MainWindow::createPopupMenu()
{
	QMenu *menu = QMainWindow::createPopupMenu();
	menu->addSeparator();
	menu->addAction(getAction("toolbarconfig"));
	if(m_smallScreenMode) {
		menu->addAction(getAction("smallscreensidetoolbar"));
		menu->addAction(getAction("smallscreenbottomtoolbar"));
		menu->addAction(getAction("smallscreenleftymode"));
	}
	return menu;
}

void MainWindow::autoJoin(
	const QUrl &url, const QString &autoRecordPath, int connectStrategy)
{
	if(m_singleSession) {
		m_doc->client()->setSessionUrl(url);
		connectToSession(url, autoRecordPath, connectStrategy, false);
	} else {
		dialogs::StartDialog *dlg =
			showStartDialogOnPage(int(dialogs::StartDialog::Join));
		dlg->autoJoin(url, autoRecordPath, connectStrategy);
	}
}

void MainWindow::onCanvasChanged(canvas::CanvasModel *canvas)
{
	setNormalLayerViewMode();

	m_canvasView->setCanvas(canvas);
	m_flipbookState = dialogs::Flipbook::State();

	connect(
		this, &MainWindow::initialCatchupFinished, canvas,
		&canvas::CanvasModel::requestProjectRecordingMetadata);

	canvas::AclState *aclState = canvas->aclState();
	connect(
		aclState, &canvas::AclState::localOpChanged, this,
		&MainWindow::onOperatorModeChange);
	connect(
		aclState, &canvas::AclState::localLockChanged, this,
		&MainWindow::triggerUpdateLockState);
	connect(
		aclState, &canvas::AclState::resetLockChanged, this,
		&MainWindow::triggerUpdateLockState);
	connect(
		aclState, &canvas::AclState::featureAccessChanged, this,
		&MainWindow::onFeatureAccessChange);
	connect(
		aclState, &canvas::AclState::ownFeatureLimitChanged, this,
		&MainWindow::onFeatureLimitChanged);

	canvas::PaintEngine *paintEngine = canvas->paintEngine();
	connect(
		paintEngine, &canvas::PaintEngine::undoDepthLimitSet, this,
		&MainWindow::onUndoDepthLimitSet);
	tools::ToolController *toolCtrl = m_doc->toolCtrl();
	connect(
		toolCtrl, &tools::ToolController::selectionEditActiveChanged,
		paintEngine, &canvas::PaintEngine::setSelectionEditActive);

	connect(canvas, &canvas::CanvasModel::chatMessageReceived, this, [this]() {
		// Show a "new message" indicator when the chatbox is collapsed
		QList<int> sizes = m_splitter->sizes();
		if(sizes.length() > 1 && sizes.at(1) == 0) {
			m_statusChatButton->show();
		}
	});

	connect(
		canvas, &canvas::CanvasModel::layerAutoselectRequest, m_dockLayers,
		&docks::LayerList::autoSelectLayer);
	connect(
		canvas, &canvas::CanvasModel::colorPicked, m_dockToolSettings,
		&docks::ToolSettings::setForegroundColor);
	connect(
		canvas, &canvas::CanvasModel::colorPickFinished,
		m_dockToolSettings->colorPickerSettings(),
		&tools::ColorPickerSettings::addColor);
	connect(
		canvas, &canvas::CanvasModel::canvasInspected,
		m_dockToolSettings->inspectorSettings(),
		&tools::InspectorSettings::onCanvasInspected);
	connect(
		canvas, &canvas::CanvasModel::previewAnnotationRequested, toolCtrl,
		&tools::ToolController::setActiveAnnotation);

	connect(
		m_dockLayers, &docks::LayerList::fillSourceSet, canvas->layerlist(),
		&canvas::LayerListModel::setFillSourceLayerId);
	connect(
		canvas->layerlist(), &canvas::LayerListModel::fillSourceSet,
		m_dockLayers, &docks::LayerList::updateFillSourceLayerId);
	connect(
		canvas->layerlist(), &canvas::LayerListModel::fillSourceSet,
		m_dockToolSettings->fillSettings(),
		&tools::FillSettings::updateFillSourceLayerId);
	connect(
		canvas->layerlist(), &canvas::LayerListModel::fillSourceSet, this,
		&MainWindow::triggerUpdateLockState);
	connect(
		canvas->selection(), &canvas::SelectionModel::selectionChanged, this,
		&MainWindow::updateSelectTransformActions);
	connect(
		canvas->selection(), &canvas::SelectionModel::selectionChanged, this,
		&MainWindow::triggerUpdateLockStateOnSelectionChange);
	connect(
		canvas->transform(), &canvas::TransformModel::transformChanged, this,
		&MainWindow::updateSelectTransformActions);
	connect(
		canvas->selection(), &canvas::SelectionModel::selectionChanged,
		m_dockToolSettings->fillSettings(),
		&tools::FillSettings::updateSelection);

	connect(
		canvas, &canvas::CanvasModel::userJoined, this,
		[this](int, const QString &name) {
			m_viewStatusBar->showMessage(tr("🙋 %1 joined!").arg(name), 2000);
		});

	connect(
		m_serverLogDialog, &dialogs::ServerLogDialog::inspectModeChanged,
		canvas, [this, canvas](unsigned int contextId) {
			canvas->inspectCanvas(
				contextId,
				m_dockToolSettings->inspectorSettings()->isShowTiles());
		});
	connect(
		m_serverLogDialog, &dialogs::ServerLogDialog::inspectModeStopped,
		canvas, &canvas::CanvasModel::stopInspectingCanvas);

	updateLayerViewMode();

	m_dockLayers->setCanvas(canvas);
	m_serverLogDialog->setUserList(canvas->userlist());
	m_dockNavigator->setCanvasModel(canvas);
	m_dockTimeline->setCanvas(canvas);

	connect(
		m_dockTimeline, &docks::Timeline::frameSelected, paintEngine,
		&canvas::PaintEngine::setViewFrame);
	connect(
		m_dockTimeline, &docks::Timeline::trackHidden, paintEngine,
		&canvas::PaintEngine::setTrackVisibility);
	connect(
		m_dockTimeline, &docks::Timeline::trackOnionSkinEnabled, paintEngine,
		&canvas::PaintEngine::setTrackOnionSkin);
	connect(
		m_dockTimeline, &docks::Timeline::trackMoveLockEnabled, paintEngine,
		&canvas::PaintEngine::setTrackMoveLock);
	connect(
		m_dockTimeline, &docks::Timeline::frameViewModeRequested, this,
		&MainWindow::autoSetFrameViewMode);

	connect(
		m_dockOnionSkins, &docks::OnionSkinsDock::onionSkinsChanged,
		paintEngine, &canvas::PaintEngine::setOnionSkins);
	m_dockOnionSkins->triggerUpdate();

	connect(
		canvas, &canvas::CanvasModel::restoreLocalStateViewMode, this,
		&MainWindow::restoreViewMode);

	connect(
		canvas, &canvas::CanvasModel::projectRecordingStarted, this,
		&MainWindow::onProjectRecordingStarted);
	connect(
		canvas, &canvas::CanvasModel::projectRecordingStopped, this,
		&MainWindow::onProjectRecordingStopped);
	connect(
		canvas, &canvas::CanvasModel::projectRecordingSizeLimitWarning, this,
		&MainWindow::showProjectRecordingSizeLimitWarning);
	connect(
		canvas, &canvas::CanvasModel::projectRecordingErrorOccurred, this,
		&MainWindow::showProjectRecordingError);

	m_dockToolSettings->inspectorSettings()->setUserList(canvas->userlist());

	// Make sure the UI matches the default feature access level
	m_currentdoctools->setEnabled(true);
	setDrawingToolsEnabled(true);
	for(int i = 0; i < DP_FEATURE_COUNT; ++i) {
		DP_Feature f = DP_Feature(i);
		onFeatureAccessChange(f, aclState->canUseFeature(f));
	}
	for(int i = 0; i < DP_FEATURE_LIMIT_COUNT; ++i) {
		DP_FeatureLimit fl = DP_FeatureLimit(i);
		onFeatureLimitChanged(fl, aclState->featureLimit(fl));
	}
	onUndoDepthLimitSet(paintEngine->undoDepthLimit());
	paintEngine->setShowSelectionMask(
		getAction("showselectionmask")->isChecked());
	paintEngine->setSelectionEditActive(toolCtrl->isSelectionEditActive());
	getAction("resetsession")->setEnabled(true);
	getAction("autorecord")->setChecked(canvas->isProjectRecording());
	m_statusAutoRecordButton->setCanvas(canvas);

	QAction *retainProjectRecordings = searchAction("retainprojectrecordings");
	if(retainProjectRecordings) {
		retainProjectRecordings->setChecked(
			canvas->isRetainProjectRecordings());
		connect(
			canvas, &canvas::CanvasModel::retainProjectRecordingsChanged,
			retainProjectRecordings, &QAction::setChecked);
		connect(
			retainProjectRecordings, &QAction::triggered, canvas,
			&canvas::CanvasModel::setRetainProjectRecordings);
	}

	updateSelectTransformActions();
	onProjectRecordingStopped(false);

	if(!m_doc->client()->isConnected()) {
		Q_EMIT hostSessionEnabled(true);
	}
}

bool MainWindow::canReplace() const
{
	return !getReplacementCriteria();
}

#ifdef __EMSCRIPTEN__
bool MainWindow::shouldPreventUnload() const
{
	return m_singleSession ? m_doc->client()->isLoggedIn() : !canReplace();
}

void MainWindow::handleMouseLeave()
{
	m_canvasView->clearKeys();
	dpAppConfig()->trySubmit();
	DRAWPILE_FS_PERSIST();
}
#endif

MainWindow::ReplacementCriteria MainWindow::getReplacementCriteria() const
{
	ReplacementCriteria rc;
	if(m_doc->isDirty()) {
		rc.setFlag(ReplacementCriterion::Dirty);
	}
	if(m_doc->client()->isConnected()) {
		rc.setFlag(ReplacementCriterion::Connected);
	}
	if(m_doc->isRecording()) {
		rc.setFlag(ReplacementCriterion::Recording);
	}
	if(m_playbackDialog || m_dumpPlaybackDialog) {
		rc.setFlag(ReplacementCriterion::Playback);
	}
	return rc;
}

void MainWindow::questionOpenFileWindowReplacement(
	const std::function<void(bool)> &block)
{
	questionWindowReplacement(
		tr("Open"),
		tr("You're about to open a new file and close this window."), block);
}

void MainWindow::questionWindowReplacement(
	const QString &title, const QString &action,
	const std::function<void(bool)> &block)
{
#ifdef SINGLE_MAIN_WINDOW
	ReplacementCriteria rc = getReplacementCriteria();
	if(rc) {
		QStringList effects;
		if(rc.testFlag(ReplacementCriterion::Connected)) {
			//: This is an effect of what will happen when closing the window.
			//: It will potentially be put into a list with other effects.
			effects.append(tr("disconnect you from the session"));
		}
		if(rc.testFlag(ReplacementCriterion::Dirty)) {
			//: This is an effect of what will happen when closing the window.
			//: It will potentially be put into a list with other effects.
			effects.append(tr("lose any unsaved changes"));
		}
		if(rc.testFlag(ReplacementCriterion::Recording)) {
			//: This is an effect of what will happen when closing the window.
			//: It will potentially be put into a list with other effects.
			effects.append(tr("stop your recording"));
		}
		if(rc.testFlag(ReplacementCriterion::Playback)) {
			//: This is an effect of what will happen when closing the window.
			//: It will potentially be put into a list with other effects.
			effects.append(tr("discard your playback"));
		}
		QString message;
		switch(effects.size()) {
		case 1:
			message = tr("Doing so will %1.").arg(effects[0]);
			break;
		case 2:
			message =
				tr("Doing so will %1 and %2.").arg(effects[0], effects[1]);
			break;
		case 3:
			message = tr("Doing so will %1, %2 and %3.")
						  .arg(effects[0], effects[1], effects[2]);
			break;
		case 4:
			message = tr("Doing so will %1, %2, %3 and %4.")
						  .arg(effects[0], effects[1], effects[2], effects[3]);
			break;
		default:
			message = tr("Doing so will %1.")
						  .arg(QLocale().createSeparatedList(effects));
			break;
		}
		QWidget *parent = getStartDialogOrThis();
		QMessageBox *box = utils::makeQuestion(
			parent, title,
			QStringLiteral("<p>%1 %2</p><p>%3</p>")
				.arg(
					action.toHtmlEscaped(), message.toHtmlEscaped(),
					tr("Are you sure you want to continue?").toHtmlEscaped()));
		box->button(QMessageBox::Yes)->setText(tr("Yes, continue"));
		box->button(QMessageBox::No)->setText(tr("No, cancel"));
		connect(box, &QMessageBox::accepted, parent, std::bind(block, true));
		connect(box, &QMessageBox::rejected, parent, std::bind(block, false));
		utils::showMessageBox(box);
	} else {
		block(true);
	}
#else
	Q_UNUSED(title);
	Q_UNUSED(action);
	block(true);
#endif
}

void MainWindow::prepareWindowReplacement()
{
#ifndef __EMSCRIPTEN__
	if(windowState().testFlag(Qt::WindowFullScreen)) {
		toggleFullscreen();
	}
#endif
	saveWindowState();
	saveSplitterState();
	dpAppConfig()->trySubmit();
	DRAWPILE_FS_PERSIST();
}

void MainWindow::createNewWindow(const std::function<void(MainWindow *)> &block)
{
	// Create the new window at top-level in the event loop, otherwise we can
	// end up deleting ourselves from under us.
	QTimer::singleShot(0, this, [this, block] {
		MainWindow *win = new MainWindow(false);
		emit windowReplacementFailed(win);
		block(win);
	});
}

void MainWindow::loadBlankDocument(const QSize &size, const QColor &background)
{
	clearPromptHistory();
	bool autoRecord = dpAppConfig()->getAutoRecordHost();
	m_doc->loadBlank(
		size, background,
		QApplication::translate("docks::LayerList", "Layer") +
			QStringLiteral(" 1"),
		QCoreApplication::translate("widgets::TimelineWidget", "Track") +
			QStringLiteral(" 1"),
		autoRecord);
}

void MainWindow::addRecentFile(const QString &file, int source)
{
	if(!file.isEmpty()) {
#ifndef __EMSCRIPTEN__
		utils::Recents &recents = dpApp().recents();
		recents.addFile(file);
#endif

		canvas::CanvasModel *canvas = m_doc->canvas();
		if(canvas && canvas->isProjectRecording()) {
			using Source = utils::Recents::Source;

			QString metadataName;
			switch(Source(source)) {
			case Source::Unknown:
			case Source::Open:
				// Not relevant.
				break;
			case Source::Save:
			case Source::SaveAs:
			case Source::SavePreResetImageAs:
			case Source::Download:
				metadataName = QStringLiteral("last_save");
				break;
			case Source::Export:
			case Source::ExportSelection:
			case Source::DownloadSelection:
				metadataName = QStringLiteral("last_export");
				break;
			}

			if(!metadataName.isEmpty()) {
				canvas->setProjectRecordingMetadataString(metadataName, file);
			}
		}
	}
}

/**
 * Set window title according to currently open file and session
 */
void MainWindow::updateTitle()
{
	QString name;
	if(m_doc->haveCurrentPath()) {
		QFileInfo info(m_doc->currentPath());
		name = info.completeBaseName();
	} else {
		name = tr("Untitled");
	}

	if(m_doc->sessionTitle().isEmpty()) {
		setWindowTitle(QStringLiteral("%1[*]").arg(name));
	} else {
		setWindowTitle(
			QStringLiteral("%1[*] - %2").arg(name, m_doc->sessionTitle()));
	}

#ifdef Q_OS_MACOS
	MacMenu::instance()->updateWindow(this);
#endif
}

void MainWindow::setDrawingToolsEnabled(bool enable)
{
	bool actuallyEnabled = enable && m_doc->canvas();
	m_drawingtools->setEnabled(actuallyEnabled);
	m_deselecttools->setEnabled(actuallyEnabled);
	if(m_freehandButton) {
		m_freehandButton->setEnabled(actuallyEnabled);
	}
}

void MainWindow::aboutToShowMenu()
{
	canvas::CanvasModel *canvas = m_doc->canvas();
	canvas::AclState *aclState = canvas ? canvas->aclState() : nullptr;
	m_canvasbgtools->setEnabled(
		aclState && aclState->canUseFeature(DP_FEATURE_BACKGROUND));
	m_putimagetools->setEnabled(
		aclState && aclState->canUseFeature(DP_FEATURE_PUT_IMAGE));
	m_resizetools->setEnabled(
		aclState && aclState->canUseFeature(DP_FEATURE_RESIZE));
	m_undotools->setEnabled(
		aclState && aclState->canUseFeature(DP_FEATURE_UNDO));
}

void MainWindow::aboutToHideMenu()
{
	m_canvasbgtools->setEnabled(true);
	m_putimagetools->setEnabled(true);
	m_resizetools->setEnabled(true);
	m_undotools->setEnabled(true);
}

#if !defined(Q_OS_ANDROID) && !defined(__EMSCRIPTEN__)
void MainWindow::updateExportPath(const QString &path)
{
	QAction *action = getAction(QStringLiteral("exportdocumentagain"));
	if(path.isEmpty()) {
		action->setText(tr("Export Again"));
		action->setEnabled(false);
	} else {
		QFileInfo info(path);
		action->setText(tr("Export Again to %1").arg(info.fileName()));
		action->setEnabled(!path.isEmpty() && !m_doc->isSaveInProgress());
	}
}
#endif
// clang-format off

/**
 * Load customized shortcuts
 */
void MainWindow::loadShortcuts(const QVariantMap &cfg)
{
	disconnect(m_textCopyConnection);
	const QKeySequence standardCopyShortcut { QKeySequence::Copy };

	for(auto *a : findChildren<QAction*>()) {
		const QString &name = a->objectName();
		if(!name.isEmpty() &&
		   !brushes::BrushPresetModel::looksLikeBrushShortcutObjectName(name)) {
			if(cfg.contains(name)) {
				const auto v = cfg.value(name);
				QList<QKeySequence> shortcuts;

				if(v.canConvert<QKeySequence>()) {
					QKeySequence shortcut = v.value<QKeySequence>();
					if(!shortcuts.contains(shortcut)) {
						shortcuts.append(shortcut);
					}
				} else {
					const auto list = v.toList();
					for(const auto &vv : list) {
						if(vv.canConvert<QKeySequence>()) {
							QKeySequence shortcut = vv.value<QKeySequence>();
							if(!shortcuts.contains(shortcut)) {
								shortcuts.append(shortcut);
							}
						}
					}
				}
				a->setShortcuts(shortcuts);

			} else {
				a->setShortcuts(CustomShortcutModel::getDefaultShortcuts(name));
			}

			if(a->shortcut() == standardCopyShortcut) {
				m_textCopyConnection = connect(a, &QAction::triggered, this, &MainWindow::copyText);
			}

			// If an action has a shortcut, show it in the tooltip
			a->setToolTip(
				a->shortcut().isEmpty()
					? QString()
					: utils::makeActionShortcutText(a->text(), a->shortcut()));

			a->setAutoRepeat(a->property("shortcutAutoRepeats").toBool());
		}
	}

	// Update enabled status of certain actions
	QAction *uncensorAction = getAction("layerviewuncensor");
	const bool canUncensor = !parentalcontrols::isLayerUncensoringBlocked();
	uncensorAction->setEnabled(canUncensor);
	if(!canUncensor) {
		uncensorAction->setChecked(false);
		updateLayerViewMode();
	}

	for(dialogs::StartDialog *dlg : findChildren<dialogs::StartDialog *>(QString{}, Qt::FindDirectChildrenOnly)) {
		setStartDialogActions(dlg);
	}

	for(dialogs::Flipbook *fp : findChildren<dialogs::Flipbook *>(QString{}, Qt::FindDirectChildrenOnly)) {
		fp->setRefreshShortcuts(getAction("showflipbook")->shortcuts());
	}

	updateFreehandToolButton(
		int(m_dockToolSettings->brushSettings()->getBrushMode()));
}

// clang-format on
void MainWindow::setBrushSlotCount(int count)
{
	tools::BrushSettings *brushSettings = m_dockToolSettings->brushSettings();
	brushSettings->setBrushSlotCount(count);
	int brushSlotCount = brushSettings->brushSlotCount();

	QVector<QPair<QString, bool>> nameDisabledPairs;
	nameDisabledPairs.reserve(19);

	for(int i = 0; i < 10; ++i) {
		QString name = QStringLiteral("quicktoolslot-%1").arg(i);
		bool enabled = i <= brushSlotCount;
		nameDisabledPairs.append({name, !enabled});

		QAction *q = findChild<QAction *>(name);
		if(q) {
			if(enabled) {
				addAction(q);
			} else {
				removeAction(q);
			}
		} else {
			qWarning("Tool slot action %d not found", i);
		}
	}

	for(int i = 0; i < 9; ++i) {
		QString name = QStringLiteral("swapslot%1").arg(i);
		bool enabled = i < brushSlotCount;
		nameDisabledPairs.append({name, !enabled});

		QAction *s = findChild<QAction *>(name);
		if(s) {
			if(enabled) {
				addAction(s);
			} else {
				removeAction(s);
			}
		} else {
			qWarning("Swap slot action %d not found", i);
		}
	}

	CustomShortcutModel::changeDisabledActionNames(nameDisabledPairs);
	emit dpApp().shortcutsChanged();
}

void MainWindow::setNormalLayerViewMode()
{
	if(!m_layerViewNormal->isChecked()) {
		m_layerViewNormal->trigger();
	}
}

void MainWindow::autoSetFrameViewMode()
{
	if(!m_layerViewCurrentFrame->isChecked() &&
	   m_dockTimeline->isActuallyVisible()) {
		m_layerViewCurrentFrame->trigger();
		m_canvasView->showPopupNotice(
			tr("Switched to frame view mode.\n"
			   "You can exit it via the View menu or the timeline."));
	}
}


void MainWindow::toggleLayerViewMode()
{
	if(!m_doc->canvas())
		return;
	// If any of the special view modes is triggered again, we want to toggle
	// back to the normal view mode. This allows the user to e.g. switch between
	// single layer and normal mode by mashing the same shortcut. Otherwise
	// pressing the shortcut again would have no useful effect anyway.
	QAction *actions[] = {
		m_layerViewCurrentLayer,
		m_layerViewCurrentGroup,
		m_layerViewCurrentFrame,
	};
	for(QAction *action : actions) {
		if(action->isChecked() && m_lastLayerViewMode == action) {
			m_layerViewNormal->setChecked(true);
			break;
		}
	}
	updateLayerViewMode();
}

void MainWindow::updateLayerViewMode()
{
	if(m_doc->canvas()) {
		bool censor = !getAction("layerviewuncensor")->isChecked();

		DP_ViewMode mode;
		QAction *action;
		if((action = m_layerViewCurrentLayer)->isChecked()) {
			mode = DP_VIEW_MODE_LAYER;
		} else if((action = m_layerViewCurrentGroup)->isChecked()) {
			mode = DP_VIEW_MODE_GROUP;
		} else if((action = m_layerViewCurrentFrame)->isChecked()) {
			mode = DP_VIEW_MODE_FRAME;
		} else {
			action = m_layerViewNormal;
			mode = DP_VIEW_MODE_NORMAL;
		}
		m_lastLayerViewMode = action;

		m_doc->canvas()->paintEngine()->setViewMode(mode, censor);
		triggerUpdateLockState();
	}
}

void MainWindow::restoreViewMode(int viewMode, bool revealCensored)
{
	if(m_doc->canvas()) {
		getAction("layerviewuncensor")->setChecked(revealCensored);
		switch(viewMode) {
		case int(DP_VIEW_MODE_NORMAL):
			m_layerViewNormal->setChecked(true);
			break;
		case int(DP_VIEW_MODE_LAYER):
			m_layerViewCurrentLayer->setChecked(true);
			break;
		case int(DP_VIEW_MODE_GROUP):
			m_layerViewCurrentGroup->setChecked(true);
			break;
		case int(DP_VIEW_MODE_FRAME):
			m_layerViewCurrentFrame->setChecked(true);
			break;
		default:
			qWarning("Can't restore unknown view mode %d", viewMode);
			break;
		}
		updateLayerViewMode();
	}
}
// clang-format off

/**
 * Read and apply mainwindow related settings.
 */
void MainWindow::readSettings(bool windowpos)
{
	config::Config *cfg = dpAppConfig();

	auto setEraserAction = [=](int action) {
#if defined(__EMSCRIPTEN__) || defined(Q_OS_ANDROID)
		m_canvasView->setEnableEraserOverride(
			action == int(tools::EraserAction::Override));
#else
		if(action == int(tools::EraserAction::Switch)) {
			connect(
				&dpApp(), &DrawpileApp::eraserNear, m_dockToolSettings,
				&docks::ToolSettings::switchToEraserSlot, Qt::UniqueConnection);
		} else {
			disconnect(
				&dpApp(), &DrawpileApp::eraserNear, m_dockToolSettings,
				&docks::ToolSettings::switchToEraserSlot);
		}
		if(action == int(tools::EraserAction::Override)) {
			connect(
				&dpApp(), &DrawpileApp::eraserNear, m_dockToolSettings,
				&docks::ToolSettings::switchToEraserMode, Qt::UniqueConnection);
		} else {
			disconnect(
				&dpApp(), &DrawpileApp::eraserNear, m_dockToolSettings,
				&docks::ToolSettings::switchToEraserMode);
		}
#endif
	};
	CFG_BIND_SET_FN(cfg, TabletEraserAction, this, setEraserAction);

	// clang-format on
	tools::BrushSettings *brushSettings = m_dockToolSettings->brushSettings();
	CFG_BIND_SET(
		cfg, ShareBrushSlotColor, brushSettings,
		tools::BrushSettings::setShareBrushSlotColor);
	CFG_BIND_SET(
		cfg, BrushPresetsAttach, brushSettings,
		tools::BrushSettings::setBrushPresetsAttach);

	// Restore previously used window size and position
	resize(cfg->getLastWindowSize());
	if(windowpos) {
		utils::moveIfOnScreen(this, cfg->getLastWindowPosition());
	}

	// Show self
	utils::showWindow(this, cfg->getLastWindowMaximized(), true);

	// The following state restoration requires the window to be resized, but Qt
	// does that lazily on the next event loop iteration. So we forcefully flush
	// the event loop here to actually get the window resized before continuing.
	dpApp().processEvents();

	if(m_smallScreenMode) {
		initSmallScreenState();
	} else {
		restoreSettings(cfg);
	}

	connect(m_splitter, &QSplitter::splitterMoved, this, [=] {
		m_saveSplitterDebounce.start();
	});

	// Restore remembered actions
	m_actionsConfig = cfg->getLastWindowActions();
	for(QAction *act : actions()) {
		if(act->isCheckable() && act->property("remembered").toBool()) {
			act->setChecked(m_actionsConfig.value(
				act->objectName(), act->property("defaultValue").toBool()));
			connect(
				act, &QAction::toggled, this, [this, act, cfg](bool checked) {
					m_actionsConfig[act->objectName()] = checked;
					cfg->setLastWindowActions(m_actionsConfig);
				});
		}
	}

	if(m_smallScreenMode) {
		setFreezeDocks(true);
		setDockOptions(dockOptions() | QMainWindow::VerticalTabs);
	}

	// Customize shortcuts
	CFG_BIND_SET(cfg, Shortcuts, this, MainWindow::loadShortcuts);
	CFG_BIND_SET(cfg, BrushSlotCount, this, MainWindow::setBrushSlotCount);

#ifndef __EMSCRIPTEN__
	// Restore recent files
	if(!m_singleSession) {
		dpApp().recents().bindFileMenu(m_recentMenu);
	}
#endif

	connect(
		&m_saveWindowDebounce, &QTimer::timeout, this,
		&MainWindow::saveWindowState);
	connect(
		&m_saveSplitterDebounce, &QTimer::timeout, this,
		&MainWindow::saveSplitterState);
#ifdef SINGLE_MAIN_WINDOW
	connect(
		&m_refitWindowDebounce, &QTimer::timeout, this,
		&MainWindow::refitWindow);
#endif

	// QMainWindow produces no event when there is a change that would cause the
	// serialised state to be different, so we will just have to make some
	// guesses listening for relevant changes in the docked widgets.
	for(QWidget *w :
		findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly)) {
		if(w->inherits("QDockWidget") || w->inherits("QToolBar")) {
			w->installEventFilter(this);
		}
	}

	refitWindow();
	startRefitWindowDebounce();
}

void MainWindow::restoreSettings(config::Config *cfg)
{
	{
		const QByteArray lastWindowState = cfg->getLastWindowState();
		QScopedValueRollback<bool> rollback(m_restoringDockState, true);
		if(!lastWindowState.isEmpty()) {
			deactivateAllDocks();
			restoreState(lastWindowState);
		} else {
			initDefaultDocks();
		}
	}

	const QByteArray lastWindowViewState = cfg->getLastWindowViewState();
	if(!lastWindowViewState.isEmpty()) {
		m_splitter->restoreState(lastWindowViewState);
		m_splitter->setHandleWidth(m_splitterOriginalHandleWidth);
	}

	const QVariantMap docksConfig = cfg->getLastWindowDocks();
	for(QDockWidget *dw :
		findChildren<QDockWidget *>(QString(), Qt::FindDirectChildrenOnly)) {
		if(!dw->objectName().isEmpty()) {
			const QVariantMap dock =
				docksConfig.value(dw->objectName()).value<QVariantMap>();
			if(dock.value("undockable", false).toBool()) {
				dw->setFloating(true);
				dw->setAllowedAreas(Qt::NoDockWidgetArea);
			}
		}
	}
}

void MainWindow::initSmallScreenState()
{
	initDefaultDocks();
	for(QDockWidget *dw :
		findChildren<QDockWidget *>(QString(), Qt::FindDirectChildrenOnly)) {
		dw->hide();
	}
	m_splitter->setHandleWidth(0);
	m_chatbox->hide();
	m_toolBarDraw->show();
}

void MainWindow::initDefaultDocks()
{
	// More event loop flushing to get Qt to *actually* apply these sizes. It
	// gets horribly confused by hidden docks and the event loop must be woken
	// up numerous times to actually apply the sizes. This arrangement works,
	// I'd recommend not messing with it unless there's actually an issue.
	setDefaultDockSizes();
	dpApp().processEvents();
	m_dockTimeline->hide();
	m_dockOnionSkins->hide();
	dpApp().processEvents();
	setDefaultDockSizes();
	dpApp().processEvents();
}

void MainWindow::setDefaultDockSizes()
{
	int leftWidth = 320, leftHeight = 220;
	int rightWidth = 260, rightHeight = 220;
	int topHeight = 300;
	resizeDocks(
		{m_dockToolSettings, m_dockBrushPalette, m_dockColorSpinner,
		 m_dockColorSliders, m_dockColorPalette, m_dockColorCircle,
		 m_dockReference, m_dockPromptManager, m_dockLayers},
		{leftWidth, leftWidth, rightWidth, rightWidth, rightWidth, rightWidth,
		 rightWidth, rightWidth, rightWidth},
		Qt::Horizontal);
	resizeDocks(
		{m_dockToolSettings, m_dockColorSpinner, m_dockColorSliders,
		 m_dockColorPalette, m_dockColorCircle, m_dockReference, m_dockTimeline,
		 m_dockOnionSkins, m_dockPromptManager},
		{leftHeight, rightHeight, rightHeight, rightHeight, rightHeight,
		 rightHeight, topHeight, topHeight, rightHeight},
		Qt::Vertical);
}
// clang-format off

bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
	switch (event->type()) {
	// This is a guessed list of events that might cause the QMainWindow state
	// to change, and it seems to work OK but may be wrong or excessive
	case QEvent::Show:
	case QEvent::Hide:
	case QEvent::Move:
	case QEvent::Resize:
	case QEvent::Close:
		m_saveWindowDebounce.start();
		startIntendedDockStateDebounce();
		startRefitWindowDebounce();
		break;
	case QEvent::Shortcut: {
		QShortcutEvent *shortcutEvent = static_cast<QShortcutEvent *>(event);
		if(shortcutEvent->isAmbiguous()) {
			handleAmbiguousShortcut(shortcutEvent);
			return true;
		}
		break;
	}
	default: {}
	}
	return QMainWindow::eventFilter(object, event);
}

// clang-format on
void MainWindow::handleAmbiguousShortcut(QShortcutEvent *shortcutEvent)
{
	const QKeySequence &keySequence = shortcutEvent->key();
	QVector<QAction *> actions;
	QStringList matchingShortcuts;

	{
		CustomShortcutModel shortcutsModel;
		shortcutsModel.loadShortcuts(dpAppConfig()->getShortcuts());
		for(const CustomShortcut &shortcut :
			shortcutsModel.getShortcutsMatching(keySequence)) {

			QAction *action =
				findChild<QAction *>(shortcut.name, Qt::FindDirectChildrenOnly);
			if(action) {
				actions.append(action);
			}

			matchingShortcuts.append(
				QString("<li>%1</li>").arg(shortcut.title.toHtmlEscaped()));
		}
	}

	dpApp().brushPresets()->presetModel()->getShortcutActions(
		[&](const QString &name, const QString &text,
			const QKeySequence &shortcut) {
			if(shortcut == keySequence) {
				QAction *action = searchAction(name);
				if(action) {
					actions.append(action);
					matchingShortcuts.append(
						QString("<li>%1</li>").arg(text.toHtmlEscaped()));
				}
			}
		});

	// Shortcuts may conflict with stuff like the main window menu bar. We can
	// resolve those pseudo.conflicts in the favor of our custom shortcuts.
	if(actions.size() == 1) {
		actions.first()->trigger();
		return;
	}

	matchingShortcuts.sort(Qt::CaseInsensitive);

	QString message =
		tr("<p>The shortcut '%1' is ambiguous, it matches:</p><ul>%2</ul>")
			.arg(keySequence.toString(QKeySequence::NativeText))
			.arg(matchingShortcuts.join(QString()));

	QMessageBox *box = utils::makeMessage(
		this, tr("Ambiguous Shortcut"), message, QString(),
		QMessageBox::Warning, QMessageBox::Close);

	QPushButton *fixButton = box->addButton(tr("Fix"), QMessageBox::ActionRole);

	connect(box, &QMessageBox::finished, box, [this, box, fixButton] {
		if(box->clickedButton() == fixButton) {
			showSettings()->initiateFixShortcutConflicts();
		}
	});
	utils::showMessageBox(box);
}

void MainWindow::stopProjectRecording()
{
	toggleProjectRecording(false);
}

void MainWindow::toggleProjectRecording(bool enabled)
{
	canvas::CanvasModel *canvas = m_doc->canvas();
	if(canvas && canvas->isProjectRecording() != enabled) {
		if(enabled) {
			int sourceType;
			if(m_doc->client()->isConnected()) {
				sourceType = DP_PROJECT_SOURCE_SESSION;
			} else if(m_doc->haveCurrentPath()) {
				sourceType = DP_PROJECT_SOURCE_FILE;
			} else {
				sourceType = DP_PROJECT_SOURCE_BLANK;
			}
			canvas->startProjectRecording(dpAppConfig(), sourceType);
		} else {
			QMessageBox *box = utils::makeQuestion(
				this, tr("Disable Autorecovery"),
				tr("Are you sure you want to disable autorecovery for this "
				   "session?"),
				tr("Unsaved data will be discarded and can't be recovered. You "
				   "will not be able to create a timelapse."));
			//: "Yes" button in the "do you want to turn off autosaving" dialog.
			box->button(QMessageBox::Yes)->setText(tr("Yes, disable"));
			//: "No" button in the "do you want to turn off autosaving" dialog.
			box->button(QMessageBox::No)->setText(tr("No, keep enabled"));
			connect(box, &QMessageBox::accepted, this, [this] {
				canvas::CanvasModel *currentCanvas = m_doc->canvas();
				if(currentCanvas) {
					currentCanvas->discardProjectRecording();
				}
			});
			connect(box, &QMessageBox::rejected, this, [this] {
				canvas::CanvasModel *currentCanvas = m_doc->canvas();
				bool isProjectRecording =
					currentCanvas && currentCanvas->isProjectRecording();
				getAction("autorecord")->setChecked(isProjectRecording);
			});
			utils::showMessageBox(box);
		}
	}
}

void MainWindow::onProjectRecordingStarted()
{
	getAction("autorecord")->setChecked(true);
	updateProjectActions();
}

void MainWindow::onProjectRecordingStopped(bool notify)
{
	getAction("autorecord")->setChecked(false);
	updateProjectActions();
	m_canvasView->hideProjectSizeLimitWarning();
	if(notify) {
		m_canvasView->showPopupNotice(tr("Autorecovery deactivated"));
	}
}

void MainWindow::setProjectRecordingSizeLimitInBytes(size_t sizeLimitInBytes)
{
	m_canvasView->hideProjectSizeLimitWarning();
	canvas::CanvasModel *canvas = m_doc->canvas();
	if(canvas && canvas->isProjectRecording()) {
		canvas->setProjectRecordingSizeLimitInBytes(sizeLimitInBytes);
	}
}

void MainWindow::showProjectRecordingSizeLimitWarning(
	size_t sizeInBytes, size_t sizeLimitInBytes)
{
	int percent =
		qFloor(double(sizeInBytes) / double(sizeLimitInBytes) * 100.0);
	m_canvasView->showProjectSizeLimitWarning(
		tr("The autorecovery file has exceeded %1% of the %2 size limit. "
		   "Autorecovery will be disabled if the limit is reached.")
			.arg(
				QString::number(percent),
				utils::paths::formatFileSize(qint64(sizeLimitInBytes))));
}

void MainWindow::showProjectRecordingError(const QString &message)
{
	QString objectName = QStringLiteral("projectrecordingerrormessagebox");
	if(!findChild<QMessageBox *>(objectName, Qt::FindDirectChildrenOnly)) {
		QMessageBox *box = utils::showWarning(
			this, tr("Autorecovery Error"),
			tr("Autorecovery error: %1").arg(message),
			tr("Autorecovery will be disabled for the current session. The "
			   "file will be left available for recovery. If you continue, you "
			   "will not be able to create a timelapse."));
		box->setObjectName(objectName);
		connect(box, &QMessageBox::finished, this, [this] {
			canvas::CanvasModel *canvas = m_doc->canvas();
			if(canvas) {
				canvas->unblockProjectRecordingErrors();
			}
		});
	}
}

void MainWindow::updateProjectActions()
{
#if defined(DRAWPILE_PROJECT_DIALOG) || defined(DRAWPILE_TIMELAPSE_DIALOG)
	bool isProjectRecording = m_doc->isProjectRecording();
	bool enabled = !m_doc->isSaveInProgress() &&
				   (!m_doc->projectPath().isEmpty() || isProjectRecording);
#endif
#ifdef DRAWPILE_PROJECT_DIALOG
	getAction(QStringLiteral("projectoverview"))->setEnabled(enabled);
#endif
#ifdef DRAWPILE_TIMELAPSE_DIALOG
	getAction(QStringLiteral("maketimelapse"))->setEnabled(enabled);
#endif
}

#ifdef DRAWPILE_PROJECT_DIALOG
void MainWindow::requestProjectOverview()
{
	QString objectName = QStringLiteral("projectdialog");
	dialogs::ProjectDialog *dlg = findChild<dialogs::ProjectDialog *>(
		objectName, Qt::FindDirectChildrenOnly);
	if(dlg) {
		dlg->activateWindow();
		dlg->raise();
	} else {
		dlg = new dialogs::ProjectDialog(this);
		dlg->setAttribute(Qt::WA_DeleteOnClose);
		dlg->setObjectName(objectName);

		m_doc->saveToTemporaryProjectFile(
			[this, dlgPtr = QPointer<dialogs::ProjectDialog>(dlg)](
				const QString &tempPath, const QString &errorMessage) {
				bool haveTempPath = !tempPath.isEmpty();
				if(dlgPtr) {
					if(haveTempPath) {
						dlgPtr->setTempPath(tempPath);
					} else {
						dlgPtr->close();
						utils::showWarning(
							this, tr("Save Failed"),
							tr("Error preparing project overview file."),
							errorMessage.isEmpty() ? tr("Unknown error.")
												   : errorMessage);
					}
				} else if(haveTempPath) {
					QFile::remove(tempPath);
				}
			});

		utils::showWindow(dlg);
	}
}
#endif

#ifdef DRAWPILE_TIMELAPSE_DIALOG
void MainWindow::requestTimelapseDialog()
{
	QString objectName = QStringLiteral("timelapsedialog");
	dialogs::TimelapseDialog *dlg = findChild<dialogs::TimelapseDialog *>(
		objectName, Qt::FindDirectChildrenOnly);
	if(dlg) {
		dlg->activateWindow();
		dlg->raise();
	} else {
		canvas::CanvasModel *canvas = m_doc->canvas();
		if(canvas) {
			canvas::PaintEngine *paintEngine = canvas->paintEngine();

			QRect crop;
			if(const canvas::TransformModel *transform = canvas->transform();
			   transform->isActive()) {
				crop = transform->dstQuad().boundingRect().toAlignedRect();
			} else if(
				canvas::SelectionModel *sel = canvas->selection();
				sel->isValid()) {
				crop = sel->bounds();
			}

			dlg = new dialogs::TimelapseDialog(
				paintEngine, crop, m_layerViewCurrentFrame->isChecked(),
				m_flipbookState.speedPercent, m_flipbookState.loopStart - 1,
				m_flipbookState.loopEnd - 1, this);
			dlg->setAttribute(Qt::WA_DeleteOnClose);
			dlg->setObjectName(objectName);

			m_doc->saveToTemporaryProjectFile(
				[this, dlgPtr = QPointer<dialogs::TimelapseDialog>(dlg)](
					const QString &tempPath, const QString &errorMessage) {
					bool haveTempPath = !tempPath.isEmpty();
					if(dlgPtr) {
						if(haveTempPath) {
							dlgPtr->setTempPath(tempPath);
						} else {
							dlgPtr->close();
							utils::showWarning(
								this, tr("Save Failed"),
								tr("Error preparing timelapse file."),
								errorMessage.isEmpty() ? tr("Unknown error.")
													   : errorMessage);
						}
					} else if(haveTempPath) {
						QFile::remove(tempPath);
					}
				});

			utils::showWindow(dlg);
		}
	}
}
#endif

void MainWindow::showSelectionMaskColorPicker()
{
	QString objectName = QStringLiteral("selectionmaskcolordialog");
	color_widgets::ColorDialog *dlg = findChild<color_widgets::ColorDialog *>(
		objectName, Qt::FindDirectChildrenOnly);
	if(dlg) {
		dlg->activateWindow();
		dlg->raise();
	} else {
		config::Config *cfg = dpAppConfig();
		dlg = dialogs::newDeleteOnCloseColorDialog(
			cfg->getSelectionColor(), this);
		dlg->setPreviewDisplayMode(color_widgets::ColorPreview::SplitColor);
		dlg->setAlphaEnabled(false);
		dlg->setObjectName(objectName);

		color_widgets::ColorPreview *preview =
			dlg->findChild<color_widgets::ColorPreview *>(
				nullptr, Qt::FindChildrenRecursively);
		if(preview) {
			preview->setComparisonColor(
				config::Config::defaultSelectionColor());
		}

		connect(
			dlg, &color_widgets::ColorDialog::colorSelected, this,
			[cfg](const QColor &color) {
				cfg->setSelectionColor(color);
			});
		utils::showWindow(dlg, shouldShowDialogMaximized());
	}
}

void MainWindow::showInputSettingsDialogStabilizerPage()
{
	showInputSettingsDialog()->showStabilizerPage();
}

dialogs::InputSettingsDialog *MainWindow::showInputSettingsDialog()
{
	QString objectName = QStringLiteral("inputsettingsdialog");
	dialogs::InputSettingsDialog *dlg =
		findChild<dialogs::InputSettingsDialog *>(
			objectName, Qt::FindDirectChildrenOnly);
	if(dlg) {
		dlg->activateWindow();
		dlg->raise();
	} else {
		dlg = new dialogs::InputSettingsDialog(this);
		dlg->setAttribute(Qt::WA_DeleteOnClose);
		tools::BrushSettings *bs = m_dockToolSettings->brushSettings();
		dlg->setStabilizerFinishStrokes(bs->isStabilizerFinishStrokes());
		connect(
			dlg, &dialogs::InputSettingsDialog::stabilizerFinishStrokesChanged,
			bs, &tools::BrushSettings::setStabilizerFinishStrokes);
		connect(
			bs, &tools::BrushSettings::stabilizerFinishStrokesChanged, dlg,
			&dialogs::InputSettingsDialog::setStabilizerFinishStrokes);
		utils::showWindow(dlg, shouldShowDialogMaximized());
	}
	return dlg;
}

void MainWindow::saveSplitterState()
{
	if(!updatesEnabled()) {
		m_saveSplitterDebounce.start();
		return;
	}
	m_saveSplitterDebounce.stop();
	if(!m_smallScreenMode && !m_chatbox->isCollapsed() &&
	   !m_chatbox->isDetached()) {
		dpAppConfig()->setLastWindowViewState(m_splitter->saveState());
	}
}

void MainWindow::saveWindowState()
{
	if(!updatesEnabled()) {
		m_saveWindowDebounce.start();
		return;
	}
	m_saveWindowDebounce.stop();

	config::Config *cfg = dpAppConfig();
	cfg->setLastWindowPosition(normalGeometry().topLeft());
	cfg->setLastWindowSize(normalGeometry().size());
	if(!m_smallScreenMode) {
		cfg->setLastWindowMaximized(
			isMaximized() || windowState().testFlag(Qt::WindowFullScreen));
		cfg->setLastWindowState(
			m_hiddenDockState.isEmpty() ? saveState() : m_hiddenDockState);

		// TODO: This should be separate from window state and happen only when
		// dock states change
		QVariantMap docksConfig;
		for(const QDockWidget *dw : findChildren<const QDockWidget *>(
				QString(), Qt::FindDirectChildrenOnly)) {
			if(!dw->objectName().isEmpty()) {
				docksConfig[dw->objectName()] = QVariantMap{
					{QStringLiteral("undockable"),
					 dw->isFloating() &&
						 dw->allowedAreas() == Qt::NoDockWidgetArea}};
			}
		}
		cfg->setLastWindowDocks(docksConfig);
	}

	m_dockToolSettings->saveSettings();
}
// clang-format off

void MainWindow::requestUserInfo(int userId)
{
	net::Client *client = m_doc->client();
	QJsonObject info{{"type", "request_user_info"}};
	client->sendMessage(net::makeUserInfoMessage(
		client->myId(), userId, QJsonDocument{info}));
}

void MainWindow::sendUserInfo(int userId)
{
	// Android reports "linux" as the kernel type, which is not helpful.
#if defined(Q_OS_ANDROID)
	QString os = QSysInfo::productType();
#else
	QString os = QSysInfo::kernelType();
#endif
	QJsonObject info{
		{"type", "user_info"},
		{"app_version", QStringLiteral("%1 (%2)").arg(
			cmake_config::version(), QSysInfo::buildCpuArchitecture())},
		{"protocol_version", DP_PROTOCOL_VERSION},
		{"qt_version", QString::number(QT_VERSION_MAJOR)},
		{"os", os},
		{"tablet_input", tabletinput::current()},
		{"tablet_mode", m_canvasView->isTabletEnabled() ? "pressure" : "none"},
		{"touch_mode", m_canvasView->isTouchDrawEnabled() ? "draw"
			: m_canvasView->isTouchScrollEnabled() ? "scroll" : "none"},
		{"smoothing", m_doc->toolCtrl()->globalSmoothing()},
		{"pressure_curve", m_canvasView->pressureCurveAsString()},
	};
	net::Client *client = m_doc->client();
	client->sendMessage(net::makeUserInfoMessage(
		client->myId(), userId, QJsonDocument{info}));
}

// clang-format on
void MainWindow::requestCurrentBrush(int userId)
{
	m_brushRequestUserId = userId;
	m_brushRequestCorrelator = QUuid::createUuid().toString();
	m_brushRequestTime.start();
	QJsonObject info{
		{QStringLiteral("type"), QStringLiteral("request_current_brush")},
		{QStringLiteral("correlator"), m_brushRequestCorrelator},
	};
	net::Client *client = m_doc->client();
	client->sendMessage(
		net::makeUserInfoMessage(client->myId(), userId, QJsonDocument{info}));
}

void MainWindow::sendCurrentBrush(int userId, const QString &correlator)
{
	const brushes::ActiveBrush &brush =
		m_dockToolSettings->brushSettings()->currentBrush();
	QJsonObject info = {
		{QStringLiteral("type"), QStringLiteral("current_brush")},
		{QStringLiteral("correlator"), correlator},
	};
	if(brush.isConfidential()) {
		info.insert(QStringLiteral("confidential"), true);
	} else {
		info.insert(QStringLiteral("brush"), brush.toShareJson());
	}
	net::Client *client = m_doc->client();
	client->sendMessage(
		net::makeUserInfoMessage(client->myId(), userId, QJsonDocument{info}));
}

void MainWindow::receiveCurrentBrush(int userId, const QJsonObject &info)
{
	bool wasRequested = m_brushRequestUserId == userId &&
						m_brushRequestCorrelator ==
							info[QStringLiteral("correlator")].toString() &&
						m_brushRequestTime.isValid() &&
						!m_brushRequestTime.hasExpired(30000);
	if(wasRequested) {
		m_brushRequestUserId = -1;
		m_brushRequestCorrelator.clear();
		m_brushRequestTime.invalidate();
		QJsonValue v = info[QStringLiteral("brush")];
		if(v.isObject()) {
			tools::BrushSettings *bs = m_dockToolSettings->brushSettings();
			bs->setCurrentBrushDetached(
				brushes::ActiveBrush::fromJson(v.toObject()));
		} else if(info.value(QStringLiteral("confidential")).toBool()) {
			m_chatbox->receiveSystemMessage(
				tr("The requested brush does not allow others to use it."));
		}
	}
}

void MainWindow::fillArea(const QColor &color, int blendMode, float opacity)
{
	switch(blendMode) {
	case DP_BLEND_MODE_ERASE:
	case DP_BLEND_MODE_LIGHT_TO_ALPHA:
	case DP_BLEND_MODE_DARK_TO_ALPHA:
		break;
	default:
		m_dockToolSettings->addLastUsedColor(color);
	}
	m_doc->fillArea(color, DP_BlendMode(blendMode), opacity);
}

void MainWindow::fillAreaWithBlendMode(int blendMode)
{
	fillArea(m_dockToolSettings->foregroundColor(), blendMode, 1.0f);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
	QScopedValueRollback<bool> rollback(m_updatingDockState, true);
	QMainWindow::resizeEvent(event);
	if(!m_resizeReactionPending && !m_restoringDockState) {
		if(dpApp().isAndroidScalingDialogShown()) {
			reactToResize();
		} else {
			m_resizeReactionPending = true;
			emit resizeReactionRequested();
		}
	}
}

void MainWindow::reactToResize()
{
	m_resizeReactionPending = false;
	if(m_restoringDockState) {
		dpApp().takeAndroidScalingJustChanged();
	} else {
		QScopedValueRollback<bool> rollback(m_updatingDockState, true);
		updateInterfaceMode();
		restoreIntendedDockState();
		m_restoreIntendedDockStateDebounce.start();
	}
}

#if defined(Q_OS_ANDROID) && defined(KRITA_QT_SCREEN_DENSITY_ADJUSTMENT)
void MainWindow::handleAndroidScalingDialogShown()
{
	dialogs::SettingsDialog *settingsDlg =
		getStartDialogOrThis()->findChild<dialogs::SettingsDialog *>(
			QString(), Qt::FindDirectChildrenOnly);
	if(settingsDlg) {
		m_androidScalingSettingsDialog = true;
		settingsDlg->close();
	} else {
		m_androidScalingSettingsDialog = false;
	}

	dialogs::StartDialog *startDlg = findChild<dialogs::StartDialog *>(
		QStringLiteral("startdialog"), Qt::FindDirectChildrenOnly);
	if(startDlg) {
		m_androidScalingStartDialog = true;
		startDlg->close();
	} else {
		m_androidScalingStartDialog = false;
	}

	if(!m_hiddenDockState.isEmpty()) {
		setDocksHidden(false);
	}
	updateIntendedDockStateWith(true);
}

void MainWindow::handleAndroidScalingDialogDismissed()
{
	if(m_smallScreenMode) {
		HudAction action;
		action.type = HudAction::Type::None;
		handleToggleAction(action);
	} else {
		m_restoreIntendedDockStateDebounce.start();
	}

	if(m_androidScalingStartDialog) {
		m_androidScalingStartDialog = false;
		showStartDialogOnPage(int(dialogs::StartDialog::Guess));
	}

	if(m_androidScalingSettingsDialog) {
		m_androidScalingSettingsDialog = false;
		dialogs::SettingsDialog *settingsDlg = showSettings();
		settingsDlg->activateUserInterfacePanel();
	}
}
#endif

void MainWindow::showSmallScreenModePreview()
{
	if(m_smallScreenMode && DrawpileApp::isAndroidScalingDialogShown() &&
	   !m_dockLayers->isVisible()) {
		HudAction action;
		action.type = HudAction::Type::ToggleLayer;
		handleToggleAction(action);
	}
}

void MainWindow::setToolBarConfig(const QVariantHash &cfg)
{
	delete m_freehandButton;
	m_freehandButton = nullptr;

	m_toolBarDraw->clear();

	dialogs::ToolBarConfigDialog::readConfig(
		cfg, QStringLiteral("draw"), m_drawingtools->actions(),
		[this](QAction *action, bool hidden) {
			if(!hidden) {
				if(action == m_freehandAction) {
					m_freehandButton = new QToolButton(this);
					m_freehandButton->setCheckable(true);
					m_freehandButton->setChecked(m_freehandAction->isChecked());
					updateFreehandToolButton(
						m_dockToolSettings->brushSettings()->getBrushMode());
					connect(
						m_freehandAction, &QAction::toggled, m_freehandButton,
						&QAbstractButton::setChecked);
					connect(
						m_freehandButton, &QAbstractButton::clicked, this,
						&MainWindow::handleFreehandToolButtonClicked);
					connect(
						m_dockToolSettings->brushSettings(),
						&tools::BrushSettings::brushModeChanged, this,
						&MainWindow::updateFreehandToolButton);
					m_toolBarDraw->addWidget(m_freehandButton);
				} else {
					m_toolBarDraw->addAction(action);
				}
			}
		});
}

void MainWindow::showToolBarConfigDialog()
{
	QString name = QStringLiteral("toolbarconfigdialog");
	dialogs::ToolBarConfigDialog *dlg =
		findChild<dialogs::ToolBarConfigDialog *>(
			name, Qt::FindDirectChildrenOnly);
	if(dlg) {
		dlg->activateWindow();
		dlg->raise();
	} else {
		config::Config *cfg = dpAppConfig();
		dlg = new dialogs::ToolBarConfigDialog(
			cfg->getToolBarConfig(), m_drawingtools->actions(), this);
		dlg->setAttribute(Qt::WA_DeleteOnClose);
		dlg->setObjectName(name);
		connect(dlg, &dialogs::ToolBarConfigDialog::accepted, this, [cfg, dlg] {
			QVariantHash toolBarConfig = cfg->getToolBarConfig();
			dlg->updateConfig(toolBarConfig);
			cfg->setToolBarConfig(toolBarConfig);
		});
		utils::showWindow(dlg);
	}
}
// clang-format off

/**
 * Confirm exit. A confirmation dialog is popped up if there are unsaved
 * changes or network connection is open.
 * @param event event info
 */
void MainWindow::closeEvent(QCloseEvent *event)
{
#ifdef __EMSCRIPTEN__
	event->ignore();
#else
	QApplication::restoreOverrideCursor();
	setEnabled(true);

	if(m_doc->isSaveInProgress()) {
		// Don't quit while save is in progress
		m_exitAction = SAVING;
		event->ignore();
		return;
	}

	if(canReplace() == false) {

		// First confirm disconnection
		if(m_doc->client()->isLoggedIn()) {
			QMessageBox box(
				QMessageBox::Information,
				tr("Exit Drawpile"),
				tr("You are still connected to a drawing session."),
				QMessageBox::NoButton, this);
			utils::disableNativeMessageBox(box);
#ifndef SINGLE_MAIN_WINDOW
			box.setWindowModality(Qt::WindowModal);
#endif

			const QPushButton *exitbtn = box.addButton(tr("Exit anyway"),
					QMessageBox::AcceptRole);
			box.addButton(tr("Cancel"),
					QMessageBox::RejectRole);

			box.exec();
			if(box.clickedButton() == exitbtn) {
				// Disconnect and wait a moment for things to settle so that
				// e.g. the builtin server gets a chance to shut down properly
				// and any pending drawing commands get executed.
				m_exitAction = DISCONNECTING;
				m_doc->client()->disconnectFromServer();
				setEnabled(false);
				QApplication::setOverrideCursor(Qt::WaitCursor);
			}
			event->ignore();
			return;
		}

		// Then confirm unsaved changes
		if(isWindowModified()) {
			QMessageBox box(QMessageBox::Question, tr("Exit Drawpile"),
					tr("There are unsaved changes. Save them before exiting?"),
					QMessageBox::NoButton, this);
			utils::disableNativeMessageBox(box);
			box.setInformativeText(makeContributionInfoText());
#ifndef SINGLE_MAIN_WINDOW
			box.setWindowModality(Qt::WindowModal);
#endif
			const QPushButton *savebtn = box.addButton(tr("Save"),
					QMessageBox::AcceptRole);
			box.addButton(tr("Discard"),
					QMessageBox::DestructiveRole);
			const QPushButton *cancelbtn = box.addButton(tr("Cancel"),
					QMessageBox::RejectRole);

			box.exec();
			bool cancel = false;
			// Save and exit, or cancel exit if couldn't save.
			if(box.clickedButton() == savebtn) {
				cancel = true;
				if(save()) {
					m_exitAction = SAVING;
				}
			}

			// Cancel exit
			if(box.clickedButton() == cancelbtn || cancel) {
				event->ignore();
				return;
			}
		}
	}

	exit();
#endif
}

// clang-format on
bool MainWindow::event(QEvent *event)
{
	switch(event->type()) {
	case QEvent::StatusTip:
		m_viewStatusBar->showMessage(
			static_cast<QStatusTipEvent *>(event)->tip());
		return true;
	case QEvent::KeyRelease: {
		// Monitor key-up events to switch back from temporary tools/tool slots.
		// A short tap of the tool switch shortcut switches the tool permanently
		// as usual, but when holding it down, the tool is activated just
		// temporarily. The previous tool be switched back automatically when
		// the shortcut key is released. Note: for simplicity, we only support
		// tools with single key shortcuts.
		const QKeyEvent *e = static_cast<const QKeyEvent *>(event);
		if(m_temporaryToolSwitchMs >= 0 && !e->isAutoRepeat()) {
			if(m_toolChangeTime.elapsed() > m_temporaryToolSwitchMs) {
				if(m_tempToolSwitchShortcut->isShortcutSent() &&
				   e->modifiers() == Qt::NoModifier) {
					// Return from temporary tool change
					for(const QAction *act : m_drawingtools->actions()) {
						const QKeySequence &seq = act->shortcut();
						if(seq.count() == 1 &&
						   compat::keyPressed(*e) == seq[0]) {
							m_dockToolSettings->setPreviousTool();
							break;
						}
					}

					// Return from temporary tool slot change
					for(const QAction *act : m_brushSlots->actions()) {
						const QKeySequence &seq = act->shortcut();
						if(seq.count() == 1 &&
						   compat::keyPressed(*e) == seq[0]) {
							m_dockToolSettings->setPreviousTool();
							break;
						}
					}
				}
			}
			m_tempToolSwitchShortcut->reset();
		}
		break;
	}
	case QEvent::ShortcutOverride: {
		// QLineEdit doesn't seem to override the Return key shortcut,
		// so we have to do it ourself.
		const QKeyEvent *e = static_cast<QKeyEvent *>(event);
		if(e->key() == Qt::Key_Return) {
			QWidget *focus = QApplication::focusWidget();
			if(focus && focus->inherits("QLineEdit")) {
				event->accept();
				return true;
			}
		}
		break;
	}
	case QEvent::Move:
	case QEvent::Resize:
	case QEvent::WindowStateChange:
		m_saveWindowDebounce.start();
		break;
	case QEvent::ActivationChange:
		if(m_saveSplitterDebounce.isActive()) {
			saveSplitterState();
		}
		if(m_saveWindowDebounce.isActive()) {
			saveWindowState();
		}
		m_canvasView->clearKeys();
		dpAppConfig()->trySubmit();
		DRAWPILE_FS_PERSIST();
		break;
	default:
		break;
	}

	return QMainWindow::event(event);
}

dialogs::StartDialog *MainWindow::showStartDialogOnPage(int page)
{
	dialogs::StartDialog *dlg =
		new dialogs::StartDialog(m_smallScreenMode, this);
	dlg->setObjectName(QStringLiteral("startdialog"));
	dlg->setAttribute(Qt::WA_DeleteOnClose);
	connectStartDialog(dlg);
	dlg->showPage(dialogs::StartDialog::Entry(page));
	utils::showWindow(dlg, shouldShowDialogMaximized());
	return dlg;
}

void MainWindow::showPopupMessage(const QString &message)
{
	m_netstatus->showMessage(message);
}

void MainWindow::showPermissionDeniedMessage(int feature)
{
	QString message;
	switch(feature) {
	case DP_FEATURE_PUT_IMAGE:
		message =
			//: "Delete" refers to Edit > Delete, which erases the contents of a
			//: selection and the default shortcut is the delete key.
			tr("You don't have permission to cut, paste, fill or delete.");
		break;
	case DP_FEATURE_RESIZE:
		message = tr("You don't have permission to resize the canvas.");
		break;
	case DP_FEATURE_BACKGROUND:
		message =
			tr("You don't have permission to change the session background.");
		break;
	case DP_FEATURE_CREATE_ANNOTATION:
		message = tr("You don't have permission to create annotations.");
		break;
	case DP_FEATURE_UNDO:
		message = tr("You don't have permission to undo or redo.");
		break;
	default:
		qWarning("Unhandled denied permission %d", feature);
		message = tr("You don't have permission to do that.");
		break;
	}
	m_canvasView->showPopupNotice(message);
}

void MainWindow::loadCanvasStateFromFile(
	const QString &path, QTemporaryFile *tempFile, bool resume)
{
	QApplication::setOverrideCursor(Qt::WaitCursor);
	QProgressDialog *progressDialog = new QProgressDialog(this);
	progressDialog->setRange(0, 0);
	progressDialog->setCancelButton(nullptr);
	if(resume) {
		progressDialog->setMinimumDuration(0);
		progressDialog->setLabelText(tr("Resuming…"));
	} else {
		progressDialog->setLabelText(tr("Opening file…"));
		progressDialog->setMinimumDuration(500);
	}

	setEnabled(false);

	CanvasLoaderRunnable *loader = new CanvasLoaderRunnable(path, this);
	loader->setAutoDelete(false);
	connect(
		loader, &CanvasLoaderRunnable::loadComplete, this,
		[this, tempFile, resume, loader, progressDialog](
			const QString &error, const QString &detail, qint64 elapsedMsec) {
			delete tempFile;
			setEnabled(true);
			delete progressDialog;
			QApplication::restoreOverrideCursor();

			const drawdance::CanvasState &canvasState = loader->canvasState();
			if(canvasState.isNull()) {
				showErrorMessageWithDetails(error, detail);
			} else {
				showElapsedStatusMessage(
					//: %1 is minutes, %2 is seconds, %3 is milliseconds.
					tr("Canvas loaded in %1:%2.%3"), elapsedMsec);
				clearPromptHistory();
				bool autoRecord = dpAppConfig()->getAutoRecordHost();
				if(resume) {
					long long resumeSessionId = loader->resumeSessionId();
					m_doc->resumeState(
						canvasState, loader->path(), autoRecord,
						resumeSessionId);
				} else {
					m_doc->loadState(
						canvasState, loader->path(), loader->type(), false,
						autoRecord, loader->sessionSourceParam(),
						loader->sessionSequenceId());
				}
			}

			loader->deleteLater();
		},
		Qt::QueuedConnection);

	QThreadPool::globalInstance()->start(loader);
}

// clang-format off

void MainWindow::connectStartDialog(dialogs::StartDialog *dlg)
{
	QString key = QStringLiteral("startdialogconnections");
	utils::Connections *previousConnections =
		dlg->findChild<utils::Connections *>(key);
	if(previousConnections) {
		previousConnections->clear();
		delete previousConnections;
	}

	utils::Connections *connections = new utils::Connections(key, dlg);
	connections->add(connect(dlg, &dialogs::StartDialog::openFile, this, &MainWindow::open));
	connections->add(connect(dlg, &dialogs::StartDialog::openRecent, this, std::bind(&MainWindow::openRecent, this, _1, nullptr)));
	connections->add(connect(dlg, &dialogs::StartDialog::openRecovery, this, &MainWindow::openRecovery));
	connections->add(connect(dlg, &dialogs::StartDialog::layouts, this, &MainWindow::showLayoutsDialog));
	connections->add(connect(dlg, &dialogs::StartDialog::preferences, this, &MainWindow::showSettings));
	connections->add(connect(dlg, &dialogs::StartDialog::networkPreferences, this, &MainWindow::showNetworkSettings));
	connections->add(connect(dlg, &dialogs::StartDialog::serverPreferences, this, &MainWindow::showServerSettings));
	connections->add(connect(dlg, &dialogs::StartDialog::join, this, &MainWindow::joinSession));
	connections->add(connect(dlg, &dialogs::StartDialog::host, this, &MainWindow::hostSession));
	connections->add(connect(dlg, &dialogs::StartDialog::create, this, &MainWindow::newDocument));
	connections->add(connect(m_doc, &Document::canvasChanged, dlg, std::bind(&MainWindow::closeStartDialog, this, dlg, true)));
	connections->add(connect(m_doc, &Document::serverLoggedIn, dlg, std::bind(&MainWindow::closeStartDialog, this, dlg, _1)));
	connections->add(connect(this, &MainWindow::hostSessionEnabled, dlg, &dialogs::StartDialog::hostPageEnabled));
	connections->add(connect(this, &MainWindow::smallScreenModeChanged, dlg, &dialogs::StartDialog::setSmallScreenMode));
	connections->add(connect(this, &MainWindow::windowReplacementFailed, dlg, [dlg](MainWindow *win){
		if(win) {
			dlg->setParent(win, dlg->windowFlags());
			win->connectStartDialog(dlg);
			utils::showWindow(dlg, win->shouldShowDialogMaximized());
			dlg->activateWindow();
			dlg->raise();
		} else {
			dlg->deleteLater();
		}
	}));
	setStartDialogActions(dlg);
}

void MainWindow::setStartDialogActions(dialogs::StartDialog *dlg)
{
	QPair<dialogs::StartDialog::Entry, const char *> pairs[] = {
		{dialogs::StartDialog::Entry::Join, "joinsession"},
		{dialogs::StartDialog::Entry::Browse, "browsesession"},
		{dialogs::StartDialog::Entry::Host, "hostsession"},
		{dialogs::StartDialog::Entry::Create, "newdocument"},
		{dialogs::StartDialog::Entry::Open, "opendocument"},
		{dialogs::StartDialog::Entry::Layouts, "layouts"},
		{dialogs::StartDialog::Entry::Preferences, "preferences"},
	};
	dialogs::StartDialog::Actions actions{};
	for(const auto &[entry, action] : pairs) {
		actions.entries[entry] = getAction(action);
	}
	dlg->setActions(actions);
}

// clang-format on
void MainWindow::closeStartDialog(dialogs::StartDialog *dlg, bool reparent)
{
	if(!dlg->isClosing()) {
		dlg->setClosing(true);
		// Linux on Qt6 crashes when reparenting children of a dialog that's
		// about to close after hosting, so we don't reparent in that case and
		// just close the dialog. On the other hand, macOS on Qt6 crashes when
		// closing the dialog instead, so we always reparent there.
#ifdef Q_OS_MACOS
		reparent = true;
#endif
		if(reparent) {
			for(QDialog *child : dlg->findChildren<QDialog *>(
					QString(), Qt::FindDirectChildrenOnly)) {
				child->setParent(this, child->windowFlags());
				child->show();
			}
		}
		// Delay closing for a moment to work around Qt bugs. Sometimes it ends
		// up losing window handles or chasing null pointers here otherwise.
		QTimer::singleShot(100, dlg, [dlg] {
			dlg->close();
		});
	}
}

QWidget *MainWindow::getStartDialogOrThis()
{
	dialogs::StartDialog *dlg = findChild<dialogs::StartDialog *>(
		QStringLiteral("startdialog"), Qt::FindDirectChildrenOnly);
	if(dlg) {
		return dlg;
	} else {
		return this;
	}
}

void MainWindow::start()
{
	showStartDialogOnPage(int(dialogs::StartDialog::Entry::Guess));
}

/**
 * Show the "new document" dialog
 */
void MainWindow::showNew()
{
	showStartDialogOnPage(int(dialogs::StartDialog::Entry::Create));
}

void MainWindow::newDocument(const QSize &size, const QColor &background)
{
	questionWindowReplacement(
		tr("New"),
		tr("You're about to create a new canvas and close this window."),
		[this, size, background](bool ok) {
			if(ok) {
				if(canReplace()) {
					loadBlankDocument(size, background);
				} else {
					prepareWindowReplacement();
					bool newProcessStarted = dpApp().runInNewProcess(
						{QStringLiteral("--no-restore-window-position"),
						 QStringLiteral("--blank"),
						 QStringLiteral("%1x%2x%3")
							 .arg(size.width())
							 .arg(size.height())
							 .arg(background.name(QColor::HexArgb)
									  .remove('#'))});
					if(newProcessStarted) {
						emit windowReplacementFailed(nullptr);
					} else {
						createNewWindow([size, background](MainWindow *win) {
							win->loadBlankDocument(size, background);
						});
					}
				}
			}
		});
}

void MainWindow::openRecent(const QString &path, QTemporaryFile *tempFile)
{
	questionWindowReplacement(
		tr("Open Recent File"),
		tr("You're about to open a recent file and close this window."),
		[this, path, tempFile](bool ok) {
			if(ok) {
				openPath(path, tempFile);
			} else {
				delete tempFile;
			}
		});
}

void MainWindow::openRecovery(const QString &path)
{
	questionWindowReplacement(
		tr("Open Recovered File"),
		tr("You're about to open a recovered file and close this window."),
		[this, path](bool ok) {
			if(ok) {
				openPath(path);
			}
		});
}

void MainWindow::openPath(const QString &path, QTemporaryFile *tempFile)
{
	if(!canReplace()) {
		prepareWindowReplacement();
		bool newProcessStarted = dpApp().runInNewProcess(
			{QStringLiteral("--no-restore-window-position"),
			 QStringLiteral("--open"), path});
		if(newProcessStarted) {
			emit windowReplacementFailed(nullptr);
			// The temporary file is only used in the browser, which will never
			// start new processes, so it really should always be null here.
			Q_ASSERT(!tempFile);
			delete tempFile;
		} else {
			createNewWindow([path, tempFile](MainWindow *win) {
				win->openPath(path, tempFile);
			});
		}
		return;
	}

	QString loadPath = tempFile ? tempFile->fileName() : path;

	constexpr QRegularExpression::PatternOption opt =
		QRegularExpression::CaseInsensitiveOption;
	if(QRegularExpression(QStringLiteral("\\Afile://"), opt)
		   .match(loadPath)
		   .hasMatch()) {
		QUrl url = QUrl::fromUserInput(loadPath);
		if(url.isValid() && url.isLocalFile()) {
			loadPath = url.toLocalFile();
		}
	}

	if(QRegularExpression{"\\.dp(rec|txt)$", opt}.match(path).hasMatch()) {
		bool isTemplate;
		DP_LoadResult result =
			m_doc->loadRecording(loadPath, false, &isTemplate);
		showLoadResultMessage(result);
		if(result == DP_LOAD_RESULT_SUCCESS && !isTemplate) {
			QFileInfo fileinfo(path);
			m_playbackDialog =
				new dialogs::PlaybackDialog(m_doc->canvas(), this);
			m_playbackDialog->setWindowTitle(
				fileinfo.completeBaseName() + " - " +
				m_playbackDialog->windowTitle());
			m_playbackDialog->setAttribute(Qt::WA_DeleteOnClose);
			m_playbackDialog->show();
			m_playbackDialog->centerOnParent();
			if(tempFile) {
				tempFile->setParent(m_playbackDialog);
			}
			connect(
				m_playbackDialog, &dialogs::PlaybackDialog::playbackToggled,
				this, &MainWindow::setRecorderStatus);
			connect(
				m_playbackDialog, &dialogs::PlaybackDialog::destroyed, this,
				[this, path](QObject *) {
					m_playbackDialog = nullptr;
					setRecorderStatus(false);
					canvas::CanvasModel *canvas = m_doc->canvas();
					if(canvas && dpAppConfig()->getAutoRecordHost()) {
						canvas->startProjectRecording(
							dpAppConfig(), DP_PROJECT_SOURCE_FILE);
					}
				});
		} else {
			delete tempFile;
		}

	} else if(
		QRegularExpression{"\\.drawdancedump$", opt}.match(path).hasMatch()) {
		DP_LoadResult result = m_doc->loadRecording(loadPath, true);
		if(result == DP_LOAD_RESULT_SUCCESS) {
			QFileInfo fileinfo{path};
			m_dumpPlaybackDialog =
				new dialogs::DumpPlaybackDialog{m_doc->canvas(), this};
			m_dumpPlaybackDialog->setWindowTitle(
				QStringLiteral("%1 - %2")
					.arg(fileinfo.completeBaseName())
					.arg(m_dumpPlaybackDialog->windowTitle()));
			m_dumpPlaybackDialog->setAttribute(Qt::WA_DeleteOnClose);
			m_dumpPlaybackDialog->show();
			if(tempFile) {
				tempFile->setParent(m_dumpPlaybackDialog);
			}
		} else {
			delete tempFile;
		}

	} else {
		loadCanvasStateFromFile(loadPath, tempFile, false);
	}

	addRecentFile(path, int(utils::Recents::Source::Open));
}

void MainWindow::resumeAutosave(const QString &path)
{
	loadCanvasStateFromFile(path, nullptr, true);
}

/**
 * Show a file selector dialog. If there are unsaved changes, open the file
 * in a new window.
 */
void MainWindow::open()
{
	questionOpenFileWindowReplacement([this](bool ok) {
		if(ok) {
			FileWrangler(getStartDialogOrThis())
				.openMain(std::bind(&MainWindow::openPath, this, _1, _2));
		}
	});
}

void MainWindow::showRecover()
{
	showStartDialogOnPage(int(dialogs::StartDialog::Entry::Recover));
}

#ifdef __EMSCRIPTEN__
void MainWindow::download()
{
	FileWrangler(this).downloadImage(m_doc);
}

void MainWindow::downloadSelection()
{
	FileWrangler(this).downloadSelection(m_doc);
}
#else
bool MainWindow::save()
{
	QString result = FileWrangler{this}.saveImage(m_doc, false);
	if(result.isEmpty()) {
		if(m_reconnectAfterSave) {
			reconnect();
		}
		return false;
	} else {
		addRecentFile(result, int(utils::Recents::Source::Save));
		return true;
	}
}

void MainWindow::saveAs()
{
	saveAsType(int(DP_SAVE_IMAGE_UNKNOWN), false);
}

void MainWindow::saveAsDpcs()
{
	saveAsType(int(DP_SAVE_IMAGE_PROJECT_CANVAS), true);
}

void MainWindow::saveAsOra()
{
	saveAsType(int(DP_SAVE_IMAGE_ORA), true);
}

void MainWindow::saveSelection()
{
	QString result = FileWrangler{this}.saveSelectionAs(m_doc);
	if(!result.isEmpty()) {
		addRecentFile(result, int(utils::Recents::Source::ExportSelection));
	}
}

void MainWindow::exportImage()
{
	QString result = FileWrangler(this).saveImageAs(
		m_doc, true, DP_SAVE_IMAGE_UNKNOWN, false);
	if(!result.isEmpty()) {
		addRecentFile(result, int(utils::Recents::Source::Export));
	}
}

#	ifndef Q_OS_ANDROID
void MainWindow::exportImageAgain()
{
	QString result = FileWrangler(this).saveImage(m_doc, true);
	if(!result.isEmpty()) {
		addRecentFile(result, int(utils::Recents::Source::Export));
	}
}
#	endif
#endif

void MainWindow::importAnimationFrames()
{
	importAnimation(int(dialogs::AnimationImportDialog::Source::Frames));
}

void MainWindow::importAnimationLayers()
{
	importAnimation(int(dialogs::AnimationImportDialog::Source::Layers));
}

void MainWindow::importAnimation(int source)
{
	// If we're on a single-window system, don't clobber that window just to
	// show the dialog. Otherwise a single mis-click could obliterate work.
#ifdef SINGLE_MAIN_WINDOW
	bool showDialogNow = true;
#else
	bool showDialogNow = canReplace();
#endif
	if(showDialogNow) {
		dialogs::AnimationImportDialog *dlg =
			new dialogs::AnimationImportDialog(source, this);
		dlg->setAttribute(Qt::WA_DeleteOnClose);
		connect(
			dlg, &dialogs::AnimationImportDialog::canvasStateImported, this,
			[this, dlg](const drawdance::CanvasState &canvasState) {
				auto block = [canvasState](MainWindow *win) {
					// Don't use the path of the imported animation to avoid
					// clobbering the old file by mashing Ctrl+S instinctually.
					bool autoRecord = dpAppConfig()->getAutoRecordHost();
					win->m_doc->loadState(
						canvasState, QString(), DP_SAVE_IMAGE_UNKNOWN, true,
						autoRecord, QString(), 0LL);
				};
				if(canReplace()) {
					block(this);
				} else {
					prepareWindowReplacement();
					createNewWindow(block);
				}
				dlg->deleteLater();
			});
		utils::showWindow(dlg, shouldShowDialogMaximized());
	} else {
		QString startPageArgument =
			dialogs::AnimationImportDialog::getStartPageArgumentForSource(
				source);
		if(!startPageArgument.isEmpty()) {
			prepareWindowReplacement();
			bool newProcessStarted = dpApp().runInNewProcess(
				{QStringLiteral("--no-restore-window-position"),
				 QStringLiteral("--start-page"), startPageArgument});
			if(newProcessStarted) {
				emit windowReplacementFailed(nullptr);
			} else {
				createNewWindow([](MainWindow *win) {
					win->importAnimationLayers();
				});
			}
		}
	}
}

void MainWindow::onCanvasSaveStarted()
{
	QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
#ifdef __EMSCRIPTEN__
	getAction("downloaddocument")->setEnabled(false);
	getAction("downloadselection")->setEnabled(false);
#else
	getAction("savedocument")->setEnabled(false);
	getAction("savedocumentas")->setEnabled(false);
	getAction("exportdocument")->setEnabled(false);
	getAction("savedocumentasdpcs")->setEnabled(false);
	getAction("savedocumentasora")->setEnabled(false);
#	ifndef Q_OS_ANDROID
	getAction("exportdocumentagain")->setEnabled(false);
#	endif
#endif
	m_viewStatusBar->showMessage(tr("Saving..."));
	m_canvasView->setSaveInProgress(true);
	updateProjectActions();
}

void MainWindow::onCanvasSaved(const QString &errorMessage, qint64 elapsedMsec)
{
	QApplication::restoreOverrideCursor();
#ifdef __EMSCRIPTEN__
	getAction("downloaddocument")->setEnabled(true);
	getAction("downloadselection")->setEnabled(true);
#else
	getAction("savedocument")->setEnabled(true);
	getAction("savedocumentas")->setEnabled(true);
	getAction("exportdocument")->setEnabled(true);
	getAction("savedocumentasdpcs")->setEnabled(true);
	getAction("savedocumentasora")->setEnabled(true);
#	ifndef Q_OS_ANDROID
	getAction("exportdocumentagain")->setEnabled(m_doc->haveExportPath());
#	endif
#endif
	m_canvasView->setSaveInProgress(false);
	updateProjectActions();

	setWindowModified(m_doc->isDirty());
	updateTitle();

	bool haveError = !errorMessage.isEmpty();
	if(haveError) {
		m_viewStatusBar->showMessage(tr("Image saving failed"), 1000);
		showErrorMessageWithDetails(tr("Couldn't save image"), errorMessage);
		m_reconnectAfterSave = false;
	} else if(elapsedMsec <= 0LL) {
		m_viewStatusBar->showMessage(tr("Image saved"), 1000);
	} else {
		showElapsedStatusMessage(
			//: %1 is minutes, %2 is seconds, %3 is milliseconds.
			tr("Image saved in %1:%2.%3"), elapsedMsec);
	}

#ifndef __EMSCRIPTEN__
	// Cancel exit if canvas is modified while it was being saved
	if(m_doc->isDirty() || m_reconnectAfterSave) {
		m_exitAction = RUNNING;
	}

	if(m_exitAction == SAVING) {
		close();
	}

	if(m_reconnectAfterSave) {
		reconnect();
	}
#endif
}

void MainWindow::onAnimationExported(
	const QString &errorMessage, qint64 elapsedMsec)
{
	if(!errorMessage.isEmpty()) {
		m_viewStatusBar->showMessage(tr("Animation export failed"), 1000);
		showErrorMessageWithDetails(
			tr("Couldn't export animation"), errorMessage);
	} else if(elapsedMsec <= 0LL) {
		m_viewStatusBar->showMessage(tr("Animation exported"), 1000);
	} else {
		showElapsedStatusMessage(
			//: %1 is minutes, %2 is seconds, %3 is milliseconds.
			tr("Animation exported in %1:%2.%3"), elapsedMsec);
	}
}

#ifdef __EMSCRIPTEN__
void MainWindow::onCanvasDownloadStarted()
{
	onCanvasSaveStarted();
}

void MainWindow::onCanvasDownloadReady(
	const QString &defaultName, const QByteArray &bytes, qint64 elapsedMsec)
{
	if(bytes.isEmpty()) {
		onCanvasSaved(tr("File is empty."), 0);
	} else {
		onCanvasSaved(QString(), elapsedMsec);
		offerDownload(defaultName, bytes);
	}
}

void MainWindow::onCanvasDownloadError(const QString &errorMessage)
{
	onCanvasSaved(errorMessage, 0);
}

void MainWindow::offerDownload(
	const QString &defaultName, const QByteArray &bytes)
{
	if(bytes.isEmpty()) {
		showErrorMessageWithDetails(
			tr("Error setting up download."), tr("File is empty."));
	} else {
		QMessageBox *msgbox = utils::makeInformationWithSaveButton(
			this, tr("Download Complete"),
			tr("Download complete, click on \"Save\" to save your file."),
			makeContributionInfoText());
		connect(
			msgbox->button(QMessageBox::Save), &QAbstractButton::clicked, this,
			[this, defaultName, bytes]() {
				if(bytes.isEmpty()) {
					showErrorMessageWithDetails(
						tr("Error performing download."), tr("File is empty."));
				} else {
					FileWrangler(this).saveFileContent(defaultName, bytes);
					if(m_reconnectAfterSave) {
						reconnectWith(true);
					}
				}
			});
		utils::showMessageBox(msgbox);
	}
}
#endif

// clang-format off

void MainWindow::showResetNoticeDialog(const drawdance::CanvasState &canvasState)
{
	m_canvasView->setCatchupProgress(0, true);
	m_canvasView->showResetNotice(m_doc->isSaveInProgress());
	if(m_preResetCanvasState.isNull()) {
		m_preResetCanvasState = canvasState;
	}
}

void MainWindow::updateCatchupProgress(int percent)
{
	if(percent >= 100 && m_initialCatchup) {
		m_initialCatchup = false;
		dpApp().notifications()->trigger(
			this, notification::Event::Login, tr("Joined the session!"));
		Q_EMIT initialCatchupFinished();
	}
	m_canvasView->setCatchupProgress(percent, false);
}

void MainWindow::updateStreamResetProgress(int percent)
{
	m_canvasView->setStreamResetProgress(percent);
}

void MainWindow::savePreResetImageAs()
{
	if(m_preResetCanvasState.isNull()) {
		m_canvasView->hideResetNotice();
	} else {
#ifdef __EMSCRIPTEN__
		if(FileWrangler(this).downloadPreResetImage(m_doc, m_preResetCanvasState)) {
			m_canvasView->hideResetNotice();
		}
#else
		QString result = FileWrangler(this).savePreResetImageAs(
			m_doc, m_preResetCanvasState);
		if(!result.isEmpty()) {
			m_preResetCanvasState = drawdance::CanvasState::null();
			m_canvasView->hideResetNotice();
			addRecentFile(result, int(utils::Recents::Source::SavePreResetImageAs));
		}
#endif
	}
}

// clang-format on
void MainWindow::discardPreResetImage()
{
	m_preResetCanvasState = drawdance::CanvasState::null();
	m_canvasView->hideResetNotice();
}

void MainWindow::showProjectRecordingSettings()
{
	QString objectName = QStringLiteral("projectrecordingsettingsdialog");
	dialogs::ProjectRecordingSettingsDialog *dlg =
		findChild<dialogs::ProjectRecordingSettingsDialog *>(
			objectName, Qt::FindDirectChildrenOnly);
	if(dlg) {
		dlg->activateWindow();
		dlg->raise();
	} else {
		canvas::CanvasModel *canvas = m_doc->canvas();
		if(canvas) {
			bool settingsOpen =
				getStartDialogOrThis()->findChild<dialogs::SettingsDialog *>(
					QStringLiteral("settingsdialog"),
					Qt::FindDirectChildrenOnly);
			dlg = new dialogs::ProjectRecordingSettingsDialog(
				getAction(QStringLiteral("autorecord")), settingsOpen, this);
			dlg->setAttribute(Qt::WA_DeleteOnClose);
			dlg->setObjectName(objectName);
			connect(
				dlg,
				&dialogs::ProjectRecordingSettingsDialog::
					setSizeLimitInBytesRequested,
				this, &MainWindow::setProjectRecordingSizeLimitInBytes);
			connect(
				dlg,
				&dialogs::ProjectRecordingSettingsDialog::preferencesRequested,
				this, [this, dlg] {
					dlg->close();
					showSettings()->activateFilesPanel();
				});
			connect(
				canvas, &canvas::CanvasModel::projectRecordingSizeChanged, dlg,
				&dialogs::ProjectRecordingSettingsDialog::updateSize);

			if(canvas->isProjectRecording()) {
				dlg->updateSize(
					canvas->projectRecordingLastReportedSizeInBytes(),
					canvas->projectRecordingSizeLimitInBytes());
			}

			utils::showWindow(dlg);
		}
	}
}

void MainWindow::showCompatibilityModeWarning()
{
	bool compatibilityMode = m_doc->isCompatibilityMode();
	if(compatibilityMode || m_doc->isMinorIncompatibility()) {
		QString title, message;
		if(compatibilityMode) {
			title = tr("Compatibility Mode");
			message = tr(
				"This session was hosted with an older version of Drawpile. "
				"Several features – such as layer clipping, some blend modes "
				"and drawing within a selection mask – will be unavailable.");
		} else {
			title = tr("Outdated Version");
			message =
				tr("This session was hosted with a newer version of Drawpile. "
				   "You will not see an effect when people use newer features "
				   "that your version doesn't have yet and you won't be able "
				   "to compress or reset the canvas. Check <a href=\"%1\">"
				   "drawpile.net</a> for updates.")
					.arg(cmake_config::website());
		}
		QMessageBox *box = new QMessageBox(
			QMessageBox::Warning, title, message, QMessageBox::Ok, this);
		box->setAttribute(Qt::WA_DeleteOnClose);
		box->setModal(false);
		utils::showMessageBox(box);
	}
}

void MainWindow::exportTemplate()
{
	QString filename = FileWrangler{this}.getSaveTemplatePath();
	if(!filename.isEmpty()) {
		m_doc->exportTemplate(filename);
	}
}
// clang-format off

void MainWindow::onTemplateExported(const QString &errorMessage)
{
	if(errorMessage.isEmpty()) {
		m_viewStatusBar->showMessage(tr("Session template saved"), 1000);
	} else {
		showErrorMessageWithDetails(tr("Couldn't export session template"), errorMessage);
	}
}

// clang-format on

void MainWindow::showAnimationExportDialog(bool fromFlipbook)
{
	QString objectName = QStringLiteral("animationexportdialog");
	dialogs::AnimationExportDialog *dlg =
		findChild<dialogs::AnimationExportDialog *>(
			objectName, Qt::FindDirectChildrenOnly);
	if(dlg) {
		dlg->activateWindow();
		dlg->raise();
	} else {
		dlg = new dialogs::AnimationExportDialog(
			m_animationExportLoops, m_animationExportScalePercent,
			m_animationExportScaleSmooth, this);
		dlg->setAttribute(Qt::WA_DeleteOnClose);
		dlg->setObjectName(objectName);
		dlg->setCanvas(m_doc->canvas());
		dlg->setFlipbookState(
			m_flipbookState.loopStart, m_flipbookState.loopEnd,
			m_flipbookState.speedPercent, m_flipbookState.crop, fromFlipbook);
		connect(
			dlg, &dialogs::AnimationExportDialog::exportRequested, this,
			&MainWindow::exportAnimation);
		utils::showWindow(dlg, shouldShowDialogMaximized());
	}
}

void MainWindow::updateFlipbookState()
{
	dialogs::AnimationExportDialog *dlg =
		findChild<dialogs::AnimationExportDialog *>(
			QStringLiteral("animationexportdialog"),
			Qt::FindDirectChildrenOnly);
	if(dlg) {
		dlg->setFlipbookState(
			m_flipbookState.loopStart, m_flipbookState.loopEnd,
			m_flipbookState.speedPercent, m_flipbookState.crop, false);
	}
}

void MainWindow::exportAnimation(
#ifndef __EMSCRIPTEN__
	const QString &path,
#endif
	const QString &ffmpegPath, int format, int loops,
	const QVector<int> &frameIndexes, double framerate, const QRect &crop,
	int scalePercent, bool scaleSmooth)
{
	m_animationExportLoops = loops;
	m_animationExportScalePercent = scalePercent;
	m_animationExportScaleSmooth = scaleSmooth;

	QProgressDialog *progressDialog = new QProgressDialog(
		tr("Saving animation..."), tr("Cancel"), 0, 100, this);
	progressDialog->setMinimumDuration(500);
	progressDialog->setValue(0);

	drawdance::CanvasState canvasState =
		m_doc->canvas()->paintEngine()->viewCanvasState();
	QRect canvasRect = QRect(QPoint(0, 0), canvasState.size());
	QRect effectiveCrop = crop & canvasRect;
	if(effectiveCrop.isEmpty()) {
		effectiveCrop = canvasRect;
	}
	QSize size = dialogs::AnimationExportDialog::getScaledSizeFor(
		scalePercent, effectiveCrop);

	AnimationSaverRunnable *saver = new AnimationSaverRunnable(
#ifndef __EMSCRIPTEN__
		path,
#endif
		format, size.width(), size.height(), loops, frameIndexes, framerate,
		effectiveCrop, scaleSmooth, canvasState, ffmpegPath, this);
	saver->setAutoDelete(true);

	connect(
		saver, &AnimationSaverRunnable::progress, progressDialog,
		&QProgressDialog::setValue);
	connect(
		saver, &AnimationSaverRunnable::saveComplete, progressDialog,
		&QProgressDialog::deleteLater);
	connect(
		saver, &AnimationSaverRunnable::saveComplete, this,
		&MainWindow::onAnimationExported);
	connect(
		progressDialog, &QProgressDialog::canceled, saver,
		&AnimationSaverRunnable::cancelExport);
#ifdef __EMSCRIPTEN__
	connect(
		saver, &AnimationSaverRunnable::downloadReady, this,
		&MainWindow::offerDownload);
#endif

	QThreadPool::globalInstance()->start(saver);
}

// clang-format off

void MainWindow::showFlipbook()
{
	dialogs::Flipbook *fp = findChild<dialogs::Flipbook *>(
		"flipbook", Qt::FindDirectChildrenOnly);
	if(fp) {
		fp->setPaintEngine(m_doc->canvas()->paintEngine());
	} else {
		fp = new dialogs::Flipbook{m_flipbookState, this};
		fp->setObjectName("flipbook");
		fp->setAttribute(Qt::WA_DeleteOnClose);
		canvas::CanvasModel *canvas = m_doc->canvas();
		canvas::SelectionModel *sel = canvas->selection();
		connect(
			fp, &dialogs::Flipbook::stateChanged, this,
			&MainWindow::updateFlipbookState);
		fp->setPaintEngine(
			canvas->paintEngine(), sel->isValid() ? sel->bounds() : QRect());
		fp->setRefreshShortcuts(getAction("showflipbook")->shortcuts());
		connect(
			fp, &dialogs::Flipbook::exportRequested, this,
			std::bind(&MainWindow::showAnimationExportDialog, this, true));
		utils::showWindow(fp, shouldShowDialogMaximized());
	}
}

void MainWindow::setRecorderStatus(bool on)
{
#ifdef __EMSCRIPTEN__
	Q_UNUSED(on);
#else
	QAction *recordAction = getAction("recordsession");

	if(m_playbackDialog) {
		if(m_playbackDialog->isPlaying()) {
			recordAction->setIcon(QIcon::fromTheme("media-playback-pause"));
			recordAction->setText(tr("Pause"));
		} else {
			recordAction->setIcon(QIcon::fromTheme("media-playback-start"));
			recordAction->setText(tr("Play"));
		}

	} else {
		if(on) {
			recordAction->setText(tr("Stop Recording"));
			recordAction->setIcon(QIcon::fromTheme("media-playback-stop"));
		} else {
			recordAction->setText(tr("Record..."));
			recordAction->setIcon(QIcon::fromTheme("media-record"));
		}
	}
#endif
}

void MainWindow::showSystemInfo()
{
	dialogs::SystemInfoDialog *dlg = findChild<dialogs::SystemInfoDialog *>(
		"systeminfodialog", Qt::FindDirectChildrenOnly);
	if(dlg) {
		dlg->setParent(getStartDialogOrThis());
	} else {
		dlg = new dialogs::SystemInfoDialog(this);
		dlg->setObjectName("systeminfodialog");
		dlg->setAttribute(Qt::WA_DeleteOnClose);
	}
	utils::showWindow(dlg, shouldShowDialogMaximized());
	dlg->activateWindow();
	dlg->raise();
}

void MainWindow::toggleRecording()
{
	if(m_playbackDialog) {
		// If the playback dialog is visible, this action works as the play/pause button
		m_playbackDialog->setPlaying(!m_playbackDialog->isPlaying());
		return;
	}

	if(m_doc->stopRecording()) {
		return; // There was a recording and we just stopped it.
	}

	QString filename = FileWrangler{this}.getSaveRecordingPath();
	if(!filename.isEmpty()) {
		drawdance::RecordStartResult result = m_doc->startRecording(filename);
		switch(result) {
		case drawdance::RECORD_START_SUCCESS:
			break;
		case drawdance::RECORD_START_UNKNOWN_FORMAT:
			showErrorMessage(tr("Unsupported format."));
			break;
		case drawdance::RECORD_START_OPEN_ERROR:
			showErrorMessageWithDetails(tr("Couldn't start recording."), DP_error());
			break;
		default:
			showErrorMessageWithDetails(tr("Unknown error."), DP_error());
			break;
		}
	}
}

void MainWindow::toggleProfile()
{
#ifdef __EMSCRIPTEN__
	QString path = QStringLiteral("/profile.dpperf");
#endif
	if(drawdance::Perf::isOpen()) {
		if(drawdance::Perf::close()) {
#ifdef __EMSCRIPTEN__
			QFile f(path);
			if(f.open(QIODevice::ReadOnly)) {
				FileWrangler(this).saveFileContent(
					QStringLiteral("profile%1.dpperf")
						.arg(QDateTime::currentSecsSinceEpoch()),
					f.readAll());
			} else {
				showErrorMessageWithDetails(
					tr("Error downloading profile."), f.errorString());
			}
			f.remove();
#endif
		} else {
			showErrorMessageWithDetails(tr("Error closing profile."), DP_error());
		}
	} else {
#ifndef __EMSCRIPTEN__
		QString path = FileWrangler{this}.getSavePerformanceProfilePath();
#endif
		if(!path.isEmpty()) {
			if(!drawdance::Perf::open(path)) {
				showErrorMessageWithDetails(tr("Error opening profile."), DP_error());
			}
		}
	}
}

// clang-format on
void MainWindow::toggleTabletEventLog()
{
#ifdef __EMSCRIPTEN__
	QString path = QStringLiteral("/eventlog.dplog");
#endif
	if(drawdance::EventLog::isOpen()) {
		if(drawdance::EventLog::close()) {
#ifdef __EMSCRIPTEN__
			QFile f(path);
			if(f.open(QIODevice::ReadOnly)) {
				FileWrangler(this).saveFileContent(
					QStringLiteral("eventlog%1.dplog")
						.arg(QDateTime::currentSecsSinceEpoch()),
					f.readAll());
			} else {
				showErrorMessageWithDetails(
					tr("Error downloading tablet event log."), f.errorString());
			}
			f.remove();
#endif
		} else {
			showErrorMessageWithDetails(
				tr("Error closing tablet event log."), DP_error());
		}
	} else {
#ifndef __EMSCRIPTEN__
		QString path = FileWrangler{this}.getSaveTabletEventLogPath();
#endif
		if(!path.isEmpty()) {
			if(drawdance::EventLog::open(path)) {
				DP_event_log_write_meta(
					"Drawpile: %s", cmake_config::version());
				DP_event_log_write_meta("Qt: %s", QT_VERSION_STR);
				DP_event_log_write_meta(
					"OS: %s", qUtf8Printable(QSysInfo::prettyProductName()));
				DP_event_log_write_meta(
					"Platform: %s",
					qUtf8Printable(QGuiApplication::platformName()));
				DP_event_log_write_meta("Input: %s", tabletinput::current());
				config::Config *cfg = dpAppConfig();
				DP_event_log_write_meta(
					"Tablet enabled: %d", cfg->getTabletEvents());
				DP_event_log_write_meta(
					"Tablet eraser action: %d", cfg->getTabletEraserAction());
				DP_event_log_write_meta(
					"One-finger touch action: %d", cfg->getOneFingerTouch());
				DP_event_log_write_meta(
					"Two-finger pinch action: %d", cfg->getTwoFingerPinch());
				DP_event_log_write_meta(
					"Two-finger twist action: %d", cfg->getTwoFingerTwist());
				DP_event_log_write_meta(
					"One-finger tap action: %d", cfg->getOneFingerTap());
				DP_event_log_write_meta(
					"Two-finger tap action: %d", cfg->getTwoFingerTap());
				DP_event_log_write_meta(
					"Three-finger tap action: %d", cfg->getThreeFingerTap());
				DP_event_log_write_meta(
					"Four-finger tap action: %d", cfg->getFourFingerTap());
				DP_event_log_write_meta(
					"Gestures: %d", cfg->getTouchGestures());
			} else {
				showErrorMessageWithDetails(
					tr("Error opening tablet event log."), DP_error());
			}
		}
	}
}

void MainWindow::showBrushSettingsDialogBrush()
{
	showBrushSettingsDialog(false);
}

void MainWindow::showBrushSettingsDialogPreset()
{
	showBrushSettingsDialog(true);
}

void MainWindow::showBrushSettingsDialog(bool openOnPresetPage)
{
	dialogs::BrushSettingsDialog *dlg =
		findChild<dialogs::BrushSettingsDialog *>(
			"brushsettingsdialog", Qt::FindDirectChildrenOnly);
	if(!dlg) {
		dlg = new dialogs::BrushSettingsDialog(this);
		dlg->setObjectName("brushsettingsdialog");
		dlg->setAttribute(Qt::WA_DeleteOnClose);

		tools::BrushSettings *brushSettings =
			m_dockToolSettings->brushSettings();
		brushes::BrushPresetModel *presetModel =
			dpApp().brushPresets()->presetModel();
		std::function<void(int, bool)> updatePreset =
			[brushSettings, presetModel, dlg](int presetId, bool attached) {
				QSignalBlocker blocker(dlg);
				dlg->setPresetAttached(attached, presetId);
				dlg->setPresetName(brushSettings->currentPresetName());
				dlg->setPresetDescription(
					brushSettings->currentPresetDescription());
				dlg->setPresetThumbnail(
					brushSettings->currentPresetThumbnail());
				dlg->setPresetShortcut(
					presetId > 0 ? presetModel->getShortcutForPresetId(presetId)
								 : QKeySequence());
			};
		connect(
			brushSettings, &tools::BrushSettings::presetIdChanged, dlg,
			updatePreset);
		connect(
			dlg, &dialogs::BrushSettingsDialog::presetNameChanged,
			brushSettings, &tools::BrushSettings::changeCurrentPresetName);
		connect(
			dlg, &dialogs::BrushSettingsDialog::presetDescriptionChanged,
			brushSettings,
			&tools::BrushSettings::changeCurrentPresetDescription);
		connect(
			dlg, &dialogs::BrushSettingsDialog::presetThumbnailChanged,
			brushSettings, &tools::BrushSettings::changeCurrentPresetThumbnail);
		connect(
			dlg, &dialogs::BrushSettingsDialog::brushSettingsChanged,
			brushSettings, &tools::BrushSettings::changeCurrentBrush);
		connect(
			brushSettings, &tools::BrushSettings::eraseModeChanged, dlg,
			&dialogs::BrushSettingsDialog::setForceEraseMode);
		updatePreset(
			brushSettings->currentPresetId(),
			brushSettings->isCurrentPresetAttached());
		dlg->setForceEraseMode(brushSettings->isCurrentEraserSlot());

		tools::ToolController *toolCtrl = m_doc->toolCtrl();
		connect(
			toolCtrl, &tools::ToolController::activeBrushChanged, dlg,
			&dialogs::BrushSettingsDialog::updateUiFromActiveBrush);
		connect(
			toolCtrl,
			&tools::ToolController::stabilizerUseBrushSampleCountChanged, dlg,
			&dialogs::BrushSettingsDialog::setStabilizerUseBrushSampleCount);
		connect(
			toolCtrl, &tools::ToolController::globalSmoothingChanged, dlg,
			&dialogs::BrushSettingsDialog::setGlobalSmoothing);
		dlg->updateUiFromActiveBrush(toolCtrl->activeBrush());
		dlg->setStabilizerUseBrushSampleCount(
			toolCtrl->stabilizerUseBrushSampleCount());
		dlg->setGlobalSmoothing(toolCtrl->globalSmoothing());

		connect(
			dlg, &dialogs::BrushSettingsDialog::newBrushRequested,
			m_dockBrushPalette, &docks::BrushPalette::newPreset);
		connect(
			dlg, &dialogs::BrushSettingsDialog::overwriteBrushRequested,
			m_dockBrushPalette,
			std::bind(
				&docks::BrushPalette::overwriteCurrentPreset,
				m_dockBrushPalette, dlg));
		connect(
			presetModel, &brushes::BrushPresetModel::presetShortcutChanged, dlg,
			[dlg](int presetId, const QKeySequence &shortcut) {
				if(dlg->isPresetAttached() && dlg->presetId() == presetId) {
					dlg->setPresetShortcut(shortcut);
				}
			});
		connect(
			dlg, &dialogs::BrushSettingsDialog::shortcutChangeRequested, this,
			[this](int presetId) {
				showSettings()->initiateBrushShortcutChange(presetId);
			});

		connect(
			m_doc, &Document::compatibilityModeChanged, dlg,
			&dialogs::BrushSettingsDialog::setCompatibilityMode);
		dlg->setCompatibilityMode(m_doc->isCompatibilityMode());

		if(openOnPresetPage) {
			dlg->showPresetPage();
		} else {
			dlg->showGeneralPage();
		}
	}

	utils::showWindow(dlg, shouldShowDialogMaximized());
	dlg->activateWindow();
	dlg->raise();
}

/**
 * The settings window will automatically destruct when it is closed.
 */
dialogs::SettingsDialog *MainWindow::showSettings()
{
	QString objectName = QStringLiteral("settingsdialog");
	QWidget *dlgParent = getStartDialogOrThis();
	dialogs::SettingsDialog *dlg =
		dlgParent->findChild<dialogs::SettingsDialog *>(
			objectName, Qt::FindDirectChildrenOnly);
	if(!dlg) {
		dlg = new dialogs::SettingsDialog(
			m_singleSession, m_smallScreenMode, dlgParent);
		dlg->setAttribute(Qt::WA_DeleteOnClose);
		dlg->setObjectName(objectName);
#if defined(Q_OS_ANDROID) && defined(KRITA_QT_SCREEN_DENSITY_ADJUSTMENT)
		connect(
			dlg, &dialogs::SettingsDialog::scalingChangeRequested, &dpApp(),
			&DrawpileApp::showAndroidScalingDialog);
#endif
		connect(
			dlg, &dialogs::SettingsDialog::tabletTesterRequested, this,
			std::bind(&MainWindow::showTabletTestDialog, this, dlg));
		connect(
			dlg, &dialogs::SettingsDialog::touchTesterRequested, this,
			std::bind(&MainWindow::showTouchTestDialog, this, dlg));
		connect(
			dlg, &dialogs::SettingsDialog::projectRecordingSettingsRequested,
			this, &MainWindow::showProjectRecordingSettings);
		utils::showWindow(dlg, shouldShowDialogMaximized());
	}
	return dlg;
}

void MainWindow::showNetworkSettings()
{
	dialogs::SettingsDialog *dlg = showSettings();
	dlg->activateNetworkPanel();
}

void MainWindow::showServerSettings()
{
	dialogs::SettingsDialog *dlg = showSettings();
	dlg->activateServerPanel();
}

void MainWindow::showSessionSettings()
{
	utils::showWindow(m_sessionSettings, shouldShowDialogMaximized());
}

void MainWindow::setSessionPassword()
{
	if(m_doc->client()->isConnected()) {
		showSessionSettings();
		m_sessionSettings->changeSessionPassword();
	}
}

dialogs::TabletTestDialog *MainWindow::showTabletTestDialog(QWidget *parent)
{
	QString name = QStringLiteral("tablettestdialog");
	dialogs::TabletTestDialog *ttd =
		parent->findChild<dialogs::TabletTestDialog *>(
			name, Qt::FindDirectChildrenOnly);
	if(ttd) {
		ttd->activateWindow();
		ttd->raise();
	} else {
		ttd = new dialogs::TabletTestDialog(parent);
		utils::makeModal(ttd);
		ttd->setAttribute(Qt::WA_DeleteOnClose);
		ttd->setObjectName(name);
		utils::showWindow(ttd, shouldShowDialogMaximized());
	}
	return ttd;
}

dialogs::TouchTestDialog *MainWindow::showTouchTestDialog(QWidget *parent)
{
	QString name = QStringLiteral("touchtestdialog");
	dialogs::TouchTestDialog *ttd =
		parent->findChild<dialogs::TouchTestDialog *>(
			name, Qt::FindDirectChildrenOnly);
	if(ttd) {
		ttd->activateWindow();
		ttd->raise();
	} else {
		ttd = new dialogs::TouchTestDialog(parent);
		utils::makeModal(ttd);
		ttd->setAttribute(Qt::WA_DeleteOnClose);
		ttd->setObjectName(name);
		utils::showWindow(ttd, shouldShowDialogMaximized());
	}
	return ttd;
}

void MainWindow::host()
{
	showStartDialogOnPage(int(dialogs::StartDialog::Entry::Host));
}

void MainWindow::hostSession(const HostParams &params, int connectStrategy)
{
	if(m_doc->client()->isConnected()) {
		showErrorMessage(
			tr("You're already connected to a session! Disconnect "
			   "first to host one."));
		return;
	}

	canvas::CanvasModel *canvas = m_doc->canvas();
	if(!canvas) {
		showErrorMessage(tr("No canvas to host! Create one or open a file."));
		return;
	}

	if(!canvas->paintEngine()->viewCanvasState().isSizeInBounds()) {
		showErrorMessage(tr("Canvas size too large to host."));
		return;
	}

	QString remoteAddress = params.address;
	bool useremote = !remoteAddress.isEmpty();
	QUrl address;

	if(useremote) {
		address = QUrl(
			net::addSchemeToUserSuppliedAddress(remoteAddress),
			QUrl::TolerantMode);
	} else {
		address.setHost(WhatIsMyIp::guessLocalAddress());
		address.setScheme(QStringLiteral("drawpile"));
	}

	if(!address.isValid() || address.host().isEmpty()) {
		showErrorMessage(tr("Invalid address"));
		return;
	}

	// Start server if hosting locally
	config::Config *cfg = dpAppConfig();
	if(!useremote) {
#ifdef DP_HAVE_BUILTIN_SERVER
		canvas::PaintEngine *paintEngine = m_doc->canvas()->paintEngine();
		server::BuiltinServer *server =
			new server::BuiltinServer(paintEngine, this);

		QString errorMessage;
		bool serverStarted = server->start(
			cfg->getServerPort(), cfg->getServerTimeout(),
			cfg->getNetworkProxyMode(), &errorMessage);
		if(!serverStarted) {
			utils::showWarning(this, tr("Host Session"), errorMessage);
			delete server;
			return;
		}

		connect(
			m_doc->client(), &net::Client::serverDisconnected, server,
			&server::BuiltinServer::stop);
		paintEngine->setServer(server);

		if(server->port() != cmake_config::proto::port()) {
			address.setPort(server->port());
		}
#else
		showErrorMessage(tr("Hosting on this computer is not available"));
		return;
#endif
	}

	net::LoginHostParams *loginParams = new net::LoginHostParams;
	loginParams->remote = useremote;
	loginParams->nsfm = params.nsfm;
	loginParams->keepChat = params.keepChat;
	loginParams->deputies = params.deputies;
	loginParams->userId = m_doc->canvas()->localUserId();
	loginParams->alias = params.alias;
	loginParams->title = params.title;
	loginParams->password = params.password;
	loginParams->operatorPassword = params.operatorPassword;
	loginParams->announcementUrls = params.announcementUrls;
	loginParams->bansToImport = params.bans;
	loginParams->authToImport = params.auth;

	if(useremote) {
		utils::ScopedOverrideCursor waitCursor;
		loginParams->initialState = m_doc->canvas()->generateSnapshot(
			true, DP_ACL_STATE_RESET_IMAGE_SESSION_RESET_FLAGS,
			params.undoLimit, &params.featurePermissions,
			&params.featureLimits);
	} else {
		QVector<uint8_t> features;
		features.reserve(DP_FEATURE_COUNT);
		for(int feature = 0; feature < DP_FEATURE_COUNT; ++feature) {
			QHash<int, int>::const_iterator found =
				params.featurePermissions.constFind(feature);
			int effectiveTier = DP_feature_access_tier_default(
				feature, int(DP_ACCESS_TIER_OPERATOR));
			if(found != params.featurePermissions.constEnd()) {
				int tier = found.value();
				if(tier >= 0 && tier < DP_ACCESS_TIER_COUNT) {
					effectiveTier = tier;
				}
			}
			features.append(effectiveTier);
		}

		QVector<int32_t> limits;
		limits.reserve(int(DP_FEATURE_LIMIT_COUNT) * int(DP_ACCESS_TIER_COUNT));
		for(int i = 0; i < DP_FEATURE_LIMIT_COUNT; ++i) {
			QHash<int, QHash<int, int>>::const_iterator tiersFound =
				params.featureLimits.constFind(i);
			for(int j = 0; j < DP_ACCESS_TIER_COUNT; ++j) {
				int effectiveLimit = DP_feature_limit_default(i, j, -1);
				if(tiersFound != params.featureLimits.constEnd()) {
					QHash<int, int>::const_iterator limitFound =
						tiersFound->constFind(j);
					if(limitFound != tiersFound->constEnd()) {
						effectiveLimit = limitFound.value();
					}
				}
				limits.append(effectiveLimit);
			}
		}

		loginParams->initialState = {
			net::makeFeatureAccessLevelsMessage(0, features),
			net::makeFeatureLimitsMessage(0, limits),
			net::makeUndoDepthMessage(0, params.undoLimit),
		};
	}

	net::LoginHandler *login = new net::LoginHandler(
		QSharedPointer<const net::LoginHostParams>(loginParams), QString(),
		address, 0, QStringList(), QJsonObject(), this);

	utils::showWindow(
		new dialogs::LoginDialog(login, getStartDialogOrThis()),
		shouldShowDialogMaximized());

	m_doc->connectToServer(
		cfg->getServerTimeout(), cfg->getNetworkProxyMode(),
		net::resolveConnectStrategy(
			connectStrategy, net::defaultConnectStrategy()),
		login, !useremote);
}

void MainWindow::invite()
{
	QString objectName = QStringLiteral("invitedialog");
	dialogs::InviteDialog *dlg = findChild<dialogs::InviteDialog *>(
		objectName, Qt::FindDirectChildrenOnly);
	if(dlg) {
		dlg->activateWindow();
		dlg->raise();
	} else {
		canvas::CanvasModel *canvas = m_doc->canvas();
		net::Client *client = m_doc->client();
		if(canvas && client->isConnected()) {
			canvas::AclState *acls = canvas->aclState();
			dlg = new dialogs::InviteDialog(
				m_netstatus, m_doc->inviteList(),
				m_doc->isSessionWebSupported(), m_doc->isSessionAllowWeb(),
				m_doc->isSessionPreferWebSockets(), m_doc->isSessionNsfm(),
				acls->amOperator(), client->isModerator(),
				m_doc->serverSupportsInviteCodes(),
				m_doc->isSessionInviteCodesEnabled(),
				m_doc->isCompatibilityMode(), this);
			dlg->setAttribute(Qt::WA_DeleteOnClose);
			dlg->setObjectName(objectName);
			connect(
				m_doc, &Document::serverDisconnected, dlg,
				&dialogs::InviteDialog::close);
			connect(
				m_doc, &Document::sessionWebSupportedChanged, dlg,
				&dialogs::InviteDialog::setSessionWebSupported);
			connect(
				m_doc, &Document::sessionAllowWebChanged, dlg,
				&dialogs::InviteDialog::setSessionAllowWeb);
			connect(
				m_doc, &Document::sessionPreferWebSocketsChanged, dlg,
				&dialogs::InviteDialog::setSessionPreferWebSockets);
			connect(
				m_doc, &Document::sessionNsfmChanged, dlg,
				&dialogs::InviteDialog::setSessionNsfm);
			connect(
				acls, &canvas::AclState::localOpChanged, dlg,
				&dialogs::InviteDialog::setOp);
			connect(
				m_doc, &Document::sessionInviteCodesEnabledChanged, dlg,
				&dialogs::InviteDialog::setSessionCodesEnabled);
			connect(
				m_doc, &Document::serverSupportsInviteCodesChanged, dlg,
				&dialogs::InviteDialog::setServerSupportsInviteCodes);
			connect(
				client, &net::Client::inviteCodeCreated, dlg,
				&dialogs::InviteDialog::selectInviteCode);
			connect(
				dlg, &dialogs::InviteDialog::createInviteCode, m_doc,
				&Document::sendCreateInviteCode);
			connect(
				dlg, &dialogs::InviteDialog::removeInviteCode, m_doc,
				&Document::sendRemoveInviteCode);
			connect(
				dlg, &dialogs::InviteDialog::setInviteCodesEnabled, m_doc,
				&Document::sendInviteCodesEnabled);
			connect(
				dlg, &dialogs::InviteDialog::sessionSettingsRequested,
				getAction("sessionsettings"), &QAction::trigger);
			connect(
				dlg, &dialogs::InviteDialog::setSessionPasswordRequested, this,
				&MainWindow::setSessionPassword);
			dlg->show();
		}
	}
}

void MainWindow::join()
{
	if(m_singleSession) {
		reconnect();
	} else {
		showStartDialogOnPage(int(dialogs::StartDialog::Entry::Join));
	}
}

void MainWindow::reconnect()
{
	reconnectWith(false);
}

void MainWindow::reconnectWith(bool downloaded)
{
	m_reconnectAfterSave = false;

#ifdef __EMSCRIPTEN__
	bool needsConfirmation = true;
#else
	Q_UNUSED(downloaded);
	bool needsConfirmation =
		getReplacementCriteria().testFlag(ReplacementCriterion::Dirty);
#endif

	if(needsConfirmation) {
		QString message;
#ifdef SINGLE_MAIN_WINDOW
#	ifdef __EMSCRIPTEN__
		if(downloaded) {
			message = tr("Did the download complete successfully?");
		} else {
			message =
				tr("You have unsaved changes, do you want to download them "
				   "before reconnecting?");
		}
#	else
		message =
			tr("You have unsaved changes, do you want to save them before "
			   "reconnecting?");
#	endif
#else
		message =
			tr("You have unsaved changes, do you want to save them before "
			   "reconnecting or reconnect in a new window?");
#endif

		QWidget *parent = getStartDialogOrThis();
		QMessageBox *box = utils::makeMessage(
			parent, tr("Reconnect"), message, QString(), QMessageBox::Question,
			QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

#ifdef __EMSCRIPTEN__
		if(downloaded) {
			box->button(QMessageBox::Save)->setText(tr("No, try again"));
			box->button(QMessageBox::Discard)->setText(tr("Yes, reconnect"));
		} else {
			box->button(QMessageBox::Save)->setText(tr("Download"));
		}
#endif

#ifndef SINGLE_MAIN_WINDOW
		QPushButton *newWindowButton =
			//: Button to reconnect in a new window instead of the current one.
			//: Is shown next to Save, Discard and Cancel buttons.
			box->addButton(tr("New Window"), QMessageBox::ActionRole);
		if(box->style()->styleHint(
			   QStyle::SH_DialogButtonBox_ButtonsHaveIcons)) {
			newWindowButton->setIcon(QIcon::fromTheme("window_"));
		}
#endif

		connect(
			box, &QMessageBox::buttonClicked, this,
			[=](QAbstractButton *button) {
				if(button == box->button(QMessageBox::Save)) {
					m_reconnectAfterSave = true;
					QTimer::singleShot(0, this, [this] {
						if(!m_doc->isSaveInProgress()) {
#ifdef __EMSCRIPTEN__
							download();
#else
							save();
#endif
						}
					});
				} else if(button == box->button(QMessageBox::Discard)) {
					reconnectToSession(true);
#ifndef SINGLE_MAIN_WINDOW
				} else if(button == newWindowButton) {
					reconnectToSession(false);
#endif
				}
			});

		utils::showMessageBox(box);
	} else {
		reconnectToSession(true);
	}
}

void MainWindow::browse()
{
	showStartDialogOnPage(int(dialogs::StartDialog::Entry::Browse));
}
// clang-format off

/**
 * Leave action triggered, ask for confirmation
 */
void MainWindow::leave()
{
	QMessageBox *leavebox = utils::makeQuestion(
		this,
		m_doc->sessionTitle().isEmpty() ? tr("Untitled") : m_doc->sessionTitle(),
		m_doc->client()->isBuiltin()
			? tr("Really leave and terminate the session?")
			: tr("Really leave the session?"));
	leavebox->button(QMessageBox::Yes)->setText(tr("Leave"));
	leavebox->button(QMessageBox::No)->setText(tr("Stay"));
	leavebox->setDefaultButton(QMessageBox::No);
	connect(leavebox, &QMessageBox::finished, this, [this](int result) {
		if(result == QMessageBox::Yes) {
			m_doc->client()->disconnectFromServer();
		}
	});

	if(m_doc->client()->uploadQueueBytes() > 0 || m_doc->isStreamingReset()) {
		leavebox->setIcon(QMessageBox::Warning);
		leavebox->setInformativeText(tr("There is still unsent data! Please wait until transmission completes!"));
	}

	utils::showMessageBox(leavebox);
}

// clang-format on
#ifndef __EMSCRIPTEN__
void MainWindow::checkForUpdates()
{
	dialogs::StartDialog *dlg =
		showStartDialogOnPage(int(dialogs::StartDialog::Entry::Welcome));
	dlg->checkForUpdates();
}
#endif

void MainWindow::reportAbuse()
{
	dialogs::AbuseReportDialog *dlg = new dialogs::AbuseReportDialog(this);
	dlg->setAttribute(Qt::WA_DeleteOnClose);

	dlg->setSessionInfo(QString(), QString(), m_doc->sessionTitle());

	const canvas::UserListModel *userlist = m_doc->canvas()->userlist();
	for(const auto &u : userlist->users()) {
		if(u.isOnline && u.id != m_doc->canvas()->localUserId())
			dlg->addUser(u.id, u.name);
	}

	connect(dlg, &dialogs::AbuseReportDialog::accepted, this, [this, dlg]() {
		m_doc->sendAbuseReport(dlg->userId(), dlg->message());
	});

	utils::showWindow(dlg, shouldShowDialogMaximized());
}

void MainWindow::tryToGainOp()
{
	utils::getInputPassword(
		this, tr("Become Operator"), tr("Enter operator password"), QString(),
		[this](const QString &opword) {
			if(!opword.isEmpty()) {
				m_doc->sendOpword(opword);
			}
		});
}

void MainWindow::resetSession()
{
	utils::ScopedOverrideCursor waitCursor;
	dialogs::ResetDialog *dlg = new dialogs::ResetDialog(
		m_doc->canvas()->paintEngine(), m_singleSession, this);
	utils::makeModal(dlg);
	dlg->setAttribute(Qt::WA_DeleteOnClose);

#ifndef SINGLE_MAIN_WINDOW
	// It's always possible to create a new document from a snapshot
	connect(dlg, &dialogs::ResetDialog::newSelected, this, [dlg]() {
		MainWindow *w = new MainWindow(false);
		w->m_doc->sendResetSession(
			dlg->getResetImage(w->m_doc->isCompatibilityMode()));
		dlg->deleteLater();
	});
#endif

	// Session resetting is available only to session operators that aren't
	// running an outdated client.
	if(m_doc->isMinorIncompatibility()) {
		dlg->setReset(dialogs::ResetDialog::Reset::DisabledIncompatible);
	} else if(!m_doc->canvas()->aclState()->amOperator()) {
		dlg->setReset(dialogs::ResetDialog::Reset::DisabledNotOp);
	} else {
		connect(dlg, &dialogs::ResetDialog::resetSelected, this, [this, dlg]() {
			utils::ScopedOverrideCursor innerWaitCursor;
			canvas::CanvasModel *canvas = m_doc->canvas();
			if(canvas->aclState()->amOperator()) {
				if(dlg->isExternalResetImage()) {
					// The user picked an external file to reset to, clear the
					// save file path so they don't accidentally overwrite it.
					m_doc->clearPaths();
				}
				net::MessageList snapshot =
					dlg->getResetImage(m_doc->isCompatibilityMode());
				canvas->amendSnapshotMetadata(
					snapshot, true,
					m_doc->isCompatibilityMode()
						? DP_ACL_STATE_RESET_IMAGE_SESSION_RESET_COMPAT_FLAGS
						: DP_ACL_STATE_RESET_IMAGE_SESSION_RESET_FLAGS);
				m_doc->sendResetSession(snapshot, dlg->getResetImageType());
			}
			dlg->deleteLater();
		});
	}

	utils::showWindow(dlg, shouldShowDialogMaximized());
}

void MainWindow::terminateSession()
{
	// When hosting on the builtin server, terminating the session isn't done
	// through mod commands, it's a matter of leaving and stopping the server.
	if(m_doc->client()->isBuiltin()) {
		leave();
	} else {
		QInputDialog *dlg = new QInputDialog(this);
		dlg->setInputMode(QInputDialog::TextInput);
		dlg->setAttribute(Qt::WA_DeleteOnClose);
		dlg->setWindowTitle(tr("Terminate session"));
		dlg->setLabelText(tr("Reason:"));
		dlg->setOkButtonText(tr("Terminate"));
#ifndef SINGLE_MAIN_WINDOW
		dlg->setWindowModality(Qt::WindowModal);
#endif
		connect(
			dlg, &QInputDialog::textValueSelected, m_doc,
			&Document::sendTerminateSession);
		dlg->show();
	}
}

void MainWindow::joinSession(
	const QUrl &url, const QString &autoRecordFile, int connectStrategy)
{
	questionWindowReplacement(
		tr("Join Session"),
		tr("You're about to connect to a new session and close this window."),
		[this, url, autoRecordFile, connectStrategy](bool ok) {
			if(ok) {
				connectToSession(url, autoRecordFile, connectStrategy, false);
			}
		});
}

void MainWindow::reconnectToSession(bool forceSameWindow)
{
	const net::Client *client = m_doc->client();
	connectToSession(
		client->sessionUrl(true), QString(), client->reconnectStrategy(),
		forceSameWindow);
}

void MainWindow::connectToSession(
	const QUrl &url, const QString &autoRecordFile, int connectStrategy,
	bool forceSameWindow)
{
	m_canvasView->hideDisconnectedWarning();

	if(!forceSameWindow && !canReplace()) {
		m_doc->clearReconnectState();
		prepareWindowReplacement();

		QStringList args;
		QVector<QPair<QString, QString>> envVars;
		args.reserve(8);
		args.append(QStringLiteral("--no-restore-window-position"));
		if(m_singleSession) {
			args.append(QStringLiteral("--single-session"));
		}
		args.append(QStringLiteral("--join"));

		QString username = url.userName();
		QString password = url.password();
		if(username.isEmpty() && password.isEmpty()) {
			args.append(url.toString(QUrl::FullyEncoded));
		} else {
			QUrl joinUrl = url;
			joinUrl.setUserInfo(QString());
			args.append(joinUrl.toString(QUrl::FullyEncoded));
			envVars.append({QStringLiteral("DRAWPILE_JOIN_USER"), username});
			envVars.append({QStringLiteral("DRAWPILE_JOIN_PASS"), password});
		}

		if(!autoRecordFile.isEmpty()) {
			args.append(QStringLiteral("--auto-record"));
			args.append(autoRecordFile);
		}

		if(connectStrategy != 0) {
			args.append(QStringLiteral("--connect-strategy"));
			args.append(net::connectStrategyToString(connectStrategy));
		}

		bool newProcessStarted = dpApp().runInNewProcess(args, envVars);
		if(newProcessStarted) {
			emit windowReplacementFailed(nullptr);
		} else {
			createNewWindow(
				[this, url, autoRecordFile, connectStrategy](MainWindow *win) {
					if(m_singleSession) {
						win->m_doc->client()->setSessionUrl(url);
					}
					win->connectToSession(
						url, autoRecordFile, connectStrategy, true);
				});
		}
		return;
	}

	QString autoJoinId = net::extractAutoJoinIdFromUrl(url);
	net::LoginHandler *login = new net::LoginHandler(
		QSharedPointer<const net::LoginHostParams>(nullptr), autoJoinId, url, 0,
		QStringList(), QJsonObject(), this);

	dialogs::LoginDialog *dlg =
		new dialogs::LoginDialog(login, getStartDialogOrThis());
	connect(
		m_doc, &Document::catchupProgress, dlg,
		&dialogs::LoginDialog::catchupProgress);
	connect(
		m_doc, &Document::serverLoggedIn, dlg,
		&dialogs::LoginDialog::onLoginDone);
	connect(
		dlg, &dialogs::LoginDialog::destroyed, this,
		&MainWindow::showCompatibilityModeWarning);
	m_canvasView->connectLoginDialog(m_doc, dlg);

	m_doc->setReconnectStatePrevious(
		m_dockLayers->currentId(), m_dockTimeline->currentTrackId(),
		m_dockTimeline->currentFrame());

	dlg->show();
	m_doc->setRecordOnConnect(autoRecordFile);
	config::Config *cfg = dpAppConfig();
	m_doc->setProjectRecordOnConnect(cfg->getAutoRecordJoin());
	m_doc->connectToServer(
		cfg->getServerTimeout(), cfg->getNetworkProxyMode(),
		net::resolveConnectStrategy(
			connectStrategy, net::defaultConnectStrategy()),
		login, false);
}

void MainWindow::onServerConnected()
{
	// Enable connection related actions
	emit hostSessionEnabled(false);
	getAction("leavesession")->setEnabled(true);
	getAction("sessionsettings")->setEnabled(true);
	if(m_singleSession) {
		getAction("joinsession")->setEnabled(false);
	}

	// Disable UI until login completes
	m_canvasView->viewWidget()->setEnabled(false);
	setDrawingToolsEnabled(false);
}
// clang-format off

void MainWindow::onServerDisconnected(
	const QString &message, const QString &errorcode, bool localDisconnect,
	bool anyMessageReceived)
{
	canvas::CanvasModel *canvas = m_doc->canvas();
	emit hostSessionEnabled(canvas != nullptr);
#ifdef DP_HAVE_BUILTIN_SERVER
	if(canvas) {
		canvas->paintEngine()->setServer(nullptr);
	}
#endif

	getAction("invitesession")->setEnabled(false);
	getAction("leavesession")->setEnabled(false);
	getAction("sessionsettings")->setEnabled(false);
	getAction("reportabuse")->setEnabled(false);
	getAction("terminatesession")->setEnabled(false);
	if(m_singleSession) {
		getAction("joinsession")->setEnabled(true);
	}
	m_admintools->setEnabled(false);
	m_sessionSettings->close();

	// Re-enable UI
	m_canvasView->viewWidget()->setEnabled(true);
	setDrawingToolsEnabled(true);

	// Display login error if not yet logged in
	if(!m_doc->client()->isLoggedIn() && !localDisconnect &&
	   m_lastDisconnectNotificationTimer.hasExpired()) {
		QString name = QStringLiteral("disconnectederrormessagebox");
		QMessageBox *msgbox = findChild<QMessageBox *>(name);
		if(!msgbox) {
			QString title;
			QString description;
			if(anyMessageReceived) {
				title = tr("Disconnected");
				description = tr("You've been disconnected from the server.");
			} else {
				title = tr("Connection Failed");
				description =
					tr("Could not establish a connection to the server.");
#ifdef __EMSCRIPTEN__
				browser::intuitFailedConnectionReason(
					description, m_doc->client()->connectionUrl());
#endif
			}

			msgbox =
				utils::showWarning(getStartDialogOrThis(), title, description);
			msgbox->setObjectName(name);
		}

		if(msgbox->informativeText().isEmpty() && !message.isEmpty()) {
			msgbox->setInformativeText(message);
		}

		if(errorcode == "SESSIONIDINUSE") {
			// We tried to host a session using with a vanity ID, but that
			// ID was taken. Show a button for quickly joining that session instead
			msgbox->setInformativeText(msgbox->informativeText() + "\n" + tr("Would you like to join the session instead?"));

			QAbstractButton *joinbutton = msgbox->addButton(tr("Join"), QMessageBox::YesRole);

			msgbox->removeButton(msgbox->button(QMessageBox::Ok));
			msgbox->addButton(QMessageBox::Cancel);

			QUrl url = m_doc->client()->sessionUrl(true);

			connect(joinbutton, &QAbstractButton::clicked, this, [this, url]() {
				connectToSession(url, QString(), m_doc->client()->reconnectStrategy(), false);
			});

		}

		msgbox->open();
	}

	// If logged in but disconnected unexpectedly, show notification bar.
	// Always show it in single-session mode, reconnecting is the only recourse.
	if(m_singleSession || (m_doc->client()->isLoggedIn() && !localDisconnect)) {
		QString notif = message.isEmpty()
				? tr("You've been disconnected from the session.")
				: tr("Disconnected: %1").arg(message);
		m_canvasView->showDisconnectedWarning(notif, m_singleSession);
		if(m_lastDisconnectNotificationTimer.hasExpired()) {
			dpApp().notifications()->trigger(
				this, notification::Event::Disconnect, notif);
			m_lastDisconnectNotificationTimer.setRemainingTime(2000);
		}
	}

#ifndef __EMSCRIPTEN__
	if(m_exitAction != RUNNING) {
		m_exitAction = RUNNING;
		QTimer::singleShot(100, this, &QMainWindow::close);
	}
#endif
}

// clang-format on
void MainWindow::onServerDisconnectedAgain(
	const QString &message, const QString &errorcode)
{
	Q_UNUSED(errorcode);
	if(!message.isEmpty()) {
		QString name = QStringLiteral("disconnectederrormessagebox");
		QMessageBox *msgbox = findChild<QMessageBox *>(name);
		if(msgbox) {
			if(msgbox->informativeText().length() < message.length()) {
				msgbox->setInformativeText(message);
			}
		}
	}
}

void MainWindow::onCompatibilityModeChanged(bool compatibilityMode)
{
	getAction("lightnesstoalphaarea")->setEnabled(!compatibilityMode);
	getAction("darknesstoalphaarea")->setEnabled(!compatibilityMode);
	QAction *maskselection = getAction("maskselection");
	maskselection->setEnabled(!compatibilityMode);
	maskselection->setChecked(
		!compatibilityMode && m_doc->toolCtrl()->isSelectionMaskingEnabled());
}
// clang-format off

/**
 * Server connection established and login successfull
 */
void MainWindow::onServerLogin(bool join, const QString &joinPassword)
{
	m_initialCatchup = join;
	net::Client *client = m_doc->client();
	m_netstatus->loggedIn(client->sessionUrl(), joinPassword);
	m_netstatus->setSecurityLevel(
		client->securityLevel(), client->hostCertificate(),
		client->isSelfSignedCertificate());
	m_canvasView->viewWidget()->setEnabled(true);
	m_canvasView->hideDisconnectedWarning();
	m_sessionSettings->setPersistenceEnabled(client->serverSuppotsPersistence());
	m_sessionSettings->setBanImpExEnabled(
		client->isModerator(), client->serverSupportsCryptBanImpEx(),
		client->serverSupportsModBanImpEx());
	m_sessionSettings->setAutoResetEnabled(client->sessionSupportsAutoReset());
	m_sessionSettings->setAuthenticated(client->isAuthenticated());
	setDrawingToolsEnabled(true);
	getAction("terminatesession")->setEnabled(
		client->isModerator() || client->isBuiltin());
	onOperatorModeChange(m_doc->canvas()->aclState()->amOperator());
	getAction("reportabuse")->setEnabled(client->serverSupportsReports());
	getAction("invitesession")->setEnabled(true);
	if(m_chatbox->isCollapsed()) {
		getAction("togglechat")->trigger();
	}
	if(!join && dpAppConfig()->getShowInviteDialogOnHost()) {
		invite();
	}
}

// clang-format on
void MainWindow::triggerUpdateLockState()
{
	if(!m_lockStateUpdatePending) {
		m_lockStateUpdatePending = true;
		emit lockStateUpdateRequested();
	}
}

void MainWindow::triggerUpdateLockStateOnSelectionChange()
{
	if(m_dockToolSettings->currentToolRequiresSelection()) {
		triggerUpdateLockState();
	}
}

namespace {
struct InpaintOptions {
	QString prompt;
	QString negativePrompt;
	int seed = -1;
	double cfg = 5.0;
	double denoise = 0.75;
	int steps = 30;
	QString scheduler = QStringLiteral("dpmpp-3m-karras");
	QString samplerPreset = QStringLiteral("balanced");
	bool refinerEnabled = false;
	QString refinerPlacement = QStringLiteral("before-detail");
	bool detailPassEnabled = false;
	int candidateCount = 3;
	int edgeFeatherPx = 24;
};

struct RefinerOptions {
	bool enabled = false;
	QString modelId = QStringLiteral("sdxl-refiner-diffusers");
	QString backend = QStringLiteral("diffusers");
	QString model = QStringLiteral("stabilityai/stable-diffusion-xl-refiner-1.0");
	double strength = 0.25;
	int steps = 20;
	bool inheritSampler = true;
	QString scheduler = QStringLiteral("dpmpp-3m-karras");
	QString placement = QStringLiteral("before-detail");
	QString upscaleBackend = QStringLiteral("pil-lanczos-unsharp");
};

struct AiRegistryModel {
	QString id;
	QString displayName;
	QString backend;
	QString model;
	QStringList capabilities;
	bool experimental = false;
};

struct DetailPassOptions {
	bool enabled = false;
	bool faceEnabled = true;
	bool bodyEnabled = false;
	bool handsEnabled = false;
	double detectionConfidence = 0.5;
	int maxRegions = 4;
	int maskPaddingPx = 32;
	int detailRenderEdge = 1024;
	int minCropEdge = 256;
	double denoise = 0.35;
	int steps = 28;
	bool inheritSampler = true;
	QString scheduler = QStringLiteral("dpmpp-3m-karras");
	QString upscaleBackend = QStringLiteral("pil-lanczos-unsharp");
};

struct ColorSeparationOptions {
	int maxRegions = 5;
	int minRegionAreaPct = 3;
	QString decompositionDepth = QStringLiteral("balanced");
	bool groupRepeatedRegions = true;
};

struct ObjectDecompositionOptions {
	QString segmentationBackend = QStringLiteral("sam");
	int maxMasks = 64;
	double minRegionAreaPct = 0.12;
	QString decompositionDepth = QStringLiteral("detailed");
	bool personPriorEnabled = true;
	int personPriorConfidencePct = 5;
	int personPriorMaxRegions = 64;
	double personPriorMinAreaPct = 0.05;
	bool objectPriorEnabled = true;
	int objectPriorConfidencePct = 12;
	int objectPriorMaxRegions = 64;
	double objectPriorMinAreaPct = 0.03;
	bool samGridFallbackEnabled = true;
	bool groupRepeatedRegions = true;
	bool repairBase = false;
};

struct ObjectDecompositionRunResult {
	ai::JobRunResult decomposition;
	ai::JobRunResult repair;
	bool repairRequested = false;
	bool repairAttempted = false;
	bool repairMaskFallbackUsed = false;
	bool semanticHelperAttempted = false;
	int semanticRegionsClassified = 0;
	int semanticRegionsFailed = 0;
	int semanticGroupsRefined = 0;
	QString semanticFatalError;
	QString semanticLastError;
	QString repairSourcePath;
	QString repairMaskPath;
};

struct SamplerPreset {
	QString id;
	QString label;
	QString scheduler;
	double cfg;
	double denoise;
	int steps;
};

class AiPreviewLabel final : public QLabel {
public:
	explicit AiPreviewLabel(QWidget *parent = nullptr)
		: QLabel(parent)
	{
		setAlignment(Qt::AlignCenter);
	}

	void setPreviewPixmap(const QPixmap &pixmap)
	{
		if(pixmap.isNull()) {
			return;
		}

		if(m_currentPixmap.isNull()) {
			m_currentPixmap = pixmap;
			m_blend = 1.0;
			updateGeometry();
			update();
			return;
		}

		m_previousPixmap = m_currentPixmap;
		m_currentPixmap = pixmap;
		m_blend = 0.0;
		if(m_animation) {
			m_animation->stop();
			m_animation->deleteLater();
		}

		m_animation = new QVariantAnimation(this);
		m_animation->setDuration(220);
		m_animation->setStartValue(0.0);
		m_animation->setEndValue(1.0);
		m_animation->setEasingCurve(QEasingCurve::InOutQuad);
		connect(
			m_animation, &QVariantAnimation::valueChanged, this,
			[this](const QVariant &value) {
				m_blend = value.toDouble();
				update();
			});
		connect(
			m_animation, &QVariantAnimation::finished, this, [this] {
				m_previousPixmap = QPixmap();
				m_blend = 1.0;
				m_animation->deleteLater();
				m_animation = nullptr;
				update();
			});
		m_animation->start();
		updateGeometry();
		update();
	}

	void setStatusText(const QString &text)
	{
		m_statusText = text;
		if(m_currentPixmap.isNull()) {
			setText(text);
		}
		update();
	}

protected:
	void paintEvent(QPaintEvent *event) override
	{
		if(m_currentPixmap.isNull()) {
			QLabel::paintEvent(event);
			drawStatusText();
			return;
		}

		QPainter painter(this);
		drawCenteredPixmap(painter, m_previousPixmap, 1.0 - m_blend);
		drawCenteredPixmap(painter, m_currentPixmap, m_blend);
		drawStatusText(&painter);
	}

	QSize sizeHint() const override
	{
		return m_currentPixmap.isNull()
				   ? QLabel::sizeHint()
				   : m_currentPixmap.size().boundedTo(QSize(280, 210));
	}

private:
	void drawCenteredPixmap(QPainter &painter, const QPixmap &pixmap, qreal opacity)
	{
		if(pixmap.isNull() || opacity <= 0.0) {
			return;
		}

		const QSize targetSize = pixmap.size().scaled(
			contentsRect().size(), Qt::KeepAspectRatio);
		const QRect targetRect(
			QPoint(
				contentsRect().center().x() - targetSize.width() / 2,
				contentsRect().center().y() - targetSize.height() / 2),
			targetSize);
		painter.setOpacity(opacity);
		painter.drawPixmap(targetRect, pixmap);
		painter.setOpacity(1.0);
	}

	void drawStatusText(QPainter *existingPainter = nullptr)
	{
		if(m_statusText.isEmpty()) {
			return;
		}
		QPainter localPainter;
		QPainter &painter = existingPainter ? *existingPainter : localPainter;
		if(!existingPainter) {
			localPainter.begin(this);
		}
		const QFontMetrics metrics(font());
		const QRect textRect = metrics.boundingRect(
			contentsRect().adjusted(8, 8, -8, -8),
			Qt::AlignLeft | Qt::AlignBottom | Qt::TextWordWrap, m_statusText);
		const QRect background = textRect.adjusted(-6, -4, 6, 4);
		painter.fillRect(background, QColor(0, 0, 0, 150));
		painter.setPen(Qt::white);
		painter.drawText(
			textRect, Qt::AlignLeft | Qt::AlignBottom | Qt::TextWordWrap,
			m_statusText);
		if(!existingPainter) {
			localPainter.end();
		}
	}

	QPixmap m_previousPixmap;
	QPixmap m_currentPixmap;
	QString m_statusText;
	qreal m_blend = 1.0;
	QVariantAnimation *m_animation = nullptr;
};

QWidget *makeIntSlider(
	QWidget *parent, int min, int max, int value,
	const std::function<QString(int)> &format, QSlider *&outSlider)
{
	QWidget *widget = new QWidget(parent);
	QHBoxLayout *layout = new QHBoxLayout(widget);
	layout->setContentsMargins(0, 0, 0, 0);
	outSlider = new QSlider(Qt::Horizontal, widget);
	outSlider->setRange(min, max);
	outSlider->setValue(qBound(min, value, max));
	QLabel *valueLabel = new QLabel(format(outSlider->value()), widget);
	valueLabel->setMinimumWidth(54);
	valueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	QObject::connect(outSlider, &QSlider::valueChanged, valueLabel, [=](int v) {
		valueLabel->setText(format(v));
	});
	layout->addWidget(outSlider, 1);
	layout->addWidget(valueLabel);
	return widget;
}

int effectiveDiffusionSteps(int steps, double strength)
{
	steps = qMax(1, steps);
	strength = qBound(0.0, strength, 1.0);
	return qMax(1, qMin(int(steps * strength), steps));
}

QString underpaintRegistryPath()
{
	const QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
	const QString configured =
		environment.value(QStringLiteral("UNDERPAINT_MODEL_REGISTRY"));
	if(!configured.isEmpty()) {
		return configured;
	}

	const QString relativePath =
		QStringLiteral("tools/ai/model-registry.json");
	const QString root =
		environment.value(QStringLiteral("UNDERPAINT_TOOL_ROOT"));
	if(!root.isEmpty()) {
		const QString path = QDir(root).filePath(relativePath);
		if(QFile::exists(path)) {
			return path;
		}
	}

	const QString currentPath = QDir::current().filePath(relativePath);
	if(QFile::exists(currentPath)) {
		return currentPath;
	}

	const QString appPath =
		QDir(QApplication::applicationDirPath()).filePath(
			QStringLiteral("../../%1").arg(relativePath));
	if(QFile::exists(appPath)) {
		return appPath;
	}

	return currentPath;
}

QVector<AiRegistryModel> loadAiModelRegistry()
{
	QVector<AiRegistryModel> models;
	QFile file(underpaintRegistryPath());
	if(!file.open(QIODevice::ReadOnly)) {
		return models;
	}
	QJsonParseError error;
	const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &error);
	if(error.error != QJsonParseError::NoError || !document.isObject()) {
		return models;
	}
	const QJsonArray entries =
		document.object().value(QStringLiteral("models")).toArray();
	for(const QJsonValue &value : entries) {
		const QJsonObject object = value.toObject();
		AiRegistryModel model;
		model.id = object.value(QStringLiteral("id")).toString();
		model.displayName =
			object.value(QStringLiteral("displayName")).toString(model.id);
		model.backend = object.value(QStringLiteral("backend")).toString();
		model.model = object.value(QStringLiteral("model")).toString();
		model.experimental =
			object.value(QStringLiteral("experimental")).toBool(false);
		const QJsonArray capabilities =
			object.value(QStringLiteral("capabilities")).toArray();
		for(const QJsonValue &capability : capabilities) {
			const QString capabilityName = capability.toString();
			if(!capabilityName.isEmpty()) {
				model.capabilities.append(capabilityName);
			}
		}
		if(!model.id.isEmpty()) {
			models.append(model);
		}
	}
	return models;
}

AiRegistryModel registryModelById(const QString &modelId)
{
	for(const AiRegistryModel &model : loadAiModelRegistry()) {
		if(model.id == modelId) {
			return model;
		}
	}
	return {};
}

RefinerOptions defaultRefinerOptions()
{
	return RefinerOptions{};
}

QString normalizeRefinerPlacement(const QString &placement)
{
	return placement == QStringLiteral("after-detail")
			   ? QStringLiteral("after-detail")
			   : QStringLiteral("before-detail");
}

QString refinerPlacementLabel(const QString &placement)
{
	return normalizeRefinerPlacement(placement) == QStringLiteral("after-detail")
			   ? MainWindow::tr("after detailer")
			   : MainWindow::tr("before detailer");
}

QString normalizeDetailUpscaleBackend(const QString &backend)
{
	return backend == QStringLiteral("pil-lanczos")
			   ? QStringLiteral("pil-lanczos")
			   : QStringLiteral("pil-lanczos-unsharp");
}

QString detailUpscaleBackendLabel(const QString &backend)
{
	return normalizeDetailUpscaleBackend(backend) == QStringLiteral("pil-lanczos")
			   ? MainWindow::tr("Lanczos")
			   : MainWindow::tr("Lanczos + sharpen");
}

void addDetailUpscaleBackendChoices(QComboBox *backend)
{
	backend->addItem(
		MainWindow::tr("Lanczos + sharpen"),
		QStringLiteral("pil-lanczos-unsharp"));
	backend->addItem(MainWindow::tr("Lanczos"), QStringLiteral("pil-lanczos"));
}

RefinerOptions loadRefinerOptions()
{
	RefinerOptions defaults = defaultRefinerOptions();
	QSettings settings;
	settings.beginGroup(QStringLiteral("underpaint/ai/refiner"));
	RefinerOptions options;
	options.enabled =
		settings.value(QStringLiteral("enabled"), defaults.enabled).toBool();
	options.modelId =
		settings.value(QStringLiteral("modelId"), defaults.modelId).toString();
	options.backend =
		settings.value(QStringLiteral("backend"), defaults.backend).toString();
	options.model =
		settings.value(QStringLiteral("model"), defaults.model).toString();
	options.strength =
		settings.value(QStringLiteral("strength"), defaults.strength).toDouble();
	options.steps =
		settings.value(QStringLiteral("steps"), defaults.steps).toInt();
	options.inheritSampler =
		settings.value(QStringLiteral("inheritSampler"), defaults.inheritSampler)
			.toBool();
	options.scheduler =
		settings.value(QStringLiteral("scheduler"), defaults.scheduler).toString();
	options.placement = normalizeRefinerPlacement(
		settings.value(QStringLiteral("placement"), defaults.placement).toString());
	options.upscaleBackend = normalizeDetailUpscaleBackend(
		settings.value(
					QStringLiteral("upscaleBackend"),
					defaults.upscaleBackend)
			.toString());
	settings.endGroup();

	if(options.model.trimmed().isEmpty()) {
		options.model = defaults.model;
	}
	const AiRegistryModel registryModel = registryModelById(options.modelId);
	if(!registryModel.id.isEmpty()) {
		if(options.backend.isEmpty()) {
			options.backend = registryModel.backend;
		}
		if(options.model.trimmed().isEmpty()) {
			options.model = registryModel.model;
		}
	}
	if(options.backend != QStringLiteral("diffusers") &&
	   options.backend != QStringLiteral("gguf")) {
		options.backend = defaults.backend;
	}
	options.strength = qBound(0.05, options.strength, 1.0);
	options.steps = qBound(1, options.steps, 200);
	if(options.scheduler.isEmpty()) {
		options.scheduler = defaults.scheduler;
	}
	return options;
}

void saveRefinerOptions(const RefinerOptions &options)
{
	QSettings settings;
	settings.beginGroup(QStringLiteral("underpaint/ai/refiner"));
	settings.setValue(QStringLiteral("enabled"), options.enabled);
	settings.setValue(QStringLiteral("modelId"), options.modelId);
	settings.setValue(QStringLiteral("backend"), options.backend);
	settings.setValue(QStringLiteral("model"), options.model);
	settings.setValue(QStringLiteral("strength"), options.strength);
	settings.setValue(QStringLiteral("steps"), options.steps);
	settings.setValue(QStringLiteral("inheritSampler"), options.inheritSampler);
	settings.setValue(QStringLiteral("scheduler"), options.scheduler);
	settings.setValue(
		QStringLiteral("placement"), normalizeRefinerPlacement(options.placement));
	settings.setValue(
		QStringLiteral("upscaleBackend"),
		normalizeDetailUpscaleBackend(options.upscaleBackend));
	settings.endGroup();
}

QString refinerSummary(const RefinerOptions &options)
{
	return options.enabled
			   ? MainWindow::tr(
					 "On: %1 via %2, strength %3, %4 steps (~%5 effective), %6, %7 pre-upscale")
					 .arg(options.model)
					 .arg(options.backend)
					 .arg(options.strength, 0, 'f', 2)
					 .arg(options.steps)
					 .arg(effectiveDiffusionSteps(options.steps, options.strength))
					 .arg(refinerPlacementLabel(options.placement))
					 .arg(detailUpscaleBackendLabel(options.upscaleBackend))
			   : MainWindow::tr("Off");
}

QString refinerRunBlocker(bool enabled)
{
	if(!enabled) {
		return QString();
	}
	const RefinerOptions options = loadRefinerOptions();
	if(options.backend == QStringLiteral("gguf") &&
	   QProcessEnvironment::systemEnvironment()
		   .value(QStringLiteral("UNDERPAINT_GGUF_REFINER_WORKER"))
		   .trimmed()
		   .isEmpty()) {
		return MainWindow::tr(
			"The selected refiner uses the experimental GGUF backend, but no "
			"GGUF image runner is configured.\n\n"
			"Turn off the refiner, choose a Diffusers refiner model, or set "
			"UNDERPAINT_GGUF_REFINER_WORKER to an external GGUF refiner adapter.");
	}
	return QString();
}

DetailPassOptions defaultDetailPassOptions()
{
	return DetailPassOptions{};
}

DetailPassOptions loadDetailPassOptions()
{
	DetailPassOptions defaults = defaultDetailPassOptions();
	QSettings settings;
	settings.beginGroup(QStringLiteral("underpaint/ai/detailPass"));
	DetailPassOptions options;
	options.enabled =
		settings.value(QStringLiteral("enabled"), defaults.enabled).toBool();
	options.faceEnabled =
		settings.value(QStringLiteral("faceEnabled"), defaults.faceEnabled)
			.toBool();
	options.bodyEnabled =
		settings.value(QStringLiteral("bodyEnabled"), defaults.bodyEnabled)
			.toBool();
	options.handsEnabled =
		settings.value(QStringLiteral("handsEnabled"), defaults.handsEnabled)
			.toBool();
	options.detectionConfidence =
		settings.value(
					QStringLiteral("detectionConfidence"),
					defaults.detectionConfidence)
			.toDouble();
	options.maxRegions =
		settings.value(QStringLiteral("maxRegions"), defaults.maxRegions).toInt();
	options.maskPaddingPx =
		settings.value(QStringLiteral("maskPaddingPx"), defaults.maskPaddingPx)
			.toInt();
	options.detailRenderEdge =
		settings.value(QStringLiteral("detailRenderEdge"), defaults.detailRenderEdge)
			.toInt();
	options.minCropEdge =
		settings.value(QStringLiteral("minCropEdge"), defaults.minCropEdge).toInt();
	options.denoise =
		settings.value(QStringLiteral("denoise"), defaults.denoise).toDouble();
	options.steps =
		settings.value(QStringLiteral("steps"), defaults.steps).toInt();
	options.inheritSampler =
		settings.value(QStringLiteral("inheritSampler"), defaults.inheritSampler)
			.toBool();
	options.scheduler =
		settings.value(QStringLiteral("scheduler"), defaults.scheduler).toString();
	options.upscaleBackend = normalizeDetailUpscaleBackend(
		settings.value(
					QStringLiteral("upscaleBackend"),
					defaults.upscaleBackend)
			.toString());
	settings.endGroup();

	options.detectionConfidence =
		qBound(0.01, options.detectionConfidence, 1.0);
	options.maxRegions = qBound(1, options.maxRegions, 16);
	options.maskPaddingPx = qBound(0, options.maskPaddingPx, 256);
	options.detailRenderEdge = qBound(1024, options.detailRenderEdge, 1536);
	options.minCropEdge = qBound(64, options.minCropEdge, 1024);
	options.denoise = qBound(0.05, options.denoise, 1.0);
	options.steps = qBound(1, options.steps, 200);
	if(options.scheduler.isEmpty()) {
		options.scheduler = defaults.scheduler;
	}
	return options;
}

void saveDetailPassOptions(const DetailPassOptions &options)
{
	QSettings settings;
	settings.beginGroup(QStringLiteral("underpaint/ai/detailPass"));
	settings.setValue(QStringLiteral("enabled"), options.enabled);
	settings.setValue(QStringLiteral("faceEnabled"), options.faceEnabled);
	settings.setValue(QStringLiteral("bodyEnabled"), options.bodyEnabled);
	settings.setValue(QStringLiteral("handsEnabled"), options.handsEnabled);
	settings.setValue(
		QStringLiteral("detectionConfidence"), options.detectionConfidence);
	settings.setValue(QStringLiteral("maxRegions"), options.maxRegions);
	settings.setValue(QStringLiteral("maskPaddingPx"), options.maskPaddingPx);
	settings.setValue(QStringLiteral("detailRenderEdge"), options.detailRenderEdge);
	settings.setValue(QStringLiteral("minCropEdge"), options.minCropEdge);
	settings.setValue(QStringLiteral("denoise"), options.denoise);
	settings.setValue(QStringLiteral("steps"), options.steps);
	settings.setValue(QStringLiteral("inheritSampler"), options.inheritSampler);
	settings.setValue(QStringLiteral("scheduler"), options.scheduler);
	settings.setValue(
		QStringLiteral("upscaleBackend"),
		normalizeDetailUpscaleBackend(options.upscaleBackend));
	settings.endGroup();
}

void saveDetailPassEnabled(bool enabled)
{
	DetailPassOptions options = loadDetailPassOptions();
	options.enabled = enabled;
	saveDetailPassOptions(options);
}

QString detailPassSummary(const DetailPassOptions &options)
{
	if(!options.enabled) {
		return MainWindow::tr("Off");
	}
	QStringList enabledParts;
	if(options.faceEnabled) {
		enabledParts.append(MainWindow::tr("face"));
	}
	if(options.bodyEnabled) {
		enabledParts.append(MainWindow::tr("body"));
	}
	if(options.handsEnabled) {
		enabledParts.append(MainWindow::tr("hands"));
	}
	return enabledParts.isEmpty()
			   ? MainWindow::tr("On, no regions enabled")
			   : MainWindow::tr(
					 "On: %1, %2 px detail edge, %3 steps (~%4 effective), %5 pre-upscale")
					 .arg(enabledParts.join(QStringLiteral(", ")))
					 .arg(options.detailRenderEdge)
					 .arg(options.steps)
					 .arg(effectiveDiffusionSteps(options.steps, options.denoise))
					 .arg(detailUpscaleBackendLabel(options.upscaleBackend));
}

void addSchedulerChoices(QComboBox *scheduler)
{
	scheduler->addItem(MainWindow::tr("Euler"), QStringLiteral("euler"));
	scheduler->addItem(MainWindow::tr("Euler A"), QStringLiteral("euler-a"));
	scheduler->addItem(MainWindow::tr("DDIM"), QStringLiteral("ddim"));
	scheduler->addItem(MainWindow::tr("DPM++ 3M"), QStringLiteral("dpmpp-3m"));
	scheduler->addItem(
		MainWindow::tr("DPM++ 3M Karras"),
		QStringLiteral("dpmpp-3m-karras"));
}

void selectScheduler(QComboBox *scheduler, const QString &value)
{
	const int index = scheduler->findData(value);
	scheduler->setCurrentIndex(index >= 0 ? index : 0);
}

QVector<SamplerPreset> samplerPresets()
{
	return {
		{
			QStringLiteral("balanced"),
			MainWindow::tr("Balanced"),
			QStringLiteral("dpmpp-3m-karras"),
			5.0,
			0.75,
			30,
		},
		{
			QStringLiteral("precise"),
			MainWindow::tr("Precise"),
			QStringLiteral("dpmpp-3m-karras"),
			4.5,
			0.55,
			36,
		},
		{
			QStringLiteral("soft-blend"),
			MainWindow::tr("Soft Blend"),
			QStringLiteral("ddim"),
			4.0,
			0.50,
			32,
		},
		{
			QStringLiteral("creative"),
			MainWindow::tr("Creative"),
			QStringLiteral("euler-a"),
			6.5,
			0.85,
			32,
		},
	};
}

SamplerPreset samplerPresetById(const QString &id)
{
	const QVector<SamplerPreset> presets = samplerPresets();
	for(const SamplerPreset &preset : presets) {
		if(preset.id == id) {
			return preset;
		}
	}
	return presets.constFirst();
}

QString samplerPresetIdForValues(
	const QString &scheduler, double cfg, double denoise, int steps)
{
	const QVector<SamplerPreset> presets = samplerPresets();
	for(const SamplerPreset &preset : presets) {
		if(preset.scheduler == scheduler && qAbs(preset.cfg - cfg) < 0.05 &&
		   qAbs(preset.denoise - denoise) < 0.005 && preset.steps == steps) {
			return preset.id;
		}
	}
	return QStringLiteral("custom");
}

void addSamplerPresetChoices(QComboBox *combo)
{
	for(const SamplerPreset &preset : samplerPresets()) {
		combo->addItem(preset.label, preset.id);
	}
	combo->addItem(MainWindow::tr("Custom"), QStringLiteral("custom"));
}

void selectSamplerPreset(QComboBox *combo, const QString &id)
{
	const int index = combo->findData(id);
	combo->setCurrentIndex(index >= 0 ? index : 0);
}

QIcon speechBubbleIcon()
{
	const QIcon themed = QIcon::fromTheme(QStringLiteral("mail-message-new"));
	if(!themed.isNull()) {
		return themed;
	}

	QPixmap pixmap(18, 18);
	pixmap.fill(Qt::transparent);
	QPainter painter(&pixmap);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setPen(QPen(QColor(70, 70, 70), 1.6));
	painter.setBrush(QColor(245, 245, 245));
	painter.drawRoundedRect(QRectF(2.5, 2.5, 13.0, 10.0), 2.5, 2.5);
	QPolygonF tail;
	tail << QPointF(6.0, 12.0) << QPointF(4.0, 16.0)
		 << QPointF(10.0, 12.0);
	painter.drawPolygon(tail);
	painter.setPen(QPen(QColor(90, 90, 90), 1.2));
	painter.drawLine(QPointF(5.5, 6.0), QPointF(12.5, 6.0));
	painter.drawLine(QPointF(5.5, 9.0), QPointF(10.5, 9.0));
	return QIcon(pixmap);
}

QJsonObject refinerParameters(bool enabled, const QString &jobScheduler)
{
	RefinerOptions options = loadRefinerOptions();
	const QString scheduler =
		options.inheritSampler ? jobScheduler : options.scheduler;
	return QJsonObject{
		{QStringLiteral("enabled"), enabled},
		{QStringLiteral("modelId"), options.modelId},
		{QStringLiteral("backend"), options.backend},
		{QStringLiteral("model"), options.model},
		{QStringLiteral("strength"), options.strength},
		{QStringLiteral("steps"), options.steps},
		{QStringLiteral("scheduler"), scheduler},
		{QStringLiteral("inheritSampler"), options.inheritSampler},
		{QStringLiteral("placement"), normalizeRefinerPlacement(options.placement)},
		{QStringLiteral("upscaleBackend"),
		 normalizeDetailUpscaleBackend(options.upscaleBackend)},
	};
}

QJsonObject detailPassParameters(bool enabled, const QString &jobScheduler)
{
	DetailPassOptions options = loadDetailPassOptions();
	const QString scheduler =
		options.inheritSampler ? jobScheduler : options.scheduler;
	return QJsonObject{
		{QStringLiteral("enabled"), enabled},
		{QStringLiteral("faceEnabled"), options.faceEnabled},
		{QStringLiteral("bodyEnabled"), options.bodyEnabled},
		{QStringLiteral("handsEnabled"), options.handsEnabled},
		{QStringLiteral("detectionConfidence"), options.detectionConfidence},
		{QStringLiteral("maxRegions"), options.maxRegions},
		{QStringLiteral("maskPaddingPx"), options.maskPaddingPx},
		{QStringLiteral("detailRenderEdge"), options.detailRenderEdge},
		{QStringLiteral("minCropEdge"), options.minCropEdge},
		{QStringLiteral("denoise"), options.denoise},
		{QStringLiteral("steps"), options.steps},
		{QStringLiteral("scheduler"), scheduler},
		{QStringLiteral("inheritSampler"), options.inheritSampler},
		{QStringLiteral("upscaleBackend"),
		 normalizeDetailUpscaleBackend(options.upscaleBackend)},
		{QStringLiteral("fallbackToEditMask"), false},
	};
}

bool showRefinerSettingsDialog(QWidget *parent)
{
	QDialog dialog(parent);
	dialog.setWindowTitle(MainWindow::tr("Refiner Settings"));
	dialog.resize(560, 320);

	QVBoxLayout *layout = new QVBoxLayout(&dialog);
	QLabel *summary = new QLabel(
		MainWindow::tr(
			"Configure the optional SDXL refiner stage. The refiner can run before "
			"the targeted face/body detailer, or after it when you want one final "
			"global polish pass."),
		&dialog);
	summary->setWordWrap(true);
	layout->addWidget(summary);

	QGroupBox *modelGroup = new QGroupBox(MainWindow::tr("Model"), &dialog);
	QFormLayout *modelForm = new QFormLayout(modelGroup);
	QCheckBox *enabled =
		new QCheckBox(MainWindow::tr("Enable refiner by default"), modelGroup);
	QComboBox *registryModel = new QComboBox(modelGroup);
	registryModel->addItem(MainWindow::tr("Custom"), QString());
	for(const AiRegistryModel &entry : loadAiModelRegistry()) {
		if(entry.capabilities.contains(QStringLiteral("refine"))) {
			QString label = entry.displayName;
			if(entry.experimental) {
				label += MainWindow::tr(" (experimental)");
			}
			registryModel->addItem(label, entry.id);
		}
	}
	QComboBox *backend = new QComboBox(modelGroup);
	backend->addItem(MainWindow::tr("Diffusers"), QStringLiteral("diffusers"));
	backend->addItem(MainWindow::tr("GGUF (experimental)"), QStringLiteral("gguf"));
	QLineEdit *model = new QLineEdit(modelGroup);
	model->setPlaceholderText(
		QStringLiteral("stabilityai/stable-diffusion-xl-refiner-1.0"));
	modelForm->addRow(QString(), enabled);
	modelForm->addRow(MainWindow::tr("Registry model"), registryModel);
	modelForm->addRow(MainWindow::tr("Backend"), backend);
	modelForm->addRow(MainWindow::tr("Model"), model);
	layout->addWidget(modelGroup);

	QGroupBox *generationGroup =
		new QGroupBox(MainWindow::tr("Generation"), &dialog);
	QFormLayout *generationForm = new QFormLayout(generationGroup);
	QSlider *strength = nullptr;
	QWidget *strengthSlider = makeIntSlider(
		&dialog, 5, 100, 25,
		[](int v) { return QString::number(v / 100.0, 'f', 2); }, strength);
	QSlider *steps = nullptr;
	QWidget *stepsSlider = makeIntSlider(
		&dialog, 1, 200, 20, [](int v) { return QString::number(v); }, steps);
	QCheckBox *inheritSampler =
		new QCheckBox(MainWindow::tr("Use the inpaint sampler"), generationGroup);
	QComboBox *scheduler = new QComboBox(generationGroup);
	addSchedulerChoices(scheduler);
	QComboBox *placement = new QComboBox(generationGroup);
	placement->addItem(
		MainWindow::tr("Before detailer"), QStringLiteral("before-detail"));
	placement->addItem(
		MainWindow::tr("After detailer"), QStringLiteral("after-detail"));
	placement->setToolTip(MainWindow::tr(
		"Before detailer keeps detected faces, bodies, and hands as the final "
		"targeted pass. After detailer applies one final whole-image polish."));
	QComboBox *upscaleBackend = new QComboBox(generationGroup);
	addDetailUpscaleBackendChoices(upscaleBackend);
	upscaleBackend->setToolTip(MainWindow::tr(
		"Upscale candidates before the refiner runs. This keeps small refiner "
		"inputs from being polished at visibly low resolution."));
	generationForm->addRow(MainWindow::tr("Strength"), strengthSlider);
	generationForm->addRow(MainWindow::tr("Steps"), stepsSlider);
	generationForm->addRow(QString(), inheritSampler);
	generationForm->addRow(MainWindow::tr("Sampler"), scheduler);
	generationForm->addRow(MainWindow::tr("Run order"), placement);
	generationForm->addRow(MainWindow::tr("Pre-upscale"), upscaleBackend);
	layout->addWidget(generationGroup);

	auto applyOptionsToControls = [&] (const RefinerOptions &options) {
		enabled->setChecked(options.enabled);
		const int registryIndex = registryModel->findData(options.modelId);
		registryModel->setCurrentIndex(registryIndex >= 0 ? registryIndex : 0);
		const int backendIndex = backend->findData(options.backend);
		backend->setCurrentIndex(backendIndex >= 0 ? backendIndex : 0);
		model->setText(options.model);
		strength->setValue(qRound(options.strength * 100.0));
		steps->setValue(options.steps);
		inheritSampler->setChecked(options.inheritSampler);
		selectScheduler(scheduler, options.scheduler);
		scheduler->setEnabled(!options.inheritSampler);
		const int placementIndex =
			placement->findData(normalizeRefinerPlacement(options.placement));
		placement->setCurrentIndex(placementIndex >= 0 ? placementIndex : 0);
		const int upscaleIndex = upscaleBackend->findData(
			normalizeDetailUpscaleBackend(options.upscaleBackend));
		upscaleBackend->setCurrentIndex(upscaleIndex >= 0 ? upscaleIndex : 0);
	};

	QObject::connect(
		registryModel, QOverload<int>::of(&QComboBox::currentIndexChanged),
		&dialog, [=](int index) {
			const QString modelId = registryModel->itemData(index).toString();
			const AiRegistryModel entry = registryModelById(modelId);
			if(entry.id.isEmpty()) {
				backend->setEnabled(true);
				model->setReadOnly(false);
				return;
			}
			const int backendIndex = backend->findData(entry.backend);
			backend->setCurrentIndex(backendIndex >= 0 ? backendIndex : 0);
			model->setText(entry.model);
			backend->setEnabled(false);
			model->setReadOnly(true);
		});
	QObject::connect(
		inheritSampler, &QCheckBox::toggled, scheduler,
		[scheduler](bool checked) { scheduler->setDisabled(checked); });
	applyOptionsToControls(loadRefinerOptions());

	QDialogButtonBox *buttons = new QDialogButtonBox(
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
	QPushButton *defaultsButton = buttons->addButton(
		MainWindow::tr("Restore Defaults"), QDialogButtonBox::ResetRole);
	layout->addWidget(buttons);
	QObject::connect(defaultsButton, &QPushButton::clicked, &dialog, [&] {
		applyOptionsToControls(defaultRefinerOptions());
	});
	QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
	QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

	if(dialog.exec() != QDialog::Accepted) {
		return false;
	}

	RefinerOptions options;
	options.enabled = enabled->isChecked();
	options.modelId = registryModel->currentData().toString();
	options.backend = backend->currentData().toString();
	options.model = model->text().trimmed();
	if(options.model.isEmpty()) {
		options.model = defaultRefinerOptions().model;
	}
	options.strength = strength->value() / 100.0;
	options.steps = steps->value();
	options.inheritSampler = inheritSampler->isChecked();
	options.scheduler = scheduler->currentData().toString();
	options.placement = normalizeRefinerPlacement(placement->currentData().toString());
	options.upscaleBackend =
		normalizeDetailUpscaleBackend(upscaleBackend->currentData().toString());
	saveRefinerOptions(options);
	return true;
}

bool showDetailPassSettingsDialog(QWidget *parent)
{
	QDialog dialog(parent);
	dialog.setWindowTitle(MainWindow::tr("Face & Body Detail Settings"));
	dialog.resize(520, 420);

	QVBoxLayout *layout = new QVBoxLayout(&dialog);
	QLabel *summary = new QLabel(
		MainWindow::tr(
			"Configure targeted detail passes that can run after inpaint and "
			"outpaint candidates are generated."),
		&dialog);
	summary->setWordWrap(true);
	layout->addWidget(summary);

	QGroupBox *regionsGroup = new QGroupBox(MainWindow::tr("Regions"), &dialog);
	QFormLayout *regionsForm = new QFormLayout(regionsGroup);
	QCheckBox *enabled =
		new QCheckBox(MainWindow::tr("Enable detail pass by default"), regionsGroup);
	QCheckBox *faces =
		new QCheckBox(MainWindow::tr("Face detailing"), regionsGroup);
	QCheckBox *bodies =
		new QCheckBox(MainWindow::tr("Body detailing"), regionsGroup);
	QCheckBox *hands =
		new QCheckBox(MainWindow::tr("Hand detailing"), regionsGroup);
	regionsForm->addRow(QString(), enabled);
	regionsForm->addRow(QString(), faces);
	regionsForm->addRow(QString(), bodies);
	regionsForm->addRow(QString(), hands);
	layout->addWidget(regionsGroup);

	QGroupBox *detectionGroup =
		new QGroupBox(MainWindow::tr("Detection"), &dialog);
	QFormLayout *detectionForm = new QFormLayout(detectionGroup);
	QSlider *confidence = nullptr;
	QWidget *confidenceSlider = makeIntSlider(
		&dialog, 1, 100, 50,
		[](int v) { return QString::number(v / 100.0, 'f', 2); },
		confidence);
	QSlider *maxRegions = nullptr;
	QWidget *maxRegionsSlider = makeIntSlider(
		&dialog, 1, 16, 4, [](int v) { return QString::number(v); },
		maxRegions);
	QSlider *padding = nullptr;
	QWidget *paddingSlider = makeIntSlider(
		&dialog, 0, 256, 32,
		[](int v) { return MainWindow::tr("%1 px").arg(v); }, padding);
	detectionForm->addRow(MainWindow::tr("Certainty"), confidenceSlider);
	detectionForm->addRow(MainWindow::tr("Max boxes"), maxRegionsSlider);
	detectionForm->addRow(MainWindow::tr("Mask padding"), paddingSlider);
	layout->addWidget(detectionGroup);

	QGroupBox *cropGroup =
		new QGroupBox(MainWindow::tr("Detail Crop"), &dialog);
	QFormLayout *cropForm = new QFormLayout(cropGroup);
	QSlider *detailRenderEdge = nullptr;
	QWidget *detailRenderEdgeSlider = makeIntSlider(
		&dialog, 1024, 1536, 1024,
		[](int v) { return MainWindow::tr("%1 px").arg(v); },
		detailRenderEdge);
	QSlider *minCropEdge = nullptr;
	QWidget *minCropEdgeSlider = makeIntSlider(
		&dialog, 64, 1024, 256,
		[](int v) { return MainWindow::tr("%1 px").arg(v); }, minCropEdge);
	QComboBox *upscaleBackend = new QComboBox(cropGroup);
	addDetailUpscaleBackendChoices(upscaleBackend);
	upscaleBackend->setToolTip(MainWindow::tr(
		"Upscale each detected crop before its diffusion detail pass. The crop "
		"will still render at a minimum 1024 px working width."));
	cropForm->addRow(MainWindow::tr("Render edge"), detailRenderEdgeSlider);
	cropForm->addRow(MainWindow::tr("Minimum crop"), minCropEdgeSlider);
	cropForm->addRow(MainWindow::tr("Pre-upscale"), upscaleBackend);
	layout->addWidget(cropGroup);

	QGroupBox *generationGroup =
		new QGroupBox(MainWindow::tr("Detail Generation"), &dialog);
	QFormLayout *generationForm = new QFormLayout(generationGroup);
	QSlider *denoise = nullptr;
	QWidget *denoiseSlider = makeIntSlider(
		&dialog, 5, 100, 35,
		[](int v) { return QString::number(v / 100.0, 'f', 2); }, denoise);
	QSlider *steps = nullptr;
	QWidget *stepsSlider = makeIntSlider(
		&dialog, 1, 200, 28, [](int v) { return QString::number(v); }, steps);
	QCheckBox *inheritSampler =
		new QCheckBox(MainWindow::tr("Use the inpaint sampler"), generationGroup);
	QComboBox *scheduler = new QComboBox(generationGroup);
	addSchedulerChoices(scheduler);
	generationForm->addRow(MainWindow::tr("Denoise"), denoiseSlider);
	generationForm->addRow(MainWindow::tr("Steps"), stepsSlider);
	generationForm->addRow(QString(), inheritSampler);
	generationForm->addRow(MainWindow::tr("Sampler"), scheduler);
	layout->addWidget(generationGroup);

	auto applyOptionsToControls = [&] (const DetailPassOptions &options) {
		enabled->setChecked(options.enabled);
		faces->setChecked(options.faceEnabled);
		bodies->setChecked(options.bodyEnabled);
		hands->setChecked(options.handsEnabled);
		confidence->setValue(qRound(options.detectionConfidence * 100.0));
		maxRegions->setValue(options.maxRegions);
		padding->setValue(options.maskPaddingPx);
		detailRenderEdge->setValue(options.detailRenderEdge);
		minCropEdge->setValue(options.minCropEdge);
		const int upscaleIndex = upscaleBackend->findData(
			normalizeDetailUpscaleBackend(options.upscaleBackend));
		upscaleBackend->setCurrentIndex(upscaleIndex >= 0 ? upscaleIndex : 0);
		denoise->setValue(qRound(options.denoise * 100.0));
		steps->setValue(options.steps);
		inheritSampler->setChecked(options.inheritSampler);
		selectScheduler(scheduler, options.scheduler);
		scheduler->setEnabled(!options.inheritSampler);
	};

	QObject::connect(
		inheritSampler, &QCheckBox::toggled, scheduler,
		[scheduler](bool checked) { scheduler->setDisabled(checked); });
	applyOptionsToControls(loadDetailPassOptions());

	QDialogButtonBox *buttons = new QDialogButtonBox(
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
	QPushButton *defaultsButton = buttons->addButton(
		MainWindow::tr("Restore Defaults"), QDialogButtonBox::ResetRole);
	layout->addWidget(buttons);
	QObject::connect(defaultsButton, &QPushButton::clicked, &dialog, [&] {
		applyOptionsToControls(defaultDetailPassOptions());
	});
	QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
	QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

	if(dialog.exec() != QDialog::Accepted) {
		return false;
	}

	DetailPassOptions options;
	options.enabled = enabled->isChecked();
	options.faceEnabled = faces->isChecked();
	options.bodyEnabled = bodies->isChecked();
	options.handsEnabled = hands->isChecked();
	options.detectionConfidence = confidence->value() / 100.0;
	options.maxRegions = maxRegions->value();
	options.maskPaddingPx = padding->value();
	options.detailRenderEdge = detailRenderEdge->value();
	options.minCropEdge = minCropEdge->value();
	options.upscaleBackend =
		normalizeDetailUpscaleBackend(upscaleBackend->currentData().toString());
	options.denoise = denoise->value() / 100.0;
	options.steps = steps->value();
	options.inheritSampler = inheritSampler->isChecked();
	options.scheduler = scheduler->currentData().toString();
	saveDetailPassOptions(options);
	return true;
}

QImage selectionMaskToInpaintMask(
	const QImage &selectionMask, const QRect &selectionBounds,
	const QRect &exportRegion)
{
	QImage inpaintMask(exportRegion.size(), QImage::Format_Grayscale8);
	inpaintMask.fill(0);
	const QPoint offset = selectionBounds.topLeft() - exportRegion.topLeft();
	const QRect targetRect(offset, selectionMask.size());
	const QRect clippedTarget = targetRect.intersected(inpaintMask.rect());
	for(int y = clippedTarget.top(); y <= clippedTarget.bottom(); ++y) {
		uchar *out = inpaintMask.scanLine(y);
		const int sourceY = y - offset.y();
		for(int x = clippedTarget.left(); x <= clippedTarget.right(); ++x) {
			out[x] = uchar(qAlpha(selectionMask.pixel(x - offset.x(), sourceY)));
		}
	}
	return inpaintMask;
}

QRect transparentPixelBounds(const QImage &image, int alphaThreshold = 8)
{
	if(image.isNull()) {
		return QRect();
	}
	const QImage source = image.convertToFormat(QImage::Format_ARGB32);
	int left = source.width();
	int top = source.height();
	int right = -1;
	int bottom = -1;
	for(int y = 0; y < source.height(); ++y) {
		const QRgb *line =
			reinterpret_cast<const QRgb *>(source.constScanLine(y));
		for(int x = 0; x < source.width(); ++x) {
			if(qAlpha(line[x]) <= alphaThreshold) {
				left = qMin(left, x);
				top = qMin(top, y);
				right = qMax(right, x);
				bottom = qMax(bottom, y);
			}
		}
	}
	return right >= left && bottom >= top
			   ? QRect(QPoint(left, top), QPoint(right, bottom))
			   : QRect();
}

QRect opaquePixelBounds(const QImage &image, int alphaThreshold = 8)
{
	if(image.isNull()) {
		return QRect();
	}
	const QImage source = image.convertToFormat(QImage::Format_ARGB32);
	int left = source.width();
	int top = source.height();
	int right = -1;
	int bottom = -1;
	for(int y = 0; y < source.height(); ++y) {
		const QRgb *line =
			reinterpret_cast<const QRgb *>(source.constScanLine(y));
		for(int x = 0; x < source.width(); ++x) {
			if(qAlpha(line[x]) > alphaThreshold) {
				left = qMin(left, x);
				top = qMin(top, y);
				right = qMax(right, x);
				bottom = qMax(bottom, y);
			}
		}
	}
	return right >= left && bottom >= top
			   ? QRect(QPoint(left, top), QPoint(right, bottom))
			   : QRect();
}

QImage alphaImageToInpaintMask(
	const QImage &image, const QRect &exportRegion, int alphaThreshold = 8)
{
	QImage inpaintMask(exportRegion.size(), QImage::Format_Grayscale8);
	inpaintMask.fill(0);
	if(image.isNull() || exportRegion.isEmpty()) {
		return inpaintMask;
	}
	const QImage source = image.convertToFormat(QImage::Format_ARGB32);
	const QRect imageBounds(QPoint(), source.size());
	const QRect clipped = exportRegion.intersected(imageBounds);
	for(int canvasY = clipped.top(); canvasY <= clipped.bottom(); ++canvasY) {
		const QRgb *sourceLine =
			reinterpret_cast<const QRgb *>(source.constScanLine(canvasY));
		uchar *out = inpaintMask.scanLine(canvasY - exportRegion.y());
		for(int canvasX = clipped.left(); canvasX <= clipped.right(); ++canvasX) {
			const int alpha = qAlpha(sourceLine[canvasX]);
			if(alpha > alphaThreshold) {
				out[canvasX - exportRegion.x()] = uchar(alpha);
			}
		}
	}
	return inpaintMask;
}

bool candidateIsExtractedObject(const ai::JobCandidate &candidate)
{
	return candidate.metadata.value(QStringLiteral("maskRole")).toString() ==
		   QStringLiteral("extracted-object");
}

bool candidateIsBaseRemainder(const ai::JobCandidate &candidate)
{
	return candidate.metadata.value(QStringLiteral("maskRole")).toString() ==
		   QStringLiteral("base-remainder");
}

QString candidateRepairRole(const ai::JobCandidate &candidate)
{
	const QString repairRole =
		candidate.metadata.value(QStringLiteral("repairRole")).toString().trimmed();
	return repairRole.isEmpty() ? QStringLiteral("remove-from-base") : repairRole;
}

bool candidateKeepsRepairContext(const ai::JobCandidate &candidate)
{
	const QString maskRole =
		candidate.metadata.value(QStringLiteral("maskRole")).toString();
	return maskRole == QStringLiteral("base-remainder") ||
		   candidateRepairRole(candidate) == QStringLiteral("keep-context");
}

bool imageHasVisibleAlpha(const QImage &image)
{
	if(image.isNull()) {
		return false;
	}
	const QImage rgba = image.convertToFormat(QImage::Format_ARGB32);
	for(int y = 0; y < rgba.height(); ++y) {
		const QRgb *line = reinterpret_cast<const QRgb *>(rgba.constScanLine(y));
		for(int x = 0; x < rgba.width(); ++x) {
			if(qAlpha(line[x]) > 0) {
				return true;
			}
		}
	}
	return false;
}

QImage objectDecompositionRepairSourcePlate(
	const QVector<ai::JobCandidate> &candidates, const QSize &canvasSize)
{
	if(candidates.isEmpty() || canvasSize.isEmpty()) {
		return QImage();
	}
	QImage plate(canvasSize, QImage::Format_ARGB32_Premultiplied);
	plate.fill(Qt::transparent);
	QPainter painter(&plate);
	for(const ai::JobCandidate &candidate : candidates) {
		if(!candidateKeepsRepairContext(candidate)) {
			continue;
		}
		QImage image(candidate.imagePath);
		if(image.isNull()) {
			continue;
		}
		if(image.size() != canvasSize) {
			image = image.scaled(
				canvasSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		}
		painter.drawImage(QPoint(0, 0), image);
	}
	painter.end();
	return imageHasVisibleAlpha(plate) ? plate : QImage();
}

QImage expandGrayscaleMask(
	const QImage &mask, int radius, int alphaThreshold);

QImage objectDecompositionRepairMask(
	const QVector<ai::JobCandidate> &candidates, const QSize &canvasSize,
	bool includeKeepContextObjects = false)
{
	if(candidates.isEmpty() || canvasSize.isEmpty()) {
		return QImage();
	}
	QImage repairMask(canvasSize, QImage::Format_Grayscale8);
	repairMask.fill(0);
	bool hasPixels = false;
	const int expandPx =
		qMax(4, qMin(14, (qMin(canvasSize.width(), canvasSize.height()) + 191) / 192));
	for(const ai::JobCandidate &candidate : candidates) {
		if(!candidateIsExtractedObject(candidate) ||
		   (!includeKeepContextObjects && candidateKeepsRepairContext(candidate))) {
			continue;
		}
		QImage candidateMask(candidate.maskPath);
		if(candidateMask.isNull()) {
			continue;
		}
		if(candidateMask.size() != canvasSize) {
			candidateMask = candidateMask.scaled(
				canvasSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		}
		const QImage sourceMask = expandGrayscaleMask(candidateMask, expandPx, 8);
		for(int y = 0; y < repairMask.height(); ++y) {
			uchar *out = repairMask.scanLine(y);
			const uchar *in = sourceMask.constScanLine(y);
			for(int x = 0; x < repairMask.width(); ++x) {
				if(in[x] > out[x]) {
					out[x] = in[x];
					hasPixels = true;
				}
			}
		}
	}
	return hasPixels ? repairMask : QImage();
}

QStringList decompositionPromptPhrases(
	const QVector<ai::JobCandidate> &candidates, bool keepContext)
{
	QStringList phrases;
	for(const ai::JobCandidate &candidate : candidates) {
		if(!candidateIsExtractedObject(candidate)) {
			continue;
		}
		const bool keepsContext = candidateKeepsRepairContext(candidate);
		if(keepsContext != keepContext) {
			continue;
		}
		QString phrase =
			candidate.metadata.value(QStringLiteral("promptPhrase"))
				.toString()
				.simplified()
				.trimmed();
		if(phrase.isEmpty()) {
			phrase = candidate.label.simplified().trimmed();
		}
		if(!phrase.isEmpty() && !phrases.contains(phrase, Qt::CaseInsensitive)) {
			phrases.append(phrase);
		}
		if(phrases.size() >= 8) {
			break;
		}
	}
	return phrases;
}

QString objectDecompositionRepairPrompt(
	const QVector<ai::JobCandidate> &candidates)
{
	const QStringList contextPhrases = decompositionPromptPhrases(candidates, true);
	QString prompt = MainWindow::tr(
		"continue the visible background plate into the removed foreground holes, "
		"matching perspective, color, lighting, texture, and painted detail; "
		"do not invent new people, clothing, limbs, shoes, props, or foreground objects");
	if(!contextPhrases.isEmpty()) {
		prompt += MainWindow::tr("; use context from %1")
					  .arg(contextPhrases.join(QStringLiteral(", ")));
	}
	return prompt;
}

QString objectDecompositionRepairNegativePrompt(
	const QVector<ai::JobCandidate> &candidates)
{
	QStringList negative = decompositionPromptPhrases(candidates, false);
	negative.prepend(MainWindow::tr("new shoes"));
	negative.prepend(MainWindow::tr("new clothing"));
	negative.prepend(MainWindow::tr("extra limbs"));
	negative.prepend(MainWindow::tr("extra people"));
	negative.prepend(MainWindow::tr("duplicate foreground objects"));
	negative.prepend(MainWindow::tr("floating cutout artifacts"));
	negative.removeDuplicates();
	return negative.join(QStringLiteral(", "));
}

QImage transparentPixelsToOutpaintMask(
	const QImage &image, const QRect &exportRegion, int alphaThreshold = 8,
	int contextBleedPx = 0)
{
	QImage outpaintMask(exportRegion.size(), QImage::Format_Grayscale8);
	outpaintMask.fill(0);
	if(image.isNull() || exportRegion.isEmpty()) {
		return outpaintMask;
	}
	const QImage source = image.convertToFormat(QImage::Format_ARGB32);
	const QRect imageBounds(QPoint(), source.size());
	const QRect clipped = exportRegion.intersected(imageBounds);
	for(int canvasY = clipped.top(); canvasY <= clipped.bottom(); ++canvasY) {
		const QRgb *sourceLine =
			reinterpret_cast<const QRgb *>(source.constScanLine(canvasY));
		uchar *out = outpaintMask.scanLine(canvasY - exportRegion.y());
		for(int canvasX = clipped.left(); canvasX <= clipped.right(); ++canvasX) {
			if(qAlpha(sourceLine[canvasX]) <= alphaThreshold) {
				out[canvasX - exportRegion.x()] = 255;
			}
		}
	}
	if(contextBleedPx > 0) {
		const int width = outpaintMask.width();
		const int height = outpaintMask.height();
		const int unreachable = width + height + contextBleedPx + 1;
		QVector<int> distance(width * height, unreachable);
		for(int y = 0; y < height; ++y) {
			const uchar *line = outpaintMask.constScanLine(y);
			for(int x = 0; x < width; ++x) {
				if(line[x] == 255) {
					distance[y * width + x] = 0;
				}
			}
		}
		for(int y = 0; y < height; ++y) {
			for(int x = 0; x < width; ++x) {
				int &value = distance[y * width + x];
				if(x > 0) {
					value = qMin(value, distance[y * width + x - 1] + 1);
				}
				if(y > 0) {
					value = qMin(value, distance[(y - 1) * width + x] + 1);
				}
			}
		}
		for(int y = height - 1; y >= 0; --y) {
			for(int x = width - 1; x >= 0; --x) {
				int &value = distance[y * width + x];
				if(x + 1 < width) {
					value = qMin(value, distance[y * width + x + 1] + 1);
				}
				if(y + 1 < height) {
					value = qMin(value, distance[(y + 1) * width + x] + 1);
				}
			}
		}
		for(int y = 0; y < height; ++y) {
			uchar *line = outpaintMask.scanLine(y);
			for(int x = 0; x < width; ++x) {
				const int d = distance[y * width + x];
				if(line[x] == 0 && d > 0 && d <= contextBleedPx) {
					line[x] = uchar(qRound(
						255.0 * double(contextBleedPx + 1 - d) /
						double(contextBleedPx + 1)));
				}
			}
		}
	}
	return outpaintMask;
}

bool maskHasEditablePixels(const QImage &mask)
{
	const QImage source = mask.convertToFormat(QImage::Format_Grayscale8);
	for(int y = 0; y < source.height(); ++y) {
		const uchar *line = source.constScanLine(y);
		for(int x = 0; x < source.width(); ++x) {
			if(line[x] != 0) {
				return true;
			}
		}
	}
	return false;
}

QImage expandGrayscaleMask(
	const QImage &mask, int radius, int alphaThreshold)
{
	QImage source = mask.convertToFormat(QImage::Format_Grayscale8);
	if(source.isNull() || radius <= 0) {
		return source;
	}
	const int width = source.width();
	const int height = source.height();
	const int unreachable = width + height + radius + 1;
	QVector<int> distance(width * height, unreachable);
	bool hasPixels = false;
	for(int y = 0; y < height; ++y) {
		const uchar *line = source.constScanLine(y);
		for(int x = 0; x < width; ++x) {
			if(line[x] > alphaThreshold) {
				distance[y * width + x] = 0;
				hasPixels = true;
			}
		}
	}
	if(!hasPixels) {
		return source;
	}
	for(int y = 0; y < height; ++y) {
		for(int x = 0; x < width; ++x) {
			int &value = distance[y * width + x];
			if(x > 0) {
				value = qMin(value, distance[y * width + x - 1] + 1);
			}
			if(y > 0) {
				value = qMin(value, distance[(y - 1) * width + x] + 1);
			}
		}
	}
	for(int y = height - 1; y >= 0; --y) {
		for(int x = width - 1; x >= 0; --x) {
			int &value = distance[y * width + x];
			if(x + 1 < width) {
				value = qMin(value, distance[y * width + x + 1] + 1);
			}
			if(y + 1 < height) {
				value = qMin(value, distance[(y + 1) * width + x] + 1);
			}
		}
	}
	QImage expanded(source.size(), QImage::Format_Grayscale8);
	expanded.fill(0);
	for(int y = 0; y < height; ++y) {
		uchar *out = expanded.scanLine(y);
		for(int x = 0; x < width; ++x) {
			if(distance[y * width + x] <= radius) {
				out[x] = 255;
			}
		}
	}
	return expanded;
}

int parentGroupIdForLayer(canvas::LayerListModel *layers, int layerId)
{
	if(!layers || layerId <= 0) {
		return 0;
	}
	const QModelIndex layerIndex = layers->layerIndex(layerId);
	if(!layerIndex.isValid()) {
		return 0;
	}
	const QModelIndex parentIndex = layerIndex.parent();
	return parentIndex.isValid()
			   ? parentIndex.data(canvas::LayerListModel::IdRole).toInt()
			   : 0;
}

QVector<int> visibleChildLayerIds(canvas::LayerListModel *layers, int groupId)
{
	QVector<int> ids;
	if(!layers || groupId <= 0) {
		return ids;
	}
	int groupLeft = -1;
	int groupRight = -1;
	for(const canvas::LayerListItem &item : layers->layerItems()) {
		if(item.id == groupId && item.group) {
			groupLeft = item.left;
			groupRight = item.right;
			break;
		}
	}
	if(groupLeft < 0 || groupRight <= groupLeft) {
		return ids;
	}
	for(const canvas::LayerListItem &item : layers->layerItems()) {
		if(!item.group && !item.hidden && item.opacity > 0.0f &&
		   item.left > groupLeft && item.right < groupRight) {
			ids.append(item.id);
		}
	}
	return ids;
}

QImage combinedLayerAlphaImage(
	canvas::CanvasModel *canvas, const QVector<int> &layerIds,
	const QRect &canvasBounds)
{
	if(!canvas || layerIds.isEmpty() || canvasBounds.isEmpty()) {
		return QImage();
	}
	QImage combined(canvasBounds.size(), QImage::Format_ARGB32);
	combined.fill(Qt::transparent);
	for(int layerId : layerIds) {
		const QImage layerImage =
			canvas->paintEngine()->getLayerImage(layerId, canvasBounds);
		if(layerImage.isNull()) {
			continue;
		}
		const QImage source = layerImage.convertToFormat(QImage::Format_ARGB32);
		for(int y = 0; y < source.height(); ++y) {
			const QRgb *in =
				reinterpret_cast<const QRgb *>(source.constScanLine(y));
			QRgb *out = reinterpret_cast<QRgb *>(combined.scanLine(y));
			for(int x = 0; x < source.width(); ++x) {
				const int alpha = qMax(qAlpha(out[x]), qAlpha(in[x]));
				if(alpha > 0) {
					out[x] = qRgba(255, 255, 255, alpha);
				}
			}
		}
	}
	return combined;
}

QString underpaintToolPath(const QString &relativePath)
{
	const QString envPath = QProcessEnvironment::systemEnvironment().value(
		QStringLiteral("UNDERPAINT_TOOL_ROOT"));
	if(!envPath.isEmpty()) {
		const QString path = QDir(envPath).filePath(relativePath);
		if(QFile::exists(path)) {
			return path;
		}
	}

	const QString currentPath = QDir::current().filePath(relativePath);
	if(QFile::exists(currentPath)) {
		return currentPath;
	}

	const QString appPath =
		QDir(QApplication::applicationDirPath()).filePath(
			QStringLiteral("../../%1").arg(relativePath));
	if(QFile::exists(appPath)) {
		return appPath;
	}

	return currentPath;
}

QString underpaintPythonPath()
{
	const QString envPath = QProcessEnvironment::systemEnvironment().value(
		QStringLiteral("UNDERPAINT_PYTHON"));
	if(!envPath.isEmpty()) {
		return envPath;
	}

	const QString venvPath =
		underpaintToolPath(QStringLiteral(".venv/bin/python"));
	if(QFile::exists(venvPath)) {
		return venvPath;
	}

	return QStringLiteral("python3");
}

QString improveInpaintPromptWithHelper(
	const QString &operation, const QRect &region, const QString &prompt,
	const QString &negativePrompt, int candidateCount, int seed, double cfg,
	double denoise, int steps, int edgeFeatherPx, QString &outError,
	const QImage &selectionImage = QImage())
{
	const QString scriptPath =
		QProcessEnvironment::systemEnvironment().value(
			QStringLiteral("UNDERPAINT_PROMPT_HELPER"),
			underpaintToolPath(
				QStringLiteral("tools/ai/underpaint-prompt-helper.py")));
	if(!QFile::exists(scriptPath)) {
		outError = MainWindow::tr("Prompt helper was not found: %1").arg(scriptPath);
		return QString();
	}

	QJsonObject payload{
		{QStringLiteral("operation"), operation},
		{QStringLiteral("prompt"), prompt},
		{QStringLiteral("negativePrompt"), negativePrompt},
		{QStringLiteral("candidateCount"), candidateCount},
		{QStringLiteral("seed"), seed},
		{QStringLiteral("cfg"), cfg},
		{QStringLiteral("denoise"), denoise},
		{QStringLiteral("steps"), steps},
		{QStringLiteral("edgeFeatherPx"), edgeFeatherPx},
		{QStringLiteral("selection"),
		 QJsonObject{
			 {QStringLiteral("width"), region.width()},
			 {QStringLiteral("height"), region.height()},
		 }},
	};

	QTemporaryFile selectionImageFile(
		QDir::temp().filePath(QStringLiteral("underpaint-helper-region-XXXXXX.png")));
	if(!selectionImage.isNull()) {
		if(!selectionImageFile.open()) {
			outError = selectionImageFile.errorString();
			return QString();
		}
		if(!selectionImage.save(&selectionImageFile, "PNG")) {
			outError = MainWindow::tr("Could not write prompt helper image.");
			return QString();
		}
		selectionImageFile.flush();
		payload.insert(
			QStringLiteral("imagePath"), selectionImageFile.fileName());
	}

	QProcess process;
	process.setProgram(underpaintPythonPath());
	process.setArguments({scriptPath});
	process.setProcessChannelMode(QProcess::SeparateChannels);
	process.start();
	if(!process.waitForStarted(3000)) {
		outError = process.errorString();
		return QString();
	}
	process.write(QJsonDocument(payload).toJson(QJsonDocument::Compact));
	process.closeWriteChannel();
	if(!process.waitForFinished(15000)) {
		process.kill();
		process.waitForFinished(1000);
		outError = MainWindow::tr("Prompt helper timed out.");
		return QString();
	}

	const QByteArray stderrBytes = process.readAllStandardError();
	QJsonParseError parseError;
	const QJsonDocument document = QJsonDocument::fromJson(
		process.readAllStandardOutput(), &parseError);
	if(process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
		outError = QString::fromUtf8(stderrBytes).trimmed();
		if(outError.isEmpty()) {
			outError = MainWindow::tr("Prompt helper exited with code %1.")
						   .arg(process.exitCode());
		}
		return QString();
	}
	if(parseError.error != QJsonParseError::NoError || !document.isObject()) {
		outError = MainWindow::tr("Prompt helper returned invalid JSON.");
		return QString();
	}

	const QJsonObject response = document.object();
	if(!response.value(QStringLiteral("ok")).toBool()) {
		outError = response.value(QStringLiteral("error"))
					   .toString(MainWindow::tr("Prompt helper failed."));
		return QString();
	}
	const QString improved =
		response.value(QStringLiteral("prompt")).toString().trimmed();
	if(improved.isEmpty()) {
		outError = MainWindow::tr("Prompt helper returned an empty prompt.");
		return QString();
	}
	return improved;
}

QString cleanLayerNameFromHelper(const QString &text)
{
	QString name = text.simplified().trimmed();
	static const QStringList prefixes = {
		QStringLiteral("layer name:"),
		QStringLiteral("name:"),
		QStringLiteral("label:"),
	};
	const QString lower = name.toLower();
	for(const QString &prefix : prefixes) {
		if(lower.startsWith(prefix)) {
			name = name.mid(prefix.size()).simplified().trimmed();
			break;
		}
	}
	name = name.trimmed();
	while(name.startsWith(QLatin1Char('"')) || name.startsWith(QLatin1Char('\'')) ||
		  name.startsWith(QLatin1Char('-')) || name.startsWith(QLatin1Char('*'))) {
		name.remove(0, 1);
		name = name.trimmed();
	}
	while(name.endsWith(QLatin1Char('"')) || name.endsWith(QLatin1Char('\'')) ||
		  name.endsWith(QLatin1Char('.'))) {
		name.chop(1);
		name = name.trimmed();
	}
	if(name.size() > 48) {
		name = name.left(48).trimmed();
	}
	return name;
}

QString labelDecompositionRegionWithHelper(
	const ai::JobCandidate &candidate, QString &outError)
{
	const QString scriptPath =
		QProcessEnvironment::systemEnvironment().value(
			QStringLiteral("UNDERPAINT_PROMPT_HELPER"),
			underpaintToolPath(
				QStringLiteral("tools/ai/underpaint-prompt-helper.py")));
	if(!QFile::exists(scriptPath)) {
		outError = MainWindow::tr("Prompt helper was not found: %1").arg(scriptPath);
		return QString();
	}
	if(candidate.imagePath.isEmpty()) {
		outError = MainWindow::tr("Candidate has no image path.");
		return QString();
	}

	QJsonObject payload{
		{QStringLiteral("operation"), QStringLiteral("decomposition-region-label")},
		{QStringLiteral("prompt"), candidate.label},
		{QStringLiteral("imagePath"), candidate.imagePath},
		{QStringLiteral("groupLabel"),
		 candidate.metadata.value(QStringLiteral("groupLabel")).toString()},
		{QStringLiteral("regionIndex"),
		 candidate.metadata.value(QStringLiteral("regionIndex")).toInt()},
		{QStringLiteral("regionCount"),
		 candidate.metadata.value(QStringLiteral("regionCount")).toInt()},
	};

	QProcess process;
	process.setProgram(underpaintPythonPath());
	process.setArguments({scriptPath});
	process.setProcessChannelMode(QProcess::SeparateChannels);
	process.start();
	if(!process.waitForStarted(3000)) {
		outError = process.errorString();
		return QString();
	}
	process.write(QJsonDocument(payload).toJson(QJsonDocument::Compact));
	process.closeWriteChannel();
	if(!process.waitForFinished(60000)) {
		process.kill();
		process.waitForFinished(1000);
		outError = MainWindow::tr("Prompt helper timed out.");
		return QString();
	}

	const QByteArray stderrBytes = process.readAllStandardError();
	QJsonParseError parseError;
	const QJsonDocument document = QJsonDocument::fromJson(
		process.readAllStandardOutput(), &parseError);
	if(process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
		outError = QString::fromUtf8(stderrBytes).trimmed();
		if(outError.isEmpty()) {
			outError = MainWindow::tr("Prompt helper exited with code %1.")
						   .arg(process.exitCode());
		}
		return QString();
	}
	if(parseError.error != QJsonParseError::NoError || !document.isObject()) {
		outError = MainWindow::tr("Prompt helper returned invalid JSON.");
		return QString();
	}

	const QJsonObject response = document.object();
	if(!response.value(QStringLiteral("ok")).toBool()) {
		outError = response.value(QStringLiteral("error"))
					   .toString(MainWindow::tr("Prompt helper failed."));
		return QString();
	}
	const QString label =
		cleanLayerNameFromHelper(response.value(QStringLiteral("prompt")).toString());
	if(label.isEmpty()) {
		outError = MainWindow::tr("Prompt helper returned an empty label.");
		return QString();
	}
	return label;
}

bool decompositionSemanticHelperEnabled()
{
	const QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
	return !env.value(QStringLiteral("UNDERPAINT_PROMPT_HELPER_URL")).isEmpty() ||
		   !env.value(QStringLiteral("QWENCH_OPENAI_URL")).isEmpty() ||
		   !env.value(QStringLiteral("OPENAI_COMPAT_URL")).isEmpty() ||
		   env.value(QStringLiteral("UNDERPAINT_USE_QWENCH_PROMPT_HELPER")) ==
			   QStringLiteral("1");
}

bool checkDecompositionSemanticHelper(QString &outError)
{
	const QString scriptPath =
		QProcessEnvironment::systemEnvironment().value(
			QStringLiteral("UNDERPAINT_PROMPT_HELPER"),
			underpaintToolPath(
				QStringLiteral("tools/ai/underpaint-prompt-helper.py")));
	if(!QFile::exists(scriptPath)) {
		outError = MainWindow::tr("Prompt helper was not found: %1").arg(scriptPath);
		return false;
	}

	QProcess process;
	process.setProgram(underpaintPythonPath());
	process.setArguments({scriptPath});
	process.setProcessChannelMode(QProcess::SeparateChannels);
	process.start();
	if(!process.waitForStarted(3000)) {
		outError = process.errorString();
		return false;
	}
	QJsonObject payload{
		{QStringLiteral("operation"), QStringLiteral("helper-health-check")},
	};
	process.write(QJsonDocument(payload).toJson(QJsonDocument::Compact));
	process.closeWriteChannel();
	if(!process.waitForFinished(8000)) {
		process.kill();
		process.waitForFinished(1000);
		outError = MainWindow::tr("Prompt helper health check timed out.");
		return false;
	}

	const QByteArray stderrBytes = process.readAllStandardError();
	QJsonParseError parseError;
	const QJsonDocument document = QJsonDocument::fromJson(
		process.readAllStandardOutput(), &parseError);
	if(process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
		outError = QString::fromUtf8(stderrBytes).trimmed();
		if(outError.isEmpty()) {
			outError = MainWindow::tr("Prompt helper exited with code %1.")
						   .arg(process.exitCode());
		}
		return false;
	}
	if(parseError.error != QJsonParseError::NoError || !document.isObject()) {
		outError = MainWindow::tr("Prompt helper returned invalid JSON.");
		return false;
	}

	const QJsonObject response = document.object();
	if(!response.value(QStringLiteral("ok")).toBool()) {
		outError = response.value(QStringLiteral("error"))
					   .toString(MainWindow::tr("Prompt helper is unavailable."));
		return false;
	}
	return true;
}

QString fallbackSemanticGroup(
	const QString &depthRole, const QString &sceneRole, const QString &repairRole)
{
	if(depthRole == QStringLiteral("sky-horizon")) {
		return MainWindow::tr("Sky / Horizon");
	}
	if(repairRole == QStringLiteral("keep-context") ||
	   depthRole == QStringLiteral("background")) {
		return MainWindow::tr("Background Context");
	}
	if(sceneRole == QStringLiteral("subject")) {
		return MainWindow::tr("Foreground Subjects");
	}
	if(sceneRole == QStringLiteral("prop")) {
		return MainWindow::tr("Foreground Props");
	}
	if(sceneRole == QStringLiteral("structure")) {
		return MainWindow::tr("Foreground Structure");
	}
	return MainWindow::tr("Foreground Objects");
}

QString normalizedSemanticChoice(
	const QJsonObject &object, const QString &key, const QStringList &allowed,
	const QString &fallback)
{
	const QString value = object.value(key).toString().trimmed().toLower();
	return allowed.contains(value) ? value : fallback;
}

QString cleanPromptPhrase(const QString &text, const QString &fallback)
{
	QString phrase = text.simplified().trimmed();
	if(phrase.isEmpty()) {
		phrase = fallback.simplified().trimmed();
	}
	if(phrase.size() > 120) {
		phrase = phrase.left(120).trimmed();
	}
	return phrase;
}

QJsonObject defaultDecompositionSemantics(const ai::JobCandidate &candidate)
{
	const bool baseRemainder = candidateIsBaseRemainder(candidate);
	const QString name = baseRemainder
							 ? MainWindow::tr("Base Remainder")
							 : cleanLayerNameFromHelper(candidate.label);
	const QString depthRole =
		baseRemainder ? QStringLiteral("background") : QStringLiteral("foreground");
	const QString sceneRole =
		baseRemainder ? QStringLiteral("scenery") : QStringLiteral("object");
	const QString repairRole = baseRemainder ? QStringLiteral("keep-context")
											 : QStringLiteral("remove-from-base");
	return QJsonObject{
		{QStringLiteral("name"),
		 name.isEmpty() ? QStringLiteral("Object") : name},
		{QStringLiteral("depthRole"), depthRole},
		{QStringLiteral("sceneRole"), sceneRole},
		{QStringLiteral("repairRole"), repairRole},
		{QStringLiteral("group"),
		 baseRemainder ? MainWindow::tr("Base Remainder")
					   : fallbackSemanticGroup(depthRole, sceneRole, repairRole)},
		{QStringLiteral("promptPhrase"),
		 cleanPromptPhrase(candidate.label, name)},
		{QStringLiteral("confidence"), 0.0},
	};
}

QJsonObject normalizeDecompositionSemantics(
	const ai::JobCandidate &candidate, const QJsonObject &raw)
{
	QJsonObject defaults = defaultDecompositionSemantics(candidate);
	const QString name = cleanLayerNameFromHelper(
		raw.value(QStringLiteral("name")).toString(
			defaults.value(QStringLiteral("name")).toString()));
	const QString depthRole = normalizedSemanticChoice(
		raw, QStringLiteral("depthRole"),
		{QStringLiteral("foreground"), QStringLiteral("midground"),
		 QStringLiteral("background"), QStringLiteral("sky-horizon"),
		 QStringLiteral("ambiguous")},
		defaults.value(QStringLiteral("depthRole")).toString());
	const QString sceneRole = raw.value(QStringLiteral("sceneRole"))
								  .toString(
									  defaults.value(QStringLiteral("sceneRole"))
										  .toString())
								  .trimmed()
								  .toLower();
	QString repairRole = normalizedSemanticChoice(
		raw, QStringLiteral("repairRole"),
		{QStringLiteral("keep-context"), QStringLiteral("remove-from-base"),
		 QStringLiteral("ambiguous")},
		defaults.value(QStringLiteral("repairRole")).toString());
	if(repairRole == QStringLiteral("ambiguous")) {
		repairRole =
			(depthRole == QStringLiteral("background") ||
			 depthRole == QStringLiteral("sky-horizon"))
				? QStringLiteral("keep-context")
				: QStringLiteral("remove-from-base");
	}
	const QString group =
		raw.value(QStringLiteral("group")).toString().simplified().trimmed();
	const QString promptPhrase = cleanPromptPhrase(
		raw.value(QStringLiteral("promptPhrase")).toString(), name);
	const double confidence =
		qBound(0.0, raw.value(QStringLiteral("confidence")).toDouble(0.0), 1.0);
	return QJsonObject{
		{QStringLiteral("name"),
		 name.isEmpty() ? defaults.value(QStringLiteral("name")).toString() : name},
		{QStringLiteral("depthRole"), depthRole},
		{QStringLiteral("sceneRole"),
		 sceneRole.isEmpty() ? QStringLiteral("ambiguous") : sceneRole},
		{QStringLiteral("repairRole"), repairRole},
		{QStringLiteral("group"),
		 group.isEmpty() ? fallbackSemanticGroup(depthRole, sceneRole, repairRole)
						 : group},
		{QStringLiteral("promptPhrase"), promptPhrase},
		{QStringLiteral("confidence"), confidence},
	};
}

void applyDecompositionSemantics(
	ai::JobCandidate &candidate, const QJsonObject &semantics,
	const QString &status)
{
	QJsonObject metadata = candidate.metadata;
	metadata.insert(
		QStringLiteral("semanticName"),
		semantics.value(QStringLiteral("name")).toString());
	metadata.insert(
		QStringLiteral("depthRole"),
		semantics.value(QStringLiteral("depthRole")).toString());
	metadata.insert(
		QStringLiteral("sceneRole"),
		semantics.value(QStringLiteral("sceneRole")).toString());
	metadata.insert(
		QStringLiteral("repairRole"),
		semantics.value(QStringLiteral("repairRole")).toString());
	metadata.insert(
		QStringLiteral("promptPhrase"),
		semantics.value(QStringLiteral("promptPhrase")).toString());
	metadata.insert(
		QStringLiteral("semanticConfidence"),
		semantics.value(QStringLiteral("confidence")).toDouble());
	metadata.insert(QStringLiteral("helperStatus"), status);
	const QString group = semantics.value(QStringLiteral("group")).toString();
	if(!group.isEmpty()) {
		metadata.insert(QStringLiteral("groupLabel"), group);
	}
	candidate.metadata = metadata;
	const QString name = semantics.value(QStringLiteral("name")).toString();
	if(!name.isEmpty()) {
		candidate.label = name;
	}
}

QJsonObject classifyDecompositionRegionWithHelper(
	const ai::JobCandidate &candidate, const QString &sourceImagePath,
	QString &outError)
{
	if(!decompositionSemanticHelperEnabled()) {
		outError = MainWindow::tr("Vision helper is not configured.");
		return QJsonObject();
	}
	const QString scriptPath =
		QProcessEnvironment::systemEnvironment().value(
			QStringLiteral("UNDERPAINT_PROMPT_HELPER"),
			underpaintToolPath(
				QStringLiteral("tools/ai/underpaint-prompt-helper.py")));
	if(!QFile::exists(scriptPath)) {
		outError = MainWindow::tr("Prompt helper was not found: %1").arg(scriptPath);
		return QJsonObject();
	}
	if(candidate.imagePath.isEmpty()) {
		outError = MainWindow::tr("Candidate has no image path.");
		return QJsonObject();
	}

	QJsonObject payload{
		{QStringLiteral("operation"),
		 QStringLiteral("decomposition-region-classify")},
		{QStringLiteral("id"), candidate.id},
		{QStringLiteral("prompt"), candidate.label},
		{QStringLiteral("sourceImagePath"), sourceImagePath},
		{QStringLiteral("imagePath"), candidate.imagePath},
		{QStringLiteral("groupLabel"),
		 candidate.metadata.value(QStringLiteral("groupLabel")).toString()},
		{QStringLiteral("regionIndex"),
		 candidate.metadata.value(QStringLiteral("regionIndex")).toInt()},
		{QStringLiteral("regionCount"),
		 candidate.metadata.value(QStringLiteral("regionCount")).toInt()},
	};

	QProcess process;
	process.setProgram(underpaintPythonPath());
	process.setArguments({scriptPath});
	process.setProcessChannelMode(QProcess::SeparateChannels);
	process.start();
	if(!process.waitForStarted(3000)) {
		outError = process.errorString();
		return QJsonObject();
	}
	process.write(QJsonDocument(payload).toJson(QJsonDocument::Compact));
	process.closeWriteChannel();
	if(!process.waitForFinished(60000)) {
		process.kill();
		process.waitForFinished(1000);
		outError = MainWindow::tr("Prompt helper timed out.");
		return QJsonObject();
	}

	const QByteArray stderrBytes = process.readAllStandardError();
	QJsonParseError parseError;
	const QJsonDocument document = QJsonDocument::fromJson(
		process.readAllStandardOutput(), &parseError);
	if(process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
		outError = QString::fromUtf8(stderrBytes).trimmed();
		if(outError.isEmpty()) {
			outError = MainWindow::tr("Prompt helper exited with code %1.")
						   .arg(process.exitCode());
		}
		return QJsonObject();
	}
	if(parseError.error != QJsonParseError::NoError || !document.isObject()) {
		outError = MainWindow::tr("Prompt helper returned invalid JSON.");
		return QJsonObject();
	}

	const QJsonObject response = document.object();
	if(!response.value(QStringLiteral("ok")).toBool()) {
		outError = response.value(QStringLiteral("error"))
					   .toString(MainWindow::tr("Prompt helper failed."));
		return QJsonObject();
	}
	const QJsonObject classification =
		response.value(QStringLiteral("classification")).toObject();
	if(classification.isEmpty()) {
		outError = MainWindow::tr("Prompt helper returned no classification.");
		return QJsonObject();
	}
	return normalizeDecompositionSemantics(candidate, classification);
}

QJsonArray decompositionGroupRefinementRegions(
	const QVector<ai::JobCandidate> &candidates)
{
	QJsonArray regions;
	for(const ai::JobCandidate &candidate : candidates) {
		if(!candidateIsExtractedObject(candidate)) {
			continue;
		}
		const QJsonObject metadata = candidate.metadata;
		regions.append(QJsonObject{
			{QStringLiteral("id"), candidate.id},
			{QStringLiteral("candidateId"), candidate.id},
			{QStringLiteral("regionIndex"),
			 metadata.value(QStringLiteral("regionIndex")).toInt()},
			{QStringLiteral("name"), candidate.label},
			{QStringLiteral("group"),
			 metadata.value(QStringLiteral("groupLabel")).toString()},
			{QStringLiteral("depthRole"),
			 metadata.value(QStringLiteral("depthRole")).toString()},
			{QStringLiteral("sceneRole"),
			 metadata.value(QStringLiteral("sceneRole")).toString()},
			{QStringLiteral("repairRole"),
			 metadata.value(QStringLiteral("repairRole")).toString()},
			{QStringLiteral("promptPhrase"),
			 metadata.value(QStringLiteral("promptPhrase")).toString()},
			{QStringLiteral("areaRatio"),
			 metadata.value(QStringLiteral("areaRatio")).toDouble()},
			{QStringLiteral("bounds"), metadata.value(QStringLiteral("bounds"))},
		});
	}
	return regions;
}

int applyDecompositionGroupRefinements(
	QVector<ai::JobCandidate> &candidates, const QJsonArray &regions)
{
	int updated = 0;
	for(const QJsonValue &value : regions) {
		if(!value.isObject()) {
			continue;
		}
		const QJsonObject item = value.toObject();
		const QString id =
			item.value(QStringLiteral("id")).toString().simplified().trimmed();
		const int regionIndex =
			item.value(QStringLiteral("regionIndex")).toInt(-1);
		if(id.isEmpty() && regionIndex < 0) {
			continue;
		}
		const QString name =
			cleanLayerNameFromHelper(item.value(QStringLiteral("name")).toString());
		const QString group =
			cleanLayerNameFromHelper(item.value(QStringLiteral("group")).toString());
		const QString promptPhrase =
			cleanPromptPhrase(item.value(QStringLiteral("promptPhrase")).toString(), name);
		for(ai::JobCandidate &candidate : candidates) {
			if(!candidateIsExtractedObject(candidate)) {
				continue;
			}
			const bool idMatches = !id.isEmpty() && candidate.id == id;
			const bool indexMatches =
				id.isEmpty() &&
				candidate.metadata.value(QStringLiteral("regionIndex")).toInt(-1) ==
					regionIndex;
			if(!idMatches && !indexMatches) {
				continue;
			}
			bool changed = false;
			QJsonObject metadata = candidate.metadata;
			const bool mayRename =
				metadata.value(QStringLiteral("helperStatus")).toString() !=
					QStringLiteral("semantic-helper") ||
				candidate.label.startsWith(QStringLiteral("Object "));
			if(mayRename && !name.isEmpty() && name != candidate.label) {
				candidate.label = name;
				metadata.insert(QStringLiteral("semanticName"), name);
				changed = true;
			}
			if(!group.isEmpty() &&
			   metadata.value(QStringLiteral("groupLabel")).toString() != group) {
				metadata.insert(QStringLiteral("groupLabel"), group);
				metadata.insert(QStringLiteral("semanticGroupRefined"), true);
				changed = true;
			}
			if(!promptPhrase.isEmpty() &&
			   metadata.value(QStringLiteral("promptPhrase")).toString() !=
				   promptPhrase) {
				metadata.insert(QStringLiteral("promptPhrase"), promptPhrase);
				changed = true;
			}
			if(changed) {
				candidate.metadata = metadata;
				++updated;
			}
			break;
		}
	}
	return updated;
}

int refineDecompositionGroupsWithHelper(
	QVector<ai::JobCandidate> &candidates, const QString &sourceImagePath,
	QString &outError)
{
	const QJsonArray regions = decompositionGroupRefinementRegions(candidates);
	if(regions.isEmpty()) {
		return 0;
	}
	const QString scriptPath =
		QProcessEnvironment::systemEnvironment().value(
			QStringLiteral("UNDERPAINT_PROMPT_HELPER"),
			underpaintToolPath(
				QStringLiteral("tools/ai/underpaint-prompt-helper.py")));
	if(!QFile::exists(scriptPath)) {
		outError = MainWindow::tr("Prompt helper was not found: %1").arg(scriptPath);
		return 0;
	}

	QJsonObject payload{
		{QStringLiteral("operation"),
		 QStringLiteral("decomposition-group-refine")},
		{QStringLiteral("sourceImagePath"), sourceImagePath},
		{QStringLiteral("regions"), regions},
	};

	QProcess process;
	process.setProgram(underpaintPythonPath());
	process.setArguments({scriptPath});
	process.setProcessChannelMode(QProcess::SeparateChannels);
	process.start();
	if(!process.waitForStarted(3000)) {
		outError = process.errorString();
		return 0;
	}
	process.write(QJsonDocument(payload).toJson(QJsonDocument::Compact));
	process.closeWriteChannel();
	if(!process.waitForFinished(90000)) {
		process.kill();
		process.waitForFinished(1000);
		outError = MainWindow::tr("Prompt helper group refinement timed out.");
		return 0;
	}

	const QByteArray stderrBytes = process.readAllStandardError();
	QJsonParseError parseError;
	const QJsonDocument document = QJsonDocument::fromJson(
		process.readAllStandardOutput(), &parseError);
	if(process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
		outError = QString::fromUtf8(stderrBytes).trimmed();
		if(outError.isEmpty()) {
			outError = MainWindow::tr("Prompt helper exited with code %1.")
						   .arg(process.exitCode());
		}
		return 0;
	}
	if(parseError.error != QJsonParseError::NoError || !document.isObject()) {
		outError = MainWindow::tr("Prompt helper returned invalid JSON.");
		return 0;
	}

	const QJsonObject response = document.object();
	if(!response.value(QStringLiteral("ok")).toBool()) {
		outError = response.value(QStringLiteral("error"))
					   .toString(MainWindow::tr("Prompt helper failed."));
		return 0;
	}
	return applyDecompositionGroupRefinements(
		candidates, response.value(QStringLiteral("regions")).toArray());
}

bool showInpaintOptionsDialog(
	QWidget *parent, const QRect &region, InpaintOptions &options,
	const QString &windowTitle, const QString &regionLabelText,
	const QString &promptPlaceholder, const QString &promptHelperOperation,
	const std::function<QImage()> &regionImageProvider = std::function<QImage()>())
{
	QDialog dialog(parent);
	dialog.setWindowTitle(windowTitle);

	QVBoxLayout *layout = new QVBoxLayout(&dialog);
	QLabel *regionLabel = new QLabel(
		MainWindow::tr("%1: %2 x %3 px")
			.arg(regionLabelText)
			.arg(region.width())
			.arg(region.height()),
		&dialog);
	layout->addWidget(regionLabel);

	QTextEdit *prompt = new QTextEdit(&dialog);
	prompt->setAcceptRichText(false);
	prompt->setPlainText(options.prompt);
	prompt->setPlaceholderText(promptPlaceholder);
	prompt->setMinimumHeight(90);

	QLineEdit *negativePrompt = new QLineEdit(options.negativePrompt, &dialog);
	negativePrompt->setPlaceholderText(
		MainWindow::tr("Things to avoid, optional"));

	QSpinBox *seed = new QSpinBox(&dialog);
	seed->setRange(-1, INT_MAX);
	seed->setValue(options.seed);
	seed->setSpecialValueText(MainWindow::tr("Random"));

	QSlider *candidateCount = nullptr;
	QWidget *candidateCountSlider = makeIntSlider(
		&dialog, 1, 4, options.candidateCount,
		[](int v) { return QString::number(v); }, candidateCount);
	QSlider *cfg = nullptr;
	QWidget *cfgSlider = makeIntSlider(
		&dialog, 10, 150, int(options.cfg * 10.0),
		[](int v) { return QString::number(v / 10.0, 'f', 1); }, cfg);
	QSlider *denoise = nullptr;
	QWidget *denoiseSlider = makeIntSlider(
		&dialog, 5, 100, int(options.denoise * 100.0),
		[](int v) { return QString::number(v / 100.0, 'f', 2); }, denoise);
	QSlider *steps = nullptr;
	QWidget *stepsSlider = makeIntSlider(
		&dialog, 1, 200, options.steps,
		[](int v) { return QString::number(v); }, steps);
	QSlider *edgeFeather = nullptr;
	QWidget *edgeFeatherSlider = makeIntSlider(
		&dialog, 0, 128, options.edgeFeatherPx,
		[](int v) { return MainWindow::tr("%1 px").arg(v); }, edgeFeather);

	QComboBox *samplerPreset = new QComboBox(&dialog);
	addSamplerPresetChoices(samplerPreset);
	QComboBox *scheduler = new QComboBox(&dialog);
	addSchedulerChoices(scheduler);
	selectScheduler(scheduler, options.scheduler);
	const QString initialPreset =
		options.samplerPreset.isEmpty()
			? samplerPresetIdForValues(
				  options.scheduler, options.cfg, options.denoise, options.steps)
			: options.samplerPreset;
	selectSamplerPreset(samplerPreset, initialPreset);

	QCheckBox *refiner = new QCheckBox(MainWindow::tr("Use SDXL refiner"), &dialog);
	refiner->setChecked(options.refinerEnabled);
	refiner->setToolTip(
		MainWindow::tr("Uses the current Refiner Settings for global polish."));
	QLabel *refinerLabel =
		new QLabel(refinerSummary(loadRefinerOptions()), &dialog);
	refinerLabel->setWordWrap(true);
	QComboBox *refinerPlacement = new QComboBox(&dialog);
	refinerPlacement->addItem(
		MainWindow::tr("Before detailer"), QStringLiteral("before-detail"));
	refinerPlacement->addItem(
		MainWindow::tr("After detailer"), QStringLiteral("after-detail"));
	const int refinerPlacementIndex =
		refinerPlacement->findData(normalizeRefinerPlacement(options.refinerPlacement));
	refinerPlacement->setCurrentIndex(
		refinerPlacementIndex >= 0 ? refinerPlacementIndex : 0);
	refinerPlacement->setToolTip(MainWindow::tr(
		"Before detailer makes face/body/hands the final targeted pass. After "
		"detailer applies one final whole-image refiner pass."));

	QCheckBox *detailPass = new QCheckBox(
		MainWindow::tr("Use face/body detailer"), &dialog);
	detailPass->setChecked(options.detailPassEnabled);
	detailPass->setToolTip(
		MainWindow::tr(
			"Uses the current Face & Body Detail Settings. The detail pass "
			"only renders detected valid regions."));
	QLabel *detailPassLabel =
		new QLabel(detailPassSummary(loadDetailPassOptions()), &dialog);
	detailPassLabel->setWordWrap(true);
	auto updateRefinerPlacementEnabled = [=] {
		refinerPlacement->setEnabled(refiner->isChecked() && detailPass->isChecked());
	};
	QObject::connect(
		refiner, &QCheckBox::toggled, &dialog,
		[updateRefinerPlacementEnabled](bool) { updateRefinerPlacementEnabled(); });
	QObject::connect(
		detailPass, &QCheckBox::toggled, &dialog,
		[updateRefinerPlacementEnabled](bool) { updateRefinerPlacementEnabled(); });
	updateRefinerPlacementEnabled();

	QWidget *promptWidget = new QWidget(&dialog);
	QHBoxLayout *promptLayout = new QHBoxLayout(promptWidget);
	promptLayout->setContentsMargins(0, 0, 0, 0);
	promptLayout->setSpacing(4);
	promptLayout->addWidget(prompt, 1);
	QToolButton *promptHelper = new QToolButton(promptWidget);
	promptHelper->setIcon(QIcon::fromTheme(QStringLiteral("document-edit")));
	promptHelper->setToolTip(
		MainWindow::tr("Rewrite prompt with the local prompt helper"));
	promptHelper->setAutoRaise(true);
	promptHelper->setFocusPolicy(Qt::NoFocus);
	promptLayout->addWidget(promptHelper, 0, Qt::AlignTop);
	QToolButton *explainSelection = new QToolButton(promptWidget);
	explainSelection->setIcon(speechBubbleIcon());
	explainSelection->setToolTip(
		MainWindow::tr("Describe the selected image area with the local helper"));
	explainSelection->setAutoRaise(true);
	explainSelection->setFocusPolicy(Qt::NoFocus);
	explainSelection->setEnabled(bool(regionImageProvider));
	promptLayout->addWidget(explainSelection, 0, Qt::AlignTop);
	QWidget *dialogParent = &dialog;
	QObject::connect(
		promptHelper, &QToolButton::clicked, &dialog,
		[dialogParent, promptHelper, prompt, negativePrompt, candidateCount,
		 seed, cfg, denoise, steps, edgeFeather, &region,
		 promptHelperOperation] {
			QString error;
			promptHelper->setEnabled(false);
			utils::ScopedOverrideCursor cursor;
			const QString improved = improveInpaintPromptWithHelper(
				promptHelperOperation, region, prompt->toPlainText().trimmed(),
				negativePrompt->text().trimmed(), candidateCount->value(),
				seed->value(), cfg->value() / 10.0, denoise->value() / 100.0,
				steps->value(), edgeFeather->value(), error);
			promptHelper->setEnabled(true);
			if(improved.isEmpty()) {
				QMessageBox::warning(
					dialogParent, MainWindow::tr("Prompt Helper"), error);
				return;
			}
			prompt->setPlainText(improved);
			prompt->setFocus();
		});
	QObject::connect(
		explainSelection, &QToolButton::clicked, &dialog,
		[dialogParent, explainSelection, prompt, negativePrompt, candidateCount,
		 seed, cfg, denoise, steps, edgeFeather, &region, regionImageProvider] {
			if(!regionImageProvider) {
				return;
			}
			QString error;
			const QImage image = regionImageProvider();
			if(image.isNull()) {
				QMessageBox::warning(
					dialogParent, MainWindow::tr("Prompt Helper"),
					MainWindow::tr("Could not capture the selected image area."));
				return;
			}
			explainSelection->setEnabled(false);
			utils::ScopedOverrideCursor cursor;
			const QString description = improveInpaintPromptWithHelper(
				QStringLiteral("inpaint-selection-explain"), region,
				prompt->toPlainText().trimmed(), negativePrompt->text().trimmed(),
				candidateCount->value(), seed->value(), cfg->value() / 10.0,
				denoise->value() / 100.0, steps->value(), edgeFeather->value(),
				error, image);
			explainSelection->setEnabled(true);
			if(description.isEmpty()) {
				QMessageBox::warning(
					dialogParent, MainWindow::tr("Prompt Helper"), error);
				return;
			}
			const QString current = prompt->toPlainText().trimmed();
			prompt->setPlainText(
				current.isEmpty()
					? description
					: MainWindow::tr("%1, %2").arg(current, description));
			prompt->setFocus();
		});

	bool applyingSamplerPreset = false;
	auto markSamplerPresetCustom = [&] {
		if(!applyingSamplerPreset) {
			selectSamplerPreset(samplerPreset, QStringLiteral("custom"));
		}
	};
	auto applySamplerPreset = [&] (const QString &id) {
		if(id == QStringLiteral("custom")) {
			return;
		}
		const SamplerPreset preset = samplerPresetById(id);
		applyingSamplerPreset = true;
		selectScheduler(scheduler, preset.scheduler);
		cfg->setValue(qRound(preset.cfg * 10.0));
		denoise->setValue(qRound(preset.denoise * 100.0));
		steps->setValue(preset.steps);
		applyingSamplerPreset = false;
	};
	QObject::connect(
		samplerPreset, QOverload<int>::of(&QComboBox::currentIndexChanged),
		&dialog, [samplerPreset, applySamplerPreset](int) {
			applySamplerPreset(samplerPreset->currentData().toString());
		});
	QObject::connect(
		scheduler, QOverload<int>::of(&QComboBox::currentIndexChanged), &dialog,
		[&] (int) { markSamplerPresetCustom(); });
	QObject::connect(
		cfg, &QSlider::valueChanged, &dialog,
		[&] (int) { markSamplerPresetCustom(); });
	QObject::connect(
		denoise, &QSlider::valueChanged, &dialog,
		[&] (int) { markSamplerPresetCustom(); });
	QObject::connect(
		steps, &QSlider::valueChanged, &dialog,
		[&] (int) { markSamplerPresetCustom(); });

	QFormLayout *form = new QFormLayout;
	form->addRow(MainWindow::tr("Prompt"), promptWidget);
	form->addRow(MainWindow::tr("Negative prompt"), negativePrompt);
	form->addRow(MainWindow::tr("Candidates"), candidateCountSlider);
	form->addRow(MainWindow::tr("Seed"), seed);
	form->addRow(MainWindow::tr("Preset"), samplerPreset);
	form->addRow(MainWindow::tr("CFG"), cfgSlider);
	form->addRow(MainWindow::tr("Denoise"), denoiseSlider);
	form->addRow(MainWindow::tr("Sampler"), scheduler);
	form->addRow(MainWindow::tr("Steps"), stepsSlider);
	form->addRow(MainWindow::tr("Edge feather"), edgeFeatherSlider);
	form->addRow(QString(), refiner);
	form->addRow(MainWindow::tr("Refiner configuration"), refinerLabel);
	form->addRow(MainWindow::tr("Refiner order"), refinerPlacement);
	form->addRow(QString(), detailPass);
	form->addRow(MainWindow::tr("Detailer configuration"), detailPassLabel);
	layout->addLayout(form);

	QDialogButtonBox *buttons = new QDialogButtonBox(
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
	buttons->button(QDialogButtonBox::Ok)->setText(
		MainWindow::tr("Generate"));
	QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
	QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
	layout->addWidget(buttons);

	if(dialog.exec() != QDialog::Accepted) {
		return false;
	}

	options.prompt = prompt->toPlainText().trimmed();
	options.negativePrompt = negativePrompt->text().trimmed();
	options.candidateCount = candidateCount->value();
	options.seed = seed->value();
	options.cfg = cfg->value() / 10.0;
	options.denoise = denoise->value() / 100.0;
	options.scheduler = scheduler->currentData().toString();
	options.samplerPreset = samplerPreset->currentData().toString();
	options.steps = steps->value();
	options.refinerEnabled = refiner->isChecked();
	options.refinerPlacement =
		normalizeRefinerPlacement(refinerPlacement->currentData().toString());
	RefinerOptions refinerOptions = loadRefinerOptions();
	refinerOptions.enabled = options.refinerEnabled;
	refinerOptions.placement = options.refinerPlacement;
	saveRefinerOptions(refinerOptions);
	options.detailPassEnabled = detailPass->isChecked();
	saveDetailPassEnabled(options.detailPassEnabled);
	options.edgeFeatherPx = edgeFeather->value();
	return true;
}

bool showColorSeparationOptionsDialog(
	QWidget *parent, const QSize &canvasSize, ColorSeparationOptions &options)
{
	QDialog dialog(parent);
	dialog.setWindowTitle(MainWindow::tr("Color Separation"));

	QVBoxLayout *layout = new QVBoxLayout(&dialog);
	QLabel *sourceLabel = new QLabel(
		MainWindow::tr("Source: %1 x %2 px")
			.arg(canvasSize.width())
			.arg(canvasSize.height()),
		&dialog);
	layout->addWidget(sourceLabel);

	QSlider *maxRegions = nullptr;
	QWidget *maxRegionsSlider = makeIntSlider(
		&dialog, 2, 200, options.maxRegions,
		[](int v) { return QString::number(v); }, maxRegions);
	QSlider *minRegionArea = nullptr;
	QWidget *minRegionAreaSlider = makeIntSlider(
		&dialog, 1, 20, options.minRegionAreaPct,
		[](int v) { return MainWindow::tr("%1%").arg(v); }, minRegionArea);
	QCheckBox *groupRepeatedRegions = new QCheckBox(&dialog);
	groupRepeatedRegions->setChecked(options.groupRepeatedRegions);

	QFormLayout *form = new QFormLayout;
	form->addRow(MainWindow::tr("Max regions"), maxRegionsSlider);
	form->addRow(MainWindow::tr("Minimum region area"), minRegionAreaSlider);
	form->addRow(MainWindow::tr("Group regions"), groupRepeatedRegions);
	layout->addLayout(form);

	QDialogButtonBox *buttons = new QDialogButtonBox(
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
	buttons->button(QDialogButtonBox::Ok)->setText(
		MainWindow::tr("Separate"));
	QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
	QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
	layout->addWidget(buttons);

	if(dialog.exec() != QDialog::Accepted) {
		return false;
	}

	options.maxRegions = maxRegions->value();
	options.minRegionAreaPct = minRegionArea->value();
	options.groupRepeatedRegions = groupRepeatedRegions->isChecked();
	return true;
}

bool showObjectDecompositionOptionsDialog(
	QWidget *parent, const QSize &canvasSize, ObjectDecompositionOptions &options)
{
	QDialog dialog(parent);
	dialog.setWindowTitle(MainWindow::tr("Object Decomposition"));

	QVBoxLayout *layout = new QVBoxLayout(&dialog);
	QLabel *sourceLabel = new QLabel(
		MainWindow::tr("Source: %1 x %2 px")
			.arg(canvasSize.width())
			.arg(canvasSize.height()),
		&dialog);
	sourceLabel->setWordWrap(true);
	layout->addWidget(sourceLabel);
	QLabel *note = new QLabel(
		MainWindow::tr(
			"Separates foreground subjects, object parts, props, and background "
			"items into movable layers for inspection and editing."),
		&dialog);
	note->setWordWrap(true);
	layout->addWidget(note);

	QComboBox *backend = new QComboBox(&dialog);
	backend->addItem(
		MainWindow::tr("Grid Pass (SAM Base)"), QStringLiteral("sam"));
	backend->addItem(
		MainWindow::tr("Grid Pass (HQ-SAM experimental)"), QStringLiteral("sam-hq"));
	const int backendIndex = backend->findData(options.segmentationBackend);
	backend->setCurrentIndex(backendIndex < 0 ? 0 : backendIndex);
	backend->setToolTip(MainWindow::tr(
		"Chooses the SAM-family backend used by the grid pass that finds "
		"unlabeled pieces after detector-first decomposition."));

	QComboBox *depth = new QComboBox(&dialog);
	depth->addItem(MainWindow::tr("Clean"), QStringLiteral("clean"));
	depth->addItem(MainWindow::tr("Balanced"), QStringLiteral("balanced"));
	depth->addItem(MainWindow::tr("Detailed"), QStringLiteral("detailed"));
	depth->addItem(MainWindow::tr("Exhaustive"), QStringLiteral("exhaustive"));
	const int depthIndex = depth->findData(options.decompositionDepth);
	depth->setCurrentIndex(depthIndex < 0 ? 1 : depthIndex);

	QSlider *maxMasks = nullptr;
	QWidget *maxMasksSlider = makeIntSlider(
		&dialog, 1, 200, options.maxMasks,
		[](int v) { return QString::number(v); }, maxMasks);
	QSlider *minRegionArea = nullptr;
	QWidget *minRegionAreaSlider = makeIntSlider(
		&dialog, 1, 2000, qRound(options.minRegionAreaPct * 100.0),
		[](int v) {
			return MainWindow::tr("%1%")
				.arg(QString::number(v / 100.0, 'f', v < 100 ? 2 : 1));
		},
		minRegionArea);
	QCheckBox *personPriorEnabled = new QCheckBox(
		MainWindow::tr("Find people first"), &dialog);
	personPriorEnabled->setChecked(options.personPriorEnabled);
	personPriorEnabled->setToolTip(MainWindow::tr(
		"Uses the installed ADetailer person detector as a first pass before "
		"SAM-family mask selection. This helps keep people together as "
		"movable decomposition layers."));
	QSlider *personPriorConfidence = nullptr;
	QWidget *personPriorConfidenceSlider = makeIntSlider(
		&dialog, 1, 90, options.personPriorConfidencePct,
		[](int v) {
			return MainWindow::tr("%1%").arg(v);
		},
		personPriorConfidence);
	personPriorConfidenceSlider->setToolTip(MainWindow::tr(
		"Lower values detect more people in crowded scenes. Higher values "
		"reject more uncertain body-shaped fragments."));
	QSlider *personPriorMaxRegions = nullptr;
	QWidget *personPriorMaxRegionsSlider = makeIntSlider(
		&dialog, 1, 128, options.personPriorMaxRegions,
		[](int v) { return QString::number(v); }, personPriorMaxRegions);
	personPriorMaxRegionsSlider->setToolTip(MainWindow::tr(
		"Caps how many person detections can be promoted before the general "
		"SAM mask proposals are ranked."));
	QSlider *personPriorMinArea = nullptr;
	QWidget *personPriorMinAreaSlider = makeIntSlider(
		&dialog, 1, 500, qRound(options.personPriorMinAreaPct * 100.0),
		[](int v) {
			return MainWindow::tr("%1%")
				.arg(QString::number(v / 100.0, 'f', v < 100 ? 2 : 1));
		},
		personPriorMinArea);
	personPriorMinAreaSlider->setToolTip(MainWindow::tr(
		"Minimum person-box size as a percentage of the source image. Crowd "
		"photos need low values so distant figures survive."));
	QCheckBox *objectPriorEnabled = new QCheckBox(
		MainWindow::tr("Find common objects first"), &dialog);
	objectPriorEnabled->setChecked(options.objectPriorEnabled);
	objectPriorEnabled->setToolTip(MainWindow::tr(
		"Uses a general segmentation detector before SAM-family mask fallback. "
		"This is usually cleaner for vehicles, animals, signs, furniture, and "
		"props."));
	QSlider *objectPriorConfidence = nullptr;
	QWidget *objectPriorConfidenceSlider = makeIntSlider(
		&dialog, 1, 90, options.objectPriorConfidencePct,
		[](int v) { return MainWindow::tr("%1%").arg(v); },
		objectPriorConfidence);
	objectPriorConfidenceSlider->setToolTip(MainWindow::tr(
		"Lower values find more objects. Higher values reject uncertain "
		"detector masks."));
	QSlider *objectPriorMaxRegions = nullptr;
	QWidget *objectPriorMaxRegionsSlider = makeIntSlider(
		&dialog, 1, 128, options.objectPriorMaxRegions,
		[](int v) { return QString::number(v); }, objectPriorMaxRegions);
	objectPriorMaxRegionsSlider->setToolTip(MainWindow::tr(
		"Caps how many general object detections can be promoted before "
		"ranking and cleanup."));
	QSlider *objectPriorMinArea = nullptr;
	QWidget *objectPriorMinAreaSlider = makeIntSlider(
		&dialog, 1, 500, qRound(options.objectPriorMinAreaPct * 100.0),
		[](int v) {
			return MainWindow::tr("%1%")
				.arg(QString::number(v / 100.0, 'f', v < 100 ? 2 : 1));
		},
		objectPriorMinArea);
	objectPriorMinAreaSlider->setToolTip(MainWindow::tr(
		"Minimum object-box size as a percentage of the source image. Lower "
		"values keep small props and distant objects."));
	QCheckBox *samGridFallbackEnabled = new QCheckBox(
		MainWindow::tr("Find unlabeled scene pieces"), &dialog);
	samGridFallbackEnabled->setChecked(options.samGridFallbackEnabled);
	samGridFallbackEnabled->setToolTip(MainWindow::tr(
		"Runs a slower SAM-family grid pass after detector proposals so "
		"robots, notes, signs, whiteboard scraps, props, and other unusual "
		"background objects can become editable layers."));
	QCheckBox *groupRepeatedRegions = new QCheckBox(&dialog);
	groupRepeatedRegions->setChecked(options.groupRepeatedRegions);

	QFormLayout *form = new QFormLayout;
	form->addRow(MainWindow::tr("Backend"), backend);
	form->addRow(MainWindow::tr("Depth"), depth);
	form->addRow(MainWindow::tr("Max masks"), maxMasksSlider);
	form->addRow(MainWindow::tr("Minimum region area"), minRegionAreaSlider);
	form->addRow(QString(), personPriorEnabled);
	form->addRow(MainWindow::tr("Person certainty"), personPriorConfidenceSlider);
	form->addRow(MainWindow::tr("Max people"), personPriorMaxRegionsSlider);
	form->addRow(MainWindow::tr("Minimum person area"), personPriorMinAreaSlider);
	form->addRow(QString(), objectPriorEnabled);
	form->addRow(MainWindow::tr("Object certainty"), objectPriorConfidenceSlider);
	form->addRow(MainWindow::tr("Max objects"), objectPriorMaxRegionsSlider);
	form->addRow(MainWindow::tr("Minimum object area"), objectPriorMinAreaSlider);
	form->addRow(QString(), samGridFallbackEnabled);
	form->addRow(MainWindow::tr("Group masks"), groupRepeatedRegions);
	layout->addLayout(form);

	auto updatePersonPriorControls = [=](bool enabled) {
		personPriorConfidenceSlider->setEnabled(enabled);
		personPriorMaxRegionsSlider->setEnabled(enabled);
		personPriorMinAreaSlider->setEnabled(enabled);
	};
	QObject::connect(
		personPriorEnabled, &QCheckBox::toggled, &dialog,
		updatePersonPriorControls);
	updatePersonPriorControls(personPriorEnabled->isChecked());
	auto updateObjectPriorControls = [=](bool enabled) {
		objectPriorConfidenceSlider->setEnabled(enabled);
		objectPriorMaxRegionsSlider->setEnabled(enabled);
		objectPriorMinAreaSlider->setEnabled(enabled);
	};
	QObject::connect(
		objectPriorEnabled, &QCheckBox::toggled, &dialog,
		updateObjectPriorControls);
	updateObjectPriorControls(objectPriorEnabled->isChecked());

	QDialogButtonBox *buttons = new QDialogButtonBox(
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
	buttons->button(QDialogButtonBox::Ok)->setText(
		MainWindow::tr("Decompose"));
	QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
	QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
	layout->addWidget(buttons);

	if(dialog.exec() != QDialog::Accepted) {
		return false;
	}

	options.maxMasks = maxMasks->value();
	options.minRegionAreaPct = minRegionArea->value() / 100.0;
	const QString selectedBackend = backend->currentData().toString();
	options.segmentationBackend = selectedBackend.isEmpty()
									  ? QStringLiteral("sam")
									  : selectedBackend;
	const QString selectedDepth = depth->currentData().toString();
	options.decompositionDepth = selectedDepth.isEmpty() ? QStringLiteral("detailed")
														 : selectedDepth;
	options.personPriorEnabled = personPriorEnabled->isChecked();
	options.personPriorConfidencePct = personPriorConfidence->value();
	options.personPriorMaxRegions = personPriorMaxRegions->value();
	options.personPriorMinAreaPct = personPriorMinArea->value() / 100.0;
	options.objectPriorEnabled = objectPriorEnabled->isChecked();
	options.objectPriorConfidencePct = objectPriorConfidence->value();
	options.objectPriorMaxRegions = objectPriorMaxRegions->value();
	options.objectPriorMinAreaPct = objectPriorMinArea->value() / 100.0;
	options.samGridFallbackEnabled = samGridFallbackEnabled->isChecked();
	options.groupRepeatedRegions = groupRepeatedRegions->isChecked();
	options.repairBase = false;
	return true;
}

void setCandidateLayerVisible(
	canvas::CanvasModel *canvas, const QVector<int> &layerIds, int index)
{
	for(int i = 0; i < layerIds.size(); ++i) {
		canvas->paintEngine()->setLayerVisibility(layerIds.at(i), i != index);
	}
}

QString candidateSeedText(const ai::JobCandidate &candidate)
{
	const QJsonValue seed = candidate.metadata.value(QStringLiteral("seed"));
	if(seed.isDouble()) {
		const int seedValue = seed.toInt(-1);
		if(seedValue >= 0) {
			return QString::number(seedValue);
		}
	}
	const QString seedString = seed.toString().trimmed();
	return seedString == QStringLiteral("-1") ? QString() : seedString;
}

QString candidateDisplayLabel(const ai::JobCandidate &candidate)
{
	const QString label = candidate.label.isEmpty()
							  ? MainWindow::tr("Candidate")
							  : candidate.label;
	const QString seed = candidateSeedText(candidate);
	return seed.isEmpty()
			   ? label
			   : MainWindow::tr("%1 - Seed %2").arg(label, seed);
}

QString candidateLayerLabel(
	const ai::JobCandidate &candidate, const QString &operationName)
{
	const QString seed = candidateSeedText(candidate);
	return seed.isEmpty()
			   ? candidateDisplayLabel(candidate)
			   : MainWindow::tr("%1 %2").arg(operationName, seed);
}

QString candidateLayerLabel(const ai::JobCandidate &candidate)
{
	return candidateLayerLabel(candidate, MainWindow::tr("Inpaint"));
}

QString candidateMetadataText(
	const ai::JobCandidate &candidate, const QString &key,
	const QString &fallback = QString())
{
	const QString value = candidate.metadata.value(key).toString().trimmed();
	return value.isEmpty() ? fallback : value;
}

QString decompositionDepthLabel(const QString &depth)
{
	if(depth == QStringLiteral("clean")) {
		return MainWindow::tr("Clean");
	}
	if(depth == QStringLiteral("detailed")) {
		return MainWindow::tr("Detailed");
	}
	if(depth == QStringLiteral("exhaustive")) {
		return MainWindow::tr("Exhaustive");
	}
	return MainWindow::tr("Balanced");
}

QString candidateRegionSetLabel(const ai::JobRunResult &jobResult)
{
	for(const ai::JobCandidate &candidate : jobResult.response.candidates) {
		if(candidate.metadata.value(QStringLiteral("modelRole")).toString() ==
		   QStringLiteral("color-separation")) {
			return MainWindow::tr("Color Bands");
		}
		const QString label = candidateMetadataText(
			candidate, QStringLiteral("regionSetLabel"));
		if(!label.isEmpty()) {
			return label;
		}
	}
	const QString depth =
		jobResult.response.diagnostics.value(QStringLiteral("decompositionDepth"))
			.toString(QStringLiteral("balanced"));
	return MainWindow::tr("Region Set - %1").arg(decompositionDepthLabel(depth));
}

QString candidateRegionGroupLabel(const ai::JobCandidate &candidate)
{
	if(candidateIsBaseRemainder(candidate)) {
		return MainWindow::tr("Base Remainder");
	}
	return candidateMetadataText(
		candidate, QStringLiteral("groupLabel"),
		MainWindow::tr("Ungrouped Regions"));
}

void appendHideLayerMessages(net::MessageList &messages, const QVector<int> &layerIds)
{
	for(int layerId : layerIds) {
		if(layerId > 0) {
			messages.append(net::makeLocalChangeLayerVisibilityMessage(layerId, true));
		}
	}
}

void hideLayersAfterImport(canvas::CanvasModel *canvas, const QVector<int> &layerIds)
{
	if(!canvas || layerIds.isEmpty()) {
		return;
	}
	QPointer<canvas::CanvasModel> guardedCanvas(canvas);
	auto hideLayers = [guardedCanvas, layerIds] {
		if(!guardedCanvas || !guardedCanvas->paintEngine()) {
			return;
		}
		for(int layerId : layerIds) {
			if(layerId > 0) {
				guardedCanvas->paintEngine()->setLayerVisibility(layerId, true);
			}
		}
	};
	QTimer::singleShot(0, canvas, hideLayers);
	QTimer::singleShot(50, canvas, hideLayers);
	QTimer::singleShot(200, canvas, hideLayers);
}

int validInpaintAnchorLayer(canvas::LayerListModel *layers, int preferredLayerId)
{
	if(!layers) {
		return 0;
	}
	if(preferredLayerId > 0 && layers->layerIndex(preferredLayerId).isValid()) {
		return preferredLayerId;
	}
	const int defaultLayerId = layers->defaultLayer();
	if(defaultLayerId > 0 && layers->layerIndex(defaultLayerId).isValid()) {
		return defaultLayerId;
	}
	for(const canvas::LayerListItem &item : layers->layerItems()) {
		if(!item.group) {
			return item.id;
		}
	}
	return 0;
}

QString candidateDetailsText(
	const ai::JobRunResult &jobResult, const QString &prefix = QString())
{
	const QJsonObject diagnostics = jobResult.response.diagnostics;
	const QJsonObject provenance = jobResult.response.provenance;
	QString details = prefix;
	if(!details.isEmpty()) {
		details += QStringLiteral("\n");
	}
	details += MainWindow::tr("Worker: %1").arg(jobResult.resolvedWorkerPath);
	details += MainWindow::tr("\nBackend: %1")
				   .arg(provenance.value(QStringLiteral("backend"))
							.toString(MainWindow::tr("unknown")));
	const QString model = provenance.value(QStringLiteral("model")).toString();
	if(!model.isEmpty()) {
		details += MainWindow::tr("\nModel: %1").arg(model);
	}
	const QString segmentationBackend =
		provenance.value(QStringLiteral("segmentationBackend"))
			.toString(diagnostics.value(QStringLiteral("segmentationBackend")).toString());
	if(!segmentationBackend.isEmpty()) {
		details += MainWindow::tr("\nSegmentation: %1")
					   .arg(segmentationBackend);
	}
	const QString scheduler =
		diagnostics.value(QStringLiteral("scheduler"))
			.toString(provenance.value(QStringLiteral("scheduler")).toString());
	if(!scheduler.isEmpty()) {
		details += MainWindow::tr("\nSampler: %1").arg(scheduler);
	}
	if(diagnostics.contains(QStringLiteral("steps"))) {
		const int steps = diagnostics.value(QStringLiteral("steps")).toInt();
		const int effectiveSteps =
			diagnostics.value(QStringLiteral("effectiveSteps")).toInt();
		const double denoise =
			diagnostics.value(QStringLiteral("denoise")).toDouble();
		details += MainWindow::tr("\nBase pass: denoise %1, %2 steps")
					   .arg(denoise, 0, 'f', 2)
					   .arg(steps);
		if(effectiveSteps > 0 && effectiveSteps != steps) {
			details += MainWindow::tr(" (~%1 effective)")
						   .arg(effectiveSteps);
		}
	}
	const QString device = diagnostics.value(QStringLiteral("device")).toString();
	if(!device.isEmpty()) {
		details += MainWindow::tr("\nDevice: %1").arg(device);
	}
	if(diagnostics.contains(QStringLiteral("elapsedMsec"))) {
		details += MainWindow::tr("\nElapsed: %1 ms")
					   .arg(diagnostics.value(QStringLiteral("elapsedMsec")).toInt());
	}
	if(diagnostics.contains(QStringLiteral("rejectedMaskCount"))) {
		details += MainWindow::tr("\nRejected masks: %1")
					   .arg(diagnostics.value(QStringLiteral("rejectedMaskCount")).toInt());
	}
		if(diagnostics.contains(QStringLiteral("peakVramMb"))) {
			details += MainWindow::tr("\nPeak VRAM: %1 MB")
						   .arg(diagnostics.value(QStringLiteral("peakVramMb")).toInt());
		}
		const QJsonObject refiner =
			diagnostics.value(QStringLiteral("refiner")).toObject();
		if(!refiner.isEmpty()) {
			const QString status =
				refiner.value(QStringLiteral("status")).toString();
			const QString backend =
				refiner.value(QStringLiteral("backend")).toString();
			details += MainWindow::tr("\nRefiner: %1")
						   .arg(status.isEmpty() ? MainWindow::tr("unknown") : status);
			if(!backend.isEmpty()) {
				details += MainWindow::tr(" (%1)").arg(backend);
			}
			const int refinerSteps =
				refiner.value(QStringLiteral("steps")).toInt();
			const int refinerEffectiveSteps =
				refiner.value(QStringLiteral("effectiveSteps")).toInt();
			const double refinerStrength =
				refiner.value(QStringLiteral("strength")).toDouble();
			if(refinerSteps > 0) {
				details += MainWindow::tr(", strength %1, %2 steps")
							   .arg(refinerStrength, 0, 'f', 2)
							   .arg(refinerSteps);
				if(refinerEffectiveSteps > 0 &&
				   refinerEffectiveSteps != refinerSteps) {
					details += MainWindow::tr(" (~%1 effective)")
								   .arg(refinerEffectiveSteps);
				}
			}
		}
		const QJsonObject detailPass =
			diagnostics.value(QStringLiteral("detailPass")).toObject();
	if(!detailPass.isEmpty()) {
		const QString status =
			detailPass.value(QStringLiteral("status")).toString();
		const QJsonArray regions =
			detailPass.value(QStringLiteral("regions")).toArray();
		QStringList regionNames;
		for(const QJsonValue &region : regions) {
			const QString name = region.toString();
			if(!name.isEmpty()) {
				regionNames.append(name);
			}
		}
		details += MainWindow::tr("\nDetail pass: %1")
					   .arg(status.isEmpty() ? MainWindow::tr("unknown") : status);
		const int detailRenderEdge =
			detailPass.value(QStringLiteral("detailRenderEdge")).toInt();
		if(detailRenderEdge > 0) {
			details += MainWindow::tr(", %1 px edge").arg(detailRenderEdge);
		}
		const int detailSteps =
			detailPass.value(QStringLiteral("steps")).toInt();
		const int detailEffectiveSteps =
			detailPass.value(QStringLiteral("effectiveSteps")).toInt();
		const double detailDenoise =
			detailPass.value(QStringLiteral("denoise")).toDouble();
		if(detailSteps > 0) {
			details += MainWindow::tr(", denoise %1, %2 steps")
						   .arg(detailDenoise, 0, 'f', 2)
						   .arg(detailSteps);
			if(detailEffectiveSteps > 0 && detailEffectiveSteps != detailSteps) {
				details += MainWindow::tr(" (~%1 effective)")
							   .arg(detailEffectiveSteps);
			}
		}
		const int detectedRegions =
			detailPass.value(QStringLiteral("detectedRegions")).toInt();
		const int rawDetectedRegions =
			detailPass.value(QStringLiteral("rawDetectedRegions")).toInt();
		const int rejectedDetections =
			detailPass.value(QStringLiteral("rejectedDetections")).toInt();
		const int selectedRegions =
			detailPass.value(QStringLiteral("selectedRegions")).toInt();
		const int truncatedRegions =
			detailPass.value(QStringLiteral("truncatedRegions")).toInt();
		if(detectedRegions > 0) {
			details += MainWindow::tr(", %1/%2 boxes")
						   .arg(selectedRegions > 0 ? selectedRegions : detectedRegions)
						   .arg(detectedRegions);
			if(truncatedRegions > 0) {
				details += MainWindow::tr(" (%1 skipped)").arg(truncatedRegions);
			}
		}
		if(rejectedDetections > 0) {
			details += MainWindow::tr(", %1 rejected").arg(rejectedDetections);
		} else if(rawDetectedRegions > 0 && detectedRegions == 0) {
			details += MainWindow::tr(", no valid boxes");
		}
		if(!regionNames.isEmpty()) {
			details += MainWindow::tr(" (%1)")
						   .arg(regionNames.join(QStringLiteral(", ")));
		}
	}
	if(!jobResult.response.message.isEmpty()) {
		details += MainWindow::tr("\nMessage: %1").arg(jobResult.response.message);
	}
	details += MainWindow::tr("\nJob directory: %1")
				   .arg(jobResult.jobDirectoryPath);
	for(const ai::JobCandidate &candidate : jobResult.response.candidates) {
		details += MainWindow::tr("\n%1 image: %2")
					   .arg(candidateDisplayLabel(candidate), candidate.imagePath);
	}
	return details;
}

bool showInpaintCandidateDialog(
	QWidget *parent, canvas::CanvasModel *canvas,
	const QVector<int> &importedLayerIds, const ai::JobRunResult &jobResult,
	const QString &windowTitle, const QString &introText)
{
	QDialog dialog(parent);
	dialog.setWindowTitle(windowTitle);
	dialog.resize(760, 520);

	QVBoxLayout *layout = new QVBoxLayout(&dialog);
	QLabel *title = new QLabel(introText, &dialog);
	layout->addWidget(title);

	QListWidget *list = new QListWidget(&dialog);
	list->setViewMode(QListView::IconMode);
	list->setResizeMode(QListView::Adjust);
	list->setMovement(QListView::Static);
	list->setSelectionMode(QAbstractItemView::SingleSelection);
	list->setIconSize(QSize(180, 140));
	list->setSpacing(8);

	for(int i = 0; i < jobResult.response.candidates.size(); ++i) {
		const ai::JobCandidate &candidate = jobResult.response.candidates.at(i);
		QPixmap preview(candidate.imagePath);
		QListWidgetItem *item = new QListWidgetItem(
			QIcon(preview.scaled(
				list->iconSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation)),
			candidateDisplayLabel(candidate));
		item->setToolTip(candidate.imagePath);
		item->setData(Qt::UserRole, i);
		list->addItem(item);
	}
	layout->addWidget(list, 1);

	QLabel *details = new QLabel(
		candidateDetailsText(jobResult, MainWindow::tr("Click a candidate to preview it.")),
		&dialog);
	details->setTextInteractionFlags(Qt::TextSelectableByMouse);
	details->setWordWrap(true);
	layout->addWidget(details);

	QDialogButtonBox *buttons = new QDialogButtonBox(
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
	buttons->button(QDialogButtonBox::Ok)->setText(MainWindow::tr("Use Candidate"));
	QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
	QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
	layout->addWidget(buttons);

	QObject::connect(
		list, &QListWidget::currentRowChanged, &dialog,
		[canvas, importedLayerIds](int row) {
			if(row >= 0 && row < importedLayerIds.size()) {
				setCandidateLayerVisible(canvas, importedLayerIds, row);
			}
		});
	list->setCurrentRow(0);
	return dialog.exec() == QDialog::Accepted;
}

struct ReasonSetter {
	using Reason = view::Lock::Reason;
	QFlags<Reason> allReasons = Reason::None;
	QFlags<Reason> activeReasons = Reason::None;

	void setReason(Reason reason, bool active = true)
	{
		allReasons.setFlag(reason);
		if(active) {
			activeReasons.setFlag(reason);
		}
	}

	void setReasons(QFlags<Reason> reasons, bool active = true)
	{
		allReasons |= reasons;
		if(active) {
			activeReasons |= reasons;
		}
	}
};
}

void MainWindow::updateLockState()
{
	using Reason = view::Lock::Reason;
	ReasonSetter reasons;
	canvas::CanvasModel *canvas = m_doc->canvas();
	canvas::AclState *aclState = canvas ? canvas->aclState() : nullptr;

	if(m_doc->isSessionOutOfSpace()) {
		reasons.setReason(Reason::OutOfSpace);
	}

	if(aclState && aclState->isResetLocked()) {
		reasons.setReason(Reason::Reset);
	}

	bool sessionLocked = aclState && aclState->isSessionLocked();
	getAction("locksession")->setChecked(sessionLocked);
	if(!m_notificationsMuted) {
		if(sessionLocked && !m_wasSessionLocked) {
			dpApp().notifications()->trigger(
				this, notification::Event::Locked, tr("Canvas locked"));
		} else if(!sessionLocked && m_wasSessionLocked) {
			dpApp().notifications()->trigger(
				this, notification::Event::Unlocked, tr("Canvas unlocked"));
		}
	}
	m_wasSessionLocked = sessionLocked;

	bool affectsCanvas = m_dockToolSettings->currentToolAffectsCanvas();
	if(sessionLocked) {
		reasons.setReason(Reason::Canvas, affectsCanvas);
		if(m_doc->isPreparingReset()) {
			reasons.setReason(Reason::Reset, affectsCanvas);
		}
	}

	if(aclState && aclState->isLocked(aclState->localUserId())) {
		reasons.setReason(Reason::User, affectsCanvas);
	}

	bool affectsLayer = m_dockToolSettings->currentToolAffectsLayer();
	reasons.setReasons(m_dockLayers->currentLayerLock(), affectsLayer);

	if(m_dockToolSettings->currentToolRequiresFillSource()) {
		reasons.setReasons(m_dockLayers->currentFillSourceLock());
	}

	if(m_dockToolSettings->currentToolRequiresSelection() &&
	   (!canvas || !canvas->selection()->isValid())) {
		reasons.setReason(Reason::NoSelection);
	}

	if(m_dockToolSettings->isCurrentToolLocked()) {
		reasons.setReason(Reason::Tool);
	}

	if(m_viewLock->updateReasons(
		   reasons.activeReasons, reasons.allReasons,
		   int(canvas ? canvas->paintEngine()->viewMode()
					  : DP_VIEW_MODE_NORMAL),
		   aclState && aclState->amOperator(),
		   !parentalcontrols::isLayerUncensoringBlocked())) {
	}

	m_lockStateUpdatePending = false;
}
// clang-format off

void MainWindow::onNsfmChanged(bool nsfm)
{
	if(nsfm && parentalcontrols::level() >= parentalcontrols::Level::Restricted) {
		m_doc->client()->disconnectFromServer();
		showErrorMessage(tr("Session blocked by parental controls"));
	}
}

void MainWindow::onOperatorModeChange(bool op)
{
	m_admintools->setEnabled(op);
	m_serverLogDialog->setOperatorMode(op);
	getAction("gainop")->setEnabled(!op && m_doc->isSessionOpword());
	getAction("sessionundodepthlimit")->setEnabled(op);
}

void MainWindow::onFeatureAccessChange(DP_Feature feature, bool canUse)
{
	switch(feature) {
	case DP_FEATURE_PUT_IMAGE:
		m_dockToolSettings->fillSettings()->setFeatureAccess(canUse);
		m_dockToolSettings->lassoFillSettings()->setFeatureAccess(canUse);
		m_dockToolSettings->gradientSettings()->setFeatureAccess(canUse);
		break;
	case DP_FEATURE_LASER:
		m_dockToolSettings->laserPointerSettings()->setFeatureAccess(canUse);
		break;
	case DP_FEATURE_SLOW_BRUSH:
		m_dockToolSettings->brushSettings()->setSlowModesAllowed(canUse);
		break;
	case DP_FEATURE_TIMELINE:
		m_dockTimeline->setFeatureAccess(canUse);
		break;
	case DP_FEATURE_MYPAINT:
		m_dockToolSettings->brushSettings()->setMyPaintAllowed(canUse);
		break;
	default: break;
	}
	triggerUpdateLockState();
}

// clang-format on
void MainWindow::onFeatureLimitChanged(DP_FeatureLimit featureLimit, int value)
{
	switch(featureLimit) {
	case DP_FEATURE_LIMIT_BRUSH_SIZE:
		m_dockToolSettings->brushSettings()->setBrushSizeLimit(value);
		m_doc->toolCtrl()->setBrushSizeLimit(value);
		break;
	default:
		break;
	}
}
// clang-format off

void MainWindow::onUndoDepthLimitSet(int undoDepthLimit)
{
	QAction *action = getAction("sessionundodepthlimit");
	action->setProperty("undodepthlimit", undoDepthLimit);
	action->setText(tr("Undo Limit... (%1)").arg(undoDepthLimit));
	action->setStatusTip(tr("Change the session's undo limit, current limit is %1.").arg(undoDepthLimit));
}

// clang-format on
#ifndef __EMSCRIPTEN__
void MainWindow::exit()
{
	if(windowState().testFlag(Qt::WindowFullScreen)) {
		toggleFullscreen();
	}
	if(!m_hiddenDockState.isEmpty()) {
		setDocksHidden(false);
	}
	QApplication::processEvents();
	saveSplitterState();
	saveWindowState();

	canvas::CanvasModel *canvas = m_doc->canvas();
	if(canvas) {
		canvas->discardProjectRecording();
	}

	deleteLater();
}
#endif

void MainWindow::showErrorMessage(const QString &message)
{
	showErrorMessageWithDetails(message, QString{});
}
// clang-format off

/**
 * @param message error message
 * @param details error details
 */
void MainWindow::showErrorMessageWithDetails(const QString &message, const QString &details)
{
	if(!message.isEmpty()) {
		utils::showWarning(this, tr("Error"), message, details);
	}
}

// clang-format on
void MainWindow::showElapsedStatusMessage(
	const QString &message, qint64 elapsedMsec)
{
	qint64 minutes = elapsedMsec / 60000LL;
	qint64 seconds = elapsedMsec / 1000LL - minutes * 60LL;
	qint64 milliseconds = elapsedMsec % 1000LL;
	m_viewStatusBar->showMessage(
		message.arg(minutes, 2, 10, QChar('0'))
			.arg(seconds, 2, 10, QChar('0'))
			.arg(milliseconds, 3, 10, QChar('0')),
		4000);
}

void MainWindow::showLoadResultMessage(DP_LoadResult result)
{
	if(result != DP_LOAD_RESULT_SUCCESS) {
		QString message = impex::getLoadResultMessage(result);
		if(impex::shouldIncludeLoadResultDpError(result)) {
			showErrorMessageWithDetails(message, DP_error());
		} else {
			showErrorMessage(message);
		}
	}
}

void MainWindow::showResetImageTooLargeErrorMessage(int maxSize, bool autoReset)
{
	if(autoReset) {
		m_doc->sendLockSession(true);
	}
	QString message =
		autoReset
			? tr("Your canvas contains too much data, the server limit is %1 "
				 "MB. Merge or delete some layers to simplify the canvas.")
			: tr("The canvas you tried to reset to contains too much data, the "
				 "server limit is %1 MB.");
	utils::showWarning(
		this, tr("Reset image too large"),
		message.arg(maxSize / double(1024 * 1024), 0, 'f', 2));
}

void MainWindow::setShowAnnotations(bool show)
{
	m_canvasView->setShowAnnotations(show);
	m_dockToolSettings->annotationSettings()->setAnnotationsShown(show);
	triggerUpdateLockState();
}

void MainWindow::setShowLaserTrails(bool show)
{
	m_canvasView->setShowLaserTrails(show);
	m_dockToolSettings->laserPointerSettings()->setLaserTrailsShown(show);
	triggerUpdateLockState();
}
// clang-format off

/**
 * @brief Enter/leave fullscreen mode
 *
 * Window position and configuration is saved when entering fullscreen mode
 * and restored when leaving. On Android, the window is always fullscreen.
 */
void MainWindow::toggleFullscreen()
{
#if defined(__EMSCRIPTEN__)
	browser::toggleFullscreen();
#elif !defined(SINGLE_MAIN_WINDOW)
	if(windowState().testFlag(Qt::WindowFullScreen)==false) {
		// Save windowed mode state. On macOS, full screen may be triggered
		// behind our backs by the system, so we don't do it for consistency.
#	ifndef Q_OS_MACOS
		m_fullscreenOldGeometry = geometry();
		m_fullscreenOldMaximized = isMaximized();
#	endif
		showFullScreen();
	} else {
		// Restore old state, or just maximize on macOS.
#	ifdef Q_OS_MACOS
		showMaximized();
#	else
		if(m_fullscreenOldMaximized) {
			showMaximized();
		} else {
			showNormal();
			setGeometry(m_fullscreenOldGeometry);
		}
#	endif
	}
#endif
}

void MainWindow::setFreezeDocks(bool freeze)
{
	const auto features = QDockWidget::DockWidgetClosable
		| QDockWidget::DockWidgetMovable
		| QDockWidget::DockWidgetFloatable;

	for(auto *dw : findChildren<QDockWidget*>(QString(), Qt::FindDirectChildrenOnly)) {
		if(freeze)
			dw->setFeatures(dw->features() & ~features);
		else
			dw->setFeatures(dw->features() | features);
	}

	for(QToolBar *tb : findChildren<QToolBar*>(QString(), Qt::FindDirectChildrenOnly)) {
		tb->setMovable(!freeze);
	}

	finishArrangingDocks()->setEnabled(!freeze);
}

// clang-format on
void MainWindow::setDocksHidden(bool hidden)
{
	finishArrangingDocks()->setEnabled(!hidden);
	keepCanvasPosition([this, hidden] {
		m_viewStatusBar->setHidden(hidden);
		QScopedValueRollback<bool> rollback(m_restoringDockState, true);
		if(hidden) {
			m_hiddenDockState = saveState();
			for(QWidget *w : findChildren<QWidget *>(
					QString(), Qt::FindDirectChildrenOnly)) {
				bool shouldHide =
					w->isVisible() &&
					(w->inherits("QDockWidget") || w->inherits("QToolBar"));
				if(shouldHide) {
					w->hide();
				}
			}
			// Force recalculation of the central widget's position. Otherwise
			// this will happen lazily on the next repaint and we can't scroll
			// properly. Doing it this way is clearly a hack, but I can't figure
			// out another way to do it that doesn't introduce flicker or the
			// window resizing.
			restoreState(saveState());
		} else if(!m_hiddenDockState.isEmpty()) {
			restoreState(m_hiddenDockState);
			m_hiddenDockState.clear();
		}
	});
	m_dockToggles->setDisabled(hidden);
}

void MainWindow::setDockArrangeMode(bool arrange)
{
	for(docks::DockBase *dw : findChildren<docks::DockBase *>(
			QString(), Qt::FindDirectChildrenOnly)) {
		dw->setArrangeMode(arrange);
	}
}

QAction *MainWindow::finishArrangingDocks()
{
	QAction *arrangeDocks = getAction("arrangedocks");
	if(arrangeDocks->isChecked()) {
		bool enabled = arrangeDocks->isEnabled();
		if(!enabled) {
			arrangeDocks->setEnabled(true);
		}
		arrangeDocks->trigger();
		if(!enabled) {
			arrangeDocks->setEnabled(false);
		}
	}
	return arrangeDocks;
}

void MainWindow::updateSideTabDocks()
{
	if(m_smallScreenMode || getAction("sidetabdocks")->isChecked()) {
		setDockOptions(dockOptions() | QMainWindow::VerticalTabs);
	} else {
		setDockOptions(dockOptions() & ~QMainWindow::VerticalTabs);
	}
}

void MainWindow::handleHudAction(
	const HudAction &action, const QPoint &globalPos)
{
	switch(action.type) {
	case HudAction::Type::None:
		return;
	case HudAction::Type::ToggleBrush:
	case HudAction::Type::ToggleTimeline:
	case HudAction::Type::ToggleLayer:
	case HudAction::Type::ToggleChat:
		handleToggleAction(action);
		return;
	case HudAction::Type::TriggerAction:
		if(action.action) {
			QTimer::singleShot(0, action.action, &QAction::trigger);
		} else {
			qWarning("handleHudAction: triggered action is null!");
		}
		return;
	case HudAction::Type::TriggerMenu:
		if(action.menu) {
			QTimer::singleShot(0, action.menu, [action, globalPos] {
				action.menu->popup(globalPos);
			});
		} else {
			qWarning("handleHudAction: triggered menu is null!");
		}
		return;
	}
	qWarning("Unknown hud action type %d", int(action.type));
}

void MainWindow::handleToggleAction(const HudAction &action)
{
	utils::ScopedUpdateDisabler disabler(this);
	QScopedValueRollback<bool> rollback(m_updatingInterfaceMode, true);
	keepCanvasPosition([this, action] {
		if(!m_dockToggles->isEnabled()) {
			getAction("hidedocks")->toggle();
		}

		m_viewStatusBar->hide();

		QPair<QWidget *, HudAction::Type> dockActions[] = {
			{m_dockToolSettings, HudAction::Type::ToggleBrush},
			{m_dockBrushPalette, HudAction::Type::ToggleBrush},
			{m_dockTimeline, HudAction::Type::ToggleTimeline},
			{m_dockOnionSkins, HudAction::Type::ToggleTimeline},
			{m_dockNavigator, HudAction::Type::None},
			{m_dockColorSpinner, HudAction::Type::ToggleLayer},
			{m_dockColorSliders, HudAction::Type::ToggleLayer},
			{m_dockColorPalette, HudAction::Type::ToggleLayer},
			{m_dockColorCircle, HudAction::Type::ToggleLayer},
			{m_dockReference, HudAction::Type::ToggleLayer},
			{m_dockLayers, HudAction::Type::ToggleLayer},
			{m_chatbox, HudAction::Type::ToggleChat},
		};
		QVector<QWidget *> docksToShow;

		HudAction::Type type = action.type;
		for(const QPair<QWidget *, HudAction::Type> &p : dockActions) {
			QWidget *dock = p.first;
			bool isActivated =
				type != HudAction::Type::None && type == p.second;
			bool isVisible = dock->isVisible();
			dock->hide();
			bool visible = isActivated ? !isVisible : false;
			if(visible) {
				docksToShow.append(dock);
			}
		}

		for(QWidget *dock : docksToShow) {
			dock->show();
		}
		m_viewStatusBar->setVisible(docksToShow.isEmpty());

		bool chatVisible = m_chatbox->isVisible();
		QAction *togglechat = getAction("togglechat");
		QSignalBlocker blocker{togglechat};
		togglechat->setChecked(chatVisible);
		if(chatVisible) {
			int h = height();
			int top = h / 2;
			m_splitter->setSizes({top, h - top});
		} else {
			m_canvasView->viewWidget()->setFocus();
		}

		// It's ridiculous how hard resizeDocks() resists resizing the docks to
		// the size it's told to. Doing it several times seems to do the trick.
		for(int i = 0; i < 5; ++i) {
			QCoreApplication::processEvents();
			setDefaultDockSizes();
		}

		updateSmallScreenToolBarVisibility();
		refitWindow();
	});
}

void MainWindow::handleCanvasShortcutAction(const QString &name)
{
	QAction *action = searchAction(name);
	if(action) {
		action->trigger();
	} else {
		qWarning("Canvas shortcut action '%s' not found", qUtf8Printable(name));
	}
}

void MainWindow::handleTouchTapAction(int action)
{
	switch(action) {
	case int(view::TouchTapAction::Undo):
		getAction("undo")->trigger();
		break;
	case int(view::TouchTapAction::Redo):
		getAction("redo")->trigger();
		break;
	case int(view::TouchTapAction::HideDocks):
		getAction("hidedocks")->trigger();
		break;
	case int(view::TouchTapAction::ColorPicker):
		if(m_dockToolSettings->currentTool() == tools::Tool::PICKER) {
			m_dockToolSettings->setPreviousTool();
		} else {
			m_dockToolSettings->setTool(tools::Tool::PICKER);
		}
		break;
	case int(view::TouchTapAction::Eraser):
		if(m_dockToolSettings->currentTool() == tools::Tool::ERASER) {
			m_dockToolSettings->setPreviousTool();
		} else {
			m_dockToolSettings->setTool(tools::Tool::ERASER);
		}
		break;
	case int(view::TouchTapAction::EraseMode):
		getAction("currenterasemode")->trigger();
		break;
	case int(view::TouchTapAction::RecolorMode):
		getAction("currentrecolormode")->trigger();
		break;
	case int(view::TouchTapAction::MirrorCanvas):
		getAction(QStringLiteral("viewmirror"))->trigger();
		break;
	case int(view::TouchTapAction::FlipCanvas):
		getAction(QStringLiteral("viewflip"))->trigger();
		break;
	default:
		qWarning("Unknown tap action %d", action);
		break;
	}
}

void MainWindow::setNotificationsMuted(bool muted)
{
	m_notificationsMuted = muted;
}

void MainWindow::setToolState(int toolState)
{
	setDrawingToolsEnabled(toolState != int(tools::ToolState::Busy));
	m_toolStateNormal = toolState == int(tools::ToolState::Normal);
	updateSelectTransformActions();
}

/**
 * User selected a tool
 * @param tool action representing the tool
 */
void MainWindow::selectTool(QAction *tool)
{
	// Note. Actions must be in the same order in the enum and the group
	int idx = m_drawingtools->actions().indexOf(tool);
	Q_ASSERT(idx >= 0);
	Q_ASSERT(idx < int(tools::Tool::Type::_LASTTOOL));
	if(idx >= 0 && idx < int(tools::Tool::Type::_LASTTOOL)) {
		if(m_dockToolSettings->currentTool() == idx) {
			if(dpAppConfig()->getToolToggle())
				m_dockToolSettings->setPreviousTool();
			m_tempToolSwitchShortcut->reset();
		} else {
			m_dockToolSettings->setTool(tools::Tool::Type(idx));
			m_toolChangeTime.start();
		}
	}
}

// clang-format on
void MainWindow::updateTemporaryToolSwitch()
{
	config::Config *cfg = dpAppConfig();
	if(cfg->getTemporaryToolSwitch()) {
		m_temporaryToolSwitchMs = cfg->getTemporaryToolSwitchMs();
	} else {
		m_temporaryToolSwitchMs = -1;
	}
}
// clang-format off

/**
 * @brief Handle tool change
 * @param tool
 */
void MainWindow::toolChanged(tools::Tool::Type tool)
{
	QAction *toolaction = m_drawingtools->actions().at(int(tool));
	toolaction->setChecked(true);

	// When using the annotation tool, highlight all text boxes
	m_canvasView->setShowAnnotationBorders(tool==tools::Tool::ANNOTATION);

	// Show own user marker if laser pointer is selected.
	bool isLaserPointerSelected = tool == tools::Tool::LASERPOINTER;
	m_canvasView->setShowOwnUserMarker(isLaserPointerSelected);

	// Send pointer updates when using the laser pointer
	m_canvasView->setPointerTracking(
		isLaserPointerSelected &&
		m_dockToolSettings->laserPointerSettings()->pointerTracking());

	// Deselect annotation when tool changed
	if(tool != tools::Tool::ANNOTATION)
		m_doc->toolCtrl()->setActiveAnnotation(0);

	m_doc->toolCtrl()->setActiveTool(tool);
	triggerUpdateLockState();
}

// clang-format on
void MainWindow::updateFreehandToolButton(int brushMode)
{
	if(m_freehandButton) {
		QString iconName;
		QString toolTip;
		QString statusTip;
		switch(brushMode) {
		case tools::BrushSettings::EraseMode:
			iconName = QStringLiteral("drawpile_brusherase");
			toolTip = tr("Freehand (erase mode, click to reset)");
			statusTip = tr("Freehand brush tool (erase mode)");
			break;
		case tools::BrushSettings::AlphaLockMode:
			iconName = QStringLiteral("drawpile_brushlock");
			toolTip = tr("Freehand (alpha lock mode, click to reset)");
			statusTip = tr("Freehand brush tool (alpha lock mode)");
			break;
		case tools::BrushSettings::NormalMode:
			iconName = QStringLiteral("draw-brush");
			toolTip = m_freehandAction->toolTip();
			statusTip = m_freehandAction->statusTip();
			break;
		default:
			return; // Eraser slot active, don't mess with the icon.
		}
		m_freehandButton->setIcon(QIcon::fromTheme(iconName));
		m_freehandButton->setToolTip(toolTip);
		m_freehandButton->setStatusTip(statusTip);
	}
}

void MainWindow::handleFreehandToolButtonClicked()
{
	if(m_freehandButton) {
		QSignalBlocker blocker(m_freehandButton);
		if(m_dockToolSettings->currentTool() == tools::Tool::FREEHAND) {
			switch(m_dockToolSettings->brushSettings()->getBrushMode()) {
			case tools::BrushSettings::EraseMode:
			case tools::BrushSettings::AlphaLockMode:
				m_dockToolSettings->brushSettings()->resetBrushMode();
				m_freehandButton->setChecked(m_freehandAction->isChecked());
				return;
			default:
				break;
			}
		}
	}
	m_freehandAction->trigger();
}

void MainWindow::updateSelectTransformActions()
{
	canvas::CanvasModel *canvas = m_doc->canvas();
	canvas::TransformModel *transform = canvas ? canvas->transform() : nullptr;
	bool haveTransform = transform && transform->isActive();
	bool canApplyTransform = haveTransform && transform->isDstQuadValid();
	bool canStampTransform = canApplyTransform && transform->isStampable();
	bool haveSelection = m_toolStateNormal && !haveTransform && canvas &&
						 canvas->selection()->isValid();
	bool haveAnnotation =
		getAction("tooltext")->isChecked() &&
		m_dockToolSettings->annotationSettings()->selected() != 0;
	bool selectionEditActive = m_doc->toolCtrl()->isSelectionEditActive();
	bool compatibilityMode = m_doc->isCompatibilityMode();

#ifdef __EMSCRIPTEN__
	getAction("downloadselection")->setEnabled(haveSelection);
#else
	getAction("saveselection")->setEnabled(haveSelection);
#endif

	getAction("cutlayer")->setEnabled(haveSelection);
	getAction("copylayer")->setEnabled(haveSelection);
	getAction("copyvisible")->setEnabled(haveSelection);
	getAction("copymerged")->setEnabled(haveSelection);
	getAction("paste")->setEnabled(!haveTransform);
	getAction("paste-centered")->setEnabled(!haveTransform);
	getAction("pastefile")->setEnabled(!haveTransform);
	getAction("stamp")->setEnabled(canStampTransform);
	getAction("cleararea")->setEnabled(haveSelection || haveAnnotation);
	getAction("selectall")->setEnabled(!haveTransform);
	getAction("selectnone")->setEnabled(haveSelection || canApplyTransform);
	getAction("selectinvert")->setEnabled(haveSelection);
	getAction("selectlayerbounds")->setEnabled(!haveTransform);
	getAction("selectlayercontents")->setEnabled(!haveTransform);
	getAction("selectalter")->setEnabled(haveSelection && !haveTransform);
	getAction("fillfgarea")->setEnabled(haveSelection);
	getAction("recolorarea")->setEnabled(haveSelection);
	getAction("colorerasearea")->setEnabled(haveSelection);
	getAction("lightnesstoalphaarea")
		->setEnabled(haveSelection && !compatibilityMode);
	getAction("darknesstoalphaarea")
		->setEnabled(haveSelection && !compatibilityMode);
	QAction *selectcrop = getAction("selectcrop");
	selectcrop->setEnabled(haveSelection || haveTransform);
	selectcrop->setText(
		haveTransform ? tr("Cr&op canvas to transform…")
					  : tr("Cr&op canvas to selection…"));
	getAction("starttransform")->setEnabled(haveSelection);
	getAction("starttransformmask")->setEnabled(haveSelection);
	getAction("transformmirror")->setEnabled(haveTransform);
	getAction("transformflip")->setEnabled(haveTransform);
	getAction("transformrotatecw")->setEnabled(haveTransform);
	getAction("transformrotateccw")->setEnabled(haveTransform);
	getAction("transformshrinktoview")->setEnabled(haveTransform);
	getAction("showselectionmask")->setEnabled(!selectionEditActive);
	m_dockToolSettings->selectionSettings()->setActionEnabled(haveSelection);
	m_dockToolSettings->gradientSettings()->setSelectionValid(haveSelection);

	HudHandler::ActionBar actionBar = HudHandler::ActionBar::None;
	if(m_actionBarEnabled) {
		if(haveTransform) {
			actionBar = HudHandler::ActionBar::Transform;
		} else if(haveSelection) {
			actionBar = HudHandler::ActionBar::Selection;
		}
	}
	m_canvasView->hud()->setCurrentActionBar(actionBar);

	if(!haveSelection || haveTransform) {
		dialogs::SelectionAlterDialog *dlg =
			findChild<dialogs::SelectionAlterDialog *>(
				QStringLiteral("selectionalterdialog"),
				Qt::FindDirectChildrenOnly);
		if(dlg) {
			dlg->deleteLater();
		}
	}
}

void MainWindow::updateSelectionMaskVisibility()
{
	bool showSelectionMask = getAction("showselectionmask")->isChecked();
	bool selectionEditActive = m_doc->toolCtrl()->isSelectionEditActive();
	emit selectionMaskVisibilityChanged(
		showSelectionMask || selectionEditActive);
	canvas::CanvasModel *canvas = m_doc->canvas();
	if(canvas) {
		canvas->paintEngine()->setShowSelectionMask(showSelectionMask);
	}
}

// clang-format off

void MainWindow::copyText()
{
	// Attempt to copy text if a text widget has focus
	QWidget *focus = QApplication::focusWidget();

	auto *textedit = qobject_cast<QTextEdit*>(focus);
	if(textedit)
		textedit->copy();
}

// clang-format on
void MainWindow::paste()
{
	utils::ScopedOverrideCursor waitCursor;
	const QMimeData *mimeData = Document::getClipboardData();
	QImage img = Document::getClipboardImageData(mimeData);
	if(!img.isNull()) {
		QPoint pastepos;
		bool pasteAtPos = false;

		// Get source position
		QByteArray srcpos = mimeData->data("x-drawpile/pastesrc");
		if(!srcpos.isNull()) {
			QList<QByteArray> pos = srcpos.split(',');
			if(pos.size() == 4) {
				bool ok1, ok2, ok3, ok4;
				pastepos = QPoint(pos.at(0).toInt(&ok1), pos.at(1).toInt(&ok2));
				qint64 pid = pos.at(2).toLongLong(&ok3);
				qulonglong doc = pos.at(3).toULongLong(&ok4);
				pasteAtPos = ok1 && ok2 && ok3 && ok4 &&
							 pid == qApp->applicationPid() &&
							 doc == m_doc->pasteId();
			}
		}

		// Paste-in-place if we're the source (same process, same document)
		if(pasteAtPos && m_canvasView->isPointVisible(pastepos)) {
			pasteImage(img, &pastepos, true);
		} else {
			pasteImage(img);
		}
	}
}

void MainWindow::pasteCentered()
{
	utils::ScopedOverrideCursor waitCursor;
	const QMimeData *mimeData = Document::getClipboardData();
	QImage img = Document::getClipboardImageData(mimeData);
	if(!img.isNull()) {
		pasteImage(img, nullptr, true);
	}
}

void MainWindow::pasteFile()
{
	if(m_doc->checkPermission(DP_FEATURE_PUT_IMAGE)) {
		FileWrangler::ImageOpenFn imageOpenCompleted =
			[this](QImage &img, const QString &error) {
				if(img.isNull()) {
					showErrorMessage(
						//: %1 is an error message.
						tr("The image could not be loaded: %1.").arg(error));
				} else {
					utils::ScopedOverrideCursor waitCursor;
					pasteImage(img);
				}
			};
		FileWrangler(this).openPasteImage(imageOpenCompleted);
	}
}

void MainWindow::pasteFilePath(const QString &path)
{
	QGuiApplication::setOverrideCursor(Qt::WaitCursor);
	QString error;
	QImage img = utils::loadImageFromFile(path, &error);
	if(img.isNull()) {
		QGuiApplication::restoreOverrideCursor();
		showErrorMessage(
			error.isEmpty() ? tr("The image could not be loaded")
							: tr("The image could not be loaded: %1.")
								  .arg(error));
	} else {
		pasteImage(img);
		QGuiApplication::restoreOverrideCursor();
	}
}

void MainWindow::pasteImage(
	const QImage &image, const QPoint *point, bool force)
{
	canvas::CanvasModel *canvas = m_canvasView->canvas();
	if(canvas && !canvas->transform()->isActive() && !image.isNull() &&
	   !image.size().isEmpty()) {
		QRect srcBounds = canvas->getPasteBounds(
			image.size(), point ? *point : m_canvasView->viewCenterPoint(),
			force);
		if(!srcBounds.isEmpty() &&
		   m_doc->checkPermission(DP_FEATURE_PUT_IMAGE)) {
			m_dockToolSettings->startTransformPaste(
				srcBounds,
				image.convertToFormat(QImage::Format_ARGB32_Premultiplied));
		}
	}
}

void MainWindow::dropImage(const QImage &image)
{
	utils::ScopedOverrideCursor waitCursor;
	pasteImage(image);
}

void MainWindow::dropUrl(const QUrl &url)
{
	if(url.isLocalFile()) {
		QString path = url.toLocalFile();
		QString suffix = QFileInfo(path).suffix();
		if(suffix.compare(QStringLiteral("zip"), Qt::CaseInsensitive) == 0) {
			m_dockBrushPalette->importBrushesFrom(path);
		} else if(
			m_canvasView->canvas() &&
			!utils::paths::looksLikeCanvasReplacingSuffix(suffix)) {
			pasteFilePath(path);
		} else {
			questionOpenFileWindowReplacement([this, path](bool ok) {
				if(ok) {
					openPath(path);
				}
			});
		}
	}
}
// clang-format off

void MainWindow::clearOrDelete()
{
	// This slot is triggered in response to the 'Clear' action, which
	// which in turn can be triggered via the 'Delete' shortcut. In annotation
	// editing mode, the current selection may be an annotation, so we should delete
	// that instead of clearing out the canvas.
	QAction *annotationtool = getAction("tooltext");
	if(annotationtool->isChecked()) {
		const int a = m_dockToolSettings->annotationSettings()->selected();
		if(a>0) {
			net::Client *client = m_doc->client();
			uint8_t contextId = client->myId();
			net::Message messages[] = {
				net::makeUndoPointMessage(contextId),
				net::makeAnnotationDeleteMessage(contextId, a),
			};
			client->sendCommands(DP_ARRAY_LENGTH(messages), messages);
			return;
		}
	}

	// No annotation selected: clear seleted area as usual
	utils::ScopedOverrideCursor waitCursor;
	m_doc->clearArea();
}

// clang-format on
void MainWindow::resizeCanvas(int expandDirection)
{
	canvas::CanvasModel *canvas = m_doc->canvas();
	if(!canvas) {
		qWarning("resizeCanvas: no canvas!");
		return;
	}

	if(!m_doc->checkPermission(DP_FEATURE_RESIZE)) {
		return;
	}

	const QSize size = m_doc->canvas()->size();
	dialogs::ResizeDialog *dlg = new dialogs::ResizeDialog(
		size, getAction("expandup"), getAction("expandleft"),
		getAction("expandright"), getAction("expanddown"), this);

	connect(
		m_doc, &Document::compatibilityModeChanged, dlg,
		&dialogs::ResizeDialog::setCompatibilityMode);
	dlg->setCompatibilityMode(m_doc->isCompatibilityMode());

	canvas::PaintEngine *paintEngine = m_doc->canvas()->paintEngine();
	dlg->setBackgroundColor(paintEngine->historyBackgroundColor());
	dlg->setPreviewImage(
		paintEngine->renderPixmap().scaled(300, 300, Qt::KeepAspectRatio));
	dlg->setAttribute(Qt::WA_DeleteOnClose);

	if(const canvas::TransformModel *transform = canvas->transform();
	   transform->isActive()) {
		dlg->setBounds(
			transform->dstQuad().boundingRect().toAlignedRect(), false);
	} else if(
		canvas::SelectionModel *sel = canvas->selection(); sel->isValid()) {
		dlg->setBounds(sel->bounds(), true);
	}

	dlg->initialExpand(expandDirection);

	connect(dlg, &QDialog::accepted, this, [this, dlg]() {
		if(m_doc->checkPermission(DP_FEATURE_RESIZE)) {
			canvas::SelectionModel *sel = m_doc->canvas()->selection();
			if(sel->isValid() && sel->bounds().contains(dlg->newBounds())) {
				m_doc->selectNone(false);
			}
			dialogs::ResizeVector r = dlg->resizeVector();
			if(!r.isZero()) {
				m_doc->sendResizeCanvas(r.top, r.right, r.bottom, r.left);
			}
		}
	});
	utils::showWindow(dlg);
}
// clang-format off

void MainWindow::updateBackgroundActions()
{
	QAction *canvasBackground = getAction("canvas-background");
	QAction *setLocalBackground = getAction("set-local-background");
	QAction *clearLocalBackground = getAction("clear-local-background");
	canvas::CanvasModel *canvas = m_doc->canvas();
	if(canvas) {
		canvasBackground->setEnabled(true);
		setLocalBackground->setEnabled(true);

		canvas::PaintEngine *paintEngine = canvas->paintEngine();
		QColor sessionColor = paintEngine->historyBackgroundColor();
		canvasBackground->setIcon(utils::makeColorIcon(16, sessionColor));

		QColor localColor;
		if(paintEngine->localBackgroundColor(localColor)) {
			setLocalBackground->setIcon(utils::makeColorIcon(16, localColor));
			clearLocalBackground->setEnabled(true);
		} else {
			setLocalBackground->setIcon(QIcon{});
			clearLocalBackground->setEnabled(false);
		}
	} else {
		QIcon nullIcon = QIcon{};
		canvasBackground->setIcon(nullIcon);
		canvasBackground->setEnabled(false);
		setLocalBackground->setIcon(nullIcon);
		setLocalBackground->setEnabled(false);
		clearLocalBackground->setEnabled(false);
	}
}

// clang-format on
void MainWindow::changeCanvasBackground()
{
	if(m_doc->checkPermission(DP_FEATURE_BACKGROUND)) {
		color_widgets::ColorDialog *dlg = dialogs::newDeleteOnCloseColorDialog(
			m_doc->canvas()->paintEngine()->historyBackgroundColor(), this);
		dlg->setPreviewDisplayMode(color_widgets::ColorPreview::AllAlpha);
		connect(
			dlg, &color_widgets::ColorDialog::colorSelected, m_doc,
			&Document::sendCanvasBackground);
		utils::showWindow(dlg, shouldShowDialogMaximized());
	}
}

void MainWindow::changeLocalCanvasBackground()
{
	if(!m_doc->canvas()) {
		qWarning("changeLocalCanvasBackground: no canvas!");
		return;
	}

	canvas::PaintEngine *paintEngine = m_doc->canvas()->paintEngine();
	QColor color;
	if(!paintEngine->localBackgroundColor(color)) {
		color = paintEngine->historyBackgroundColor();
	}

	color_widgets::ColorDialog *dlg =
		dialogs::newDeleteOnCloseColorDialog(color, this);
	dlg->setPreviewDisplayMode(color_widgets::ColorPreview::AllAlpha);
	connect(
		dlg, &color_widgets::ColorDialog::colorSelected, paintEngine,
		&canvas::PaintEngine::setLocalBackgroundColor);
	utils::showWindow(dlg, shouldShowDialogMaximized());
}

void MainWindow::clearLocalCanvasBackground()
{
	if(!m_doc->canvas()) {
		qWarning("clearLocalCanvasBackground: no canvas!");
		return;
	}
	m_doc->canvas()->paintEngine()->clearLocalBackgroundColor();
}

void MainWindow::showLayoutsDialog()
{
	if(!m_smallScreenMode) {
		dialogs::LayoutsDialog *dlg = findChild<dialogs::LayoutsDialog *>(
			"layoutsdialog", Qt::FindDirectChildrenOnly);
		finishArrangingDocks();
		if(dlg) {
			dlg->setParent(getStartDialogOrThis());
		} else {
			dlg =
				new dialogs::LayoutsDialog{saveState(), getStartDialogOrThis()};
			dlg->setObjectName("layoutsdialog");
			dlg->setAttribute(Qt::WA_DeleteOnClose);
			connect(
				dlg, &dialogs::LayoutsDialog::applyState,
				[this](const QByteArray &state) {
					if(!m_smallScreenMode) {
						QScopedValueRollback<bool> updateRollback(
							m_updatingDockState, true);
						QScopedValueRollback<bool> restoreRollback(
							m_restoringDockState, true);
						m_intendedDockState = state;
						deactivateAllDocks();
						restoreState(state);
						refitWindow();
					}
				});
		}
		dlg->show();
		dlg->activateWindow();
		dlg->raise();
	}
}
// clang-format off

void MainWindow::showUserInfoDialog(int userId)
{
	for(auto *dlg : findChildren<dialogs::UserInfoDialog *>(QString(), Qt::FindDirectChildrenOnly)) {
		if(dlg->userId() == userId) {
			dlg->triggerUpdate();
			dlg->activateWindow();
			dlg->raise();
			return;
		}
	}

	canvas::User user = m_doc->canvas()->userlist()->getOptionalUserById(userId)
		.value_or(canvas::User{
			userId, tr("User #%1").arg(userId), {}, false, false, false, false,
			false, false, false, false, false, false});
	dialogs::UserInfoDialog *dlg = new dialogs::UserInfoDialog{user, this};
	dlg->setAttribute(Qt::WA_DeleteOnClose);
	connect(dlg, &dialogs::UserInfoDialog::requestUserInfo, this,
		&MainWindow::requestUserInfo);
	connect(m_doc->client(), &net::Client::userInfoReceived, dlg,
		&dialogs::UserInfoDialog::receiveUserInfo);
	dlg->triggerUpdate();
	dlg->show();
}

// clang-format on
void MainWindow::showAlterSelectionDialog()
{
	QString name = QStringLiteral("selectionalterdialog");
	dialogs::SelectionAlterDialog *dlg =
		findChild<dialogs::SelectionAlterDialog *>(
			name, Qt::FindDirectChildrenOnly);
	if(dlg) {
		dlg->activateWindow();
		dlg->raise();
	} else {
		dlg = new dialogs::SelectionAlterDialog(this);
		dlg->setAttribute(Qt::WA_DeleteOnClose);
		dlg->setWindowTitle(
			utils::scrubAccelerators(getAction("selectalter")->text()));
		connect(
			dlg, &dialogs::SelectionAlterDialog::alterSelectionRequested, this,
			&MainWindow::alterSelection, Qt::QueuedConnection);
		utils::showWindow(dlg);
	}
}

void MainWindow::alterSelection(
	int expand, int kernel, int feather, bool fromEdge)
{
	canvas::CanvasModel *canvas = m_doc->canvas();
	if(canvas) {
		SelectionAlteration *sa = new SelectionAlteration(
			canvas->paintEngine()->viewCanvasState(), m_doc->client()->myId(),
			canvas::CanvasModel::MAIN_SELECTION_ID, expand, kernel, feather,
			fromEdge);
		sa->setAutoDelete(true);
		connect(
			sa, &SelectionAlteration::success, m_doc, &Document::selectMask);

		QProgressDialog *progressDialog = new QProgressDialog(this);
		utils::makeModal(progressDialog, utils::Modality::Window);
		progressDialog->setRange(0, 0);
		progressDialog->setMinimumDuration(0);
		progressDialog->setLabelText(tr("Altering selection…"));
		connect(
			sa, &SelectionAlteration::success, progressDialog,
			&QProgressDialog::deleteLater);
		connect(
			sa, &SelectionAlteration::failure, progressDialog,
			&QProgressDialog::deleteLater);
		connect(
			progressDialog, &QProgressDialog::canceled, sa,
			&SelectionAlteration::cancel);
		progressDialog->show();

		QThreadPool::globalInstance()->start(sa);
	}
}

void MainWindow::changeUndoDepthLimit()
{
	QAction *action = getAction("sessionundodepthlimit");
	bool ok;
	int previousUndoDepthLimit = action->property("undodepthlimit").toInt(&ok);

	dialogs::SessionUndoDepthLimitDialog *dlg =
		new dialogs::SessionUndoDepthLimitDialog(
			ok ? previousUndoDepthLimit : DP_DUMP_UNDO_DEPTH_LIMIT, this);
	dlg->setAttribute(Qt::WA_DeleteOnClose);
	connect(
		dlg, &dialogs::SessionUndoDepthLimitDialog::accepted, this,
		[this, previousUndoDepthLimit, dlg] {
			int undoDepthLimit = dlg->undoDepthLimit();
			if(undoDepthLimit != previousUndoDepthLimit) {
				m_doc->client()->sendMessage(
					net::makeUndoDepthMessage(
						m_doc->canvas()->localUserId(), undoDepthLimit));
			}
		});
	dlg->show();
}

void MainWindow::updateDevToolsActions()
{
	QAction *tabletEventLogAction = getAction("tableteventlog");
	tabletEventLogAction->setText(
		drawdance::EventLog::isOpen() ? tr("Stop Tablet Event Log")
									  : tr("Tablet Event Log..."));

	QAction *profileAction = getAction("profile");
	profileAction->setText(
		drawdance::Perf::isOpen() ? tr("Stop Profile") : tr("Profile..."));

	net::Client *client = m_doc->client();
	bool connected = client->isConnected();

	QAction *artificialLagAction =
		searchAction(QStringLiteral("artificiallag"));
	if(artificialLagAction) {
		artificialLagAction->setEnabled(connected);
		int artificialLagMs = client->artificialLagMs();
		artificialLagAction->setText(
			tr("Set Artificial Lag... (currently %1 ms)").arg(artificialLagMs));
	}

	QAction *artificialDisconnectAction = searchAction("artificialdisconnect");
	if(artificialDisconnectAction) {
		artificialDisconnectAction->setEnabled(connected);
	}
#ifndef __EMSCRIPTEN__
	QAction *debugDumpAction = getAction("debugdump");
	debugDumpAction->setChecked(m_doc->wantCanvasHistoryDump());
#endif
}

void MainWindow::setArtificialLag()
{
	utils::getInputInt(
		this, tr("Set Artificial Lag"),
		tr("Artificial lag in milliseconds (0 to disable):"),
		m_doc->client()->artificialLagMs(), 0, INT_MAX,
		[this](int artificialLagMs) {
			m_doc->client()->setArtificialLagMs(artificialLagMs);
		});
}

void MainWindow::setArtificialDisconnect()
{
	utils::getInputInt(
		this, tr("Artificial Disconnect"),
		tr("Simulate a disconnect after this many seconds:"), 1, 0, INT_MAX,
		[this](int seconds) {
			QTimer::singleShot(
				seconds * 1000, m_doc->client(),
				&net::Client::artificialDisconnect);
		});
}

#ifndef __EMSCRIPTEN__
void MainWindow::toggleDebugDump()
{
	if(m_doc->wantCanvasHistoryDump()) {
		m_doc->setWantCanvasHistoryDump(false);
	} else {
		QString path = utils::paths::writablePath("dumps");
		QMessageBox::StandardButton result = QMessageBox::question(
			this, tr("Record Debug Dumps"),
			tr("Debug dumps will record local and remote drawing commands. "
			   "They can be used to fix network issues, but not much else. "
			   "If you want to make a regular recording, use File > Record... "
			   "instead.\n\nDebug dump recording starts on the next canvas "
			   "reset and the files will be saved in %1\n\nAre you sure you"
			   "want to start recording debug dumps?")
				.arg(path));
		if(result == QMessageBox::Yes) {
			m_doc->setWantCanvasHistoryDump(true);
		}
	}
}
#endif

void MainWindow::openDebugDump()
{
	questionWindowReplacement(
		tr("Open Debug Dump"),
		tr("You're about to open a debug dump and close this window."),
		[this](bool ok) {
			if(ok) {
				FileWrangler(this).openDebugDump(
					std::bind(&MainWindow::openPath, this, _1, _2));
			}
		});
}

#ifdef DRAWPILE_PROJECT_INFO_DIALOG
void MainWindow::openProjectInfo()
{
	QString path = FileWrangler(this).openProjectInfo();
	if(!path.isEmpty()) {
		dialogs::ProjectInfoDialog *dlg = new dialogs::ProjectInfoDialog(this);
		utils::showWindow(dlg);
		dlg->loadProjectInfo(path);
	}
}
#endif

void MainWindow::causeCrash()
{
	QMessageBox *box = utils::makeQuestion(
		this, tr("Cause Crash"),
		tr("Do you really want to crash Drawpile? This will terminate the "
		   "program and you will lose any unsaved data!"));
	connect(box, &QMessageBox::finished, this, [this](int result) {
		if(result == QMessageBox::Yes) {
			QObject *nonexistent = findChild<QObject *>(
				QStringLiteral("nonexistent"), Qt::FindDirectChildrenOnly);
			nonexistent->setObjectName(QStringLiteral("stillnonexistent"));
			nonexistent->setProperty("nonexistent", true);
			nonexistent->deleteLater();
		}
	});
	utils::showMessageBox(box);
}
// clang-format off


void MainWindow::about()
{
	auto [pixelSize, mmSize] = DrawpileApp::screenResolution();
	QMessageBox::about(nullptr, tr("About Drawpile"),
			QStringLiteral("<p><b>Drawpile %1</b> (%2)<br>").arg(cmake_config::version(), QSysInfo::buildCpuArchitecture()) +
			tr("A collaborative drawing program.") + QStringLiteral("</p>"

			"<p>Copyright © REDACTED and contributors. Originally made by Calle Laakkonen.</p>"

			"<p>This program is free software; you may redistribute it and/or "
			"modify it under the terms of the GNU General Public License as "
			"published by the Free Software Foundation, either version 3, or "
			"(at your opinion) any later version.</p>"

			"<p>This program is distributed in the hope that it will be useful, "
			"but WITHOUT ANY WARRANTY; without even the implied warranty of "
			"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
			"GNU General Public License for more details.</p>"

			"<p>You should have received a copy of the GNU General Public License "
			"along with this program.  If not, see <a href=\"http://www.gnu.org/licences/\">http://www.gnu.org/licenses/</a>.</p>"
			) +
			QStringLiteral("<hr><p><b>%1</b> %2</p><p><b>%3</b> %4</p><p><b>%5</b> %6</p>")
				.arg(tr("Settings File:"))
				.arg(dpAppConfig()->path().toHtmlEscaped())
				.arg(tr("Tablet Input:"))
				.arg(QCoreApplication::translate("tabletinput", tabletinput::current()))
				.arg(tr("Primary screen:"))
				.arg(tr("%1x%2px² (%3x%4mm²)")
					.arg(pixelSize.width()).arg(pixelSize.height())
					.arg(mmSize.width()).arg(mmSize.height())));
}

// clang-format on
void MainWindow::homepage()
{
	QDesktopServices::openUrl(QUrl(cmake_config::website()));
}

void MainWindow::donate()
{
	QDesktopServices::openUrl(
		QUrl(QStringLiteral("https://drawpile.net/donate/")));
}
// clang-format off

/**
 * @brief Create a new action.
 *
 * All created actions are added to a list that is used in the
 * settings dialog to edit the shortcuts.
 *
 * @param name (internal) name of the action.
 * @param text action text
 */
ActionBuilder MainWindow::makeAction(const char *name, const QString& text)
{
	Q_ASSERT(name);
	QAction *act = new QAction(text, this);
	act->setObjectName(name);
	act->setAutoRepeat(false);

	// Add this action to the mainwindow so its shortcut can be used
	// even when the menu/toolbar is not visible
	addAction(act);

	return ActionBuilder(act);
}

// clang-format on
QAction *MainWindow::getAction(const QString &name)
{
	QAction *action = searchAction(name);
	Q_ASSERT(action);
	if(action) {
		return action;
	} else {
		qFatal("%s: no such action", qUtf8Printable(name));
		std::abort();
	}
}

QAction *MainWindow::searchAction(const QString &name)
{
	return findChild<QAction *>(name, Qt::FindDirectChildrenOnly);
}

void MainWindow::addBrushShortcut(
	const QString &name, const QString &text, const QKeySequence &shortcut)
{
	Q_ASSERT(!searchAction(name));
	QAction *action = new QAction(text, this);
	action->setObjectName(name);
	action->setShortcut(shortcut);
	addAction(action);
	connect(
		action, &QAction::triggered, this,
		std::bind(&MainWindow::triggerBrushShortcut, this, action));
	action->installEventFilter(this);
}

void MainWindow::changeBrushShortcut(const QString &name, const QString &text)
{
	QAction *action = searchAction(name);
	if(action) {
		action->setText(text);
	} else {
		qWarning(
			"changeBrushShortcut: action '%s' not found", qUtf8Printable(name));
	}
}

void MainWindow::removeBrushShortcut(const QString &name)
{
	QAction *action = searchAction(name);
	if(action) {
		removeAction(action);
		delete action;
	} else {
		qWarning(
			"removeBrushShortcut: action '%s' not found", qUtf8Printable(name));
	}
}

void MainWindow::triggerBrushShortcut(QAction *action)
{
	m_dockBrushPalette->setSelectedPresetIdsFromShortcut(action->shortcut());
}
// clang-format off

/**
 * @brief Create actions, menus and toolbars
 */
void MainWindow::setupActions()
{
	Q_ASSERT(m_doc);
	Q_ASSERT(m_dockLayers);

	config::Config *cfg = dpAppConfig();

	// Action groups
	m_currentdoctools = new QActionGroup(this);
	m_currentdoctools->setExclusive(false);
	m_currentdoctools->setEnabled(false);

	m_admintools = new QActionGroup(this);
	m_admintools->setExclusive(false);

	m_canvasbgtools = new QActionGroup(this);
	m_canvasbgtools->setEnabled(false);

	m_resizetools = new QActionGroup(this);
	m_putimagetools = new QActionGroup(this);
	m_undotools = new QActionGroup(this);

	m_drawingtools = new QActionGroup(this);
	connect(m_drawingtools, SIGNAL(triggered(QAction*)), this, SLOT(selectTool(QAction*)));

	QMenu *toggletoolbarmenu = new QMenu(this);
	QMenu *toggledockmenu = new QMenu(this);
	m_dockToggles = new QActionGroup{this};
	m_dockToggles->setExclusive(false);

	m_desktopModeActions = new QActionGroup(this);
	m_desktopModeActions->setExclusive(false);
	m_smallScreenModeActions = new QActionGroup(this);
	m_smallScreenModeActions->setExclusive(false);

	// clang-format on
	QList<const docks::DockBase *> docks =
		findChildren<const docks::DockBase *>(
			QString(), Qt::FindDirectChildrenOnly);
	std::sort(
		docks.begin(), docks.end(),
		[](const docks::DockBase *a, const docks::DockBase *b) {
			return a->fullTitle().compare(b->fullTitle(), Qt::CaseInsensitive) <
				   0;
		});
	for(const docks::DockBase *dw : docks) {
		QAction *toggledockaction = dw->toggleViewAction();
		Q_ASSERT(!dw->objectName().isEmpty());
		Q_ASSERT(toggledockaction->objectName().isEmpty());
		toggledockaction->setObjectName(
			QStringLiteral("toggledock%1").arg(dw->objectName()));
		toggledockmenu->addAction(toggledockaction);
		m_dockToggles->addAction(toggledockaction);
		CustomShortcutModel::registerCustomizableAction(
			toggledockaction->objectName(),
			tr("Toggle Dock %1").arg(toggledockaction->text()),
			toggledockaction->icon(), toggledockaction->shortcut(),
			QKeySequence());
		addAction(toggledockaction);
		m_desktopModeActions->addAction(toggledockaction);
		connect(
			toggledockaction, &QAction::triggered, dw,
			&docks::DockBase::makeTabCurrent, Qt::QueuedConnection);
		connect(
			dw, &docks::DockBase::tabUpdateRequested, this,
			&MainWindow::prepareDockTabUpdate);
		connect(
			dw, &docks::DockBase::arrangingFinished, this,
			&MainWindow::finishArrangingDocks);
	}
	//clang-format off

	toggledockmenu->addSeparator();
	QAction *freezeDocks = makeAction("freezedocks", tr("Lock Docks"))
							   .noDefaultShortcut()
							   .checkable()
							   .remembered();
	toggledockmenu->addAction(freezeDocks);
	m_desktopModeActions->addAction(freezeDocks);
	connect(freezeDocks, &QAction::toggled, this, &MainWindow::setFreezeDocks);

	QAction *dockTabIcons = makeAction("docktabicons", tr("Show Icons on Tabs"))
								.noDefaultShortcut()
								.checked()
								.remembered();
	toggledockmenu->addAction(dockTabIcons);
	m_desktopModeActions->addAction(dockTabIcons);
	connect(
		dockTabIcons, &QAction::toggled, this,
		&MainWindow::prepareDockTabUpdate);

	QAction *sideTabDocks =
		makeAction("sidetabdocks", tr("Vertical Tabs on Sides"))
			.noDefaultShortcut()
			.checkable()
			.remembered();
	toggledockmenu->addAction(sideTabDocks);
	m_desktopModeActions->addAction(sideTabDocks);
	connect(
		sideTabDocks, &QAction::toggled, this, &MainWindow::updateSideTabDocks);
	updateSideTabDocks();

	QAction *hideDocks =
		makeAction("hidedocks", tr("Hide Docks")).checkable().shortcut("tab");
	toggledockmenu->addAction(hideDocks);
	m_desktopModeActions->addAction(hideDocks);
	connect(hideDocks, &QAction::toggled, this, &MainWindow::setDocksHidden);

	QAction *arrangeDocks = makeAction("arrangedocks", tr("Arrange Docks"))
								.noDefaultShortcut()
								.checkable();
	toggledockmenu->addAction(arrangeDocks);
	m_desktopModeActions->addAction(arrangeDocks);
	connect(
		arrangeDocks, &QAction::toggled, this, &MainWindow::setDockArrangeMode);

	//
	// File menu and toolbar
	//
	QAction *newdocument = makeAction("newdocument", tr("&New"))
							   .icon("document-new")
							   .shortcut(QKeySequence::New);
	QAction *open = makeAction("opendocument", tr("&Open..."))
						.icon("document-open")
						.shortcut(QKeySequence::Open);
#ifdef Q_OS_MACOS
	QAction *closefile =
		makeAction("closedocument", tr("Close")).shortcut(QKeySequence::Close);
#endif
	QAction *autoRecord = makeAction("autorecord", tr("Autorecovery"))
							  .noDefaultShortcut()
							  .checkable();
	QAction *autoRecordSettings =
		makeAction("autorecordsettings", tr("Manage autorecovery…"))
			.noDefaultShortcut();
#ifdef __EMSCRIPTEN__
	QAction *download = makeAction("downloaddocument", tr("&Download Image…"))
							.icon("document-save")
							.shortcut(QKeySequence::Save);
	QAction *downloadsel =
		makeAction("downloadselection", tr("Download Selection…"))
			.icon("select-rectangular")
			.noDefaultShortcut();
#else
	QAction *save = makeAction("savedocument", tr("&Save"))
						.icon("document-save")
						.shortcut(QKeySequence::Save);
	QAction *saveas = makeAction("savedocumentas", tr("Save &As..."))
						  .icon("document-save-as")
						  .shortcut(QKeySequence::SaveAs);
	QAction *saveAsDpcs = makeAction("savedocumentasdpcs", tr("Sa&ve As DPCS…"))
							  .noDefaultShortcut();
	QAction *saveAsOra = makeAction("savedocumentasora", tr("Sa&ve As ORA…"))
							 .noDefaultShortcut();
	QAction *exportDocument = makeAction("exportdocument", tr("Export Image…"))
								  .icon("document-export")
								  .noDefaultShortcut();
#	ifndef Q_OS_ANDROID
	QAction *exportDocumentAgain =
		makeAction("exportdocumentagain", tr("Export Again"))
			.noDefaultShortcut();
#	endif
	QAction *savesel = makeAction("saveselection", tr("Export Selection..."))
						   .icon("select-rectangular")
						   .noDefaultShortcut();
	QAction *exportTemplate =
		makeAction("exporttemplate", tr("Export Session &Template..."))
			.noDefaultShortcut();
#endif
	QAction *exportAnimation =
		makeAction("exportanim", tr("Export &Animation…"))
			.icon("document-save-all")
			.noDefaultShortcut();
#ifndef __EMSCRIPTEN__
	QAction *importAnimationFrames =
		makeAction("importanimationframes", tr("Import Animation &Frames…"))
			.noDefaultShortcut();
#endif
	QAction *importAnimationLayers =
		makeAction("importoldanimation", tr("Import Animation from &Layers…"))
			.noDefaultShortcut();
	QAction *importBrushes =
		makeAction("importbrushes", tr("Import &Brushes..."))
			.noDefaultShortcut();
	QAction *exportBrushes =
		makeAction("exportbrushes", tr("Export &Brushes…")).noDefaultShortcut();

#ifndef __EMSCRIPTEN__
	QAction *record = makeAction("recordsession", tr("Record..."))
						  .icon("media-record")
						  .noDefaultShortcut();
#endif
#ifdef DRAWPILE_PROJECT_DIALOG
	QAction *projectOverview =
		makeAction("projectoverview", tr("Project statistics…"))
			.noDefaultShortcut();
#endif
#ifdef DRAWPILE_TIMELAPSE_DIALOG
	QAction *makeTimelapse =
		makeAction("maketimelapse", tr("Make timelapse…")).noDefaultShortcut();
#endif
	QAction *start = makeAction("start", tr("Start...")).noDefaultShortcut();
	QAction *recover = makeAction("recover", tr("Recover…"))
						   .icon(QStringLiteral("backup"))
						   .noDefaultShortcut();
#ifndef __EMSCRIPTEN__
	QAction *quit = makeAction("exitprogram", tr("&Quit"))
						.icon("application-exit")
						.shortcut("Ctrl+Q");
#endif
#ifdef Q_OS_MACOS
	QAction *macQuit =
		makeAction("macexitprogram", tr("&Quit")).menuRole(QAction::QuitRole);
#endif

#ifdef Q_OS_MACOS
	m_currentdoctools->addAction(closefile);
#endif
#ifdef __EMSCRIPTEN__
	m_currentdoctools->addAction(download);
	m_currentdoctools->addAction(downloadsel);
#else
	m_currentdoctools->addAction(save);
	m_currentdoctools->addAction(saveas);
	m_currentdoctools->addAction(saveAsDpcs);
	m_currentdoctools->addAction(saveAsOra);
	m_currentdoctools->addAction(exportTemplate);
	m_currentdoctools->addAction(savesel);
	m_currentdoctools->addAction(exportDocument);
#	ifndef Q_OS_ANDROID
	m_currentdoctools->addAction(exportDocumentAgain);
#	endif
	m_currentdoctools->addAction(record);
	m_currentdoctools->addAction(autoRecord);
#endif
	m_currentdoctools->addAction(exportAnimation);

	connect(newdocument, SIGNAL(triggered()), this, SLOT(showNew()));
	connect(open, SIGNAL(triggered()), this, SLOT(open()));
#ifdef __EMSCRIPTEN__
	connect(download, &QAction::triggered, this, &MainWindow::download);
	connect(
		downloadsel, &QAction::triggered, this, &MainWindow::downloadSelection);
#else
	connect(save, &QAction::triggered, this, &MainWindow::save);
	connect(saveas, &QAction::triggered, this, &MainWindow::saveAs);
	connect(saveAsDpcs, &QAction::triggered, this, &MainWindow::saveAsDpcs);
	connect(saveAsOra, &QAction::triggered, this, &MainWindow::saveAsOra);
	connect(
		exportDocument, &QAction::triggered, this, &MainWindow::exportImage);
#	ifndef Q_OS_ANDROID
	connect(
		exportDocumentAgain, &QAction::triggered, this,
		&MainWindow::exportImageAgain);
#	endif
	connect(
		exportTemplate, &QAction::triggered, this, &MainWindow::exportTemplate);
	connect(savesel, &QAction::triggered, this, &MainWindow::saveSelection);

	connect(record, &QAction::triggered, this, &MainWindow::toggleRecording);
#endif
	connect(
		autoRecord, &QAction::triggered, this,
		&MainWindow::toggleProjectRecording);
	connect(
		autoRecordSettings, &QAction::triggered, this,
		&MainWindow::showProjectRecordingSettings);
	connect(
		m_statusAutoRecordButton, &QToolButton::clicked, autoRecordSettings,
		&QAction::trigger);

	connect(
		exportAnimation, &QAction::triggered, this,
		std::bind(&MainWindow::showAnimationExportDialog, this, false));

#ifndef __EMSCRIPTEN__
	connect(
		importAnimationFrames, &QAction::triggered, this,
		&MainWindow::importAnimationFrames);
#endif
	connect(
		importAnimationLayers, &QAction::triggered, this,
		&MainWindow::importAnimationLayers);
	connect(
		importBrushes, &QAction::triggered, m_dockBrushPalette,
		&docks::BrushPalette::importBrushes);
	connect(
		exportBrushes, &QAction::triggered, m_dockBrushPalette,
		&docks::BrushPalette::exportBrushes);
#ifdef DRAWPILE_PROJECT_DIALOG
	connect(
		projectOverview, &QAction::triggered, this,
		&MainWindow::requestProjectOverview);
#endif
#ifdef DRAWPILE_TIMELAPSE_DIALOG
	connect(
		makeTimelapse, &QAction::triggered, this,
		&MainWindow::requestTimelapseDialog);
#endif
	connect(start, &QAction::triggered, this, &MainWindow::start);
	connect(recover, &QAction::triggered, this, &MainWindow::showRecover);

#ifndef __EMSCRIPTEN__
#	ifdef Q_OS_MACOS
	connect(closefile, SIGNAL(triggered()), this, SLOT(close()));
	connect(quit, SIGNAL(triggered()), MacMenu::instance(), SLOT(quitAll()));
	connect(macQuit, SIGNAL(triggered()), MacMenu::instance(), SLOT(quitAll()));
#	else
	connect(quit, SIGNAL(triggered()), this, SLOT(close()));
#	endif
#endif

	setMenuBar(new widgets::NonAltStealingMenuBar(this));

	QMenu *filemenu = menuBar()->addMenu(tr("File"));
	filemenu->addAction(newdocument);
	filemenu->addAction(open);
#ifndef __EMSCRIPTEN__
	if(!m_singleSession) {
		m_recentMenu = filemenu->addMenu(tr("Open &Recent"));
		m_recentMenu->setIcon(QIcon::fromTheme("document-open-recent"));
	}
#endif
	filemenu->addSeparator();

#ifdef __EMSCRIPTEN__
	filemenu->addAction(download);
	filemenu->addAction(downloadsel);
#else
#	ifdef Q_OS_MACOS
	filemenu->addAction(closefile);
#	endif
	filemenu->addAction(save);
	filemenu->addAction(saveas);
	filemenu->addAction(saveAsDpcs);
	filemenu->addAction(saveAsOra);
	filemenu->addAction(savesel);
	filemenu->addAction(exportDocument);
#	ifndef Q_OS_ANDROID
	filemenu->addAction(exportDocumentAgain);
#	endif
#endif
	filemenu->addSeparator();

	QMenu *importMenu = filemenu->addMenu(tr("&Import"));
	importMenu->setIcon(QIcon::fromTheme("document-import"));
#ifndef __EMSCRIPTEN__
	importMenu->addAction(importAnimationFrames);
#endif
	importMenu->addAction(importAnimationLayers);
	importMenu->addAction(importBrushes);

	QMenu *exportMenu = filemenu->addMenu(tr("&Export"));
	exportMenu->setIcon(QIcon::fromTheme("document-export"));
#ifndef __EMSCRIPTEN__
	exportMenu->addAction(exportDocument);
	exportMenu->addAction(exportTemplate);
#endif
	exportMenu->addAction(exportAnimation);
	exportMenu->addAction(exportBrushes);
#ifndef __EMSCRIPTEN__
	filemenu->addAction(record);
#endif
	filemenu->addAction(autoRecord);
	filemenu->addAction(autoRecordSettings);
#if defined(DRAWPILE_PROJECT_DIALOG) || defined(DRAWPILE_TIMELAPSE_DIALOG)
	filemenu->addSeparator();
#endif
#ifdef DRAWPILE_PROJECT_DIALOG
	filemenu->addAction(projectOverview);
#endif
#ifdef DRAWPILE_TIMELAPSE_DIALOG
	filemenu->addAction(makeTimelapse);
#endif
	filemenu->addSeparator();
	filemenu->addAction(start);
	filemenu->addAction(recover);
#ifndef __EMSCRIPTEN__
	filemenu->addAction(quit);
#endif
#ifdef Q_OS_MACOS
	filemenu->addAction(macQuit);
#endif

	m_toolBarFile = new QToolBar(tr("File Tools"));
	m_toolBarFile->setObjectName("filetoolsbar");
	toggletoolbarmenu->addAction(m_toolBarFile->toggleViewAction());

	// clang-format on

	if(!m_singleSession) {
		m_toolBarFile->addAction(newdocument);
		m_toolBarFile->addAction(open);
	}
#ifdef __EMSCRIPTEN__
	m_toolBarFile->addAction(download);
#else
	m_toolBarFile->addAction(save);
#endif

#ifndef __EMSCRIPTEN__
	if(!m_singleSession) {
		connect(m_recentMenu, &QMenu::triggered, this, [this](QAction *action) {
			QVariant filepath = action->property("filepath");
			if(filepath.isValid()) {
				this->openRecent(filepath.toString());
			} else {
				showStartDialogOnPage(int(dialogs::StartDialog::Entry::Recent));
			}
		});
	}
#endif
	// clang-format off

	//
	// Edit menu
	//
#ifdef Q_OS_ANDROID
	QKeySequence undoShortcut = QKeySequence{Qt::Key_VolumeUp};
	QKeySequence redoShortcut = QKeySequence{Qt::Key_VolumeDown};
	QKeySequence undoAlternateShortcut = QKeySequence::Undo;
	QKeySequence redoAlternateShortcut = QKeySequence::Redo;
#else
	QKeySequence undoShortcut = QKeySequence::Undo;
	QKeySequence undoAlternateShortcut = QKeySequence();
#	if defined(Q_OS_WIN) || defined(__EMSCRIPTEN__)
	QKeySequence redoShortcut = QKeySequence("Ctrl+Y");
	QKeySequence redoAlternateShortcut = QKeySequence("Ctrl+Shift+Z");
#	elif defined(Q_OS_LINUX)
	QKeySequence redoShortcut = QKeySequence("Ctrl+Shift+Z");
	QKeySequence redoAlternateShortcut = QKeySequence("Ctrl+Y");
#	else
	QKeySequence redoShortcut = QKeySequence::Redo;
	QKeySequence redoAlternateShortcut = QKeySequence();
#	endif
#endif
	QAction *undo = makeAction("undo", tr("&Undo"))
						.icon("edit-undo")
						.shortcut(undoShortcut, undoAlternateShortcut)
						.autoRepeat();
	QAction *redo = makeAction("redo", tr("&Redo"))
						.icon("edit-redo")
						.shortcut(redoShortcut, redoAlternateShortcut)
						.autoRepeat();
	QAction *copy = makeAction("copyvisible", tr("&Copy Merged"))
						.icon("edit-copy")
						.statusTip(tr("Copy selected area to the clipboard"))
						.shortcut("Shift+Ctrl+C");
	QAction *copyMerged =
		makeAction("copymerged", tr("Copy Without Background"))
			.icon("edit-copy")
			.statusTip(tr("Copy selected area, excluding the background, to "
						  "the clipboard"))
			.shortcut("Ctrl+Alt+C");
	QAction *copylayer =
		makeAction("copylayer", tr("Copy From &Layer"))
			.icon("edit-copy")
			.statusTip(
				tr("Copy selected area of the current layer to the clipboard"))
			.shortcut(QKeySequence::Copy);
	QAction *cutlayer =
		makeAction("cutlayer", tr("Cu&t From Layer"))
			.icon("edit-cut")
			.statusTip(
				tr("Cut selected area of the current layer to the clipboard"))
			.shortcut(QKeySequence::Cut);
	QAction *paste = makeAction("paste", tr("&Paste"))
						 .icon("edit-paste")
						 .shortcut(QKeySequence::Paste);
	QAction *pasteCentered =
		makeAction("paste-centered", tr("Paste in View Center"))
			.icon("edit-paste")
			.shortcut("Ctrl+Shift+V");
#ifndef SINGLE_MAIN_WINDOW
	QAction *pickFromScreen =
		makeAction("pickfromscreen", tr("Pic&k From Screen"))
			.icon("monitor")
			.shortcut("Shift+I");
#endif

	QAction *pastefile = makeAction("pastefile", tr("Paste &From File..."))
							 .icon("document-open")
							 .noDefaultShortcut();
	QAction *deleteAnnotations =
		makeAction("deleteemptyannotations", tr("Delete Empty Annotations"))
			.noDefaultShortcut();
	QAction *resize =
		makeAction("resizecanvas", tr("Resi&ze Canvas...")).noDefaultShortcut();
	QAction *canvasBackground =
		makeAction("canvas-background", tr("Set Session Background..."))
			.noDefaultShortcut();
	QAction *setLocalBackground =
		makeAction("set-local-background", tr("Set Local Background..."))
			.noDefaultShortcut();
	QAction *clearLocalBackground =
		makeAction("clear-local-background", tr("Clear Local Background"))
			.noDefaultShortcut();
	QAction *brushSettings = makeAction("brushsettings", tr("&Brush Settings"))
								 .icon("draw-brush")
								 .shortcut("F7");
	QAction *inputSettings = makeAction("inputsettings", tr("Input Settings"))
								 .icon("pathshape")
								 .noDefaultShortcut();
	QAction *preferences = makeAction("preferences", tr("Prefere&nces"))
							   .icon("configure")
							   .noDefaultShortcut();
#ifdef Q_OS_MACOS
	QAction *macPreferences = makeAction("macpreferences", tr("Prefere&nces"))
								  .menuRole(QAction::PreferencesRole);
#endif

#ifdef Q_OS_WIN32
	QVector<QAction *> drivers;
	drivers.append(
		makeAction(
			"driverkistabletwindowsink",
			QCoreApplication::translate(
				"dialogs::settingsdialog::Tablet", "Windows Ink"))
			.noDefaultShortcut()
			.checkable()
			.property("tabletdriver", int(tools::TabletInputMode::KisTabletWinink)));
	drivers.append(
		makeAction(
			"driverkistabletwindowsinknonnative",
			QCoreApplication::translate(
				"dialogs::settingsdialog::Tablet", "Windows Ink Non-Native"))
			.noDefaultShortcut()
			.checkable()
			.property(
				"tabletdriver",
				int(tools::TabletInputMode::KisTabletWininkNonNative)));
	drivers.append(
		makeAction(
			"driverkistabletwintab",
			QCoreApplication::translate(
				"dialogs::settingsdialog::Tablet", "Wintab"))
			.noDefaultShortcut()
			.checkable()
			.property("tabletdriver", int(tools::TabletInputMode::KisTabletWintab)));
	drivers.append(
		makeAction(
			"driverkistabletwintabrelative",
			QCoreApplication::translate(
				"dialogs::settingsdialog::Tablet", "Wintab Relative"))
			.noDefaultShortcut()
			.checkable()
			.property(
				"tabletdriver",
				int(tools::TabletInputMode::KisTabletWintabRelativePenHack)));
#	if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	drivers.append(
		makeAction(
			"driverqt5", QCoreApplication::translate(
							 "dialogs::settingsdialog::Tablet", "Qt5"))
			.noDefaultShortcut()
			.checkable()
			.property("tabletdriver", int(tools::TabletInputMode::Qt5)));
#	else
	drivers.append(
		makeAction(
			"driverqt6windowsink",
			QCoreApplication::translate(
				"dialogs::settingsdialog::Tablet", "Qt6 Windows Ink"))
			.noDefaultShortcut()
			.checkable()
			.property("tabletdriver", int(tools::TabletInputMode::Qt6Winink)));
	drivers.append(
		makeAction(
			"driverqt6wintab",
			QCoreApplication::translate(
				"dialogs::settingsdialog::Tablet", "Qt6 Wintab"))
			.noDefaultShortcut()
			.checkable()
			.property("tabletdriver", int(tools::TabletInputMode::Qt6Wintab)));
#	endif
#endif

	// clang-format on
	QAction *expandup = makeAction("expandup", tr("Expand &Up…"))
							.icon("drawpile_expandup")
							.shortcut(CTRL_KEY | Qt::Key_J);
	QAction *expanddown = makeAction("expanddown", tr("Expand &Down…"))
							  .icon("drawpile_expanddown")
							  .shortcut(CTRL_KEY | Qt::Key_K);
	QAction *expandleft = makeAction("expandleft", tr("Expand &Left…"))
							  .icon("drawpile_expandleft")
							  .shortcut(CTRL_KEY | Qt::Key_H);
	QAction *expandright = makeAction("expandright", tr("Expand &Right…"))
							   .icon("drawpile_expandright")
							   .shortcut(CTRL_KEY | Qt::Key_L);

	QAction *cleararea = makeAction("cleararea", tr("Delete"))
							 .shortcut(QKeySequence::Delete)
							 .icon("trash-empty");

	m_currentdoctools->addAction(copy);
	m_currentdoctools->addAction(copylayer);
	m_currentdoctools->addAction(deleteAnnotations);

	m_undotools->addAction(undo);
	m_undotools->addAction(redo);

	m_putimagetools->addAction(cutlayer);
	m_putimagetools->addAction(paste);
	m_putimagetools->addAction(pasteCentered);
	m_putimagetools->addAction(pastefile);
	m_putimagetools->addAction(cleararea);

	m_canvasbgtools->addAction(canvasBackground);
	m_resizetools->addAction(resize);
	m_resizetools->addAction(expandup);
	m_resizetools->addAction(expanddown);
	m_resizetools->addAction(expandleft);
	m_resizetools->addAction(expandright);

	connect(undo, &QAction::triggered, m_doc, &Document::undo);
	connect(redo, &QAction::triggered, m_doc, &Document::redo);
	connect(copy, &QAction::triggered, m_doc, &Document::copyVisible);
	connect(copyMerged, &QAction::triggered, m_doc, &Document::copyMerged);
	connect(copylayer, &QAction::triggered, m_doc, &Document::copyLayer);
	connect(cutlayer, &QAction::triggered, m_doc, &Document::cutLayer);
	connect(paste, &QAction::triggered, this, &MainWindow::paste);
	// clang-format off
	connect(pasteCentered, &QAction::triggered, this, &MainWindow::pasteCentered);
#ifndef SINGLE_MAIN_WINDOW
	connect(
		pickFromScreen, &QAction::triggered,
		m_dockToolSettings->colorPickerSettings(),
		&tools::ColorPickerSettings::startPickFromScreen);
#endif
	connect(pastefile, SIGNAL(triggered()), this, SLOT(pasteFile()));
	connect(deleteAnnotations, &QAction::triggered, m_doc, &Document::removeEmptyAnnotations);
	connect(cleararea, &QAction::triggered, this, &MainWindow::clearOrDelete);
	connect(
		resize, &QAction::triggered, this,
		std::bind(
			&MainWindow::resizeCanvas, this,
			int(dialogs::ResizeDialog::ExpandDirection::None)));
	connect(canvasBackground, &QAction::triggered, this, &MainWindow::changeCanvasBackground);
	connect(setLocalBackground, &QAction::triggered, this, &MainWindow::changeLocalCanvasBackground);
	connect(clearLocalBackground, &QAction::triggered, this, &MainWindow::clearLocalCanvasBackground);
	connect(brushSettings, &QAction::triggered, this, &MainWindow::showBrushSettingsDialogBrush);
	connect(inputSettings, &QAction::triggered, this, &MainWindow::showInputSettingsDialog);
	connect(preferences, SIGNAL(triggered()), this, SLOT(showSettings()));
#ifdef Q_OS_MACOS
	connect(macPreferences, SIGNAL(triggered()), this, SLOT(showSettings()));
#endif
#ifdef Q_OS_WIN32
	for(QAction *driver : drivers) {
		connect(driver, &QAction::triggered, this, [this, cfg, driver](bool checked) {
			if(checked) {
				cfg->setTabletDriver(driver->property("tabletdriver").toInt());
			}
		});
	}
#endif

	// clang-format on
	connect(
		expandup, &QAction::triggered, this,
		std::bind(
			&MainWindow::resizeCanvas, this,
			int(dialogs::ResizeDialog::ExpandDirection::Up)));
	connect(
		expanddown, &QAction::triggered, this,
		std::bind(
			&MainWindow::resizeCanvas, this,
			int(dialogs::ResizeDialog::ExpandDirection::Down)));
	connect(
		expandleft, &QAction::triggered, this,
		std::bind(
			&MainWindow::resizeCanvas, this,
			int(dialogs::ResizeDialog::ExpandDirection::Left)));
	connect(
		expandright, &QAction::triggered, this,
		std::bind(
			&MainWindow::resizeCanvas, this,
			int(dialogs::ResizeDialog::ExpandDirection::Right)));

	QMenu *editmenu = menuBar()->addMenu(tr("Edit"));
	editmenu->addAction(undo);
	editmenu->addAction(redo);
	editmenu->addSeparator();
	editmenu->addAction(cutlayer);
	editmenu->addAction(copy);
	editmenu->addAction(copyMerged);
	editmenu->addAction(copylayer);
	editmenu->addAction(paste);
	editmenu->addAction(pasteCentered);
	editmenu->addAction(pastefile);
#ifndef SINGLE_MAIN_WINDOW
	editmenu->addAction(pickFromScreen);
#endif
	editmenu->addSeparator();

	editmenu->addAction(resize);
	QMenu *expandmenu = editmenu->addMenu(tr("&Expand Canvas"));
	expandmenu->addAction(expandup);
	expandmenu->addAction(expanddown);
	expandmenu->addAction(expandleft);
	expandmenu->addAction(expandright);
	QMenu *backgroundmenu = editmenu->addMenu(tr("Canvas Background"));
	backgroundmenu->addAction(canvasBackground);
	backgroundmenu->addAction(setLocalBackground);
	backgroundmenu->addAction(clearLocalBackground);
	// clang-format off
	connect(backgroundmenu, &QMenu::aboutToShow, this, &MainWindow::updateBackgroundActions);

	editmenu->addSeparator();
	editmenu->addAction(deleteAnnotations);
	editmenu->addAction(cleararea);

	QMenu *settingsMenu = menuBar()->addMenu(tr("Settings"));
	QMenu *aiSettingsMenu = settingsMenu->addMenu(tr("AI"));
	settingsMenu->addSeparator();
	settingsMenu->addAction(brushSettings);
	settingsMenu->addAction(inputSettings);
	#ifdef Q_OS_WIN32
	QMenu *driverMenu = settingsMenu->addMenu(QIcon::fromTheme("input-tablet"), tr("Tablet Driver"));
	for(QAction *driver : drivers) {
		driverMenu->addAction(driver);
	}
	connect(driverMenu, &QMenu::aboutToShow, this, [this, cfg, drivers]() {
		int mode = cfg->getTabletDriver();
		for(QAction *driver : drivers) {
			QSignalBlocker blocker(driver);
			driver->setChecked(driver->property("tabletdriver").toInt() == mode);
		}
	});
	#endif
	settingsMenu->addSeparator();
	settingsMenu->addAction(preferences);
	#ifdef Q_OS_MACOS
	settingsMenu->addAction(macPreferences);
	#endif

	m_toolBarEdit = new QToolBar(tr("Edit Tools"));
	m_toolBarEdit->setObjectName("edittoolsbar");
	toggletoolbarmenu->addAction(m_toolBarEdit->toggleViewAction());
	m_toolBarEdit->addAction(undo);
	m_toolBarEdit->addAction(redo);
	m_toolBarEdit->addAction(cutlayer);
	m_toolBarEdit->addAction(copylayer);
	m_toolBarEdit->addAction(paste);
	m_toolBarEdit->addWidget(m_dualColorButton);

	//
	// View menu
	//
	// clang-format on
#if defined(Q_OS_ANDROID) && defined(KRITA_QT_SCREEN_DENSITY_ADJUSTMENT)
	QAction *interfaceScaleAction =
		makeAction("interfacescale", tr("Interface scale…"))
			.icon("monitor")
			.noDefaultShortcut();
#endif
	QAction *layoutsAction =
		makeAction("layouts", tr("&Layouts...")).icon("window_").shortcut("F9");

	QAction *toolbartoggles = new QAction(tr("&Toolbars"), this);
	toolbartoggles->setMenu(toggletoolbarmenu);

	QAction *toolbarconfig =
		makeAction("toolbarconfig", tr("Configure drawing toolbar…"))
			.noDefaultShortcut();

	QAction *docktoggles = new QAction(tr("&Docks"), this);
	docktoggles->setMenu(toggledockmenu);

	QAction *smallScreenSideToolbar =
		makeAction("smallscreensidetoolbar", tr("Always show side toolbar"))
			.noDefaultShortcut()
			.checkable()
			.checked()
			.remembered();
	QAction *smallScreenBottomToolbar =
		makeAction("smallscreenbottomtoolbar", tr("Always show bottom toolbar"))
			.noDefaultShortcut()
			.checkable()
			.checked()
			.remembered();
	QAction *smallScreenLeftyMode =
		makeAction("smallscreenleftymode", tr("Left-handed mode"))
			.noDefaultShortcut()
			.checkable();
	// clang-format off

	QAction *toggleChat = makeAction("togglechat", tr("Chat")).shortcut("Alt+C").checked();

	QAction *moveleft = makeAction("moveleft", tr("Move Canvas Left")).noDefaultShortcut().autoRepeat();
	QAction *moveright = makeAction("moveright", tr("Move Canvas Right")).noDefaultShortcut().autoRepeat();
	QAction *moveup = makeAction("moveup", tr("Move Canvas Up")).noDefaultShortcut().autoRepeat();
	QAction *movedown = makeAction("movedown", tr("Move Canvas Down")).noDefaultShortcut().autoRepeat();
#ifdef Q_OS_MAC
#	define ZOOM_IN_SHORTCUT QKeySequence::ZoomIn
#else
#	define ZOOM_IN_SHORTCUT QKeySequence("Ctrl++"), QKeySequence("Ctrl+=")
#endif
	QAction *zoomin = makeAction("zoomin", tr("Zoom &In")).icon("zoom-in").shortcut(ZOOM_IN_SHORTCUT).autoRepeat();
	QAction *zoomincenter = makeAction("zoomincenter", tr("Zoom In On Center")).noDefaultShortcut().autoRepeat();
	QAction *zoomout = makeAction("zoomout", tr("Zoom &Out")).icon("zoom-out").shortcut(QKeySequence::ZoomOut).autoRepeat();
	QAction *zoomoutcenter = makeAction("zoomoutcenter", tr("Zoom Out From Center")).noDefaultShortcut().autoRepeat();
	QAction *zoomorig = makeAction("zoomone", tr("&Reset Zoom")).icon("zoom-original").shortcut(QKeySequence("ctrl+0"));
	QAction *zoomorigcenter = makeAction("zoomonecenter", tr("Reset Zoom At Center")).noDefaultShortcut();
	// clang-format on
	QAction *zoomfit = makeAction("zoomfit", tr("&Fit Canvas"))
						   .icon("zoom-select")
						   .noDefaultShortcut();
	QAction *zoomfitwidth = makeAction("zoomfitwidth", tr("Fit Canvas &Width"))
								.icon("zoom-fit-width")
								.noDefaultShortcut();
	QAction *zoomfitheight =
		makeAction("zoomfitheight", tr("Fit Canvas &Height"))
			.icon("zoom-fit-height")
			.noDefaultShortcut();
	QAction *rotateorig = makeAction("rotatezero", tr("&Reset Canvas Rotation"))
							  .icon("transform-rotate")
							  .shortcut(QKeySequence("ctrl+r"));
	QAction *rotatecw = makeAction("rotatecw", tr("Rotate Canvas Clockwise"))
							.shortcut(QKeySequence("shift+."))
							.icon("drawpile_rotate_right")
							.autoRepeat();
	QAction *rotateccw =
		makeAction("rotateccw", tr("Rotate Canvas Counter-Clockwise"))
			.shortcut(QKeySequence("shift+,"))
			.icon("drawpile_rotate_left")
			.autoRepeat();

	QAction *viewmirror =
		makeAction("viewmirror", tr("Mirror Canvas"))
			.icon("drawpile_mirror")
			.statusTip(tr("Mirror the canvas horizontally"))
			.shortcutWithSearchText(
				tr("mirror/flip canvas horizontally"), QKeySequence("V"))
			.checkable();
	QAction *viewflip =
		makeAction("viewflip", tr("Flip Canvas"))
			.icon("drawpile_flip")
			.statusTip(tr("Flip the canvas upside-down"))
			.shortcutWithSearchText(
				tr("mirror/flip canvas vertically"), QKeySequence("C"))
			.checkable();
	// clang-format off

	QAction *showannotations = makeAction("showannotations", tr("Show &Annotations")).noDefaultShortcut().checked().remembered();
	QAction *showusermarkers = makeAction("showusermarkers", tr("Show User &Pointers")).noDefaultShortcut().checked().remembered();
	QAction *showusernames = makeAction("showmarkernames", tr("Show Names")).noDefaultShortcut().checked().remembered();
	QAction *showuserlayers = makeAction("showmarkerlayers", tr("Show Layers")).noDefaultShortcut().checked().remembered();
	QAction *showuseravatars = makeAction("showmarkeravatars", tr("Show Avatars")).noDefaultShortcut().checked().remembered();
	QAction *evadeusercursors = makeAction("evadeusercursors", tr("Hide From Cursor")).noDefaultShortcut().checked().remembered();
	QAction *showlasers = makeAction("showlasers", tr("Show La&ser Trails")).noDefaultShortcut().checked().remembered();
	QAction *showgrid = makeAction("showgrid", tr("Show Pixel &Grid")).noDefaultShortcut().checked().remembered();
	QAction *showrulers = makeAction("showrulers", tr("Show &Rulers")).noDefaultShortcut().checkable().remembered();
	// clang-format on
	QAction *showselectionmask =
		makeAction("showselectionmask", tr("Show Selection &Mask"))
			.statusTip(
				tr("Toggle selection display between a mask and an outline"))
			.noDefaultShortcut()
			.checkable()
			.remembered();
	QAction *setselectionmaskcolor =
		makeAction("setselectionmaskcolor", tr("Set Selection Mask &Color…"))
			.statusTip(tr("Change the color tint of the selection mask"))
			.icon("color-picker")
			.noDefaultShortcut();
	QAction *showactionbar =
		makeAction("showactionbar", tr("Show Selection Action Bar"))
			.noDefaultShortcut()
			.checkable()
			.checked();
	QAction *actionbartopleft =
		makeAction("actionbartopleft", tr("Top-left"))
			.noDefaultShortcutWithTitle(tr("Selection action bar top-left"))
			.checkable();
	QAction *actionbartopcenter =
		makeAction("actionbartopcenter", tr("Top"))
			.noDefaultShortcutWithTitle(tr("Selection action bar top"))
			.checkable();
	QAction *actionbartopright =
		makeAction("actionbartopright", tr("Top-right"))
			.noDefaultShortcutWithTitle(tr("Selection action bar top-right"))
			.checkable();
	QAction *actionbarbottomleft =
		makeAction("actionbarbottomleft", tr("Bottom-left"))
			.noDefaultShortcutWithTitle(tr("Selection action bar bottom-left"))
			.checkable();
	QAction *actionbarbottomcenter =
		makeAction("actionbarbottomcenter", tr("Bottom"))
			.noDefaultShortcutWithTitle(tr("Selection action bar bottom"))
			.checkable();
	QAction *actionbarbottomright =
		makeAction("actionbarbottomright", tr("Bottom-right"))
			.noDefaultShortcutWithTitle(tr("Selection action bar bottom-right"))
			.checkable();
#ifdef SINGLE_MAIN_WINDOW
	QAction *fittoscreen =
		makeAction("fittoscreen", tr("&Fit to Screen")).noDefaultShortcut();
#endif
#if (!defined(Q_OS_MACOS) && !defined(SINGLE_MAIN_WINDOW)) ||                  \
	defined(__EMSCRIPTEN__)
#	ifdef __EMSCRIPTEN__
#		define FULLSCREEN_SHORTCUT                                            \
			QKeySequence(Qt::ALT | Qt::Key_Return),                            \
				QKeySequence(Qt::ALT | Qt::Key_Enter)
#	else
#		define FULLSCREEN_SHORTCUT QKeySequence::FullScreen
#	endif
	QAction *fullscreen = makeAction("fullscreen", tr("&Full Screen"))
							  .shortcut(FULLSCREEN_SHORTCUT)
							  .checkable();
#endif

#if defined(Q_OS_ANDROID) && defined(KRITA_QT_SCREEN_DENSITY_ADJUSTMENT)
	connect(
		interfaceScaleAction, &QAction::triggered, &dpApp(),
		&DrawpileApp::showAndroidScalingDialog);
#endif
	connect(
		layoutsAction, &QAction::triggered, this,
		&MainWindow::showLayoutsDialog);
	connect(
		toolbarconfig, &QAction::triggered, this,
		&MainWindow::showToolBarConfigDialog);
	connect(
		m_statusChatButton, &QToolButton::clicked, toggleChat,
		&QAction::trigger);

	connect(
		m_chatbox, &widgets::ChatBox::requestUserInfo, this,
		&MainWindow::showUserInfoDialog);
	connect(
		m_chatbox, &widgets::ChatBox::requestCurrentBrush, this,
		&MainWindow::requestCurrentBrush);
	connect(
		m_chatbox, &widgets::ChatBox::expandedChanged, toggleChat,
		&QAction::setChecked);
	connect(
		m_chatbox, &widgets::ChatBox::expandedChanged, m_statusChatButton,
		&QToolButton::hide);
	connect(
		m_chatbox, &widgets::ChatBox::expandPlease, toggleChat,
		&QAction::trigger);

	connect(toggleChat, &QAction::triggered, this, [this, cfg](bool show) {
		if(m_smallScreenMode) {
			HudAction action;
			action.type = HudAction::Type::ToggleChat;
			handleToggleAction(action);
		} else {
			if(show) {
				QByteArray state = cfg->getLastWindowViewState();
				if(!state.isEmpty()) {
					m_splitter->restoreState(state);
				}

				if(m_chatbox->isCollapsed()) {
					int h = height();
					m_splitter->setSizes({h * 2 / 3, h / 3});
				}

				m_chatbox->focusInput();
				m_saveSplitterDebounce.start();
			} else {
				saveSplitterState();
				m_splitter->setSizes({1, 0});
				m_canvasView->viewWidget()->setFocus();
			}
		}
	});
	connect(
		m_chatbox, &widgets::ChatBox::muteChanged, this,
		&MainWindow::setNotificationsMuted);

	connect(
		smallScreenSideToolbar, &QAction::triggered, this,
		&MainWindow::updateSmallScreenToolBarVisibility);
	connect(
		smallScreenBottomToolbar, &QAction::triggered, this,
		&MainWindow::updateSmallScreenToolBarVisibility);
	connect(
		showrulers, &QAction::toggled, m_canvasFrame,
		&widgets::CanvasFrame::setShowRulers);
	connect(
		showactionbar, &QAction::triggered, this,
		std::bind(&MainWindow::setActionBarEnabled, this, _1, true));
	connect(
		showselectionmask, &QAction::toggled, this,
		&MainWindow::updateSelectionMaskVisibility);
	connect(
		setselectionmaskcolor, &QAction::triggered, this,
		&MainWindow::showSelectionMaskColorPicker);

	QActionGroup *actionBarLocations = new QActionGroup(this);
	actionBarLocations->addAction(actionbartopleft);
	actionBarLocations->addAction(actionbartopcenter);
	actionBarLocations->addAction(actionbartopright);
	actionBarLocations->addAction(actionbarbottomleft);
	actionBarLocations->addAction(actionbarbottomcenter);
	actionBarLocations->addAction(actionbarbottomright);
	connect(
		actionBarLocations, &QActionGroup::triggered, this,
		&MainWindow::onActionBarLocationActionTriggered);

	m_canvasView->connectActions(
		{moveleft,		moveright,		moveup,			 movedown,
		 zoomin,		zoomincenter,	zoomout,		 zoomoutcenter,
		 zoomorig,		zoomorigcenter, zoomfit,		 zoomfitwidth,
		 zoomfitheight, rotateorig,		rotatecw,		 rotateccw,
		 viewflip,		viewmirror,		showgrid,		 showusermarkers,
		 showusernames, showuserlayers, showuseravatars, evadeusercursors});

#ifdef SINGLE_MAIN_WINDOW
	connect(fittoscreen, &QAction::triggered, this, &MainWindow::refitWindow);
#endif
#if (!defined(Q_OS_MACOS) && !defined(SINGLE_MAIN_WINDOW)) ||                  \
	defined(__EMSCRIPTEN__)
	connect(
		fullscreen, &QAction::triggered, this, &MainWindow::toggleFullscreen);
#endif
	// clang-format off

	connect(showannotations, &QAction::toggled, this, &MainWindow::setShowAnnotations);
	connect(showlasers, &QAction::toggled, this, &MainWindow::setShowLaserTrails);

	m_viewstatus->setActions(viewflip, viewmirror, rotateorig, {zoomorig, zoomfit, zoomfitwidth, zoomfitheight});

	// clang-format on
	QMenu *viewmenu = menuBar()->addMenu(tr("View"));
#if defined(Q_OS_ANDROID) && defined(KRITA_QT_SCREEN_DENSITY_ADJUSTMENT)
	viewmenu->addAction(interfaceScaleAction);
#endif
	viewmenu->addAction(layoutsAction);
	m_desktopModeActions->addAction(layoutsAction);
	viewmenu->addAction(toolbartoggles);
	viewmenu->addAction(docktoggles);
	m_desktopModeActions->addAction(docktoggles);
	viewmenu->addAction(toggleChat);
	m_desktopModeActions->addAction(toggleChat);
	viewmenu->addAction(smallScreenSideToolbar);
	m_smallScreenModeActions->addAction(smallScreenSideToolbar);
	viewmenu->addAction(smallScreenBottomToolbar);
	m_smallScreenModeActions->addAction(smallScreenBottomToolbar);
	viewmenu->addAction(smallScreenLeftyMode);
	m_smallScreenModeActions->addAction(smallScreenLeftyMode);
	viewmenu->addSeparator();

	QMenu *zoommenu = viewmenu->addMenu(tr("&Zoom"));
	zoommenu->addAction(zoomin);
	zoommenu->addAction(zoomout);
	zoommenu->addAction(zoomorig);
	zoommenu->addAction(zoomfit);
	zoommenu->addAction(zoomfitwidth);
	zoommenu->addAction(zoomfitheight);

	QMenu *rotatemenu = viewmenu->addMenu(tr("Rotation"));
	rotatemenu->addAction(rotateorig);
	rotatemenu->addAction(rotatecw);
	rotatemenu->addAction(rotateccw);

	viewmenu->addAction(viewflip);
	viewmenu->addAction(viewmirror);

	viewmenu->addSeparator();

	m_layerViewNormal = makeAction("layerviewnormal", tr("Normal View"))
							.statusTip(tr("Show all layers normally"))
							.noDefaultShortcut()
							.checkable()
							.checked();
	m_layerViewCurrentLayer =
		makeAction("layerviewcurrentlayer", tr("Layer View"))
			.statusTip(tr("Show only the current layer"))
			.shortcut("Home")
			.checkable();
	m_layerViewCurrentGroup =
		makeAction("layerviewcurrentgroup", tr("Group View"))
			.statusTip(tr("Show only the current parent layer group"))
			.shortcut("Ctrl+Home")
			.checkable();
	m_layerViewCurrentFrame =
		makeAction("layerviewcurrentframe", tr("Frame View"))
			.statusTip(tr("Show only layers in the current frame"))
			.shortcut("Shift+Home")
			.checkable();
	QAction *layerViewNotices =
		makeAction("layerviewnotices", tr("On-canvas view mode notices"))
			.noDefaultShortcut()
			.checkable();
	QAction *layerUncensor =
		makeAction("layerviewuncensor", tr("Show Censored Layers"))
			.noDefaultShortcut()
			.checkable();
	m_lastLayerViewMode = m_layerViewNormal;

	QActionGroup *layerViewModeGroup = new QActionGroup(this);
	layerViewModeGroup->setExclusive(true);
	layerViewModeGroup->addAction(m_layerViewNormal);
	layerViewModeGroup->addAction(m_layerViewCurrentLayer);
	layerViewModeGroup->addAction(m_layerViewCurrentGroup);
	layerViewModeGroup->addAction(m_layerViewCurrentFrame);

	QMenu *layerViewMenu = viewmenu->addMenu(tr("Layer View Mode"));
	layerViewMenu->addAction(m_layerViewNormal);
	layerViewMenu->addAction(m_layerViewCurrentLayer);
	layerViewMenu->addAction(m_layerViewCurrentGroup);
	layerViewMenu->addAction(m_layerViewCurrentFrame);
	layerViewMenu->addSeparator();
	layerViewMenu->addAction(layerViewNotices);
	viewmenu->addAction(layerUncensor);

	// clang-format off
	connect(layerViewModeGroup, &QActionGroup::triggered, this, &MainWindow::toggleLayerViewMode);
	connect(layerUncensor, &QAction::toggled, this, &MainWindow::updateLayerViewMode);

	viewmenu->addSeparator();
	QMenu *userpointermenu = viewmenu->addMenu(tr("User Pointers"));
	userpointermenu->addAction(showusermarkers);
	userpointermenu->addAction(showlasers);
	userpointermenu->addSeparator();
	userpointermenu->addAction(showusernames);
	userpointermenu->addAction(showuserlayers);
	userpointermenu->addAction(showuseravatars);
	userpointermenu->addAction(evadeusercursors);

	QMenu *stayTimeMenu = userpointermenu->addMenu(tr("Stay Time"));
	QActionGroup *stayTimeGroup = new QActionGroup(this);
	stayTimeGroup->setExclusive(true);
	QPair<QString, int> stayTimeActions[] = {
		{tr("1 Second", "user pointer stay time"), 1000},
		{tr("10 Seconds", "user pointer stay time"), 10000},
		{tr("1 Minute", "user pointer stay time"), 60000},
		{tr("1 Hour", "user pointer stay time"), 3600000},
		{tr("Forever", "user pointer stay time"), -1},
	};
	int userMarkerPersistence = cfg->getUserMarkerPersistence();
	for(const QPair<QString, int> &p : stayTimeActions) {
		QAction *action = stayTimeMenu->addAction(p.first);
		action->setCheckable(true);
		int persistence = p.second;
		action->setChecked(userMarkerPersistence == persistence);
		stayTimeGroup->addAction(action);
		connect(
			action, &QAction::toggled, this, [cfg, persistence](bool checked) {
				if(checked) {
					cfg->setUserMarkerPersistence(persistence);
				}
			});
	}

	// clang-format on
	viewmenu->addAction(showannotations);

	viewmenu->addAction(showgrid);
	viewmenu->addAction(showrulers);
	viewmenu->addAction(showselectionmask);
	viewmenu->addAction(setselectionmaskcolor);

	viewmenu->addSeparator();
	viewmenu->addAction(showactionbar);

	m_actionBarLocationMenu =
		viewmenu->addMenu(tr("Selection Action Bar Location"));
	m_actionBarLocationMenu->addActions(actionBarLocations->actions());

	viewmenu->addSeparator();
#ifdef SINGLE_MAIN_WINDOW
	viewmenu->addAction(fittoscreen);
#endif
#if (!defined(Q_OS_MACOS) && !defined(SINGLE_MAIN_WINDOW)) ||                  \
	defined(__EMSCRIPTEN__)
	viewmenu->addAction(fullscreen);
#	ifdef __EMSCRIPTEN__
	connect(viewmenu, &QMenu::aboutToShow, this, [fullscreen] {
		QSignalBlocker blocker(fullscreen);
		fullscreen->setEnabled(browser::isFullscreenSupported());
		fullscreen->setChecked(browser::isFullscreen());
	});
#	else
	connect(viewmenu, &QMenu::aboutToShow, this, [this, fullscreen] {
		QSignalBlocker blocker(fullscreen);
		fullscreen->setChecked(windowState().testFlag(Qt::WindowFullScreen));
	});
#	endif
#endif
	// clang-format off

	//
	// Layer menu
	//
	QAction *layerAdd = makeAction("layeradd", tr("New Layer")).shortcut("Shift+Ctrl+Insert").icon("list-add");
	QAction *groupAdd = makeAction("groupadd", tr("New Layer Group")).icon("folder-new").noDefaultShortcut();
	QAction *layerDupe = makeAction("layerdupe", tr("Duplicate Layer")).icon("edit-copy").noDefaultShortcut();
	QAction *layerMerge = makeAction("layermerge", tr("Merge Layer")).icon("arrow-down-double").noDefaultShortcut();
	// clang-format on
	QVector<QAction *> layerColors;
	for(const utils::MarkerColor &mc : utils::markerColors()) {
		layerColors.append(makeAction(mc.layerActionName, mc.layerActionText)
							   .icon(utils::makeColorIcon(16, mc.color))
							   .property("markercolor", mc.color)
							   .noDefaultShortcut());
	}
	// clang-format off
	QAction *layerProperties = makeAction("layerproperties", tr("Layer Properties…")).icon("configure").noDefaultShortcut();
	QAction *layerDelete = makeAction("layerdelete", tr("Delete Layer")).icon("trash-empty").noDefaultShortcut();
	QAction *layerVisibilityToggle = makeAction("layervisibilitytoggle", tr("Toggle Layer &Visibility")).icon("view-visible").noDefaultShortcut();
	QAction *layerSketchToggle = makeAction("layersketchtoggle", tr("Toggle Layer &Sketch Mode")).icon("draw-freehand").noDefaultShortcut();
	QAction *layerSetFillSource = makeAction("layersetfillsource", tr("Set as Fill Source")).icon("tag").noDefaultShortcut();
	QAction *layerClearFillSource = makeAction("layerclearfillsource", tr("Clear Fill Source")).icon("tag-delete").noDefaultShortcut();

	// clang-format on
	QAction *layerAlphaBlend =
		makeAction(
			"layeralphablend", QCoreApplication::translate(
								   "dialogs::LayerProperties", "Blend alpha"))
			.checkable()
			.checked()
			.noDefaultShortcut();
	QAction *layerAlphaPreserve =
		makeAction(
			"layeralphapreserve",
			QCoreApplication::translate(
				"dialogs::LayerProperties", "Inherit alpha"))
			.checkable()
			.noDefaultShortcut();
	QAction *layerClip =
		makeAction(
			"layerclip", QCoreApplication::translate(
							 "dialogs::LayerProperties", "Clip to layer below"))
			.checkable()
			.noDefaultShortcut();
	QActionGroup *layerAlphaGroup = new QActionGroup(this);
	layerAlphaGroup->addAction(layerAlphaBlend);
	layerAlphaGroup->addAction(layerAlphaPreserve);
	layerAlphaGroup->addAction(layerClip);
	QAction *layerAutomaticAlphaPreserve =
		makeAction(
			"layerautomaticalphapreserve", tr("Automatically inherit alpha"))
			.statusTip(tr("Inherit and preserve alpha based on blend mode"))
			.checkable()
			.noDefaultShortcut();

	QAction *layerAlphaLock =
		makeAction("layeralphalock", tr("Alpha lock layer for you"))
			.checkable()
			.noDefaultShortcut();
	QAction *layerCensor = makeAction("layercensor", tr("Censor layer"))
							   .checkable()
							   .noDefaultShortcut();
	QAction *layerCensorLocal =
		makeAction("layercensorlocal", tr("Block layer (censor for you)"))
			.checkable()
			.noDefaultShortcut();
	QAction *layerLockAll =
		makeAction("layerlockall", tr("Lock layer entirely"))
			.checkable()
			.noDefaultShortcut();
	QAction *layerLockContent =
		makeAction("layerlockcontent", tr("Lock layer content"))
			.checkable()
			.noDefaultShortcut();
	QAction *layerLockProps =
		makeAction("layerlockprops", tr("Lock layer properties"))
			.checkable()
			.noDefaultShortcut();
	QAction *layerLockMove =
		makeAction("layerlockmove", tr("Lock layer position"))
			.checkable()
			.noDefaultShortcut();
	// clang-format off

	QAction *layerUpAct = makeAction("layer-up", tr("Select Above")).shortcut("Shift+X").autoRepeat();
	QAction *layerDownAct = makeAction("layer-down", tr("Select Below")).shortcut("Shift+Z").autoRepeat();

	QAction *layerCheckToggle =
		makeAction("layerchecktoggle", tr("&Toggle Layer Check State"))
			.noDefaultShortcut();
	QAction *layerCheckAll =
		makeAction("layercheckall", tr("&Check All")).noDefaultShortcut();
	QAction *layerUncheckAll =
		makeAction("layeruncheckall", tr("&Uncheck All")).noDefaultShortcut();

	connect(layerUpAct, &QAction::triggered, m_dockLayers, &docks::LayerList::selectAbove);
	connect(layerDownAct, &QAction::triggered, m_dockLayers, &docks::LayerList::selectBelow);

	// clang-format on
	QMenu *layerMenu = menuBar()->addMenu(tr("Layer"));
	layerMenu->addAction(layerAdd);
	layerMenu->addAction(groupAdd);
	layerMenu->addAction(layerDupe);
	layerMenu->addAction(layerMerge);
	layerMenu->addAction(layerDelete);
	QMenu *layerColorMenu = layerMenu->addMenu(
		utils::makeColorIcon(16, QColor()), tr("Layer Color Marker"));
	for(QAction *layerColor : layerColors) {
		layerColorMenu->addAction(layerColor);
	}
	layerMenu->addAction(layerProperties);
	layerMenu->addAction(layerVisibilityToggle);
	layerMenu->addAction(layerSketchToggle);
	layerMenu->addAction(layerSetFillSource);
	layerMenu->addAction(layerClearFillSource);

	layerMenu->addSeparator();
	layerMenu->addAction(layerAlphaBlend);
	layerMenu->addAction(layerAlphaPreserve);
	layerMenu->addAction(layerClip);
	layerMenu->addAction(layerAutomaticAlphaPreserve);

	layerMenu->addSeparator();
	QMenu *layerLockMenu =
		layerMenu->addMenu(QIcon::fromTheme("object-locked"), tr("&Locks"));
	layerLockMenu->addAction(layerAlphaLock);
	layerLockMenu->addSeparator();
	layerLockMenu->addAction(layerLockAll);
	layerLockMenu->addAction(layerLockContent);
	layerLockMenu->addAction(layerLockProps);
	layerLockMenu->addAction(layerLockMove);
	layerMenu->addAction(layerCensor);
	layerMenu->addAction(layerCensorLocal);

	layerMenu->addSeparator();
	layerMenu->addAction(layerUpAct);
	layerMenu->addAction(layerDownAct);

	layerMenu->addSeparator();
	layerMenu->addAction(layerCheckToggle);
	layerMenu->addAction(layerCheckAll);
	layerMenu->addAction(layerUncheckAll);

	connect(
		layerMenu, &QMenu ::aboutToShow, m_dockLayers,
		&docks::LayerList::updateLayerColorMenuIcon);

	//
	// Select menu
	//
	QAction *selectall = makeAction("selectall", tr("Select &All"))
							 .shortcut(QKeySequence::SelectAll)
							 .icon("edit-select-all");
	QAction *selectnone =
		makeAction("selectnone", tr("&Deselect"))
			.shortcut(QKeySequence("Ctrl+Shift+A"), QKeySequence("Ctrl+D"))
			.icon("edit-select-none");
	QAction *selectinvert = makeAction("selectinvert", tr("&Invert Selection"))
								.noDefaultShortcut()
								.icon("edit-select-invert");
	QAction *selectlayerbounds =
		makeAction("selectlayerbounds", tr("Select Layer &Bounds"))
			.shortcut(Qt::SHIFT | Qt::Key_B)
			.icon("select-rectangular");
	QAction *selectlayercontents =
		makeAction("selectlayercontents", tr("&Layer to Selection"))
			.shortcut(Qt::SHIFT | Qt::Key_L)
			.icon("edit-image");
	QAction *selectalter =
		//: "Feather" is a verb here, referring to blurring the selection.
		makeAction("selectalter", tr("&Expand/Shrink/Feather Selection"))
			.noDefaultShortcut()
			.icon("zoom-select");
	QAction *fillfgarea = makeAction("fillfgarea", tr("Fill Selection"))
							  .shortcut(CTRL_KEY | Qt::Key_Comma);
	QAction *recolorarea = makeAction("recolorarea", tr("Recolor Selection"))
							   .shortcut(CTRL_KEY | Qt::SHIFT | Qt::Key_Comma);
	QAction *colorerasearea =
		makeAction("colorerasearea", tr("Color Erase Selection"))
			.shortcut(Qt::SHIFT | Qt::Key_Delete);
	QAction *lightnesstoalphaarea =
		makeAction("lightnesstoalphaarea", tr("Selection Lightness to Alpha"))
			.noDefaultShortcut();
	QAction *darknesstoalphaarea =
		makeAction("darknesstoalphaarea", tr("Selection Darkness to Alpha"))
			.noDefaultShortcut();
	QAction *selectcrop =
		makeAction("selectcrop", tr("Cr&op canvas to selection/transform…"))
			.icon(QStringLiteral("drawpile_crop"))
			.noDefaultShortcut();
	QAction *starttransform =
		makeAction("starttransform", tr("&Transform"))
			.shortcut("T")
			.icon("drawpile_transform")
			.statusTip(
				tr("Transform the selection, switch back tools afterwards"));
	QAction *starttransformmask =
		makeAction("starttransformmask", tr("Transform Selection &Mask"))
			.icon("transform-crop-and-resize")
			.statusTip(
				tr("Transform only the selection mask itself, switch "
				   "back tools afterwards"));
	QAction *transformmirror =
		makeAction("transformmirror", tr("&Mirror Transform"))
			.icon("drawpile_mirror")
			.statusTip(tr("Mirror the transformed image horizontally"))
			.noDefaultShortcutWithSearchText(
				tr("mirror/flip transformed image horizontally"));
	QAction *transformflip =
		makeAction("transformflip", tr("&Flip Transform"))
			.icon("drawpile_flip")
			.statusTip(tr("Flip the transformed image upside-down"))
			.noDefaultShortcutWithSearchText(
				tr("mirror/flip transformed image vertically"));
	QAction *transformrotatecw =
		makeAction("transformrotatecw", tr("&Rotate Transform Clockwise"))
			.icon("drawpile_rotate_right")
			.noDefaultShortcut();
	QAction *transformrotateccw =
		makeAction(
			"transformrotateccw", tr("Rotate Transform &Counter-Clockwise"))
			.icon("drawpile_rotate_left")
			.noDefaultShortcut();
	QAction *transformshrinktoview =
		makeAction("transformshrinktoview", tr("Shrink Transform to &Fit View"))
			.icon("zoom-out")
			.noDefaultShortcut();
	QAction *stamp =
		makeAction("stamp", tr("&Stamp Transform")).shortcut("Ctrl+T");
	QAction *editselection =
		makeAction("editselection", tr("Dra&w on Selection"))
			.checkable()
			.noDefaultShortcut();
	QAction *maskselection =
		makeAction(
			"maskselection", tr("Mas&k Strokes/Lasso Fills by Selection"))
			.statusTip(tr("Keep brush strokes inside the selection mask"))
			.checkable()
			.checked()
			.noDefaultShortcut();

	m_currentdoctools->addAction(selectall);
	m_currentdoctools->addAction(selectnone);
	m_currentdoctools->addAction(selectinvert);
	m_currentdoctools->addAction(selectlayerbounds);
	m_currentdoctools->addAction(selectlayercontents);
	m_currentdoctools->addAction(selectalter);
	m_currentdoctools->addAction(editselection);

	m_putimagetools->addAction(fillfgarea);
	m_putimagetools->addAction(recolorarea);
	m_putimagetools->addAction(colorerasearea);
	m_putimagetools->addAction(lightnesstoalphaarea);
	m_putimagetools->addAction(darknesstoalphaarea);
	m_putimagetools->addAction(stamp);

	m_resizetools->addAction(selectcrop);

	connect(selectall, &QAction::triggered, m_doc, &Document::selectAll);
	connect(
		selectnone, &QAction::triggered, m_doc,
		std::bind(&Document::selectNone, m_doc, true));
	connect(selectinvert, &QAction::triggered, m_doc, &Document::selectInvert);
	connect(
		selectlayerbounds, &QAction::triggered, m_doc,
		&Document::selectLayerBounds);
	connect(
		selectlayercontents, &QAction::triggered, m_doc,
		&Document::selectLayerContents);
	connect(
		selectalter, &QAction::triggered, this,
		&MainWindow::showAlterSelectionDialog);
	connect(selectcrop, &QAction::triggered, resize, &QAction::trigger);
	connect(
		fillfgarea, &QAction::triggered, this,
		std::bind(
			&MainWindow::fillAreaWithBlendMode, this, DP_BLEND_MODE_NORMAL));
	connect(
		recolorarea, &QAction::triggered, this,
		std::bind(
			&MainWindow::fillAreaWithBlendMode, this, DP_BLEND_MODE_RECOLOR));
	connect(
		colorerasearea, &QAction::triggered, this,
		std::bind(
			&MainWindow::fillAreaWithBlendMode, this,
			DP_BLEND_MODE_COLOR_ERASE));
	connect(
		lightnesstoalphaarea, &QAction::triggered, this,
		std::bind(
			&MainWindow::fillAreaWithBlendMode, this,
			DP_BLEND_MODE_LIGHT_TO_ALPHA));
	connect(
		darknesstoalphaarea, &QAction::triggered, this,
		std::bind(
			&MainWindow::fillAreaWithBlendMode, this,
			DP_BLEND_MODE_DARK_TO_ALPHA));
	connect(
		starttransform, &QAction::triggered, m_dockToolSettings,
		&docks::ToolSettings::startTransformMoveActiveLayer);
	connect(
		starttransformmask, &QAction::triggered, m_dockToolSettings,
		&docks::ToolSettings::startTransformMoveMask);
	connect(
		editselection, &QAction::triggered, m_doc->toolCtrl(),
		&tools::ToolController::setSelectionEditActive);
	connect(
		m_doc->toolCtrl(), &tools::ToolController::selectionEditActiveChanged,
		editselection, &QAction::setChecked);
	connect(
		m_doc->toolCtrl(), &tools::ToolController::selectionEditActiveChanged,
		this, &MainWindow::updateSelectTransformActions);
	connect(
		m_doc->toolCtrl(), &tools::ToolController::selectionEditActiveChanged,
		this, &MainWindow::updateSelectionMaskVisibility);
	connect(
		maskselection, &QAction::triggered, m_doc->toolCtrl(),
		&tools::ToolController::setSelectionMaskingEnabled);

	QMenu *selectMenu = menuBar()->addMenu(tr("Selection"));
	selectMenu->addAction(selectall);
	selectMenu->addAction(selectnone);
	selectMenu->addAction(selectinvert);
	selectMenu->addAction(selectlayerbounds);
	selectMenu->addAction(selectlayercontents);
	selectMenu->addAction(selectalter);
	selectMenu->addSeparator();
	selectMenu->addAction(cleararea);
	selectMenu->addAction(fillfgarea);
	selectMenu->addAction(recolorarea);
	selectMenu->addAction(colorerasearea);
	selectMenu->addAction(lightnesstoalphaarea);
	selectMenu->addAction(darknesstoalphaarea);
	selectMenu->addAction(selectcrop);
	selectMenu->addSeparator();
	selectMenu->addAction(starttransform);
	selectMenu->addAction(starttransformmask);
	selectMenu->addAction(transformmirror);
	selectMenu->addAction(transformflip);
	selectMenu->addAction(transformrotatecw);
	selectMenu->addAction(transformrotateccw);
	selectMenu->addAction(transformshrinktoview);
	selectMenu->addAction(stamp);
	selectMenu->addSeparator();
	selectMenu->addAction(editselection);
	selectMenu->addAction(showselectionmask);
	selectMenu->addAction(setselectionmaskcolor);
	selectMenu->addAction(maskselection);

	m_dockToolSettings->gradientSettings()->setActions(
		selectall, selectlayerbounds);
	m_dockToolSettings->selectionSettings()->setAction(starttransform);
	m_dockToolSettings->transformSettings()->setActions(
		transformmirror, transformflip, transformrotatecw, transformrotateccw,
		transformshrinktoview, stamp);

	//
	// AI menu
	//
	QAction *aiSceneSeparation =
		makeAction("ai-scene-separation", tr("&Color Separation..."))
			.statusTip(tr("Separate the image into luminance/color region layers"))
			.noDefaultShortcut();
	QAction *aiObjectDecomposition =
		makeAction("ai-object-decomposition", tr("&Object Decomposition..."))
			.statusTip(tr("Separate image objects and parts into movable mask-backed layers"))
			.noDefaultShortcut();
	QAction *aiBackgroundRemoval =
		makeAction("ai-background-removal", tr("&Remove Background..."))
			.statusTip(tr("Create a foreground cutout and matte from the visible canvas"))
			.noDefaultShortcut();
	QAction *aiUnderpaintBehind =
		makeAction("ai-underpaint-behind", tr("&Underpaint Behind Active Layer..."))
			.statusTip(tr("Repair the background behind the active extracted layer"))
			.noDefaultShortcut();
	QAction *aiInpaint =
		makeAction("ai-inpaint", tr("&Inpaint Selection..."))
			.statusTip(tr("Inpaint the current selection with generated variants"))
			.noDefaultShortcut();
	QAction *aiOutpaint =
		makeAction("ai-outpaint", tr("&Outpaint..."))
			.statusTip(tr("Generate content for an intentional canvas extension"))
			.noDefaultShortcut();
	QAction *aiDetailSettings =
		makeAction("ai-detail-settings", tr("Face && Body Detail Settings..."))
			.statusTip(tr("Configure targeted face, body, and hand detail passes"))
			.noDefaultShortcut();
	QAction *aiRefinerSettings =
		makeAction("ai-refiner-settings", tr("&Refiner Settings..."))
			.statusTip(tr("Configure the optional SDXL refiner stage"))
			.noDefaultShortcut();
	QAction *aiModelManager =
		makeAction("ai-model-manager", tr("&Model Manager..."))
			.statusTip(tr("Review local AI models and storage"))
			.noDefaultShortcut();
	QAction *aiPreferences =
		makeAction("ai-preferences", tr("AI &Preferences..."))
			.statusTip(tr("Configure restoration defaults and candidate counts"))
			.noDefaultShortcut();

	connect(aiSceneSeparation, &QAction::triggered, this, [=] {
		canvas::CanvasModel *canvas = m_doc->canvas();
		if(!canvas) {
			showErrorMessage(tr("No canvas is available for color separation."));
			return;
		}
		if(!m_doc->checkPermission(DP_FEATURE_PUT_IMAGE)) {
			return;
		}
		canvas::AclState *aclState = canvas->aclState();
		if(!aclState ||
		   !(aclState->canUseFeature(DP_FEATURE_EDIT_LAYERS) ||
			 aclState->canUseFeature(DP_FEATURE_OWN_LAYERS))) {
			m_doc->permissionDenied(DP_FEATURE_EDIT_LAYERS);
			return;
		}

		drawdance::CanvasState canvasState =
			canvas->paintEngine()->viewCanvasState();
		const QRect canvasBounds(QPoint(), canvasState.size());
		if(canvasBounds.isEmpty()) {
			showErrorMessage(tr("The canvas is empty."));
			return;
		}

		static ColorSeparationOptions lastColorSeparationSettings;
		ColorSeparationOptions options = lastColorSeparationSettings;
		if(!showColorSeparationOptionsDialog(
			   this, canvasBounds.size(), options)) {
			return;
		}
		lastColorSeparationSettings = options;

		drawdance::ViewModeBuffer viewModeBuffer;
		QImage sourceImage = canvas->paintEngine()->getFlatImage(
			viewModeBuffer, canvasState, true, true, &canvasBounds);
		if(sourceImage.isNull()) {
			showErrorMessage(
				tr("Could not export the source image for color separation."));
			return;
		}

		QTemporaryDir assetDir(
			QDir::temp().filePath(QStringLiteral("underpaint-ai-assets-XXXXXX")));
		if(!assetDir.isValid()) {
			showErrorMessage(tr("Could not create AI asset directory."));
			return;
		}
		assetDir.setAutoRemove(false);
		const QString sourcePath =
			QDir(assetDir.path()).filePath(QStringLiteral("source.png"));
		if(!sourceImage.save(sourcePath, "PNG")) {
			showErrorMessage(tr("Could not write AI source asset."));
			return;
		}

		const int sourceLayerId = validInpaintAnchorLayer(
			canvas->layerlist(), m_doc->toolCtrl()->activeLayer());
		ai::JobRequest request =
			ai::JobRequest::create(ai::Operation::SceneSeparation);
		ai::JobAsset sourceAsset;
		sourceAsset.role = QStringLiteral("source-image");
		sourceAsset.path = sourcePath;
		sourceAsset.mimeType = QStringLiteral("image/png");
		sourceAsset.metadata = QJsonObject{
			{QStringLiteral("colorSpace"), QStringLiteral("srgb")},
		};
		request.inputs = {sourceAsset};
		request.region = QJsonObject{
			{QStringLiteral("x"), canvasBounds.x()},
			{QStringLiteral("y"), canvasBounds.y()},
			{QStringLiteral("width"), canvasBounds.width()},
			{QStringLiteral("height"), canvasBounds.height()},
		};
		request.parameters = QJsonObject{
			{QStringLiteral("maxRegions"), options.maxRegions},
			{QStringLiteral("maxMasks"), options.maxRegions},
			{QStringLiteral("minRegionAreaPct"), options.minRegionAreaPct},
			{QStringLiteral("decompositionDepth"), options.decompositionDepth},
			{QStringLiteral("groupRepeatedRegions"), options.groupRepeatedRegions},
		};
		request.preferences = QJsonObject{
			{QStringLiteral("maxRenderEdge"), 1024},
			{QStringLiteral("cacheGuides"), true},
			{QStringLiteral("safe4070Mode"), true},
		};
		request.source = QJsonObject{
			{QStringLiteral("activeLayerId"), sourceLayerId},
			{QStringLiteral("selectionSource"), QStringLiteral("full-canvas")},
		};
			request.provenance = QJsonObject{
				{QStringLiteral("createdBy"), QStringLiteral("underpaint")},
				{QStringLiteral("uiEntryPoint"), QStringLiteral("Power Tools/Color Separation")},
			};

		QProgressDialog *progress = new QProgressDialog(
			tr("Separating image into color regions..."), tr("Cancel"), 0,
			qMax(1, options.maxRegions), this);
		AiPreviewLabel *progressLabel = new AiPreviewLabel(progress);
		progressLabel->setText(tr("Separating image into color regions..."));
		progressLabel->setAlignment(Qt::AlignCenter);
		progressLabel->setMinimumWidth(280);
		progressLabel->setMinimumHeight(192);
		progress->setLabel(progressLabel);
		progress->setWindowTitle(tr("Color Separation"));
		progress->setWindowModality(Qt::WindowModal);
		progress->setMinimumDuration(0);
		progress->setAutoClose(false);
		progress->setAutoReset(false);
		progress->setValue(0);
		progress->show();

		auto cancelRequested = std::make_shared<std::atomic_bool>(false);
		connect(progress, &QProgressDialog::canceled, this, [cancelRequested] {
			cancelRequested->store(true);
		});

		ai::JobRunResult *result = new ai::JobRunResult;
		QPointer<AiPreviewLabel> progressLabelPointer(progressLabel);
		QPointer<QProgressDialog> progressPointer(progress);
		const int progressRegions = qMax(1, options.maxRegions);
		QThread *thread = QThread::create(
			[request, result, progressLabelPointer, progressPointer,
			 progressRegions, cancelRequested] {
			*result = ai::JobRunner::run(
				request, QString(), 15 * 60 * 1000,
				[progressLabelPointer, progressPointer,
				 progressRegions](const QJsonObject &event) {
					QMetaObject::invokeMethod(
						qApp,
						[progressLabelPointer, progressPointer, progressRegions,
						 event] {
							if(!progressLabelPointer || !progressPointer) {
								return;
							}
							const QString type =
								event.value(QStringLiteral("type")).toString();
							if(type == QStringLiteral("detail")) {
								const int candidate =
									event.value(QStringLiteral("candidate")).toInt();
								const QString status =
									event.value(QStringLiteral("status")).toString();
								const QString region =
									event.value(QStringLiteral("region")).toString();
								QString text = MainWindow::tr("Candidate %1 detail pass")
												   .arg(candidate);
								if(!status.isEmpty()) {
									text += MainWindow::tr(" - %1").arg(status);
								}
								if(!region.isEmpty()) {
									text += MainWindow::tr(" (%1)").arg(region);
								}
								progressLabelPointer->setToolTip(text);
								progressLabelPointer->setStatusText(text);
								return;
							}
							if(type != QStringLiteral("preview") &&
							   type != QStringLiteral("candidate")) {
								return;
							}
							const QString imagePath =
								event.value(QStringLiteral("imagePath")).toString();
							QPixmap preview(imagePath);
							if(preview.isNull()) {
								return;
							}
							const int candidate =
								event.value(QStringLiteral("candidate")).toInt();
							QString text =
								type == QStringLiteral("preview")
									? MainWindow::tr("Region %1 preview")
										  .arg(candidate)
									: MainWindow::tr("Region %1 complete")
										  .arg(candidate);
							const QString label =
								event.value(QStringLiteral("label")).toString();
							if(!label.isEmpty()) {
								text += MainWindow::tr(" - %1").arg(label);
							}
							progressLabelPointer->setToolTip(text);
							progressLabelPointer->setPreviewPixmap(preview.scaled(
								256, 192, Qt::KeepAspectRatio,
								Qt::SmoothTransformation));
							progressPointer->setMaximum(progressRegions);
							progressPointer->setValue(
								qMin(qMax(1, candidate), progressRegions));
						},
						Qt::QueuedConnection);
				},
				cancelRequested.get());
		});
		connect(thread, &QThread::finished, this, [=] {
			progress->close();
			progress->deleteLater();

			const ai::JobRunResult jobResult = *result;
			delete result;
			thread->deleteLater();

			canvas::CanvasModel *currentCanvas = m_doc->canvas();
			if(!currentCanvas) {
				showErrorMessage(
					tr("The canvas was closed before AI results could import."));
				return;
			}
			if(jobResult.canceled) {
				return;
			}
			if(!jobResult.ok) {
				showErrorMessageWithDetails(
					tr("Color separation worker failed."),
					jobResult.errorMessage);
				return;
			}
			if(jobResult.response.candidates.isEmpty()) {
				showErrorMessage(
					tr("The AI worker returned no color separation layers."));
				return;
			}

			canvas::LayerListModel *layers = currentCanvas->layerlist();
			const int regionCount = jobResult.response.candidates.size();
			QStringList groupOrder;
			for(const ai::JobCandidate &candidate : jobResult.response.candidates) {
				const QString groupLabel = candidateRegionGroupLabel(candidate);
				if(!groupOrder.contains(groupLabel)) {
					groupOrder.append(groupLabel);
				}
			}
			const int requestedLayerIds = regionCount * 2 + groupOrder.size() + 4;
			QVector<int> layerIds = layers->getAvailableLayerIds(requestedLayerIds);
			if(layerIds.size() < requestedLayerIds) {
				showErrorMessage(
					tr("Could not allocate layers for color separation."));
				return;
			}

			const uint8_t contextId = currentCanvas->localUserId();
			int nextLayerIdIndex = 0;
			const int groupId = layerIds.at(nextLayerIdIndex++);
			const int regionSetGroupId = layerIds.at(nextLayerIdIndex++);
			const int sourceSnapshotLayerId = layerIds.at(nextLayerIdIndex++);
			const int masksGroupId = layerIds.at(nextLayerIdIndex++);
			net::MessageList messages;
			messages.append(net::makeUndoPointMessage(contextId));
			messages.append(
				net::makeLayerTreeCreateMessage(
					contextId, groupId, 0, qMax(0, sourceLayerId), 0,
					DP_MSG_LAYER_TREE_CREATE_FLAGS_GROUP,
					layers->getAvailableLayerName(
						tr("Color Separation"))));
			messages.append(
				net::makeLayerTreeCreateMessage(
					contextId, regionSetGroupId, 0, groupId, 0,
					DP_MSG_LAYER_TREE_CREATE_FLAGS_GROUP |
						DP_MSG_LAYER_TREE_CREATE_FLAGS_INTO,
					layers->getAvailableLayerName(
						candidateRegionSetLabel(jobResult))));
			messages.append(
				net::makeLayerTreeCreateMessage(
					contextId, sourceSnapshotLayerId, 0, regionSetGroupId, 0,
					DP_MSG_LAYER_TREE_CREATE_FLAGS_INTO,
					layers->getAvailableLayerName(tr("Source Snapshot"))));
			net::makePutImageMessagesCompat(
				messages, contextId, sourceSnapshotLayerId, DP_BLEND_MODE_NORMAL,
				canvasBounds.x(), canvasBounds.y(), sourceImage,
				currentCanvas->isCompatibilityMode());
			messages.append(
				net::makeLayerTreeCreateMessage(
					contextId, masksGroupId, 0, regionSetGroupId, 0,
					DP_MSG_LAYER_TREE_CREATE_FLAGS_GROUP |
						DP_MSG_LAYER_TREE_CREATE_FLAGS_INTO,
					layers->getAvailableLayerName(tr("Masks"))));

			QMap<QString, int> regionGroupIds;
			for(const QString &groupLabel : std::as_const(groupOrder)) {
				const int regionGroupId = layerIds.at(nextLayerIdIndex++);
				regionGroupIds.insert(groupLabel, regionGroupId);
				messages.append(
					net::makeLayerTreeCreateMessage(
						contextId, regionGroupId, 0, regionSetGroupId, 0,
						DP_MSG_LAYER_TREE_CREATE_FLAGS_GROUP |
							DP_MSG_LAYER_TREE_CREATE_FLAGS_INTO,
						layers->getAvailableLayerName(groupLabel)));
			}

			QVector<int> importedLayerIds;
			importedLayerIds.reserve(regionCount);
			QVector<ai::JobCandidate> importedCandidates;
			importedCandidates.reserve(regionCount);
			QVector<int> hiddenArtifactLayerIds;
			hiddenArtifactLayerIds.reserve(regionCount + 2);
			hiddenArtifactLayerIds.append(sourceSnapshotLayerId);
			hiddenArtifactLayerIds.append(masksGroupId);
			for(int i = 0; i < regionCount; ++i) {
				const ai::JobCandidate &candidate =
					jobResult.response.candidates.at(i);
				QImage image(candidate.imagePath);
				if(image.isNull()) {
					continue;
				}
				const int layerId = layerIds.at(nextLayerIdIndex++);
				const int maskLayerId = layerIds.at(nextLayerIdIndex++);
				importedLayerIds.append(layerId);
				importedCandidates.append(candidate);
				const int parentGroupId = regionGroupIds.value(
					candidateRegionGroupLabel(candidate), regionSetGroupId);
				messages.append(
					net::makeLayerTreeCreateMessage(
						contextId, layerId, 0, parentGroupId, 0,
						DP_MSG_LAYER_TREE_CREATE_FLAGS_INTO,
						layers->getAvailableLayerName(
							candidateLayerLabel(candidate))));
				net::makePutImageMessagesCompat(
					messages, contextId, layerId, DP_BLEND_MODE_NORMAL,
					canvasBounds.x(), canvasBounds.y(), image,
					currentCanvas->isCompatibilityMode());
				QImage maskImage(candidate.maskPath);
				if(!maskImage.isNull()) {
					hiddenArtifactLayerIds.append(maskLayerId);
					messages.append(
						net::makeLayerTreeCreateMessage(
							contextId, maskLayerId, 0, masksGroupId, 0,
							DP_MSG_LAYER_TREE_CREATE_FLAGS_INTO,
							layers->getAvailableLayerName(
								tr("Mask - %1").arg(candidateDisplayLabel(candidate)))));
					net::makePutImageMessagesCompat(
						messages, contextId, maskLayerId, DP_BLEND_MODE_NORMAL,
						canvasBounds.x(), canvasBounds.y(), maskImage,
						currentCanvas->isCompatibilityMode());
				}
			}

			if(importedLayerIds.isEmpty()) {
				showErrorMessage(
					tr("The AI worker returned unreadable color separation layers."));
				return;
			}

			layers->setLayerIdToSelect(importedLayerIds.first());
			appendHideLayerMessages(messages, hiddenArtifactLayerIds);
			m_doc->client()->sendCommands(messages.size(), messages.constData());
			hideLayersAfterImport(currentCanvas, hiddenArtifactLayerIds);
			if(sourceLayerId > 0) {
				hideLayersAfterImport(currentCanvas, QVector<int>{sourceLayerId});
			}
			QPointer<MainWindow> window(this);
			QThread *labelThread = QThread::create(
				[window, importedLayerIds, importedCandidates] {
				const int count =
					qMin(importedLayerIds.size(), importedCandidates.size());
				for(int i = 0; i < count; ++i) {
					QString error;
					const QString label = labelDecompositionRegionWithHelper(
						importedCandidates.at(i), error);
					if(label.isEmpty()) {
						continue;
					}
					const int layerId = importedLayerIds.at(i);
					QMetaObject::invokeMethod(
						qApp,
						[window, layerId, label] {
							if(!window || !window->m_doc ||
							   !window->m_doc->canvas()) {
								return;
							}
							canvas::LayerListModel *currentLayers =
								window->m_doc->canvas()->layerlist();
							const QModelIndex index =
								currentLayers->layerIndex(layerId);
							if(!index.isValid()) {
								return;
							}
							const QString oldTitle =
								index.data(canvas::LayerListModel::TitleRole)
									.toString();
							if(oldTitle == label) {
								return;
							}
							const uint8_t retitleContextId =
								window->m_doc->canvas()->localUserId();
							const QString availableLabel =
								currentLayers->getAvailableLayerName(label);
							net::Message message = net::makeLayerRetitleMessage(
								retitleContextId, layerId, availableLabel);
							window->m_doc->client()->sendMessage(message);
							window->statusBar()->showMessage(
								MainWindow::tr("Named color separation layer: %1")
									.arg(availableLabel),
								3000);
						},
						Qt::QueuedConnection);
				}
			});
			connect(
				labelThread, &QThread::finished, labelThread,
				&QObject::deleteLater);
			labelThread->start();
			utils::showInformation(
				this, tr("Color Separation"),
				tr("Imported %1 color separation layer(s).")
					.arg(importedLayerIds.size()),
				candidateDetailsText(jobResult));
		});
		thread->start();
	});
	connect(aiObjectDecomposition, &QAction::triggered, this, [=] {
		canvas::CanvasModel *canvas = m_doc->canvas();
		if(!canvas) {
			showErrorMessage(tr("No canvas is available for object decomposition."));
			return;
		}
		if(!m_doc->checkPermission(DP_FEATURE_PUT_IMAGE)) {
			return;
		}
		canvas::AclState *aclState = canvas->aclState();
		if(!aclState ||
		   !(aclState->canUseFeature(DP_FEATURE_EDIT_LAYERS) ||
			 aclState->canUseFeature(DP_FEATURE_OWN_LAYERS))) {
			m_doc->permissionDenied(DP_FEATURE_EDIT_LAYERS);
			return;
		}

		drawdance::CanvasState canvasState =
			canvas->paintEngine()->viewCanvasState();
		const QRect canvasBounds(QPoint(), canvasState.size());
		if(canvasBounds.isEmpty()) {
			showErrorMessage(tr("The canvas is empty."));
			return;
		}

		static ObjectDecompositionOptions lastObjectDecompositionSettings;
		ObjectDecompositionOptions options = lastObjectDecompositionSettings;
		if(!showObjectDecompositionOptionsDialog(
			   this, canvasBounds.size(), options)) {
			return;
		}
		options.repairBase = false;
		lastObjectDecompositionSettings = options;
		const bool semanticHelperEnabled = decompositionSemanticHelperEnabled();
		if(semanticHelperEnabled) {
			QString helperError;
			if(!checkDecompositionSemanticHelper(helperError)) {
				showErrorMessageWithDetails(
					tr("Object decomposition naming needs the vision helper."),
					helperError);
				return;
			}
		}

		drawdance::ViewModeBuffer viewModeBuffer;
		QImage sourceImage = canvas->paintEngine()->getFlatImage(
			viewModeBuffer, canvasState, true, true, &canvasBounds);
		if(sourceImage.isNull()) {
			showErrorMessage(
				tr("Could not export the source image for object decomposition."));
			return;
		}

		QTemporaryDir assetDir(
			QDir::temp().filePath(QStringLiteral("underpaint-ai-assets-XXXXXX")));
		if(!assetDir.isValid()) {
			showErrorMessage(tr("Could not create AI asset directory."));
			return;
		}
		assetDir.setAutoRemove(false);
		const QString sourcePath =
			QDir(assetDir.path()).filePath(QStringLiteral("source.png"));
		if(!sourceImage.save(sourcePath, "PNG")) {
			showErrorMessage(tr("Could not write AI source asset."));
			return;
		}

		const int sourceLayerId = validInpaintAnchorLayer(
			canvas->layerlist(), m_doc->toolCtrl()->activeLayer());
		ai::JobRequest request =
			ai::JobRequest::create(ai::Operation::ObjectDecomposition);
		ai::JobAsset sourceAsset;
		sourceAsset.role = QStringLiteral("source-image");
		sourceAsset.path = sourcePath;
		sourceAsset.mimeType = QStringLiteral("image/png");
		sourceAsset.metadata = QJsonObject{
			{QStringLiteral("colorSpace"), QStringLiteral("srgb")},
		};
		request.inputs = {sourceAsset};
		request.region = QJsonObject{
			{QStringLiteral("x"), canvasBounds.x()},
			{QStringLiteral("y"), canvasBounds.y()},
			{QStringLiteral("width"), canvasBounds.width()},
			{QStringLiteral("height"), canvasBounds.height()},
		};
		request.parameters = QJsonObject{
			{QStringLiteral("segmentationBackend"), options.segmentationBackend},
			{QStringLiteral("maxMasks"), options.maxMasks},
			{QStringLiteral("minRegionAreaPct"), options.minRegionAreaPct},
			{QStringLiteral("decompositionDepth"), options.decompositionDepth},
			{QStringLiteral("personPriorEnabled"), options.personPriorEnabled},
			{QStringLiteral("personPriorConfidence"),
			 options.personPriorConfidencePct / 100.0},
			{QStringLiteral("personPriorMaxRegions"), options.personPriorMaxRegions},
			{QStringLiteral("personPriorMinAreaPct"), options.personPriorMinAreaPct},
			{QStringLiteral("objectPriorEnabled"), options.objectPriorEnabled},
			{QStringLiteral("objectPriorConfidence"),
			 options.objectPriorConfidencePct / 100.0},
			{QStringLiteral("objectPriorMaxRegions"), options.objectPriorMaxRegions},
			{QStringLiteral("objectPriorMinAreaPct"), options.objectPriorMinAreaPct},
			{QStringLiteral("samGridFallbackEnabled"),
			 options.samGridFallbackEnabled},
			{QStringLiteral("groupRepeatedRegions"), options.groupRepeatedRegions},
			{QStringLiteral("repairBase"), false},
		};
		request.preferences = QJsonObject{
			{QStringLiteral("maxRenderEdge"), 1024},
			{QStringLiteral("cacheGuides"), true},
			{QStringLiteral("safe4070Mode"), true},
		};
		request.source = QJsonObject{
			{QStringLiteral("activeLayerId"), sourceLayerId},
			{QStringLiteral("selectionSource"), QStringLiteral("full-canvas")},
		};
		request.provenance = QJsonObject{
			{QStringLiteral("createdBy"), QStringLiteral("underpaint")},
			{QStringLiteral("uiEntryPoint"),
			 QStringLiteral("Power Tools/Object Decomposition")},
		};

		QProgressDialog *progress = new QProgressDialog(
			tr("Finding objects and parts..."), tr("Cancel"), 0,
			qMax(1, options.maxMasks), this);
		AiPreviewLabel *progressLabel = new AiPreviewLabel(progress);
		progressLabel->setText(tr("Finding objects and parts..."));
		progressLabel->setAlignment(Qt::AlignCenter);
		progressLabel->setMinimumWidth(280);
		progressLabel->setMinimumHeight(192);
		progress->setLabel(progressLabel);
		progress->setWindowTitle(tr("Object Decomposition"));
		progress->setWindowModality(Qt::WindowModal);
		progress->setMinimumDuration(0);
		progress->setAutoClose(false);
		progress->setAutoReset(false);
		progress->setValue(0);
		progress->show();

		auto cancelRequested = std::make_shared<std::atomic_bool>(false);
		connect(progress, &QProgressDialog::canceled, this, [cancelRequested] {
			cancelRequested->store(true);
		});

		ObjectDecompositionRunResult *result = new ObjectDecompositionRunResult;
		QPointer<AiPreviewLabel> progressLabelPointer(progressLabel);
		QPointer<QProgressDialog> progressPointer(progress);
		const int progressRegions = qMax(1, options.maxMasks);
		const int progressRepairSteps = options.repairBase ? 30 : 0;
		progress->setMaximum(progressRegions + progressRepairSteps);
		QThread *thread = QThread::create(
			[request, result, progressLabelPointer, progressPointer, progressRegions,
			 progressRepairSteps, sourcePath, sourceLayerId, canvasBounds,
			 semanticHelperEnabled, cancelRequested] {
			const auto updateProgress =
				[progressLabelPointer, progressPointer](
					const QString &text, const QString &imagePath, int value,
					int maximum) {
					QMetaObject::invokeMethod(
						qApp,
						[progressLabelPointer, progressPointer, text, imagePath, value,
						 maximum] {
							if(!progressLabelPointer || !progressPointer) {
								return;
							}
							progressLabelPointer->setToolTip(text);
							progressLabelPointer->setStatusText(text);
							if(!imagePath.isEmpty()) {
								QPixmap preview(imagePath);
								if(!preview.isNull()) {
									progressLabelPointer->setPreviewPixmap(
										preview.scaled(
											256, 192, Qt::KeepAspectRatio,
											Qt::SmoothTransformation));
								}
							}
							progressPointer->setMaximum(qMax(1, maximum));
							progressPointer->setValue(
								qBound(0, value, progressPointer->maximum()));
						},
						Qt::QueuedConnection);
				};
			result->decomposition = ai::JobRunner::run(
				request, QString(), 15 * 60 * 1000,
				[progressLabelPointer, progressPointer,
				 progressRegions](const QJsonObject &event) {
					QMetaObject::invokeMethod(
						qApp,
						[progressLabelPointer, progressPointer, progressRegions,
						 event] {
							if(!progressLabelPointer || !progressPointer) {
								return;
							}
							const QString type =
								event.value(QStringLiteral("type")).toString();
							if(type != QStringLiteral("preview") &&
							   type != QStringLiteral("candidate")) {
								return;
							}
							const QString imagePath =
								event.value(QStringLiteral("imagePath")).toString();
							QPixmap preview(imagePath);
							if(preview.isNull()) {
								return;
							}
							const int candidate =
								event.value(QStringLiteral("candidate")).toInt();
							QString text =
								type == QStringLiteral("preview")
									? MainWindow::tr("Object %1 preview")
										  .arg(candidate)
									: MainWindow::tr("Object %1 complete")
										  .arg(candidate);
							const QString label =
								event.value(QStringLiteral("label")).toString();
							if(!label.isEmpty()) {
								text += MainWindow::tr(" - %1").arg(label);
							}
							progressLabelPointer->setToolTip(text);
							progressLabelPointer->setPreviewPixmap(preview.scaled(
								256, 192, Qt::KeepAspectRatio,
								Qt::SmoothTransformation));
							progressPointer->setMaximum(progressRegions);
							progressPointer->setValue(
								qMin(qMax(1, candidate), progressRegions));
						},
						Qt::QueuedConnection);
				},
				cancelRequested.get());
			if(!result->decomposition.canceled && result->decomposition.ok) {
				for(ai::JobCandidate &candidate :
					result->decomposition.response.candidates) {
					applyDecompositionSemantics(
						candidate, defaultDecompositionSemantics(candidate),
						QStringLiteral("default"));
				}
			}
			if(result->decomposition.canceled || !result->decomposition.ok ||
			   cancelRequested->load()) {
				return;
			}

			if(semanticHelperEnabled) {
				result->semanticHelperAttempted = true;
				const int totalRegions =
					result->decomposition.response.candidates.size();
				for(int i = 0; i < totalRegions; ++i) {
					if(cancelRequested->load()) {
						return;
					}
					ai::JobCandidate &candidate =
						result->decomposition.response.candidates[i];
					if(!candidateIsExtractedObject(candidate)) {
						continue;
					}
					updateProgress(
						MainWindow::tr("Classifying object %1/%2")
							.arg(i + 1)
							.arg(totalRegions),
						candidate.imagePath, progressRegions,
						progressRegions + progressRepairSteps);
					QString semanticError;
					const QJsonObject semantics =
						classifyDecompositionRegionWithHelper(
							candidate, sourcePath, semanticError);
					if(semantics.isEmpty()) {
						++result->semanticRegionsFailed;
						if(!semanticError.isEmpty()) {
							result->semanticLastError = semanticError;
						}
						continue;
					}
					applyDecompositionSemantics(
						candidate, semantics, QStringLiteral("semantic-helper"));
					++result->semanticRegionsClassified;
				}
				if(result->semanticRegionsClassified == 0) {
					result->semanticFatalError =
						result->semanticLastError.isEmpty()
							? MainWindow::tr(
								  "The vision helper did not classify any extracted regions.")
							: result->semanticLastError;
					return;
				}
				updateProgress(
					MainWindow::tr("Grouping related object parts..."),
					QString(), progressRegions,
					progressRegions + progressRepairSteps);
				QString groupRefinementError;
				result->semanticGroupsRefined =
					refineDecompositionGroupsWithHelper(
						result->decomposition.response.candidates, sourcePath,
						groupRefinementError);
				if(result->semanticGroupsRefined == 0 &&
				   !groupRefinementError.isEmpty()) {
					result->semanticLastError = groupRefinementError;
				}
			}

			if(!request.parameters.value(QStringLiteral("repairBase")).toBool() ||
			   cancelRequested->load()) {
				return;
			}

			result->repairRequested = true;
			QImage repairMask = objectDecompositionRepairMask(
				result->decomposition.response.candidates, canvasBounds.size());
			if(repairMask.isNull() || !maskHasEditablePixels(repairMask)) {
				repairMask = objectDecompositionRepairMask(
					result->decomposition.response.candidates, canvasBounds.size(),
					true);
				result->repairMaskFallbackUsed =
					!repairMask.isNull() && maskHasEditablePixels(repairMask);
			}
			if(repairMask.isNull() || !maskHasEditablePixels(repairMask)) {
				return;
			}
			QString repairSourcePath = sourcePath;
			QImage repairSourcePlate = objectDecompositionRepairSourcePlate(
				result->decomposition.response.candidates, canvasBounds.size());
			if(!repairSourcePlate.isNull()) {
				const QString repairSourcePlatePath =
					QFileInfo(sourcePath).absoluteDir().filePath(
						QStringLiteral("object-decomposition-repair-source.png"));
				if(!repairSourcePlate.save(repairSourcePlatePath, "PNG")) {
					result->repairAttempted = true;
					result->repair.ok = false;
					result->repair.errorMessage =
						MainWindow::tr("Could not write object repair source plate.");
					return;
				}
				repairSourcePath = repairSourcePlatePath;
			}
			result->repairSourcePath = repairSourcePath;
			const QString repairMaskPath =
				QFileInfo(sourcePath).absoluteDir().filePath(
					QStringLiteral("object-decomposition-repair-mask.png"));
			if(!repairMask.save(repairMaskPath, "PNG")) {
				result->repairAttempted = true;
				result->repair.ok = false;
				result->repair.errorMessage =
					MainWindow::tr("Could not write object repair mask.");
				return;
			}
			result->repairMaskPath = repairMaskPath;
			result->repairAttempted = true;
			updateProgress(
				MainWindow::tr("Repairing background under extracted objects..."),
				QString(), progressRegions, progressRegions + progressRepairSteps);

			ai::JobRequest repairRequest =
				ai::JobRequest::create(ai::Operation::Inpaint);
			ai::JobAsset repairSourceAsset;
			repairSourceAsset.role = QStringLiteral("source-image");
			repairSourceAsset.path = repairSourcePath;
			repairSourceAsset.mimeType = QStringLiteral("image/png");
			repairSourceAsset.metadata = QJsonObject{
				{QStringLiteral("colorSpace"), QStringLiteral("srgb")},
				{QStringLiteral("sourceRole"),
				 QStringLiteral("semantic-repair-source-plate")},
			};
			ai::JobAsset repairMaskAsset;
			repairMaskAsset.role = QStringLiteral("mask");
			repairMaskAsset.path = repairMaskPath;
			repairMaskAsset.mimeType = QStringLiteral("image/png");
			repairMaskAsset.metadata = QJsonObject{
				{QStringLiteral("whiteMeans"), QStringLiteral("object-to-remove")},
			};
			repairRequest.inputs = {repairSourceAsset, repairMaskAsset};
			repairRequest.parameters = QJsonObject{
				{QStringLiteral("prompt"),
				 objectDecompositionRepairPrompt(
					 result->decomposition.response.candidates)},
				{QStringLiteral("negativePrompt"),
				 objectDecompositionRepairNegativePrompt(
					 result->decomposition.response.candidates)},
				{QStringLiteral("seed"), -1},
				{QStringLiteral("cfg"), 4.0},
				{QStringLiteral("denoise"), 0.72},
				{QStringLiteral("scheduler"), QStringLiteral("dpmpp-3m-karras")},
				{QStringLiteral("steps"), progressRepairSteps},
				{QStringLiteral("refiner"), QJsonObject{{QStringLiteral("enabled"), false}}},
				{QStringLiteral("detailPass"), QJsonObject{{QStringLiteral("enabled"), false}}},
				{QStringLiteral("candidateCount"), 1},
				{QStringLiteral("edgeFeatherPx"), 36},
				{QStringLiteral("prefillNoise"), 0.0},
				{QStringLiteral("prefillStyle"),
				 QStringLiteral("object-context-plate")},
			};
			repairRequest.preferences = QJsonObject{
				{QStringLiteral("maxRenderEdge"), 1024},
				{QStringLiteral("variantMode"), QStringLiteral("sequential")},
				{QStringLiteral("unloadPolicy"), QStringLiteral("idle")},
				{QStringLiteral("vaeTiling"), true},
				{QStringLiteral("cacheGuides"), true},
				{QStringLiteral("safe4070Mode"), true},
			};
			repairRequest.region = QJsonObject{
				{QStringLiteral("x"), canvasBounds.x()},
				{QStringLiteral("y"), canvasBounds.y()},
				{QStringLiteral("width"), canvasBounds.width()},
				{QStringLiteral("height"), canvasBounds.height()},
			};
			repairRequest.source = QJsonObject{
				{QStringLiteral("activeLayerId"), sourceLayerId},
				{QStringLiteral("selectionSource"),
				 QStringLiteral("object-decomposition-union-mask")},
			};
			repairRequest.provenance = QJsonObject{
				{QStringLiteral("createdBy"), QStringLiteral("underpaint")},
				{QStringLiteral("uiEntryPoint"),
				 QStringLiteral("Power Tools/Object Decomposition/Base Repair")},
			};
			result->repair = ai::JobRunner::run(
				repairRequest, QString(), 15 * 60 * 1000,
				[updateProgress, progressRegions, progressRepairSteps](
					const QJsonObject &event) {
					const QString type =
						event.value(QStringLiteral("type")).toString();
					if(type != QStringLiteral("preview") &&
					   type != QStringLiteral("candidate") &&
					   type != QStringLiteral("refiner") &&
					   type != QStringLiteral("detail")) {
						return;
					}
					const int step = event.value(QStringLiteral("step")).toInt();
					const int steps = event.value(QStringLiteral("steps"))
										  .toInt(progressRepairSteps);
					QString text =
						type == QStringLiteral("candidate")
							? MainWindow::tr("Background repair complete")
							: MainWindow::tr("Repairing background");
					if(step > 0 && steps > 0) {
						text += MainWindow::tr(" - step %1/%2").arg(step).arg(steps);
					}
					const int value =
						type == QStringLiteral("candidate")
							? progressRegions + progressRepairSteps
							: progressRegions + qBound(1, step, progressRepairSteps);
					updateProgress(
						text, event.value(QStringLiteral("imagePath")).toString(),
						value, progressRegions + progressRepairSteps);
				},
				cancelRequested.get());
		});
		connect(thread, &QThread::finished, this, [=] {
			progress->close();
			progress->deleteLater();

			const ObjectDecompositionRunResult runResult = *result;
			delete result;
			thread->deleteLater();
			const ai::JobRunResult jobResult = runResult.decomposition;

			canvas::CanvasModel *currentCanvas = m_doc->canvas();
			if(!currentCanvas) {
				showErrorMessage(
					tr("The canvas was closed before AI results could import."));
				return;
			}
			if(jobResult.canceled) {
				return;
			}
			if(runResult.repairAttempted && runResult.repair.canceled) {
				return;
			}
			if(!jobResult.ok) {
				showErrorMessageWithDetails(
					tr("Object decomposition worker failed."),
					jobResult.errorMessage);
				return;
			}
			if(!runResult.semanticFatalError.isEmpty()) {
				showErrorMessageWithDetails(
					tr("Object decomposition semantic classification failed."),
					runResult.semanticFatalError);
				return;
			}
			if(jobResult.response.candidates.isEmpty()) {
				showErrorMessage(
					tr("The AI worker returned no object masks."));
				return;
			}

			canvas::LayerListModel *layers = currentCanvas->layerlist();
			const int regionCount = jobResult.response.candidates.size();
			const bool hasRepairBase =
				runResult.repairRequested && runResult.repairAttempted &&
				!runResult.repair.canceled && runResult.repair.ok &&
				!runResult.repair.response.candidates.isEmpty() &&
				!QImage(runResult.repair.response.candidates.first().imagePath)
					 .isNull();
			QStringList groupOrder;
			for(const ai::JobCandidate &candidate : jobResult.response.candidates) {
				const QString groupLabel = candidateRegionGroupLabel(candidate);
				if(!groupOrder.contains(groupLabel)) {
					groupOrder.append(groupLabel);
				}
			}
			const int requestedLayerIds =
				regionCount * 2 + groupOrder.size() + 4 + (hasRepairBase ? 1 : 0);
			QVector<int> layerIds = layers->getAvailableLayerIds(requestedLayerIds);
			if(layerIds.size() < requestedLayerIds) {
				showErrorMessage(
					tr("Could not allocate layers for object decomposition."));
				return;
			}

			const uint8_t contextId = currentCanvas->localUserId();
			int nextLayerIdIndex = 0;
			const int groupId = layerIds.at(nextLayerIdIndex++);
			const int regionSetGroupId = layerIds.at(nextLayerIdIndex++);
			const int sourceSnapshotLayerId = layerIds.at(nextLayerIdIndex++);
			const int masksGroupId = layerIds.at(nextLayerIdIndex++);
			net::MessageList messages;
			messages.append(net::makeUndoPointMessage(contextId));
			messages.append(
				net::makeLayerTreeCreateMessage(
					contextId, groupId, 0, qMax(0, sourceLayerId), 0,
					DP_MSG_LAYER_TREE_CREATE_FLAGS_GROUP,
					layers->getAvailableLayerName(
						tr("Object Decomposition"))));
			messages.append(
				net::makeLayerTreeCreateMessage(
					contextId, regionSetGroupId, 0, groupId, 0,
					DP_MSG_LAYER_TREE_CREATE_FLAGS_GROUP |
						DP_MSG_LAYER_TREE_CREATE_FLAGS_INTO,
					layers->getAvailableLayerName(
						candidateRegionSetLabel(jobResult))));
			messages.append(
				net::makeLayerTreeCreateMessage(
					contextId, sourceSnapshotLayerId, 0, regionSetGroupId, 0,
					DP_MSG_LAYER_TREE_CREATE_FLAGS_INTO,
					layers->getAvailableLayerName(tr("Source Snapshot"))));
			net::makePutImageMessagesCompat(
				messages, contextId, sourceSnapshotLayerId, DP_BLEND_MODE_NORMAL,
				canvasBounds.x(), canvasBounds.y(), sourceImage,
				currentCanvas->isCompatibilityMode());
			messages.append(
				net::makeLayerTreeCreateMessage(
					contextId, masksGroupId, 0, regionSetGroupId, 0,
					DP_MSG_LAYER_TREE_CREATE_FLAGS_GROUP |
						DP_MSG_LAYER_TREE_CREATE_FLAGS_INTO,
					layers->getAvailableLayerName(tr("Masks"))));

			QMap<QString, int> regionGroupIds;
			for(const QString &groupLabel : std::as_const(groupOrder)) {
				const int regionGroupId = layerIds.at(nextLayerIdIndex++);
				regionGroupIds.insert(groupLabel, regionGroupId);
				messages.append(
					net::makeLayerTreeCreateMessage(
						contextId, regionGroupId, 0, regionSetGroupId, 0,
						DP_MSG_LAYER_TREE_CREATE_FLAGS_GROUP |
							DP_MSG_LAYER_TREE_CREATE_FLAGS_INTO,
						layers->getAvailableLayerName(groupLabel)));
			}

			if(hasRepairBase) {
				const ai::JobCandidate &repairCandidate =
					runResult.repair.response.candidates.first();
				QImage repairImage(repairCandidate.imagePath);
				const int repairLayerId = layerIds.at(nextLayerIdIndex++);
				const int repairParentGroupId = regionGroupIds.value(
					QStringLiteral("Base Remainder"), regionSetGroupId);
				QString repairLayerName = tr("Repaired Base");
				const QString repairSeed = candidateSeedText(repairCandidate);
				if(!repairSeed.isEmpty()) {
					repairLayerName =
						tr("Repaired Base - Seed %1").arg(repairSeed);
				}
				messages.append(
					net::makeLayerTreeCreateMessage(
						contextId, repairLayerId, 0, repairParentGroupId, 0,
						DP_MSG_LAYER_TREE_CREATE_FLAGS_INTO,
						layers->getAvailableLayerName(repairLayerName)));
				net::makePutImageMessagesCompat(
					messages, contextId, repairLayerId, DP_BLEND_MODE_NORMAL,
					canvasBounds.x(), canvasBounds.y(), repairImage,
					currentCanvas->isCompatibilityMode());
			}

			QVector<int> importedLayerIds;
			importedLayerIds.reserve(regionCount);
			QVector<ai::JobCandidate> importedCandidates;
			importedCandidates.reserve(regionCount);
			QVector<int> hiddenArtifactLayerIds;
			hiddenArtifactLayerIds.reserve(regionCount + 2);
			hiddenArtifactLayerIds.append(sourceSnapshotLayerId);
			hiddenArtifactLayerIds.append(masksGroupId);
			for(int i = 0; i < regionCount; ++i) {
				const ai::JobCandidate &candidate =
					jobResult.response.candidates.at(i);
				QImage image(candidate.imagePath);
				if(image.isNull()) {
					continue;
				}
				const int layerId = layerIds.at(nextLayerIdIndex++);
				const int maskLayerId = layerIds.at(nextLayerIdIndex++);
				importedLayerIds.append(layerId);
				importedCandidates.append(candidate);
				const int parentGroupId = regionGroupIds.value(
					candidateRegionGroupLabel(candidate), regionSetGroupId);
				messages.append(
					net::makeLayerTreeCreateMessage(
						contextId, layerId, 0, parentGroupId, 0,
						DP_MSG_LAYER_TREE_CREATE_FLAGS_INTO,
						layers->getAvailableLayerName(
							candidateLayerLabel(candidate))));
				net::makePutImageMessagesCompat(
					messages, contextId, layerId, DP_BLEND_MODE_NORMAL,
					canvasBounds.x(), canvasBounds.y(), image,
					currentCanvas->isCompatibilityMode());
				QImage maskImage(candidate.maskPath);
				if(!maskImage.isNull()) {
					hiddenArtifactLayerIds.append(maskLayerId);
					messages.append(
						net::makeLayerTreeCreateMessage(
							contextId, maskLayerId, 0, masksGroupId, 0,
							DP_MSG_LAYER_TREE_CREATE_FLAGS_INTO,
							layers->getAvailableLayerName(
								tr("Mask - %1").arg(candidateDisplayLabel(candidate)))));
					net::makePutImageMessagesCompat(
						messages, contextId, maskLayerId, DP_BLEND_MODE_NORMAL,
						canvasBounds.x(), canvasBounds.y(), maskImage,
						currentCanvas->isCompatibilityMode());
				}
			}

			if(importedLayerIds.isEmpty()) {
				showErrorMessage(
					tr("The AI worker returned unreadable object layers."));
				return;
			}

			layers->setLayerIdToSelect(importedLayerIds.first());
			for(int i = 0; i < importedLayerIds.size(); ++i) {
				if(importedCandidates.value(i).metadata.value(
					   QStringLiteral("maskRole")).toString() ==
				   QStringLiteral("extracted-object")) {
					layers->setLayerIdToSelect(importedLayerIds.at(i));
					break;
				}
			}
			appendHideLayerMessages(messages, hiddenArtifactLayerIds);
			m_doc->client()->sendCommands(messages.size(), messages.constData());
			hideLayersAfterImport(currentCanvas, hiddenArtifactLayerIds);
			if(sourceLayerId > 0) {
				hideLayersAfterImport(currentCanvas, QVector<int>{sourceLayerId});
			}
			QPointer<MainWindow> window(this);
			QThread *labelThread = QThread::create(
				[window, importedLayerIds, importedCandidates] {
				const int count =
					qMin(importedLayerIds.size(), importedCandidates.size());
				for(int i = 0; i < count; ++i) {
					const ai::JobCandidate candidate = importedCandidates.at(i);
					if(candidate.metadata.value(QStringLiteral("maskRole"))
						   .toString() != QStringLiteral("extracted-object")) {
						continue;
					}
					if(candidate.metadata.value(QStringLiteral("helperStatus"))
						   .toString() == QStringLiteral("semantic-helper")) {
						continue;
					}
					QString error;
					const QString label =
						labelDecompositionRegionWithHelper(candidate, error);
					if(label.isEmpty()) {
						continue;
					}
					const int layerId = importedLayerIds.at(i);
					QMetaObject::invokeMethod(
						qApp,
						[window, layerId, label] {
							if(!window || !window->m_doc ||
							   !window->m_doc->canvas()) {
								return;
							}
							canvas::LayerListModel *currentLayers =
								window->m_doc->canvas()->layerlist();
							const QModelIndex index =
								currentLayers->layerIndex(layerId);
							if(!index.isValid()) {
								return;
							}
							const QString oldTitle =
								index.data(canvas::LayerListModel::TitleRole)
									.toString();
							if(oldTitle == label) {
								return;
							}
							const uint8_t retitleContextId =
								window->m_doc->canvas()->localUserId();
							const QString availableLabel =
								currentLayers->getAvailableLayerName(label);
							net::Message message = net::makeLayerRetitleMessage(
								retitleContextId, layerId, availableLabel);
							window->m_doc->client()->sendMessage(message);
							window->statusBar()->showMessage(
								MainWindow::tr("Named object layer: %1")
									.arg(availableLabel),
								3000);
						},
						Qt::QueuedConnection);
				}
			});
			connect(
				labelThread, &QThread::finished, labelThread,
				&QObject::deleteLater);
			labelThread->start();
			QString importSummary =
				hasRepairBase
					? tr("Imported %1 object layer(s) with a repaired base.")
						  .arg(importedLayerIds.size())
					: tr("Imported %1 object layer(s). Base remainder stays visible so the stack reconstructs the source.")
						  .arg(importedLayerIds.size());
			QString details = candidateDetailsText(jobResult);
			if(runResult.semanticHelperAttempted) {
				details += QStringLiteral("\n\n");
				details += tr("Semantic classification: %1 classified, %2 failed.")
							   .arg(runResult.semanticRegionsClassified)
							   .arg(runResult.semanticRegionsFailed);
				if(runResult.semanticGroupsRefined > 0) {
					details += QStringLiteral("\n");
					details += tr("Object grouping: %1 layer(s) refined.")
								   .arg(runResult.semanticGroupsRefined);
				}
			}
			if(runResult.repairRequested) {
				if(hasRepairBase) {
					details += QStringLiteral("\n\n");
					details += tr("Base repair:");
					details += QStringLiteral("\n");
					if(!runResult.repairSourcePath.isEmpty()) {
						details += tr("Repair source plate: %1")
									   .arg(runResult.repairSourcePath);
						details += QStringLiteral("\n");
					}
					if(runResult.repairMaskFallbackUsed) {
						details += tr("Repair mask: used all extracted objects because semantic roles left no removable pixels.");
						details += QStringLiteral("\n");
					}
					details += candidateDetailsText(runResult.repair);
				} else if(runResult.repairAttempted && !runResult.repair.ok) {
					details += QStringLiteral("\n\n");
					details += tr("Base repair failed: %1")
								   .arg(runResult.repair.errorMessage);
				} else if(!runResult.repairAttempted) {
					details += QStringLiteral("\n\n");
					details += tr("Base repair skipped: no extracted-object mask pixels were available.");
				}
			}
			utils::showInformation(
				this, tr("Object Decomposition"), importSummary, details);
		});
		thread->start();
	});
	connect(aiBackgroundRemoval, &QAction::triggered, this, [=] {
		canvas::CanvasModel *canvas = m_doc->canvas();
		if(!canvas) {
			showErrorMessage(tr("No canvas is available for background removal."));
			return;
		}
		if(!m_doc->checkPermission(DP_FEATURE_PUT_IMAGE)) {
			return;
		}
		canvas::AclState *aclState = canvas->aclState();
		if(!aclState ||
		   !(aclState->canUseFeature(DP_FEATURE_EDIT_LAYERS) ||
			 aclState->canUseFeature(DP_FEATURE_OWN_LAYERS))) {
			m_doc->permissionDenied(DP_FEATURE_EDIT_LAYERS);
			return;
		}

		drawdance::CanvasState canvasState =
			canvas->paintEngine()->viewCanvasState();
		const QRect canvasBounds(QPoint(), canvasState.size());
		if(canvasBounds.isEmpty()) {
			showErrorMessage(tr("The canvas is empty."));
			return;
		}

		drawdance::ViewModeBuffer viewModeBuffer;
		QImage sourceImage = canvas->paintEngine()->getFlatImage(
			viewModeBuffer, canvasState, true, true, &canvasBounds);
		if(sourceImage.isNull()) {
			showErrorMessage(
				tr("Could not export the source image for background removal."));
			return;
		}

		QTemporaryDir assetDir(
			QDir::temp().filePath(QStringLiteral("underpaint-ai-assets-XXXXXX")));
		if(!assetDir.isValid()) {
			showErrorMessage(tr("Could not create AI asset directory."));
			return;
		}
		assetDir.setAutoRemove(false);
		const QString sourcePath =
			QDir(assetDir.path()).filePath(QStringLiteral("source.png"));
		if(!sourceImage.save(sourcePath, "PNG")) {
			showErrorMessage(tr("Could not write AI source asset."));
			return;
		}

		const int sourceLayerId = validInpaintAnchorLayer(
			canvas->layerlist(), m_doc->toolCtrl()->activeLayer());
		ai::JobRequest request =
			ai::JobRequest::create(ai::Operation::BackgroundRemoval);
		ai::JobAsset sourceAsset;
		sourceAsset.role = QStringLiteral("source-image");
		sourceAsset.path = sourcePath;
		sourceAsset.mimeType = QStringLiteral("image/png");
		sourceAsset.metadata = QJsonObject{
			{QStringLiteral("colorSpace"), QStringLiteral("srgb")},
		};
		request.inputs = {sourceAsset};
		request.region = QJsonObject{
			{QStringLiteral("x"), canvasBounds.x()},
			{QStringLiteral("y"), canvasBounds.y()},
			{QStringLiteral("width"), canvasBounds.width()},
			{QStringLiteral("height"), canvasBounds.height()},
		};
		request.parameters = QJsonObject{
			{QStringLiteral("decompositionDepth"), QStringLiteral("clean")},
		};
		request.preferences = QJsonObject{
			{QStringLiteral("maxRenderEdge"), 1024},
			{QStringLiteral("cacheGuides"), true},
			{QStringLiteral("safe4070Mode"), true},
		};
		request.source = QJsonObject{
			{QStringLiteral("activeLayerId"), sourceLayerId},
			{QStringLiteral("selectionSource"), QStringLiteral("full-canvas")},
		};
			request.provenance = QJsonObject{
				{QStringLiteral("createdBy"), QStringLiteral("underpaint")},
				{QStringLiteral("uiEntryPoint"), QStringLiteral("Power Tools/Remove Background")},
			};

		QProgressDialog *progress = new QProgressDialog(
			tr("Removing background..."), tr("Cancel"), 0, 1, this);
		AiPreviewLabel *progressLabel = new AiPreviewLabel(progress);
		progressLabel->setText(tr("Removing background..."));
		progressLabel->setAlignment(Qt::AlignCenter);
		progressLabel->setMinimumWidth(280);
		progressLabel->setMinimumHeight(192);
		progress->setLabel(progressLabel);
		progress->setWindowTitle(tr("Remove Background"));
		progress->setWindowModality(Qt::WindowModal);
		progress->setMinimumDuration(0);
		progress->setAutoClose(false);
		progress->setAutoReset(false);
		progress->setValue(0);
		progress->show();

		auto cancelRequested = std::make_shared<std::atomic_bool>(false);
		connect(progress, &QProgressDialog::canceled, this, [cancelRequested] {
			cancelRequested->store(true);
		});

		ai::JobRunResult *result = new ai::JobRunResult;
		QPointer<AiPreviewLabel> progressLabelPointer(progressLabel);
		QPointer<QProgressDialog> progressPointer(progress);
		QThread *thread = QThread::create(
			[request, result, progressLabelPointer, progressPointer,
			 cancelRequested] {
			*result = ai::JobRunner::run(
				request, QString(), 15 * 60 * 1000,
				[progressLabelPointer, progressPointer](
					const QJsonObject &event) {
					QMetaObject::invokeMethod(
						qApp,
						[progressLabelPointer, progressPointer, event] {
							if(!progressLabelPointer || !progressPointer) {
								return;
							}
							const QString type =
								event.value(QStringLiteral("type")).toString();
							if(type != QStringLiteral("candidate")) {
								return;
							}
							const QString imagePath =
								event.value(QStringLiteral("imagePath")).toString();
							QPixmap preview(imagePath);
							if(preview.isNull()) {
								return;
							}
							progressLabelPointer->setToolTip(
								MainWindow::tr("Background removal complete"));
							progressLabelPointer->setPreviewPixmap(preview.scaled(
								256, 192, Qt::KeepAspectRatio,
								Qt::SmoothTransformation));
							progressPointer->setValue(1);
						},
						Qt::QueuedConnection);
				},
				cancelRequested.get());
		});
		connect(thread, &QThread::finished, this, [=] {
			progress->close();
			progress->deleteLater();

			const ai::JobRunResult jobResult = *result;
			delete result;
			thread->deleteLater();

			canvas::CanvasModel *currentCanvas = m_doc->canvas();
			if(!currentCanvas) {
				showErrorMessage(
					tr("The canvas was closed before AI results could import."));
				return;
			}
			if(jobResult.canceled) {
				return;
			}
			if(!jobResult.ok) {
				showErrorMessageWithDetails(
					tr("Background removal worker failed."),
					jobResult.errorMessage);
				return;
			}
			if(jobResult.response.candidates.isEmpty()) {
				showErrorMessage(
					tr("The AI worker returned no background removal layer."));
				return;
			}

			const ai::JobCandidate candidate =
				jobResult.response.candidates.first();
			QImage image(candidate.imagePath);
			if(image.isNull()) {
				showErrorMessage(
					tr("The AI worker returned an unreadable cutout layer."));
				return;
			}

			canvas::LayerListModel *layers = currentCanvas->layerlist();
			const QVector<int> layerIds = layers->getAvailableLayerIds(4);
			if(layerIds.size() < 4) {
				showErrorMessage(
					tr("Could not allocate layers for background removal."));
				return;
			}

			const uint8_t contextId = currentCanvas->localUserId();
			const int groupId = layerIds.at(0);
			const int cutoutLayerId = layerIds.at(1);
			const int sourceSnapshotLayerId = layerIds.at(2);
			const int matteLayerId = layerIds.at(3);
			net::MessageList messages;
			messages.append(net::makeUndoPointMessage(contextId));
			messages.append(
				net::makeLayerTreeCreateMessage(
					contextId, groupId, 0, qMax(0, sourceLayerId), 0,
					DP_MSG_LAYER_TREE_CREATE_FLAGS_GROUP,
					layers->getAvailableLayerName(tr("Background Removal"))));
			messages.append(
				net::makeLayerTreeCreateMessage(
					contextId, cutoutLayerId, 0, groupId, 0,
					DP_MSG_LAYER_TREE_CREATE_FLAGS_INTO,
					layers->getAvailableLayerName(
						candidateLayerLabel(candidate, tr("Cutout")))));
			net::makePutImageMessagesCompat(
				messages, contextId, cutoutLayerId, DP_BLEND_MODE_NORMAL,
				canvasBounds.x(), canvasBounds.y(), image,
				currentCanvas->isCompatibilityMode());
			messages.append(
				net::makeLayerTreeCreateMessage(
					contextId, sourceSnapshotLayerId, 0, groupId, 0,
					DP_MSG_LAYER_TREE_CREATE_FLAGS_INTO,
					layers->getAvailableLayerName(tr("Source Snapshot"))));
			net::makePutImageMessagesCompat(
				messages, contextId, sourceSnapshotLayerId, DP_BLEND_MODE_NORMAL,
				canvasBounds.x(), canvasBounds.y(), sourceImage,
				currentCanvas->isCompatibilityMode());
			QImage matteImage(candidate.maskPath);
			if(!matteImage.isNull()) {
				messages.append(
					net::makeLayerTreeCreateMessage(
						contextId, matteLayerId, 0, groupId, 0,
						DP_MSG_LAYER_TREE_CREATE_FLAGS_INTO,
						layers->getAvailableLayerName(tr("Foreground Matte"))));
				net::makePutImageMessagesCompat(
					messages, contextId, matteLayerId, DP_BLEND_MODE_NORMAL,
					canvasBounds.x(), canvasBounds.y(), matteImage,
					currentCanvas->isCompatibilityMode());
			}

			layers->setLayerIdToSelect(cutoutLayerId);
			m_doc->client()->sendCommands(messages.size(), messages.constData());
			currentCanvas->paintEngine()->setLayerVisibility(
				sourceSnapshotLayerId, true);
			if(!matteImage.isNull()) {
				currentCanvas->paintEngine()->setLayerVisibility(matteLayerId, true);
			}
			if(sourceLayerId > 0) {
				currentCanvas->paintEngine()->setLayerVisibility(sourceLayerId, true);
			}
			utils::showInformation(
				this, tr("Remove Background"),
				tr("Imported a foreground cutout and matte."),
				candidateDetailsText(jobResult));
		});
		thread->start();
	});
	connect(aiUnderpaintBehind, &QAction::triggered, this, [=] {
		canvas::CanvasModel *canvas = m_doc->canvas();
		if(!canvas) {
			showErrorMessage(tr("No canvas is available for underpainting."));
			return;
		}
		if(!m_doc->checkPermission(DP_FEATURE_PUT_IMAGE)) {
			return;
		}
		canvas::AclState *aclState = canvas->aclState();
		if(!aclState ||
		   !(aclState->canUseFeature(DP_FEATURE_EDIT_LAYERS) ||
			 aclState->canUseFeature(DP_FEATURE_OWN_LAYERS))) {
			m_doc->permissionDenied(DP_FEATURE_EDIT_LAYERS);
			return;
		}

		canvas::LayerListModel *layers = canvas->layerlist();
		const int activeLayerId = validInpaintAnchorLayer(
			layers, m_doc->toolCtrl()->activeLayer());
		if(activeLayerId <= 0) {
			showErrorMessage(
				tr("Select an extracted object layer to underpaint behind."));
			return;
		}
		const QModelIndex activeLayerIndex = layers->layerIndex(activeLayerId);
		const bool activeLayerIsGroup =
			activeLayerIndex.isValid() &&
			activeLayerIndex.data(canvas::LayerListModel::IsGroupRole).toBool();

		drawdance::CanvasState canvasState =
			canvas->paintEngine()->viewCanvasState();
		const QRect canvasBounds(QPoint(), canvasState.size());
		if(canvasBounds.isEmpty()) {
			showErrorMessage(tr("The canvas is empty."));
			return;
		}

		const QImage layerImage =
			activeLayerIsGroup
				? combinedLayerAlphaImage(
					  canvas, visibleChildLayerIds(layers, activeLayerId),
					  canvasBounds)
				: canvas->paintEngine()->getLayerImage(activeLayerId, canvasBounds);
		if(layerImage.isNull()) {
			showErrorMessage(
				activeLayerIsGroup
					? tr("Could not read visible child layers as an underpaint mask.")
					: tr("Could not read the active layer as an underpaint mask."));
			return;
		}
		const QRect targetRegion = opaquePixelBounds(layerImage);
		if(targetRegion.isEmpty()) {
			showErrorMessage(
				activeLayerIsGroup
					? tr("The active group has no visible child pixels to underpaint behind.")
					: tr("The active layer has no visible pixels to underpaint behind."));
			return;
		}

		const int contextPadding = 128;
		const QRect exportRegion =
			targetRegion
				.adjusted(-contextPadding, -contextPadding, contextPadding,
						  contextPadding)
				.intersected(canvasBounds);
		QImage mask = alphaImageToInpaintMask(layerImage, exportRegion);
		if(exportRegion.isEmpty() || mask.isNull() || !maskHasEditablePixels(mask)) {
			showErrorMessage(tr("Could not export an editable underpaint mask."));
			return;
		}

			static InpaintOptions lastUnderpaintSettings;
			InpaintOptions options = lastUnderpaintSettings;
			const RefinerOptions refinerDefaults = loadRefinerOptions();
			options.refinerEnabled = refinerDefaults.enabled;
			options.refinerPlacement = refinerDefaults.placement;
			options.detailPassEnabled = loadDetailPassOptions().enabled;
		options.prompt.clear();
		options.negativePrompt.clear();
		options.prompt = takeReusableInpaintPrompt();
		const QString activeLayerTitle =
			activeLayerIndex.isValid()
				? activeLayerIndex.data(canvas::LayerListModel::TitleRole)
					  .toString()
				: tr("Active Layer");
		if(!showInpaintOptionsDialog(
			   this, targetRegion, options, tr("Underpaint Behind"),
			   activeLayerIsGroup ? tr("Active group") : tr("Active layer"),
			   tr("Describe what should exist behind this region"),
			   QStringLiteral("inpaint-prompt-improve"),
			   [canvas, canvasState, targetRegion] {
				   drawdance::ViewModeBuffer viewModeBuffer;
				   QRect region = targetRegion;
				   return canvas->paintEngine()->getFlatImage(
					   viewModeBuffer, canvasState, true, true, &region);
			   })) {
			return;
		}
		rememberInpaintPrompt(options.prompt);
		lastUnderpaintSettings = options;
		lastUnderpaintSettings.prompt.clear();
		lastUnderpaintSettings.negativePrompt.clear();
		const QString refinerError = refinerRunBlocker(options.refinerEnabled);
		if(!refinerError.isEmpty()) {
			showErrorMessageWithDetails(
				tr("Refiner backend is not ready."), refinerError);
			return;
		}

		drawdance::ViewModeBuffer viewModeBuffer;
		QImage sourceImage = canvas->paintEngine()->getFlatImage(
			viewModeBuffer, canvasState, true, true, &exportRegion);
		if(sourceImage.isNull()) {
			showErrorMessage(tr("Could not export the source image for AI."));
			return;
		}

		QTemporaryDir assetDir(
			QDir::temp().filePath(QStringLiteral("underpaint-ai-assets-XXXXXX")));
		if(!assetDir.isValid()) {
			showErrorMessage(tr("Could not create AI asset directory."));
			return;
		}
		assetDir.setAutoRemove(false);
		const QString sourcePath =
			QDir(assetDir.path()).filePath(QStringLiteral("source.png"));
		const QString maskPath =
			QDir(assetDir.path()).filePath(QStringLiteral("mask.png"));
		if(!sourceImage.save(sourcePath, "PNG") || !mask.save(maskPath, "PNG")) {
			showErrorMessage(tr("Could not write AI source assets."));
			return;
		}

		ai::JobRequest request = ai::JobRequest::create(ai::Operation::Inpaint);
		ai::JobAsset sourceAsset;
		sourceAsset.role = QStringLiteral("source-image");
		sourceAsset.path = sourcePath;
		sourceAsset.mimeType = QStringLiteral("image/png");
		sourceAsset.metadata = QJsonObject{
			{QStringLiteral("colorSpace"), QStringLiteral("srgb")},
		};
		ai::JobAsset maskAsset;
		maskAsset.role = QStringLiteral("mask");
		maskAsset.path = maskPath;
		maskAsset.mimeType = QStringLiteral("image/png");
		maskAsset.metadata = QJsonObject{
			{QStringLiteral("whiteMeans"), QStringLiteral("object-to-remove")},
		};
		request.inputs = {sourceAsset, maskAsset};
		request.parameters = QJsonObject{
			{QStringLiteral("prompt"), options.prompt},
			{QStringLiteral("negativePrompt"), options.negativePrompt},
			{QStringLiteral("seed"), options.seed},
			{QStringLiteral("cfg"), options.cfg},
			{QStringLiteral("denoise"), options.denoise},
			{QStringLiteral("scheduler"), options.scheduler},
			{QStringLiteral("steps"), options.steps},
			{QStringLiteral("refiner"),
			 refinerParameters(options.refinerEnabled, options.scheduler)},
			{QStringLiteral("detailPass"),
			 detailPassParameters(options.detailPassEnabled, options.scheduler)},
			{QStringLiteral("candidateCount"), options.candidateCount},
			{QStringLiteral("edgeFeatherPx"), options.edgeFeatherPx},
		};
		request.preferences = QJsonObject{
			{QStringLiteral("maxRenderEdge"), 1024},
			{QStringLiteral("variantMode"), QStringLiteral("sequential")},
			{QStringLiteral("unloadPolicy"), QStringLiteral("idle")},
			{QStringLiteral("vaeTiling"), true},
			{QStringLiteral("cacheGuides"), true},
			{QStringLiteral("safe4070Mode"), true},
		};
		request.region = QJsonObject{
			{QStringLiteral("x"), exportRegion.x()},
			{QStringLiteral("y"), exportRegion.y()},
			{QStringLiteral("width"), exportRegion.width()},
			{QStringLiteral("height"), exportRegion.height()},
			{QStringLiteral("selectionX"), targetRegion.x()},
			{QStringLiteral("selectionY"), targetRegion.y()},
			{QStringLiteral("selectionWidth"), targetRegion.width()},
			{QStringLiteral("selectionHeight"), targetRegion.height()},
			{QStringLiteral("contextPadding"), contextPadding},
		};
		request.source = QJsonObject{
			{QStringLiteral("activeLayerId"), activeLayerId},
			{QStringLiteral("activeLayerTitle"), activeLayerTitle},
			{QStringLiteral("activeLayerIsGroup"), activeLayerIsGroup},
			{QStringLiteral("selectionSource"),
			 activeLayerIsGroup ? QStringLiteral("active-group-alpha")
								: QStringLiteral("active-layer-alpha")},
		};
			request.provenance = QJsonObject{
				{QStringLiteral("createdBy"), QStringLiteral("underpaint")},
				{QStringLiteral("uiEntryPoint"), QStringLiteral("Power Tools/Underpaint Behind")},
			};

			const int progressSteps =
				effectiveDiffusionSteps(options.steps, options.denoise);
			const RefinerOptions progressRefinerOptions = loadRefinerOptions();
			const int progressRefinerSteps =
				options.refinerEnabled
					? effectiveDiffusionSteps(
						  progressRefinerOptions.steps,
						  progressRefinerOptions.strength)
					: 0;
			const DetailPassOptions progressDetailOptions = loadDetailPassOptions();
			const int progressDetailSteps =
				options.detailPassEnabled
					? effectiveDiffusionSteps(
						  progressDetailOptions.steps, progressDetailOptions.denoise)
					: 0;
		const int progressStepStride =
			progressSteps + progressRefinerSteps + progressDetailSteps;
		const int progressCandidates = qMax(1, options.candidateCount);

		QProgressDialog *progress = new QProgressDialog(
			tr("Underpainting behind active layer..."), tr("Cancel"), 0,
			qMax(1, progressCandidates * progressStepStride), this);
		AiPreviewLabel *progressLabel = new AiPreviewLabel(progress);
		progressLabel->setText(tr("Underpainting behind active layer..."));
		progressLabel->setAlignment(Qt::AlignCenter);
		progressLabel->setMinimumWidth(280);
		progressLabel->setMinimumHeight(192);
		progress->setLabel(progressLabel);
		progress->setWindowTitle(tr("Underpaint Behind"));
		progress->setWindowModality(Qt::WindowModal);
		progress->setMinimumDuration(0);
		progress->setAutoClose(false);
		progress->setAutoReset(false);
		progress->setValue(0);
		progress->show();

		auto cancelRequested = std::make_shared<std::atomic_bool>(false);
		connect(progress, &QProgressDialog::canceled, this, [cancelRequested] {
			cancelRequested->store(true);
		});

		ai::JobRunResult *result = new ai::JobRunResult;
		QPointer<AiPreviewLabel> progressLabelPointer(progressLabel);
		QPointer<QProgressDialog> progressPointer(progress);
		QThread *thread = QThread::create(
			[request, result, progressLabelPointer, progressPointer,
			 progressStepStride, progressCandidates, cancelRequested] {
			*result = ai::JobRunner::run(
				request, QString(), 15 * 60 * 1000,
				[progressLabelPointer, progressPointer, progressStepStride,
				 progressCandidates](const QJsonObject &event) {
					QMetaObject::invokeMethod(
						qApp,
						[progressLabelPointer, progressPointer, progressStepStride,
						 progressCandidates, event] {
							if(!progressLabelPointer || !progressPointer) {
								return;
							}
							const QString type =
								event.value(QStringLiteral("type")).toString();
							const int candidate =
								event.value(QStringLiteral("candidate")).toInt();
							const int step = event.value(QStringLiteral("step")).toInt();
							const int steps =
								event.value(QStringLiteral("steps")).toInt();
							QString text;
							if(type == QStringLiteral("detail")) {
								text = MainWindow::tr("Candidate %1 detail pass")
										   .arg(candidate);
							} else if(type == QStringLiteral("refiner")) {
								text = MainWindow::tr("Candidate %1 refiner")
										   .arg(candidate);
							} else if(type == QStringLiteral("preview") ||
									  type == QStringLiteral("candidate")) {
								text = type == QStringLiteral("preview")
										   ? MainWindow::tr("Candidate %1 preview")
												 .arg(candidate)
										   : MainWindow::tr("Candidate %1 complete")
												 .arg(candidate);
								const QString imagePath =
									event.value(QStringLiteral("imagePath")).toString();
								QPixmap preview(imagePath);
								if(!preview.isNull()) {
									progressLabelPointer->setPreviewPixmap(
										preview.scaled(
											256, 192, Qt::KeepAspectRatio,
											Qt::SmoothTransformation));
								}
							} else {
								return;
							}
							if(step > 0 && steps > 0) {
								text += MainWindow::tr(" - step %1/%2")
											.arg(step)
											.arg(steps);
							}
							progressLabelPointer->setToolTip(text);
							progressLabelPointer->setStatusText(text);
							const int candidateIndex = qMax(1, candidate);
							int progressValue =
								(candidateIndex - 1) * progressStepStride;
							if(type == QStringLiteral("candidate")) {
								progressValue = candidateIndex * progressStepStride;
							} else if(step > 0) {
								progressValue += step;
							} else {
								progressValue += 1;
							}
							progressPointer->setMaximum(
								qMax(1, progressCandidates * progressStepStride));
							progressPointer->setValue(qMin(
								progressValue, progressPointer->maximum()));
						},
						Qt::QueuedConnection);
				},
				cancelRequested.get());
		});
		connect(thread, &QThread::finished, this, [=] {
			progress->close();
			progress->deleteLater();

			const ai::JobRunResult jobResult = *result;
			delete result;
			thread->deleteLater();

			canvas::CanvasModel *currentCanvas = m_doc->canvas();
			if(!currentCanvas) {
				showErrorMessage(
					tr("The canvas was closed before AI results could import."));
				return;
			}
			if(jobResult.canceled) {
				return;
			}
			if(!jobResult.ok) {
				showErrorMessageWithDetails(
					tr("Underpaint worker failed."), jobResult.errorMessage);
				return;
			}
			if(jobResult.response.candidates.isEmpty()) {
				showErrorMessage(tr("The AI worker returned no candidates."));
				return;
			}

			canvas::LayerListModel *currentLayers = currentCanvas->layerlist();
			const int candidateCount = jobResult.response.candidates.size();
			QVector<int> layerIds =
				currentLayers->getAvailableLayerIds(candidateCount + 1);
			if(layerIds.size() < candidateCount + 1) {
				showErrorMessage(
					tr("Could not allocate layers for underpaint candidates."));
				return;
			}

			const uint8_t contextId = currentCanvas->localUserId();
			const int groupId = layerIds.first();
			const int parentGroupId =
				parentGroupIdForLayer(currentLayers, activeLayerId);
			net::MessageList messages;
			messages.append(net::makeUndoPointMessage(contextId));
			messages.append(
				net::makeLayerTreeCreateMessage(
					contextId, groupId, 0, activeLayerId, 0,
					DP_MSG_LAYER_TREE_CREATE_FLAGS_GROUP,
					currentLayers->getAvailableLayerName(
						tr("Underpaint Candidates"))));
			messages.append(
				net::makeLayerTreeMoveMessage(
					contextId, groupId, parentGroupId, activeLayerId));

			QVector<int> importedLayerIds;
			importedLayerIds.reserve(candidateCount);
			QVector<ai::JobCandidate> importedCandidates;
			importedCandidates.reserve(candidateCount);
			for(int i = 0; i < candidateCount; ++i) {
				const ai::JobCandidate &candidate =
					jobResult.response.candidates.at(i);
				QImage image(candidate.imagePath);
				if(image.isNull()) {
					continue;
				}
				const int layerId = layerIds.at(i + 1);
				importedLayerIds.append(layerId);
				importedCandidates.append(candidate);
				messages.append(
					net::makeLayerTreeCreateMessage(
						contextId, layerId, 0, groupId, 0,
						DP_MSG_LAYER_TREE_CREATE_FLAGS_INTO,
						currentLayers->getAvailableLayerName(
							candidateLayerLabel(candidate, tr("Underpaint")))));
				net::makePutImageMessagesCompat(
					messages, contextId, layerId, DP_BLEND_MODE_NORMAL,
					exportRegion.x(), exportRegion.y(), image,
					currentCanvas->isCompatibilityMode());
			}

			if(importedLayerIds.isEmpty()) {
				showErrorMessage(
					tr("The AI worker returned unreadable candidates."));
				return;
			}

			currentLayers->setLayerIdToSelect(importedLayerIds.first());
			m_doc->client()->sendCommands(messages.size(), messages.constData());
			for(int i = 1; i < importedLayerIds.size(); ++i) {
				currentCanvas->paintEngine()->setLayerVisibility(
					importedLayerIds.at(i), true);
			}

			ai::JobRunResult importedJobResult = jobResult;
			importedJobResult.response.candidates = importedCandidates;
			const bool accepted = showInpaintCandidateDialog(
				this, currentCanvas, importedLayerIds, importedJobResult,
				tr("Underpaint Candidates"),
				tr("Choose the repaired background to keep behind the layer."));
			if(!accepted) {
				net::MessageList deleteMessages;
				deleteMessages.append(net::makeUndoPointMessage(contextId));
				deleteMessages.append(
					net::makeLayerTreeDeleteMessage(contextId, groupId, 0));
				m_doc->client()->sendCommands(
					deleteMessages.size(), deleteMessages.constData());
			}
			m_dockLayers->selectLayer(activeLayerId);
		});
		thread->start();
	});
	connect(aiInpaint, &QAction::triggered, this, [=] {
		canvas::CanvasModel *canvas = m_doc->canvas();
		if(!canvas) {
			showErrorMessage(tr("No canvas is available for inpaint."));
			return;
		}
		if(!m_doc->checkPermission(DP_FEATURE_PUT_IMAGE)) {
			return;
		}
		canvas::AclState *aclState = canvas->aclState();
		if(!aclState ||
		   !(aclState->canUseFeature(DP_FEATURE_EDIT_LAYERS) ||
			 aclState->canUseFeature(DP_FEATURE_OWN_LAYERS))) {
			m_doc->permissionDenied(DP_FEATURE_EDIT_LAYERS);
			return;
		}

		canvas::SelectionModel *selection = canvas->selection();
		drawdance::CanvasState canvasState =
			canvas->paintEngine()->viewCanvasState();
		const QRect canvasBounds(QPoint(), canvasState.size());
		QRect selectionRegion;
		QRect exportRegion;
		QImage mask;
		if(!selection || !selection->isValid()) {
			QAction *selectionTool = getAction("toolselectrect");
			if(selectionTool && !selectionTool->isChecked()) {
				selectionTool->trigger();
			}
			showErrorMessage(
				tr("Select the area to repaint, then run Inpaint again."));
			return;
		}

		selectionRegion = canvasBounds.intersected(selection->bounds());
		const int contextPadding = 128;
		exportRegion = selectionRegion
						   .adjusted(
							   -contextPadding, -contextPadding, contextPadding,
							   contextPadding)
						   .intersected(canvasBounds);
		QString selectionSource = QStringLiteral("current-selection");
		mask = selectionMaskToInpaintMask(
			selection->image(), selectionRegion, exportRegion);
		if(selectionRegion.isEmpty() || exportRegion.isEmpty()) {
			showErrorMessage(tr("The selected area is empty."));
			return;
		}
		if(mask.isNull()) {
			showErrorMessage(tr("Could not export the selection mask for AI."));
			return;
		}

			const int sourceLayerId = validInpaintAnchorLayer(
				canvas->layerlist(), m_doc->toolCtrl()->activeLayer());
			static InpaintOptions lastInpaintSettings;
			InpaintOptions options = lastInpaintSettings;
			const RefinerOptions refinerDefaults = loadRefinerOptions();
			options.refinerEnabled = refinerDefaults.enabled;
			options.refinerPlacement = refinerDefaults.placement;
			options.detailPassEnabled = loadDetailPassOptions().enabled;
		options.prompt.clear();
		options.negativePrompt.clear();
		options.prompt = takeReusableInpaintPrompt();
		if(!showInpaintOptionsDialog(
			   this, selectionRegion, options, tr("Inpaint"), tr("Selection"),
			   tr("Describe what should appear in the selected area"),
			   QStringLiteral("inpaint-prompt-improve"),
			   [canvas, canvasState, selectionRegion] {
				   drawdance::ViewModeBuffer viewModeBuffer;
				   QRect region = selectionRegion;
				   return canvas->paintEngine()->getFlatImage(
					   viewModeBuffer, canvasState, true, true, &region);
			   })) {
			return;
		}
			rememberInpaintPrompt(options.prompt);
			lastInpaintSettings = options;
			lastInpaintSettings.prompt.clear();
			lastInpaintSettings.negativePrompt.clear();
			const QString refinerError = refinerRunBlocker(options.refinerEnabled);
			if(!refinerError.isEmpty()) {
				showErrorMessageWithDetails(
					tr("Refiner backend is not ready."), refinerError);
				return;
			}

			drawdance::ViewModeBuffer viewModeBuffer;
		QImage sourceImage = canvas->paintEngine()->getFlatImage(
			viewModeBuffer, canvasState, true, true, &exportRegion);
		if(sourceImage.isNull()) {
			showErrorMessage(tr("Could not export the source image for AI."));
			return;
		}

		QTemporaryDir assetDir(
			QDir::temp().filePath(QStringLiteral("underpaint-ai-assets-XXXXXX")));
		if(!assetDir.isValid()) {
			showErrorMessage(tr("Could not create AI asset directory."));
			return;
		}
		assetDir.setAutoRemove(false);
		const QString sourcePath =
			QDir(assetDir.path()).filePath(QStringLiteral("source.png"));
		const QString maskPath =
			QDir(assetDir.path()).filePath(QStringLiteral("mask.png"));
		if(!sourceImage.save(sourcePath, "PNG") || !mask.save(maskPath, "PNG")) {
			showErrorMessage(tr("Could not write AI source assets."));
			return;
		}

		ai::JobRequest request =
			ai::JobRequest::create(ai::Operation::Inpaint);
		ai::JobAsset sourceAsset;
		sourceAsset.role = QStringLiteral("source-image");
		sourceAsset.path = sourcePath;
		sourceAsset.mimeType = QStringLiteral("image/png");
		sourceAsset.metadata = QJsonObject{
			{QStringLiteral("colorSpace"), QStringLiteral("srgb")},
		};
		ai::JobAsset maskAsset;
		maskAsset.role = QStringLiteral("mask");
		maskAsset.path = maskPath;
		maskAsset.mimeType = QStringLiteral("image/png");
		maskAsset.metadata = QJsonObject{
			{QStringLiteral("whiteMeans"), QStringLiteral("editable-region")},
		};
		request.inputs = {sourceAsset, maskAsset};
			request.parameters = QJsonObject{
				{QStringLiteral("prompt"), options.prompt},
				{QStringLiteral("negativePrompt"), options.negativePrompt},
				{QStringLiteral("seed"), options.seed},
				{QStringLiteral("cfg"), options.cfg},
				{QStringLiteral("denoise"), options.denoise},
				{QStringLiteral("scheduler"), options.scheduler},
				{QStringLiteral("steps"), options.steps},
				{QStringLiteral("refiner"),
				 refinerParameters(options.refinerEnabled, options.scheduler)},
				{QStringLiteral("detailPass"),
				 detailPassParameters(options.detailPassEnabled, options.scheduler)},
				{QStringLiteral("candidateCount"), options.candidateCount},
				{QStringLiteral("edgeFeatherPx"), options.edgeFeatherPx},
			};
		request.preferences = QJsonObject{
			{QStringLiteral("maxRenderEdge"), 1024},
			{QStringLiteral("variantMode"), QStringLiteral("sequential")},
			{QStringLiteral("unloadPolicy"), QStringLiteral("idle")},
			{QStringLiteral("vaeTiling"), true},
			{QStringLiteral("cacheGuides"), true},
			{QStringLiteral("safe4070Mode"), true},
		};

		request.region = QJsonObject{
			{QStringLiteral("x"), exportRegion.x()},
			{QStringLiteral("y"), exportRegion.y()},
			{QStringLiteral("width"), exportRegion.width()},
			{QStringLiteral("height"), exportRegion.height()},
			{QStringLiteral("selectionX"), selectionRegion.x()},
			{QStringLiteral("selectionY"), selectionRegion.y()},
			{QStringLiteral("selectionWidth"), selectionRegion.width()},
			{QStringLiteral("selectionHeight"), selectionRegion.height()},
			{QStringLiteral("contextPadding"), contextPadding},
		};
		request.source = QJsonObject{
			{QStringLiteral("activeLayerId"), sourceLayerId},
			{QStringLiteral("selectionSource"), selectionSource},
		};
			request.provenance = QJsonObject{
				{QStringLiteral("createdBy"), QStringLiteral("underpaint")},
				{QStringLiteral("uiEntryPoint"), QStringLiteral("Power Tools/Inpaint")},
			};

			const int progressSteps =
				effectiveDiffusionSteps(options.steps, options.denoise);
			const RefinerOptions progressRefinerOptions = loadRefinerOptions();
			const int progressRefinerSteps =
				options.refinerEnabled
					? effectiveDiffusionSteps(
						  progressRefinerOptions.steps,
						  progressRefinerOptions.strength)
					: 0;
			const DetailPassOptions progressDetailOptions = loadDetailPassOptions();
			const int progressDetailSteps =
				options.detailPassEnabled
					? effectiveDiffusionSteps(
						  progressDetailOptions.steps, progressDetailOptions.denoise)
					: 0;
		const int progressStepStride =
			progressSteps + progressRefinerSteps + progressDetailSteps;
		const int progressCandidates = qMax(1, options.candidateCount);

		QProgressDialog *progress = new QProgressDialog(
			tr("Loading AI model and generating candidates..."), tr("Cancel"), 0,
			qMax(1, progressCandidates * progressStepStride), this);
		AiPreviewLabel *progressLabel =
			new AiPreviewLabel(progress);
		progressLabel->setText(tr("Loading AI model and generating candidates..."));
		progressLabel->setAlignment(Qt::AlignCenter);
		progressLabel->setMinimumWidth(280);
		progressLabel->setMinimumHeight(192);
		progress->setLabel(progressLabel);
		progress->setWindowTitle(tr("Inpaint"));
		progress->setWindowModality(Qt::WindowModal);
		progress->setMinimumDuration(0);
		progress->setAutoClose(false);
		progress->setAutoReset(false);
		progress->setValue(0);
		progress->show();

		auto cancelRequested = std::make_shared<std::atomic_bool>(false);
		connect(progress, &QProgressDialog::canceled, this, [cancelRequested] {
			cancelRequested->store(true);
		});

		ai::JobRunResult *result = new ai::JobRunResult;
		QPointer<AiPreviewLabel> progressLabelPointer(progressLabel);
		QPointer<QProgressDialog> progressPointer(progress);
		QThread *thread = QThread::create(
			[request, result, progressLabelPointer, progressPointer,
			 progressSteps, progressRefinerSteps, progressDetailSteps,
			 progressStepStride, progressCandidates, cancelRequested] {
			*result = ai::JobRunner::run(
				request, QString(), 15 * 60 * 1000,
				[progressLabelPointer, progressPointer, progressSteps,
				 progressRefinerSteps, progressDetailSteps, progressStepStride,
				 progressCandidates](const QJsonObject &event) {
					QMetaObject::invokeMethod(
						qApp,
						[progressLabelPointer, progressPointer, progressSteps,
						 progressRefinerSteps, progressDetailSteps, progressStepStride,
						 progressCandidates, event] {
							if(!progressLabelPointer || !progressPointer) {
								return;
							}
							const QString type =
								event.value(QStringLiteral("type")).toString();
							if(type == QStringLiteral("detail")) {
								const int candidate =
									event.value(QStringLiteral("candidate")).toInt();
								const QString status =
									event.value(QStringLiteral("status")).toString();
								const QString region =
									event.value(QStringLiteral("region")).toString();
								const int step =
									event.value(QStringLiteral("step")).toInt();
								const int steps =
									event.value(QStringLiteral("steps")).toInt();
								QString text = MainWindow::tr("Candidate %1 detail pass")
												   .arg(candidate);
								if(!status.isEmpty()) {
									text += MainWindow::tr(" - %1").arg(status);
								}
								if(!region.isEmpty()) {
									text += MainWindow::tr(" (%1)").arg(region);
								}
								if(step > 0 && steps > 0) {
									text += MainWindow::tr(" - step %1/%2")
												.arg(step)
												.arg(steps);
								}
								progressLabelPointer->setToolTip(text);
								progressLabelPointer->setStatusText(text);
								const int candidateIndex = qMax(1, candidate);
								const int detailStepCount =
									steps > 0 ? steps : progressDetailSteps;
								int progressValue =
									(candidateIndex - 1) * progressStepStride +
									progressSteps + progressRefinerSteps;
								if(step > 0 && detailStepCount > 0) {
									progressValue += qMin(qMax(1, step), detailStepCount);
								}
								progressPointer->setMaximum(
									qMax(1, progressCandidates * progressStepStride));
								progressPointer->setValue(qMin(
									progressValue, progressPointer->maximum()));
								return;
							}
							if(type == QStringLiteral("refiner")) {
								const int candidate =
									event.value(QStringLiteral("candidate")).toInt();
								const QString status =
									event.value(QStringLiteral("status")).toString();
								const int step =
									event.value(QStringLiteral("step")).toInt();
								const int steps =
									event.value(QStringLiteral("steps")).toInt();
								QString text = MainWindow::tr("Candidate %1 refiner")
												   .arg(candidate);
								if(!status.isEmpty()) {
									text += MainWindow::tr(" - %1").arg(status);
								}
								if(step > 0 && steps > 0) {
									text += MainWindow::tr(" - step %1/%2")
												.arg(step)
												.arg(steps);
								}
								progressLabelPointer->setToolTip(text);
								progressLabelPointer->setStatusText(text);
								const int candidateIndex = qMax(1, candidate);
								const int refinerStepCount =
									steps > 0 ? steps : progressRefinerSteps;
								int progressValue =
									(candidateIndex - 1) * progressStepStride +
									progressSteps;
								if(step > 0 && refinerStepCount > 0) {
									progressValue += qMin(qMax(1, step), refinerStepCount);
								}
								progressPointer->setMaximum(
									qMax(1, progressCandidates * progressStepStride));
								progressPointer->setValue(qMin(
									progressValue, progressPointer->maximum()));
								return;
							}
							if(type != QStringLiteral("preview") &&
							   type != QStringLiteral("candidate")) {
								return;
							}
							const QString imagePath =
								event.value(QStringLiteral("imagePath")).toString();
							QPixmap preview(imagePath);
							if(preview.isNull()) {
								return;
							}
							const int candidate =
								event.value(QStringLiteral("candidate")).toInt();
							const int step = event.value(QStringLiteral("step")).toInt();
							const int steps =
								event.value(QStringLiteral("steps")).toInt();
							const int seed = event.value(QStringLiteral("seed")).toInt(-1);
							QString text = type == QStringLiteral("preview")
											   ? MainWindow::tr(
													 "Candidate %1 preview")
													 .arg(candidate)
											   : MainWindow::tr(
													 "Candidate %1 complete")
													 .arg(candidate);
							if(step > 0 && steps > 0) {
								text += MainWindow::tr(" - step %1/%2")
											.arg(step)
											.arg(steps);
							}
							if(seed >= 0) {
								text += MainWindow::tr(" - seed %1").arg(seed);
							}
							progressLabelPointer->setToolTip(text);
							progressLabelPointer->setStatusText(text);
							progressLabelPointer->setPreviewPixmap(preview.scaled(
								256, 192, Qt::KeepAspectRatio,
								Qt::SmoothTransformation));
							const int candidateIndex = qMax(1, candidate);
							int progressValue = 0;
							if(type == QStringLiteral("preview")) {
								const int stepCount = steps > 0 ? steps : progressSteps;
								progressValue =
									(candidateIndex - 1) * progressStepStride +
									qMin(qMax(1, step), stepCount);
							} else {
								progressValue = candidateIndex * progressStepStride;
							}
							progressPointer->setMaximum(
								qMax(1, progressCandidates * progressStepStride));
							progressPointer->setValue(qMin(
								progressValue, progressPointer->maximum()));
						},
						Qt::QueuedConnection);
				},
				cancelRequested.get());
		});
		connect(thread, &QThread::finished, this, [=] {
			progress->close();
			progress->deleteLater();

			const ai::JobRunResult jobResult = *result;
			delete result;
			thread->deleteLater();

			canvas::CanvasModel *currentCanvas = m_doc->canvas();
			if(!currentCanvas) {
				showErrorMessage(
					tr("The canvas was closed before AI results could import."));
				return;
			}

			if(jobResult.canceled) {
				return;
			}
			if(!jobResult.ok) {
				showErrorMessageWithDetails(
					tr("Inpaint worker failed."),
					jobResult.errorMessage);
				return;
			}

			if(jobResult.response.candidates.isEmpty()) {
				showErrorMessage(tr("The AI worker returned no candidates."));
				return;
			}

			canvas::LayerListModel *layers = currentCanvas->layerlist();
			const int candidateCount = jobResult.response.candidates.size();
			QVector<int> layerIds =
				layers->getAvailableLayerIds(candidateCount + 1);
			if(layerIds.size() < candidateCount + 1) {
				showErrorMessage(
					tr("Could not allocate layers for AI candidates."));
				return;
			}

			const uint8_t contextId = currentCanvas->localUserId();
			const int groupId = layerIds.first();
			net::MessageList messages;
			messages.append(net::makeUndoPointMessage(contextId));
			messages.append(
				net::makeLayerTreeCreateMessage(
					contextId, groupId, 0, 0, 0,
					DP_MSG_LAYER_TREE_CREATE_FLAGS_GROUP,
					layers->getAvailableLayerName(
						tr("Inpaint Candidates"))));

			QVector<int> importedLayerIds;
			importedLayerIds.reserve(candidateCount);
			QVector<ai::JobCandidate> importedCandidates;
			importedCandidates.reserve(candidateCount);
			for(int i = 0; i < candidateCount; ++i) {
				const ai::JobCandidate &candidate =
					jobResult.response.candidates.at(i);
				QImage image(candidate.imagePath);
				if(image.isNull()) {
					continue;
				}
				const int layerId = layerIds.at(i + 1);
				importedLayerIds.append(layerId);
				importedCandidates.append(candidate);
				messages.append(
					net::makeLayerTreeCreateMessage(
						contextId, layerId, 0, groupId, 0,
						DP_MSG_LAYER_TREE_CREATE_FLAGS_INTO,
						layers->getAvailableLayerName(
							candidateLayerLabel(candidate))));
				net::makePutImageMessagesCompat(
					messages, contextId, layerId, DP_BLEND_MODE_NORMAL,
					exportRegion.x(), exportRegion.y(), image,
					currentCanvas->isCompatibilityMode());
			}

			if(importedLayerIds.isEmpty()) {
				showErrorMessage(
					tr("The AI worker returned unreadable candidates."));
				return;
			}

			layers->setLayerIdToSelect(importedLayerIds.first());
			m_doc->client()->sendCommands(messages.size(), messages.constData());
			for(int i = 1; i < importedLayerIds.size(); ++i) {
				currentCanvas->paintEngine()->setLayerVisibility(
					importedLayerIds.at(i), true);
			}

			ai::JobRunResult importedJobResult = jobResult;
			importedJobResult.response.candidates = importedCandidates;
			const bool accepted = showInpaintCandidateDialog(
				this, currentCanvas, importedLayerIds, importedJobResult,
				tr("Inpaint Candidates"),
				tr("Choose the candidate to show on the canvas."));
			if(!accepted) {
				net::MessageList deleteMessages;
				deleteMessages.append(net::makeUndoPointMessage(contextId));
				deleteMessages.append(
					net::makeLayerTreeDeleteMessage(contextId, groupId, 0));
				m_doc->client()->sendCommands(
					deleteMessages.size(), deleteMessages.constData());
			}
			if(sourceLayerId > 0) {
				m_dockLayers->selectLayer(sourceLayerId);
			}
		});
		thread->start();
	});
	connect(aiOutpaint, &QAction::triggered, this, [=] {
		canvas::CanvasModel *canvas = m_doc->canvas();
		if(!canvas) {
			showErrorMessage(tr("No canvas is available for outpaint."));
			return;
		}
		if(!m_doc->checkPermission(DP_FEATURE_PUT_IMAGE)) {
			return;
		}
		canvas::AclState *aclState = canvas->aclState();
		if(!aclState ||
		   !(aclState->canUseFeature(DP_FEATURE_EDIT_LAYERS) ||
			 aclState->canUseFeature(DP_FEATURE_OWN_LAYERS))) {
			m_doc->permissionDenied(DP_FEATURE_EDIT_LAYERS);
			return;
		}

		drawdance::CanvasState canvasState =
			canvas->paintEngine()->viewCanvasState();
		const QRect canvasBounds(QPoint(), canvasState.size());
		if(canvasBounds.isEmpty()) {
			showErrorMessage(tr("The canvas is empty."));
			return;
		}

		canvas::SelectionModel *selection = canvas->selection();
		const int contextPadding = 128;
		const int contextBleedPx = 48;
		QRect targetRegion;
		QRect exportRegion;
		QImage mask;
		QString selectionSource;
		if(selection && selection->isValid()) {
			targetRegion = canvasBounds.intersected(selection->bounds());
			if(targetRegion.isEmpty()) {
				showErrorMessage(tr("The selected outpaint area is empty."));
				return;
			}
			exportRegion = targetRegion
							   .adjusted(
								   -contextPadding, -contextPadding,
								   contextPadding, contextPadding)
							   .intersected(canvasBounds);
			mask = selectionMaskToInpaintMask(
				selection->image(), targetRegion, exportRegion);
			selectionSource = QStringLiteral("current-selection");
		} else {
			const QImage alphaImage =
				canvas->paintEngine()->getLayerImage(-1, canvasBounds);
			targetRegion = transparentPixelBounds(alphaImage);
			if(targetRegion.isEmpty()) {
				showErrorMessage(
					tr("Expand the canvas to create transparent space, or "
					   "select an edge region to outpaint."));
				return;
			}
			exportRegion = targetRegion
							   .adjusted(
								   -contextPadding, -contextPadding,
								   contextPadding, contextPadding)
							   .intersected(canvasBounds);
			mask = transparentPixelsToOutpaintMask(
				alphaImage, exportRegion, 8, contextBleedPx);
			selectionSource = QStringLiteral("transparent-canvas");
		}
		if(exportRegion.isEmpty()) {
			showErrorMessage(tr("The outpaint region is empty."));
			return;
		}
		if(mask.isNull() || !maskHasEditablePixels(mask)) {
			showErrorMessage(tr("Could not export an editable outpaint mask."));
			return;
		}

		const int sourceLayerId = validInpaintAnchorLayer(
			canvas->layerlist(), m_doc->toolCtrl()->activeLayer());
		static InpaintOptions lastOutpaintSettings;
		static bool lastOutpaintSettingsInitialized = false;
		InpaintOptions options = lastOutpaintSettings;
				if(!lastOutpaintSettingsInitialized) {
					options.denoise = 0.9;
					options.samplerPreset = QStringLiteral("custom");
				}
				const RefinerOptions refinerDefaults = loadRefinerOptions();
				options.refinerEnabled = refinerDefaults.enabled;
				options.refinerPlacement = refinerDefaults.placement;
				options.detailPassEnabled = loadDetailPassOptions().enabled;
			options.prompt.clear();
			options.negativePrompt.clear();
			options.prompt = takeReusableInpaintPrompt();
		if(!showInpaintOptionsDialog(
			   this, targetRegion, options, tr("Outpaint"),
			   tr("Outpaint region"),
			   tr("Describe what should appear in the expanded area"),
			   QStringLiteral("outpaint-prompt-improve"),
			   [canvas, canvasState, exportRegion] {
				   drawdance::ViewModeBuffer viewModeBuffer;
				   QRect region = exportRegion;
				   return canvas->paintEngine()->getFlatImage(
					   viewModeBuffer, canvasState, false, true, &region);
			   })) {
			return;
		}
			rememberInpaintPrompt(options.prompt);
			lastOutpaintSettings = options;
			lastOutpaintSettingsInitialized = true;
			lastOutpaintSettings.prompt.clear();
			lastOutpaintSettings.negativePrompt.clear();
			const QString refinerError = refinerRunBlocker(options.refinerEnabled);
			if(!refinerError.isEmpty()) {
				showErrorMessageWithDetails(
					tr("Refiner backend is not ready."), refinerError);
				return;
			}

			drawdance::ViewModeBuffer viewModeBuffer;
		QImage sourceImage = canvas->paintEngine()->getFlatImage(
			viewModeBuffer, canvasState, false, true, &exportRegion);
		if(sourceImage.isNull()) {
			showErrorMessage(tr("Could not export the source image for AI."));
			return;
		}

		QTemporaryDir assetDir(
			QDir::temp().filePath(QStringLiteral("underpaint-ai-assets-XXXXXX")));
		if(!assetDir.isValid()) {
			showErrorMessage(tr("Could not create AI asset directory."));
			return;
		}
		assetDir.setAutoRemove(false);
		const QString sourcePath =
			QDir(assetDir.path()).filePath(QStringLiteral("source.png"));
		const QString maskPath =
			QDir(assetDir.path()).filePath(QStringLiteral("mask.png"));
		if(!sourceImage.save(sourcePath, "PNG") || !mask.save(maskPath, "PNG")) {
			showErrorMessage(tr("Could not write AI source assets."));
			return;
		}

		ai::JobRequest request =
			ai::JobRequest::create(ai::Operation::Outpaint);
		ai::JobAsset sourceAsset;
		sourceAsset.role = QStringLiteral("source-image");
		sourceAsset.path = sourcePath;
		sourceAsset.mimeType = QStringLiteral("image/png");
		sourceAsset.metadata = QJsonObject{
			{QStringLiteral("colorSpace"), QStringLiteral("srgb")},
		};
		ai::JobAsset maskAsset;
		maskAsset.role = QStringLiteral("mask");
		maskAsset.path = maskPath;
		maskAsset.mimeType = QStringLiteral("image/png");
		maskAsset.metadata = QJsonObject{
			{QStringLiteral("whiteMeans"), QStringLiteral("editable-region")},
		};
		request.inputs = {sourceAsset, maskAsset};
			request.parameters = QJsonObject{
				{QStringLiteral("prompt"), options.prompt},
				{QStringLiteral("negativePrompt"), options.negativePrompt},
				{QStringLiteral("seed"), options.seed},
				{QStringLiteral("cfg"), options.cfg},
				{QStringLiteral("denoise"), options.denoise},
				{QStringLiteral("scheduler"), options.scheduler},
				{QStringLiteral("steps"), options.steps},
				{QStringLiteral("refiner"),
				 refinerParameters(options.refinerEnabled, options.scheduler)},
				{QStringLiteral("detailPass"),
				 detailPassParameters(options.detailPassEnabled, options.scheduler)},
				{QStringLiteral("candidateCount"), options.candidateCount},
				{QStringLiteral("edgeFeatherPx"), options.edgeFeatherPx},
				{QStringLiteral("contextBleedPx"), contextBleedPx},
				{QStringLiteral("prefillNoise"), 0.0},
				{QStringLiteral("prefillStyle"), QStringLiteral("edge-slices-25")},
			};
		request.preferences = QJsonObject{
			{QStringLiteral("maxRenderEdge"), 1024},
			{QStringLiteral("variantMode"), QStringLiteral("sequential")},
			{QStringLiteral("unloadPolicy"), QStringLiteral("idle")},
			{QStringLiteral("vaeTiling"), true},
			{QStringLiteral("cacheGuides"), true},
			{QStringLiteral("safe4070Mode"), true},
		};
		request.region = QJsonObject{
			{QStringLiteral("x"), exportRegion.x()},
			{QStringLiteral("y"), exportRegion.y()},
			{QStringLiteral("width"), exportRegion.width()},
			{QStringLiteral("height"), exportRegion.height()},
			{QStringLiteral("selectionX"), targetRegion.x()},
			{QStringLiteral("selectionY"), targetRegion.y()},
			{QStringLiteral("selectionWidth"), targetRegion.width()},
			{QStringLiteral("selectionHeight"), targetRegion.height()},
			{QStringLiteral("contextPadding"), contextPadding},
		};
		request.source = QJsonObject{
			{QStringLiteral("activeLayerId"), sourceLayerId},
			{QStringLiteral("selectionSource"), selectionSource},
		};
			request.provenance = QJsonObject{
				{QStringLiteral("createdBy"), QStringLiteral("underpaint")},
				{QStringLiteral("uiEntryPoint"), QStringLiteral("Power Tools/Outpaint")},
			};

			const int progressSteps =
				effectiveDiffusionSteps(options.steps, options.denoise);
			const RefinerOptions progressRefinerOptions = loadRefinerOptions();
			const int progressRefinerSteps =
				options.refinerEnabled
					? effectiveDiffusionSteps(
						  progressRefinerOptions.steps,
						  progressRefinerOptions.strength)
					: 0;
			const DetailPassOptions progressDetailOptions = loadDetailPassOptions();
			const int progressDetailSteps =
				options.detailPassEnabled
					? effectiveDiffusionSteps(
						  progressDetailOptions.steps, progressDetailOptions.denoise)
					: 0;
		const int progressStepStride =
			progressSteps + progressRefinerSteps + progressDetailSteps;
		const int progressCandidates = qMax(1, options.candidateCount);

		QProgressDialog *progress = new QProgressDialog(
			tr("Loading AI model and generating outpaint candidates..."),
			tr("Cancel"), 0, qMax(1, progressCandidates * progressStepStride),
			this);
		AiPreviewLabel *progressLabel = new AiPreviewLabel(progress);
		progressLabel->setText(
			tr("Loading AI model and generating outpaint candidates..."));
		progressLabel->setAlignment(Qt::AlignCenter);
		progressLabel->setMinimumWidth(280);
		progressLabel->setMinimumHeight(192);
		progress->setLabel(progressLabel);
		progress->setWindowTitle(tr("Outpaint"));
		progress->setWindowModality(Qt::WindowModal);
		progress->setMinimumDuration(0);
		progress->setAutoClose(false);
		progress->setAutoReset(false);
		progress->setValue(0);
		progress->show();

		auto cancelRequested = std::make_shared<std::atomic_bool>(false);
		connect(progress, &QProgressDialog::canceled, this, [cancelRequested] {
			cancelRequested->store(true);
		});

		ai::JobRunResult *result = new ai::JobRunResult;
		QPointer<AiPreviewLabel> progressLabelPointer(progressLabel);
		QPointer<QProgressDialog> progressPointer(progress);
		QThread *thread = QThread::create(
			[request, result, progressLabelPointer, progressPointer,
			 progressSteps, progressRefinerSteps, progressDetailSteps,
			 progressStepStride, progressCandidates, cancelRequested] {
			*result = ai::JobRunner::run(
				request, QString(), 15 * 60 * 1000,
				[progressLabelPointer, progressPointer, progressSteps,
				 progressRefinerSteps, progressDetailSteps, progressStepStride,
				 progressCandidates](const QJsonObject &event) {
					QMetaObject::invokeMethod(
						qApp,
						[progressLabelPointer, progressPointer, progressSteps,
						 progressRefinerSteps, progressDetailSteps, progressStepStride,
						 progressCandidates, event] {
							if(!progressLabelPointer || !progressPointer) {
								return;
							}
							const QString type =
								event.value(QStringLiteral("type")).toString();
							if(type == QStringLiteral("detail")) {
								const int candidate =
									event.value(QStringLiteral("candidate")).toInt();
								const QString status =
									event.value(QStringLiteral("status")).toString();
								const QString region =
									event.value(QStringLiteral("region")).toString();
								const int step =
									event.value(QStringLiteral("step")).toInt();
								const int steps =
									event.value(QStringLiteral("steps")).toInt();
								QString text = MainWindow::tr("Candidate %1 detail pass")
												   .arg(candidate);
								if(!status.isEmpty()) {
									text += MainWindow::tr(" - %1").arg(status);
								}
								if(!region.isEmpty()) {
									text += MainWindow::tr(" (%1)").arg(region);
								}
								if(step > 0 && steps > 0) {
									text += MainWindow::tr(" - step %1/%2")
												.arg(step)
												.arg(steps);
								}
								progressLabelPointer->setToolTip(text);
								progressLabelPointer->setStatusText(text);
								const int candidateIndex = qMax(1, candidate);
								const int detailStepCount =
									steps > 0 ? steps : progressDetailSteps;
								int progressValue =
									(candidateIndex - 1) * progressStepStride +
									progressSteps + progressRefinerSteps;
								if(step > 0 && detailStepCount > 0) {
									progressValue += qMin(qMax(1, step), detailStepCount);
								}
								progressPointer->setMaximum(
									qMax(1, progressCandidates * progressStepStride));
								progressPointer->setValue(qMin(
									progressValue, progressPointer->maximum()));
								return;
							}
							if(type == QStringLiteral("refiner")) {
								const int candidate =
									event.value(QStringLiteral("candidate")).toInt();
								const QString status =
									event.value(QStringLiteral("status")).toString();
								const int step =
									event.value(QStringLiteral("step")).toInt();
								const int steps =
									event.value(QStringLiteral("steps")).toInt();
								QString text = MainWindow::tr("Candidate %1 refiner")
												   .arg(candidate);
								if(!status.isEmpty()) {
									text += MainWindow::tr(" - %1").arg(status);
								}
								if(step > 0 && steps > 0) {
									text += MainWindow::tr(" - step %1/%2")
												.arg(step)
												.arg(steps);
								}
								progressLabelPointer->setToolTip(text);
								progressLabelPointer->setStatusText(text);
								const int candidateIndex = qMax(1, candidate);
								const int refinerStepCount =
									steps > 0 ? steps : progressRefinerSteps;
								int progressValue =
									(candidateIndex - 1) * progressStepStride +
									progressSteps;
								if(step > 0 && refinerStepCount > 0) {
									progressValue += qMin(qMax(1, step), refinerStepCount);
								}
								progressPointer->setMaximum(
									qMax(1, progressCandidates * progressStepStride));
								progressPointer->setValue(qMin(
									progressValue, progressPointer->maximum()));
								return;
							}
							if(type != QStringLiteral("preview") &&
							   type != QStringLiteral("candidate")) {
								return;
							}
							const QString imagePath =
								event.value(QStringLiteral("imagePath")).toString();
							QPixmap preview(imagePath);
							if(preview.isNull()) {
								return;
							}
							const int candidate =
								event.value(QStringLiteral("candidate")).toInt();
							const int step = event.value(QStringLiteral("step")).toInt();
							const int steps =
								event.value(QStringLiteral("steps")).toInt();
							const int seed =
								event.value(QStringLiteral("seed")).toInt(-1);
							QString text =
								type == QStringLiteral("preview")
									? MainWindow::tr("Candidate %1 preview")
										  .arg(candidate)
									: MainWindow::tr("Candidate %1 complete")
										  .arg(candidate);
							if(step > 0 && steps > 0) {
								text += MainWindow::tr(" - step %1/%2")
											.arg(step)
											.arg(steps);
							}
							if(seed >= 0) {
								text += MainWindow::tr(" - seed %1").arg(seed);
							}
							progressLabelPointer->setToolTip(text);
							progressLabelPointer->setStatusText(text);
							progressLabelPointer->setPreviewPixmap(preview.scaled(
								256, 192, Qt::KeepAspectRatio,
								Qt::SmoothTransformation));
							const int candidateIndex = qMax(1, candidate);
							int progressValue = 0;
							if(type == QStringLiteral("preview")) {
								const int stepCount =
									steps > 0 ? steps : progressSteps;
								progressValue =
									(candidateIndex - 1) * progressStepStride +
									qMin(qMax(1, step), stepCount);
							} else {
								progressValue = candidateIndex * progressStepStride;
							}
							progressPointer->setMaximum(
								qMax(1, progressCandidates * progressStepStride));
							progressPointer->setValue(qMin(
								progressValue, progressPointer->maximum()));
						},
						Qt::QueuedConnection);
				},
				cancelRequested.get());
		});
		connect(thread, &QThread::finished, this, [=] {
			progress->close();
			progress->deleteLater();

			const ai::JobRunResult jobResult = *result;
			delete result;
			thread->deleteLater();

			canvas::CanvasModel *currentCanvas = m_doc->canvas();
			if(!currentCanvas) {
				showErrorMessage(
					tr("The canvas was closed before AI results could import."));
				return;
			}
			if(jobResult.canceled) {
				return;
			}
			if(!jobResult.ok) {
				showErrorMessageWithDetails(
					tr("Outpaint worker failed."), jobResult.errorMessage);
				return;
			}
			if(jobResult.response.candidates.isEmpty()) {
				showErrorMessage(tr("The AI worker returned no candidates."));
				return;
			}

			canvas::LayerListModel *layers = currentCanvas->layerlist();
			const int candidateCount = jobResult.response.candidates.size();
			QVector<int> layerIds =
				layers->getAvailableLayerIds(candidateCount + 1);
			if(layerIds.size() < candidateCount + 1) {
				showErrorMessage(
					tr("Could not allocate layers for AI candidates."));
				return;
			}

			const uint8_t contextId = currentCanvas->localUserId();
			const int groupId = layerIds.first();
			net::MessageList messages;
			messages.append(net::makeUndoPointMessage(contextId));
			messages.append(
				net::makeLayerTreeCreateMessage(
					contextId, groupId, 0, 0, 0,
					DP_MSG_LAYER_TREE_CREATE_FLAGS_GROUP,
					layers->getAvailableLayerName(
						tr("Outpaint Candidates"))));

			QVector<int> importedLayerIds;
			importedLayerIds.reserve(candidateCount);
			QVector<ai::JobCandidate> importedCandidates;
			importedCandidates.reserve(candidateCount);
			for(int i = 0; i < candidateCount; ++i) {
				const ai::JobCandidate &candidate =
					jobResult.response.candidates.at(i);
				QImage image(candidate.imagePath);
				if(image.isNull()) {
					continue;
				}
				const int layerId = layerIds.at(i + 1);
				importedLayerIds.append(layerId);
				importedCandidates.append(candidate);
				messages.append(
					net::makeLayerTreeCreateMessage(
						contextId, layerId, 0, groupId, 0,
						DP_MSG_LAYER_TREE_CREATE_FLAGS_INTO,
						layers->getAvailableLayerName(
							candidateLayerLabel(candidate, tr("Outpaint")))));
				net::makePutImageMessagesCompat(
					messages, contextId, layerId, DP_BLEND_MODE_NORMAL,
					exportRegion.x(), exportRegion.y(), image,
					currentCanvas->isCompatibilityMode());
			}

			if(importedLayerIds.isEmpty()) {
				showErrorMessage(
					tr("The AI worker returned unreadable candidates."));
				return;
			}

			layers->setLayerIdToSelect(importedLayerIds.first());
			m_doc->client()->sendCommands(messages.size(), messages.constData());
			for(int i = 1; i < importedLayerIds.size(); ++i) {
				currentCanvas->paintEngine()->setLayerVisibility(
					importedLayerIds.at(i), true);
			}

			ai::JobRunResult importedJobResult = jobResult;
			importedJobResult.response.candidates = importedCandidates;
			const bool accepted = showInpaintCandidateDialog(
				this, currentCanvas, importedLayerIds, importedJobResult,
				tr("Outpaint Candidates"),
				tr("Choose the outpaint candidate to show on the canvas."));
			if(!accepted) {
				net::MessageList deleteMessages;
				deleteMessages.append(net::makeUndoPointMessage(contextId));
				deleteMessages.append(
					net::makeLayerTreeDeleteMessage(contextId, groupId, 0));
				m_doc->client()->sendCommands(
					deleteMessages.size(), deleteMessages.constData());
			}
			if(sourceLayerId > 0) {
				m_dockLayers->selectLayer(sourceLayerId);
			}
		});
		thread->start();
	});
	connect(aiDetailSettings, &QAction::triggered, this, [=] {
		showDetailPassSettingsDialog(this);
	});
	connect(aiRefinerSettings, &QAction::triggered, this, [=] {
		showRefinerSettingsDialog(this);
	});
	connect(aiModelManager, &QAction::triggered, this, [=] {
		QString objectName = QStringLiteral("aimodelmanagerdialog");
		dialogs::AiModelManagerDialog *dlg =
			findChild<dialogs::AiModelManagerDialog *>(
				objectName, Qt::FindDirectChildrenOnly);
		if(!dlg) {
			dlg = new dialogs::AiModelManagerDialog(this);
			dlg->setAttribute(Qt::WA_DeleteOnClose);
			dlg->setObjectName(objectName);
		}
		utils::showWindow(dlg, shouldShowDialogMaximized());
	});
	connect(aiPreferences, &QAction::triggered, this, [=] {
		QString objectName = QStringLiteral("aipreferencesdialog");
		dialogs::AiPreferencesDialog *dlg =
			findChild<dialogs::AiPreferencesDialog *>(
				objectName, Qt::FindDirectChildrenOnly);
		if(!dlg) {
			dlg = new dialogs::AiPreferencesDialog(this);
			dlg->setAttribute(Qt::WA_DeleteOnClose);
			dlg->setObjectName(objectName);
		}
		utils::showWindow(dlg, shouldShowDialogMaximized());
	});

	QMenu *aiMenu = menuBar()->addMenu(tr("Power Tools"));
	aiMenu->addAction(aiObjectDecomposition);
	aiMenu->addAction(aiSceneSeparation);
	aiMenu->addAction(aiBackgroundRemoval);
	aiMenu->addAction(aiUnderpaintBehind);
	aiMenu->addAction(aiInpaint);
	aiMenu->addAction(aiOutpaint);

	aiSettingsMenu->addAction(aiModelManager);
	aiSettingsMenu->addAction(aiPreferences);
	aiSettingsMenu->addSeparator();
	aiSettingsMenu->addAction(aiRefinerSettings);
	aiSettingsMenu->addAction(aiDetailSettings);

	//
	// Animation menu
	//
	QAction *showFlipbook = makeAction("showflipbook", tr("Flipbook"))
								.icon("media-playback-start")
								.statusTip(tr("Show animation preview window"))
								.shortcut("Ctrl+F");
	QAction *animationProperties =
		makeAction("frame-count-set", tr("Change Frame Range or FPS..."))
			.icon("kdenlive-show-video")
			.noDefaultShortcut();
	QAction *timelineToolNormal =
		makeAction("timeline-tool-normal", tr("Select"))
			.icon(QStringLiteral("cursor-arrow"))
			.noDefaultShortcutWithTitle(tr("Timeline tool: select"))
			.checkable()
			.checked();
	QAction *timelineToolExposure =
		makeAction("timeline-tool-exposure", tr("Exposure"))
			.icon(QStringLiteral("drawpile_exposure"))
			.noDefaultShortcutWithTitle(tr("Timeline tool: exposure"))
			.checkable();
	QAction *timelineZoomIn =
		makeAction("timeline-zoom-in", tr("Zoom in"))
			.icon(QStringLiteral("zoom-in"))
			.noDefaultShortcutWithTitle(tr("Timeline: zoom in"))
			.autoRepeat();
	QAction *timelineZoomOut =
		makeAction("timeline-zoom-out", tr("Zoom out"))
			.icon(QStringLiteral("zoom-out"))
			.noDefaultShortcutWithTitle(tr("Timeline: zoom out"))
			.autoRepeat();
	QAction *timelineZoomReset =
		makeAction("timeline-zoom-reset", tr("Reset zoom"))
			.icon(QStringLiteral("zoom-original"))
			.noDefaultShortcutWithTitle(tr("Timeline: reset zoom"));
	QAction *keyFrameSetLayer =
		makeAction("key-frame-set-layer", tr("Set Key Frame to Current Layer"))
			.icon("keyframe")
			.shortcut("Ctrl+Shift+F");
	QAction *keyFrameSetEmpty =
		makeAction("key-frame-set-empty", tr("Set Blank Key Frame"))
			.icon("keyframe-disable")
			.shortcut("Ctrl+Shift+B");
	QAction *keyFrameCut = makeAction("key-frame-cut", tr("Cut Key Frame"))
							   .icon("edit-cut")
							   .noDefaultShortcut();
	QAction *keyFrameCopy = makeAction("key-frame-copy", tr("Copy Key Frame"))
								.icon("edit-copy")
								.noDefaultShortcut();
	QAction *keyFramePaste =
		makeAction("key-frame-paste", tr("Paste Key Frame"))
			.icon("edit-paste")
			.noDefaultShortcut();
	QVector<QAction *> keyFrameColors;
	for(const utils::MarkerColor &mc : utils::markerColors()) {
		keyFrameColors.append(
			makeAction(mc.keyFrameActionName, mc.keyFrameActionText)
				.icon(utils::makeColorIcon(16, mc.color))
				.property("markercolor", mc.color)
				.noDefaultShortcut());
	}
	QAction *keyFrameProperties =
		makeAction("key-frame-properties", tr("Key Frame Properties..."))
			.icon("configure")
			.shortcut("Ctrl+Shift+P");
	QAction *keyFrameDeleteLayer =
		makeAction("key-frame-delete", tr("Delete Key Frame"))
			.icon("keyframe-remove")
			.shortcut("Ctrl+Shift+G");
	QAction *keyFrameUnassign =
		makeAction("key-frame-unassign", tr("Unassign Key Frame"))
			.icon("drawpile_keyframe_unlink")
			.noDefaultShortcut();
	QAction *keyFrameExposureIncrease =
		makeAction(
			"key-frame-exposure-increase",
			tr("Increase Exposure on Current Track"))
			.icon("sidebar-expand-left")
			.shortcut("Ctrl+Shift++");
	QAction *keyFrameExposureIncreaseAll =
		makeAction(
			"key-frame-exposure-increase-visible",
			tr("Increase Exposure on All Tracks"))
			.shortcut("Ctrl+Shift+Alt++");
	QAction *keyFrameExposureDecrease =
		makeAction(
			"key-frame-exposure-decrease",
			tr("Decrease Exposure on Current Track"))
			.icon("sidebar-collapse-left")
			.shortcut("Ctrl+Shift+-");
	QAction *keyFrameExposureDecreaseAll =
		makeAction(
			"key-frame-exposure-decrease-visible",
			tr("Decrease Exposure on All Tracks"))
			.shortcut("Ctrl+Shift+Alt+-");
	// clang-format off
	QAction *trackAdd = makeAction("track-add", tr("New Track")).icon("list-add").noDefaultShortcut();
	QAction *trackVisible = makeAction("track-visible", tr("Track Visible for You")).checkable().noDefaultShortcut();
	QAction *trackOnionSkin = makeAction("track-onion-skin", tr("Track Onion Skin for You")).checkable().shortcut("Ctrl+Shift+O");
	QAction *trackMoveLock = makeAction("track-move-lock", tr("Track Frame Move Locked for You")).checkable().noDefaultShortcut();
	QAction *trackDuplicate = makeAction("track-duplicate", tr("Duplicate Track")).icon("edit-copy").noDefaultShortcut();
	QAction *trackRetitle = makeAction("track-retitle", tr("Rename Track")).icon("edit-rename").noDefaultShortcut();
	QAction *trackDelete = makeAction("track-delete", tr("Delete Track")).icon("trash-empty").noDefaultShortcut();
	QAction *frameNext = makeAction("frame-next", tr("Next Frame")).icon("keyframe-next").shortcut("Ctrl+Shift+L").autoRepeat();
	QAction *framePrev = makeAction("frame-prev", tr("Previous Frame")).icon("keyframe-previous").shortcut("Ctrl+Shift+H").autoRepeat();
	QAction *frameNextClamp = makeAction("frame-next-clamp", tr("Next Frame Within Range")).icon("keyframe-next").noDefaultShortcut().autoRepeat();
	QAction *framePrevClamp = makeAction("frame-prev-clamp", tr("Previous Frame Within Range")).icon("keyframe-previous").noDefaultShortcut().autoRepeat();
	QAction *keyFrameNext = makeAction("key-frame-next", tr("Next Key Frame")).icon("keyframe-next").shortcut("Ctrl+Alt+Shift+L").autoRepeat();
	QAction *keyFramePrev = makeAction("key-frame-prev", tr("Previous Key Frame")).icon("keyframe-previous").shortcut("Ctrl+Alt+Shift+H").autoRepeat();
	QAction *trackAbove = makeAction("track-above", tr("Track Above")).icon("arrow-up").shortcut("Ctrl+Shift+K").autoRepeat();
	QAction *trackBelow = makeAction("track-below", tr("Track Below")).icon("arrow-down").shortcut("Ctrl+Shift+J").autoRepeat();

	QAction *keyFrameCreateLayer = makeAction("key-frame-create-layer", tr("Create Layers on Current Key Frame")).icon("keyframe-add").shortcut("Ctrl+Shift+R");
	QAction *keyFrameCreateLayerNext = makeAction("key-frame-create-layer-next", tr("Create Layers on Next Key Frame")).icon("keyframe-next").shortcut("Ctrl+Shift+T");
	QAction *keyFrameCreateLayerPrev = makeAction("key-frame-create-layer-prev", tr("Create Layers on Previous Key Frame")).icon("keyframe-previous").shortcut("Ctrl+Shift+E");
	QAction *keyFrameCreateGroup = makeAction("key-frame-create-group", tr("Create Group on Current Key Frame")).icon("keyframe-add").noDefaultShortcut();
	QAction *keyFrameCreateGroupNext = makeAction("key-frame-create-group-next", tr("Create Group on Next Key Frame")).icon("keyframe-next").noDefaultShortcut();
	QAction *keyFrameCreateGroupPrev = makeAction("key-frame-create-group-prev", tr("Create Group on Previous Key Frame")).icon("keyframe-previous").noDefaultShortcut();
	QAction *keyFrameDuplicateNext = makeAction("key-frame-duplicate-next", tr("Duplicate to Next Key Frame")).icon("keyframe-next").shortcut("Ctrl+Alt+Shift+T");
	QAction *keyFrameDuplicatePrev = makeAction("key-frame-duplicate-prev", tr("Duplicate to Previous Key Frame")).icon("keyframe-previous").shortcut("Ctrl+Alt+Shift+E");

	// clang-format on
	QActionGroup *timelineToolGroup = new QActionGroup(this);
	timelineToolGroup->addAction(timelineToolNormal);
	timelineToolGroup->addAction(timelineToolExposure);

	QActionGroup *layerKeyFrameGroup = new QActionGroup{this};
	layerKeyFrameGroup->addAction(keyFrameCreateLayer);
	layerKeyFrameGroup->addAction(keyFrameCreateLayerNext);
	layerKeyFrameGroup->addAction(keyFrameCreateLayerPrev);
	layerKeyFrameGroup->addAction(keyFrameCreateGroup);
	layerKeyFrameGroup->addAction(keyFrameCreateGroupNext);
	layerKeyFrameGroup->addAction(keyFrameCreateGroupPrev);
	layerKeyFrameGroup->addAction(keyFrameDuplicateNext);
	layerKeyFrameGroup->addAction(keyFrameDuplicatePrev);

	QMenu *animationMenu = menuBar()->addMenu(tr("Animation"));
	animationMenu->addAction(showFlipbook);
	animationMenu->addAction(animationProperties);
	animationMenu->addAction(exportAnimation);
	animationMenu->addSeparator();
	animationMenu->addAction(keyFrameSetLayer);
	animationMenu->addAction(keyFrameSetEmpty);
	animationMenu->addAction(keyFrameCut);
	animationMenu->addAction(keyFrameCopy);
	animationMenu->addAction(keyFramePaste);
	QMenu *animationKeyFrameColorMenu = animationMenu->addMenu(
		utils::makeColorIcon(16, QColor()), tr("Key Frame Color Marker"));
	for(QAction *keyFrameColor : keyFrameColors) {
		animationKeyFrameColorMenu->addAction(keyFrameColor);
	}
	animationMenu->addAction(keyFrameProperties);
	animationMenu->addAction(keyFrameDeleteLayer);
	animationMenu->addAction(keyFrameUnassign);
	animationMenu->addSeparator();
	animationMenu->addAction(keyFrameExposureIncrease);
	animationMenu->addAction(keyFrameExposureIncreaseAll);
	animationMenu->addAction(keyFrameExposureDecrease);
	animationMenu->addAction(keyFrameExposureDecreaseAll);
	animationMenu->addSeparator();
	QMenu *animationLayerMenu = animationMenu->addMenu(
		QIcon::fromTheme("layer-visible-on"), tr("Create Layers on Key Frame"));
	animationLayerMenu->addAction(keyFrameCreateLayer);
	animationLayerMenu->addAction(keyFrameCreateLayerNext);
	animationLayerMenu->addAction(keyFrameCreateLayerPrev);
	QMenu *animationGroupMenu = animationMenu->addMenu(
		QIcon::fromTheme("folder"), tr("Create Group on Key Frame"));
	animationGroupMenu->addAction(keyFrameCreateGroup);
	animationGroupMenu->addAction(keyFrameCreateGroupNext);
	animationGroupMenu->addAction(keyFrameCreateGroupPrev);
	QMenu *animationDuplicateMenu = animationMenu->addMenu(
		QIcon::fromTheme("edit-copy"), tr("Duplicate Key Frame"));
	animationDuplicateMenu->addAction(keyFrameDuplicateNext);
	animationDuplicateMenu->addAction(keyFrameDuplicatePrev);
	animationMenu->addSeparator();
	animationMenu->addAction(trackAdd);
	animationMenu->addAction(trackDuplicate);
	animationMenu->addAction(trackRetitle);
	animationMenu->addAction(trackDelete);
	animationMenu->addAction(trackVisible);
	animationMenu->addAction(trackOnionSkin);
	animationMenu->addAction(trackMoveLock);
	animationMenu->addSeparator();
	animationMenu->addAction(frameNext);
	animationMenu->addAction(framePrev);
	animationMenu->addAction(frameNextClamp);
	animationMenu->addAction(framePrevClamp);
	animationMenu->addAction(keyFrameNext);
	animationMenu->addAction(keyFramePrev);
	animationMenu->addAction(trackAbove);
	animationMenu->addAction(trackBelow);

	m_currentdoctools->addAction(showFlipbook);
	m_dockLayers->setLayerEditActions({
		layerAdd,
		groupAdd,
		layerDupe,
		layerMerge,
		layerProperties,
		layerDelete,
		layerVisibilityToggle,
		layerSketchToggle,
		layerSetFillSource,
		layerClearFillSource,
		keyFrameSetLayer,
		keyFrameCreateLayer,
		keyFrameCreateLayerNext,
		keyFrameCreateLayerPrev,
		keyFrameCreateGroup,
		keyFrameCreateGroupNext,
		keyFrameCreateGroupPrev,
		keyFrameDuplicateNext,
		keyFrameDuplicatePrev,
		layerKeyFrameGroup,
		layerCheckToggle,
		layerCheckAll,
		layerUncheckAll,
		layerAlphaGroup,
		layerAlphaBlend,
		layerAlphaPreserve,
		layerClip,
		layerAutomaticAlphaPreserve,
		layerColorMenu,
		layerViewMenu,
		layerLockMenu,
		layerAlphaLock,
		layerLockAll,
		layerLockContent,
		layerLockProps,
		layerLockMove,
		layerCensor,
		layerCensorLocal,
	});
	m_dockTimeline->setActions(
		{
			timelineToolGroup,
			timelineToolNormal,
			timelineToolExposure,
			timelineZoomIn,
			timelineZoomOut,
			timelineZoomReset,
			keyFrameSetLayer,
			keyFrameSetEmpty,
			keyFrameCreateLayer,
			keyFrameCreateLayerNext,
			keyFrameCreateLayerPrev,
			keyFrameCreateGroup,
			keyFrameCreateGroupNext,
			keyFrameCreateGroupPrev,
			keyFrameDuplicateNext,
			keyFrameDuplicatePrev,
			keyFrameCut,
			keyFrameCopy,
			keyFramePaste,
			keyFrameProperties,
			keyFrameDeleteLayer,
			keyFrameUnassign,
			keyFrameExposureIncrease,
			keyFrameExposureIncreaseAll,
			keyFrameExposureDecrease,
			keyFrameExposureDecreaseAll,
			trackAdd,
			trackVisible,
			trackOnionSkin,
			trackMoveLock,
			trackDuplicate,
			trackRetitle,
			trackDelete,
			animationProperties,
			frameNext,
			framePrev,
			frameNextClamp,
			framePrevClamp,
			keyFrameNext,
			keyFramePrev,
			trackAbove,
			trackBelow,
			animationKeyFrameColorMenu,
			animationLayerMenu,
			animationGroupMenu,
			animationDuplicateMenu,
		},
		m_layerViewNormal, m_layerViewCurrentFrame, showFlipbook);
	m_dockToolSettings->fillSettings()->setActions(layerAutomaticAlphaPreserve);

	connect(showFlipbook, &QAction::triggered, this, &MainWindow::showFlipbook);
	connect(
		animationMenu, &QMenu ::aboutToShow, m_dockTimeline,
		&docks::Timeline::updateKeyFrameColorMenuIcon);
	// clang-format off

	//
	// Session menu
	//
	QAction *host = makeAction("hostsession", tr("&Host...")).statusTip(tr("Share your canvas with others")).noDefaultShortcut().icon("network-server");
	QAction *invite = makeAction("invitesession", tr("&Invite...")).statusTip(tr("Invite another user to this session")).noDefaultShortcut().icon("resource-group-new").disabled();
	QAction *join = makeAction("joinsession", tr("&Join...")).statusTip(tr("Join another user's drawing session")).noDefaultShortcut().icon("network-connect");
	QAction *browse = makeAction("browsesession", tr("&Browse...")).statusTip(tr("Browse session listings")).noDefaultShortcut().icon("edit-find");
	QAction *logout = makeAction("leavesession", tr("&Leave")).statusTip(tr("Leave this drawing session")).noDefaultShortcut().icon("network-disconnect").disabled();

	QAction *serverlog = makeAction("viewserverlog", tr("Event Log")).noDefaultShortcut();
	QAction *sessionSettings = makeAction("sessionsettings", tr("Settings...")).statusTip(tr("Change session settings, permissions, announcements and bans")).icon("configure").noDefaultShortcut().disabled();
	QAction *sessionUndoDepthLimit = makeAction("sessionundodepthlimit", tr("Undo Limit…")).noDefaultShortcut().disabled();

	QAction *gainop = makeAction("gainop", tr("Become Operator...")).noDefaultShortcut().disabled();
	QAction *resetsession = makeAction("resetsession", tr("&Reset...")).noDefaultShortcut().disabled();
	QAction *terminatesession = makeAction("terminatesession", tr("Terminate")).noDefaultShortcut();
	QAction *reportabuse = makeAction("reportabuse", tr("Report...")).noDefaultShortcut().disabled();

	QAction *locksession = makeAction("locksession", tr("Lock Everything")).statusTip(tr("Prevent changes to the canvas")).shortcut("F12").checkable();

	// clang-format on
	m_admintools->addAction(locksession);
	terminatesession->setEnabled(false);
	m_admintools->setEnabled(false);

	connect(host, &QAction::triggered, this, &MainWindow::host);
	connect(this, &MainWindow::hostSessionEnabled, host, &QAction::setEnabled);
	connect(invite, &QAction::triggered, this, &MainWindow::invite);
	connect(join, &QAction::triggered, this, &MainWindow::join);
	connect(browse, &QAction::triggered, this, &MainWindow::browse);
	connect(logout, &QAction::triggered, this, &MainWindow::leave);
	connect(
		sessionSettings, &QAction::triggered, this,
		&MainWindow::showSessionSettings);
	// clang-format off
	connect(sessionUndoDepthLimit, &QAction::triggered, this, &MainWindow::changeUndoDepthLimit);
	connect(serverlog, &QAction::triggered, m_serverLogDialog, [this](){
		utils::showWindow(m_serverLogDialog, shouldShowDialogMaximized());
	});
	connect(reportabuse, &QAction::triggered, this, &MainWindow::reportAbuse);
	connect(gainop, &QAction::triggered, this, &MainWindow::tryToGainOp);
	connect(locksession, &QAction::triggered, m_doc, &Document::sendLockSession);

	connect(m_doc, &Document::sessionOpwordChanged, this, [gainop, this](bool hasOpword) {
		gainop->setEnabled(hasOpword && !m_doc->canvas()->aclState()->amOperator());
	});

	connect(resetsession, &QAction::triggered, this, &MainWindow::resetSession);
	connect(terminatesession, &QAction::triggered, this, &MainWindow::terminateSession);

	QMenu *sessionmenu = menuBar()->addMenu(tr("Session"));
	sessionmenu->addAction(host);
	sessionmenu->addAction(invite);
	sessionmenu->addAction(join);
	sessionmenu->addAction(browse);
	sessionmenu->addAction(logout);
	sessionmenu->addSeparator();

	QMenu *modmenu = sessionmenu->addMenu(tr("Moderation"));
	modmenu->addAction(gainop);
	modmenu->addAction(terminatesession);
	modmenu->addAction(reportabuse);

	sessionmenu->addAction(resetsession);
	sessionmenu->addSeparator();
	sessionmenu->addAction(serverlog);
	sessionmenu->addAction(sessionSettings);
	sessionmenu->addAction(sessionUndoDepthLimit);
	sessionmenu->addAction(locksession);

	m_chatbox->setActions(invite, sessionSettings);

	//
	// Tools menu and toolbar
	//
	m_freehandAction = makeAction("toolbrush", tr("Freehand")).icon("draw-brush").statusTip(tr("Freehand brush tool")).shortcut("B").checkable();
	QAction *erasertool = makeAction("tooleraser", tr("Eraser")).icon("draw-eraser").statusTip(tr("Freehand eraser brush")).shortcut("E").checkable();
	QAction *linetool = makeAction("toolline", tr("&Line")).icon("draw-line").statusTip(tr("Draw straight lines")).shortcut("U").checkable();
	QAction *recttool = makeAction("toolrect", tr("&Rectangle")).icon("draw-rectangle").statusTip(tr("Draw unfilled squares and rectangles")).shortcut("R").checkable();
	QAction *ellipsetool = makeAction("toolellipse", tr("&Ellipse")).icon("draw-ellipse").statusTip(tr("Draw unfilled circles and ellipses")).shortcut("O").checkable();
	QAction *beziertool = makeAction("toolbezier", tr("Bezier Curve")).icon("draw-bezier-curves").statusTip(tr("Draw bezier curves")).shortcut("Ctrl+B").checkable();
	QAction *filltool = makeAction("toolfill", tr("&Flood Fill")).icon("fill-color").statusTip(tr("Fill areas")).shortcut("F").checkable();
	QAction *lassofilltool = makeAction("toollassofill", tr("Lass&o Fill")).icon("drawpile_lassofill").statusTip(tr("Fill enclosed areas")).shortcut("Shift+F").checkable();
	QAction *gradienttool = makeAction("toolgradient", tr("&Gradient")).icon("drawpile_gradient").statusTip(tr("Create a gradient inside selected areas")).shortcut("G").checkable();
	QAction *annotationtool = makeAction("tooltext", tr("&Annotation")).icon("draw-text").statusTip(tr("Add text to the picture")).shortcut("A").checked();

	QAction *pickertool = makeAction("toolpicker", tr("&Color Picker")).icon("color-picker").statusTip(tr("Pick colors from the image")).shortcut("I").checkable();
	QAction *lasertool = makeAction("toollaser", tr("&Laser Pointer")).icon("cursor-arrow").statusTip(tr("Point out things on the canvas")).shortcut("L").checkable();
	QAction *selectiontool = makeAction("toolselectrect", tr("&Select")).icon("select-rectangular").statusTip(tr("Select rectangular area")).shortcut("S").checkable();
	QAction *lassotool = makeAction("toolselectpolygon", tr("&Lasso Select")).icon("edit-select-lasso").statusTip(tr("Select a free-form area")).shortcut("D").checkable();
	QAction *magicwandtool = makeAction("toolselectmagicwand", tr("&Magic Wand Select")).icon("drawpile_magicwand").statusTip(tr("Select areas with similar colors")).shortcut("W").checkable();
	QAction *transformtool = makeAction("tooltransform", tr("&Transform Tool")).icon("drawpile_transform").statusTip(tr("Transform selection")).noDefaultShortcut().checkable();
	QAction *pantool = makeAction("toolpan", tr("Pan")).icon("hand").statusTip(tr("Pan canvas view")).shortcut("P").checkable();
	QAction *zoomtool = makeAction("toolzoom", tr("Zoom")).icon("edit-find").statusTip(tr("Zoom the canvas view")).shortcut("Z").checkable();
	QAction *rotationtool = makeAction("toolrotation", tr("Rotation")).icon("drawpile_rotate").statusTip(tr("Rotate the canvas view")).shortcut("Shift+R").checkable();
	QAction *inspectortool = makeAction("toolinspector", tr("Inspector")).icon("help-whatsthis").statusTip(tr("Find out who did it")).shortcut("Ctrl+I").checkable();

	// clang-format on
	m_drawingtools->addAction(m_freehandAction);
	m_drawingtools->addAction(erasertool);
	m_drawingtools->addAction(linetool);
	m_drawingtools->addAction(recttool);
	m_drawingtools->addAction(ellipsetool);
	m_drawingtools->addAction(beziertool);
	m_drawingtools->addAction(filltool);
	m_drawingtools->addAction(lassofilltool);
	m_drawingtools->addAction(gradienttool);
	m_drawingtools->addAction(annotationtool);
	m_drawingtools->addAction(pickertool);
	m_drawingtools->addAction(lasertool);
	m_drawingtools->addAction(selectiontool);
	m_drawingtools->addAction(lassotool);
	m_drawingtools->addAction(magicwandtool);
	m_drawingtools->addAction(transformtool);
	m_drawingtools->addAction(pantool);
	m_drawingtools->addAction(zoomtool);
	m_drawingtools->addAction(rotationtool);
	m_drawingtools->addAction(inspectortool);

	m_deselecttools = new QActionGroup(this);
	for(QAction *toolAction : m_drawingtools->actions()) {
		QString name =
			QStringLiteral("%1deselect").arg(toolAction->objectName());
		//: This is the text for keyboard shortcuts that switch tools and
		//: remove the selection in a single action. %1 is the name of a tool,
		//: like "Freehand", "Eraser" or "Line".
		QString text = tr("%1 and Deselect").arg(toolAction->text());
		QAction *deselectAction =
			makeAction(qUtf8Printable(name), text)
				.icon(toolAction->icon())
				.statusTip(tr("Switch tool to %1 and deselect at once")
							   .arg(toolAction->text()))
				.noDefaultShortcut();
		connect(
			deselectAction, &QAction::triggered, toolAction, &QAction::trigger);
		connect(
			deselectAction, &QAction::triggered, selectnone, &QAction::trigger);
		m_deselecttools->addAction(deselectAction);
	}

	QMenu *toolsmenu = menuBar()->addMenu(tr("Tools"));
	toolsmenu->addActions(m_drawingtools->actions());
	toolsmenu->addAction(toolbarconfig);
	toolsmenu->addSeparator();

	QMenu *toolshortcuts = toolsmenu->addMenu(tr("&Shortcuts"));
	QMenu *deselectshortcuts = toolsmenu->addMenu(tr("Deselect Shortcuts"));
	deselectshortcuts->addActions(m_deselecttools->actions());

	QMenu *devtoolsmenu = toolsmenu->addMenu(tr("Developer Tools"));
	// clang-format off
	QAction *systeminfo = makeAction("systeminfo", tr("System Information…")).noDefaultShortcut();
	QAction *tableteventlog = makeAction("tableteventlog", tr("Tablet Event Log...")).noDefaultShortcut();
	QAction *profile = makeAction("profile", tr("Profile...")).noDefaultShortcut();
#ifndef __EMSCRIPTEN__
	QAction *debugDump = makeAction("debugdump", tr("Record Debug Dumps")).checkable().noDefaultShortcut();
#endif
	QAction *openDebugDump = makeAction("opendebugdump", tr("Open Debug Dump...")).noDefaultShortcut();
#ifdef DRAWPILE_PROJECT_INFO_DIALOG
	QAction *projectInfo = makeAction("projectinfo", tr("Project Information…"));
#endif
	// clang-format on
#ifdef Q_OS_ANDROID
	QAction *androidTextDebug =
		makeAction("androidtextdebug", tr("Text Input Debug Overlay"))
			.checkable()
			.noDefaultShortcut();
#endif
	// clang-format off
	QAction *showNetStats = makeAction("shownetstats", tr("Statistics…")).noDefaultShortcut();
	devtoolsmenu->addAction(systeminfo);
	devtoolsmenu->addAction(tableteventlog);
	devtoolsmenu->addAction(profile);
#ifndef __EMSCRIPTEN__
	devtoolsmenu->addAction(debugDump);
#endif
	devtoolsmenu->addAction(openDebugDump);
#ifdef DRAWPILE_PROJECT_INFO_DIALOG
	devtoolsmenu->addAction(projectInfo);
#endif
	// clang-format on
#ifdef Q_OS_ANDROID
	devtoolsmenu->addAction(androidTextDebug);
#endif
	devtoolsmenu->addAction(showNetStats);
	// clang-format off
	connect(devtoolsmenu, &QMenu::aboutToShow, this, &MainWindow::updateDevToolsActions);
	connect(systeminfo, &QAction::triggered, this, &MainWindow::showSystemInfo);
	connect(tableteventlog, &QAction::triggered, this, &MainWindow::toggleTabletEventLog);
	connect(profile, &QAction::triggered, this, &MainWindow::toggleProfile);
#ifndef __EMSCRIPTEN__
	connect(debugDump, &QAction::triggered, this, &MainWindow::toggleDebugDump);
#endif
	connect(openDebugDump, &QAction::triggered, this, &MainWindow::openDebugDump);
#ifdef DRAWPILE_PROJECT_INFO_DIALOG
	connect(projectInfo, &QAction::triggered, this, &MainWindow::openProjectInfo);
#endif
	// clang-format on
#ifdef Q_OS_ANDROID
	connect(androidTextDebug, &QAction::triggered, this, [](bool checked) {
		if(checked) {
			qputenv(
				"KRITA_ANDROID_EDIT_TEXT_DEBUG_DRAW", QByteArrayLiteral("1"));
		} else {
			qunsetenv("KRITA_ANDROID_EDIT_TEXT_DEBUG_DRAW");
		}
	});
#endif
	// clang-format off
	connect(showNetStats, &QAction::triggered, m_netstatus, &widgets::NetStatus::showNetStats);

	// clang-format on
	if(DrawpileApp::isEnvTrue("DRAWPILE_DEV_MODE")) {
		QAction *artificialLag =
			makeAction("artificiallag", tr("Set Artificial Lag..."))
				.noDefaultShortcut();
		QAction *artificialDisconnect =
			makeAction("artificialdisconnect", tr("Artifical Disconnect..."))
				.noDefaultShortcut();
		QAction *causeCrash =
			makeAction("causecrash", tr("Cause Crash…")).noDefaultShortcut();
		QAction *retainProjectRecordings =
			makeAction(
				"retainprojectrecordings",
				QStringLiteral("Retain project recordings"))
				.checkable()
				.noDefaultShortcut();
		devtoolsmenu->addSeparator();
		devtoolsmenu->addAction(artificialLag);
		devtoolsmenu->addAction(artificialDisconnect);
		devtoolsmenu->addAction(causeCrash);
		devtoolsmenu->addAction(retainProjectRecordings);
		connect(
			artificialLag, &QAction::triggered, this,
			&MainWindow::setArtificialLag);
		connect(
			artificialDisconnect, &QAction::triggered, this,
			&MainWindow::setArtificialDisconnect);
		connect(causeCrash, &QAction::triggered, this, &MainWindow::causeCrash);
	}
	// clang-format off

	QAction *currentEraseMode = makeAction("currenterasemode", tr("Toggle Eraser Mode")).shortcut("Ctrl+E");
	QAction *currentRecolorMode =
		makeAction("currentrecolormode", tr("Toggle Alpha Preserve"))
			.shortcutWithSearchText(
				tr("toggle recolor mode/alpha preserve"),
				QKeySequence("Shift+E"));
	QAction *changeForegroundColor = makeAction("chnageforegroundcolor", widgets::DualColorButton::foregroundText()).statusTip(tr("Choose the current foreground color")).noDefaultShortcut();
	QAction *changeBackgroundColor = makeAction("changebackgroundcolor", widgets::DualColorButton::backgroundText()).statusTip(tr("Choose the current background color")).noDefaultShortcut();
	QAction *swapcolors = makeAction("swapcolors", widgets::DualColorButton::swapText()).statusTip(tr("Swap current foreground and background color with each other")).shortcut("X");
	QAction *resetcolors = makeAction("resetcolors", widgets::DualColorButton::resetText()).statusTip(tr("Set foreground color to black and background color to white")).noDefaultShortcut();
	// clang-format on
	QAction *smallerbrush =
		makeAction("ensmallenbrush", tr("&Decrease Brush Size"))
			.shortcut(Qt::Key_BracketLeft)
			.autoRepeat();
	QAction *biggerbrush =
		makeAction("embiggenbrush", tr("&Increase Brush Size"))
			.shortcut(Qt::Key_BracketRight)
			.autoRepeat();
	QAction *stepdown2 = makeAction("stepdown2", tr("Decrease Brush Opacity"))
							 .noDefaultShortcut()
							 .autoRepeat();
	QAction *stepup2 = makeAction("stepup2", tr("Increase Brush Opacity"))
						   .noDefaultShortcut()
						   .autoRepeat();
	QAction *stepdown3 = makeAction("stepdown3", tr("Decrease Brush Hardness"))
							 .noDefaultShortcut()
							 .autoRepeat();
	QAction *stepup3 = makeAction("stepup3", tr("Increase Brush Hardness"))
						   .noDefaultShortcut()
						   .autoRepeat();
	QAction *reloadPreset = makeAction("reloadpreset", tr("&Reset Brush"))
								.icon("view-refresh")
								.shortcut("Shift+P");
	QAction *reloadPresetSlots =
		makeAction("reloadpresetslots", tr("Reset All Brush &Slots"))
			.noDefaultShortcut();
	QAction *reloadAllPresets =
		makeAction("reloadallpresets", tr("Reset All &Brushes"))
			.shortcut("Shift+Alt+P");
	QAction *nextPreset =
		makeAction("nextpreset", tr("&Next Brush")).shortcut(".").autoRepeat();
	QAction *previousPreset = makeAction("prevpreset", tr("&Previous Brush"))
								  .shortcut(",")
								  .autoRepeat();
	QAction *nextTag = makeAction("nexttag", tr("Next Brush Tag"))
						   .noDefaultShortcut()
						   .autoRepeat();
	QAction *previousTag = makeAction("prevtag", tr("Previous Brush Tag"))
							   .noDefaultShortcut()
							   .autoRepeat();
	QAction *nextSlot = makeAction("nextslot", tr("Next Brush Slot"))
							.noDefaultShortcut()
							.autoRepeat();
	QAction *previousSlot = makeAction("prevslot", tr("Previous Brush Slot"))
								.noDefaultShortcut()
								.autoRepeat();
	connect(
		currentEraseMode, &QAction::triggered, m_dockToolSettings,
		&docks::ToolSettings::toggleEraserMode);
	connect(
		currentRecolorMode, &QAction::triggered, m_dockToolSettings,
		&docks::ToolSettings::toggleAlphaPreserve);
	connect(
		changeForegroundColor, &QAction::triggered, m_dockToolSettings,
		&docks::ToolSettings::changeForegroundColor);
	connect(
		changeBackgroundColor, &QAction::triggered, m_dockToolSettings,
		&docks::ToolSettings::changeBackgroundColor);
	connect(
		swapcolors, &QAction::triggered, m_dockToolSettings,
		&docks::ToolSettings::swapColors);
	connect(
		resetcolors, &QAction::triggered, m_dockToolSettings,
		&docks::ToolSettings::resetColors);

	connect(smallerbrush, &QAction::triggered, this, [this]() {
		m_dockToolSettings->stepAdjustCurrent1(false);
	});
	connect(biggerbrush, &QAction::triggered, this, [this]() {
		m_dockToolSettings->stepAdjustCurrent1(true);
	});
	connect(stepdown2, &QAction::triggered, this, [this]() {
		m_dockToolSettings->stepAdjustCurrent2(false);
	});
	connect(stepup2, &QAction::triggered, this, [this]() {
		m_dockToolSettings->stepAdjustCurrent2(true);
	});
	connect(stepdown3, &QAction::triggered, this, [this]() {
		m_dockToolSettings->stepAdjustCurrent3(false);
	});
	connect(stepup3, &QAction::triggered, this, [this]() {
		m_dockToolSettings->stepAdjustCurrent3(true);
	});
	connect(
		reloadPreset, &QAction::triggered, m_dockToolSettings->brushSettings(),
		&tools::BrushSettings::resetPreset);
	connect(
		reloadPresetSlots, &QAction::triggered,
		m_dockToolSettings->brushSettings(),
		&tools::BrushSettings::resetPresetsInAllSlots);
	connect(
		reloadAllPresets, &QAction::triggered, m_dockBrushPalette,
		&docks::BrushPalette::resetAllPresets);

	toolshortcuts->addAction(currentEraseMode);
	toolshortcuts->addAction(currentRecolorMode);
	toolshortcuts->addAction(changeForegroundColor);
	toolshortcuts->addAction(changeBackgroundColor);
	toolshortcuts->addAction(swapcolors);
	toolshortcuts->addAction(resetcolors);
	toolshortcuts->addAction(smallerbrush);
	toolshortcuts->addAction(biggerbrush);
	toolshortcuts->addAction(stepdown2);
	toolshortcuts->addAction(stepup2);
	toolshortcuts->addAction(stepdown3);
	toolshortcuts->addAction(stepup3);
	toolshortcuts->addAction(reloadPreset);
	toolshortcuts->addAction(reloadPresetSlots);
	toolshortcuts->addAction(reloadAllPresets);
	toolshortcuts->addAction(nextPreset);
	toolshortcuts->addAction(previousPreset);
	toolshortcuts->addAction(nextTag);
	toolshortcuts->addAction(previousTag);
	toolshortcuts->addAction(nextSlot);
	toolshortcuts->addAction(previousSlot);

	m_toolBarDraw = new QToolBar(tr("Drawing tools"));
	m_toolBarDraw->setObjectName("drawtoolsbar");
	toggletoolbarmenu->addAction(m_toolBarDraw->toggleViewAction());
	toggletoolbarmenu->addSeparator();
	toggletoolbarmenu->addAction(toolbarconfig);

	for(const canvas::blendmode::Named &named :
		canvas::blendmode::shortcutModeNames()) {
		QString name = QStringLiteral("toggletoolblend%1").arg(int(named.mode));
		QString text = tr("Tool blend mode: %1").arg(named.name);
		QAction *action =
			makeAction(qUtf8Printable(name), text).noDefaultShortcut();
		connect(
			action, &QAction::triggered, m_dockToolSettings,
			std::bind(
				&docks::ToolSettings::toggleBlendMode, m_dockToolSettings,
				int(named.mode)));
	}

	m_dockToolSettings->brushSettings()->setActions(
		reloadPreset, reloadPresetSlots, reloadAllPresets, nextSlot,
		previousSlot, layerAutomaticAlphaPreserve, maskselection,
		layerSetFillSource);
	m_dockToolSettings->lassoFillSettings()->setActions(
		layerAutomaticAlphaPreserve, maskselection);
	m_dockToolSettings->gradientSettings()->setActions(
		layerAutomaticAlphaPreserve);
	m_dockToolSettings->rotationSettings()->setActions(
		rotateccw, rotateorig, rotatecw);
	m_dockBrushPalette->setActions(
		nextPreset, previousPreset, nextTag, previousTag);
	// clang-format off

	resetDefaultToolbars();

	//
	// Window menu (Mac only)
	//
#ifdef Q_OS_MACOS
	menuBar()->addMenu(MacMenu::instance()->windowMenu());
#endif

	//
	// Help menu
	//
	// clang-format on
	QAction *homepage = makeAction("dphomepage", tr("&Homepage"))
							.icon("globe")
							.statusTip(cmake_config::website())
							.noDefaultShortcut();
	QAction *donate =
		makeAction(
			"dpdonate", QCoreApplication::translate("donations", "Donate"))
			.icon("love")
			.statusTip(
				QCoreApplication::translate(
					"donations", "Open Drawpile's donate page in your browser"))
			.noDefaultShortcut();
	// clang-format off
	QAction *tablettester = makeAction("tablettester", tr("Tablet Tester")).icon("input-tablet").noDefaultShortcut();
	QAction *touchtester = makeAction("touchtester", tr("Touch Tester")).icon("input-touchscreen").noDefaultShortcut();
	QAction *showlogfile = makeAction("showlogfile", tr("Log File")).noDefaultShortcut();
	// clang-format on
	QAction *about =
		makeAction("dpabout", tr("&About Drawpile")).noDefaultShortcut();
	QAction *aboutqt =
		makeAction("aboutqt", tr("About &Qt")).noDefaultShortcut();
#ifdef Q_OS_MACOS
	QAction *macabout = makeAction("macdpabout", tr("&About Drawpile"))
							.menuRole(QAction::AboutRole);
	QAction *macaboutqt = makeAction("macaboutqt", tr("About &Qt"))
							  .menuRole(QAction::AboutQtRole);
	// clang-format off
#endif
#ifndef __EMSCRIPTEN__
	QAction *versioncheck = makeAction("versioncheck", tr("Check For Updates")).noDefaultShortcut();
#endif


	connect(homepage, &QAction::triggered, &MainWindow::homepage);
	connect(donate, &QAction::triggered, &MainWindow::donate);
	connect(about, &QAction::triggered, &MainWindow::about);
	connect(aboutqt, &QAction::triggered, &QApplication::aboutQt);
#ifdef Q_OS_MACOS
	connect(macabout, &QAction::triggered, &MainWindow::about);
	connect(macaboutqt, &QAction::triggered, &QApplication::aboutQt);
#endif

#ifndef __EMSCRIPTEN__
	connect(
		versioncheck, &QAction::triggered, this, &MainWindow::checkForUpdates);
#endif

	// clang-format on
	connect(
		tablettester, &QAction::triggered, this,
		std::bind(&MainWindow::showTabletTestDialog, this, this));
	connect(
		touchtester, &QAction::triggered, this,
		std::bind(&MainWindow::showTouchTestDialog, this, this));
	// clang-format off

	connect(showlogfile, &QAction::triggered, [this] {
		QString logFilePath = utils::logFilePath();
		QFile logFile{logFilePath};
		if(!logFile.exists()) {
			utils::showWarning(
				this, tr("Missing Log File"),
				tr("Log file doesn't exist, do you need to enable logging in the preferences?"));
			return;
		}

#if defined(Q_OS_ANDROID) || defined(__EMSCRIPTEN__)
		if(logFile.open(QIODevice::ReadOnly)) {
			QString defaultName =
				QStringLiteral("drawpile-log-%1-%2.txt")
					.arg(cmake_config::version())
					.arg(QDateTime::currentDateTime().toString("yyyyMMddHHMMSS"));
			QString error;
			if(!FileWrangler(this).saveLogFile(
					defaultName, logFile.readAll(), &error)) {
				utils::showWarning(
					this, tr("Error Saving Log File"),
					tr("Could not write log file: %1").arg(error));
			}
		} else {
			utils::showWarning(
				this, tr("Error Saving Log File"),
				tr("Could not read log file: %1").arg(logFile.errorString()));
		}
#else
		QDesktopServices::openUrl(QUrl::fromLocalFile(utils::logFilePath()));
#endif
	});

	QMenu *helpmenu = menuBar()->addMenu(tr("Help"));
	helpmenu->addAction(homepage);
	helpmenu->addAction(donate);
	helpmenu->addAction(tablettester);
	helpmenu->addAction(touchtester);
	helpmenu->addAction(showlogfile);
	helpmenu->addSeparator();
	helpmenu->addAction(about);
	helpmenu->addAction(aboutqt);
#ifdef Q_OS_MACOS
	helpmenu->addAction(macabout);
	helpmenu->addAction(macaboutqt);
#endif
	helpmenu->addSeparator();
#ifndef __EMSCRIPTEN__
	helpmenu->addAction(versioncheck);
#endif

	// clang-format on

	QAction *menuFileAction =
		makeAction("menu-file", tr("File menu")).shortcut("Alt+F");
	QAction *menuEditAction =
		makeAction("menu-edit", tr("Edit menu")).shortcut("Alt+E");
	QAction *menuViewAction =
		makeAction("menu-view", tr("View menu")).shortcut("Alt+V");
	QAction *menuLayerAction =
		makeAction("menu-layer", tr("Layer menu")).shortcut("Alt+L");
	QAction *menuSelectionAction =
		makeAction("menu-selection", tr("Selection menu")).shortcut("Alt+N");
	QAction *menuAiAction =
		makeAction("menu-ai", tr("AI menu")).shortcut("Alt+I");
	QAction *menuAnimationAction =
		makeAction("menu-animation", tr("Animation menu")).shortcut("Alt+A");
	QAction *menuSessionAction =
		makeAction("menu-session", tr("Session menu")).shortcut("Alt+S");
	QAction *menuToolsAction =
		makeAction("menu-tools", tr("Tools menu")).shortcut("Alt+T");
	QAction *menuHelpAction =
		makeAction("menu-help", tr("Help menu")).shortcut("Alt+H");

	QPair<QAction *, QMenu *> menuPairs[] = {
		{menuFileAction, filemenu},		   {menuEditAction, editmenu},
		{menuViewAction, viewmenu},		   {menuLayerAction, layerMenu},
		{menuSelectionAction, selectMenu}, {menuAiAction, aiMenu},
		{menuAnimationAction, animationMenu},
		{menuSessionAction, sessionmenu},  {menuToolsAction, toolsmenu},
		{menuHelpAction, helpmenu},
	};

	for(const QPair<QAction *, QMenu *> &p : menuPairs) {
		QAction *action = p.first;
		QMenu *menu = p.second;
		// Hooks to disable menu actions the user doesn't have permission for
		// when the menus are shown and then reenable them afterwards so that
		// shortcuts still attempt to activate them and trigger a permission
		// denied message.
		connect(menu, &QMenu::aboutToShow, this, &MainWindow::aboutToShowMenu);
		connect(menu, &QMenu::aboutToHide, this, &MainWindow::aboutToHideMenu);
		// Menu action shortcuts, effectively emulating Alt+Mnemonic behavior,
		// but letting the user configure shortcuts for them.
		connect(
			action, &QAction::triggered, this,
			[this, menuAction = menu->menuAction()] {
				menuBar()->setActiveAction(menuAction);
			});
	}

	// Brush slot shortcuts
	m_brushSlots = new QActionGroup(this);
	for(int i = 0; i < 10; ++i) {
		QAction *q = new QAction(tr("Brush slot #%1").arg(i + 1), this);
		q->setAutoRepeat(false);
		q->setObjectName(QStringLiteral("quicktoolslot-%1").arg(i));
		q->setShortcut(QKeySequence(QString::number((i + 1) % 10)));
		q->setProperty("toolslotidx", i);
		CustomShortcutModel::registerCustomizableAction(
			q->objectName(), q->text(), q->icon(), q->shortcut(),
			QKeySequence());
		m_brushSlots->addAction(q);
		addAction(q);
		// Swapping with the eraser slot doesn't make sense.
		if(i != 9) {
			QAction *s =
				new QAction(tr("Swap With Brush Slot #%1").arg(i + 1), this);
			s->setAutoRepeat(false);
			s->setObjectName(QStringLiteral("swapslot%1").arg(i));
			CustomShortcutModel::registerCustomizableAction(
				s->objectName(), s->text(), s->icon(), QKeySequence(),
				QKeySequence());
			addAction(s);
			connect(s, &QAction::triggered, this, [this, i] {
				m_dockToolSettings->brushSettings()->swapWithSlot(i);
			});
		}
	}
	connect(m_brushSlots, &QActionGroup::triggered, this, [this](QAction *a) {
		m_dockToolSettings->setToolSlot(a->property("toolslotidx").toInt());
		m_toolChangeTime.start();
	});
	// clang-format off

	// Color swatch shortcuts
	for(int i = 0; i < docks::ToolSettings::LASTUSED_COLOR_COUNT; ++i) {
		QAction *swatchAction = makeAction(
			qUtf8Printable(QStringLiteral("swatchcolor%1").arg(i)),
			tr("Swatch Color %1").arg(i + 1)).noDefaultShortcut();
		connect(swatchAction, &QAction::triggered, m_dockToolSettings, [=] {
			m_dockToolSettings->setLastUsedColor(i);
		});
	}

	// Add temporary tool change shortcut detector
	for(QAction *act : m_drawingtools->actions())
		act->installEventFilter(m_tempToolSwitchShortcut);

	for(QAction *act : m_brushSlots->actions())
		act->installEventFilter(m_tempToolSwitchShortcut);

	// Other shortcuts
	QAction *finishStrokeShortcut =
		makeAction("finishstroke", tr("Finish action"))
			.shortcut(Qt::Key_Return, Qt::Key_Enter);
	connect(finishStrokeShortcut, &QAction::triggered,
			m_doc->toolCtrl(), &tools::ToolController::finishMultipartDrawing);

	QAction *escapeShortcut = makeAction("cancelaction", tr("Cancel action")).shortcut(Qt::Key_Escape);
	connect(escapeShortcut, &QAction::triggered,
			m_doc->toolCtrl(), &tools::ToolController::cancelMultipartDrawing);

	QAction *focusCanvas = makeAction("focuscanvas", tr("Focus canvas")).shortcut(CTRL_KEY | Qt::Key_Tab);
	// clang-format on
	connect(focusCanvas, &QAction::triggered, this, [this] {
		m_canvasView->viewWidget()->setFocus();
	});

	// Lock status actions
	QMenu *layerViewNoticeMenu = new QMenu(this);
	layerViewNoticeMenu->addAction(m_layerViewNormal);
	layerViewNoticeMenu->addAction(m_layerViewCurrentLayer);
	layerViewNoticeMenu->addAction(m_layerViewCurrentGroup);
	layerViewNoticeMenu->addAction(m_layerViewCurrentFrame);
	layerViewNoticeMenu->addSeparator();
	QAction *disableViewModeNotices = layerViewNoticeMenu->addAction(
		QIcon::fromTheme("drawpile_close"), tr("Disable view mode notices"));
	connect(disableViewModeNotices, &QAction::triggered, this, [this, cfg] {
		cfg->setShowViewModeNotices(false);
		m_canvasView->showPopupNotice(
			tr("Layer view mode notices disabled.\n"
			   "You can re-enable them via the View menu or preferences."));
	});
	m_viewLock->exitLayerViewModeAction()->setMenu(layerViewNoticeMenu);
	m_viewLock->exitGroupViewModeAction()->setMenu(layerViewNoticeMenu);
	m_viewLock->exitFrameViewModeAction()->setMenu(layerViewNoticeMenu);
	connect(
		m_viewLock->exitLayerViewModeAction(), &QAction::triggered, this,
		&MainWindow::setNormalLayerViewMode);
	connect(
		m_viewLock->exitGroupViewModeAction(), &QAction::triggered, this,
		&MainWindow::setNormalLayerViewMode);
	connect(
		m_viewLock->exitFrameViewModeAction(), &QAction::triggered, this,
		&MainWindow::setNormalLayerViewMode);
	connect(
		m_viewLock->unlockCanvasAction(), &QAction::triggered, this,
		[locksession] {
			if(locksession->isChecked()) {
				locksession->trigger();
			}
		});
	connect(
		m_viewLock->resetCanvasAction(), &QAction::triggered, resetsession,
		&QAction::trigger);
	connect(
		m_viewLock->selectAllAction(), &QAction::triggered, selectall,
		&QAction::trigger);
	connect(
		m_viewLock->selectLayerBoundsAction(), &QAction::triggered,
		selectlayerbounds, &QAction::trigger);
	connect(
		m_viewLock->disableAntiOverflowAction(), &QAction::triggered,
		m_dockToolSettings->brushSettings(),
		&tools::BrushSettings::disableAntiOverflow);
	connect(
		m_viewLock->setFillSourceAction(), &QAction::triggered,
		layerSetFillSource, &QAction::trigger);
	connect(
		m_viewLock->clearFillSourceAction(), &QAction::triggered,
		layerClearFillSource, &QAction::trigger);
	connect(
		m_viewLock->uncensorLayersAction(), &QAction::triggered, this,
		[layerUncensor] {
			if(!layerUncensor->isChecked() &&
			   !parentalcontrols::isLayerUncensoringBlocked()) {
				layerUncensor->trigger();
			}
		});

	const QList<QAction *> globalDockActions = {
		sideTabDocks, hideDocks, arrangeDocks, nullptr, layoutsAction};
	for(docks::DockBase *dw : findChildren<docks::DockBase *>(
			QString(), Qt::FindDirectChildrenOnly)) {
		if(docks::TitleWidget *titlebar =
			   qobject_cast<docks::TitleWidget *>(dw->actualTitleBarWidget())) {
			titlebar->addGlobalDockActions(globalDockActions);
		}
	}

	for(QObject *child : findChildren<QObject *>()) {
		if(qobject_cast<QAction *>(child)) {
			child->installEventFilter(this);
		}
	}

	if(m_singleSession) {
		QActionGroup *singleGroup = new QActionGroup(this);
		singleGroup->setExclusive(false);
		singleGroup->setEnabled(false);
		singleGroup->setVisible(false);
		singleGroup->addAction(newdocument);
		singleGroup->addAction(open);
		singleGroup->addAction(start);
#ifndef __EMSCRIPTEN__
		singleGroup->addAction(importAnimationFrames);
#endif
		singleGroup->addAction(importAnimationLayers);
		singleGroup->addAction(host);
		singleGroup->addAction(browse);
	}

	m_statusAutoRecordButton->setStatusTip(autoRecordSettings->text());
	updateSmallScreenToolBarVisibility();
	updateInterfaceModeActions();
}

void MainWindow::setupBrushShortcuts()
{
	brushes::BrushPresetModel *brushPresetModel =
		dpApp().brushPresets()->presetModel();
	brushPresetModel->getShortcutActions(
		std::bind(&MainWindow::addBrushShortcut, this, _1, _2, _3));
	connect(
		brushPresetModel, &brushes::BrushPresetModel::shortcutActionAdded, this,
		&MainWindow::addBrushShortcut);
	connect(
		brushPresetModel, &brushes::BrushPresetModel::shortcutActionChanged,
		this, &MainWindow::changeBrushShortcut);
	connect(
		brushPresetModel, &brushes::BrushPresetModel::shortcutActionRemoved,
		this, &MainWindow::removeBrushShortcut);
}

void MainWindow::setupHud()
{
	using drawingboard::ActionBarItem;
	HudHandler *hud = m_canvasView->hud();
	CFG_BIND_SET(
		dpAppConfig(), ActionBarLocation, hud,
		HudHandler::setActionBarLocation);

	QAction *locationMenuAction = new QAction(tr("Bar Location"), this);
	locationMenuAction->setMenu(m_actionBarLocationMenu);

	QAction *disableAction = new QAction(
		QIcon::fromTheme(QStringLiteral("drawpile_close")),
		tr("Disable This Bar"), this);
	connect(
		disableAction, &QAction::triggered, this,
		&MainWindow::disableActionBar);

	ActionBarItem *selectionActionBar = hud->selectionActionBar();
	selectionActionBar->setButtons({
		ActionBarItem::Button(getAction(QStringLiteral("selectnone"))),
		ActionBarItem::Button(
			//: Refers to inverting the selection.
			getAction(QStringLiteral("selectinvert")), tr("Invert")),
		ActionBarItem::Button(
			//: Refers to expanding or shrinking the selection.
			getAction(QStringLiteral("selectalter")), tr("Expand/Shrink")),
		ActionBarItem::Button(getAction(QStringLiteral("starttransform"))),
#ifdef __EMSCRIPTEN__
		ActionBarItem::Button(
			getAction(QStringLiteral("downloadselection")),
			QIcon::fromTheme(QStringLiteral("document-export"))),
#else
		ActionBarItem::Button(
			//: Refers to saving the selected area as an image.
			getAction(QStringLiteral("saveselection")),
			QIcon::fromTheme(QStringLiteral("document-export"))),
#endif
	});
	selectionActionBar->setOverflowMenuActions({
		getAction(QStringLiteral("selectall")),
		getAction(QStringLiteral("selectlayerbounds")),
		getAction(QStringLiteral("selectlayercontents")),
		nullptr,
		getAction(QStringLiteral("cleararea")),
		getAction(QStringLiteral("fillfgarea")),
		getAction(QStringLiteral("recolorarea")),
		getAction(QStringLiteral("selectcrop")),
		nullptr,
		getAction(QStringLiteral("showselectionmask")),
		getAction(QStringLiteral("editselection")),
		getAction(QStringLiteral("maskselection")),
		nullptr,
		locationMenuAction,
		disableAction,
	});

	ActionBarItem *transformActionBar = hud->transformActionBar();
	transformActionBar->setButtons({
		ActionBarItem::Button(
			getAction("finishstroke"), tr("Apply"),
			QIcon::fromTheme(QStringLiteral("checkbox"))),
		ActionBarItem::Button(
			//: Refers to mirroring a transform horizontally.
			getAction(QStringLiteral("transformmirror")), tr("Mirror")),
		ActionBarItem::Button(
			//: Refers to mirroring a transform vertically (flip upside-down.)
			getAction(QStringLiteral("transformflip")), tr("Flip")),
		ActionBarItem::Button(
			//: Refers to rotating a transform 90 degrees counter-clockwise.
			getAction(QStringLiteral("transformrotateccw")), tr("Rotate -90°")),
		ActionBarItem::Button(
			//: Refers to rotating a transform 90 degrees clockwise.
			getAction(QStringLiteral("transformrotatecw")), tr("Rotate +90°")),
	});
	transformActionBar->setOverflowMenuActions({
		getAction(QStringLiteral("cancelaction")),
		getAction(QStringLiteral("selectnone")),
		nullptr,
		getAction(QStringLiteral("transformshrinktoview")),
		getAction(QStringLiteral("stamp")),
		nullptr,
		locationMenuAction,
		disableAction,
	});

	connect(
		hud, &HudHandler::hudActionActivated, this,
		&MainWindow::handleHudAction);
}

void MainWindow::setActionBarSetting(int actionBar)
{
	// The setting is an integer for the sake of ease of later extension.
	setActionBarEnabled(actionBar > 0, false);
}

void MainWindow::disableActionBar()
{
	setActionBarEnabled(false, true);
	m_canvasView->showPopupNotice(
		tr("Selection action bar disabled.\n"
		   "You can re-enable it via the View menu."));
}

void MainWindow::setActionBarEnabled(bool enabled, bool updateSetting)
{
	if(m_actionBarEnabled != enabled) {
		m_actionBarEnabled = enabled;
		updateSelectTransformActions();
		QAction *showactionbar = getAction(QStringLiteral("showactionbar"));
		QSignalBlocker blocker(showactionbar);
		showactionbar->setChecked(enabled);
		if(updateSetting) {
			dpAppConfig()->setActionBar(enabled ? 1 : 0);
		}
	}
}

void MainWindow::setActionBarLocation(int location)
{
	QList<QAction *> actions = m_actionBarLocationMenu->actions();
	actions[qBound(0, location, actions.size() - 1)]->setChecked(true);
}

void MainWindow::onActionBarLocationActionTriggered(QAction *action)
{
	int location = m_actionBarLocationMenu->actions().indexOf(action);
	if(location != -1) {
		dpAppConfig()->setActionBarLocation(location);
	}
}

void MainWindow::updateInterfaceModeActions()
{
	m_desktopModeActions->setEnabled(!m_smallScreenMode);
	m_desktopModeActions->setVisible(!m_smallScreenMode);
	m_smallScreenModeActions->setEnabled(m_smallScreenMode);
	m_smallScreenModeActions->setVisible(m_smallScreenMode);
	bool haveSmallScreenEditActions = !m_smallScreenEditActions.isEmpty();
	if(m_smallScreenMode && !haveSmallScreenEditActions) {
		m_smallScreenEditActions.append(m_toolBarEdit->addSeparator());
		QAction *viewflip = getAction("viewflip");
		m_toolBarEdit->addAction(viewflip);
		m_smallScreenEditActions.append(viewflip);
		QAction *viewmirror = getAction("viewmirror");
		m_toolBarEdit->addAction(viewmirror);
		m_smallScreenEditActions.append(viewmirror);
		m_smallScreenEditActions.append(m_toolBarEdit->addSeparator());
		QAction *zoomorig = getAction("zoomone");
		m_toolBarEdit->addAction(zoomorig);
		m_smallScreenEditActions.append(zoomorig);
		QAction *rotateorig = getAction("rotatezero");
		m_toolBarEdit->addAction(rotateorig);
		m_smallScreenEditActions.append(rotateorig);
	} else if(!m_smallScreenMode && haveSmallScreenEditActions) {
		for(QAction *action : m_smallScreenEditActions) {
			m_toolBarEdit->removeAction(action);
			if(action->isSeparator()) {
				delete action;
			}
		}
		m_smallScreenEditActions.clear();
	}

	QList<QToolBar *> toolbars =
		findChildren<QToolBar *>(QString(), Qt::FindDirectChildrenOnly);
	for(QToolBar *toolbar : toolbars) {
		toolbar->toggleViewAction()->setEnabled(!m_smallScreenMode);
	}
}
// clang-format off

void MainWindow::reenableUpdates()
{
	setUpdatesEnabled(true);
	// Qt will inherit the update enabled state when restoring floating widgets,
	// but won't when re-enabling them on the parent. Gotta do it ourselves.
	for(QWidget *widget : findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly)) {
		if(!widget->updatesEnabled()) {
			widget->setUpdatesEnabled(true);
		}
	}
}

// clang-format on
void MainWindow::keepCanvasPosition(const std::function<void()> &block)
{
	QWidget *canvasWidget = m_canvasView->viewWidget();
	QPoint centralPosBefore = centralWidget()->pos();
	QSize canvasSizeBefore = canvasWidget->size();
	QPointF offsetBefore = m_canvasView->viewTransformOffset();
	QPoint referencePosition =
		m_dockReference ? m_dockReference->scrollPosition() : QPoint();
	block();
	if(m_dockReference) {
		m_dockReference->setScrollPosition(referencePosition);
	}
	QPoint centralPosDelta = centralWidget()->pos() - centralPosBefore;
	QSize canvasSizeDelta = canvasWidget->size() - canvasSizeBefore;
	QPointF offsetDelta = m_canvasView->viewTransformOffset() - offsetBefore;
	emit viewShifted(
		centralPosDelta.x() + canvasSizeDelta.width() / 2.0 - offsetDelta.x(),
		centralPosDelta.y() + canvasSizeDelta.height() / 2.0 - offsetDelta.y());
}

void MainWindow::createDocks()
{
	Q_ASSERT(m_doc);
	Q_ASSERT(m_canvasView);

	setDockNestingEnabled(true);

	// Create tool settings
	m_dockToolSettings = new docks::ToolSettings(m_doc->toolCtrl(), this);
	m_dockToolSettings->setObjectName("ToolSettings");
	m_dockToolSettings->setAllowedAreas(Qt::AllDockWidgetAreas);

	// Create brush palette
	m_dockBrushPalette = new docks::BrushPalette(this);
	m_dockBrushPalette->setObjectName("BrushPalette");
	m_dockBrushPalette->setAllowedAreas(Qt::AllDockWidgetAreas);

	m_dockBrushPalette->connectBrushSettings(
		m_dockToolSettings->brushSettings());

	// Create color docks
	//: "Wheel" refers to the color wheel.
	m_dockColorSpinner = new docks::ColorSpinnerDock(this);
	m_dockColorSpinner->setObjectName("colorspinnerdock");
	m_dockColorSpinner->setAllowedAreas(Qt::AllDockWidgetAreas);

	m_dockColorPalette = new docks::ColorPaletteDock(this);
	m_dockColorPalette->setObjectName("colorpalettedock");
	m_dockColorPalette->setAllowedAreas(Qt::AllDockWidgetAreas);

	m_dockColorSliders = new docks::ColorSliderDock(this);
	m_dockColorSliders->setObjectName("colorsliderdock");
	m_dockColorSliders->setAllowedAreas(Qt::AllDockWidgetAreas);

	m_dockColorCircle = new docks::ColorCircleDock(this);
	m_dockColorCircle->setObjectName("colorcircledock");
	m_dockColorCircle->setAllowedAreas(Qt::AllDockWidgetAreas);

	// Create layer list
	m_dockLayers = new docks::LayerList(this);
	m_dockLayers->setObjectName("LayerList");
	m_dockLayers->setAllowedAreas(Qt::AllDockWidgetAreas);

	// Create navigator
	m_dockNavigator = new docks::Navigator(this);
	m_dockNavigator->setObjectName("navigatordock");
	m_dockNavigator->setAllowedAreas(Qt::AllDockWidgetAreas);

	// Create timeline
	m_dockTimeline = new docks::Timeline(this);
	m_dockTimeline->setObjectName("Timeline");
	m_dockTimeline->setAllowedAreas(Qt::AllDockWidgetAreas);

	// Create onion skin settings
	m_dockOnionSkins = new docks::OnionSkinsDock(this);
	m_dockOnionSkins->setObjectName("onionskins");
	m_dockOnionSkins->setAllowedAreas(Qt::AllDockWidgetAreas);

	m_dockReference = new docks::ReferenceDock(this);
	m_dockReference->setObjectName("referencedock");
	m_dockReference->setAllowedAreas(Qt::AllDockWidgetAreas);

	m_dockPromptManager = new QDockWidget(tr("Proompt Manager"), this);
	m_dockPromptManager->setObjectName(QStringLiteral("UnderpaintPromptManager"));
	m_dockPromptManager->setAllowedAreas(Qt::AllDockWidgetAreas);

	QWidget *promptManagerWidget = new QWidget(m_dockPromptManager);
	QVBoxLayout *promptManagerLayout = new QVBoxLayout(promptManagerWidget);
	promptManagerLayout->setContentsMargins(4, 4, 4, 4);
	promptManagerLayout->setSpacing(4);

	m_promptHistorySearch = new QLineEdit(promptManagerWidget);
	m_promptHistorySearch->setPlaceholderText(tr("Search prompts"));
	promptManagerLayout->addWidget(m_promptHistorySearch);

	m_promptHistoryList = new QListWidget(promptManagerWidget);
	m_promptHistoryList->setSelectionMode(QAbstractItemView::NoSelection);
	m_promptHistoryList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_promptHistoryList->setUniformItemSizes(false);
	promptManagerLayout->addWidget(m_promptHistoryList, 1);

	m_dockPromptManager->setWidget(promptManagerWidget);
	connect(
		m_promptHistorySearch, &QLineEdit::textChanged, this,
		[this] { refreshPromptManager(); });
	refreshPromptManager();
}

void MainWindow::resetDefaultDocks()
{
	bool leftyMode = m_smallScreenMode && m_leftyMode;
	Qt::DockWidgetArea leftArea =
		leftyMode ? Qt::RightDockWidgetArea : Qt::LeftDockWidgetArea;
	Qt::DockWidgetArea rightArea =
		leftyMode ? Qt::LeftDockWidgetArea : Qt::RightDockWidgetArea;
	addDockWidget(leftArea, m_dockToolSettings);
	m_dockToolSettings->show();
	addDockWidget(leftArea, m_dockBrushPalette);
	m_dockBrushPalette->show();
	addDockWidget(rightArea, m_dockColorSpinner);
	m_dockColorSpinner->show();
	addDockWidget(rightArea, m_dockColorPalette);
	m_dockColorPalette->show();
	addDockWidget(rightArea, m_dockColorSliders);
	m_dockColorSliders->show();
	addDockWidget(rightArea, m_dockColorCircle);
	m_dockColorCircle->show();
	addDockWidget(rightArea, m_dockReference);
	m_dockReference->show();
	tabifyDockWidget(m_dockReference, m_dockColorCircle);
	tabifyDockWidget(m_dockColorCircle, m_dockColorPalette);
	tabifyDockWidget(m_dockColorPalette, m_dockColorSliders);
	tabifyDockWidget(m_dockColorSliders, m_dockColorSpinner);
	addDockWidget(rightArea, m_dockPromptManager);
	m_dockPromptManager->show();
	addDockWidget(rightArea, m_dockLayers);
	m_dockLayers->show();
	addDockWidget(rightArea, m_dockNavigator);
	m_dockNavigator->hide(); // hidden by default
	addDockWidget(Qt::TopDockWidgetArea, m_dockTimeline);
	m_dockTimeline->show();
	addDockWidget(Qt::TopDockWidgetArea, m_dockOnionSkins);
	m_dockOnionSkins->show();
	if(m_smallScreenMode) {
		tabifyDockWidget(m_dockTimeline, m_dockOnionSkins);
	}
}

void MainWindow::clearPromptHistory()
{
	m_inpaintPromptHistory.clear();
	m_reusableInpaintPrompt.clear();
	if(m_promptHistorySearch) {
		m_promptHistorySearch->clear();
	}
	refreshPromptManager();
}

void MainWindow::rememberInpaintPrompt(const QString &prompt)
{
	const QString cleaned = prompt.simplified().trimmed();
	if(cleaned.isEmpty()) {
		return;
	}

	m_inpaintPromptHistory.removeAll(cleaned);
	m_inpaintPromptHistory.prepend(cleaned);
	while(m_inpaintPromptHistory.size() > 25) {
		m_inpaintPromptHistory.removeLast();
	}
	refreshPromptManager();
}

QString MainWindow::takeReusableInpaintPrompt()
{
	const QString prompt = m_reusableInpaintPrompt;
	m_reusableInpaintPrompt.clear();
	return prompt;
}

void MainWindow::setReusableInpaintPrompt(const QString &prompt)
{
	m_reusableInpaintPrompt = prompt;
	showPopupMessage(tr("Prompt queued for reuse."));
}

void MainWindow::deleteInpaintPromptHistoryEntry(const QString &prompt)
{
	m_inpaintPromptHistory.removeAll(prompt);
	if(m_reusableInpaintPrompt == prompt) {
		m_reusableInpaintPrompt.clear();
	}
	refreshPromptManager();
}

void MainWindow::refreshPromptManager()
{
	if(!m_promptHistoryList) {
		return;
	}

	m_promptHistoryList->clear();
	const QString filter =
		m_promptHistorySearch ? m_promptHistorySearch->text().trimmed() : QString();
	int visibleCount = 0;
	for(const QString &prompt : m_inpaintPromptHistory) {
		if(!filter.isEmpty() &&
		   !prompt.contains(filter, Qt::CaseInsensitive)) {
			continue;
		}

		++visibleCount;
		QListWidgetItem *item = new QListWidgetItem(m_promptHistoryList);
		QWidget *row = new QWidget(m_promptHistoryList);
		QHBoxLayout *rowLayout = new QHBoxLayout(row);
		rowLayout->setContentsMargins(4, 2, 4, 2);
		rowLayout->setSpacing(4);

		QString preview = prompt;
		if(preview.size() > 140) {
			preview = preview.left(137) + QStringLiteral("...");
		}
		QLabel *label = new QLabel(preview, row);
		label->setToolTip(prompt);
		label->setWordWrap(true);
		label->setTextInteractionFlags(Qt::TextSelectableByMouse);
		rowLayout->addWidget(label, 1);

		QToolButton *reuse = new QToolButton(row);
		reuse->setIcon(QIcon::fromTheme(QStringLiteral("view-refresh")));
		if(reuse->icon().isNull()) {
			reuse->setText(QStringLiteral("R"));
		}
		reuse->setToolTip(tr("Reuse this prompt"));
		reuse->setAutoRaise(true);
		rowLayout->addWidget(reuse);
		connect(
			reuse, &QToolButton::clicked, this,
			[this, prompt] { setReusableInpaintPrompt(prompt); });

		QToolButton *deleteButton = new QToolButton(row);
		deleteButton->setIcon(QIcon::fromTheme(QStringLiteral("window-close")));
		if(deleteButton->icon().isNull()) {
			deleteButton->setText(QStringLiteral("X"));
		}
		deleteButton->setToolTip(tr("Delete this prompt"));
		deleteButton->setAutoRaise(true);
		rowLayout->addWidget(deleteButton);
		connect(
			deleteButton, &QToolButton::clicked, this,
			[this, prompt] { deleteInpaintPromptHistoryEntry(prompt); });

		item->setSizeHint(row->sizeHint());
		m_promptHistoryList->setItemWidget(item, row);
	}

	if(visibleCount == 0) {
		QListWidgetItem *empty = new QListWidgetItem(
			filter.isEmpty() ? tr("No prompts yet.") : tr("No matching prompts."),
			m_promptHistoryList);
		empty->setFlags(Qt::NoItemFlags);
	}
}

void MainWindow::resetDefaultToolbars()
{
	if(m_smallScreenMode) {
		addToolBar(Qt::BottomToolBarArea, m_toolBarEdit);
		addToolBar(Qt::BottomToolBarArea, m_toolBarFile);
		addToolBar(
			m_leftyMode ? Qt::RightToolBarArea : Qt::LeftToolBarArea,
			m_toolBarDraw);
		if(!m_smallScreenLeftSpacer) {
			m_smallScreenLeftSpacer = new QWidget;
			m_smallScreenLeftSpacer->setFixedWidth(16);
			m_toolBarEdit->insertWidget(
				m_toolBarEdit->actions().constFirst(), m_smallScreenLeftSpacer);
		}
		if(!m_smallScreenRightSpacer) {
			m_smallScreenRightSpacer = new QWidget;
			m_smallScreenRightSpacer->setFixedWidth(16);
			m_toolBarEdit->addWidget(m_smallScreenRightSpacer);
		}
	} else {
		addToolBar(Qt::TopToolBarArea, m_toolBarFile);
		addToolBar(Qt::TopToolBarArea, m_toolBarEdit);
		addToolBar(Qt::TopToolBarArea, m_toolBarDraw);
		delete m_smallScreenLeftSpacer;
		m_smallScreenLeftSpacer = nullptr;
		delete m_smallScreenRightSpacer;
		m_smallScreenRightSpacer = nullptr;
	}
	m_toolBarFile->show();
	m_toolBarEdit->show();
	m_toolBarDraw->show();
}

void MainWindow::restoreDefaultStateWith(
	const QList<QDockWidget *> &dockWidgets)
{
	Q_ASSERT(m_restoringDockState);
	setFreezeDocks(false);
	for(QDockWidget *dw : dockWidgets) {
		dw->setFloating(false);
		dw->show();
		removeDockWidget(dw);
	}
	removeToolBar(m_toolBarFile);
	removeToolBar(m_toolBarEdit);
	removeToolBar(m_toolBarDraw);
	resetDefaultDocks();
	resetDefaultToolbars();
	initDefaultDocks();
	setFreezeDocks(getAction("freezedocks")->isChecked());
}

bool MainWindow::isInitialSmallScreenMode()
{
	config::Config *cfg = dpAppConfig();
	switch(cfg->getInterfaceMode()) {
	case int(view::InterfaceMode::Desktop):
		return false;
	case int(view::InterfaceMode::SmallScreen):
		return true;
	default:
		break;
	}

	bool useScreenSize;
#ifdef SINGLE_MAIN_WINDOW
	useScreenSize = true;
#else
	useScreenSize = cfg->getLastWindowMaximized();
#endif

	QSize s;
	if(useScreenSize) {
		QScreen *screen = QGuiApplication::primaryScreen();
		if(screen) {
			s = screen->availableSize();
		}
	} else {
		s = cfg->getLastWindowSize();
	}
	return utils::isSmallScreenModeSize(s);
}

void MainWindow::setLeftyMode(bool leftyMode)
{
	if(m_leftyMode != leftyMode) {
		m_leftyMode = leftyMode;
		if(m_smallScreenMode) {
			setUpdatesEnabled(false);
			resetDefaultDocks();
			resetDefaultToolbars();
			initDefaultDocks();
			for(QDockWidget *dw : findChildren<QDockWidget *>(
					QString(), Qt::FindDirectChildrenOnly)) {
				dw->hide();
			}
			updateSmallScreenToolBarVisibility();
			m_canvasView->setShowToggleItems(true, leftyMode);
			reenableUpdates();
		}
	}
}

void MainWindow::updateInterfaceMode()
{
	if(!m_updatingInterfaceMode &&
	   !findChild<dialogs::LayoutsDialog *>(
		   "layoutsdialog", Qt::FindDirectChildrenOnly)) {
		QScopedValueRollback<bool> rollback(m_updatingInterfaceMode, true);
		bool smallScreenMode = shouldUseSmallScreenMode();
		if(smallScreenMode && !m_smallScreenMode) {
			switchInterfaceMode(true);
		} else if(!smallScreenMode && m_smallScreenMode) {
			switchInterfaceMode(false);
		}
	}
}

bool MainWindow::shouldUseSmallScreenMode()
{
	switch(dpAppConfig()->getInterfaceMode()) {
	case int(view::InterfaceMode::Desktop):
		return false;
	case int(view::InterfaceMode::SmallScreen):
		return true;
	default:
		return utils::isSmallScreenModeSize(size());
	}
}

void MainWindow::switchInterfaceMode(bool smallScreenMode)
{
	setUpdatesEnabled(false);
	finishArrangingDocks();
	saveSplitterState();
	saveWindowState();
	m_smallScreenMode = smallScreenMode;

	QList<QDockWidget *> dockWidgets =
		findChildren<QDockWidget *>(QString(), Qt::FindDirectChildrenOnly);
	if(smallScreenMode) {
		if(m_hiddenDockState.isEmpty()) {
			m_desktopModeState = saveState();
		} else {
			m_desktopModeState.clear();
			m_desktopModeState.swap(m_hiddenDockState);
		}

		setFreezeDocks(false);
		for(QDockWidget *dw : dockWidgets) {
			dw->setFloating(false);
			dw->show();
			removeDockWidget(dw);
		}
		removeToolBar(m_toolBarFile);
		removeToolBar(m_toolBarEdit);
		m_splitter->setHandleWidth(0);
		m_chatbox->setSmallScreenMode(true);
		m_chatbox->hide();
		m_toolBarDraw->show();
		m_viewStatusBar->show();
		m_viewstatus->setHidden(true);
		setFreezeDocks(true);
		updateSideTabDocks();

		resetDefaultDocks();
		resetDefaultToolbars();
		initDefaultDocks();
		for(QDockWidget *dw : dockWidgets) {
			dw->hide();
		}
	} else {
		m_splitter->setHandleWidth(m_splitterOriginalHandleWidth);
		m_chatbox->show();
		m_chatbox->setSmallScreenMode(false);
		m_viewstatus->setHidden(false);
		m_viewStatusBar->show();
		updateSideTabDocks();

		QByteArray stateToRestore;
		stateToRestore.swap(m_desktopModeState);
		if(stateToRestore.isEmpty()) {
			stateToRestore = dpAppConfig()->getLastWindowState();
		}

		QScopedValueRollback<bool> rollback(m_restoringDockState, true);
		if(stateToRestore.isEmpty()) {
			restoreDefaultStateWith(dockWidgets);
		} else {
			restoreState(stateToRestore);
			setFreezeDocks(getAction("freezedocks")->isChecked());
		}
	}

	m_canvasView->setShowToggleItems(smallScreenMode, m_leftyMode);
	updateSmallScreenToolBarVisibility();
	updateInterfaceModeActions();
	reenableUpdates();

	if(smallScreenMode) {
		// Show the layers panel so the user can tell how large the UI is.
		Q_EMIT smallScreenPreviewRequested();
	} else {
		// Hide chat if not connected, since otherwise toggling to small-screen
		// mode and back makes it pop up.
		if(!m_chatbox->isCollapsed() && !m_doc->client()->isConnected()) {
			getAction("togglechat")->trigger();
		}
		updateIntendedDockState();
	}

	emit smallScreenModeChanged(smallScreenMode);
}

void MainWindow::updateSmallScreenToolBarVisibility()
{
	if(m_smallScreenMode) {
		bool sideAlwaysVisible =
			getAction("smallscreensidetoolbar")->isChecked();
		bool bottomAlwaysVisible =
			getAction("smallscreenbottomtoolbar")->isChecked();

		bool anyWidgetVisible = false;
		if(!sideAlwaysVisible || !bottomAlwaysVisible) {
			QWidget *widgets[] = {
				m_dockToolSettings, m_dockBrushPalette, m_dockTimeline,
				m_dockOnionSkins,	m_dockNavigator,	m_dockColorSpinner,
				m_dockColorSliders, m_dockColorPalette, m_dockColorCircle,
				m_dockReference,	m_dockLayers,		m_chatbox,
			};
			for(QWidget *widget : widgets) {
				if(widget->isVisible()) {
					anyWidgetVisible = true;
					break;
				}
			}
		}

		m_toolBarFile->setVisible(bottomAlwaysVisible || anyWidgetVisible);
		m_toolBarEdit->setVisible(bottomAlwaysVisible || anyWidgetVisible);
		m_toolBarDraw->setVisible(sideAlwaysVisible || anyWidgetVisible);
	}
}

bool MainWindow::shouldShowDialogMaximized() const
{
#ifdef SINGLE_MAIN_WINDOW
	return m_smallScreenMode;
#else
	return m_smallScreenMode && isMaximized();
#endif
}

void MainWindow::startIntendedDockStateDebounce()
{
	if(!m_updatingDockState && !m_smallScreenMode &&
	   m_hiddenDockState.isEmpty()) {
		m_updateIntendedDockStateDebounce.start();
	} else {
		m_updateIntendedDockStateDebounce.stop();
	}
}

void MainWindow::updateIntendedDockState()
{
	updateIntendedDockStateWith(false);
}

void MainWindow::updateIntendedDockStateWith(bool force)
{
	m_updateIntendedDockStateDebounce.stop();
	if(!m_updatingDockState && !m_smallScreenMode &&
	   m_hiddenDockState.isEmpty() &&
	   (force || canRememberDockStateFromWindow())) {
		m_intendedDockState = saveState();
	}
}

bool MainWindow::canRememberDockStateFromWindow() const
{
	// We only really want to save intended dock states from maximized windows.
	// On Linux, depending on your window manager, you may not have the concept
	// of a maximized window, so we just always remember the dock state. On
	// Android and in the browser, the window is always full screen anyway,
	// although the former might currently be in the process of scaling the UI,
	// during which we don't want to go around saving intermediate states.
#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
	return isMaximized() || isFullScreen();
#else
	return !DrawpileApp::isAndroidScalingDialogShown();
#endif
}

void MainWindow::restoreIntendedDockState()
{
	bool androidScalingJustChanged = dpApp().takeAndroidScalingJustChanged();
	QScopedValueRollback<bool> updateRollback(m_updatingDockState, true);
	m_restoreIntendedDockStateDebounce.stop();
	if(!m_restoringDockState && !m_smallScreenMode &&
	   m_hiddenDockState.isEmpty()) {
		if(!m_intendedDockState.isEmpty()) {
			QScopedValueRollback<bool> restoreRollback(
				m_restoringDockState, true);
			restoreState(m_intendedDockState);
		} else if(androidScalingJustChanged) {
			QScopedValueRollback<bool> restoreRollback(
				m_restoringDockState, true);
			restoreState(dialogs::LayoutsDialog::defaultState());
			refitWindow();
		}
	}
}

void MainWindow::startRefitWindowDebounce()
{
#ifdef SINGLE_MAIN_WINDOW
	if(!m_refitting) {
		m_refitWindowDebounce.start();
	}
#endif
}

void MainWindow::refitWindow()
{
#ifdef SINGLE_MAIN_WINDOW
	if(!m_refitting) {
		QScopedValueRollback<bool> rollback(m_refitting, true);
		m_refitWindowDebounce.stop();
		QCoreApplication::processEvents();
		resize(compat::widgetScreen(*this)->availableSize());
	}
#endif
}

void MainWindow::deactivateAllDocks()
{
	for(const docks::DockBase *dw : findChildren<const docks::DockBase *>(
			QString(), Qt::FindDirectChildrenOnly)) {
		QAction *action = dw->toggleViewAction();
		if(action->isChecked()) {
			action->trigger();
		}
	}
}

void MainWindow::prepareDockTabUpdate()
{
	if(!m_dockTabUpdatePending) {
		m_dockTabUpdatePending = true;
		emit dockTabUpdateRequested();
	}
}

void MainWindow::updateDockTabs()
{
	m_dockTabUpdatePending = false;
	bool showIcons =
		m_smallScreenMode || getAction("docktabicons")->isChecked();

	QHash<quintptr, docks::DockBase *> docksByTabId;
	for(docks::DockBase *dw : findChildren<docks::DockBase *>(
			QString(), Qt::FindDirectChildrenOnly)) {
		dw->setShowIcons(showIcons);
		if(!dw->isFloating()) {
			docksByTabId.insert(
				quintptr(qobject_cast<const QDockWidget *>(dw)), dw);
		}
	}

	for(QTabBar *tabBar :
		findChildren<QTabBar *>(QString(), Qt::FindDirectChildrenOnly)) {
		int count = tabBar->count();
		for(int i = 0; i < count; ++i) {
			quintptr value = tabBar->tabData(i).value<quintptr>();
			QHash<quintptr, docks::DockBase *>::iterator it =
				docksByTabId.find(value);
			if(it != docksByTabId.end()) {
				tabBar->setTabIcon(
					i, showIcons ? it.value()->tabIcon() : QIcon());
				docksByTabId.erase(it);
			}
		}
	}
}

#ifndef __EMSCRIPTEN__
bool MainWindow::saveAsType(int saveImageType, bool force)
{
	QString result = FileWrangler(this).saveImageAs(
		m_doc, false, DP_SaveImageType(saveImageType), force);
	if(result.isEmpty()) {
		return false;
	} else {
		addRecentFile(result, int(utils::Recents::Source::SaveAs));
		return true;
	}
}

void MainWindow::setPreferredSaveFormat(const QString &format)
{
	bool defaultIsOra = format == QStringLiteral("ora");
	getAction(QStringLiteral("savedocumentasdpcs"))->setVisible(defaultIsOra);
	getAction(QStringLiteral("savedocumentasora"))->setVisible(!defaultIsOra);
}
#endif

void MainWindow::setDonationLinkEnabled(bool enabled)
{
	QAction *action = searchAction(QStringLiteral("dpdonate"));
	if(action) {
		action->setEnabled(enabled);
		action->setVisible(enabled);
	}
}

QString MainWindow::makeContributionInfoText()
{
	if(!dpAppConfig()->getDonationLinksEnabled()) {
		return QString();
	}

	QColor color = palette().windowText().color();
	color.setAlphaF(0.7);
	QString attrs =
		QStringLiteral(" style=\"color:%1;\"").arg(color.name(QColor::HexArgb));

	QString donationText = utils::toHtmlWithLink(
		//: The [] will be turned into a clickable link! Keep them in
		//: translation. You can copy the heart ♥ into your text if it doesn't
		//: look weird for your language.
		QCoreApplication::translate(
			"donations", "[♥ Donate to Drawpile] to help keep development "
						 "going and the servers running."),
		utils::getDonationLink(), attrs);

	QString helpText = utils::toHtmlWithLink(
		//: The [] will be turned into a clickable link to Drawpile's help page!
		//: Keep them in your translation.
		tr("To report a bug or suggest a feature, [take a look here]."),
		utils::getHelpLink(), attrs);

	return QStringLiteral("<p%1>%2</p><p%1>%3<br></p>")
		.arg(attrs, donationText, helpText);
}
