#include "passwordlabel.h"
#include <QPainter>
#include <QIcon>
#include <QGSettings/QGSettings>
#include <QDebug>
#include <QRect>

#define THEME_QT_SCHEMA  "org.ukui.style"
#define MODE_QT_KEY      "style-name"

EyeBtn::EyeBtn(QWidget *parent) :QLabel(parent)
{
    this->setPixmap(QIcon::fromTheme("ukui-eye-hidden-symbolic").pixmap(24,24));
    this->setProperty("useIconHighlightEffect", 0x2);
    this->setFixedSize(36, 36);
    this->setAlignment(Qt::AlignCenter);
}

EyeBtn::~EyeBtn()
{

}

void EyeBtn::setIconStatus(bool isHidden)
{
    if (isHidden) {
        this->setPixmap(QIcon::fromTheme("ukui-eye-hidden-symbolic").pixmap(24,24));
    } else {
        this->setPixmap(QIcon::fromTheme("ukui-eye-display-symbolic").pixmap(24,24));
    }
    mIsHidden = isHidden;
}

void EyeBtn::mousePressEvent(QMouseEvent * event)
{
}

void EyeBtn::mouseReleaseEvent(QMouseEvent *event)
{
    setIconStatus(!mIsHidden);

    Q_EMIT clicked(mIsHidden);
}

PasswordLabel::PasswordLabel(QWidget *parent) :QWidget(parent)
{
    mPassword  =  new QLineEdit(this);
    mPassword->setEnabled(false);
    mPassword->setEchoMode(QLineEdit::Password);
    mPassword->installEventFilter(this);
    QPalette palette = mPassword->palette();
    QColor color = palette.color(QPalette::Active, QPalette::Text);
    palette.setColor(QPalette::Disabled, QPalette::Button, Qt::transparent);
    palette.setColor(QPalette::Disabled, QPalette::Text, color);
    mPassword->setPalette(palette);

    mPassword->setContentsMargins(0,0,0,0);
    mPassword->setMinimumWidth(0);
    mEyesBtn = new EyeBtn(this);
    QHBoxLayout *mainHLayout = new QHBoxLayout();
    mainHLayout->setContentsMargins(0,0,0,0);
    mainHLayout->setSpacing(0);
    mainHLayout->addWidget(mPassword);
    mainHLayout->addWidget(mEyesBtn);
    mainHLayout->addStretch();
    this->setLayout(mainHLayout);

    const QByteArray idd(THEME_QT_SCHEMA);
    QGSettings *qtSettings  = new QGSettings(idd, QByteArray(), this);
    connect(qtSettings, &QGSettings::changed, this, [=](const QString &key) {
        if (key == "styleName") {
            QPalette textPal = mEyesBtn->palette();
            QColor color1 = textPal.color(QPalette::Active, QPalette::Text);
            textPal.setColor(QPalette::Disabled, QPalette::Button, Qt::transparent);
            textPal.setColor(QPalette::Disabled, QPalette::Text, color1);
            mPassword->setPalette(textPal);
        }
    });

    connect(mEyesBtn, &EyeBtn::clicked, this, [=](bool isHidden){
        if (isHidden) {
            mPassword->setEchoMode(QLineEdit::Password);
        } else {
            mPassword->setEchoMode(QLineEdit::Normal);
        }
    });
}

PasswordLabel::~PasswordLabel()
{

}

void PasswordLabel::setStatus(bool isPassword)
{
    if (isPassword && mPassword->echoMode() == QLineEdit::Normal) {
        mPassword->setEchoMode(QLineEdit::Password);
        mEyesBtn->setIconStatus(isPassword);
    }
}

void PasswordLabel::setText(const QString &str)
{
    mPassword->setText(str);
}

QString PasswordLabel::text() const
{
    return mPassword->text();
}

void PasswordLabel::paintEvent(QPaintEvent *event)
{
    QFontMetrics fontMetrics(this->font());
    if (mPassword->echoMode() == QLineEdit::Password) {
        QFontInfo fInfo(this->font());
        int charWidth = fInfo.pixelSize();
        mPassword->setFixedWidth(charWidth*mPassword->text().size() + 16);
    } else {
        int fontSize = fontMetrics.width(mPassword->text());
        mPassword->setFixedWidth(fontSize+16);
    }
}


