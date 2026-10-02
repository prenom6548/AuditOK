#include "programkeyboary.h"

ProgramKeyboary::ProgramKeyboary(QWidget *parent) : QWidget(parent)
{
    init();
    initLayout();
}

void ProgramKeyboary::init()
{
    for (int i = 0; i < m_btnList.size(); i++) {
        QPushButton *btn = new QPushButton();
        btn->setAccessibleName("kylin-calculator_QPushButton_btn");
        // 设置按钮标识
        btn->setAccessibleName(m_btnList[i]);
        if (QString("NOT") == m_btnList[i] || QString("Lsh") == m_btnList[i] || QString("Rsh") == m_btnList[i]
            || QString("XY") == m_btnList[i] || QString("YX") == m_btnList[i]) {
            btn->setIconSize(QSize(162, 56));
        } else {
            btn->setIconSize(QSize(82, 56));
        }
        if (QString("Equal") == m_btnList[i]) {
            btn->setProperty("isImportant", true);
        } else {
            btn->setProperty("useButtonPalette", true);
        }
        btn->setIcon(QIcon(":/image/programmer/btn" + m_btnList[i] + ".svg"));
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        btn->setFocusPolicy(Qt::NoFocus);
        connect(btn, &QPushButton::clicked, this, &ProgramKeyboary::onClicked);
        if (m_btnDecDisList.contains(m_btnList[i])) {
            // 该按钮在十进制中需要禁用
            btn->setEnabled(false);
        }
        m_btnMap.insert(m_btnList[i], btn);
    }
}

void ProgramKeyboary::initLayout()
{
    m_layout = new QGridLayout();
    // 第一行
    m_layout->addWidget(m_btnMap.find(QString("OR")).value(), 0, 0, 1, 1);
    m_layout->addWidget(m_btnMap.find(QString("AC")).value(), 0, 1, 1, 1);
    m_layout->addWidget(m_btnMap.find(QString("Clear")).value(), 0, 2, 1, 1);
    m_layout->addWidget(m_btnMap.find(QString("D")).value(), 0, 3, 1, 1);
    m_layout->addWidget(m_btnMap.find(QString("E")).value(), 0, 4, 1, 1);
    m_layout->addWidget(m_btnMap.find(QString("F")).value(), 0, 5, 1, 1);
    m_layout->addWidget(m_btnMap.find(QString("Delete")).value(), 0, 6, 1, 1);
    // 第二行
    m_layout->addWidget(m_btnMap.find(QString("AND")).value(), 1, 0, 1, 1);
    m_layout->addWidget(m_btnMap.find(QString("NOT")).value(), 1, 1, 1, 1);
    m_layout->addWidget(m_btnMap.find(QString("Rev")).value(), 1, 2, 1, 1);
    m_layout->addWidget(m_btnMap.find(QString("A")).value(), 1, 3, 1, 1);
    m_layout->addWidget(m_btnMap.find(QString("B")).value(), 1, 4, 1, 1);
    m_layout->addWidget(m_btnMap.find(QString("C")).value(), 1, 5, 1, 1);
    m_layout->addWidget(m_btnMap.find(QString("ADD")).value(), 1, 6, 1, 1);
    // 第三行
    m_layout->addWidget(m_btnMap.find(QString("RoR")).value(), 2, 0, 1, 1);
    m_layout->addWidget(m_btnMap.find(QString("Rsh")).value(), 2, 1, 1, 2);
    m_layout->addWidget(m_btnMap.find(QString("7")).value(), 2, 3, 1, 1);
    m_layout->addWidget(m_btnMap.find(QString("8")).value(), 2, 4, 1, 1);
    m_layout->addWidget(m_btnMap.find(QString("9")).value(), 2, 5, 1, 1);
    m_layout->addWidget(m_btnMap.find(QString("SUB")).value(), 2, 6, 1, 1);
    // 第四行
    m_layout->addWidget(m_btnMap.find(QString("RoL")).value(), 3, 0, 1, 1);
    m_layout->addWidget(m_btnMap.find(QString("Lsh")).value(), 3, 1, 1, 2);
    m_layout->addWidget(m_btnMap.find(QString("4")).value(), 3, 3, 1, 1);
    m_layout->addWidget(m_btnMap.find(QString("5")).value(), 3, 4, 1, 1);
    m_layout->addWidget(m_btnMap.find(QString("6")).value(), 3, 5, 1, 1);
    m_layout->addWidget(m_btnMap.find(QString("Multi")).value(), 3, 6, 1, 1);
    // 第五行
    m_layout->addWidget(m_btnMap.find(QString("XOR")).value(), 4, 0, 1, 1);
    m_layout->addWidget(m_btnMap.find(QString("XY")).value(), 4, 1, 1, 2);
    m_layout->addWidget(m_btnMap.find(QString("1")).value(), 4, 3, 1, 1);
    m_layout->addWidget(m_btnMap.find(QString("2")).value(), 4, 4, 1, 1);
    m_layout->addWidget(m_btnMap.find(QString("3")).value(), 4, 5, 1, 1);
    m_layout->addWidget(m_btnMap.find(QString("Div")).value(), 4, 6, 1, 1);
    // 第六行
    m_layout->addWidget(m_btnMap.find(QString("NOR")).value(), 5, 0, 1, 1);
    m_layout->addWidget(m_btnMap.find(QString("YX")).value(), 5, 1, 1, 2);
    m_layout->addWidget(m_btnMap.find(QString("LBra")).value(), 5, 3, 1, 1);
    m_layout->addWidget(m_btnMap.find(QString("RBra")).value(), 5, 4, 1, 1);
    m_layout->addWidget(m_btnMap.find(QString("0")).value(), 5, 5, 1, 1);
    m_layout->addWidget(m_btnMap.find(QString("Equal")).value(), 5, 6, 1, 1);

    m_btnMap.find(QString("Rsh")).value()->setToolTip(tr("Move X 1 bit to the right"));
    m_btnMap.find(QString("Lsh")).value()->setToolTip(tr("Move X 1 bit to the left"));
    m_btnMap.find(QString("XY")).value()->setToolTip(tr("Move X to the right by y bits"));
    m_btnMap.find(QString("YX")).value()->setToolTip(tr("Move X to the left by y bits"));

    m_layout->setHorizontalSpacing(2);
    m_layout->setVerticalSpacing(2);
    m_layout->setMargin(0);

    this->setLayout(m_layout);
}

bool ProgramKeyboary::containsNum(QString num)
{
    return m_btnAllNumList.contains(num);
}

bool ProgramKeyboary::containsOp(QString op)
{
    return m_btnAllOpList.contains(op);
}


void ProgramKeyboary::setBtnList(QStringList btnList, bool state)
{
    for (int i = 0; i < btnList.size(); i++) {
        m_btnMap.find(btnList[i]).value()->setEnabled(state);
    }
}

void ProgramKeyboary::setBtnEnable(int base)
{
    setBtnList(m_btnAllNumList, true);
    switch (base) {
    case 2:
        setBtnList(m_btnBinDisList, false);
        break;
    case 8:
        setBtnList(m_btnOtcDisList, false);
        break;
    case 10:
        setBtnList(m_btnDecDisList, false);
        break;
    case 16:
        break;
    default:
        break;
    }
}

void ProgramKeyboary::onClicked()
{
    QPushButton *btn = qobject_cast<QPushButton *>(sender());
    // 获取按钮标识
    QString btnText = btn->accessibleName();
    if (m_symbolMap.contains(btnText)) {
        // 如果是运算符btnText需要变成对应的运算符
        btnText = m_symbolMap.find(btnText).value();
    }
    emit sigBtnClicked(btnText);
}

void ProgramKeyboary::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_0:
        m_btnMap.find(QString("0")).value()->click();
        break;
    case Qt::Key_1:
        m_btnMap.find(QString("1")).value()->click();
        break;
    case Qt::Key_2:
        m_btnMap.find(QString("2")).value()->click();
        break;
    case Qt::Key_3:
        m_btnMap.find(QString("3")).value()->click();
        break;
    case Qt::Key_4:
        m_btnMap.find(QString("4")).value()->click();
        break;
    case Qt::Key_5:
        m_btnMap.find(QString("5")).value()->click();
        break;
    case Qt::Key_6:
        m_btnMap.find(QString("6")).value()->click();
        break;
    case Qt::Key_7:
        m_btnMap.find(QString("7")).value()->click();
        break;
    case Qt::Key_8:
        m_btnMap.find(QString("8")).value()->click();
        break;
    case Qt::Key_9:
        m_btnMap.find(QString("9")).value()->click();
        break;
    case Qt::Key_A:
        m_btnMap.find(QString("A")).value()->click();
        break;
    case Qt::Key_B:
        m_btnMap.find(QString("B")).value()->click();
        break;
    case Qt::Key_C:
        if (event->modifiers() != Qt::ControlModifier) {
            m_btnMap.find(QString("C")).value()->click();
        }
        break;
    case Qt::Key_D:
        m_btnMap.find(QString("D")).value()->click();
        break;
    case Qt::Key_E:
        m_btnMap.find(QString("E")).value()->click();
        break;
    case Qt::Key_Multi_key:
        m_btnMap.find(QString("F")).value()->click();
        break;
    case Qt::Key_Plus:
        m_btnMap.find(QString("ADD")).value()->click();
        break;
    case Qt::Key_Minus:
        m_btnMap.find(QString("SUB")).value()->click();
        break;
    case Qt::Key_Asterisk:
        m_btnMap.find(QString("Multi")).value()->click();
        break;
    case Qt::Key_Slash:
        m_btnMap.find(QString("Div")).value()->click();
        break;
    case Qt::Key_Escape:
        m_btnMap.find(QString("Clear")).value()->click();
        break;
    case Qt::Key_ParenLeft:
        m_btnMap.find(QString("LBra")).value()->click();
        break;
    case Qt::Key_ParenRight:
        m_btnMap.find(QString("RBra")).value()->click();
        break;
    case Qt::Key_Backspace:
        m_btnMap.find(QString("Delete")).value()->click();
        break;
    case Qt::Key_Enter:
        m_btnMap.find(QString("Equal")).value()->click();
        break;
    case 16777220:
        m_btnMap.find(QString("Equal")).value()->click();
        break;
    case 0x3d:
        m_btnMap.find(QString("Equal")).value()->click();
        break;
    default:
        return QWidget::keyPressEvent(event);
    }

    return;
}
