#include "diskitemwidget.h"
#include "chartviewwidget.h"

#include <QPen>
#include <QPainter>
#include <QPalette>
#include <QtMath>
#include <QApplication>
#include "commoninfo.h"

const int margin = 6;
const int spacing = 6;
const int sectionSize = 6;
const int TextSpacing = 12;

DiskItemWidget::DiskItemWidget(QWidget *parent) : QWidget(parent)
{
    m_pDiskChartWidget = new ChartViewWidget(ChartViewWidget::ChartViewTypes::MEM_CHART, this);
    m_pDiskChartWidget->setData1Color(m_readColor);
    m_pDiskChartWidget->setData2Color(m_writeColor);
}

DiskItemWidget::~DiskItemWidget()
{
    if (m_pDiskChartWidget) {
        delete m_pDiskChartWidget;
        m_pDiskChartWidget = nullptr;
    }
}

void DiskItemWidget::updateWidgetGeometry()
{
    QFont font = QApplication::font();
    font.setPointSizeF(font.pointSizeF() - 1);
    int fontHeight = QFontMetrics(font).height();
    int curXMargin = margin;
    m_pDiskChartWidget->setGeometry(curXMargin, fontHeight * 2 + TextSpacing, this->width() - 2 * curXMargin, this->height() - fontHeight * 2 - TextSpacing - margin);
}

void DiskItemWidget::fontChanged(const QFont &font)
{
    m_font = font;
    updateWidgetGeometry();
}

void DiskItemWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateWidgetGeometry();
}

void DiskItemWidget::updateData(const QString &deviceName)
{
    m_deviceName = deviceName;

    double readSpeed = DiskSpeedInfo::instance()->readSpeed(deviceName);
    double writeSpeed = DiskSpeedInfo::instance()->writeSpeed(deviceName);

    m_pDiskChartWidget->updateChartDate(writeSpeed, readSpeed);

    this->update();
}

void DiskItemWidget::activeItemWidget(bool isShow)
{
    m_isActive = isShow;
    update();
}

void DiskItemWidget::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    emit clicked(m_deviceName);
}

void DiskItemWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    int curXMargin = margin;
    QPainter painter(this);
    QFont font = QApplication::font();
    font.setPointSizeF(font.pointSizeF() - 1);
    painter.setFont(font);
    painter.setRenderHint(QPainter:: Antialiasing, true);

    const QPalette &palette = QPalette();

    int deviceNameWidth = painter.fontMetrics().width(m_deviceName);
    int deviceNameHeight = painter.fontMetrics().height();
    QRect devtitleRect(curXMargin, margin, deviceNameWidth, deviceNameHeight);

    const QColor &textColor = palette.color(QPalette::Text);
    if (m_isActive && m_mode == TITLE_VERTICAL) {
        painter.setPen(Qt::NoPen);
        QColor selectColor = palette.color(QPalette::Highlight);
        selectColor.setAlphaF(0.1);

        painter.setBrush(selectColor);
        painter.drawRoundedRect(rect(), 8, 8);

        painter.setPen(palette.color(QPalette::Highlight));
    } else {
        painter.setPen(textColor);
    }
    painter.drawText(devtitleRect, Qt::AlignLeft | Qt::AlignVCenter, m_deviceName);
    painter.setPen(textColor);

    QString readTitle = QString("%1 %2")
                        .arg(tr("Disk Read"))
                        .arg(DiskSpeedInfo::instance()->readSpeedStr(m_deviceName));

    QString writeTitle = QString("%1 %2")
                         .arg(tr("Disk Write"))
                         .arg(DiskSpeedInfo::instance()->writeSpeedStr(m_deviceName));
    int readTitleWidth = painter.fontMetrics().width(readTitle);
    int readTitleHeight = painter.fontMetrics().height();

    painter.setPen(Qt::NoPen);
    painter.setBrush(m_readColor);

    QRect readStrRect(devtitleRect.left() + spacing + sectionSize, devtitleRect.y() + deviceNameHeight, readTitleWidth, readTitleHeight);
    painter.drawEllipse(devtitleRect.left(), readStrRect.y() + qCeil((readStrRect.height() - sectionSize) / 2.0), sectionSize, sectionSize); // 读硬盘速度的颜色提示
    painter.setPen(palette.color(QPalette::Text));
    painter.drawText(readStrRect, Qt::AlignLeft | Qt::AlignVCenter, readTitle);

    painter.setPen(Qt::NoPen);
    painter.setBrush(m_writeColor);

    QRect writeStrRect(devtitleRect.left() + spacing + sectionSize, readStrRect.y() + readTitleHeight, painter.fontMetrics().width(writeTitle), painter.fontMetrics().height());
    painter.drawEllipse(devtitleRect.left(), writeStrRect.y() + qCeil((writeStrRect.height() - sectionSize) / 2.0), sectionSize, sectionSize); // 写硬盘速度的颜色提示
    painter.setPen(palette.color(QPalette::Text));
    painter.drawText(writeStrRect, Qt::AlignLeft | Qt::AlignVCenter, writeTitle);
}

void DiskItemWidget::setMode(int mode)
{
    m_mode = mode;
    this->update();
}

bool DiskItemWidget::isActiveItem()
{
    return  m_isActive;
}
