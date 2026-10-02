/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "colordialog.h"
#include "maskwidget.h"
#include <QDebug>
#include <QFontMetrics>
#include <QTimer>
#include <kysdk/applications/accessinfohelper.h>

using namespace kdk;

ColorDialog::ColorDialog(QWidget *parent) :
    KDialog(parent)
{
    initUi();
    initSetup();
    signalsBind();
}

ColorDialog::~ColorDialog()
{
    qDebug() << "this is color destructor:";
}

void ColorDialog::initUi()
{
    setWindowIcon(QStringLiteral("ukui-control-center"));
    setFixedWidth(DIALOG_WIDTH);

    QFrame *colorPickerFrame = new QFrame(this);
    colorPickerFrame->setMinimumHeight(328);
    colorPickerFrame->setFrameShape(QFrame::StyledPanel);
    colorPickerFrame->setFrameShadow(QFrame::Raised);
    colorPickerFrame->setLineWidth(0);

    m_colorPickerHBoxLayout = new QHBoxLayout();
    m_colorPickerHBoxLayout->setSpacing(24);
    m_colorPickerHBoxLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *colorPickerLayout = new QHBoxLayout(colorPickerFrame);
    colorPickerLayout->setSpacing(0);
    colorPickerLayout->setContentsMargins(0, 0, 0, 0);
    colorPickerLayout->addLayout(m_colorPickerHBoxLayout);

    // 色相滑块须在 HEX 等输入框之前创建，保证 Tab：关闭 → slider → HEX…
    gradientSlider = new GradientSlider(this);
    KDK_EXTEND_ALL_INFO_FORMAT(gradientSlider, "Wallpaper", "gradientSlider", "gradientSlider of ColorDialog");
    gradientSlider->setOrientation(Qt::Vertical);
    gradientSlider->setFocusPolicy(Qt::StrongFocus);
    // KDK 后再设，读屏报「色相」而非物件名
    gradientSlider->setAccessibleName(tr("色相"));

    colorSquare = new ColorSquare(this);
    KDK_EXTEND_ALL_INFO_FORMAT(colorSquare, "Wallpaper", "", "");
    QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    colorSquare->setSizePolicy(sizePolicy);
    // 色板仅鼠标点选，不插入关闭与滑块之间的 Tab 链
    colorSquare->setFocusPolicy(Qt::NoFocus);

    MaskWidget *maskWidget = new MaskWidget(colorSquare);
    maskWidget->setGeometry(0, 0, colorSquare->width(), colorSquare->height());

    m_colorPickerHBoxLayout->addWidget(colorSquare);
    m_colorPickerHBoxLayout->addWidget(gradientSlider);

    colorPreview = new ColorPreview(this);
    colorPreview->setFixedSize(80, 80);
    colorPreview->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_hexLabel = new KLabel(this);
    m_hexLabel->setText(tr("HEX"));

    m_colorLineEdit = new QLineEdit(this);
    m_colorLineEdit->setMinimumHeight(36);
    m_colorLineEdit->setMinimumWidth(0);
    m_colorLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_colorLineEdit->setFocusPolicy(Qt::StrongFocus);
    m_colorLineEdit->setAlignment(Qt::AlignCenter);

    m_hexHBoxLayout = new QHBoxLayout();
    m_hexHBoxLayout->setSpacing(4);
    m_hexHBoxLayout->setContentsMargins(0, 0, 0, 0);
    m_hexHBoxLayout->addWidget(m_hexLabel, 0);
    m_hexHBoxLayout->addWidget(m_colorLineEdit, 1);

    m_rgbLabel = new KLabel(this);
    m_rgbLabel->setText(tr("RGB"));

    m_spinBoxR = new QSpinBox(this);
    m_spinBoxB = new QSpinBox(this);
    m_spinBoxG = new QSpinBox(this);

    const QList<QSpinBox *> rgbSpinBoxes = {m_spinBoxR, m_spinBoxB, m_spinBoxG};
    for (QSpinBox *spinBox : rgbSpinBoxes) {
        spinBox->setMinimumHeight(36);
        spinBox->setMinimumWidth(0);
        spinBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        spinBox->setAlignment(Qt::AlignCenter);
        spinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
    }

    m_rgbHBoxLayout = new QHBoxLayout();
    m_rgbHBoxLayout->setSpacing(4);
    m_rgbHBoxLayout->setContentsMargins(0, 0, 0, 0);
    m_rgbHBoxLayout->addWidget(m_rgbLabel, 0);
    m_rgbHBoxLayout->addWidget(m_spinBoxR, 1);
    m_rgbHBoxLayout->addWidget(m_spinBoxB, 1);
    m_rgbHBoxLayout->addWidget(m_spinBoxG, 1);

    QVBoxLayout *inputRowsLayout = new QVBoxLayout();
    inputRowsLayout->setSpacing(8);
    inputRowsLayout->addLayout(m_hexHBoxLayout);
    inputRowsLayout->addLayout(m_rgbHBoxLayout);

    m_inputHBoxLayout = new QHBoxLayout();
    m_inputHBoxLayout->setSpacing(16);
    m_inputHBoxLayout->setContentsMargins(0, 0, 0, 0);
    m_inputHBoxLayout->addWidget(colorPreview, 0);
    m_inputHBoxLayout->addLayout(inputRowsLayout, 1);

    QFrame *inputFrame = new QFrame(this);
    inputFrame->setFrameShape(QFrame::StyledPanel);
    inputFrame->setFrameShadow(QFrame::Raised);
    inputFrame->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    QHBoxLayout *inputFrameLayout = new QHBoxLayout(inputFrame);
    inputFrameLayout->setContentsMargins(0, 0, 0, 0);
    inputFrameLayout->addLayout(m_inputHBoxLayout);

    m_cancelBtn = new KPushButton(this);
    m_cancelBtn->setText(tr("Cancel"));
    m_cancelBtn->setFixedSize(96, 36);
    // 需可 Tab 到「取消」；勿设 NoFocus
    m_cancelBtn->setFocusPolicy(Qt::StrongFocus);
    m_cancelBtn->setAutoDefault(false);

    m_okBtn = new KPushButton(this);
    m_okBtn->setText(tr("OK"));
    m_okBtn->setFixedSize(96, 36);
    // 避免对话框打开时默认焦点落在「确定」
    m_okBtn->setAutoDefault(false);
    m_okBtn->setDefault(false);
    m_okBtn->setFocusPolicy(Qt::StrongFocus);

    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->setSpacing(16);
    bottomLayout->addStretch();
    bottomLayout->addWidget(m_cancelBtn);
    bottomLayout->addWidget(m_okBtn);

    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setContentsMargins(24, 12, 24, 24);
    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(colorPickerFrame);
    m_mainLayout->addSpacing(16);
    m_mainLayout->addWidget(inputFrame);
    m_mainLayout->addSpacing(32);
    m_mainLayout->addLayout(bottomLayout);

    mainWidget()->setLayout(m_mainLayout);
}

void ColorDialog::initSetup()
{
    setWindowTitle(tr("Choose a custom color"));
    setAttribute(Qt::WA_DeleteOnClose);

    spinAlpha = new QSpinBox(this);
    spinAlpha->setVisible(false);
    spinAlpha->setMaximum(100);
    sliderAlpha = new GradientSlider(this);
    sliderAlpha->setVisible(false);
    sliderAlpha->setMaximum(100);
    sliderAlpha->setOrientation(Qt::Horizontal);

    spinHue = new QSpinBox(this);
    spinHue->setVisible(false);
    spinHue->setMaximum(359);
    sliderHue = new GradientSlider(this);
    sliderHue->setVisible(false);
    sliderHue->setMaximum(359);
    sliderHue->setOrientation(Qt::Horizontal);

    QVector<QColor> rainbow;
    for (int i = 0; i < 360; i += 360 / 6) {
        rainbow.push_back(QColor::fromHsv(i, 255, 255));
    }
    rainbow.push_back(Qt::red);
    sliderHue->setColors(rainbow);

    spinSat = new QSpinBox(this);
    spinSat->setVisible(false);
    spinSat->setMaximum(100);
    sliderSat = new GradientSlider(this);
    sliderSat->setVisible(false);
    sliderSat->setMaximum(100);
    sliderSat->setOrientation(Qt::Horizontal);

    spinVal = new QSpinBox(this);
    spinVal->setVisible(false);
    spinVal->setMaximum(100);
    sliderVal = new GradientSlider(this);
    sliderVal->setVisible(false);
    sliderVal->setMaximum(100);
    sliderVal->setOrientation(Qt::Horizontal);

    spinRed = m_spinBoxR;
    spinRed->setMaximum(255);
    sliderRed = new GradientSlider(this);
    sliderRed->setVisible(false);
    sliderRed->setMaximum(255);
    sliderRed->setOrientation(Qt::Horizontal);

    spinGreen = m_spinBoxG;
    spinGreen->setMaximum(255);
    sliderGreen = new GradientSlider(this);
    sliderGreen->setVisible(false);
    sliderGreen->setMaximum(255);
    sliderGreen->setOrientation(Qt::Horizontal);

    spinBlue = m_spinBoxB;
    spinBlue->setMaximum(255);
    sliderBlue = new GradientSlider(this);
    sliderBlue->setVisible(false);
    sliderBlue->setMaximum(255);
    sliderBlue->setOrientation(Qt::Horizontal);

    KDK_EXTEND_ALL_INFO_FORMAT(m_hexLabel, "Wallpaper", "customColorHEXLabel", "HEX label of ColorDialog");
    KDK_EXTEND_ALL_INFO_FORMAT(m_rgbLabel, "Wallpaper", "customColorRGBLabel", "RGB label of ColorDialog");
    KDK_EXTEND_ALL_INFO_FORMAT(m_colorLineEdit, "Wallpaper", "customColorLineEdit", "lineedit of ColorDialog");
    KDK_EXTEND_ALL_INFO_FORMAT(m_spinBoxR, "Wallpaper", "customColorRSpinBox", "R spinBox of ColorDialog");
    KDK_EXTEND_ALL_INFO_FORMAT(m_spinBoxG, "Wallpaper", "customColorGSpinBox", "G spinBox of ColorDialog");
    KDK_EXTEND_ALL_INFO_FORMAT(m_spinBoxB, "Wallpaper", "customColorBSpinBox", "B spinBox of ColorDialog");
    KDK_EXTEND_ALL_INFO_FORMAT(colorPreview, "Wallpaper", "customColorPreview", "ColorPreview of ColorDialog");
    KDK_EXTEND_ALL_INFO_FORMAT(m_cancelBtn, "Wallpaper", "customColorCancelBtn", "cancel button of ColorDialog");
    KDK_EXTEND_ALL_INFO_FORMAT(m_okBtn, "Wallpaper", "customColorOkBtn", "ok button of ColorDialog");

    m_hexLabel->setAccessibleName(tr("HEX"));
    m_hexLabel->setBuddy(m_colorLineEdit);
    m_colorLineEdit->setAccessibleName(tr("HEX"));
    m_rgbLabel->setAccessibleName(tr("RGB"));
    m_rgbLabel->setBuddy(m_spinBoxR);
    m_spinBoxR->setAccessibleName(tr("RGB 红"));
    m_spinBoxG->setAccessibleName(tr("RGB 绿"));
    m_spinBoxB->setAccessibleName(tr("RGB 蓝"));
    m_cancelBtn->setAccessibleName(tr("Cancel"));
    m_okBtn->setAccessibleName(tr("OK"));

    updateInputLayout();
    drawSlider();
    // 须在样式 polish / 子控件建完后设置，避免滑块被刷成 ClickFocus 后 Tab 先落到 HEX
    setupDialogTabOrder();
}

void ColorDialog::setupDialogTabOrder()
{
    if (!gradientSlider || !m_colorLineEdit) {
        return;
    }
    gradientSlider->setFocusPolicy(Qt::StrongFocus);
    if (QPushButton *closeBtn = closeButton()) {
        closeBtn->setFocusPolicy(Qt::StrongFocus);
        QWidget::setTabOrder(closeBtn, gradientSlider);
    }
    QWidget::setTabOrder(gradientSlider, m_colorLineEdit);
    QWidget::setTabOrder(m_colorLineEdit, m_spinBoxR);
    QWidget::setTabOrder(m_spinBoxR, m_spinBoxB);
    QWidget::setTabOrder(m_spinBoxB, m_spinBoxG);
    QWidget::setTabOrder(m_spinBoxG, m_cancelBtn);
    QWidget::setTabOrder(m_cancelBtn, m_okBtn);
}

void ColorDialog::updateInputLayout()
{
    if (!m_mainLayout || mainWidget()->width() <= 0) {
        return;
    }

    const QMargins margins = m_mainLayout->contentsMargins();
    const int contentWidth = mainWidget()->width() - margins.left() - margins.right();
    const int inputAreaWidth = contentWidth - colorPreview->width() - m_inputHBoxLayout->spacing();
    if (inputAreaWidth <= 0) {
        return;
    }

    const QFontMetrics fm(m_hexLabel->font());
    const int labelWidth = qMax(fm.horizontalAdvance(m_hexLabel->text()),
                                fm.horizontalAdvance(m_rgbLabel->text())) + 8;
    m_hexLabel->setFixedWidth(qMin(labelWidth, inputAreaWidth / 3));
    m_rgbLabel->setFixedWidth(m_hexLabel->width());
    m_hexLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_rgbLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
}

void ColorDialog::resizeEvent(QResizeEvent *event)
{
    KDialog::resizeEvent(event);
    updateInputLayout();
}

void ColorDialog::showEvent(QShowEvent *event)
{
    KDialog::showEvent(event);
    // 进入对话框先落在标题栏关闭；Tab 再按 setupDialogTabOrder：关闭 → slider → HEX…
    QTimer::singleShot(0, this, [this]() {
        setupDialogTabOrder();
        if (QPushButton *closeBtn = closeButton()) {
            closeBtn->setFocus(Qt::OtherFocusReason);
        }
    });
}

void ColorDialog::signalsBind()
{
    connect(m_cancelBtn, &KPushButton::clicked, this, &ColorDialog::close);
    connect(m_okBtn, &KPushButton::clicked, this, &ColorDialog::okSlot);

    connect(sliderAlpha, &GradientSlider::valueChanged, spinAlpha, &QSpinBox::setValue);
    connect(spinAlpha, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            sliderAlpha, &GradientSlider::setValue);
    connect(sliderHue, &GradientSlider::valueChanged, spinHue, &QSpinBox::setValue);
    connect(spinHue, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            sliderHue, &GradientSlider::setValue);
    connect(sliderSat, &GradientSlider::valueChanged, spinSat, &QSpinBox::setValue);
    connect(spinSat, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            sliderSat, &GradientSlider::setValue);
    connect(sliderVal, &GradientSlider::valueChanged, spinVal, &QSpinBox::setValue);
    connect(spinVal, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            sliderVal, &GradientSlider::setValue);
    connect(sliderRed, &GradientSlider::valueChanged, spinRed, &QSpinBox::setValue);
    connect(spinRed, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            sliderRed, &GradientSlider::setValue);
    connect(sliderGreen, &GradientSlider::valueChanged, spinGreen, &QSpinBox::setValue);
    connect(spinGreen, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            sliderGreen, &GradientSlider::setValue);
    connect(sliderBlue, &GradientSlider::valueChanged, spinBlue, &QSpinBox::setValue);
    connect(spinBlue, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            sliderBlue, &GradientSlider::setValue);

    connect(sliderHue, &GradientSlider::valueChanged, this, &ColorDialog::SetHsvSlot);
    connect(sliderSat, &GradientSlider::valueChanged, this, &ColorDialog::SetHsvSlot);
    connect(sliderVal, &GradientSlider::valueChanged, this, &ColorDialog::SetHsvSlot);
    connect(sliderRed, &GradientSlider::valueChanged, this, &ColorDialog::setRgbSlot);
    connect(sliderGreen, &GradientSlider::valueChanged, this, &ColorDialog::setRgbSlot);
    connect(sliderBlue, &GradientSlider::valueChanged, this, &ColorDialog::setRgbSlot);

    connect(sliderAlpha, &GradientSlider::valueChanged, this, &ColorDialog::updateWidgetsSlot);
    connect(colorSquare, &ColorSquare::colorSelected, this, &ColorDialog::updateWidgetsSlot);
    connect(this, &ColorDialog::checkedChanged, colorSquare, &ColorSquare::setCheckedColorSlot);

    m_colorLineEdit->setText(QStringLiteral("#000000"));
    connect(m_colorLineEdit, &QLineEdit::textChanged, this, [=]() {
        QColor mcolor;
        mcolor.setNamedColor(m_colorLineEdit->text());
        m_spinBoxR->setValue(mcolor.red());
        m_spinBoxB->setValue(mcolor.blue());
        m_spinBoxG->setValue(mcolor.green());
    });
}

void ColorDialog::drawSlider()
{
    QVector<QColor> rainbow;
    for (int i = 0; i < 360; i += 360 / 6) {
        rainbow.push_front(QColor::fromHsv(i, 255, 255));
    }
    rainbow.push_front(Qt::red);
    gradientSlider->setMaximum(sliderHue->maximum());
    gradientSlider->setColors(rainbow);
    gradientSlider->setValue(sliderHue->value());
    connect(gradientSlider, &GradientSlider::valueChanged, sliderHue, &GradientSlider::setValue);
    emit checkedChanged('H');
}

QColor ColorDialog::color() const
{
    return colorSquare->color();
}

void ColorDialog::SetColor(QColor color)
{
    colorSquare->setColor(color);
    sliderAlpha->setValue(color.alpha() / 2.55);
    updateWidgetsSlot();
}

void ColorDialog::okSlot()
{
    emit colorSelected(color());
    close();
}

void ColorDialog::setRgbSlot()
{
    if (!signalsBlocked()) {
        QColor color(sliderRed->value(), sliderGreen->value(), sliderBlue->value());
        if (color.saturation() == 0) {
            color = QColor::fromHsv(sliderHue->value(), 0, color.value());
        }
        colorSquare->setColor(color);
        updateWidgetsSlot();
    }
}

void ColorDialog::SetHsvSlot()
{
    if (!signalsBlocked()) {
        QColor color = QColor::fromHsv(gradientSlider->value() * 360 / gradientSlider->maximum(), 100, 100);
        colorSquare->setColor(color);
        updateWidgetsSlot();
    }
}

void ColorDialog::updateWidgetsSlot()
{
    blockSignals(true);
    foreach (QWidget *w, findChildren<QWidget *>()) {
        w->blockSignals(true);
    }

    QColor col = color();

    sliderRed->setValue(col.red());
    spinRed->setValue(sliderRed->value());
    sliderRed->setFirstColor(QColor(0, col.green(), col.blue()));
    sliderRed->setLastColor(QColor(255, col.green(), col.blue()));

    sliderGreen->setValue(col.green());
    spinGreen->setValue(sliderGreen->value());
    sliderGreen->setFirstColor(QColor(col.red(), 0, col.blue()));
    sliderGreen->setLastColor(QColor(col.red(), 255, col.blue()));

    sliderBlue->setValue(col.blue());
    spinBlue->setValue(sliderBlue->value());
    sliderBlue->setFirstColor(QColor(col.red(), col.green(), 0));
    sliderBlue->setLastColor(QColor(col.red(), col.green(), 255));

    sliderHue->setValue(qRound(colorSquare->hue() * 360.0));
    spinHue->setValue(sliderHue->value());

    sliderSat->setValue(qRound(colorSquare->saturation() * 100.0));
    spinSat->setValue(sliderSat->value());
    sliderSat->setFirstColor(QColor::fromHsvF(colorSquare->hue(), 0, colorSquare->value()));
    sliderSat->setLastColor(QColor::fromHsvF(colorSquare->hue(), 1, colorSquare->value()));

    sliderVal->setValue(qRound(colorSquare->value() * 100.0));
    spinVal->setValue(sliderVal->value());
    sliderVal->setFirstColor(QColor::fromHsvF(colorSquare->hue(), colorSquare->saturation(), 0));
    sliderVal->setLastColor(QColor::fromHsvF(colorSquare->hue(), colorSquare->saturation(), 1));

    colorPreview->setColor(col);
    if (!m_colorLineEdit->hasFocus()) {
        m_colorLineEdit->blockSignals(true);
        m_colorLineEdit->setText(col.name().toUpper());
        m_colorLineEdit->blockSignals(false);
    }
    SetVerticalSlider();

    blockSignals(false);
    foreach (QWidget *w, findChildren<QWidget *>()) {
        w->blockSignals(false);
    }

    emit colorChanged(col);
}

void ColorDialog::SetVerticalSlider()
{
    disconnect(gradientSlider, SIGNAL(valueChanged(int)), sliderHue, SLOT(setValue(int)));
    disconnect(gradientSlider, SIGNAL(valueChanged(int)), sliderSat, SLOT(setValue(int)));
    disconnect(gradientSlider, SIGNAL(valueChanged(int)), sliderVal, SLOT(setValue(int)));
    disconnect(gradientSlider, SIGNAL(valueChanged(int)), sliderRed, SLOT(setValue(int)));
    disconnect(gradientSlider, SIGNAL(valueChanged(int)), sliderGreen, SLOT(setValue(int)));
    disconnect(gradientSlider, SIGNAL(valueChanged(int)), sliderBlue, SLOT(setValue(int)));

    QVector<QColor> rainbow;
    for (int i = 0; i < 360; i += 360 / 6) {
        rainbow.push_front(QColor::fromHsv(i, 255, 255));
    }
    rainbow.push_front(Qt::red);
    gradientSlider->setMaximum(sliderHue->maximum());
    gradientSlider->setColors(rainbow);
    gradientSlider->setValue(sliderHue->value());

    connect(gradientSlider, SIGNAL(valueChanged(int)), sliderHue, SLOT(setValue(int)));
    emit checkedChanged('H');
}

void ColorDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        m_okBtn->click();
    } else if (event->key() == Qt::Key_Escape) {
        m_cancelBtn->click();
    } else {
        KDialog::keyPressEvent(event);
    }
}
