#include "selectwidgetentry.h"

selectWidgetEntry::selectWidgetEntry(QWidget *parent) : QWidget(parent)
{
    status = NORMAL;
    initUI();
}

selectWidgetEntry::~selectWidgetEntry()
{

}

void selectWidgetEntry::initUI()
{
    this->setFixedSize(432, 40);

    m_pDiskNameLabel = new QLabel();

    QPalette palette;
    palette = m_pDiskNameLabel->palette();
    palette.setColor(QPalette::Text, Qt::white);
    m_pDiskNameLabel->setPalette(palette);
    QFont font = m_pDiskNameLabel->font();
    font.setPixelSize(14);
    font.setFamily("Noto Sans CJK SC-Regular");
    m_pDiskNameLabel->setFont(font);

    m_pIconLabel = new QLabel();
    m_pIconLabel->setVisible(false);
    m_pIconLabel->setFixedSize(16, 16);
    m_pIconLabel->setPixmap(QIcon::fromTheme("ukui-selected", \
                                        QIcon(":/data/svg/ukui-selected.png")).pixmap(16, 16));

    m_pMainHBoxLayout = new QHBoxLayout();
    m_pMainHBoxLayout->setContentsMargins(16, 0, 16, 0);
    m_pMainHBoxLayout->setSpacing(0);

    m_pMainHBoxLayout->addWidget(m_pDiskNameLabel, Qt::AlignLeft);
    m_pMainHBoxLayout->addWidget(m_pIconLabel, Qt::AlignRight);
    this->setLayout(m_pMainHBoxLayout);
}

void selectWidgetEntry::setDiskName(QString diskName)
{
    if (diskName.length() == 0) {
        return;
    }
    QStringList infoList = diskName.split("(");
    m_distNameSelectEntry = QString(infoList.last()).remove(")");
    m_pDiskNameLabel->setText(diskName);
    return;
}

void selectWidgetEntry::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button()==Qt::LeftButton) {
        emit updateDiskSelectEntry(this);
    }
    return;
}

void selectWidgetEntry::mousePressEvent(QMouseEvent *event)
{
    status = PRESS;
    this->update();
    QWidget::mousePressEvent(event);
    return;
}

void selectWidgetEntry::enterEvent(QEvent *event)
{
    status=HOVER;
    this->update();
    QWidget::enterEvent(event);
    return;
}

void selectWidgetEntry::leaveEvent(QEvent *event)
{
    status=NORMAL;
    this->update();
    QWidget::leaveEvent(event);
    return;
}

void selectWidgetEntry::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    QPainterPath path;
    switch (status) {
      case NORMAL: {
            p.setBrush(QBrush(QColor(255, 255, 255)));
            p.setOpacity(0);
            break;
          }
      case HOVER: {
            p.setBrush(QBrush(QColor(255, 255, 255)));
            p.setOpacity(0.15);
            break;
          }
      case PRESS: {
            p.setBrush(QBrush(QColor(255, 255, 255)));
            p.setOpacity(0.08);
            break;
          }
    }
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(opt.rect, 6, 6);
    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    setProperty("blurRegion", QRegion(path.toFillPolygon().toPolygon()));
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    QWidget::paintEvent(event);
}
