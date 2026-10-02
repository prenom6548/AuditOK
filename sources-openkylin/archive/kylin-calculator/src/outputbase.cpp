#include "outputbase.h"
#include "data_warehouse.h"
#include <QDebug>

OutputBase::OutputBase(QWidget *parent) : QWidget(parent)
{
    this->setObjectName(OUTPUTBASE_OBJECT_NAME);
    // 历史记录
    this->staLabLast = new QLabel(this);
    staLabLast->setAccessibleName("kylin-calculator_QLabel_staLabLast");
    // 预测输出
    this->staLabPre = new QLabel(this);
    staLabPre->setAccessibleName("kylin-calculator_QLabel_staLabPre");
    // 当前输入
    this->staLabNow = new QLabel(this);
    staLabNow->setAccessibleName("kylin-calculator_QLabel_staLabNow");
    //当前输入控件布局
    this->staLabNowLeft = new QPushButton(this);
    staLabNowLeft->setAccessibleName("kylin-calculator_QPushButton_staLabNowLeft");
    connect(this->staLabNowLeft, &QPushButton::clicked, this, &OutputBase::onStaLabNowLeftClick);
    this->staLabNowLeft->setFixedSize(36, 36);
    this->staLabNowLeft->setIcon(QIcon::fromTheme("ukui-start.symbolic"));
    this->staLabNowLeft->hide();
    this->staLabNowRight = new QPushButton(this);
    staLabNowRight->setAccessibleName("kylin-calculator_QPushButton_staLabNowRight");
    connect(this->staLabNowRight, &QPushButton::clicked, this, &OutputBase::onStaLabNowRightClick);
    this->staLabNowRight->setFixedSize(36, 36);
    this->staLabNowRight->setIcon(QIcon::fromTheme("ukui-end.symbolic"));
    this->staLabNowRight->hide();
    this->staLabNowLayout = new QHBoxLayout();
    this->staLabNowLayout->setMargin(0);
    this->staLabNowLayout->addWidget(this->staLabNowLeft);
    this->staLabNowLayout->addWidget(this->staLabNow);
    this->staLabNowLayout->addWidget(this->staLabNowRight);

    //    this->staLabLast->setBackgroundRole(QPalette::Dark);
    //    this->staLabLast->setAutoFillBackground(true);
    //    staLabLast->frameWidth();
    //    this->staLabNow->setBackgroundRole(QPalette::Highlight);
    //    this->staLabNow->setAutoFillBackground(true);
}

void OutputBase::setHistoryText(const QString &text)
{
    if (text.isEmpty()) {
        this->staLabLast->setText(text);
        return;
    }
    // TODO:label内边距不是frameStyle()
    int realWidth = this->staLabLast->width() - this->staLabLast->frameStyle();
    QString showText;
    QStringList list = text.split("\n");
    for (const QString &str : list) {
        if (QFontMetrics(this->staLabLast->font()).width(str) > realWidth) {
            QStringList equation = str.split("=");
            QString result = equation.last();
            QString test = equation.first();
            test.chop(1);
            while (!test.isEmpty() && (QFontMetrics(this->staLabLast->font()).width(test + "...=" + result) > realWidth)) {
                test.chop(1);
            }
            showText += test + "...=" + result + "\n";
        } else {
            showText += str + "\n";
        }
    }
    showText.chop(1);
    this->staLabLast->setText(showText);
}

void OutputBase::setNowText(const QString &text)
{
    staLabNowLeft->hide();
    staLabNowRight->hide();
    if (staLabNow->font().pixelSize() > MIN_FONT_SIZE) {
        this->staLabNow->setText(text);
        return;
    }
    m_preIsLeft = true;
    m_indexList.clear();
    m_listIndex = 0;
    int realWidth = this->staLabNow->width() - this->staLabNow->frameStyle();
    if (QFontMetrics(this->staLabNow->font()).width(text) > realWidth) {
        bool isPreNotNumber = true;
        for (int i = 0; i < text.length(); i++) {
            if (text.at(i).isNumber() || text.at(i) == ',') {
                if (isPreNotNumber) {
                    m_indexList.append(i);
                }
                isPreNotNumber = false;
            } else {
                isPreNotNumber = true;
            }
        }
        staLabNowLeft->show();
        m_nowTextString = text;
        QString tmpText = text;
        int showHeadIndex = 0;
        //能显示出来的数字位置
        while (QFontMetrics(this->staLabNow->font()).width(tmpText) > realWidth) {
            tmpText.remove(0, 1);
            showHeadIndex++;
        }
        //该数字在队列中的位置
        for (int i = 0; i < m_indexList.length(); i++) {
            if (m_indexList.at(i) > showHeadIndex) {
                m_listIndex = i - 1;
                break;
            }
        }
    }
    if (DataWarehouse::getInstance()->platform == QString("intel")) {
        this->staLabNow->setAlignment(Qt::AlignRight | Qt::AlignBottom);
    } else {
        this->staLabNow->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }
    this->staLabNow->setText(text);
}

void OutputBase::onStaLabNowLeftClick()
{
    this->staLabNowRight->show();
    if (!m_preIsLeft) {
        //计算左侧数字位置
        QString tmpText = this->staLabNow->text();
        int realWidth = this->staLabNow->width() - this->staLabNow->frameStyle();
        int showHeadIndex = 0;
        while (QFontMetrics(this->staLabNow->font()).width(tmpText) > realWidth) {
            tmpText.remove(0, 1);
            showHeadIndex++;
        }
        for (int i = 0; i < m_indexList.length(); i++) {
            if (m_indexList.at(i) > showHeadIndex) {
                m_listIndex = i - 1;
                break;
            }
        }
    }
    if (m_listIndex > 0) {
        m_listIndex--;
    } else {
        m_listIndex = 0;
    }
    if (DataWarehouse::getInstance()->platform == QString("intel")) {
        this->staLabNow->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
    } else {
        this->staLabNow->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    }
    int strIndex = m_indexList.at(m_listIndex);
    if (strIndex == 0) {
        this->staLabNowLeft->hide();
    } else {
        this->staLabNowLeft->show();
    }
    QString showText = m_nowTextString.mid(strIndex);
    this->staLabNow->setText(showText);
    m_preIsLeft = true;
}

void OutputBase::onStaLabNowRightClick()
{
    this->staLabNowLeft->show();
    if (m_preIsLeft) {
        //计算右侧数字位置
        QString tmpText = this->staLabNow->text();
        int realWidth = this->staLabNow->width() - this->staLabNow->frameStyle();
        int showLastIndex = m_nowTextString.length() - 1;
        while (QFontMetrics(this->staLabNow->font()).width(tmpText) > realWidth) {
            tmpText.chop(1);
            showLastIndex--;
        }
        for (int i = 0; i < m_indexList.length(); i++) {
            if (m_indexList.at(i) > showLastIndex) {
                m_listIndexR = i;
                break;
            }
        }
    }
    if (m_listIndexR < m_indexList.length() - 1) {
        m_listIndexR++;
    } else {
        m_listIndexR = m_indexList.length() - 1;
    }
    if (DataWarehouse::getInstance()->platform == QString("intel")) {
        this->staLabNow->setAlignment(Qt::AlignRight | Qt::AlignBottom);
    } else {
        this->staLabNow->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }
    int strIndex = m_indexList.at(m_listIndexR);
    int strIndexR = strIndex;
    for (int i = strIndex + 1; i < m_nowTextString.length(); i++) {
        if (m_nowTextString.at(i).isNumber() || m_nowTextString.at(i) == ',') {
            strIndexR++;
        } else {
            break;
        }
    }
    if (strIndexR == m_nowTextString.length() - 1) {
        this->staLabNowRight->hide();
    } else {
        this->staLabNowRight->show();
    }
    QString showText = m_nowTextString.mid(0, strIndexR + 1);
    this->staLabNow->setText(showText);
    m_preIsLeft = false;
}
