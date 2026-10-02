#include "tableheaderview.h"
#include <QMouseEvent>

TableHeaderView::TableHeaderView(Qt::Orientation orientation, QWidget*  parent)
    : QHeaderView(orientation, parent),
      m_checkable(false),
      m_bTristate(false),
      m_bNoChange(false),
      m_bMoving(false),
      m_bChecked(false),
      m_bPressed(false)
{

    setHighlightSections(false);
    setMouseTracking(true);

    // 响应鼠标
    setSectionsClickable(true);
}

void TableHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    painter->save();
    QHeaderView::paintSection(painter, rect, logicalIndex);
    painter->restore();

        if(logicalIndex == 0)
    {
        QStyleOptionButton option;
        option.initFrom(this);

        if (m_bChecked)
            option.state |= QStyle::State_Sunken;

        if (!m_checkable) {
            option.state = QStyle::State_Off;
        } else {
            if (m_bTristate && m_bNoChange)
                option.state |= QStyle::State_NoChange;
            else
                option.state |= m_bChecked ? QStyle::State_On : QStyle::State_Off;

        }
        if (testAttribute(Qt::WA_Hover) && underMouse()) {
            if (m_bMoving)
                option.state |= QStyle::State_MouseOver;
            else
                option.state &= ~QStyle::State_MouseOver;
        }
        QCheckBox checkBox;
        option.iconSize = QSize(16, 16);
        option.rect = QRect(rect.x() + 3 +4, rect.y() + (rect.height() - 16)/2, 16, 16);
        checkBox.setEnabled(m_checkable);
        style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &option, painter, &checkBox);
    }

}


void TableHeaderView::mousePressEvent(QMouseEvent *event)
{
    // if (isOn) {
    //     isOn = false;
    // } else {
    //     isOn = true;
    // }
    // this->viewport()->update();
    // QHeaderView::mousePressEvent(event);


    int nColumn = logicalIndexAt(event->pos());
    if ((event->buttons() & Qt::LeftButton) && (nColumn == 0))
    {
        m_bPressed = true;
    }
    else
    {
        QHeaderView::mousePressEvent(event);
    }
}

void TableHeaderView::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_bPressed)
    {
        if (!m_checkable) {

            QHeaderView::mouseReleaseEvent(event);

        } else {
            if (m_bTristate && m_bNoChange)
            {
                m_bChecked = true;
                m_bNoChange = false;
            }
            else
            {
                m_bChecked = !m_bChecked;
            }

            this->viewport()->update();

            Qt::CheckState state = m_bChecked ? Qt::Checked : Qt::Unchecked;

            Q_EMIT signal_headerCheckStateChanged(state);
        }
    }
    else
    {
        // m_iSelectedCol = logicalIndexAt(event->pos());
        QHeaderView::mouseReleaseEvent(event);
    }

    m_bPressed = false;
}

bool TableHeaderView::event(QEvent *event)
{
    if (event->type() == QEvent::Enter || event->type() == QEvent::Leave)
    {
        QMouseEvent *pEvent = static_cast<QMouseEvent *>(event);
        int nColumn = logicalIndexAt(pEvent->x());
        if (nColumn == 0)
        {
            m_bMoving = (event->type() == QEvent::Enter);

            this->viewport()->update();
            return true;
        }
    }

    return QHeaderView::event(event);
}


void TableHeaderView::slot_checkStateChange(int iState)
{
    // iState = 0-Unchecked 1-PartiallyChecked 2-Checked
    if (iState == Qt::PartiallyChecked) {
        m_bTristate = true;
        m_bNoChange = true;
    } else {
        m_bNoChange = false;
    }

    m_bChecked = (iState != Qt::Unchecked);
    this->viewport()->update();
}

void TableHeaderView::setHeaderCheckBoxCheckEnable(bool checkEnable)
{
    m_checkable = checkEnable;
}