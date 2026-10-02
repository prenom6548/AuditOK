#include "chooselineedit.h"

#include <QHBoxLayout>
#include <QIcon>

ChooseLineEdit::ChooseLineEdit(QWidget *parent /*= nullptr*/) :
    QLineEdit(parent)
{
    setTextMargins(0, 0, 62, 0);

    // 清除按钮
    m_clear = new ClickLabel;
    m_clear->setToolTip(tr("Clear"));
    m_clear->setFixedSize({ 30, 30 });
    QIcon titleIcon = QIcon::fromTheme("window-close-symbolic", QIcon(":/symbos/window-close-symbolic.svg"));
    m_clear->setPixmap(titleIcon.pixmap(titleIcon.actualSize({ 24, 24 })));
    connect(m_clear, &ClickLabel::clicked, this, [=]() {
        setText("");
        emit clearText();
    });
    m_clear->setVisible(false);
    connect(this, &ChooseLineEdit::textChanged, this, [=](const QString& text) {
        if (!text.isEmpty()) {
            m_clear->setVisible(true);
        } else {
            m_clear->setVisible(false);
        }
    });

    // 选择按钮
    m_choose = new ClickLabel;
    m_choose->setToolTip(tr("Choose"));
    m_choose->setFixedSize({ 30, 30 });
    titleIcon = QIcon::fromTheme("preferences-system-search-symbolic", QIcon(":/symbos/preferences-system-search-symbolic.svg"));
    m_choose->setPixmap(titleIcon.pixmap(titleIcon.actualSize({ 24, 24 })));
    connect(m_choose, &ClickLabel::clicked, this, [=](){
        emit choose();
    });

    QHBoxLayout * hlayout = new QHBoxLayout;
    hlayout->setMargin(5);
    hlayout->setSpacing(0);
    hlayout->addWidget(m_clear);
    hlayout->addWidget(m_choose);
    hlayout->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    setLayout(hlayout);
}

ChooseLineEdit::~ChooseLineEdit()
{}

