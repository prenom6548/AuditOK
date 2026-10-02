#ifndef MOBILEQRCODE_H
#define MOBILEQRCODE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <kdialog.h>
#include "publicattributes.hpp"

class MobileQRcode : public QWidget
{
    Q_OBJECT
public:
    MobileQRcode(QWidget *parent = nullptr);

    void setQRCode(QString codeInfo = "");

    void setTheme(PublicAttributes::Theme theme);

private:
    // 初始化界面
    void initWidget();

private:
    QLabel *m_qrCodeLabel = nullptr;
    QLabel *m_illustrate = nullptr;
    QLabel *m_scanQRCode = nullptr;
    QLabel *m_appDownLab = nullptr;
    QLabel *m_descLab1 = nullptr;
    QPixmap m_pixmap;
    kdk::KDialog *m_downAppWin = nullptr;
    PublicAttributes::Theme m_theme = PublicAttributes::Theme::Light;

public Q_SLOTS:
    void onLinkActivated(const QString url);
};

#endif  // MOBILEQRCODE_H
