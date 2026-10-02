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
#ifndef OCR_H
#define OCR_H

#include <QObject>
#include <QVector>
#include <QString>

enum EngineType { KySDK = 0, Others = 1 };

class OCR : public QObject
{
    Q_OBJECT
public:
    OCR();
    virtual ~OCR() {}
    virtual void getText(QString) = 0;
    //接口暂未启用,先注释掉
    //    virtual void getRect(QString);
Q_SIGNALS:
    void recResult(QString, QVector<QString>);

private:
    OCR *m_ocr = nullptr;
};
class OcrFunction : public QObject
{
    Q_OBJECT
public:
    OcrFunction();
    ~OcrFunction();
    void setEngineType(EngineType type = EngineType::KySDK);
    void getText(QString imgPath);

private:
    OCR *m_ocr = nullptr;
    QThread *m_thread = nullptr;
    EngineType m_type;
Q_SIGNALS:
    void recResult(QString, QVector<QString>);
    void realGetText(QString);
};
#endif // OCR_H
