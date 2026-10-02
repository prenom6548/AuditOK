/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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
 */

#include <fstream>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QButtonGroup>
#include <kswitchbutton.h>
#ifdef HAVE_KYSDK_WAYLANDHELPER
#include "ukuistylehelper/ukuistylehelper.h"
#else
#include "ukuistylehelper.h"
#endif
#include "ui_config_win.h"
#include "toolutils.h"
#include "ipweb_widget.h"
#include "config_win.h"

const static int ITEM_HEIGHT = 60;

namespace custom_network {

Config::Config()
    : on_(false)
{
    ips_.clear();
    webs_.clear();
}

bool Config::ParseConfig(const QString &path) {
    QFile file(path);
    if (!file.exists()) {
        qCritical() << "file " << path << " is not exists!";
        return false;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "open " << path << " fail!";
        return false;
    }

    auto ctx = file.readAll();
    file.close();

    QJsonParseError error;
    auto doc = QJsonDocument::fromJson(ctx, &error);
    if (error.error != QJsonParseError::NoError) {
        qCritical() << "Parser json fail: " << error.errorString();
        return false;
    }

    auto root = doc.object();
    if (root.isEmpty()) {
        qCritical() << "json doc switch fail!";
        return false;
    }

    if (root.contains("config-usual")) {
        auto obj = root.value("config-usual").toObject();
        if (obj.contains("switch"))
            on_ = obj.value("switch").toBool();
    }

    if (root.contains("config-ip")) {
        auto array = root.value("config-ip").toArray();
        for (int i = 0; i < array.size(); i++) {
            ips_.push_back(array.at(i).toString());
        }
    }

    if (root.contains("config-web")) {
        auto array = root.value("config-web").toArray();
        for (int i = 0; i < array.size(); i++) {
            webs_.push_back(array.at(i).toString());
        }
    }

    return true;
}

bool Config::SaveConfig(const QString &path) {
    QJsonObject root;

    QJsonObject config_usual;
    config_usual.insert("switch", on_);

    QJsonArray ips;
    for (const auto &ip: ips_) {
        if (!ip.isEmpty())
            ips.push_back(ip);
    }

    QJsonArray webs;
    for (const auto &web: webs_) {
        if (!web.isEmpty())
            webs.push_back(web);
    }

    root.insert("config-usual", config_usual);
    root.insert("config-ip", ips);
    root.insert("config-web", webs);

    QJsonDocument doc;
    doc.setObject(root);
    auto json_data = doc.toJson();

    QFileInfo fileinfo(path);
    if (!fileinfo.exists()) {
        QDir dir(fileinfo.path());
        if (!dir.exists()) {
            if (!dir.mkpath(fileinfo.path())) {
                qCritical() << "custom network create directory fail!";
                return false;
            }
        }
    }

    std::ofstream ofs(path.toStdString(), std::ios::out | std::ios::trunc);
    if (!ofs.is_open()) {
        qCritical() << "custom netwrok save file fail!";
        return false;
    }

    ofs << json_data.data() << '\n';
    ofs.flush();
    ofs.close();

    return true;
}

}

ConfigWin::ConfigWin(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::ConfigWin),
      system_config_path_(CUSTOM_NETWORK_SYSTEM_CONFIG_FILE),
      user_config_path_(QString("%1/%2").arg(QDir::homePath()).arg(CUSTOM_NETWORK_USER_CONFIG_FILE))
{
    ui->setupUi(this);
    ui->titleBar->setTitleName(tr("IntraNetSet"));
    ui->quadBtns->setShowBtnsMode(QuadBtnsShowMode::ONLY_CLOSE_BTN, this);

    system_config_.ParseConfig(system_config_path_);
    user_config_.ParseConfig(user_config_path_);

    if (QFileInfo::exists(user_config_path_)) {
        m_isON = user_config_.on_;
    } else {
        m_isON = system_config_.on_;
    }

    setWin();
    setWindowTitle(tr("IntraNetSet"));
    setWindowModality(Qt::ApplicationModal);
    setWindowFlags(Qt::Tool);
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);

    kdk::UkuiStyleHelper::self()->removeHeader(this);

    //QString platform = QGuiApplication::platformName();
    //if (platform.startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)) {
    //    ;
    //} else {
    //    MotifWmHints hints1;
    //    hints1.flags = MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS;
    //    hints1.functions = MWM_FUNC_ALL;
    //    hints1.decorations = MWM_DECOR_BORDER;
    //    XAtomHelper::getInstance()->setWindowMotifHint(this->winId(), hints1);
    //}

    connect(ui->switchButton, SIGNAL(stateChanged(bool)), this, SLOT(setInnerCheckShow(bool)));
    connect(ui->ipWidget, SIGNAL(addWinSize()), this, SLOT(resizeWinSize()));
    connect(ui->webWidget, SIGNAL(addWinSize()), this, SLOT(resizeWinSize()));
    connect(ui->ipWidget, SIGNAL(minWinSize()), this, SLOT(resizeWinSize()));
    connect(ui->webWidget, SIGNAL(minWinSize()), this, SLOT(resizeWinSize()));

    ui->cancelBtn->setText(tr("Cancel"));
    ui->saveBtn->setText(tr("Save"));
    ui->saveBtn->setProperty("isImportant", true);


    connect(ui->cancelBtn, SIGNAL(pressed()), this, SLOT(cancelPress()));
    connect(ui->saveBtn, SIGNAL(pressed()), this, SLOT(savePress()));

    connect(ui->ipWidget, SIGNAL(changedEvent()), this, SLOT(recordChange()));
    connect(ui->webWidget, SIGNAL(changedEvent()), this, SLOT(recordChange()));
}

ConfigWin::~ConfigWin()
{
    delete ui;
}

void ConfigWin::setWin()
{
    QList<IpWeb> ip_items;
    for (const auto &ip: system_config_.ips_) {
        IpWeb ip_item;
        ip_item.level = IpWeb::Level::kSystem;
        ip_item.text = ip;
        ip_items.push_back(ip_item);
    }
    for (const auto &ip: user_config_.ips_) {
        IpWeb ip_item;
        ip_item.level = IpWeb::Level::kUser;
        ip_item.text = ip;
        ip_items.push_back(ip_item);
    }

    QList<IpWeb> web_items;
    for (const auto &web: system_config_.webs_) {
        IpWeb web_item;
        web_item.level = IpWeb::Level::kSystem;
        web_item.text = web;
        web_items.push_back(web_item);
    }
    for (const auto &web: user_config_.webs_) {
        IpWeb web_item;
        web_item.level = IpWeb::Level::kUser;
        web_item.text = web;
        web_items.push_back(web_item);
    }

    int ip_num = ip_items.size();
    int web_num = web_items.size();

    if (m_isON) {
        ui->switchButton->setChecked(true);

        if (ip_num == 0)
            ui->ipWidget->setItemNums(1, ip_items, IPWebWidgetType::IP_WIDGET);
        else
            ui->ipWidget->setItemNums(ip_num, ip_items, IPWebWidgetType::IP_WIDGET);

        if (web_num == 0)
            ui->webWidget->setItemNums(1, web_items, IPWebWidgetType::WEB_WIDGET);
        else
            ui->webWidget->setItemNums(web_num, web_items, IPWebWidgetType::WEB_WIDGET);

        ui->ipWidget->showListWidget(true, IPWebWidgetType::IP_WIDGET);
        ui->webWidget->showListWidget(true, IPWebWidgetType::WEB_WIDGET);

        int totalItemNum = web_num + ip_num;
        if (totalItemNum == 0 || totalItemNum == 1)
            this->setFixedSize(420, 184 + 2 * ITEM_HEIGHT);
        else if (ip_num == 0 || web_num == 0)
            this->setFixedSize(420, 184 + totalItemNum * ITEM_HEIGHT + ITEM_HEIGHT);
        else
            this->setFixedSize(420, 184 + totalItemNum * ITEM_HEIGHT);
    } else {
        ui->ipWidget->setItemNums(0, ip_items, IPWebWidgetType::IP_WIDGET);
        ui->webWidget->setItemNums(0, web_items, IPWebWidgetType::WEB_WIDGET);
        ui->switchButton->setChecked(false);
        this->setFixedSize(420, 184);
        ui->webWidget->hide();
        ui->ipWidget->hide();
    }
}

void ConfigWin::showWin()
{
    ui->saveBtn->setEnabled(false);
    if (!m_isShow) {
        this->exec();
    }
}

bool ConfigWin::saveSettings()
{
    QList<QString> ips = ui->ipWidget->getAllSettings().split(";", QString::SkipEmptyParts);
    QList<QString> webs = ui->webWidget->getAllSettings().split(";", QString::SkipEmptyParts);

    bool on = false;
    if (ui->switchButton->isChecked() &&
        (!ips.isEmpty() || !webs.isEmpty() ||
        !system_config_.ips_.isEmpty() || !system_config_.webs_.isEmpty()))
    {
        on = true;
    }

    custom_network::Config config;
    config.on_ = on;
    config.ips_.swap(ips);
    config.webs_.swap(webs);
    if (!config.SaveConfig(user_config_path_))
        return false;

    emit sigUpdateConfigFile();
    return true;
}

void ConfigWin::setInnerCheckShow(bool isCheck)
{
    m_isON = isCheck;
    if (isCheck) {
        ui->ipWidget->showListWidget(true, IPWebWidgetType::IP_WIDGET);
        ui->webWidget->showListWidget(true, IPWebWidgetType::WEB_WIDGET);
    } else {
        ui->ipWidget->hide();
        ui->webWidget->hide();
    }
    ui->saveBtn->setEnabled(true);
    resizeWinSize();
}

void ConfigWin::resizeWinSize()
{
    if (m_isON) {
        int ipItemNum = ui->ipWidget->getWidgetItemNums();
        int webItemNum = ui->webWidget->getWidgetItemNums();
        qDebug() << "ConfigWin::resizeWinSize size:" << ipItemNum << webItemNum;
        this->setFixedSize(420, 184 + (ipItemNum + webItemNum) * ITEM_HEIGHT);
    } else {
        this->setFixedSize(420, 184);
    }

    this->update();
}

void ConfigWin::cancelPress()
{
    this->hide();
}

void ConfigWin::savePress()
{
    if (saveSettings()) {
        this->hide();
    } else {
        // TODO:此处应有UI提示
        qCritical() << "保存失败！";
        this->hide();
    }
}

void ConfigWin::recordChange()
{
    ui->saveBtn->setEnabled(true);
    if (!ui->ipWidget->getAllFormatStatus() || !ui->webWidget->getAllFormatStatus()) {
        ui->saveBtn->setEnabled(false);
    }
}
