#include "customcombobox.h"
#include <QStylePainter>
#include <QToolTip>
#include <QApplication>
#include <QTimer>
#include <QCursor>
#include <QEnterEvent>

CustomComboBox::CustomComboBox(QWidget *parent)
    : QComboBox(parent)
{
    setMinimumContentsLength(1); // 重置默认的最小长度限制
    
    // 连接信号槽，当选项改变时更新tooltip
    connect(this, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CustomComboBox::onCurrentIndexChanged);

    setMouseTracking(true);

    // 初始化tooltip
    updateTooltip();
}

void CustomComboBox::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);

    QStylePainter painter(this);

    painter.setPen(palette().color(QPalette::Text));

    // 绘制ComboBox的框架（保持原生样式）
    QStyleOptionComboBox opt;
#ifndef UNIT_TEST
    initStyleOption(&opt);
    painter.drawComplexControl(QStyle::CC_ComboBox, opt);

    // 处理文本显示：裁剪超长文本并添加省略号
    if (currentIndex() >= 0) {

        QString text = itemText(currentIndex());
        QString userData = itemData(currentIndex()).toString();
        if(text.contains(userData) && !userData.isEmpty()){
            text = userData;
        }

        // 使用QStyle获取文本区域的可用宽度
        QRect textRect = style()->subControlRect(QStyle::CC_ComboBox, &opt, QStyle::SC_ComboBoxEditField, this);
        int textWidth = textRect.width();

        // 考虑图标的宽度
        if (itemIcon(currentIndex()).isNull() == false) {
            // 如果有图标，需要减去图标的宽度
            int iconWidth = style()->pixelMetric(QStyle::PM_ButtonIconSize, &opt, this);
            textWidth -= (iconWidth + 6); // 4是图标和文本之间的间距
        }

        // 裁剪文本
        QString elidedText = elideText(text, textWidth);
        // 更新显示文本
        opt.currentText = elidedText;
    }
#endif
    // 绘制文本（不含框架，避免重复绘制）
    painter.drawControl(QStyle::CE_ComboBoxLabel, opt);
}

QString CustomComboBox::elideText(const QString &text, int maxWidth)
{
    if (maxWidth <= 0)
        return text;

    // 创建字体度量对象，计算文本宽度
    QFontMetrics fm(font());
    // 裁剪文本，超出部分用省略号代替
    return fm.elidedText(text, Qt::ElideRight, maxWidth);
}

bool CustomComboBox::isTextElided(const QString &text, int maxWidth) const
{
    if (maxWidth <= 0)
        return false;
    
    QFontMetrics fm(font());
    QString elided = fm.elidedText(text, Qt::ElideRight, maxWidth);
    return elided != text;
}

void CustomComboBox::updateTooltip()
{
    if (currentIndex() >= 0) {
        m_tooltipContent = itemText(currentIndex());

        QString text = itemText(currentIndex());
        QString userData = itemData(currentIndex()).toString();
        if(text.contains(userData) && !userData.isEmpty()){
            m_tooltipContent = userData;
        }
        
        // 获取实际可用宽度（减去下拉箭头的宽度）
        QStyleOptionComboBox opt;
        initStyleOption(&opt);

        // 使用QStyle获取文本区域的可用宽度
        QRect textRect = style()->subControlRect(QStyle::CC_ComboBox, &opt, QStyle::SC_ComboBoxEditField, this);
        int availableWidth = textRect.width();

        if (itemIcon(currentIndex()).isNull() == false) {
            int iconWidth = style()->pixelMetric(QStyle::PM_ButtonIconSize, &opt, this);
            availableWidth -= (iconWidth + 6); // 4是图标和文本之间的间距
        }
        
        // 检查当前文本是否会被裁剪
        if (!isTextElided(m_tooltipContent, availableWidth)) {
            m_tooltipContent = QString();
        }
    } else {
        m_tooltipContent = QString();
    }
}

void CustomComboBox::onCurrentIndexChanged(int index)
{
    Q_UNUSED(index);
    updateTooltip();
}

void CustomComboBox::showTooltip()
{
    m_tooltipShown = false;
    // 鼠标进入时检查是否需要显示tooltip
    updateTooltip();
    // 延迟显示tooltip，避免误触
    QTimer::singleShot(500, this, [this]() {
        if (!m_tooltipShown && underMouse() && !m_tooltipContent.isEmpty()) {
            // 计算tooltip显示位置，确保显示在QComboBox正下方
            QPoint tooltipPos = mapToGlobal(QPoint(0, height()));
            QToolTip::showText(tooltipPos, m_tooltipContent, this);
            m_tooltipShown = true;
        }
    });
}

void CustomComboBox::hideTooltip()
{
    QToolTip::hideText();
    m_tooltipShown = false;
}

bool CustomComboBox::event(QEvent *event)
{
    if (event->type() == QEvent::Enter)
    {
        showTooltip();
        return true;
    }
    else if (event->type() == QEvent::Leave)
    {
        hideTooltip();
        return true;
    }
    return QComboBox::event(event);
}

