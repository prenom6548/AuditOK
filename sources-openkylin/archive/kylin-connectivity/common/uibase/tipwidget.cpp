//
// Created by sqp on 2021/11/17.
//

#include "tipwidget.h"
#include <QTimer>

TipWidget::TipWidget() : mpParent(nullptr), mnTransparent(200)
{
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAlignment(Qt::AlignCenter);

    mpTimer = new QTimer(this);
    connect(mpTimer, &QTimer::timeout, this, &TipWidget::OnTimer);
}

TipWidget::~TipWidget()
{
    deleteLater();
}

void TipWidget::OnTimer()
{
    mnTransparent -= 3;
    if (mnTransparent > 0) {
        if (mpParent && parentWidget()) {
            QPoint pt((parentWidget()->width() - width()) >> 1, (parentWidget()->height() - height()) - 100);
            if (pos() != pt) {
                move(pt);
            }
        }
        setStyleSheet(
            QString(
                "color:white;font:12px \"Microsoft YaHei\";border-radius:5px;background-color:rgba(80, 80, 80, %1);")
                .arg(mnTransparent));
    } else {
        mpTimer->stop();
        setVisible(false);
        deleteLater();
    }
}

void TipWidget::SetMesseage(const QString &strMessage, const QPoint *pPoint, const int time)
{
    if (strMessage.isEmpty()) {
        return;
    }

    QFontMetrics fm1(font());
    setFixedSize(fm1.width(strMessage) + 30, 30);

    mpParent = parentWidget();

    if (width() > mpParent->width()) {
        setFixedSize(mpParent->width() - 60, 60);
        setWordWrap(true);
    } else {
        setWordWrap(false);
    }

    setText(strMessage);

    if (nullptr != mpParent) {
        if (nullptr != pPoint) {
            move(mpParent->mapFromGlobal(*pPoint));
            mpParent = nullptr;
        } else {
            move((mpParent->width() - width()) >> 1, (mpParent->height() - height()) - 100);
        }
    }

    setVisible(true);
    mnTransparent = 200;

    mpTimer->start(3 * time);
}
