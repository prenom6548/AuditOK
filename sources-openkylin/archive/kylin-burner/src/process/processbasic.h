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


#ifndef PROCESSBASIC_H
#define PROCESSBASIC_H

#include <QObject>
#include <QProcess>
#include <QIcon>
#include <QTimer>

class ProcessBasic : public QObject
{
    Q_OBJECT
    enum ProcessStep
    {
        Prepare = 0,
        Config,
        Working,
        Worked
    };

Q_SIGNALS:
    // progress
    void title(QString);
    void setRange(int, int);
    void setValue(int, QIcon icon = QIcon(), QString str = QString());
    void addValue(int, QIcon icon = QIcon(), QString str = QString());
    void cancel(bool);

    // tip
    void success(QString);
    void fail(QString);
    void eject();
    void umountfail(QString);
public:
    ProcessBasic(QObject *parent = nullptr);
    ~ProcessBasic();
public:
    virtual void exec();
protected:
    virtual bool prepare();
    virtual bool config();
    virtual bool working();
    virtual bool worked();
protected slots:
    virtual void slotsCancel();
    virtual void slotsStart() = 0;
    virtual void slotsOutput() = 0;
    virtual void slotsError() = 0;
    virtual void slotsFinished(int,QProcess::ExitStatus) = 0;
    virtual void slotsExit();
private:
    void connection();
protected:
    bool            m_bCancel;
    bool            m_oKeep;
    QProcess       *m_oProcess;
    QString         m_oProcessName;
    QStringList     m_oArguments;
    QString         m_lastErrorMsg;
    QIcon           m_oIcon;
};

#endif // PROCESSBASIC_H
