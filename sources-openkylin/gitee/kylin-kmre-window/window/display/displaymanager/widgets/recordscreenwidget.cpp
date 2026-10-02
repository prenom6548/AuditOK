/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Alan Xie    xiehuijun@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "recordscreenwidget.h"
#include "ui_recordscreenwidget.h"
#include "common.h"
#include "kmreenv.h"
#include "utils.h"
#include "record/AV/Output/Muxer.h"
#include "record/AV/Output/VideoEncoder.h"
#include "record/AV/Output/AudioEncoder.h"
#include "record/AV/Output/Synchronizer.h"
#include "record/AV/Input/ALSAInput.h"
#include "messagebox.h"
#include "displaymanager/displaymanager.h"
#include "kmrewindow.h"
#include "sessionsettings.h"

#include "record/AV/Input/X11Input.h"

#include <QPropertyAnimation>
#include <QHBoxLayout>
#include <QDebug>

#define MAX_VIDEO_FILE_SIZE (100 * 1024 * 1024) // 100MB
#define DEFAULT_RECORD_PANEL_WIDTH 200
#define DEFAULT_RECORD_PANEL_HEIGHT 24

using DisplayPlatform = SessionSettings::DisplayPlatform;

RecordingToolPanel::RecordingToolPanel(KmreWindow* window)
	: QWidget(window)
	, mMainWindow(window)
	, mStarted(false)
{
	setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    setAttribute(Qt::WA_NoMousePropagation);
#if defined(__x86_64) || defined(__x86_64__) || defined(__amd64) || defined(__amd64__) || \
    defined(__i386) || defined(__i386__) || defined(__i686) || defined(__i686__)
    if (SessionSettings::getInstance().windowUsePlatformX11()) {
        setAttribute(Qt::WA_NativeWindow);
    }
#endif

	mRecordIcon = new QLabel(this);
	mRecordIcon->setFixedSize(16, 16);

	mTimeText = new QLabel(this);
	mTimeText->setText("00:00:00");

	mStartPauseBtn = new QPushButton(this);
	mStartPauseBtn->setIcon(QIcon(":/res/record_start.png"));
	mStartPauseBtn->setFixedSize(20, 20);
	mStartPauseBtn->setToolTip(tr("Start/Pause"));
	connect(mStartPauseBtn, SIGNAL(clicked()), this, SLOT(onStartPauseRecord()));

	mStopBtn = new QPushButton(this);
	mStopBtn->setIcon(QIcon(":/res/record_stop.png"));
	mStopBtn->setFixedSize(20, 20);
	mStopBtn->setToolTip(tr("Stop"));
	mStopBtn->setEnabled(false);
	connect(mStopBtn, SIGNAL(clicked()), this, SLOT(onStopRecord()));

	mShareBtn = new QPushButton(this);
	mShareBtn->setIcon(QIcon(":/res/record_share.png"));
	mShareBtn->setFixedSize(20, 20);
	mShareBtn->setToolTip(tr("Share"));
	mShareBtn->setEnabled(false);
	connect(mShareBtn, SIGNAL(clicked()), this, SLOT(onShareRecord()));

	mCloseBtn = new QPushButton(this);
	mCloseBtn->setIcon(QIcon(":/res/record_close.png"));
	mCloseBtn->setFixedSize(20, 20);
	mCloseBtn->setToolTip(tr("Cancel"));
	connect(mCloseBtn, SIGNAL(clicked()), this, SLOT(onCloseRecord()));

	QHBoxLayout *layout = new QHBoxLayout();
	layout->addWidget(mRecordIcon);
	layout->addWidget(mTimeText);
	layout->addWidget(mStartPauseBtn);
	layout->addWidget(mStopBtn);
	layout->addWidget(mShareBtn);
	layout->addWidget(mCloseBtn);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setAlignment(Qt::AlignCenter);
	setLayout(layout);

	setFixedSize(DEFAULT_RECORD_PANEL_WIDTH, DEFAULT_RECORD_PANEL_HEIGHT);
	setRecordIcon(mStarted);

	mTimer = new QTimer(this);
	connect(mTimer, SIGNAL(timeout()), this, SLOT(onUpdateTimeText()));
}

RecordingToolPanel::~RecordingToolPanel()
{
	mTimer->stop();
}

void RecordingToolPanel::setRecordIcon(bool on)
{
    if (on) {
        QPixmap icon(":/res/record.svg");
        icon = icon.scaled(mRecordIcon->width(), mRecordIcon->height());
        mRecordIcon->setPixmap(icon);
    }
    else {
        QPixmap icon(":/res/record2.svg");
        icon = icon.scaled(mRecordIcon->width(), mRecordIcon->height());
        mRecordIcon->setPixmap(icon);
    }
}

void RecordingToolPanel::updateTimeText()
{
	uint32_t hour = mSeconds / 60 / 60;
	uint32_t min = mSeconds / 60 % 60;
	uint32_t sec = mSeconds % 60;

	QString str_hour = (hour >= 10) ? QString::number(hour) : "0" + QString::number(hour);
	QString str_min = (min >= 10) ? QString::number(min) : "0" + QString::number(min);
	QString str_sec = (sec >= 10) ? QString::number(sec) : "0" + QString::number(sec);
	QString timeStr = str_hour + ":" + str_min + ":" + str_sec;

	mTimeText->setText(timeStr);
}

void RecordingToolPanel::onUpdateTimeText()
{
	static uint32_t counter = 0;

	counter++;
	setRecordIcon(counter % 2);
	if ((counter % 2) == 0) {// 1s
		mSeconds++;
		updateTimeText();
	}
}

void RecordingToolPanel::onStartPauseRecord()
{
	if (mStarted) {
		pause(true);
	}
	else {
		start(true);
	}
}

void RecordingToolPanel::onStopRecord()
{
	stop(true);
}

void RecordingToolPanel::onCloseRecord()
{
	if (mStarted) {
		if (KylinUI::MessageBox::question(parentWidget(), tr("Cancel recording?"), tr("Recording is going on! Do you want to cancel recording now?"))) {
            stop(false);
        }
		else {
			return;
		}
	}
	emit sigCloseRecord();
}

void RecordingToolPanel::onShareRecord()
{
	if (!mStarted) {
		emit sigShareRecord();
	}
}

void RecordingToolPanel::start(bool emitSig)
{
	mTimer->start(500);
	setRecordIcon(true);
	updateTimeText();
	mStarted = true;
	mStartPauseBtn->setIcon(QIcon(":/res/record_pause.png"));
	mStopBtn->setEnabled(true);
	mShareBtn->setEnabled(false);
	if (emitSig) {
		emit sigStartRecord();
	}
}

void RecordingToolPanel::pause(bool emitSig)
{
	mTimer->stop();
	setRecordIcon(false);
	mStarted = false;
	mStartPauseBtn->setIcon(QIcon(":/res/record_start.png"));
	mShareBtn->setEnabled(false);
	if (emitSig) {
		emit sigPauseRecord();
	}
}

void RecordingToolPanel::stop(bool emitSig)
{
	if (mStarted) {
		mTimer->stop();
		mStarted = false;
		mStartPauseBtn->setIcon(QIcon(":/res/record_start.png"));
		setRecordIcon(false);
	}
	mSeconds = 0;
	mStopBtn->setEnabled(false);
	mShareBtn->setEnabled(true);
	if (emitSig) {
		emit sigStopRecord();
	}
}

void RecordingToolPanel::showPanel(bool show)
{
	if (show) {
		setVisible(true);
		raise();// avoid to override by main window
	}
	else {
		setVisible(false);
		mMainWindow->getDisplayManager()->onDisplayForceRedraw();
	}
}

static std::vector<QRect> GetScreenGeometries() 
{
	std::vector<QRect> screen_geometries;
	for(QScreen *screen :  QApplication::screens()) {
		QRect geometry = screen->geometry();
		qreal ratio = screen->devicePixelRatio();
		screen_geometries.emplace_back(geometry.x(), geometry.y(), lrint((qreal) geometry.width() * ratio), lrint((qreal) geometry.height() * ratio));
	}
	return screen_geometries;
}

static QRect CombineScreenGeometries(const std::vector<QRect>& screen_geometries) 
{
	QRect combined_geometry;
	for(const QRect &geometry : screen_geometries) {
		combined_geometry |= geometry;
	}
	return combined_geometry;
}

static QPoint GetMousePhysicalCoordinates() 
{
	Window root, child;
	int root_x, root_y;
	int win_x, win_y;
	unsigned int mask_return;
	XQueryPointer(QX11Info::display(), QX11Info::appRootWindow(), &root, &child, &root_x, &root_y, &win_x, &win_y, &mask_return);
	return QPoint(root_x, root_y);
}

static QRect MapToLogicalCoordinates(const QRect& rect) 
{
	for(QScreen *screen :  QApplication::screens()) {
		QRect geometry = screen->geometry();
		qreal ratio = screen->devicePixelRatio();
		QRect physical_geometry(geometry.x(), geometry.y(), lrint((qreal) geometry.width() * ratio), lrint((qreal) geometry.height() * ratio));
		if(physical_geometry.contains(rect.center())) {
			return QRect(
						geometry.x() + lrint((qreal) (rect.x() - physical_geometry.x()) / ratio - 0.4999),
						geometry.y() + lrint((qreal) (rect.y() - physical_geometry.y()) / ratio - 0.4999),
						lrint((qreal) rect.width() / ratio - 0.4999),
						lrint((qreal) rect.height() / ratio - 0.4999));
		}
	}
	return rect;
}

static QRect ValidateRubberBandRectangle(QRect rect) 
{
	std::vector<QRect> screen_geometries = GetScreenGeometries();
	QRect combined_geometry = CombineScreenGeometries(screen_geometries);
	return rect.normalized() & combined_geometry.adjusted(-10, -10, 10, 10);
}

static QString GetNewSegmentFile(bool add_timestamp) 
{
    QString pdir = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);
    if (pdir.isEmpty()) {
        return QString("");
    }
    QDir dir(pdir);
    if(!dir.exists()) {
        bool ok = dir.mkpath(pdir);
        Q_UNUSED(ok)
    }

    QFileInfo fi(QString("%1/srv.mp4").arg(pdir));
	QDateTime now = QDateTime::currentDateTime();
	QString newfile;
	unsigned int counter = 0;
	do {
		++counter;
		newfile = fi.completeBaseName();
		if(add_timestamp) {
			if(!newfile.isEmpty())
				newfile += "-";
			newfile += now.toString("yyyy-MM-dd_hh.mm.ss");
		}
		if(counter != 1) {
			if(!newfile.isEmpty())
				newfile += "-";
			newfile += "(" + QString::number(counter) + ")";
		}
		if(!fi.suffix().isEmpty())
			newfile += "." + fi.suffix();
		newfile = fi.path() + "/" + newfile;
	} while(QFileInfo(newfile).exists());
	return newfile;
}

RecordingFrameWindow::RecordingFrameWindow(QWidget* parent, bool outside)
	: QWidget(parent, Qt::Window | Qt::BypassWindowManagerHint | Qt::FramelessWindowHint | 
		Qt::WindowStaysOnTopHint | Qt::WindowTransparentForInput | Qt::WindowDoesNotAcceptFocus) 
{
	QImage image(16, 16, QImage::Format_RGB32);
	image.fill(QColor(255, 255, 255, 255));
	m_texture = QPixmap::fromImage(image);
	UpdateMask();
}

void RecordingFrameWindow::SetRectangle(const QRect& r) 
{
	QRect rect = MapToLogicalCoordinates(ValidateRubberBandRectangle(r));
	if(rect.isEmpty()) {
		hide();
	} else {
		setGeometry(rect);
		show();
	}
}

void RecordingFrameWindow::UpdateMask() 
{
#if defined(KYLIN_V10)
	setMask(QRegion(0, 0, width(), height()).subtracted(QRegion(BORDER_WIDTH, BORDER_WIDTH, width() - 2 * BORDER_WIDTH, height() - 2 * BORDER_WIDTH)));
	setWindowOpacity(1.0);
#endif

    if (SessionSettings::getInstance().windowUsePlatformWayland()) {
        setMask(QRegion(0, 0, width(), height()).subtracted(QRegion(BORDER_WIDTH, BORDER_WIDTH, width() - 2 * BORDER_WIDTH, height() - 2 * BORDER_WIDTH)));
        setWindowOpacity(1.0);
    } else {
        if(QX11Info::isCompositingManagerRunning()) {
            clearMask();
            setWindowOpacity(0.5);
        } else {
            setMask(QRegion(0, 0, width(), height()).subtracted(QRegion(BORDER_WIDTH, BORDER_WIDTH, width() - 2 * BORDER_WIDTH, height() - 2 * BORDER_WIDTH)));
            setWindowOpacity(1.0);
        }
    }
}

void RecordingFrameWindow::resizeEvent(QResizeEvent *event) 
{
	Q_UNUSED(event);
	UpdateMask();
}

void RecordingFrameWindow::paintEvent(QPaintEvent* event) 
{
	Q_UNUSED(event);
	QPainter painter(this);
	m_texture.setDevicePixelRatio(devicePixelRatioF());
	painter.setPen(QColor(0, 0, 0, 128));
	painter.setBrush(Qt::NoBrush);
	painter.drawTiledPixmap(0, 0, width(), height(), m_texture);
	painter.drawRect(QRectF(0.5, 0.5, (qreal) width() - 1.0, (qreal) height() - 1.0));
}

static Window X11FindRealWindow(Display* display, Window window) 
{

	// is this the real window?
	Atom actual_type;
	int actual_format;
	unsigned long items, bytes_left;
	unsigned char *data = NULL;
	XGetWindowProperty(display, window, XInternAtom(display, "WM_STATE", true),
					   0, 0, false, AnyPropertyType, &actual_type, &actual_format, &items, &bytes_left, &data);
	if(data != NULL)
		XFree(data);
	if(actual_type != None)
		return window;

	// get the child windows
	Window root, parent, *childs;
	unsigned int childcount;
	if(!XQueryTree(display, window, &root, &parent, &childs, &childcount)) {
		return None;
	}

	// recursively call this function for all childs
	Window real_window = None;
	for(unsigned int i = childcount; i > 0; ) {
		--i;
		Window w = X11FindRealWindow(display, childs[i]);
		if(w != None) {
			real_window = w;
			break;
		}
	}

	// free child window list
	if(childs != NULL)
		XFree(childs);

	return real_window;

}

RecordScreenWidget::RecordScreenWidget(KmreWindow* window) 
	: QWidget(window)
	, mMainWindow(window)
  	, ui(new Ui::RecordScreenWidget)
  	, m_visibleState(STATE_HIDE)
	, mPreferences(KmreConfig::Preferences::getInstance())
	, mIsCodecChecked(false)
{
    ui->setupUi(this);
#if defined(__x86_64) || defined(__x86_64__) || defined(__amd64) || defined(__amd64__) || \
    defined(__i386) || defined(__i386__) || defined(__i686) || defined(__i686__)
    if (SessionSettings::getInstance().windowUsePlatformX11()) {
        setAttribute(Qt::WA_NativeWindow);
    }
#endif
    this->setBackgroundRole(QPalette::Base);
    this->setAutoFillBackground(true);
    this->setAttribute(Qt::WA_NoMousePropagation);
    this->setCursor(Qt::ArrowCursor);

    this->setFixedWidth(RECORD_WIDGET_DEFAULT_WIDTH);

    ui->btn_left->setCursor(Qt::PointingHandCursor);
    ui->btn_left->setFixedSize(RECORD_WIDGET_MIN_WIDTH, 80);
    ui->btn_left->setStyleSheet("QPushButton{border-image:url(':/res/spread-d.png');border:0px;} QPushButton:hover{border-image:url(':/res/spread-h.png');} QPushButton:pressed{border-image:url(':/res/spread-h.png');}");
    ui->btn_left->setToolTip(tr("Screen recording Shared"));

    ui->recordBtn->setCursor(Qt::PointingHandCursor);
	ui->recordBtn->setText(tr("Record"));
    ui->selectRegionBtn->setCursor(Qt::PointingHandCursor);

	mRecordingToolPanel = new RecordingToolPanel(window);
	mRecordingToolPanel->setVisible(false);

    this->initRecordingState();
    this->initConnection();
    this->initAnimation();

    InitDefaultSettings();
}

RecordScreenWidget::~RecordScreenWidget()
{
    StopRecord(false);
	if (m_file_watcher_timer) {
		m_file_watcher_timer->stop();
		delete m_file_watcher_timer;
	}
    delete ui;
}

void RecordScreenWidget::initRecordingState()
{
    m_record_started = false;
	m_input_started = false;
	m_output_started = false;
	m_grabbing = false;
	m_wait_saving = false;

    m_file_reached_max_size = false;
	m_file_watcher_timer = new QTimer(this);
	connect(m_file_watcher_timer, SIGNAL(timeout()), this, SLOT(OnCheckFileSize()));

	ui->audioCheckBox->setChecked(mPreferences->m_recordWithAudio);
    // Only support full screen recording on wayland!
    if (SessionSettings::getInstance().displayPlatform() == DisplayPlatform::WAYLAND) {
        mPreferences->m_recordFullscreen = true;
        ui->fullscreenCheckBox->setChecked(mPreferences->m_recordFullscreen);
        ui->fullscreenCheckBox->setEnabled(false);
        ui->selectRegionBtn->setEnabled(!mPreferences->m_recordFullscreen);
    } else {
        ui->fullscreenCheckBox->setChecked(mPreferences->m_recordFullscreen);
        ui->selectRegionBtn->setEnabled(!mPreferences->m_recordFullscreen);
    }
	ui->recordBtn->setEnabled(mPreferences->m_recordFullscreen);
}

void RecordScreenWidget::InitDefaultSettings() 
{
	// video input settings
	m_video_area = VIDEO_AREA_SCREEN;
	mPreferences->m_recordFullscreen = true;// record full screen default
	setRecordFullScreen();

	m_video_scaling = false;// no scaling default
	m_video_scaled_width = 854;
	m_video_scaled_height = 480;
	m_video_record_cursor = true;
	m_video_follow_cursor = false;

	// audio input settings
	m_audio_source_exist = true;
	mPreferences->m_recordWithAudio = false;// dont record audio default
	m_audio_codec_avname = QString();
	m_audio_sample_rate = 48000;
	m_audio_backend = AUDIO_BACKEND_ALSA;
	m_alsa_source = GetALSASourceName();
	if (m_alsa_source.isEmpty()) {
		syslog(LOG_ERR, "[RecordScreenWidget] Can't find alsa source! Disable audio record!");
        ui->audioCheckBox->setEnabled(false);
		m_audio_source_exist = false;
	}
}

void RecordScreenWidget::checkAudioVideoCodec()
{
	if (mIsCodecChecked) {
		return;
	}

	mIsCodecChecked = true;
	m_output_settings.file = QString(); // will be set later
	m_output_settings.container_avname = m_default_container;

	QString video_codec = QString();
	QString audio_codec = QString();

	for (auto &codec_name : m_supported_video_codecs) {
		if (VideoEncoder::AVCodecIsSupported(codec_name)) {
			video_codec = codec_name;
			break;
		}
	}
	for (auto &codec_name : m_supported_audio_codecs) {
		if (AudioEncoder::AVCodecIsSupported(codec_name)) {
			audio_codec = codec_name;
			break;
		}
	}

	if (video_codec.isEmpty()) {
        KylinUI::MessageBox::warning(this, tr("Warning"), tr("Can't find any supported video codecs!"));
		ui->recordBtn->setEnabled(false);
		ui->audioCheckBox->setEnabled(false);
        if (SessionSettings::getInstance().displayPlatform() == DisplayPlatform::X11) {
            ui->fullscreenCheckBox->setEnabled(false);
            ui->selectRegionBtn->setEnabled(false);
        }
	}
	else {
		m_output_settings.video_codec_avname = video_codec;
		m_output_settings.video_kbit_rate = 5000;
		m_output_settings.video_width = 0;
		m_output_settings.video_height = 0;
		m_output_settings.video_frame_rate = 30;
		m_output_settings.video_allow_frame_skipping = true;

		if (audio_codec.isEmpty()) {
			KylinUI::MessageBox::warning(this, tr("Warning"), tr("Can't find any supported audio codecs! will disable audio record!"));
			ui->audioCheckBox->setEnabled(false);
			m_output_settings.audio_codec_avname = QString();
		}
		else {
			m_audio_codec_avname = audio_codec;
			m_output_settings.audio_codec_avname = mPreferences->m_recordWithAudio ? m_audio_codec_avname : QString();
			m_output_settings.audio_kbit_rate = 128;
			m_output_settings.audio_channels = 2;
			m_output_settings.audio_sample_rate = m_audio_sample_rate;
		}
	}
}

void RecordScreenWidget::showOrHideUI()
{
    if (m_visibleState == STATE_SHOW) {
        if (m_record_started || mRecordingToolPanel->isVisible()) {
            slotHide(false);
        }
        else {
            slotHide(true);
        }
    }
    else {
		slotShow();
		checkAudioVideoCodec();
    }
}

void RecordScreenWidget::slotShow()
{
//    this->resize(RECORD_WIDGET_DEFAULT_WIDTH, this->height());
    this->setFixedWidth(RECORD_WIDGET_DEFAULT_WIDTH);

    if (pos().x() == ((QWidget*)parent())->width() - width() - MOUSE_MARGINS) {
        return;
    }

    if (m_showAnm->state() == QPropertyAnimation::Running) {
        return;
    }

    raise();
    QWidget::show();

    m_hideAnm->stop();
    m_showAnm->setStartValue(geometry());
    m_showAnm->setEndValue(QRect(((QWidget*)parent())->width() - width() - MOUSE_MARGINS, DEFAULT_TITLEBAR_HEIGHT, width(), height()));
    m_showAnm->start();

	m_visibleState = STATE_SHOW;
	//syslog(LOG_DEBUG, "[RecordScreenWidget][%s]", __func__);
}

void RecordScreenWidget::slotHide(bool b)
{
//    if (pos().x() == ((QWidget*)parent())->width() - MOUSE_MARGINS) {
//        return;
//    }

//    if (m_hideAnm->state() == QPropertyAnimation::Running) {
//        return;
//    }

//    m_showAnm->stop();

//    m_hideAnm->setStartValue(geometry());
//    m_hideAnm->setEndValue(QRect(((QWidget*)parent())->width() - MOUSE_MARGINS, DEFAULT_TITLEBAR_HEIGHT, width(), height()));
//    m_hideAnm->start();

//    QWidget::hide();

    if (m_hideAnm->state() == QPropertyAnimation::Running) {
        return;
    }

    lower();//解决隐藏时动画残影
    m_showAnm->stop();
	//syslog(LOG_DEBUG, "[RecordScreenWidget][%s] %s", __func__, b ? "hide" : "fold");
    if (b) {
        m_hideAnm->setStartValue(geometry());
        this->setFixedWidth(RECORD_WIDGET_MIN_WIDTH);
        m_hideAnm->setEndValue(QRect(((QWidget*)parent())->width() - MOUSE_MARGINS, DEFAULT_TITLEBAR_HEIGHT, RECORD_WIDGET_MIN_WIDTH, height()));
        m_hideAnm->start();

        QWidget::hide();
        m_visibleState = STATE_HIDE;
    }
    else {
        m_hideAnm->setStartValue(geometry());
        this->setFixedWidth(RECORD_WIDGET_MIN_WIDTH);
        m_hideAnm->setEndValue(QRect(((QWidget*)parent())->width() - MOUSE_MARGINS - RECORD_WIDGET_MIN_WIDTH, DEFAULT_TITLEBAR_HEIGHT, RECORD_WIDGET_MIN_WIDTH, height()));
        m_hideAnm->start();

        m_visibleState = STATE_FOLD;

        QTimer::singleShot(m_hideAnm->duration(), this, [=] {
            raise();
        });
    }
}

void RecordScreenWidget::updateShowIcon()
{
    if (m_showAnm) {
        m_showAnm->stop();
    }
    if (m_hideAnm) {
        m_hideAnm->stop();
    }

    ui->btn_left->setVisible(true);
    ui->btn_left->setStyleSheet("QPushButton{border-image:url(':/res/shrink-d.png');border:0px;} QPushButton:hover{border-image:url(':/res/shrink-h.png');} QPushButton:pressed{border-image:url(':/res/shrink-h.png');}");
}

void RecordScreenWidget::updateHideIcon()
{
    if (m_showAnm) {
        m_showAnm->stop();
    }
    if (m_hideAnm) {
        m_hideAnm->stop();
    }

    ui->btn_left->setVisible(true);
    ui->btn_left->setStyleSheet("QPushButton{border-image:url(':/res/spread-d.png');border:0px;} QPushButton:hover{border-image:url(':/res/spread-h.png');} QPushButton:pressed{border-image:url(':/res/spread-h.png');}");

    emit this->sigRefreshMainUI();
}

void RecordScreenWidget::initAnimation()
{
    m_showAnm = new QPropertyAnimation(this, "geometry");
    m_showAnm->setDuration(300);
    m_showAnm->setEasingCurve(QEasingCurve::OutCubic);
    connect(m_showAnm, &QPropertyAnimation::finished, this, &RecordScreenWidget::updateShowIcon);

    m_hideAnm = new QPropertyAnimation(this, "geometry");
    m_hideAnm->setDuration(300);
    m_hideAnm->setEasingCurve(QEasingCurve::OutCubic);//m_hideAnm->setEasingCurve(QEasingCurve::OutInExpo);
    connect(m_hideAnm, &QPropertyAnimation::finished, this, &RecordScreenWidget::updateHideIcon);
}

void RecordScreenWidget::initConnection()
{
    connect(ui->btn_left, &QPushButton::clicked, [&]() {
        ui->btn_left->setVisible(false);
        this->showOrHideUI();
    });

    connect(ui->recordBtn, &QPushButton::clicked, [&]() {
        if (!m_record_started) {
			slotHide(false);
			mRecordingToolPanel->showPanel(true);
			mRecordingToolPanel->start();
			updateRecordPanelPos();
			ui->recordBtn->setEnabled(false);
			OnRecordStart();
        }

    });

    connect(ui->selectRegionBtn, &QPushButton::clicked, [&]() {
        if (!m_record_started) {
            StartGrabbing();
        }
    });

    connect(ui->audioCheckBox, &QCheckBox::clicked, [&](bool cheched){
        mPreferences->m_recordWithAudio = cheched;
        m_output_settings.audio_codec_avname = cheched ? m_audio_codec_avname : QString();
    });

    connect(ui->fullscreenCheckBox, &QCheckBox::clicked, [&](bool cheched){
        mPreferences->m_recordFullscreen = cheched;
        if (SessionSettings::getInstance().displayPlatform() == DisplayPlatform::X11) {
            ui->selectRegionBtn->setEnabled(!cheched);
        }
        ui->recordBtn->setEnabled(cheched);
    });

	connect(mRecordingToolPanel, SIGNAL(sigStartRecord()), this, SLOT(OnRecordStart()));
	connect(mRecordingToolPanel, SIGNAL(sigPauseRecord()), this, SLOT(OnRecordPause()));
	connect(mRecordingToolPanel, SIGNAL(sigStopRecord()), this, SLOT(OnRecordStop()));
	connect(mRecordingToolPanel, SIGNAL(sigShareRecord()), this, SLOT(OnShareRecordedVideo()));
	connect(mRecordingToolPanel, SIGNAL(sigCloseRecord()), this, SLOT(OnCancelRecording()));
}

void RecordScreenWidget::resetRecordSettings()
{
	if (m_audio_source_exist) {
        ui->audioCheckBox->setEnabled(true);
    }
    if (SessionSettings::getInstance().displayPlatform() == DisplayPlatform::X11) {
        ui->fullscreenCheckBox->setEnabled(true);
        ui->selectRegionBtn->setEnabled(!mPreferences->m_recordFullscreen);
    }
	ui->recordBtn->setEnabled(true);
}

void RecordScreenWidget::updateRecordPanelPos()
{
	if (mRecordingToolPanel) {
		int xpos, ypos;
		DisplayManager *displayManager = mMainWindow->getDisplayManager();
		if (displayManager->isFullScreen()) {
			QRect screenSize = displayManager->getScreenSize();
			xpos = (screenSize.width() - DEFAULT_RECORD_PANEL_WIDTH) / 2;
			ypos = DEFAULT_TITLEBAR_HEIGHT;
		}
		else {
			xpos = (displayManager->getDisplayWidth() - DEFAULT_RECORD_PANEL_WIDTH) / 2 + MOUSE_MARGINS;
			ypos = DEFAULT_TITLEBAR_HEIGHT;
		}
		//syslog(LOG_DEBUG, "[RecordScreenWidget] Move Record Panel to: x = %d, y = %d", xpos, ypos);
		mRecordingToolPanel->move(xpos, ypos);
	}
}

void RecordScreenWidget::StartGrabbing() 
{
	m_grabbing = true;
    if (SessionSettings::getInstance().displayPlatform() == DisplayPlatform::X11) {
        ui->selectRegionBtn->setDown(true);
    }
	grabMouse(Qt::CrossCursor);
	grabKeyboard();
	setMouseTracking(true);
}

void RecordScreenWidget::StopGrabbing() 
{
	m_rubber_band.reset();
	setMouseTracking(false);
	releaseKeyboard();
	releaseMouse();
	activateWindow();
    if (SessionSettings::getInstance().displayPlatform() == DisplayPlatform::X11) {
        ui->selectRegionBtn->setDown(false);
    }
	m_grabbing = false;
}

void RecordScreenWidget::UpdateRubberBand() 
{
	if(m_rubber_band == NULL) {
		m_rubber_band.reset(new RecordingFrameWindow(this, false));
	}
	m_rubber_band->SetRectangle(m_rubber_band_rect);
}

void RecordScreenWidget::SetVideoAreaFromRubberBand() 
{
	QRect r = m_rubber_band_rect.normalized();
	m_video_x = r.x();
	m_video_y = r.y();
	m_video_in_width = r.width();
	m_video_in_height = r.height();
	mPreferences->m_recordFullscreen = false;
	ui->recordBtn->setEnabled(true);
	//syslog(LOG_DEBUG, "[RecordScreenWidget] %d,%d,%d,%d\n", r.x(), r.y(), r.width(), r.height());
}

void RecordScreenWidget::StartRecord() 
{
    if (!m_record_started) {
        m_record_started = true;
        m_wait_saving = false;
        slotHide(false);
        UpdateInput();
    }
}

void RecordScreenWidget::StopRecord(bool save) 
{
	if (m_record_started) {
        StopOutput(true);
        StopInput();

        if (m_output_manager) {// stop the output
            if(save) {
                FinishOutput();
				QString file_name = m_output_settings.file.section('/', -1);
				mMainWindow->getDisplayManager()->showTip(tr("Record video ") + "'" + file_name + "'" + tr(" saved in video folder under user home path."), 3000);
            }
            else if(QFileInfo(m_output_settings.file).exists()) {// delete the file if it isn't needed
                QFile(m_output_settings.file).remove();
            }
            m_output_manager.reset();
        }
        m_record_started = false;
        m_file_reached_max_size = false;
    }
}

void RecordScreenWidget::StartOutput()
{
    Q_ASSERT(m_record_started);
	if (!m_output_started) {
        try {
            //syslog(LOG_DEBUG, "[RecordScreenWidget] Starting output ...");
            if(m_output_manager == NULL) {
                m_output_settings.file = GetNewSegmentFile(true);
                if(m_video_scaling) {
                    m_output_settings.video_width = m_video_scaled_width / 2 * 2;
                    m_output_settings.video_height = m_video_scaled_height / 2 * 2;
                } 
                else {
                    m_video_in_width = m_video_in_width / 2 * 2;
                    m_video_in_height = m_video_in_height / 2 * 2;
                    m_output_settings.video_width = m_video_in_width;
                    m_output_settings.video_height = m_video_in_height;
                }
                //syslog(LOG_DEBUG, "[RecordScreenWidget] video_width = %d, video_height = %d", 
				//		m_output_settings.video_width, m_output_settings.video_height);
                m_output_manager.reset(new OutputManager(m_output_settings));
            } 
            else {
                m_output_manager->GetSynchronizer()->NewSegment();
            }

            m_output_started = true;
            this->slotHide(false);
            UpdateInput();
        } 
        catch(...) {
            syslog(LOG_ERR, "[RecordScreenWidget] Something went wrong during starting output.");
        }
    }
}

void RecordScreenWidget::StopOutput(bool final) 
{
    Q_ASSERT(m_record_started);
	if (m_output_started) {
        //syslog(LOG_DEBUG, "[RecordScreenWidget] Stopped output.\n");
        m_output_started = false;
        UpdateInput();
    }
}

void RecordScreenWidget::StartInput() 
{
	Q_ASSERT(m_record_started);

	if (m_input_started) {
        return;
    }
    Q_ASSERT(m_input == NULL);
	Q_ASSERT(m_alsa_input == NULL);

	try {
		//syslog(LOG_DEBUG, "[RecordScreenWidget] Starting input ...");
		//syslog(LOG_DEBUG, "[RecordScreenWidget] Recording area: x = %d, y = %d, width = %d, height = %d\n", 
		//		m_video_x, m_video_y, m_video_in_width, m_video_in_height);
		// start the video input
        if (SessionSettings::getInstance().displayPlatform() == DisplayPlatform::WAYLAND) {
            return;
        } else {
            m_input.reset(new X11Input(m_video_x, m_video_y, m_video_in_width, m_video_in_height, m_video_record_cursor, m_video_follow_cursor, m_video_area_follow_fullscreen));
        }
        connect(m_input.get(), SIGNAL(ScreenSizeChanged()), this, SLOT(OnScreenSizeChanged()), Qt::QueuedConnection);
		// start the audio input
		if(mPreferences->m_recordWithAudio) {
			if(m_audio_backend == AUDIO_BACKEND_ALSA)
				m_alsa_input.reset(new ALSAInput(m_alsa_source, m_audio_sample_rate));
		}
		m_input_started = true;
	} 
    catch(...) {
		syslog(LOG_ERR, "[RecordScreenWidget] Something went wrong during starting input.");
        m_input.reset();
		m_alsa_input.reset();
	}
}

void RecordScreenWidget::StopInput() 
{
    Q_ASSERT(m_record_started);
	if (m_input_started) {
        //syslog(LOG_DEBUG, "[RecordScreenWidget] Stopping input ...\n");
        m_input.reset();
        m_alsa_input.reset();
        m_input_started = false;
    }
}

void RecordScreenWidget::FinishOutput() 
{
	if (m_output_manager) {
        m_output_manager->Finish();
        m_wait_saving = true;
        unsigned int frames_done = 0, frames_total = 0;

        QProgressDialog dialog(tr("Encoding frame data ..."), QString(), 0, frames_total, this);
		dialog.setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
        dialog.setWindowModality(Qt::WindowModal);
        dialog.setCancelButton(NULL);
        dialog.setMinimumDuration(500);

        while(!m_output_manager->IsFinished()) {
            unsigned int frames = m_output_manager->GetTotalQueuedFrameCount();
            if(frames > frames_total)
                frames_total = frames;
            if(frames_total - frames > frames_done)
                frames_done = frames_total - frames;
            dialog.setMaximum(frames_total);
            dialog.setValue(frames_done);
            usleep(20000);
        }
        m_wait_saving = false;
    }
}

void RecordScreenWidget::UpdateInput() 
{
	Q_ASSERT(m_record_started);

	if(m_output_started) {
		StartInput();
	} 
    else {
		StopInput();
	}

	// get sources
	VideoSource *video_source = NULL;
	AudioSource *audio_source = NULL;
		
    video_source = m_input.get();

	if(mPreferences->m_recordWithAudio) {
		if(m_audio_backend == AUDIO_BACKEND_ALSA)
			audio_source = m_alsa_input.get();
	}

	// connect sinks
	if(m_output_manager != NULL) {
		if(m_output_started) {
			m_output_manager->GetSynchronizer()->ConnectVideoSource(video_source, 0);
			m_output_manager->GetSynchronizer()->ConnectAudioSource(audio_source, 0);
		} else {
			m_output_manager->GetSynchronizer()->ConnectVideoSource(NULL);
			m_output_manager->GetSynchronizer()->ConnectAudioSource(NULL);
		}
	}
}

void RecordScreenWidget::OnRecordStart() 
{
	if (IsBusy() || m_wait_saving) {
		return;
    }
	if (!m_file_reached_max_size) {
		//syslog(LOG_DEBUG, "[RecordScreenWidget][%s] ...\n", __func__);
		if (mPreferences->m_recordFullscreen) {
			setRecordFullScreen();
		}
		StartRecord();
        StartOutput();
		ui->audioCheckBox->setEnabled(false);
        if (SessionSettings::getInstance().displayPlatform() == DisplayPlatform::X11) {
            ui->fullscreenCheckBox->setEnabled(false);
            ui->selectRegionBtn->setEnabled(false);
        }

		m_file_watcher_timer->start(1000);
	}
}

void RecordScreenWidget::OnRecordPause()
{
	if (IsBusy() || (!m_record_started) || m_wait_saving) {
		return;
    }
	if (m_output_started) {
		//syslog(LOG_DEBUG, "[RecordScreenWidget][%s] ...\n", __func__);
		StopOutput(false);
        ui->audioCheckBox->setEnabled(false);
        if (SessionSettings::getInstance().displayPlatform() == DisplayPlatform::X11) {
            ui->fullscreenCheckBox->setEnabled(false);
            ui->selectRegionBtn->setEnabled(false);
        }

		m_file_watcher_timer->stop();
	}
}

void RecordScreenWidget::OnRecordStop()
{
	if (IsBusy() || (!m_record_started) || m_wait_saving) {
        return;
    }
    //syslog(LOG_DEBUG, "[RecordScreenWidget][%s] ...\n", __func__);
    StopRecord(true);

	m_file_watcher_timer->stop();
    m_file_reached_max_size = false;
}

void RecordScreenWidget::OnShareRecordedVideo() 
{
	if (IsBusy() || (m_record_started) || m_wait_saving) {
        return;
    }
    //syslog(LOG_DEBUG, "[RecordScreenWidget][%s] ...\n", __func__);

	// share video to app
	mMainWindow->shareFileToAndroid(m_output_settings.file);

	mRecordingToolPanel->showPanel(false);

	resetRecordSettings();
	slotHide(true);
}

void RecordScreenWidget::OnCancelRecording() 
{
	if (IsBusy() || m_wait_saving) {
        return;
    }
    //syslog(LOG_DEBUG, "[RecordScreenWidget][%s] ...\n", __func__);
	if (m_record_started) {
    	StopRecord(false);
	}

	mRecordingToolPanel->showPanel(false);

	resetRecordSettings();
	slotShow();
}

void RecordScreenWidget::OnScreenSizeChanged()
{
    syslog(LOG_ERR, "[RecordScreenWidget][%s] Screen size changed, so stop and cancel recording!\n", __func__);
	mRecordingToolPanel->stop();
	OnCancelRecording();
	KylinUI::MessageBox::warning(this, tr("Warning"), tr("Screen size changed, so stop and cancel recording!"));
}

void RecordScreenWidget::mousePressEvent(QMouseEvent* event) 
{
	if(m_grabbing) {
		if(event->button() == Qt::LeftButton) {
			if (calculateRubberBandRect()) {
				UpdateRubberBand();
			}
		} else {
			StopGrabbing();
		}
		event->accept();
		return;
	}
	event->ignore();
}

void RecordScreenWidget::mouseReleaseEvent(QMouseEvent* event) 
{
	if(m_grabbing) {
		if(event->button() == Qt::LeftButton) {
			if(m_rubber_band != NULL) {
				SetVideoAreaFromRubberBand();
			}
		}
		StopGrabbing();
		event->accept();
		return;
	}
	event->ignore();
}

void RecordScreenWidget::mouseMoveEvent(QMouseEvent* event) 
{
	if(m_grabbing) {
		if (calculateRubberBandRect()) {
			UpdateRubberBand();
		}
		event->accept();
		return;
	}
	event->ignore();
}

void RecordScreenWidget::keyPressEvent(QKeyEvent* event) 
{
	if(m_grabbing) {
		if(event->key() == Qt::Key_Escape) {
			StopGrabbing();
			return;
		}
		event->accept();
		return;
	}
	event->ignore();
}

bool RecordScreenWidget::calculateRubberBandRect()
{
	QPoint mouse_physical = GetMousePhysicalCoordinates();
	Window selected_window;
	int x, y;

	if (XTranslateCoordinates(QX11Info::display(), QX11Info::appRootWindow(), QX11Info::appRootWindow(), mouse_physical.x(), mouse_physical.y(), &x, &y, &selected_window)) {
		XWindowAttributes attributes;
		if (selected_window != None && XGetWindowAttributes(QX11Info::display(), selected_window, &attributes)) {

			// naive outer/inner rectangle, this won't work for window decorations
			m_select_window_outer_rect = QRect(attributes.x, attributes.y, attributes.width + 2 * attributes.border_width, attributes.height + 2 * attributes.border_width);
			m_select_window_inner_rect = QRect(attributes.x + attributes.border_width, attributes.y + attributes.border_width, attributes.width, attributes.height);

			// try to find the real window (rather than the decorations added by the window manager)
			Window real_window = X11FindRealWindow(QX11Info::display(), selected_window);
			if (real_window != None) {
				Atom actual_type;
				int actual_format;
				unsigned long items, bytes_left;
				long *data = NULL;
				int result = XGetWindowProperty(QX11Info::display(), real_window, XInternAtom(QX11Info::display(), "_NET_FRAME_EXTENTS", true),
												0, 4, false, AnyPropertyType, &actual_type, &actual_format, &items, &bytes_left, (unsigned char**) &data);
				if (result == Success) {
					if(items == 4 && bytes_left == 0 && actual_format == 32) { // format 32 means 'long', even if long is 64-bit ...
						Window child;
						// the attributes of the real window only store the *relative* position which is not what we need, so use XTranslateCoordinates again
						if(XTranslateCoordinates(QX11Info::display(), real_window, QX11Info::appRootWindow(), 0, 0, &x, &y, &child)
									&& XGetWindowAttributes(QX11Info::display(), real_window, &attributes)) {
							m_select_window_inner_rect = QRect(x, y, attributes.width, attributes.height);
							m_select_window_outer_rect = m_select_window_inner_rect.adjusted(-data[0], -data[2], data[1], data[3]);
						} 
                        else {
							m_select_window_inner_rect = m_select_window_outer_rect.adjusted(data[0], data[2], -data[1], -data[3]);
						}
					}
				}
				if (data != NULL)
					XFree(data);
			}
			// pick the inner rectangle if the users clicks inside the window, or the outer rectangle otherwise
			m_rubber_band_rect = (m_select_window_inner_rect.contains(mouse_physical))? m_select_window_inner_rect : m_select_window_outer_rect;
			return true;
		}
	}
	return false;
}

void RecordScreenWidget::setRecordFullScreen()
{
	QScreen *screen = QGuiApplication::primaryScreen();
	QSize size = screen->size() ;
	m_video_x = 0;
	m_video_y = 0;
	m_video_in_width = size.width();
	m_video_in_height = size.height();
	m_video_area_follow_fullscreen = false;
}

QString RecordScreenWidget::GetALSASourceName() 
{
	std::vector<ALSAInput::Source> m_alsa_sources;
	m_alsa_sources = ALSAInput::GetSourceList();
	// return the first alsa audio source (it's default source commonly)
	QString soure_name = m_alsa_sources.empty() ? QString() : QString::fromStdString(m_alsa_sources[0].m_name);
	syslog(LOG_INFO, "[GetALSASourceName] ALSA source name: %s", soure_name.toStdString().c_str());
	return soure_name;
}

void RecordScreenWidget::OnCheckFileSize()
{
	QFileInfo video_file(m_output_settings.file);
	if (video_file.exists()) {
		//syslog(LOG_DEBUG, "[RecordScreenWidget] OnCheckFileSize: %lld", video_file.size());
		if (video_file.size() > (qint64)(MAX_VIDEO_FILE_SIZE * 0.8)) {
			mRecordingToolPanel->stop();
			OnRecordStop();
			m_file_reached_max_size = true;
			KylinUI::MessageBox::warning(this, tr("Warning"), tr("Video file will be reach the largest limited size soon! So stop recording now!"));
			syslog(LOG_WARNING, "[RecordScreenWidget] Video file will be reach the largest limited size soon! So stop recording now!");
		}
	}
}

bool RecordScreenWidget::IsBusy() 
{
	return (QApplication::activeModalWidget() != NULL || QApplication::activePopupWidget() != NULL);
}
