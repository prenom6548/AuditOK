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


#ifndef COPYDISC_H
#define COPYDISC_H

#include <QWidget>
#include <QMutex>
#include <QLabel>
#include <QListWidget>

namespace Ui {
class CopyDisc;
}

class CopyDisc : public QWidget
{
    Q_OBJECT
    friend class CopyDiscTest;
Q_SIGNALS:
    void busness(int);
    void busness(bool);
public:
    static CopyDisc *instance();
public:
    CopyDisc(const CopyDisc*)=delete;
    CopyDisc *operator =(const CopyDisc*)=delete;
public slots:
    void check();
protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
private:
    explicit CopyDisc(QWidget *parent = nullptr);
    ~CopyDisc();
    void setAccessibleName();
private slots:
    void slotsAddDestination(void *);
    void slotsDelDestination(int);
    void slotsDestinationChange(int);
    void slotsAddSource(void *);
    void slotsDelSource(int);
    void slotsButtonClick();
    void updateLabelIsoTip(QString tipInfo);

public slots:
    void slotChangeUIFontSize(int fontSize);

private:
    static CopyDisc *m_oInstance;
    static QMutex     m_oMutex;
private:
    QWidget          *m_oShowISO;
    QLabel           *m_oIcon;
    QLabel           *m_oDisplay;
    QWidget          *m_oShowBurner;
    QLabel           *m_oIconBurner;
    QLabel           *m_oDisplayBurner;
    QListWidget      *m_oListWidget;
    QStringList       m_oISOPaths;
    QListWidget      *m_oListWidgetBurner;
    QList<void *>     m_oDes;
    QList<void *>     m_oSrc;
private:
    Ui::CopyDisc *ui;
};
#define PAGE_CopyDisc CopyDisc::instance()
#endif // COPYDISC_H
