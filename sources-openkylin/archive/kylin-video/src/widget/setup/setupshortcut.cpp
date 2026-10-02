/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "setupshortcut.h"

#include <QLabel>
#include <QDebug>
#include <QKeyEvent>
#include <QLineEdit>
#include <QScrollBar>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QStackedWidget>
#include <QDBusInterface>
#include <QDBusConnection>
#include <KGlobalAccel>
#include "global/global.h"
using namespace Global;

QSet<QString> SetupShortcut::key_set;
QDBusInterface *g_kglobalIfc = nullptr;

QString regKeyString(QString key){
    if(key.endsWith("PgUp"))
        key.replace("PgUp", "Page Up");
    else if(key.endsWith("PgDown"))
        key.replace("PgDown", "Page Down");
    else if(key.endsWith("Return"))
        key.replace("Return", "Enter");
    else if(key.endsWith("空格"))
        key.replace("空格", "Space");
    else if(key.endsWith("{"))
        key.replace("{", "[");
    else if(key.endsWith("}"))
        key.replace("}", "]");
    else if(key.endsWith("~"))
        key.replace("~", "`");
    else if(key.endsWith("!"))
        key.replace("!", "1");
    else if(key.endsWith("@"))
        key.replace("@", "2");
    else if(key.endsWith("#"))
        key.replace("#", "3");
    else if(key.endsWith("$"))
        key.replace("$", "4");
    else if(key.endsWith("%"))
        key.replace("%", "5");
    else if(key.endsWith("^"))
        key.replace("^", "6");
    else if(key.endsWith("&"))
        key.replace("&", "7");
    else if(key.endsWith("*"))
        key.replace("*", "8");
    else if(key.endsWith("("))
        key.replace("(", "9");
    else if(key.endsWith(")"))
        key.replace(")", "0");
    else if(key.endsWith("_"))
        key.replace("_", "-");
    else if(key.endsWith("+")) {
        key.chop(1);
        key.append("=");
    }
    else if(key.endsWith("|"))
        key.replace("|", "\\");
    else if(key.endsWith(":"))
        key.replace(":", ";");
    else if(key.endsWith("\""))
        key.replace("\"", "\'");
    else if(key.endsWith("<"))
        key.replace("<", ",");
    else if(key.endsWith(">"))
        key.replace(">", ".");
    else if(key.endsWith("?"))
        key.replace("?", "/");
    key.replace("+", " + ");
    if(key.contains("Meta"))
        key.replace("Meta", "Super");
    return key;
}

SetupShortcut::SetupShortcut(QWidget *parent) :
    ThemeWidget(parent)
{
    // 初始化 KDE 全局快捷键接口
    if (!g_kglobalIfc) {
        g_kglobalIfc = new QDBusInterface("org.kde.kglobalaccel",
                                          "/kglobalaccel",
                                          "org.kde.KGlobalAccel",
                                          QDBusConnection::sessionBus(),
                                          this);
    }

    initLayout();

    initListTitle();
    initFileShortCut();
    initPlayShortCut();
    initImageShortCut();
    initVolumeShortCut();
    initSubtitleShortCut();
    initOtherShortCut();

    m_fileList->verticalScrollBar()->setProperty("drawScrollBarGroove", false);
    m_playList->verticalScrollBar()->setProperty("drawScrollBarGroove", false);
    m_imageList->verticalScrollBar()->setProperty("drawScrollBarGroove", false);
    m_subtitleList->verticalScrollBar()->setProperty("drawScrollBarGroove", false);
    m_volumeList->verticalScrollBar()->setProperty("drawScrollBarGroove", false);
    m_otherList->verticalScrollBar()->setProperty("drawScrollBarGroove", false);
}

SetupShortcut::~SetupShortcut()
{
}

void SetupShortcut::setBlackTheme()
{
    m_isBlackTheme = true;

    for(int i=0; i<m_fileList->count(); i++)
        ((ShortCutItem*)m_fileList->itemWidget(m_fileList->item(i)))->setBlackTheme();
    for(int i=0; i<m_playList->count(); i++)
        ((ShortCutItem*)m_playList->itemWidget(m_playList->item(i)))->setBlackTheme();
    for(int i=0; i<m_imageList->count(); i++)
        ((ShortCutItem*)m_imageList->itemWidget(m_imageList->item(i)))->setBlackTheme();
    for(int i=0; i<m_volumeList->count(); i++)
        ((ShortCutItem*)m_volumeList->itemWidget(m_volumeList->item(i)))->setBlackTheme();
    for(int i=0; i<m_subtitleList->count(); i++)
        ((ShortCutItem*)m_subtitleList->itemWidget(m_subtitleList->item(i)))->setBlackTheme();
    for(int i=0; i<m_otherList->count(); i++)
        ((ShortCutItem*)m_otherList->itemWidget(m_otherList->item(i)))->setBlackTheme();

    updateHightLightColor();
}

void SetupShortcut::setDefaultTheme()
{
    m_isBlackTheme = false;

    for(int i=0; i<m_fileList->count(); i++)
        ((ShortCutItem*)m_fileList->itemWidget(m_fileList->item(i)))->setDefaultTheme();
    for(int i=0; i<m_playList->count(); i++)
        ((ShortCutItem*)m_playList->itemWidget(m_playList->item(i)))->setDefaultTheme();
    for(int i=0; i<m_imageList->count(); i++)
        ((ShortCutItem*)m_imageList->itemWidget(m_imageList->item(i)))->setDefaultTheme();
    for(int i=0; i<m_volumeList->count(); i++)
        ((ShortCutItem*)m_volumeList->itemWidget(m_volumeList->item(i)))->setDefaultTheme();
    for(int i=0; i<m_subtitleList->count(); i++)
        ((ShortCutItem*)m_subtitleList->itemWidget(m_subtitleList->item(i)))->setDefaultTheme();
    for(int i=0; i<m_otherList->count(); i++)
        ((ShortCutItem*)m_otherList->itemWidget(m_otherList->item(i)))->setDefaultTheme();

    updateHightLightColor();
}

void SetupShortcut::flushChange()
{
    // 逐项保存（有冲突的项将被后端 resetShort 拒绝返回空串）
    for(std::pair<QString, QString> p : chg_map)
    {
        QString useless_key = g_shortcut->resetShort(p.first, p.second);
        // 若保存返回空串（表示冲突），则跳过集合更新
        if (useless_key.isEmpty())
            continue;
        key_set.insert(regKeyString(p.second));
        key_set.remove(regKeyString(useless_key));
    }
    foreach (auto item, item_set) {
        item->flush();
    }
    chg_map.clear();
}

void SetupShortcut::clearChange()
{
    chg_map.clear();
    foreach (auto item, item_set) {
        item->reset();
    }
}

void SetupShortcut::reloadFromShortcuts()
{
    key_set.clear();

    QHash<QString, QString> name2key;
    for (const auto &p : g_shortcut->get_short_map()) {
        if (p.second)
            name2key.insert(p.second->objectName(), regKeyString(p.second->key().toString()));
    }

    auto refreshList = [&](QListWidget *list){
        for (int i = 0; i < list->count(); ++i) {
            ShortCutItem *w = static_cast<ShortCutItem*>(list->itemWidget(list->item(i)));
            if (!w) continue;
            const QString key = name2key.value(w->objectName());
            if (key.isEmpty()) continue;
            key_set.insert(key);
            w->setKeyForce(key);
        }
    };

    refreshList(m_fileList);
    refreshList(m_playList);
    refreshList(m_imageList);
    refreshList(m_volumeList);
    refreshList(m_subtitleList);
    refreshList(m_otherList);
}

void SetupShortcut::setWidgetFont(QString family, int size)
{
    QFont f(family);
    f.setPointSize(size);
    foreach (auto item, item_set) {
        item->setWidgetFont(f);
    }

    m_titleList->setFont(f);
    for(int i=0; i<m_titleList->count(); i++)
    {
        QListWidgetItem *it = m_titleList->item(i);
        resetText(it);
    }
}

void SetupShortcut::initLayout()
{
    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setContentsMargins(30, 10, 8, 8);
    lay->setSpacing(8);

    m_titleList = new QListWidget;
    m_titleList->setAccessibleName("kylin-video_QListWidget_m_titleList");
    m_titleList->setFixedHeight(30);
    lay->addWidget(m_titleList);

    {
        m_stackedWidget = new QStackedWidget;
        m_stackedWidget->setAccessibleName("kylin-video_QStackedWidget_m_stackedWidget");
        lay->addWidget(m_stackedWidget);

        m_fileList = new QListWidget;
        m_fileList->setAccessibleName("kylin-video_QListWidget_m_fileList");
        m_stackedWidget->addWidget(m_fileList);

        m_playList = new QListWidget;
        m_playList->setAccessibleName("kylin-video_QListWidget_m_playList");
        m_stackedWidget->addWidget(m_playList);

        m_imageList = new QListWidget;
        m_imageList->setAccessibleName("kylin-video_QListWidget_m_imageList");
        m_stackedWidget->addWidget(m_imageList);

        m_volumeList = new QListWidget;
        m_volumeList->setAccessibleName("kylin-video_QListWidget_m_volumeList");
        m_stackedWidget->addWidget(m_volumeList);

        m_subtitleList = new QListWidget;
        m_subtitleList->setAccessibleName("kylin-video_QListWidget_m_subtitleList");
        m_stackedWidget->addWidget(m_subtitleList);

        m_otherList = new QListWidget;
        m_otherList->setAccessibleName("kylin-video_QListWidget_m_otherList");
        m_stackedWidget->addWidget(m_otherList);
    }
}

void SetupShortcut::initListTitle()
{
    m_titleList->setFlow(QListView::LeftToRight);
    m_titleList->addItems(QStringList() <<
                             tr("file") <<
                             tr("play") <<
                             tr("image") <<
                             tr("audio") <<
                             tr("sub") <<
                             tr("other"));

    for(int i=0; i<m_titleList->count(); i++)
    {
        QListWidgetItem *it = m_titleList->item(i);
        it->setTextAlignment(Qt::AlignCenter);
        it->setSizeHint(QSize(67, 30));
    }
    m_titleList->setCurrentRow(0);

    m_fileList->setSpacing(2);
    m_fileList->setStyleSheet(ListWidgetStyle);
    m_fileList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_playList->setSpacing(2);
    m_playList->setStyleSheet(ListWidgetStyle);
    m_playList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_imageList->setSpacing(2);
    m_imageList->setStyleSheet(ListWidgetStyle);
    m_imageList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_volumeList->setSpacing(2);
    m_volumeList->setStyleSheet(ListWidgetStyle);
    m_volumeList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_subtitleList->setSpacing(2);
    m_subtitleList->setStyleSheet(ListWidgetStyle);
    m_subtitleList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_otherList->setSpacing(2);
    m_otherList->setStyleSheet(ListWidgetStyle);
    m_otherList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);


    QFont f("Noto Sans CJK SC Regular");
    f.setPixelSize(14);
    m_titleList->setFont(f);

    for(int i=0; i<m_titleList->count(); i++)
    {
        QListWidgetItem *it = m_titleList->item(i);
        resetText(it);
    }

    initConnect();
}

void SetupShortcut::initConnect()
{
    connect(m_titleList, &QListWidget::currentRowChanged, m_stackedWidget, &QStackedWidget::setCurrentIndex);
}

void SetupShortcut::initFileShortCut()
{
    // key item
    for(std::pair<void(*)(), QShortcut*> pair : g_shortcut->get_short_map())
    {
        // 防止快捷键 new 失败导致崩溃
        if(pair.second == nullptr)
            continue;
        if(pair.second->objectName() == tr("open file") ||
            pair.second->objectName() == tr("open dir") ||
            pair.second->objectName() == tr("prev file") ||
            pair.second->objectName() == tr("next file"))
        {
            QString shortKey = regKeyString(pair.second->key().toString());
            key_set.insert(shortKey);
            QListWidgetItem *it = new QListWidgetItem;
            it->setSizeHint(QSize(400, 35));
            ShortCutItem *itemw = new ShortCutItem(pair.second->objectName(), shortKey, this);
            item_set.insert(itemw);
            connect(itemw, &ShortCutItem::sigKeyChange, this, &SetupShortcut::slotChangeKeyCache);
            m_fileList->addItem(it);
            m_fileList->setItemWidget(it, itemw);
        }
    }
}

void SetupShortcut::initPlayShortCut()
{
    // key item
    for(std::pair<void(*)(), QShortcut*> pair : g_shortcut->get_short_map())
    {
        // 防止快捷键 new 失败导致崩溃
        if(pair.second == nullptr)
            continue;
        if(pair.second->objectName() == tr("play/pause") ||
                pair.second->objectName() == tr("speed up") ||
                pair.second->objectName() == tr("speed down") ||
                pair.second->objectName() == tr("speed normal") ||
                pair.second->objectName() == tr("forword") ||
                pair.second->objectName() == tr("backword") ||
                pair.second->objectName() == tr("forward 30s") ||
                pair.second->objectName() == tr("backword 30s") ||
                pair.second->objectName() == tr("insert bookmark") /*||
                pair.second->objectName() == tr("ib notes")*/)
        {
            QString shortKey = regKeyString(pair.second->key().toString());
            key_set.insert(shortKey);
            QListWidgetItem *it = new QListWidgetItem;
            it->setSizeHint(QSize(400, 35));
            ShortCutItem *itemw = new ShortCutItem(pair.second->objectName(), shortKey, this);
            item_set.insert(itemw);
            connect(itemw, &ShortCutItem::sigKeyChange, this, &SetupShortcut::slotChangeKeyCache);
            m_playList->addItem(it);
            m_playList->setItemWidget(it, itemw);
        }
    }
}

void SetupShortcut::initImageShortCut()
{
    // key item
    for(std::pair<void(*)(), QShortcut*> pair : g_shortcut->get_short_map())
    {
        // 防止快捷键 new 失败导致崩溃
        if(pair.second == nullptr)
            continue;
        if ((g_config->videoOutputType() == GlobalConfig::VO_WID || Global::isTablet) && pair.second->objectName() == tr("mini mode"))
            continue;
        // 某些显卡不支持翻转功能，隐藏快捷键设置
        if (isDisableShortcuts && (pair.second->objectName() == tr("horizontal flip") ||
                pair.second->objectName() == tr("vertical flip")))
            continue;
        if(pair.second->objectName() == tr("fullscreen") ||
                pair.second->objectName() == tr("mini mode") ||
                pair.second->objectName() == tr("to top") ||
                pair.second->objectName() == tr("screenshot") ||
//                pair.second->objectName() == tr("cut") ||             // 截取先不做实现
                pair.second->objectName() == tr("light up") ||
                pair.second->objectName() == tr("light down") ||
                pair.second->objectName() == tr("forward rotate") ||
                pair.second->objectName() == tr("backward rotate") ||
                pair.second->objectName() == tr("horizontal flip") ||
                pair.second->objectName() == tr("vertical flip") /*||*/
//                pair.second->objectName() == tr("image boost")        // 画质增强先不做实现
                )
        {
            QString shortKey = regKeyString(pair.second->key().toString());
            key_set.insert(shortKey);
            QListWidgetItem *it = new QListWidgetItem;
            it->setSizeHint(QSize(400, 35));
            ShortCutItem *itemw = new ShortCutItem(pair.second->objectName(), shortKey, this);
            item_set.insert(itemw);
            connect(itemw, &ShortCutItem::sigKeyChange, this, &SetupShortcut::slotChangeKeyCache);
            m_imageList->addItem(it);
            m_imageList->setItemWidget(it, itemw);
        }
    }
}

void SetupShortcut::initVolumeShortCut()
{
    // key item
    for (std::pair<void(*)(), QShortcut*> pair : g_shortcut->get_short_map())
    {
        // 防止快捷键 new 失败导致崩溃
        if (pair.second == nullptr)
            continue;
        if (Global::isTablet && pair.second->objectName() == tr("mute"))
            continue;
        if (pair.second->objectName() == tr("volume up") ||
                pair.second->objectName() == tr("volume down") ||
                pair.second->objectName() == tr("mute") ||
                pair.second->objectName() == tr("audio next") ||
                pair.second->objectName() == tr("default channel") ||
                pair.second->objectName() == tr("left channel") ||
                pair.second->objectName() == tr("right channel"))
        {
            QString shortKey = regKeyString(pair.second->key().toString());
            key_set.insert(shortKey);
            QListWidgetItem *it = new QListWidgetItem;
            it->setSizeHint(QSize(400, 35));
            ShortCutItem *itemw = new ShortCutItem(pair.second->objectName(), shortKey, this);
            item_set.insert(itemw);
            connect(itemw, &ShortCutItem::sigKeyChange, this, &SetupShortcut::slotChangeKeyCache);
            m_volumeList->addItem(it);
            m_volumeList->setItemWidget(it, itemw);
        }
    }
}

void SetupShortcut::initSubtitleShortCut()
{
    // key item
    for(std::pair<void(*)(), QShortcut*> pair : g_shortcut->get_short_map())
    {
        // 防止快捷键 new 失败导致崩溃
        if(pair.second == nullptr)
            continue;
        if(pair.second->objectName() == tr("sub load") ||
                pair.second->objectName() == tr("sub earlier") ||
                pair.second->objectName() == tr("sub later") ||
                pair.second->objectName() == tr("sub up") ||
                pair.second->objectName() == tr("sub down") ||
                pair.second->objectName() == tr("sub next"))
        {
            QString shortKey = regKeyString(pair.second->key().toString());
            key_set.insert(shortKey);
            QListWidgetItem *it = new QListWidgetItem;
            it->setSizeHint(QSize(400, 35));
            ShortCutItem *itemw = new ShortCutItem(pair.second->objectName(), shortKey, this);
            item_set.insert(itemw);
            connect(itemw, &ShortCutItem::sigKeyChange, this, &SetupShortcut::slotChangeKeyCache);
            m_subtitleList->addItem(it);
            m_subtitleList->setItemWidget(it, itemw);
        }
    }
}

void SetupShortcut::initOtherShortCut()
{
    // key item
    for(std::pair<void(*)(), QShortcut*> pair : g_shortcut->get_short_map())
    {
        // 防止快捷键 new 失败导致崩溃
        if (pair.second == nullptr)
            continue;
        if (pair.second->objectName() == tr("play list") ||
                pair.second->objectName() == tr("setup"))
        {
            QString shortKey = regKeyString(pair.second->key().toString());
            key_set.insert(shortKey);
            QListWidgetItem *it = new QListWidgetItem;
            it->setSizeHint(QSize(400, 35));
            ShortCutItem *itemw = new ShortCutItem(pair.second->objectName(), shortKey, this);
            item_set.insert(itemw);
            connect(itemw, &ShortCutItem::sigKeyChange, this, &SetupShortcut::slotChangeKeyCache);
            m_otherList->addItem(it);
            m_otherList->setItemWidget(it, itemw);
        }
    }
}

void SetupShortcut::updateHightLightColor()
{
    QString hover_color = m_isBlackTheme ? "#333333" : "#f5f5f5";
    m_titleList->setStyleSheet(
                QString("QListWidget{background-color: rgba(1,1,1,0);border-radius:10px;}"
                        "QListWidget::item{border-radius:6px;color:#8c8c8c;}"
                        "QListWidget::item::selected{background-color:rgba(1,1,1,0);color:%1;}"
                        "QListWidget::item::hover{background-color:%2;}")
                        .arg(m_highlightColor.name())
                        .arg(hover_color));
}

/** **********************************************
 * 快捷键修改临时缓存
 * 说明：只是保存需要修改的哪些快捷键，如果设置窗口点击了
 *      确认按钮，才会真正的修改这些快捷键。
 * @param:name 修改快捷键的名字
 * @param:newKey 要修改的快捷键组合
 * @param:isConflict 是否有冲突 如果有冲突设置为空
 *************************************************/
void SetupShortcut::slotChangeKeyCache(QString name, QString newKey, bool isConflict)
{
    // 临时改变快捷键
    if (!isConflict)
        chg_map[name] = newKey;
}

void SetupShortcut::resetText(QListWidgetItem* item)
{
    QFontMetrics fontWidth(font());//得到每个字符的宽度
    QString item_name = item->text();
    if (item->toolTip() != "")
    {
        item_name = item->toolTip();
    }
    QString show_name = fontWidth.elidedText(item_name, Qt::ElideRight, item->sizeHint().width() - 10);
    item->setText(show_name);
    if (show_name.endsWith("…"))
        item->setToolTip(item_name);
    else
        item->setToolTip("");
}

ShortCutItem::ShortCutItem(QString name, QString key, QWidget *parent) :
    ThemeWidget(parent),
    m_name(name),
    m_key(key),
    m_isConflict(false)
{
    setObjectName(name);
    m_name = name;
    lab_name = new QLabel;
    lab_name->setAccessibleName("kylin-video_QLabel_lab_name");
    lab_name->setText(name);
    lab_name->setFixedWidth(130);

    QFont f("Noto Sans CJK SC Regular");
    f.setPixelSize(16);
    lab_name->setFont(f);
    resetText(f);

    le_key = new ShortEdit;
    le_key->setAccessibleName("kylin-video_ShortEdit_le_key");
    le_key->setReadOnly(true);
    le_key->setText(key);
    le_key->setFixedSize(237, 30);

    /// 设置界面关闭的时候如果没有保存，需要刷新快捷键界面为打开时的快捷键。目前没做。
    // 先更新冲突状态，再发出变更信号，确保外部拿到的 isConflict 是最新的
    // 同时检查应用内部快捷键和 KDE 全局快捷键冲突
    connect(le_key, &ShortEdit::textChanged, [&](QString tmpKey){
        bool internalConflict = (SetupShortcut::key_set.find(tmpKey) != SetupShortcut::key_set.end());
        if(tmpKey == m_key) {
            setHotkeyConflict(false);
        }
        else {
            setHotkeyConflict(internalConflict || le_key->m_isKdeConflict);
        }
    });
    connect(le_key, &ShortEdit::textChanged, [&](QString key){
        emit sigKeyChange(m_name, key, m_isConflict);
    });
    le_key->setCursor(Qt::IBeamCursor);

    // 热键冲突标签
    lay_key = new QHBoxLayout(le_key);
    lay_key->setObjectName("kylin-video_QHBoxLayout_lay_key");
    lab_conflict = new QLabel;
    lab_conflict->setAccessibleName("kylin-video_QLabel_lab_conflict");
    lab_conflict->setText(tr("Hotkey conflict"));
    lab_conflict->setStyleSheet("color:#f44e50");
    lay_key->addStretch();
    lay_key->addWidget(lab_conflict);
    lay_key->setContentsMargins(0, 0, 8, 0);
    lab_conflict->hide();

    lay = new QHBoxLayout(this);
    lay->setObjectName("kylin-video_QHBoxLayout_lay");
    lay->setContentsMargins(0, 0, 0, 0);

    lay->addWidget(lab_name);
    lay->addWidget(le_key);
    lay->addStretch();

    connectThemeSetting();
}

ShortCutItem::~ShortCutItem()
{
    delete lab_conflict;
    lab_conflict = nullptr;
    delete lay_key;
    lay_key = nullptr;
    delete lab_name;
    lab_name = nullptr;
    delete le_key;
    le_key = nullptr;
    delete lay;
    lay = nullptr;
}

void ShortCutItem::setBlackTheme()
{
    le_key->setBlackTheme();
    lab_name->setStyleSheet("color:#d9d9d9;");
}

void ShortCutItem::setDefaultTheme()
{
    le_key->setDefaultTheme();
    lab_name->setStyleSheet("color:#262626;");
}

void ShortCutItem::resetText(QFont f)
{
    QFontMetrics fontWidth(f);
    QString lab_show_name = fontWidth.elidedText(m_name,Qt::ElideRight,lab_name->width());
    lab_name->setText(lab_show_name);
    if (lab_name->text().endsWith("…"))
        lab_name->setToolTip(m_name);
    else
        lab_name->setToolTip("");
}

void ShortCutItem::setWidgetFont(QFont f)
{
    lab_name->setFont(f);
    lab_conflict->setFont(f);
    le_key->setFont(f);
    resetText(f);
}

void ShortCutItem::reset()
{
    le_key->setText(m_key);
}

void ShortCutItem::flush()
{
    if(!m_isConflict)
        m_key = le_key->text();
}

void ShortCutItem::setKeyForce(QString key)
{
    m_key = key;
    le_key->setText(key);
}

void ShortCutItem::setHotkeyConflict(bool isConflict)
{
    m_isConflict = isConflict;
    if(isConflict)
        lab_conflict->show();
    else
        lab_conflict->hide();
}

ShortEdit::~ShortEdit()
{

}

void ShortEdit::setBlackTheme()
{
    m_theme = STYLE_UKUI_BLACK;
    setStyleSheet("background-color:#404040;color:#d9d9d9;border:0px;border-radius:4px;");
}

void ShortEdit::setDefaultTheme()
{
    m_theme = STYLE_UKUI_LIGHT;
    setStyleSheet("background-color:#f0f0f0;color:#262626;border:0px;border-radius:4px;");
}

void ShortEdit::setShortCut(QString s)
{
    m_start_text = s;
    setText(m_start_text);
}

ShortEdit::ShortEdit(QWidget *parent) :
    QLineEdit(parent)
{
    setTextMargins(8, 0, 0, 0);
    QFont f("Noto Sans CJK SC Regular");
    f.setPixelSize(14);
    setFont(f);

    m_highlightColor = QApplication::palette(this).highlight().color();
}

void ShortEdit::updateHightLightColor()
{
    if (hasFocus()) {
        if (m_theme == STYLE_UKUI_LIGHT) {
            setStyleSheet(QString("QLineEdit{background-color:#f5f5f5;color:#262626;border:1px solid %1;border-radius:4px;}")
                          .arg(m_highlightColor.name()));
        }
        else {
            setStyleSheet(QString("QLineEdit{background-color:#333333;color:#d9d9d9;border:1px solid %1;border-radius:4px;}")
                          .arg(m_highlightColor.name()));
        }
    }
}

void ShortEdit::focusInEvent(QFocusEvent *e)
{
    // 屏蔽 KDE 全局快捷键，避免设置快捷键时触发系统功能
    if (g_kglobalIfc && g_kglobalIfc->isValid()) {
        g_kglobalIfc->call("blockGlobalShortcuts", true);
    }

    updateHightLightColor();
    return QLineEdit::focusInEvent(e);
}

void ShortEdit::focusOutEvent(QFocusEvent *e)
{
    // 恢复 KDE 全局快捷键
    if (g_kglobalIfc && g_kglobalIfc->isValid()) {
        g_kglobalIfc->call("blockGlobalShortcuts", false);
    }

    if(m_theme == STYLE_UKUI_LIGHT) {
        setStyleSheet("QLineEdit{background-color:#f0f0f0;color:#262626;border:0px;border-radius:4px;}");
    }
    else {
        setStyleSheet("QLineEdit{background-color:#404040;color:#d9d9d9;border:0px;border-radius:4px;}");
    }

    if(m_start_text != text())
    {
        m_start_text = text();
    }
    return QLineEdit::focusOutEvent(e);
}

void ShortEdit::keyPressEvent(QKeyEvent *e)
{
    // 组合键不能单独去设置
    if (e->key() == Qt::Key_Meta || e->key() == Qt::Key_Shift || e->key() == Qt::Key_Alt || e->key() == Qt::Key_Control || e->key() == Qt::Key_Super_L || e->key() == Qt::Key_Super_R)
        return;
    QKeySequence qs(e->key());
    QString keyStr = regKeyString(qs.toString());
    QString finalKey;
    if(e->modifiers() == Qt::ShiftModifier)
    {
        // shift 组合键
        finalKey = "Shift + " + keyStr;
    }
    else if(e->modifiers() == Qt::ControlModifier)
    {
        // ctrl 组合键
        finalKey = "Ctrl + " + keyStr;
    }
    else if(e->modifiers() == Qt::AltModifier)
    {
        // alt 组合键
        finalKey = "Alt + " + keyStr;
    }
    else if(e->modifiers() == (Qt::AltModifier|Qt::ControlModifier))
    {
        // ctrl+alt 组合键
        finalKey = "Ctrl + Alt + " + keyStr;
    }
    else if(e->modifiers() == (Qt::AltModifier|Qt::ShiftModifier))
    {
        // alt+shift 组合键
        finalKey = "Alt + Shift + " + keyStr;
    }
    else if(e->modifiers() == (Qt::ControlModifier|Qt::ShiftModifier))
    {
        // ctrl+shift 组合键
        finalKey = "Ctrl + Shift + " + keyStr;
    }

    else if(e->modifiers() == (Qt::MetaModifier))
    {
        // Meta 组合键
        finalKey = "Super + " + keyStr;
    }


    else if(e->modifiers() == (Qt::MetaModifier|Qt::ControlModifier))
    {
        // Meta+Ctrl 组合键
        finalKey = "Super + Ctrl + " + keyStr;
    }

    else if(e->modifiers() == (Qt::MetaModifier|Qt::ShiftModifier))
    {
        // Meta + Shift 组合键
        finalKey = "Super + Shift + " + keyStr;
    }

    else if(e->modifiers() == (Qt::MetaModifier|Qt::AltModifier))
    {
        // Meta + Alt 组合键
        finalKey = "Super + Alt + " + keyStr;
    }

    else
    {
        finalKey = keyStr;
    }

    // 检测是否与 KDE 全局快捷键冲突

    // 直接使用 Qt 按键值构建 QKeySequence，而不是通过字符串
    QKeySequence keySeq(e->key() | e->modifiers());

    // 初始化冲突状态
    m_isKdeConflict = false;

    // 使用 getGlobalShortcutsByKey 检测是否有全局快捷键占用
    QList<KGlobalShortcutInfo> shortcuts = KGlobalAccel::globalShortcutsByKey(keySeq);
    qInfo() << "KGlobalAccel check:" << keySeq << "shortcuts count:" << shortcuts.count();
    if (!shortcuts.isEmpty()) {
        // 获取第一个占用该快捷键的应用名称
        qInfo() << "Conflict with:" << shortcuts.first().friendlyName();
        m_isKdeConflict = true;
    }

    setText(finalKey);

}

bool ShortEdit::event(QEvent *e)
{
    if (e->type() == QEvent::PaletteChange) {
        m_highlightColor = QApplication::palette(this).highlight().color();
    }
    return QLineEdit::event(e);
}
