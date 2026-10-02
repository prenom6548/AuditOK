#include "searchdevicewin.h"

#include <QDebug>
#include <QScreen>
#include <QApplication>
#include "windowmanage.hpp"

SearchDeviceWin::SearchDeviceWin(QWidget *parent) : QWidget(parent)
{
    initUI();
}

SearchDeviceWin::~SearchDeviceWin()
{
    if (m_loadAnimation != nullptr) {
        m_loadAnimation->deleteLater();
        m_loadAnimation = nullptr;
    }
}

void SearchDeviceWin::initUI()
{
    m_stackedWidget = new QStackedWidget(this);

    QHBoxLayout *listWidgetHLayout = new QHBoxLayout();
    listWidgetHLayout->setMargin(0);
    listWidgetHLayout->setSpacing(0);
    listWidgetHLayout->addWidget(m_stackedWidget);

    QVBoxLayout *mainVLayout = new QVBoxLayout();
    mainVLayout->setMargin(0);
    mainVLayout->setSpacing(0);
    mainVLayout->addWidget(m_stackedWidget);
    this->setLayout(mainVLayout);

    m_loadAnimation = new LoadAnimation;
    m_loadAnimation->setHighLight();
    QWidget *loadWid = new QWidget(this);
    QLabel *loadLab = new QLabel(this);
    loadLab->setText(tr("Searching for nearby available devices..."));

    QHBoxLayout *loadHLayout = new QHBoxLayout();
    loadHLayout->setMargin(0);
    loadHLayout->setSpacing(0);
    loadHLayout->addStretch();
    loadHLayout->addWidget(loadLab);
    loadHLayout->addStretch();

    QVBoxLayout *loadVLayout = new QVBoxLayout();
    loadVLayout->setMargin(0);
    loadVLayout->setSpacing(0);
    loadVLayout->addStretch();
    loadVLayout->addWidget(m_loadAnimation);
    loadVLayout->addLayout(loadHLayout);
    loadVLayout->addStretch();
    loadWid->setLayout(loadVLayout);
    m_stackedWidget->addWidget(loadWid);
    m_stackedWidget->setCurrentIndex(WinIndex::WinLoad);

    m_listWidget = new QListWidget(this);
    m_listWidget->setContentsMargins(0, 0, 0, 0);
    m_listWidget->setSpacing(8);
    m_listWidget->setFocusPolicy(Qt::NoFocus);
    // 设置显示图标模式
    m_listWidget->setViewMode(QListView::IconMode);
    // 自适应布局
    m_listWidget->setResizeMode(QListView::Adjust);
    // 列表中的图标不可拖动
    m_listWidget->setMovement(QListView::Static);
    // 去边框
    m_listWidget->setFrameShape(QListWidget::NoFrame);

    m_stackedWidget->addWidget(m_listWidget);

    m_noNetWorkLab = new QLabel(this);
    m_noNetWorkLab->setAlignment(Qt::AlignCenter);
    m_noNetWorkLab->setText(tr("No network"));
    m_stackedWidget->addWidget(m_noNetWorkLab);
}

void SearchDeviceWin::showLoad()
{
    isSearchInitFinish = false;
    showSearchBtn(false);
    m_stackedWidget->setCurrentIndex(WinIndex::WinLoad);
}

void SearchDeviceWin::showNoNetWork()
{
    showSearchBtn(false);
    m_stackedWidget->setCurrentIndex(WinIndex::WinNoNetWork);
}

void SearchDeviceWin::setSearchBtnState(bool isShow)
{
    isShowSearchBtn = isShow;
    if (isShowSearchBtn && isSearchInitFinish) {
        showSearchBtn(true);
    } else {
        showSearchBtn(false);
    }
}

void SearchDeviceWin::changeFontSize(double fontSize)
{
    m_fontSize = fontSize;
    if (m_searchBtn != nullptr) {
        m_searchBtn->changeFontSize(m_fontSize);
    }
    for (int i = 0; i < m_listWidget->count(); i++) {
        SearchDeviceItem *itemWin = qobject_cast<SearchDeviceItem *>(m_listWidget->itemWidget(m_listWidget->item(i)));
        itemWin->changeFontSize(m_fontSize);
    }
}

void SearchDeviceWin::resizeEvent(QResizeEvent *event)
{
    if (m_searchBtn != nullptr) {
        m_searchBtn->move(
            QPoint(this->width() / 2 - m_searchBtn->width() / 2, this->height() - 16 - m_searchBtn->height()));
    }
    QWidget::resizeEvent(event);
}

void SearchDeviceWin::setItemInfo(const QMap<QString, KCommon::DiscoveryDeviceInfo> &map)
{
    qInfo() << "device count" << map.size();
    m_listWidget->clear();
    QMap<QString, KCommon::DiscoveryDeviceInfo>::const_iterator it = map.begin();

    while (it != map.end()) {
        QString iconStr = "";
        if (it.value().deviceType == KCommon::DeviceType::PC) {
            iconStr = QString(":/devicetype/pc.svg");
        } else if (it.value().deviceType == KCommon::DeviceType::TV) {
            iconStr = QString(":/devicetype/tv.svg");
        } else if (it.value().deviceType == KCommon::DeviceType::Phone) {
            iconStr = QString(":/devicetype/phone.svg");
        } else {
            // 其他类型暂时不显示
            it++;
            continue;
        }
        QListWidgetItem *item = new QListWidgetItem(m_listWidget);
        SearchDeviceItem *searchDeviceItem = new SearchDeviceItem();
        m_listWidget->setItemWidget(item, searchDeviceItem);
        connect(searchDeviceItem, &SearchDeviceItem::clicked, this, [=]() {
            QString itemIP = searchDeviceItem->getItemIp();
            KCommon::DiscoveryDeviceInfo itemDeviceInfo = searchDeviceItem->getItemDeviceInfo();
            Q_EMIT sigItemInfo(itemDeviceInfo.ip);
        });
        item->setSizeHint(QSize(234, 80));
        searchDeviceItem->setFocusPolicy(Qt::NoFocus);
        // 设置图标
        searchDeviceItem->setItemIcon(iconStr);
        // 设置IP
        searchDeviceItem->setItemIP(it.key());
        // 设备名
        searchDeviceItem->setItemName(it.value().deviceName);
        // 设备信息
        searchDeviceItem->setItemDeviceInfo(it.value());
        searchDeviceItem->setTheme(m_theme);
        searchDeviceItem->changeFontSize(m_fontSize);
        it++;
    }
    m_stackedWidget->setCurrentIndex(WinIndex::WinRequest);
    isSearchInitFinish = true;
    if (isShowSearchBtn && isSearchInitFinish) {
        showSearchBtn(true);
    }
}

void SearchDeviceWin::showSearchBtn(bool isShow)
{
    if (isShow) {
        if (m_searchBtn == nullptr) {
            m_searchBtn = new LevitationButton(this);
            m_searchBtn->setIcon(":/connectinterface/searchrefresh.svg");
            m_searchBtn->setText(tr("Search"));
            m_searchBtn->changeFontSize(m_fontSize);
            connect(m_searchBtn, &LevitationButton::clicked, this, [=]() {
                qInfo() << "continue search clicked";
                Q_EMIT sigContinueSearch();
            });
            m_searchBtn->move(
                QPoint(this->width() / 2 - m_searchBtn->width() / 2, this->height() - 16 - m_searchBtn->height()));
        }
        m_searchBtn->show();
    } else {
        if (m_searchBtn != nullptr) {
            m_searchBtn->hide();
        }
    }
}

void SearchDeviceWin::setTheme(PublicAttributes::Theme theme)
{
    m_theme = theme;
    switch (m_theme) {
        case PublicAttributes::Theme::Light: {
            m_listWidget->setStyleSheet("border:1px;border-radius:8px;background: #F6F6F6;");
        } break;
        case PublicAttributes::Theme::Dark: {
            m_listWidget->setStyleSheet("border:1px;border-radius:8px;background: #232426;");
        } break;
    }
    for (int i = 0; i < m_listWidget->count(); i++) {
        SearchDeviceItem *itemWin = qobject_cast<SearchDeviceItem *>(m_listWidget->itemWidget(m_listWidget->item(i)));
        itemWin->setTheme(m_theme);
    }
}