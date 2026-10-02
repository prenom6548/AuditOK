#include "messagedialog.h"
#include "windowmanage.hpp"

const int ICON_SIZE = 24;
const int BTN_W = 88;
const int BTN_H = 48;
const int WIN_W = 510;
const int WIN_H = 202;

MessageDialog::MessageDialog(QWidget *parent) : QDialog(parent)
{
    kabase::WindowManage::removeHeader(this);
    setWindowModality(Qt::WindowModal);
    setWindowModality(Qt::ApplicationModal);
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);
    setWindowIcon(QIcon::fromTheme("kylin-connectivity"));

    m_titleIcon = new QPushButton(this);
    m_titleIcon->setIconSize(QSize(24, 24));
    QString m_titleIconStyle = "QPushButton{border:0px;border-radius:4px;background:transparent;}"
                               "QPushButton:Hover{border:0px;border-radius:4px;background:transparent;}"
                               "QPushButton:Pressed{border:0px;border-radius:4px;background:transparent;}";
    m_titleIcon->setStyleSheet(m_titleIconStyle);
    m_titleIcon->setIcon(QIcon::fromTheme("kylin-connectivity"));
    m_titleIcon->setContentsMargins(8, 8, 8, 8);

    m_titleNameLab = new QLabel(this);
    m_titleNameLab->setText(tr("kylin-connectivity"));

    m_closeBtn = new QPushButton(this);
    m_closeBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));
    m_closeBtn->setFixedSize(36, 36);
    m_closeBtn->setIconSize(QSize(16, 16));
    m_closeBtn->setProperty("isWindowButton", 0x2);
    m_closeBtn->setProperty("useIconHighlightEffect", 0x8);
    m_closeBtn->setFlat(true);
    m_closeBtn->setContentsMargins(4, 0, 4, 0);
    m_closeBtn->setFocusPolicy(Qt::NoFocus);
    m_closeBtn->setToolTip(tr("Close"));
    connect(m_closeBtn, SIGNAL(clicked()), this, SLOT(close()));

    QHBoxLayout *titleLay = new QHBoxLayout;
    titleLay->setMargin(0);
    titleLay->setSpacing(0);
    titleLay->addSpacing(8);
    titleLay->addWidget(m_titleIcon);
    titleLay->addWidget(m_titleNameLab);
    titleLay->addStretch();
    titleLay->addWidget(m_closeBtn);
    titleLay->addSpacing(4);

    m_icon = new QLabel(this);
    m_icon->resize(ICON_SIZE, ICON_SIZE);

    QVBoxLayout *iconLayout = new QVBoxLayout;
    iconLayout->setMargin(0);
    iconLayout->setSpacing(0);
    iconLayout->addWidget(m_icon);
    iconLayout->addStretch();

    m_textLayout = new QVBoxLayout;
    m_textLayout->setMargin(0);
    m_textLayout->setSpacing(0);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setContentsMargins(24, 0, 24, 0);
    hLayout->setSpacing(0);
    hLayout->addLayout(iconLayout);
    hLayout->addSpacing(8);
    hLayout->addLayout(m_textLayout);
    hLayout->addStretch();

    m_btnLayout = new QHBoxLayout;
    m_btnLayout->setContentsMargins(24, 0, 24, 0);
    m_btnLayout->setSpacing(14);
    m_btnLayout->addStretch();

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(0, 4, 0, 24);
    layout->setSpacing(0);
    layout->addLayout(titleLay);
    layout->addSpacing(22);
    layout->addLayout(hLayout);
    layout->addStretch();
    layout->addLayout(m_btnLayout);

    setLayout(layout);
    // mainWidget()->setLayout(layout);
}

void MessageDialog::setText(QString title, QString text)
{
    m_titleLab = new QLabel(this);
    m_titleLab->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_titleLab->setText(title);
    m_textLayout->addWidget(m_titleLab);
    if (text.isEmpty()) {
        setFixedSize(WIN_W, WIN_H);
    } else {
        setFixedSize(WIN_W, WIN_H + 30);
        QFont font;
        font.setBold(true);
        m_titleLab->setFont(font);

        m_textLab = new QTextEdit(this);
        m_textLab->setFixedWidth(400);
        m_textLab->setFrameStyle(QFrame::NoFrame);
        m_textLab->setReadOnly(true);
        m_textLab->setTextInteractionFlags(Qt::NoTextInteraction);
        m_textLab->viewport()->setCursor(Qt::ArrowCursor);
        m_textLab->setText(text);
        m_textLayout->addSpacing(8);
        m_textLayout->addWidget(m_textLab);
    }
    m_textLayout->addStretch();
}

void MessageDialog::setIconPixmap(QIcon icon)
{
    m_icon->setPixmap(icon.pixmap(QSize(ICON_SIZE, ICON_SIZE)));
}

QPushButton *MessageDialog::addButton(QString text)
{
    QPushButton *btn = new QPushButton(this);
    btn->setText(text);
    connect(btn, &QPushButton::clicked, this, [=]() {
        Q_EMIT buttonClicked(btn);
        close();
    });
    m_btnLayout->addWidget(btn);

    return btn;
}

void MessageDialog::closeEvent(QCloseEvent *event)
{
    Q_EMIT sigClose();

    return QDialog::closeEvent(event);
}
