/* Copyright (C) 2020, KylinSoft Co., Ltd.
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include "resizeimagedialog.h"
#include <QIntValidator>
#include <QSignalBlocker>
#include <QRegExpValidator>

ResizeImageDialog::ResizeImageDialog(QWidget *parent): kdk::KDialog(parent)
{
    this->setFixedSize(324, 265);
    this->setWindowIcon(QIcon::fromTheme("kylin-photo-viewer"));
    this->setWindowTitle(tr("Adjust the image size"));

    initComponents();
    initConnect();
}

void ResizeImageDialog::initComponents()
{
    m_mainLayout = new QVBoxLayout();

    QFrame *frame1 = new QFrame(this->mainWidget());
    m_widthLayout = new QHBoxLayout(frame1);
    m_widthLabel = new QLabel(frame1);
    m_widthLabel->setText(tr("Width"));
    //m_widthLabel->setFixedSize(38, 22);
    m_widthValue = new QLineEdit(frame1);
    m_widthValue->setFixedSize(232, 36);
    // 设置数字验证器，只允许输入1-99999的正整数
    QRegExpValidator *widthValidator = new QRegExpValidator(QRegExp("^[1-9][0-9]{0,4}$"), m_widthValue);
    m_widthValue->setValidator(widthValidator);
    m_widthValue->setPlaceholderText(tr("Enter width (1-99999)"));
    m_widthLayout->addWidget(m_widthLabel);
    m_widthLayout->addWidget(m_widthValue);

    QFrame *frame2 = new QFrame(this->mainWidget());
    m_heightLayout = new QHBoxLayout(frame2);
    m_heightLabel = new QLabel(frame2);
    m_heightLabel->setText(tr("Height"));
    //m_heightLabel->setFixedSize(38, 22);
    m_heightValue = new QLineEdit(frame2);
    m_heightValue->setFixedSize(232, 36);
    // 设置数字验证器，只允许输入1-99999的正整数
    QRegExpValidator *heightValidator = new QRegExpValidator(QRegExp("^[1-9][0-9]{0,4}$"), m_heightValue);
    m_heightValue->setValidator(heightValidator);
    m_heightValue->setPlaceholderText(tr("Enter height (1-99999)"));
    m_heightLayout->addWidget(m_heightLabel);
    m_heightLayout->addWidget(m_heightValue);

    m_widthDefaultPalette = m_widthValue->palette();
    m_heightDefaultPalette = m_heightValue->palette();

    QFrame *frame3 = new QFrame(this->mainWidget());
    m_checkBoxLayout = new QHBoxLayout(frame3);
    m_checkBox = new QCheckBox(frame3);
    m_checkBox->setChecked(true);
    m_checkBox->setFixedSize(232, 36);
    m_checkBox->setText(tr("Keep the aspect ratio"));
    m_checkBoxLayout->addStretch();
    m_checkBoxLayout->addWidget(m_checkBox);

    QFrame *frame4 = new QFrame(this->mainWidget());
    m_btnLayout = new QHBoxLayout(frame4);
    m_cancel = new kdk::KPushButton(frame4);
    m_cancel->setText(tr("Cancel"));
    m_cancel->setFixedSize(96, 36);
    m_save = new kdk::KPushButton(frame4);
    m_save->setText(tr("Save as a copy"));
    m_save->setFixedSize(96, 36);
    m_btnLayout->addWidget(m_cancel);
    m_btnLayout->addWidget(m_save);

    m_mainLayout->addWidget(frame1);
    m_mainLayout->addWidget(frame2);
    m_mainLayout->addWidget(frame3);
    m_mainLayout->addWidget(frame4, 0, Qt::AlignRight);
    m_mainLayout->setContentsMargins(12, 6, 12, 20);
    this->mainWidget()->setLayout(m_mainLayout);
}

void ResizeImageDialog::initConnect()
{
    connect(m_widthValue, &QLineEdit::textChanged, this, &ResizeImageDialog::widthChanged);
    connect(m_widthValue, &QLineEdit::textEdited, this, &ResizeImageDialog::onWidthEdited);
    connect(m_heightValue, &QLineEdit::textChanged, this, &ResizeImageDialog::heightChanged);
    connect(m_heightValue, &QLineEdit::textEdited, this, &ResizeImageDialog::onHeightEdited);
    connect(m_cancel, &kdk::KPushButton::clicked, this, &ResizeImageDialog::cancel);
    connect(m_save, &kdk::KPushButton::clicked, this, &ResizeImageDialog::saveAsCopy);
}

void ResizeImageDialog::onWidthEdited(const QString &text)
{
    if (!m_checkBox->isChecked())
    {
        return;
    }
    int w_text = text.toInt();
    if(w_text<=0 || m_origW <= 0 || m_origH <= 0)
    {
        return;
    }
    int h = qMax(1,(int)qRound((double)w_text * (double)m_origH / (double)m_origW));
    QSignalBlocker blocker(m_heightValue);
    m_heightValue->clear();
    m_heightValue->setText(QString::number(h));
    validateInputs();
}

void ResizeImageDialog::onHeightEdited(const QString &text)
{
    if (!m_checkBox->isChecked())
    {
        return;
    }
    int w_text = text.toInt();
    if(w_text<=0 || m_origW <= 0 || m_origH <= 0)
    {
        return;
    }
    int w = qMax(1,(int)qRound((double)w_text * (double)m_origW / (double)m_origH));
    QSignalBlocker blocker(m_widthValue);
    m_widthValue->clear();
    m_widthValue->setText(QString::number(w));
    validateInputs();
}

// 修改后的宽度变化处理函数
void ResizeImageDialog::widthChanged(QString value)
{
    // 调用统一的验证函数
    validateInputs();
}

// 修改后的高度变化处理函数
void ResizeImageDialog::heightChanged(QString value)
{
    // 调用统一的验证函数
    validateInputs();
}

void ResizeImageDialog::validateInputs()
{
    int curWidth = m_widthValue->text().toInt();
    int curHeight = m_heightValue->text().toInt();
    // 检查像素总数是否超过8K分辨率像素总数
    qint64 product = static_cast<qint64>(curWidth) * static_cast<qint64>(curHeight);
    bool exceedsLimit = product > 33177600;
    // 设置文字颜色
    if (exceedsLimit) {
        QPalette warnPalette = m_widthValue->palette();
        warnPalette.setColor(QPalette::Text, Qt::red);
        m_widthValue->setPalette(warnPalette);

        warnPalette = m_heightValue->palette();
        warnPalette.setColor(QPalette::Text, Qt::red);
        m_heightValue->setPalette(warnPalette);
    } else {
        m_widthValue->setPalette(m_widthDefaultPalette);
        m_heightValue->setPalette(m_heightDefaultPalette);
    }

    m_save->setEnabled(curWidth > 0 && curHeight > 0 && !exceedsLimit);
}

void ResizeImageDialog::saveAsCopy()
{
    int curWidth = m_widthValue->text().toInt();
    int curHeight = m_heightValue->text().toInt();
    Interaction::getInstance()->resizeImage(curWidth, curHeight);
    this->close();
}

void ResizeImageDialog::cancel()
{
    this->close();
}

void ResizeImageDialog::setOriginalSize(int width, int height)
{
    m_originalWidth = width;
    m_originalHeight = height;
    m_origW = width;
    m_origH = height;

    // 初始直接在输入框中显示原图尺寸
    {
        QSignalBlocker blockerW(m_widthValue);
        QSignalBlocker blockerH(m_heightValue);
        m_widthValue->setText(QString::number(m_originalWidth));
        m_heightValue->setText(QString::number(m_originalHeight));
    }

    // 更新占位符：只在用户清空时才看到通用提示
    updatePlaceholders();

    // 设置保存按钮状态
    validateInputs();
}

void ResizeImageDialog::updatePlaceholders()
{
    // 仅在用户清空输入框时显示通用提示
    m_widthValue->setPlaceholderText(tr("Enter width (1-99999)"));
    m_heightValue->setPlaceholderText(tr("Enter height (1-99999)"));
}

void ResizeImageDialog::keyPressEvent(QKeyEvent *event)
{
    if(event && (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)){
        event->accept();
        return;
    }
    kdk::KDialog::keyPressEvent(event);
}

