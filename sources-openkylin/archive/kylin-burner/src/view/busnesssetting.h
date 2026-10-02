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


#ifndef BUSNESSSETTING_H
#define BUSNESSSETTING_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QEvent>
#include "src/view/components/iconlabel.h"

namespace Ui {
class BusnessSetting;
}

class CustomLineEdit : public QLineEdit{
    friend class BusnessSettingTest;
    Q_OBJECT
public:
    explicit CustomLineEdit(QWidget *parent = nullptr);
protected:
    void changeEvent(QEvent* event) override{
        if(event->type() == QEvent::EnabledChange || event->type() == QEvent::FocusIn){
            setCursor(Qt::IBeamCursor);
        }
        QLineEdit::changeEvent(event);
    }
};

class DataSetting;
class BusnessSetting : public QWidget
{
    friend class BusnessSettingTest;
    Q_OBJECT

public:
    explicit BusnessSetting(QWidget *parent = nullptr);
    ~BusnessSetting();

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void checkVolumnID();
    void checkCopyright();
    void checkTempPath();
    void updateModelDialogEnsureBtnState();
    void setAccessibleName();

public slots:
    void load(void *);
    void setSpeed(int i) { m_iSpeed = i; }
    void setBurnFormat(QString s) { m_SBurnFormat = s; }
    void setClean(bool b) { m_bClean = b; }
    void setCreateISO(bool b) { m_bCreateISO = b; }
    void setCloseSession(bool b) { m_bCloseSession = b; }
    void setVerficationData(bool b) { m_bVerfication = b; }
    void setOpen(bool b) { m_bOpen = b; }
    void setPreparer(QString s) { m_oPreparer = s; }
    void setPublisher(QString s) { m_oPublisher = s; }
    void setVolumnID(QString s);
    void setCopyright(QString s) { m_oCopyright = s; }
    void setTempPath(QString s);
    void setIsShowBeforBurn(bool b) { m_isShowBeforBurn = b; }

    void resetText(QLabel* item);
    void updateLabelText();
    void slotChangeUIFontSize(int fontSize);


private:
    void setTemporaryPath(QString);
    void uiAdjust();
    void initTempPathLineEdit();
    void slotChangeFileSystemFormat(int);
    void updateCheckBoxStates();

private slots:
    void slotsBrowse();
    void slotsAccept();

private:
    int         m_iSpeed;
    bool        m_bClean;
    bool        m_bCreateISO;
    bool        m_bCloseSession;
    bool        m_bVerfication;
    bool        m_bOpen;
    bool        m_isShowBeforBurn;
    bool        m_isVaildVolumID;
    bool        m_isVaildCopyright;
    bool        m_isVaildTempPath;
    void       *m_oCDROM;
    QString     m_oPreparer;
    QString     m_oPublisher;
    QString     m_oVolumnID;
    QString     m_oCopyright;
    QString     m_oTemporaryPth;
    QString     m_oBrowsePath;
    QStringList m_oSpeed;
    QList<int>  m_oSpeeds;
    QString     m_SBurnFormat;
    QList<QString>  m_oBurnFormat;
    IConLabel  *buttonBrowse;
private:
    Ui::BusnessSetting *ui;
Q_SIGNALS:
    void ensureBtnStateChanged(bool);
    void cancelBtnStateChanged(bool);
    void title(QString);
};

#endif // BUSNESSSETTING_H
