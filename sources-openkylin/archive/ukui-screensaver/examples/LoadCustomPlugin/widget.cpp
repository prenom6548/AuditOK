/*
 * Copyright (C) 2023 KylinSoftCo., Ltd.
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
#pragma execution_character_set("utf-8")

#include "widget.h"
#include "ui_widget.h"
#include <QFileDialog>
#include <QPluginLoader>
#include <QHBoxLayout>
#include <memory>
#include <ukui-screensaver/screensaverplugin.h>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    connect(ui->load_plugin_btn,SIGNAL(clicked(bool)),this, SLOT(slot_load_plugin()));
}

Widget::~Widget()
{
    delete ui;
}

void Widget::slot_load_plugin()
{
	/*
    QString file_path = QFileDialog::getOpenFileName(NULL,"加载插件",".","dll (*.dll *.so)");
    if(file_path.isEmpty())
    {
        return;
    }
    */
    QPluginLoader pluginLoader("/usr/lib/ukui-screensaver/libscreensaver-default.so");
    pluginLoader.load();
    QObject* plugin = pluginLoader.instance();
    if (plugin) {
        std::unique_ptr<ScreensaverPlugin> interface_ptr = std::unique_ptr<ScreensaverPlugin>(qobject_cast<ScreensaverPlugin*>(plugin));
        QWidget* widget = interface_ptr->createWidget(false,this);
        widget->setFixedHeight(180);
        widget->setFixedWidth(300);
        QHBoxLayout* mainLayout = new QHBoxLayout(this);

        mainLayout->addWidget(widget);
        ui->widget->setLayout(mainLayout);
    }
}
