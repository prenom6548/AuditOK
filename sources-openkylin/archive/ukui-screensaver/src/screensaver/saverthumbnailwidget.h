#ifndef SAVERTHUMBNAILWIDGET_H
#define SAVERTHUMBNAILWIDGET_H

#include "timethumbnailwidget.h"
#include "masklable.h"
#include <QVBoxLayout>
#include <QWidget>
#include <QVariantAnimation>

class SaverThumbnailWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SaverThumbnailWidget(QWidget *parent = nullptr, bool isControlSaver = false);
    void setBackground(QPixmap background);
    void setSelectStatus(bool status);
    void setDictumText(QString topText, QString bottomText, QString authorText);

private:
    void initUI();
    void animateSize(const QSize &targetSize);

protected:
    void paintEvent(QPaintEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);

Q_SIGNALS:
    void defaultSaverModeClicked();
private:
    TimeThumbnailWidget* m_timeThumbnailWidget = nullptr;
    QWidget *m_dictumWidget = nullptr;
    QLabel *m_topLabel = nullptr;
    QLabel *m_bottomLabel = nullptr;
    QLabel *m_authorLabel = nullptr;

    QVBoxLayout* m_mainLayout = nullptr;
    QPixmap m_background;
    QVariantAnimation* m_animation;
    MaskLable* m_maskLabel = nullptr;
    bool m_bHoverStatus = false;
    bool m_bSelectStatus = false;
    bool m_bControlSaver = false;
};

#endif // SAVERTHUMBNAILWIDGET_H
