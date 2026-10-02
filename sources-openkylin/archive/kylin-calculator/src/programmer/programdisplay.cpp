#include "programdisplay.h"

ProgramDisplay::ProgramDisplay(QWidget *parent) : QWidget(parent)
{
    init();
    initLayout();
}

void ProgramDisplay::setHisLab(QString value)
{
    if (value.isEmpty()) {
        m_labHis->setText("");
        return;
    }

    // 获取已显示的文本
    QString hisLabText = m_labHis->text();
    if (hisLabText.isEmpty()) {
        // 当前没有历史记录，直接设置
        m_labHis->setText(value);
        return;
    }

    if (hisLabText.count("\n") < HIS_NUM_MAX - 1) {
        // 显示的历史记录少于5条
        hisLabText = hisLabText + "\n" + value;
        m_labHis->setText(hisLabText);
        return;
    }
    // 显示的历史记录多余五条
    QString newHisLabText;
    // 将第一条历史记录去除
    for (int i = 0;i < HIS_NUM_MAX - 1; i++) {
        int index = hisLabText.lastIndexOf("\n");
        QString str = hisLabText.mid(index + 1, hisLabText.size() - index - 1);
        newHisLabText.push_front(str + "\n");
        hisLabText = hisLabText.mid(0, index);
    }
    // 将传入的历史记录添加
    newHisLabText = newHisLabText + value;
    m_labHis->setText(newHisLabText);
}

void ProgramDisplay::setBudLab(QString value)
{
    m_labBud->setText(value);

    // 还原lab字号
    m_labBud->setStyleSheet("color:#FB9119;font-size:20px;margin:0 7px 0 7px;");

    // 获取lab适应字号
    QString budSize = QString::number(fontSize(m_labBud));

    if ("-1" == budSize || !m_isInput) {
        // 字符过长，设置错误
        m_labBud->setText(tr("input too long!"));
        m_labBud->setStyleSheet("color:#FB9119;font-size:20px;margin:0 7px 0 7px;");
        return;
    }

    // 设置lab字号
    m_labBud->setStyleSheet("color:#FB9119;font-size:" + budSize + "px;margin:0 7px 0 7px;");
}

void ProgramDisplay::setCurLab(QString value)
{
    QString labText = m_labCur->text();
    m_labCur->setText(value);

    // 还原lab字号
    m_labCur->setStyleSheet("color:" + m_fontColor + ";font-size:36px;font-weight:15px;margin:0 0 5px 7px;");

    // 获取lab适应字号
    QString curSize = QString::number(fontSize(m_labCur));

    if (QString("-1") == curSize) {
        // 字符过长，设置错误
        m_isInput = false;
        m_labBud->setText(tr("input too long!"));
        m_labBud->setStyleSheet("color:#FB9119;font-size:20px;margin:0 7px 0 7px;");
        m_labCur->setText(labText);
        QString curSize = QString::number(fontSize(m_labCur));
        // 设置lab字号
        m_labCur->setStyleSheet("color:" + m_fontColor + ";font-size:" + curSize + "px;font-weight:15px;margin:0 0 5px 7px;");
        return;
    }

    // 设置lab字号
    m_labCur->setStyleSheet("color:" + m_fontColor + ";font-size:" + curSize + "px;font-weight:15px;margin:0 0 5px 7px;");
}

void ProgramDisplay::setCodeLab(QString value)
{
    m_labCode->setText(value);
    m_labCode->setStyleSheet("font-size:14px;");
}

QVector<QString> ProgramDisplay::data()
{
    QVector<QString> vector;
    vector.push_back(m_labCur->text());
    vector.push_back(m_labBud->text());
    vector.push_back(m_labHis->text());
    vector.push_back(m_labCode->text());

    return vector;
}

void ProgramDisplay::clearLab()
{
    m_labHis->setText("");
    m_labBud->setText("");
    m_labCur->setText("0");
    m_labCode->setText("");
    m_isInput = true;
}

void ProgramDisplay::setLightUI()
{
    m_fontColor = "#272A2D";
    m_labHis->setStyleSheet("color:#8C8C8C;font-size:12px;margin:0 7px 0 7px;");
    m_labBud->setStyleSheet("color:#FB9119;font-size:20px;margin:0 7px 0 7px;");
    m_labCur->setStyleSheet("color:" + m_fontColor + ";font-size:36px;font-weight:15px;margin:0 0 5px 7px;");
}

void ProgramDisplay::setDarkUI()
{
    m_fontColor = "#FFFFFF";
    m_labHis->setStyleSheet("color:#8C8C8C;font-size:12px;margin:0 7px 0 7px;");
    m_labBud->setStyleSheet("color:#FB9119;font-size:20px;margin:0 7px 0 7px;");
    m_labCur->setStyleSheet("color:" + m_fontColor + ";font-size:48px;font-weight:15px;margin:0 0 0 7px;");
}

void ProgramDisplay::setIsInput(bool state)
{
    m_isInput = state;
}

bool ProgramDisplay::isInput()
{
    return m_isInput;
}


void ProgramDisplay::init()
{
    m_labHis = new QLabel();
    m_labHis->setAccessibleName("kylin-calculator_QLabel_m_labHis");
    m_labBud = new QLabel();
    m_labBud->setAccessibleName("kylin-calculator_QLabel_m_labBud");
    m_labCur = new QLabel();
    m_labCur->setAccessibleName("kylin-calculator_QLabel_m_labCur");
    m_labCode = new QLabel();
    m_labCode->setAccessibleName("kylin-calculator_QLabel_m_labCode");

    m_labHis->setFont(QFont("SourceHanSansCN-Light", 40, 15));
    m_labBud->setFont(QFont("SourceHanSansCN-Light", 40, 15));
    m_labCur->setFont(QFont("SourceHanSansCN-Normal", 48, 15));
    m_labHis->setMinimumHeight(PROGRAM_HIS_HEIGHT);
    m_labBud->setMinimumHeight(PROGRAM_BUDGET_HEIGHT);
    m_labCur->setMinimumHeight(PROGRAM_CUR_HEIGHT);

    m_labHis->setAlignment(Qt::AlignTop | Qt::AlignRight);
    m_labBud->setAlignment(Qt::AlignBottom | Qt::AlignRight);
    m_labCur->setAlignment(Qt::AlignRight);
    m_labCode->setAlignment(Qt::AlignBottom | Qt::AlignRight);

    m_labCode->setFixedWidth(PROGRAM_CODE_WIDTH);

    clearLab();

    if (WidgetStyle::ThemeColor::LIGHT == WidgetStyle::themeColor) {
        setLightUI();
    } else if (WidgetStyle::ThemeColor::DARK == WidgetStyle::themeColor) {
        setDarkUI();
    }

}

void ProgramDisplay::initLayout()
{
    m_vlayout = new QVBoxLayout();
    m_vlayout->addWidget(m_labHis);
    m_vlayout->addWidget(m_labBud);
    m_vlayout->addWidget(m_labCur);
    m_vlayout->setSpacing(0);
    m_vlayout->setMargin(0);

    m_hlayout = new QHBoxLayout();
    m_hlayout->addWidget(m_labCode);
    m_hlayout->addLayout(m_vlayout);
    m_hlayout->setStretchFactor(m_labCode, 1);
    m_hlayout->setStretchFactor(m_vlayout, 11);
    m_hlayout->setSpacing(0);
    m_hlayout->setMargin(0);

    this->setLayout(m_hlayout);
}

int ProgramDisplay::fontSize(QLabel *lab)
{

    /* 数字过长字号缩小 */
    QFont labFont = lab->font();
    QFontMetrics fontMts(labFont);
    int dif = fontMts.width(lab->text()) - lab->width();
    int fontSize = lab->fontInfo().pixelSize();

    while (dif >= -6) {
        labFont.setPixelSize(labFont.pixelSize() - 1);
        QFontMetrics fontMts(labFont);
        dif = fontMts.width(lab->text()) - lab->width();
        fontSize = labFont.pixelSize() - 1;
        if (fontSize < 14) {
            return -1;
        }
    }

    return fontSize;
}
