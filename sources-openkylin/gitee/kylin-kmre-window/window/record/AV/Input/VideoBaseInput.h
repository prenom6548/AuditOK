#pragma once

#include <QObject>

#include "../SourceSink.h"

class VideoBaseInput : public QObject, public VideoSource {
	Q_OBJECT

public:
	// Reads the current recording rectangle.
	virtual void GetCurrentRectangle(unsigned int* x, unsigned int* y, unsigned int* width, unsigned int* height);

	// Reads the current size of the stream.
	virtual void GetCurrentSize(unsigned int* width, unsigned int* height);

signals:
	void CurrentRectangleChanged();
	void ScreenSizeChanged();
};
