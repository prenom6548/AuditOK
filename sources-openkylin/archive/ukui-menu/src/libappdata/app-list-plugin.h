/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
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
 *
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#ifndef UKUI_MENU_APP_LIST_PLUGIN_H
#define UKUI_MENU_APP_LIST_PLUGIN_H

#include <QObject>
#include <QList>

class QAction;
class QAbstractItemModel;

namespace UkuiMenu {

class LabelItem
{
    Q_GADGET
    Q_PROPERTY(UkuiMenu::LabelItem::Type type READ type)
    Q_PROPERTY(QString label READ labelName)
    Q_PROPERTY(QString display READ displayName)
public:
    enum Type {
        Text,
        Icon
    };
    Q_ENUM(Type)

    explicit LabelItem(QString labelName = "", QString displayName = "", Type type = Text);

    Type type() const;
    QString labelName() const;
    QString displayName() const;

private:
    Type m_type;
    QString m_labelName;
    QString m_displayName;
};

class LabelBottle : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int column READ column NOTIFY columnChanged)
    Q_PROPERTY(bool visible READ visible NOTIFY visibleChanged)
    Q_PROPERTY(QList<UkuiMenu::LabelItem> labels READ labels NOTIFY labelsChanged)
public:
    explicit LabelBottle(QObject *parent = nullptr);

    int column() const;
    bool visible() const;
    QList<UkuiMenu::LabelItem> labels() const;

    void setColumn(int column);
    void setVisible(bool visible);
    void setLabels(const QList<UkuiMenu::LabelItem> &labels);

Q_SIGNALS:
    void columnChanged();
    void visibleChanged();
    void labelsChanged();

private:
    int m_column {2};
    bool m_visible {false};
    QList<LabelItem> m_labels;
};

class AppListPluginGroup
{
    Q_GADGET
public:
    enum Group {
        Display, /**> 应用展示模式 */
        Search   /**> 应用搜索模式 */
    };
    Q_ENUM(Group)
};

class AppListPluginInterface : public QObject
{
    Q_OBJECT
public:
    explicit AppListPluginInterface(QObject *parent = nullptr);

    virtual AppListPluginGroup::Group group() = 0;
    virtual QString name() = 0;
    virtual QString title() = 0;
    virtual QList<QAction*> actions() = 0;
    virtual QAbstractItemModel *dataModel() = 0;
    virtual void search(const QString &keyword);
    virtual LabelBottle *labelBottle();

Q_SIGNALS:
    void titleChanged();
    void labelChanged();
};

} // UkuiMenu

Q_DECLARE_METATYPE(UkuiMenu::AppListPluginGroup::Group)
Q_DECLARE_METATYPE(UkuiMenu::LabelItem)
Q_DECLARE_METATYPE(UkuiMenu::LabelBottle*)

#endif //UKUI_MENU_APP_LIST_PLUGIN_H
