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

#ifndef RECORDSCREENWIDGET_H
#define RECORDSCREENWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include "record/RecordDef.h"
#include "record/AV/Output/OutputSettings.h"
#include "record/AV/Output/OutputManager.h"
#include "record/AV/Input/ALSAInput.h"
#include "record/AV/Input/VideoBaseInput.h"
#include "preferences.h"

#define RECORD_WIDGET_DEFAULT_WIDTH 160
#define RECORD_WIDGET_MIN_WIDTH 16

class QPropertyAnimation;
class Muxer;
class VideoEncoder;
class AudioEncoder;
class Synchronizer;
class X11Input;
class WaylandInput;
class RecordScreenWidget;
class KmreWindow;

class RecordingToolPanel : public QWidget 
{
	Q_OBJECT

public:
	RecordingToolPanel(KmreWindow* window);
	~RecordingToolPanel();

	void start(bool emitSig = false);
	void pause(bool emitSig = false);
	void stop(bool emitSig = false);
	void showPanel(bool show);
	bool isStarted(){return mStarted;}

private slots:
	void onUpdateTimeText();
	void onStartPauseRecord();
	void onStopRecord();
	void onCloseRecord();
	void onShareRecord();

signals:
	void sigStartRecord();
	void sigPauseRecord();
	void sigStopRecord();
	void sigCloseRecord();
	void sigShareRecord();
	
private:
	void setRecordIcon(bool on);
	void updateTimeText();

private:
	KmreWindow* mMainWindow = nullptr;
	QLabel *mRecordIcon = nullptr;
	QLabel *mTimeText = nullptr;
	bool mStarted = false;
	QPushButton *mStartPauseBtn = nullptr;
	QPushButton *mStopBtn = nullptr;
	QPushButton *mShareBtn = nullptr;
	QPushButton *mCloseBtn = nullptr;
	QTimer *mTimer = nullptr;
	uint32_t mSeconds = 0;
};

class RecordingFrameWindow : public QWidget 
{
	Q_OBJECT

private:
	QPixmap m_texture;

public:
	static constexpr int BORDER_WIDTH = 6;

public:
	RecordingFrameWindow(QWidget* parent, bool outside);

	void SetRectangle(const QRect& r);

private:
	void UpdateMask();

protected:
	virtual void resizeEvent(QResizeEvent* event) override;
	virtual void paintEvent(QPaintEvent* event) override;

};

namespace Ui {
class RecordScreenWidget;
}

class RecordScreenWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RecordScreenWidget(KmreWindow* window);
    ~RecordScreenWidget();

	typedef enum {
		STATE_AUTO,
		STATE_START,
		STATE_PAUSE,
		STATE_STOP,
	}Record_State;

    void showOrHideUI();
	void updateRecordPanelPos();
    bool isRecordStarted(){return m_record_started;}

signals:
    void sigRefreshMainUI();
    void rqDragFileInfo(QString path, QString pkg_name);

public slots:
    void slotShow();
    void slotHide(bool b);
    void updateShowIcon();
    void updateHideIcon();
    void OnCheckFileSize();
    void OnShareRecordedVideo();
    void OnCancelRecording();
	void OnScreenSizeChanged();
	void OnRecordStart();
	void OnRecordPause();
	void OnRecordStop();

private:
	KmreWindow* mMainWindow = nullptr;
    Ui::RecordScreenWidget *ui;
	RecordingToolPanel *mRecordingToolPanel = nullptr;
	KmreConfig::Preferences *mPreferences;
	bool mIsCodecChecked;

	typedef enum {
		STATE_HIDE,
		STATE_FOLD,
		STATE_SHOW,
	}VisibleState;

    VisibleState m_visibleState;
    QPropertyAnimation *m_showAnm = nullptr;
    QPropertyAnimation *m_hideAnm = nullptr;

	bool m_record_started, m_input_started, m_output_started;
	bool m_grabbing, m_wait_saving;

	enum enum_video_area {
		VIDEO_AREA_SCREEN,
		VIDEO_AREA_FIXED,
		VIDEO_AREA_CURSOR,
		VIDEO_AREA_COUNT // must be last
	};
	enum_video_area m_video_area;
	bool m_video_area_follow_fullscreen;

	unsigned int m_video_x, m_video_y, m_video_in_width, m_video_in_height;
	bool m_video_scaling;
	unsigned int m_video_scaled_width, m_video_scaled_height;
	bool m_video_record_cursor;
	bool m_video_follow_cursor;
#if 0
	// default container format: "MKV"
	const QString m_default_container = "matroska";
	// supported video codec: H.264, VP8, Theora
	const QStringList m_supported_video_codecs = {"libx264", "libvpx", "libtheora"};
	// supported audio codec: Vorbis, MP3, AAC, Uncompressed
	const QStringList m_supported_audio_codecs = {"libvorbis", "libmp3lame", "libvo_aacenc", "pcm_s16le"};
#else 
	// default container format: "MP4"
	const QString m_default_container = "mp4";
	// supported video codec: H.264
	const QStringList m_supported_video_codecs = {"libx264"};
	// supported audio codec: Vorbis, MP3, AAC
	const QStringList m_supported_audio_codecs = {"libvorbis", "libmp3lame", "libvo_aacenc"};
#endif
	enum enum_audio_backend {
		AUDIO_BACKEND_ALSA,
		AUDIO_BACKEND_COUNT // must be last
	};
	enum_audio_backend m_audio_backend;
	QString m_alsa_source;
	unsigned int m_audio_sample_rate;
	bool m_audio_source_exist;
	QString m_audio_codec_avname;

	OutputSettings m_output_settings;
	std::unique_ptr<OutputManager> m_output_manager;

    std::unique_ptr<VideoBaseInput> m_input;
	std::unique_ptr<ALSAInput> m_alsa_input;

    std::unique_ptr<RecordingFrameWindow> m_rubber_band;
	QRect m_rubber_band_rect, m_select_window_outer_rect, m_select_window_inner_rect;

    QTimer *m_file_watcher_timer;
	bool m_file_reached_max_size;

    void initAnimation();
    void initConnection();
    void initRecordingState();
    bool calculateRubberBandRect();
    void InitDefaultSettings();
	void checkAudioVideoCodec();
    void setRecordFullScreen();
    QString GetALSASourceName();
    void StartGrabbing();
    void StopGrabbing();
    void UpdateRubberBand();
	void resetRecordSettings();
    void SetVideoAreaFromRubberBand();
    void StartRecord();
    void StopRecord(bool save);
    void StartOutput();
    void StopOutput(bool final);
    void StartInput();
    void StopInput();
    void FinishOutput();
    void UpdateInput();
    bool IsBusy();

    void mousePressEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);
	void keyPressEvent(QKeyEvent* event);

};

#endif // RECORDSCREENWIDGET_H
