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
#ifndef SETUPSHORTCUT_H
#define SETUPSHORTCUT_H

#include "setupshortcut.h"
#include "../themewidget.h"

#include <QSet>
#include <QLineEdit>
#include <QListWidgetItem>

#define ListWidgetStyle "QListWidget{background-color: rgba(1,1,1,0)}" \
                        "QListWidget::item::selected{background-color:rgba(1,1,1,0);}" \
                        "QListWidget::item::hover{background-color:rgba(1,1,1,0);}"


class QLabel;
class QListWidget;
class QHBoxLayout;
class QStackedWidget;

class ShortEdit : public QLineEdit
{
    Q_OBJECT
    friend class ShortCutSettingTest;
public:
    explicit ShortEdit(QWidget *parent = nullptr);
    ~ShortEdit();

    void setBlackTheme();
    void setDefaultTheme();
    void setShortCut(QString s);
    void setWidgetFont(QString family, int size);
    bool m_isKdeConflict = false;

private:
    QString m_theme;
    QString m_start_text;
    QColor m_highlightColor;

protected:
    void updateHightLightColor();
    void focusInEvent(QFocusEvent *e) override;
    void focusOutEvent(QFocusEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    bool event(QEvent *e) override;
};

class ShortCutItem : public ThemeWidget
{
    Q_OBJECT

public:
    explicit ShortCutItem(QString name, QString key, QWidget *parent = nullptr);
    ~ShortCutItem();

    void setBlackTheme();
    void setDefaultTheme();
    void setWidgetFont(QFont f);
    void resetText(QFont f);

    void reset();
    void flush();
    void setKeyForce(QString key);

public slots:
    void setHotkeyConflict(bool isConflict);

signals:
    /// @param isConflict:如果冲突为 true 目的是为了删除之前未冲突保存的临时修改
    void sigKeyChange(QString name, QString newKey, bool isConflict);
    void sigKeyTextChange(QString tmpKey);

private:
    QString m_name, m_key;
    QHBoxLayout *lay;
    QHBoxLayout *lay_key;
    QLabel *lab_name;
    QLabel *lab_conflict;
    ShortEdit *le_key;
    bool m_isConflict;
};

class SetupShortcut : public ThemeWidget
{
    friend class ShortCutItem;

    Q_OBJECT

public:
    explicit SetupShortcut(QWidget *parent = nullptr);
    ~SetupShortcut();

    void setBlackTheme();
    void setDefaultTheme();
    void updateHightLightColor();
    void flushChange();
    void clearChange();
    void setWidgetFont(QString family, int size);
    void reloadFromShortcuts();

private:
    QListWidget *m_titleList;
    QStackedWidget *m_stackedWidget;

    QListWidget *m_fileList;
    QListWidget *m_playList;
    QListWidget *m_imageList;
    QListWidget *m_volumeList;
    QListWidget *m_subtitleList;
    QListWidget *m_otherList;

    std::map<QString, QString> chg_map;
    QSet<ShortCutItem*> item_set;
    static QSet<QString> key_set;

    void initLayout();
    void initListTitle();
    void initConnect();

    void initFileShortCut();
    void initPlayShortCut();
    void initImageShortCut();
    void initVolumeShortCut();
    void initSubtitleShortCut();
    void initOtherShortCut();

    void resetText(QListWidgetItem* item);

private slots:
    void slotChangeKeyCache(QString name, QString newKey, bool isConflict);

};

#endif // SETUPSHORTCUT_H
