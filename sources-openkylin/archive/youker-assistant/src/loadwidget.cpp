/*
 * Copyright (C) 2021 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Yang Min yangmin@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "loadwidget.h"
#include "commondef.h"

#include <QPainter>
#include <QPainterPath>
#include <QScreen>

LoadWidget::LoadWidget(QWidget *parent):
    QDialog(parent)
{
     this->setFixedSize(200,60);

    // 添加窗管协议

    initUI();
    
}

LoadWidget::LoadWidget(QString devStatus , QWidget *parent):
    QDialog(parent),m_status(devStatus)
{
     this->setFixedSize(200,60);

    // 添加窗管协议

    initUI();
    
}

LoadWidget::~LoadWidget()
{
    if (qtSettings) {
        delete qtSettings;
        qtSettings = nullptr;
    }
}

void LoadWidget::initUI()
{
    QFont font;
    font.setPixelSize(16);

    QLabel *tooltip = new QLabel(this);
    tooltip->setGeometry(0,0,200,60);
    tooltip->setAlignment(Qt::AlignCenter);
    tooltip->setFont(font);
    
    if("enable" == m_status){
        tooltip->setText(tr("Enabling, please wait"));
    }

    if("disable" == m_status){
        tooltip->setText(tr("Disabling, please wait"));
    }

    initThemeMode();
}

void LoadWidget::setAppIcon(const QString &text)
{
    if(text.isEmpty() || text == ""){
        qDebug() << Q_FUNC_INFO << "arg is empty!!!";
        return;
    }else{
        if(QIcon::hasThemeIcon(text)){
            title_icon->setPixmap(QIcon::fromTheme(text).pixmap(24,24));
            app_icon->setPixmap(QIcon::fromTheme(text).pixmap(96,96));
        }else{
            qDebug() << Q_FUNC_INFO << "No icon exists: " << text << " !!!";
        }
    }
}

void LoadWidget::setAppName(const QString &text)
{
    if(text.isEmpty() || text == ""){
        qDebug() << Q_FUNC_INFO << "APP name should not be empty!!!";
        return;
    }else{
        title_name->setText(text);
        this->setWindowTitle(text);
        title_name->update();
        app_name->setText(text);
    }
}

void LoadWidget::setAppVersion(const QString &text)
{
    if(text.isEmpty() || text == ""){
        qDebug() << Q_FUNC_INFO << "APP version should not be empty!!!";
        return;
    }else{
        app_version->setText(tr("VERSION")+":  "+text);
    }
}

void LoadWidget::setAppDescription(const QString &text)
{
    app_description->append(text);
}

//void AboutWidget::closeEvent(QCloseEvent *event)
//{
//    Q_UNUSED(event);
//    qDebug() << Q_FUNC_INFO;
//}

void LoadWidget::initThemeMode()
{
    const QByteArray idd(THEME_QT_SCHEMA);
    if(QGSettings::isSchemaInstalled(idd)) {
        qtSettings = new QGSettings(idd);
    }
    if (qtSettings) {
        //监听主题改变
        connect(qtSettings, &QGSettings::changed, this, [=](const QString &key)
        {
            if (key == "styleName") {
            } else if ("iconThemeName" == key) {
                setAppIcon("kylin-assistant");
            }
        });
    }
}

void LoadWidget::paintEvent(QPaintEvent *event)
{
    QPainterPath path;
    QPainter painter(this);

    path.addRect(this->rect());
    path.setFillRule(Qt::WindingFill);
    painter.setBrush(this->palette().base());
    painter.setPen(Qt::transparent);
    painter.drawPath(path);
    QDialog::paintEvent(event);
}

void LoadWidget::keyPressEvent(QKeyEvent *event)
{
    // switch (event->key())
    // {
    // case Qt::Key_Escape:
    //     break;
    // default:
        QDialog::keyPressEvent(event);
    // }
}
