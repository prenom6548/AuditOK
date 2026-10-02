#include "toolbar.h"
#include <ktabbar.h>

ToolBar::ToolBar(QWidget *parent) : QWidget(parent)
{
    init();
    initLayout();
}


void ToolBar::init()
{
    // 创建控件
    m_btnASCII = new QPushButton();
    m_btnASCII->setAccessibleName("kylin-calculator_QPushButton_m_btnASCII");
    m_btnUnicode = new QPushButton();
    m_btnUnicode->setAccessibleName("kylin-calculator_QPushButton_m_btnUnicode");
    m_labCode = new QLabel();
    m_labCode->setAccessibleName("kylin-calculator_QLabel_m_labCode");
    m_btnBin = new QPushButton();
    m_btnBin->setAccessibleName("kylin-calculator_QPushButton_m_btnBin");
    m_btnMS = new QPushButton();
    m_tabBase = new kdk::KTabBar(kdk::SegmentDark, this);
    m_tabBase->addTab("8");
    m_tabBase->addTab("10");
    m_tabBase->addTab("16");
    m_tabBase->setBorderRadius(4);
    m_tabBase->setFont(QFont("SourceHanSansCN"));
    m_tabBase->setUsesScrollButtons(false);
    m_tabBase->setCurrentIndex(1);
    connect(m_tabBase, &QTabBar::currentChanged,
            this, &ToolBar::onBaseTabClicked);
    m_labBase = new QLabel();
    m_labBase->setAccessibleName("kylin-calculator_QLabel_m_labBase");
    m_boxDigit = new QComboBox();
    m_boxDigit->setAccessibleName("kylin-calculator_QComboBox_m_boxDigit");
    // 设置大小
    m_btnASCII->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_btnUnicode->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_btnBin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_btnMS->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_boxDigit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_btnASCII->setFocusPolicy(Qt::NoFocus);
    m_btnUnicode->setFocusPolicy(Qt::NoFocus);
    m_btnBin->setFocusPolicy(Qt::NoFocus);
    m_btnMS->setFocusPolicy(Qt::NoFocus);
    m_boxDigit->setFocusPolicy(Qt::NoFocus);

    m_btnASCII->setMinimumWidth(75);
    m_btnUnicode->setMinimumWidth(100);
    m_btnBin->setMinimumWidth(113);
    m_btnMS->setMinimumWidth(55);
    m_boxDigit->setMinimumWidth(100);


    m_btnASCII->setFont(QFont("SourceHanSansCN"));
    m_btnUnicode->setFont(QFont("SourceHanSansCN"));
    m_btnBin->setFont(QFont("SourceHanSansCN"));
    m_btnMS->setFont(QFont("SourceHanSansCN"));
    m_boxDigit->setFont(QFont("SourceHanSansCN"));

    // 按钮设置文本
    m_btnASCII->setText("ASCII");
    m_btnUnicode->setText("Unicode");
    m_btnBin->setText(tr("ShowBinary"));
    m_btnMS->setText("MS");

//    m_boxDigit->setStyleSheet("font-size:14px;QComboBox{font-size:14px;}");
//    m_btnBin->setStyleSheet("font-size:14px;");
//    m_btnASCII->setStyleSheet("font-size:14px;");
//    m_btnUnicode->setStyleSheet("font-size:14px;");
//    m_btnMS->setStyleSheet("font-size:14px;");

    // 连接槽
    connect(m_btnASCII, &QPushButton::clicked, this, &ToolBar::onClicked);
    connect(m_btnUnicode, &QPushButton::clicked, this, &ToolBar::onClicked);
    connect(m_btnMS, &QPushButton::clicked, this, &ToolBar::onClicked);
    connect(m_btnBin, &QPushButton::clicked, this, &ToolBar::onClicked);

    // 设置下拉列表
    m_boxDigit->addItem("QWORD"); // index = 0
    m_boxDigit->addItem("DWORD"); // index = 1
    m_boxDigit->addItem("WORD");  // index = 2
    m_boxDigit->addItem("BYTE");  // index = 3
    connect(m_boxDigit, SIGNAL(currentIndexChanged(int)), this, SLOT(valueChanged(int)));

    // 暂时现将MS功能隐藏
    m_btnMS->hide();
}

void ToolBar::initLayout()
{
    m_layout = new QHBoxLayout();
    m_layoutCode = new QHBoxLayout();
    m_layoutBase = new QHBoxLayout();

    m_layoutCode->addWidget(m_btnASCII);
    m_layoutCode->addSpacing(8);
    m_layoutCode->addWidget(m_btnUnicode);
    m_layoutCode->setSpacing(0);
    m_layoutCode->setMargin(0);
    m_labCode->setLayout(m_layoutCode);
    m_labCode->layout()->setSizeConstraint(QLayout::SetMinimumSize);

    m_layoutBase->addWidget(m_tabBase);
    m_layoutBase->setSpacing(0);
    m_layoutBase->setMargin(0);
    m_labBase->setLayout(m_layoutBase);
    m_labBase->layout()->setSizeConstraint(QLayout::SetMinimumSize);

    m_layout->addWidget(m_labCode);
    m_layout->addStretch(1);
    m_layout->addWidget(m_boxDigit);
    m_layout->addStretch(1);
    m_layout->addWidget(m_btnBin);
    m_layout->addStretch(1);
    m_layout->addWidget(m_btnMS);
    m_layout->addStretch(1);
    m_layout->addWidget(m_labBase);
    m_layout->setMargin(0);

    this->setLayout(m_layout);
}

void ToolBar::setBaseEnabled(bool state)
{
    m_tabBase->setEnabled(state);
    m_btnASCII->setEnabled(state);
    m_btnUnicode->setEnabled(state);
    m_boxDigit->setEnabled(state);
}

void ToolBar::onClicked()
{
    QPushButton *btn = qobject_cast<QPushButton *>(sender());

    // 设置特殊按钮组的状态
    if (m_btnASCII == btn) {
        if (m_isASCII) {
            // 按钮已经被选中
            emit sigBtnClicked(QString("cancel"));
            m_isASCII = false;
            m_btnASCII->setProperty("isImportant", false);
        } else {
            // 按钮未被选中,发送信号
            emit sigBtnClicked(btn->text());
            m_isASCII = true;
            m_isUnicode = false;
            m_btnASCII->setProperty("isImportant", true);
            m_btnUnicode->setProperty("isImportant", false);
        }
        m_btnASCII->update();
        m_btnUnicode->update();
        return;
    }
    if (m_btnUnicode == btn) {
        if (m_isUnicode) {
            // 按钮已经被选中
            emit sigBtnClicked(QString("cancel"));
            m_isUnicode = false;
            m_btnUnicode->setProperty("isImportant", false);
        } else {
            // 按钮未被选中
            emit sigBtnClicked(btn->text());
            m_isASCII = false;
            m_isUnicode = true;
            m_btnASCII->setProperty("isImportant", false);
            m_btnUnicode->setProperty("isImportant", true);
       }
        m_btnASCII->update();
        m_btnUnicode->update();
        return;
    }

    emit sigBtnClicked(btn->text());


    // 设置m_btnBin文本
    if (QString(tr("ShowBinary")) == btn->text()) {
        btn->setText(tr("HideBinary"));
    } else if (QString(tr("HideBinary")) == btn->text()) {
        btn->setText(tr("ShowBinary"));
    }
}

void ToolBar::valueChanged(int index)
{
    emit sigBoxValueChanged(index);
}


// —— 当用户点击 8/10/16 时发信号 ——
void ToolBar::onBaseTabClicked(int index)
{
    emit sigBtnClicked(m_tabBase->tabText(index));
}
