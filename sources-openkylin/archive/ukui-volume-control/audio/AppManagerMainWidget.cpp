/*
 * Copyright (C) 2020-2024 KylinSoft Co., Ltd.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "AppManagerMainWidget.h"

#include <QVBoxLayout>
#include <QComboBox>

#include "Ukui4CustomControl.h"

AppManagerMainWidget::AppManagerMainWidget(std::list<AppManagerItem>& appManagerList, QWidget* parent)
    : m_appWidgetList(appManagerList), KWidget(parent)
{
    initUi();
    initSlots();
}

QList<ClientDeviceInfo> getAvailablePortList(const NodeType& type)
{
    QList<ClientDeviceInfo> list;

    QDBusMessage reply = DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                                                  AUDIO_OBJECT_PATH,
                                                                  AUDIO_INTERFACE,
                                                                  "getAvailablePortList",
                                                                  EnumToInt(type));

    if (reply.arguments().isEmpty()) {
        qDebug() << "返回参数为空";
        return {};
    }

    const QDBusArgument& dbusArgs = reply.arguments().at(0).value<QDBusArgument>();

    dbusArgs.beginArray();
    while (!dbusArgs.atEnd()) {
        ClientDeviceInfo info;
        dbusArgs >> info;
        list.push_back(info);
    }
    dbusArgs.endArray();

    return list;
}

void AppManagerMainWidget::initUi()
{
    setIcon(QIcon::fromTheme("ukui-control-center"));
    setWidgetName(tr("App Sound Control"));
    setWindowFlags(Qt::Dialog);
    setWindowModality(Qt::WindowModality::WindowModal);
    setContentsMargins(0, 0, 0, 0);
    setFixedSize(760, 520);

    m_pNavigationBar = new kdk::KNavigationBar(this);

    for (const auto& [k, v] : m_itemMap) {
        m_pNavigationBar->addItem(v);
    }

    m_pNavigationBar->setFixedSize(200, 478);
    m_pNavigationBar->setContentsMargins(0, 0, 0, 0);

    auto vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(6, 20, 0, 0);
    vLayout->setSpacing(0);
    vLayout->addWidget(m_pNavigationBar);
    sideBar()->setLayout(vLayout);

    // 创建QStackedWidget用于管理不同的界面
    m_pStackedWidget = new QStackedWidget(this);
    m_pStackedWidget->setContentsMargins(0, 0, 0, 0);
    m_pStackedWidget->setFixedSize(560, 478);

    for (const auto& w : m_appWidgetList) {
        m_pStackedWidget->addWidget(std::move(w.wid));
    }


    // 添加右侧设备窗口布局
    auto vLayout2 = new QVBoxLayout;
    vLayout2->setContentsMargins(0, 20, 0, 0);
    vLayout2->setSpacing(0);
    vLayout2->addWidget(m_pStackedWidget);
    baseBar()->setLayout(vLayout2);

    setLayoutType(kdk::HorizontalType);
}

void AppManagerMainWidget::initSlots()
{
    connect(m_pNavigationBar->listview(), &QListView::clicked, this, [this](const QModelIndex &index){
        auto idx = index.row();
        m_pStackedWidget->setCurrentIndex(idx);
    });

    // 键盘方向键 + enter键支持
    connect(m_pNavigationBar->listview(), &QListView::activated, this, [this](const QModelIndex &index){
        auto idx = index.row();
        m_pStackedWidget->setCurrentIndex(idx);
    });
}

void AppManagerMainWidget::addItem(int dire, int idx, int value, bool muted, const QString& icon, const QString& desc, const QList<ClientDeviceInfo>& outputList, const QList<ClientDeviceInfo>& inputList)
{
    if (desc.contains("Loopback from") || desc.contains("Loopback to") || desc.contains("Simultaneous output on"))
        return;

    /** The application management interface does not display blacklisted applications */
    if (desc.isEmpty() || m_blackList.contains(desc) || m_blackList.contains(icon)) {
        qDebug() << QString("add desc: %1 icon: %2 abandoned.").arg(desc).arg(icon);
        return;
    }

    AppManagerItem appItem;

    if (dire)
        appItem.outputStreamVec.emplace_back(idx);
    else
        appItem.inputStreamVec.emplace_back(idx);

    auto wid = new AppManagerItemWidget(new AutoOmitLabel(desc), icon, this);
    auto item = new QStandardItem(QIcon::fromTheme(icon), desc);

    wid->setMuted(VolumeSliderType::VOLUME_SLIDER_SINKINPUT, muted);
    wid->setValue(VolumeSliderType::VOLUME_SLIDER_SINKINPUT, value);

    qDebug() << QString("add %1 to app widget, value: %2, muted: %3.").arg(desc).arg(value).arg(muted);

    appItem.iconName = icon;
    appItem.index = m_itemMap.size();
    appItem.wid = wid;
    m_appWidgetList.push_back(appItem);

    qDebug() << QString("add %1 to app widget, index: %2, icon: %3.").arg(desc).arg(idx).arg(icon);

    // 设置项的样式
    QString itemStyle = "QStandardItem { min-width: 176px; max-width: 176px; min-height: 36px; max-height: 36px; }";
    item->setData(itemStyle, Qt::UserRole);

    m_pStackedWidget->addWidget(wid);
    m_pNavigationBar->addItem(item);
    m_itemMap.emplace(icon, item);

    bool isEnableOutPut = false;
    for (const auto dev : outputList) {
        if (dev.enabled) {
            isEnableOutPut = true;
            addPortToAppItem(appItem.iconName, NodeType::OUTPUT, QString(dev.portLabel + " (" + dev.cardDesc + ")"),
                             QVariant(std::initializer_list<QString> {dev.cardName, dev.portName}));
        }
    }
    if (!isEnableOutPut) {
        addPortToAppItem(appItem.iconName, NodeType::OUTPUT, tr("None"),
                         QVariant(std::initializer_list<QString> {"None", "None"}));
    }

    bool isEnableInPut = false;
    for (const auto dev : inputList) {
        if (dev.enabled) {
            isEnableInPut = true;
            addPortToAppItem(appItem.iconName, NodeType::INPUT, QString(dev.portLabel + " (" + dev.cardDesc + ")"),
                             QVariant(std::initializer_list<QString> {dev.cardName, dev.portName}));
        }
    }
    if (!isEnableInPut) {
        addPortToAppItem(appItem.iconName, NodeType::INPUT, tr("None"),
                         QVariant(std::initializer_list<QString> {"None", "None"}));
    }

    auto s = static_cast<QSlider*>(wid->getSliderItem()->getSlider()->getWidget());
    auto btn = wid->getSliderItem()->getMuteBtn();

    connect(wid, &AppManagerItemWidget::closeWindowSignals, this, [this]() {
        Q_EMIT closeWindowSignals();
    });

    if (s) {
        // 无输出设备时仅调整系统项；其他应用项只设置自身音量/静音，不改 enable
        const bool isSystemItem = (icon.compare(QStringLiteral("kylin-settings-system")) == 0);
        if (isSystemItem) {
            wid->setValue(VolumeSliderType::VOLUME_SLIDER_SINKINPUT, isEnableOutPut ? value : 0);
            wid->setMuted(VolumeSliderType::VOLUME_SLIDER_SINKINPUT, isEnableOutPut ? muted : true);
            s->setEnabled(isEnableOutPut);
            btn->setEnabled(isEnableOutPut);
        } else {
            wid->setValue(VolumeSliderType::VOLUME_SLIDER_SINKINPUT, value);
            wid->setMuted(VolumeSliderType::VOLUME_SLIDER_SINKINPUT, muted);
        }

        connect(s, &QSlider::valueChanged, this, [idx, icon, wid, this](int val) {
            if (wid->getSliderItem()) {
                wid->getSliderItem()->setValue(val);
            }
            Q_EMIT valueChangedSignals(idx, val);
            //start
            qDebug() << "AppManagerMainWidget valueChanged icon:" << icon << "val:" << val;
            if ((icon.compare(QString("kylin-music")) == 0)
                || (icon.compare(QString("kylin-video")) == 0)) {
                QDBusMessage msg = QDBusMessage::createSignal(QString("/"), QString("org.ukui.media"), QString("sinkVolumeChanged"));
                msg << icon << val << false;
                QDBusConnection::sessionBus().send(msg);
            }
            //end
        });
    }

    if (btn) {
        connect(btn, &QPushButton::clicked, this, [idx, this]() {
            Q_EMIT muteChangedSignals(idx);
        });
    }

    auto selectComboxList = wid->getSelectComboxItems();
    for (const auto& box : selectComboxList) {

        auto combox = static_cast<QComboBox*>(box.second->getSelectBox()->getWidget());
        connect(combox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [idx, box, desc, this](int) {
            auto data = box.second->getSelectBox()->currentData().toStringList();
            if (data.size() < 2) {
                qWarning() << QString("Ivalid combox data.");
                return;
            }

            qDebug() << QString("%1 application change device to card: %2, port: %3").arg(desc).arg(data.at(0)).arg(data.at(1));
            Q_EMIT currentIndexChangedSignals(box.first, idx, data.at(0), data.at(1));
        });
    }
}

void AppManagerMainWidget::removeItem(const QString& name)
{
    auto it = std::find_if(m_appWidgetList.begin(), m_appWidgetList.end(), [name](const AppManagerItem& item) {
        return item.iconName == name;
    });

    if (it != m_appWidgetList.end()) {
        m_pStackedWidget->removeWidget((*it).wid);
        m_appWidgetList.erase(it);

        // 重新索引剩余项，保持索引连续性 bug#592847
        // 修复问题：当应用播放完成被移除后，如果不重新索引，
        // 剩余应用的 index 会与 m_itemMap.size() 不一致，
        // 导致 initAppCurrentDefaultDevice 使用错误的索引查找应用项，
        // 造成设备显示对调（例如：kylin-music 显示"模拟输出"，kylin-video 显示"模拟耳机"）
        int newIndex = 0;
        for (auto& item : m_appWidgetList)
            item.index = newIndex++;
    }
    else
        qWarning() << QString("Can not found %1 stream in app manager widget, remove failed.").arg(name);

    auto at = m_itemMap.find(name);
    if (at != m_itemMap.end()) {
        m_pNavigationBar->model()->removeRow(at->second->row());
        m_itemMap.erase(at);
    }
    else
        qWarning() << QString("Can not found %1 stream in app manager widget, remove failed.").arg(name);
}

void AppManagerMainWidget::setValue(const QString& name, int value)
{
    auto it = std::find_if(m_appWidgetList.begin(), m_appWidgetList.end(), [name](const AppManagerItem& item) {
        return item.iconName == name;
    });

    if (it != m_appWidgetList.end()) {
        (*it).wid->getSliderItem()->setValue(value);
    }
    else {
        qWarning() << QString("Does not exit %1 stream, set value failed.").arg(name);
    }
}

void AppManagerMainWidget::setMute(const QString& name, bool mute)
{
    auto it = std::find_if(m_appWidgetList.begin(), m_appWidgetList.end(), [name](const AppManagerItem& item) {
        return item.iconName == name;
    });

    if (it != m_appWidgetList.end()) {
        (*it).wid->getSliderItem()->setMute(mute);
    }
    else {
        qWarning() << QString("Does not exit %1 stream, set mute failed.").arg(name);
    }
}

void AppManagerMainWidget::setCurrentItem(int index)
{
    auto model = m_pNavigationBar->model();
    m_pNavigationBar->listview()->setCurrentIndex(model->item(index, 0)->index());
}

void AppManagerMainWidget::setCurrentIndex(const QString& name, const NodeType& type, int index)
{
    auto it = std::find_if(m_appWidgetList.begin(), m_appWidgetList.end(), [name](const AppManagerItem& item) {
        return item.iconName == name;
    });

    if (it != m_appWidgetList.end()) {
        if (index < 0) {
            int noneIndex = (*it).wid->findData(type, QVariant(std::initializer_list<QString> {"None", "None"}));
            (*it).wid->setCurrentIndex(type, noneIndex);
        } else {
            (*it).wid->setCurrentIndex(type, index);
        }
    }
    else {
        qWarning() << QString("Does not exit %1 stream, set current index failed.").arg(name);
    }
}

void AppManagerMainWidget::addPortToAppItem(const QString& name, const NodeType& type, const QString& text, const QVariant& data)
{
    auto it = std::find_if(m_appWidgetList.begin(), m_appWidgetList.end(), [name](const AppManagerItem& item) {
        return item.iconName == name;
    });

    if (it != m_appWidgetList.end()) {
        (*it).wid->insertItem(type, text, data);
    }
    else {
        qWarning() << QString("Does not exit %1 stream, add port failed.").arg(name);
    }
}

void AppManagerMainWidget::setAppCbxEnabled(const QString& name, const NodeType& type, bool enable)
{
    qDebug() << QString("Set %1 %2 combox enable to %3").arg(name).arg(EnumToInt(type)).arg(enable);

    auto it = std::find_if(m_appWidgetList.begin(), m_appWidgetList.end(), [name](const AppManagerItem& item) {
        return item.iconName == name;
    });

    if (it != m_appWidgetList.end()) {
        //start 录音会重置声音设置的输入设备，经评审录音时输入设备不可用；并且录音时录音没有输出音频流，故输出设备也不可用
        if ((name.compare("kylin-recorder") == 0) && (type == NodeType::INPUT)) {
            if (enable) {
                qDebug() << QString("kylin-recorder disable sink combox.");
                auto s = static_cast<QSlider*>((*it).wid->getSliderItem()->getSlider()->getWidget());
                auto btn = (*it).wid->getSliderItem()->getMuteBtn();
                s->setEnabled(false);
                btn->setEnabled(false);
                (*it).wid->setEnabled(NodeType::OUTPUT, false);
                return;
            } else {
                qDebug() << QString("kylin-recorder enable sink combox.");
                auto s = static_cast<QSlider*>((*it).wid->getSliderItem()->getSlider()->getWidget());
                auto btn = (*it).wid->getSliderItem()->getMuteBtn();
                s->setEnabled(true);
                btn->setEnabled(true);
                (*it).wid->setEnabled(NodeType::OUTPUT, true);
            }
        }
        //end

        (*it).wid->setEnabled(type, enable);
    }
    else {
        qWarning() << QString("Does not exit %1 stream, set combox enable failed.").arg(name);
    }
}

void AppManagerMainWidget::updateOutputVolumeControls(const QString& name, bool enable, int value, bool muted)
{
    auto it = std::find_if(m_appWidgetList.begin(), m_appWidgetList.end(), [name](const AppManagerItem& item) {
        return item.iconName == name;
    });

    if (it == m_appWidgetList.end()) {
        qWarning() << QString("Does not exit %1 stream, update output volume controls failed.").arg(name);
        return;
    }

    auto* sliderItem = (*it).wid->getSliderItem();
    if (!sliderItem)
        return;

    qDebug() << QString("Update %1 output volume controls, enable: %2, value: %3, muted: %4")
                    .arg(name).arg(enable).arg(value).arg(muted);

    // AppManagerSliderItem::setValue 仅在 slider 可用时生效，因此启用时先 enable 再设值；
    // 禁用时先设 0/静音再 disable，避免无输出时仍显示旧音量。
    if (enable) {
        sliderItem->setEnabled(true);
        sliderItem->setValue(value);
        sliderItem->setMute(muted);
    } else {
        sliderItem->setEnabled(true);
        sliderItem->setValue(0);
        sliderItem->setMute(true);
        sliderItem->setEnabled(false);
    }
}

void AppManagerMainWidget::removeProtFromAppItem(const QString& name, const NodeType& type, int cbxIndex)
{
    auto it = std::find_if(m_appWidgetList.begin(), m_appWidgetList.end(), [name](const AppManagerItem& item) {
        return item.iconName == name;
    });

    if (it != m_appWidgetList.end()) {
        (*it).wid->removeItem(type, cbxIndex);
    }
    else {
        qWarning() << QString("Does not exit %1 stream, remove port failed.").arg(name);
    }

}

std::list<AppManagerItem>& AppManagerMainWidget::getAppItems()
{
    return m_appWidgetList;
}

AppManagerItemWidget* AppManagerMainWidget::getAppWidgetByIndex(int idx)
{
    auto it = std::find_if(m_appWidgetList.begin(), m_appWidgetList.end(), [idx](const AppManagerItem& item) {
        return item.index == idx;
    });

    if (it != m_appWidgetList.end()) {
        return (*it).wid;
    }

    return {};
}

void AppManagerMainWidget::hideEvent(QHideEvent *event)
{
    qDebug() << "hideEvent enter";
    Q_UNUSED(event);
    Q_EMIT hideWindowSignals();
    QWidget::hideEvent(event);
}
