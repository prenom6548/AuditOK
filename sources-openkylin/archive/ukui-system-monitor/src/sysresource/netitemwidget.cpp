#include "netitemwidget.h"

#include "chartviewwidget.h"
#include "commoninfo.h"
//#include "system/netif.h"

#include <QtMath>
#include <QPainter>

#include <QApplication>

const int margin = 6;
const int spacing = 6;
const int sectionSize = 6;
const int TextSpacing = 12;

//DWIDGET_USE_NAMESPACE
NetItemWidget::NetItemWidget(QWidget *parent, const QString &mac) : QWidget(parent)
{
    m_ifname = mac;

    m_ChartWidget = new ChartViewWidget(ChartViewWidget::ChartViewTypes::NET_CHART, this);
    m_ChartWidget->setData1Color(m_recvColor);
    m_ChartWidget->setData2Color(m_sentColor);
    updateWidgetGeometry();
}

void NetItemWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    int curXMargin = m_mode == TITLE_HORIZONTAL ? 0 : margin;
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setFont(m_font);

    const QPalette &palette = QPalette();

    const QColor &textColor = palette.color(QPalette::Text);
    if (m_isActive) {
        painter.setPen(Qt::NoPen);
        QColor selectColor = palette.color(QPalette::Highlight);
        selectColor.setAlphaF(0.1);

        painter.setBrush(selectColor);
        painter.drawRoundedRect(rect(), 8, 8);

        painter.setPen(palette.color(QPalette::Highlight));
    } else {
        painter.setPen(textColor);
    }

    QRect ifnameRect(curXMargin, margin / 2, painter.fontMetrics().width(m_ifname), painter.fontMetrics().height());
    painter.drawText(ifnameRect, Qt::AlignLeft | Qt::AlignVCenter, m_ifname);

    painter.setPen(textColor);
    if (m_mode == TITLE_HORIZONTAL) {
        QString strLeftBrackets = "(";
        QRect leftBracketsRect(ifnameRect.right() + spacing, ifnameRect.y(), painter.fontMetrics().width(strLeftBrackets), painter.fontMetrics().height());
        painter.drawText(leftBracketsRect, Qt::AlignLeft | Qt::AlignVCenter, strLeftBrackets);

        QString strRecvData = tr("Receive");
        strRecvData = strRecvData + " " + m_recv_bps;
        QRect recvRect(leftBracketsRect.right() + spacing * 2 + sectionSize, ifnameRect.y(), painter.fontMetrics().width(strRecvData), painter.fontMetrics().height());
        painter.drawText(recvRect, Qt::AlignLeft | Qt::AlignVCenter, strRecvData);

        QString strSentData = tr("Send");
        strSentData = strSentData + " " + m_sent_bps + " )";
        QRect sentRect(sectionSize + recvRect.right() + 3 * spacing, ifnameRect.y(), painter.fontMetrics().width(strSentData), painter.fontMetrics().height());
        painter.drawText(sentRect, Qt::AlignLeft | Qt::AlignVCenter, strSentData);

        painter.setPen(Qt::NoPen);
        painter.setBrush(m_recvColor);
        painter.drawEllipse(leftBracketsRect.right() + spacing, leftBracketsRect.y() + qCeil((leftBracketsRect.height() - sectionSize) / 2.0), sectionSize, sectionSize);

        painter.setBrush(m_sentColor);
        painter.drawEllipse(recvRect.right() + 2 * spacing, recvRect.y() + qCeil((recvRect.height() - sectionSize) / 2.0), sectionSize, sectionSize);
    } else {

        QString strRecvData = tr("Receive");
        strRecvData = strRecvData + " " + m_recv_bps;
        QRect recvRect(margin + spacing + sectionSize, ifnameRect.bottom(), painter.fontMetrics().width(strRecvData), painter.fontMetrics().height());
        painter.drawText(recvRect, Qt::AlignLeft | Qt::AlignVCenter, strRecvData);

        QString strSentData = tr("Send");
        strSentData = strSentData  + " " + m_sent_bps;
        QRect sentRect(margin + spacing + sectionSize, recvRect.bottom(), painter.fontMetrics().width(strSentData), painter.fontMetrics().height());
        painter.drawText(sentRect, Qt::AlignLeft | Qt::AlignVCenter, strSentData);


        painter.setPen(Qt::NoPen);
        painter.setBrush(m_recvColor);
        painter.drawEllipse(margin, recvRect.y() + qCeil((recvRect.height() - sectionSize) / 2.0), sectionSize, sectionSize);

        painter.setBrush(m_sentColor);
        painter.drawEllipse(margin, sentRect.y() + qCeil((sentRect.height() - sectionSize) / 2.0), sectionSize, sectionSize);
    }
}

void NetItemWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateWidgetGeometry();
}

void NetItemWidget::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    emit clicked(m_ifname);
}

void NetItemWidget::fontChanged(const QFont &font)
{
    m_font = font;
    this->setFont(m_font);
    updateWidgetGeometry();
}

void NetItemWidget::setMode(int mode)
{
    m_mode = mode;
    this->update();
}

void NetItemWidget::updateData(const NetifInfo &netifInfo)
{
    // 更新图表
    m_ChartWidget->updateChartDate(netifInfo.sendBytes, netifInfo.receiveBytes);

    if (!netifInfo.netName.isEmpty()) {
        m_ifname = netifInfo.netName;
    }

    m_recv_bps = commom::format::formatNetworkBrandWidth(netifInfo.receiveBytes, false);
    m_sent_bps = commom::format::formatNetworkBrandWidth(netifInfo.sendBytes, false);

    this->update();
}

void NetItemWidget::updateActiveStatus(bool active)
{
    m_isActive = active;
    update();
}

void NetItemWidget::updateWidgetGeometry()
{
    QFont ft1;
    ft1.setPixelSize(14);
    m_font = ft1;
    int fontHeight = QFontMetrics(m_font).height();
    int curXMargin = m_mode == TITLE_HORIZONTAL ? 0 : margin;
    if (m_mode == TITLE_HORIZONTAL) {
        m_ChartWidget->setGeometry(curXMargin, TextSpacing, this->width() - 2 * curXMargin, this->height() - TextSpacing - margin);
    } else {
        m_ChartWidget->setGeometry(curXMargin, fontHeight * 2 + TextSpacing, this->width() - 2 * curXMargin, this->height() - fontHeight * 2 - TextSpacing - margin);
    }
}
