/* Copyright (C) 2020, KylinSoft Co., Ltd.
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#ifndef APNGIMAGEHANDLER_P_H
#define APNGIMAGEHANDLER_P_H

#include <QImageIOHandler>
#include <QVariant>
#include <QImage>
#include <QVector>
class ApngReader;

class ApngImageHandler : public QImageIOHandler
{
	Q_DISABLE_COPY(ApngImageHandler)
public:
	ApngImageHandler();
	~ApngImageHandler() override;

	// QImageIOHandler interface
	QByteArray name() const final;
	bool canRead() const final;
	bool read(QImage *image) final;
	QVariant option(ImageOption option) const final;
	bool supportsOption(ImageOption option) const final;
	bool jumpToNextImage() final;
	bool jumpToImage(int imageNumber) final;
	int loopCount() const final;
	int imageCount() const final;
	int nextImageDelay() const final;
	int currentImageNumber() const final;

private:
	quint32 _index = 0;
	QScopedPointer<ApngReader> _reader;
};

#endif // APNGIMAGEHANDLER_P_H
