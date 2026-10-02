/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/


#ifndef BURNIMAGE_H
#define BURNIMAGE_H

#include <QWidget>
#include <QMutex>
#include <QLabel>
#include <QListWidget>
#include <QTimer>
#include <QLabel>

namespace Ui {
class BurnImage;
}

class AutoEllipseLabel : public QLabel{
    Q_OBJECT
public:
    AutoEllipseLabel(QWidget* parent = nullptr) : QLabel(parent){
    }
public Q_SLOTS:
    void setText(const QString& text) ;
    void setFont(const QFont& font);
    void clear(){
        m_rawString.clear();
        QLabel::clear();
    }
private:
    QString m_rawString;
};

class BurnImage : public QWidget
{
    Q_OBJECT
    friend class BurnImageTest;
Q_SIGNALS:
    // logic
    void newISOPath(QString);
    void currentBurnerChanged(QString uuid);
    void busness(bool);
public:
    static BurnImage *instance();
public:
    BurnImage(const BurnImage*)=delete;
    BurnImage *operator =(const BurnImage*)=delete;
public:
    void update();
protected:
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
private:
    explicit BurnImage(QWidget *parent = nullptr);
    ~BurnImage();
    void setAccessibleName();
    void updataComboxIsoTextShow();
    void updateComboboxTooltip();
    void updateLabelIsoTip(QString);
protected:
    bool eventFilter(QObject *watched, QEvent *event);

public slots:
    void slotsNewISOPath(QString);
private slots:
    void slotsBrowseISO();
    void slotsISOChange(int);
    void slotsISOError(QString);
    void slotsBurnerError(QString);
    void slotsReady(QString);
    void slotsNewBurner(QIcon miniIcon,QString display, QString tooltip, QString uuid);
    void slotsDeleteBurner(QString uuid);
    void slotsBurnerChange(int);
    void slotsUpdateToolTipText(void *cdrom, QString toolTip);
    void on_comboBoxBurner_activated(int index);

public slots:
    void slotChangeUIFontSize(int fontSize);
private:
    static BurnImage *m_oInstance;
    static QMutex     m_oMutex;
private:
    int               m_iBurnerIndex;
    QStringList       m_oISOPaths;
    AutoEllipseLabel *m_burner_errorinfo_label;
    QList<QString>   m_burner_list;
private:
    Ui::BurnImage    *ui;
};
#define PAGE_BurnImage BurnImage::instance()
#endif // BURNIMAGE_H
