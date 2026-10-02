/*
 * Copyright (C) 2024 KylinSoftCo., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/
#ifndef MATEOUPUT_H
#define MATEOUPUT_H
#include <QObject>
#include <QDebug>
#include <QList>

#define DEFINE_QSTRING(var) QString var; QString get##var(){return var;} void set##var(QString e){var = e;}
#define DEFINE_BOOL(var) bool var; bool get##var(){return var;} void set##var(bool e){var = e;}
#define DEFINE_DOUBLE(var) private: double var; public: double get##var(){return var;} void set##var(double e){var = e;}
//#define DEFINE_QSTRING(var) QString get##var(){return var;} void set##var(QString e){e = var;}

class UsdOuputProperty : public QObject{
Q_OBJECT
    Q_PROPERTY(QString name READ getname WRITE setname)
    Q_PROPERTY(QString enable READ getenable WRITE setenable)
    Q_PROPERTY(QString vendor READ getvendor WRITE setvendor)
    Q_PROPERTY(QString product READ getproduct WRITE setproduct)
    Q_PROPERTY(QString serial READ getserial WRITE setserial)
    Q_PROPERTY(QString width READ getwidth WRITE setwidth)
    Q_PROPERTY(QString height READ getheight WRITE setheight)
    Q_PROPERTY(QString rate READ getrate WRITE setrate)
    Q_PROPERTY(QString x READ getx WRITE setx)
    Q_PROPERTY(QString y READ gety WRITE sety)
    Q_PROPERTY(QString rotation READ getrotation WRITE setrotation)
    Q_PROPERTY(QString primary READ getprimary WRITE setprimary)
    Q_PROPERTY(bool seted READ getseted WRITE setseted)
    Q_PROPERTY(double scale READ getscale WRITE setscale)
public:

    UsdOuputProperty(){
//        qDebug()<<"**********************************88" <<"new";
    }

    ~UsdOuputProperty(){
//        qDebug()<<"**********************************88" <<"delete";
    }

private:
    DEFINE_QSTRING(name)
    DEFINE_QSTRING(vendor)
    DEFINE_QSTRING(product)
    DEFINE_QSTRING(serial)
    DEFINE_QSTRING(width)
    DEFINE_QSTRING(height)
    DEFINE_QSTRING(rate)
    DEFINE_QSTRING(x)
    DEFINE_QSTRING(y)
    DEFINE_QSTRING(rotation)
    DEFINE_QSTRING(primary)
    DEFINE_QSTRING(enable)
    DEFINE_BOOL(seted)
    DEFINE_DOUBLE(scale)
};

class OutputsConfig{
public:
    QString m_clone;
    int m_x = 0;
    int m_screenWidth = 0;
    int m_screenHeight = 0;
    int m_foundInCrtc = 0;
    QList<UsdOuputProperty*> m_outputList;
};



#endif // MATEOUPUT_H
