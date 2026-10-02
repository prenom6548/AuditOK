/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 * Alan Xie    xiehuijun@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "gamekeymanager.h"
#include "kmrewindow.h"
#include "preferences.h"
#include "eventdata.h"
#include "displaymanager.h"
#include "android_display/displaywidget.h"
#include "widgets/messagebox.h"
#include "kmreenv.h"
#include "sessionsettings.h"

#include <QApplication>
#include <QDebug>
#include <QMouseEvent>
#include <QPainter>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonParseError>
#include <QStandardPaths>
#include <algorithm>
#include <sys/syslog.h>

GameKeyManager::GameKeyManager(KmreWindow *window, bool enable) 
    : QObject(window)
    , mMainWindow(window)
    , mEnableGameKey(enable)
    , mKeyIndex(0)
    , mKeyOrientation(-1)
    , mIsVisible(false)
    , mIsMoving(false)
    , mIsHidenInTray(false)
    , mTransparency(1.0)
    , mIsGameKeyInitialed(false)
    , mIsGameKeyInEditing(false)
    , mGameSteelingWheel(nullptr)
    , mSettingsPanel(nullptr)
{
    createGameKeysXmlDir();
    initGameKeyTransparency();
}

GameKeyManager::~GameKeyManager()
{
    if (mSettingsPanel) {
        delete mSettingsPanel;
        mSettingsPanel = nullptr;
    }

    for(auto game_key : mGameKeys) {
        delete game_key;
    }
    mGameKeys.clear();

    if (mGameSteelingWheel) {
        delete mGameSteelingWheel;
        mGameSteelingWheel = nullptr;
    }
}

void GameKeyManager::initSettingsPanel()
{
    if (!mSettingsPanel && mEnableGameKey) {
        mSettingsPanel = new SettingsPanel(mMainWindow);
        updateSettingsPanelPos();
        mSettingsPanel->enableAddSteelingWheelBtn(!mGameSteelingWheel);
        mSettingsPanel->setVisible(false);

        connect(mSettingsPanel, SIGNAL(addSingleKey()), this, SLOT(addKey()));
        connect(mSettingsPanel, SIGNAL(addSteelingWheel()), this, SLOT(addSteelingWheel()));
        connect(mSettingsPanel, SIGNAL(clearGameKeys()), this, SLOT(removeGameKeys()));
        connect(mSettingsPanel, SIGNAL(hideGameKeys()), this, SLOT(hideGameKeys()));
        connect(mSettingsPanel, SIGNAL(saveGameKeys()), this, SLOT(saveGameKeys()));
        connect(mSettingsPanel, SIGNAL(updateTransparency(double)), this, SLOT(updateTransparency(double)));
    }
}

void GameKeyManager::updateSettingsPanelPos()
{
    if (mSettingsPanel) {
        int xpos, ypos;
        DisplayManager *displayManager = mMainWindow->getDisplayManager();
        if (mMainWindow->isFullScreen()) {
            QRect screenSize = displayManager->getScreenSize();
            xpos = (screenSize.width() - DEFAULT_SETTINGS_PANEL_WIDTH) / 2;
            ypos = 0;
        }
        else {
            xpos = (displayManager->getDisplayWidth() - DEFAULT_SETTINGS_PANEL_WIDTH) / 2 + MOUSE_MARGINS;
            ypos = DEFAULT_TITLEBAR_HEIGHT;
        }
        //syslog(LOG_DEBUG, "[GameKeyManager] Move Settings Panel to: x = %d, y = %d", xpos, ypos);
        mSettingsPanel->move(xpos, ypos);
    }
}

void GameKeyManager::setSettingsPanelVisible(bool visible)
{
    if (mSettingsPanel) {
        mSettingsPanel->setVisible(visible);
        if (visible) {
            mSettingsPanel->raise();// avoid to override by main window
        }
    }
}

void GameKeyManager::createGameKeysXmlDir()
{
    mXmlDirectory = KmreEnv::GetConfigPath() + "/gamekey";
    QDir dir;
    if (!dir.exists(mXmlDirectory)) {
        if (!dir.mkpath(mXmlDirectory)) {
            syslog(LOG_ERR, "[GameKeyManager]Fail to create directory %s", mXmlDirectory.toUtf8().data());
        }
    }
}

void GameKeyManager::initGameKeys()
{
    //syslog(LOG_DEBUG, "[GameKeyManager] initGameKeys");
    if (!mEnableGameKey) {
        return;
    }
    QString jsonFile = QString("%1/%2.json").arg(mXmlDirectory).arg(mMainWindow->getPackageName());
    if (utils::isFileExist(jsonFile)) {
        QFile file(jsonFile);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QByteArray ba = file.readAll();
            file.close();
            QJsonParseError err;
            QJsonDocument jsonDocment = QJsonDocument::fromJson(ba, &err);
            if (err.error != QJsonParseError::NoError) {
                return;
            }
            if (jsonDocment.isNull() || jsonDocment.isEmpty()) {
                return;
            }
            QJsonObject jsonObj = jsonDocment.object();
            if (jsonObj.isEmpty() || jsonObj.size() == 0) {
                return;
            }
            if (jsonObj.contains("SteeringWheel")) {
                QJsonObject wheelObj = jsonObj.value("SteeringWheel").toObject();
                if (!wheelObj.isEmpty() || wheelObj.size() > 0) {
                    if (wheelObj.contains("valid") && wheelObj.contains("x") && wheelObj.contains("y") && wheelObj.contains("width") && wheelObj.contains("height")) {
                        if (wheelObj.value("valid").toBool()) {
                            _addSteelingWheel(wheelObj.value("x").toDouble(), 
                                              wheelObj.value("y").toDouble(), 
                                              wheelObj.value("width").toDouble(),
                                              wheelObj.value("height").toDouble(),
                                              false);
                        }
                    }
                }
            }
            if (jsonObj.contains("Keys")) {
                QJsonValue arrayVaule = jsonObj.value("Keys");
                if (arrayVaule.isArray()) {
                    QJsonArray array = arrayVaule.toArray();
                    for (int i = 0;i<array.size();i++) {
                        QJsonValue value = array.at(i);
                        QJsonObject child = value.toObject();
                        if (child.contains("key") && child.contains("x") && child.contains("y")) {
                            _addKey(child.value("key").toString(), 
                                    child.value("x").toDouble(), 
                                    child.value("y").toDouble(), 
                                    false);
                        }
                    }
                }
            }
        }
    }
}

void GameKeyManager::writeGameKeysConfigToFile()
{
    //syslog(LOG_DEBUG, "[GameKeyManager] writeGameKeysConfigToFile");
    QString jsonFile = QString("%1/%2.json").arg(mXmlDirectory).arg(mMainWindow->getPackageName());
    QFile file(jsonFile);
    if (file.open(QIODevice::WriteOnly)) {
        file.resize(0);
        
        QJsonObject transparency;
        transparency.insert("value", mTransparency);

        QJsonObject steeling_wheel;
        if (mGameSteelingWheel) {
            steeling_wheel.insert("valid", true);
            steeling_wheel.insert("x", mGameSteelingWheel->getEventX());
            steeling_wheel.insert("y", mGameSteelingWheel->getEventY());
            steeling_wheel.insert("width", mGameSteelingWheel->getWidthRatio());
            steeling_wheel.insert("height", mGameSteelingWheel->getHeightRatio());
        }
        else {
            steeling_wheel.insert("valid", false);
        }

        QJsonArray keyArray;
        foreach (SingleKey* key, mGameKeys) {
            QJsonObject keyObj;
            keyObj.insert("key", key->getKeyString());
            double x, y;
            key->getEventXY(x, y);
            keyObj.insert("x", x);
            keyObj.insert("y", y);
            keyArray.append(keyObj);
            key->updateStoredKeyString();
        }

        QJsonObject rootObject;
        rootObject.insert("Transparency", transparency);
        rootObject.insert("SteeringWheel", steeling_wheel);
        if (!keyArray.isEmpty()) {
            rootObject.insert("Keys", keyArray);
        }
        QJsonDocument jsonDoc;
        jsonDoc.setObject(rootObject);
        file.write(jsonDoc.toJson());
        file.close();
    }
}

void GameKeyManager::initGameKeyTransparency()
{
    //syslog(LOG_DEBUG, "[GameKeyManager] readGameKeyTransparency");
    double transparency = 1.0;
    QString jsonFile = QString("%1/%2.json").arg(mXmlDirectory).arg(mMainWindow->getPackageName());
    if (utils::isFileExist(jsonFile)) {
        QFile file(jsonFile);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QByteArray ba = file.readAll();
            file.close();
            QJsonParseError err;
            QJsonDocument jsonDocment = QJsonDocument::fromJson(ba, &err);
            if (err.error != QJsonParseError::NoError) {
                return;
            }
            if (jsonDocment.isNull() || jsonDocment.isEmpty()) {
                return;
            }
            QJsonObject jsonObj = jsonDocment.object();
            if (jsonObj.isEmpty() || jsonObj.size() == 0) {
                return;
            }
            if (jsonObj.contains("Transparency")) {
                QJsonObject wheelObj = jsonObj.value("Transparency").toObject();
                if (!wheelObj.isEmpty() || wheelObj.size() > 0) {
                    if (wheelObj.contains("value")) {
                        transparency = wheelObj.value("value").toDouble();
                    }
                }
            }
        }
    }

    mTransparency = transparency;
}

void GameKeyManager::clearGameKeysConfigFile()
{
    //syslog(LOG_DEBUG, "[GameKeyManager] clearGameKeysConfigFile");
    QString jsonFile = QString("%1/%2.json").arg(mXmlDirectory).arg(mMainWindow->getPackageName());
    QFile file(jsonFile);
    if (file.open(QIODevice::WriteOnly)) {
        file.resize(0);
        file.close();
    }
}

void GameKeyManager::saveGameKeysConfig()
{
    //syslog(LOG_DEBUG, "[GameKeyManager] saveGameKeysConfig");
    DisplayManager *displayManager = mMainWindow->getDisplayManager();

    int mainWidget_x, mainWidget_y;
    int margin_width, titlebar_height;
    displayManager->getMainWidgetInfo(mainWidget_x, mainWidget_y, margin_width, titlebar_height);
    QSize emptySpace = getEmptySpace();
    
    QSize displaySize = displayManager->getMainWidgetDisplaySize();
    int displayWidth = displaySize.width();
    int displayHeight = displaySize.height();

    foreach (SingleKey* key, mGameKeys) {
        QRect keyWidgetRect = key->geometry();
        int deltaX;
        int deltaY;
        if (SessionSettings::getInstance().windowUsePlatformWayland()) {
            deltaX = keyWidgetRect.x() - margin_width - emptySpace.width();
            deltaY = keyWidgetRect.y() - titlebar_height - emptySpace.height();
        } else {
            deltaX = keyWidgetRect.x() - mainWidget_x - margin_width - emptySpace.width();
            deltaY = keyWidgetRect.y() - mainWidget_y - titlebar_height - emptySpace.height();
        }
        if (deltaX < 0) {
            deltaX = 0;
        }
        else if (deltaX + keyWidgetRect.width() > displayWidth) {
            deltaX = displayWidth - keyWidgetRect.width();
        }
        deltaX += keyWidgetRect.width() / 2;

        if (deltaY < 0) {
            deltaY = 0;
        }
        else if (deltaY + keyWidgetRect.height() > displayHeight) {
            deltaY = displayHeight - keyWidgetRect.height();
        }
        deltaY += keyWidgetRect.height() / 2;

        double x = deltaX / static_cast<double>(displayWidth);
        double y = deltaY / static_cast<double>(displayHeight);
        key->setEventXY(x, y);
    }

    if (mGameSteelingWheel) {
        QRect steelWheelWidgetRect = mGameSteelingWheel->geometry();
        int deltaX;
        int deltaY;
        if (SessionSettings::getInstance().windowUsePlatformWayland()) {
            deltaX = steelWheelWidgetRect.x() - margin_width - emptySpace.width();
            deltaY = steelWheelWidgetRect.y() - titlebar_height - emptySpace.height();
        } else {
            deltaX = steelWheelWidgetRect.x() - mainWidget_x - margin_width - emptySpace.width();
            deltaY = steelWheelWidgetRect.y() - mainWidget_y - titlebar_height - emptySpace.height();
        }
        if (deltaX < 0) {
            deltaX = 0;
        }
        else if (deltaX + steelWheelWidgetRect.width() > displayWidth) {
            deltaX = displayWidth - steelWheelWidgetRect.width();
        }
        if (deltaY < 0) {
            deltaY = 0;
        }
        else if (deltaY + steelWheelWidgetRect.height() > displayHeight) {
            deltaY = displayHeight - steelWheelWidgetRect.height();
        }
        double x = deltaX / static_cast<double>(displayWidth);
        double y = deltaY / static_cast<double>(displayHeight);
        double w = steelWheelWidgetRect.width() / static_cast<double>(displayWidth);
        double h = steelWheelWidgetRect.height() / static_cast<double>(displayHeight);

        mGameSteelingWheel->setCoordinateData(x, y, w, h);
    }
}

void GameKeyManager::saveGameKeys()
{
    //syslog(LOG_DEBUG, "[GameKeyManager] saveGameKeys");
    enableGameKeyEdit(false);
    saveGameKeysConfig();
    writeGameKeysConfigToFile();
    setSettingsPanelVisible(false);
    DisplayManager *displayManager = mMainWindow->getDisplayManager();
    displayManager->onDisplayForceRedraw();
    mKeyOrientation = displayManager->getCurrentDisplayOrientation();
}

QSize GameKeyManager::getEmptySpace()
{
    if (SessionSettings::getInstance().windowUsePlatformWayland()) {
        if (mMainWindow->isFullScreen()) {
            DisplayManager *displayManager = mMainWindow->getDisplayManager();
            QSize displaySize = displayManager->getMainWidgetDisplaySize();
            int displayWidth = displaySize.width();
            int displayHeight = displaySize.height();
            QRect screenSize = displayManager->getScreenSize();
            int empty_space_w = (screenSize.width() - displayWidth) / 2;
            int empty_space_h = (screenSize.height() - displayHeight) / 2;
            return QSize(empty_space_w, empty_space_h);
        }
        else {
            return QSize(0, 0);
        }
    } else {
        return QSize(0, 0);
    }
}

QPoint GameKeyManager::getGlobalPos(QPoint pos)
{
    int mainWidget_x, mainWidget_y;
    int margin_width, titlebar_height;
    mMainWindow->getDisplayManager()->getMainWidgetInfo(mainWidget_x, mainWidget_y, margin_width, titlebar_height);
    //syslog(LOG_ERR, "[GameKeyManager] getGlobalPos, main widget: x = %d, y = %d", mainWidget_x, mainWidget_y);
    QSize emptySpace = getEmptySpace();

    if (SessionSettings::getInstance().windowUsePlatformWayland()) {
        return QPoint(pos.x() + margin_width + emptySpace.width(), pos.y() + titlebar_height + emptySpace.height());
    } else {
        return QPoint(pos.x() + mainWidget_x + margin_width + emptySpace.width(), pos.y() + mainWidget_y + titlebar_height + emptySpace.height());
    }
}

QRect GameKeyManager::getGameKeyValidRect()
{
    DisplayManager *displayManager = mMainWindow->getDisplayManager();
    QSize displaySize = displayManager->getMainWidgetDisplaySize();
    int displayWidth = displaySize.width();
    int displayHeight = displaySize.height();

    if (mMainWindow->isFullScreen()) {
        QRect screenSize = displayManager->getScreenSize();
        int pos_x = (screenSize.width() - displayWidth) / 2;
        int pos_y = (screenSize.height() - displayHeight) / 2;
        return QRect(pos_x, pos_y, displayWidth, displayHeight);
    }
    else {
        if (SessionSettings::getInstance().windowUsePlatformWayland()) {
            return QRect(MOUSE_MARGINS, 
                         DEFAULT_TITLEBAR_HEIGHT, 
                         displayWidth, 
                         displayHeight);
        } else {
            QRect mainWidgetRect = mMainWindow->geometry();
            return QRect(mainWidgetRect.x() + MOUSE_MARGINS, 
                         mainWidgetRect.y() + DEFAULT_TITLEBAR_HEIGHT, 
                         displayWidth, 
                         displayHeight);
        }
    }
}

void GameKeyManager::addKey()
{
    int displayWidth, displayHeight, initialWidth, initialHeight;
    mMainWindow->getDisplayManager()->getMainWidgetSize(displayWidth, displayHeight, initialWidth, initialHeight);
    double width_scale_ratio = initialWidth / static_cast<double>(displayWidth);
    double height_scale_ratio = initialHeight / static_cast<double>(displayHeight);

    int xpos = (mKeyIndex % 5 + 1) * 60 / width_scale_ratio;
    int ypos = (mKeyIndex / 5 + 2) * 60 / height_scale_ratio;
    double x = xpos / static_cast<double>(displayWidth);
    double y = ypos / static_cast<double>(displayHeight);

    _addKey("", x, y, true);
}

bool GameKeyManager::_addKey(QString key, double x, double y, bool show)
{
    int xpos, ypos;
    std::lock_guard<std::mutex> lk(mKeyLock);

    if ((x <= 0) || (y <= 0)) {
        syslog(LOG_ERR, "[GameKeyManager] _addKey(%d) failed!: x = %f, y = %f", mKeyIndex, x, y);
        return false;
    }

    if (mGameKeys.size() >= 25) {
        KylinUI::MessageBox::warning(mMainWindow, tr("Warning"), tr("Can't add more game key!"));
        return false;
    }

    SingleKey* gameKey = new SingleKey(mKeyIndex, mMainWindow);
    gameKey->setEventXY(x, y);
    gameKey->setKeyString(key);
    gameKey->updateStoredKeyString();
    gameKey->updateSize();
    gameKey->updatePos();

    connect(gameKey, SIGNAL(deleteGameKey(int)), this, SLOT(deleteKey(int)));
    
    gameKey->showKey(show);
    mIsVisible = show;
    
    mGameKeys.push_back(gameKey);
    mKeyIndex++;

    return true;
}

void GameKeyManager::deleteKey(int idx)
{
    std::lock_guard<std::mutex> lk(mKeyLock);

    for(auto game_key : mGameKeys) {
        if (game_key->getIndex() == idx) {
            disconnect(game_key, SIGNAL(deleteGameKey(int)), this, SLOT(deleteKey(int)));
            delete game_key;
            //syslog(LOG_DEBUG, "[GameKeyManager] deleteKey: %d", idx);
            mGameKeys.removeAll(game_key);
            break;
        }
    }
}

bool GameKeyManager::isGameKeyExist(const QString &strKey)
{
    // 界面（尚未保存到配置文件）
    foreach (SingleKey* key, mGameKeys) {
        if (strKey == key->getKeyString()) {
            return true;
        }
    }
    // 配置文件
    return isGameKeyStored(strKey);
}

bool GameKeyManager::isGameKeyFocused(QWindow *win)
{
    foreach (SingleKey* key, mGameKeys) {
        if (win == key->windowHandle()) {
            return true;
        }
    }

    if (mGameSteelingWheel) {
        if (win == mGameSteelingWheel->windowHandle()) {
            return true;
        }
    }

    return false;
}

void GameKeyManager::_checkGameKeyWithSteelingWheel()
{
    foreach (SingleKey* key, mGameKeys) {
        QString keyStr = key->getKeyString();
        if (("A" == keyStr) || ("W" == keyStr) || ("S" == keyStr) || ("D" == keyStr)) {
            key->setKeyString("");
        }
    }
}

bool GameKeyManager::isGameKeyStored(const QString &key)
{
    if (key.isEmpty() || key.isNull()) {
        return false;
    }
    QString jsonFile = QString("%1/%2.json").arg(mXmlDirectory).arg(mMainWindow->getPackageName());
    if (utils::isFileExist(jsonFile)) {
        QFile file(jsonFile);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QByteArray ba = file.readAll();
            file.close();
            QJsonParseError err;
            QJsonDocument jsonDocment = QJsonDocument::fromJson(ba, &err);
            if (err.error != QJsonParseError::NoError) {
                return false;
            }
            if (jsonDocment.isNull() || jsonDocment.isEmpty()) {
                return false;
            }
            QJsonObject jsonObj = jsonDocment.object();
            if (jsonObj.isEmpty() || jsonObj.size() == 0) {
                return false;
            }

            if (jsonObj.contains("Keys")) {
                QJsonValue arrayVaule = jsonObj.value("Keys");
                if (arrayVaule.isArray()) {
                    QJsonArray array = arrayVaule.toArray();
                    for (int i = 0;i<array.size();i++) {
                        QJsonValue value = array.at(i);
                        QJsonObject child = value.toObject();
                        if (child.contains("key") && (child.value("key").toString() == key)) {
                            return true;
                        }
                    }
                }
            }
        }
    }

    return false;
}

void GameKeyManager::addSteelingWheel()
{
    int displayWidth, displayHeight, initialWidth, initialHeight;
    mMainWindow->getDisplayManager()->getMainWidgetSize(displayWidth, displayHeight, initialWidth, initialHeight);

    int xpos = (displayWidth - DEFAULT_STEELING_WHEEL_SIZE) / 2;
    int ypos = (displayHeight - DEFAULT_STEELING_WHEEL_SIZE) / 2;

    double x = xpos / static_cast<double>(displayWidth);
    double y = ypos / static_cast<double>(displayHeight);
    double w = DEFAULT_STEELING_WHEEL_SIZE / static_cast<double>(initialWidth);
    double h = DEFAULT_STEELING_WHEEL_SIZE / static_cast<double>(initialHeight);

    _addSteelingWheel(x, y, w, h, true);
}

bool GameKeyManager::_addSteelingWheel(double x, double y, double w, double h, bool show)
{
    std::lock_guard<std::mutex> lk(mWheelLock);

    if (!mGameSteelingWheel) {
        DisplayManager *displayManager = mMainWindow->getDisplayManager();
        if ((x >= 0) && (y >= 0) && (w > 0) && (h > 0)) {
            mGameSteelingWheel = new SteelingWheel(mMainWindow);
            mGameSteelingWheel->setCoordinateData(x, y, w, h);
            mGameSteelingWheel->updateSize();
            mGameSteelingWheel->updatePos();
        }
        else {
            syslog(LOG_ERR, "[GameKeyManager] addSteelingWheel failed! x = %f, y = %f, w = %f, h = %f",  x, y, w, h);
            return false;
        }

        connect(mGameSteelingWheel, SIGNAL(deleteGameSteelingWheel()), this, SLOT(deleteSteelingWheel()));

        if (mSettingsPanel) {
            mSettingsPanel->enableAddSteelingWheelBtn(false);
        }
        _checkGameKeyWithSteelingWheel();
    }

    mGameSteelingWheel->showKey(show);
    mIsVisible = show;

    return true;
}

void GameKeyManager::deleteSteelingWheel()
{
    std::lock_guard<std::mutex> lk(mWheelLock);
    //syslog(LOG_DEBUG, "[GameKeyManager] deleteSteelingWheel");
    if (mGameSteelingWheel) {
        disconnect(mGameSteelingWheel, SIGNAL(deleteGameSteelingWheel()), this, SLOT(deleteSteelingWheel()));
        delete mGameSteelingWheel;
        mGameSteelingWheel = nullptr;
        if (mSettingsPanel) {
            mSettingsPanel->enableAddSteelingWheelBtn(true);
        }
    }
}

void GameKeyManager::_showGameKeys(bool show)
{
    syslog(LOG_DEBUG, "[GameKeyManager] _showGameKeys: %s", show ? "true" : "false");
    for(auto game_key : mGameKeys) {
        game_key->showKey(show);
        game_key->setOpacity(mTransparency);
    }

    if (mGameSteelingWheel) {
        mGameSteelingWheel->showKey(show);
        mGameSteelingWheel->setOpacity(mTransparency);
    }
}

void GameKeyManager::_showGameKeys()
{
    mIsVisible = true;
    updateGameKeySizeAndPos();//updateGameKeyPos();
    _showGameKeys(true);
    setSettingsPanelVisible(true);
}

void GameKeyManager::showGameKeys()
{
    DisplayManager *displayManager = mMainWindow->getDisplayManager();

    if (!mIsGameKeyInitialed) {
        if (displayManager->isMultiDisplayEnabled()) {
            KylinUI::MessageBox::warning(mMainWindow, tr("Warning"), tr("Don't support game key under multi display mode! Please disable multi display mode of this app and restart!"));
            return;
        }

        initGameKeys();
        initSettingsPanel();
        mIsGameKeyInitialed = true;
        mKeyOrientation = displayManager->getCurrentDisplayOrientation();
    }

    if (mKeyOrientation == displayManager->getCurrentDisplayOrientation()) {
        _showGameKeys();
    }
    else {// 屏幕旋转后，需重新配置所有游戏按键，因此询问用户是否清除已有配置
        //syslog(LOG_DEBUG, "[GameKeyManager] Window rotationed! Must clear old config before set new config.");
        if (KylinUI::MessageBox::question(mMainWindow, tr("Reset Game Key?"), tr("Window rotationed! Do you want to clear current game key settings and continue ?"))) {
            removeGameKeys();
            _showGameKeys();
        }
    }
}

void GameKeyManager::showGameKeysAfterMove()
{
    if (mIsVisible) {
        updateGameKeyPos();
        _showGameKeys(true);
        mIsMoving = false;
    }
}

void GameKeyManager::showGameKeysAfterShownFromTray()
{
    if (mIsVisible && mIsHidenInTray) {
        syslog(LOG_DEBUG, "[GameKeyManager] showGameKeysAfterShownFromTray");
        updateGameKeyPos();
        _showGameKeys(true);
        mIsHidenInTray = false;
    }
}

void GameKeyManager::hideGameKeys()
{
    //syslog(LOG_DEBUG, "[GameKeyManager] hideGameKeys");
    saveGameKeys();
    _showGameKeys(false);
    mIsVisible = false;
}

void GameKeyManager::hideGameKeysWhileMove()
{
    if (mIsVisible) {
        //syslog(LOG_DEBUG, "[GameKeyManager] hideGameKeysWhileMove");
        _showGameKeys(false);
        mIsMoving = true;
    }
}

void GameKeyManager::hideGameKeysWhileHidenInTray()
{
    if (mIsVisible && (!mIsHidenInTray)) {
        //syslog(LOG_DEBUG, "[GameKeyManager] hideGameKeysWhileHidenInTray");
        _showGameKeys(false);
        mIsHidenInTray = true;
    }
}

void GameKeyManager::hideGameKeysWhileRotation()
{
    if (mIsVisible) {
        syslog(LOG_DEBUG, "[GameKeyManager] hideGameKeysWhileRotation...");
        hideGameKeys();
    }
}

void GameKeyManager::removeGameKeys()
{
    //syslog(LOG_DEBUG, "[GameKeyManager] removeGameKeys");
    while (mGameKeys.size()) {
        delete mGameKeys.back();
        mGameKeys.pop_back();
    }
    mKeyIndex = 0;

    deleteSteelingWheel();

    clearGameKeysConfigFile();
}

void GameKeyManager::updateTransparency(double ratio)
{
    //syslog(LOG_DEBUG, "[GameKeyManager] updateTransparency: %f", ratio);
    if ((ratio >= 0) && (ratio <= 1)) {
        for(auto game_key : mGameKeys) {
            game_key->setOpacity(ratio);
        }

        if (mGameSteelingWheel) {
            mGameSteelingWheel->setOpacity(ratio);
        }

        mTransparency = ratio;
    }
}

void GameKeyManager::enableGameKeyEdit(bool enable)
{
    //syslog(LOG_DEBUG, "[GameKeyManager] enableGameKeyEdit: %d", enable);
    for(auto game_key : mGameKeys) {
        game_key->enableEdit(enable);
    }
    if (mGameSteelingWheel) {
        mGameSteelingWheel->enableEdit(enable);
    }

    mIsGameKeyInEditing = enable;
}

void GameKeyManager::updateGameKeyPos()
{
    //syslog(LOG_DEBUG, "[GameKeyManager] updateGameKeyPos");
    if (!mIsVisible) 
        return;

    for(auto game_key : mGameKeys) {
        game_key->updatePos();
    }

    if (mGameSteelingWheel) {
        mGameSteelingWheel->updatePos();
    }

    if (mSettingsPanel) {
        updateSettingsPanelPos();
        if (mSettingsPanel->isVisible()) {
            mSettingsPanel->raise();// avoid to override by main window
        }
    }
}

void GameKeyManager::updateGameKeySize()
{
    //syslog(LOG_DEBUG, "[GameKeyManager] updateGameKeySize");
    if (!mIsVisible) 
        return;

    for(auto game_key : mGameKeys) {
        game_key->updateSize();
    }

    if (mGameSteelingWheel) {
        mGameSteelingWheel->updateSize();
    }
}

void GameKeyManager::updateGameKeySizeAndPos()
{
    //syslog(LOG_DEBUG, "[GameKeyManager] updateGameKeySizeAndPos");
    if (!mIsVisible) 
        return;

    updateGameKeySize();
    updateGameKeyPos();
}

QPoint GameKeyManager::_getGameKeyPos(QWidget *key)
{
    DisplayManager *displayManager = mMainWindow->getDisplayManager();

    int displayWidth, displayHeight, initialWidth, initialHeight;
    displayManager->getMainWidgetSize(displayWidth, displayHeight, initialWidth, initialHeight);

    double width_scale_ratio = initialWidth / static_cast<double>(displayWidth);
    double height_scale_ratio = initialHeight / static_cast<double>(displayHeight);

    if (!displayManager->isCurrentLandscapeOrientation()) {
        width_scale_ratio *= displayManager->getOriginalInitialWidth() / static_cast<double>(initialWidth);
        height_scale_ratio *= displayManager->getOriginalInitialHeight() / static_cast<double>(initialHeight);
    }

    int mainWidget_x, mainWidget_y;
    int margin_width, titlebar_height;
    displayManager->getMainWidgetInfo(mainWidget_x, mainWidget_y, margin_width, titlebar_height);
    QRect keyWidgetRect = key->geometry();

    int pos_x = (keyWidgetRect.x() - mainWidget_x - margin_width + keyWidgetRect.width() / 2.0) * width_scale_ratio;
    int pos_y = (keyWidgetRect.y() - mainWidget_y - titlebar_height + keyWidgetRect.height() / 2.0)  * height_scale_ratio;
    //syslog(LOG_DEBUG, "[GameKeyManager][%s] pos_x = %d, pos_y = %d", __func__, pos_x, pos_y);
    return QPoint(pos_x, pos_y);
}

void GameKeyManager::onKeyPressed(QString strKey)
{
    if ((!mIsVisible) || mIsGameKeyInEditing || mIsMoving || mIsHidenInTray) {
        return;
    }

    DisplayWidget* display = mMainWindow->getDisplayManager()->getFocusedDisplay();
    if (display && (!strKey.isEmpty())) {
        foreach (SingleKey* key, mGameKeys) {
            if (key->getKeyString() == strKey) {
                //多指处理，选择一个按指序号
                for (int i=0; i< 4; i++) {
                    if (mMouseClickOrder[i] == 0) {
                        key->mouseOrderNumber = i+2;
                        mMouseClickOrder[i] = 1;
                        break;
                    }
                }
                QPoint pos = _getGameKeyPos(key);
                //syslog(LOG_DEBUG, "[GameKeyManager]onOneKeyPressed, x = %d, y = %d", pos.x(), pos.y());
                display->sendMouseEvent(MouseEventInfo{Button1_Press, key->mouseOrderNumber, pos.x(), pos.y()});
            }
        }
    }
}

void GameKeyManager::onKeyRelease(QString strKey)
{
    if ((!mIsVisible) || mIsGameKeyInEditing || mIsMoving || mIsHidenInTray) {
        return;
    }

    DisplayWidget* display = mMainWindow->getDisplayManager()->getFocusedDisplay();
    if (display && (!strKey.isEmpty())) {
        foreach (SingleKey* key, mGameKeys) {
            if (key->getKeyString() == strKey) {
                QPoint pos = _getGameKeyPos(key);
                //syslog(LOG_DEBUG, "[GameKeyManager]onKeyRelease, x = %d, y = %d", pos.x(), pos.y());
                display->sendMouseEvent(MouseEventInfo{Button1_Release, key->mouseOrderNumber, pos.x(), pos.y()});
                int i = key->mouseOrderNumber - 2;
                mMouseClickOrder[i] = 0;
                key->mouseOrderNumber = 0;
            }
        }
    }
}

void GameKeyManager::_getSteelingWheelSlideOffset(int &offset_h, int &offset_v)
{
    DisplayManager *displayManager = mMainWindow->getDisplayManager();

    int displayWidth, displayHeight, initialWidth, initialHeight;
    displayManager->getMainWidgetSize(displayWidth, displayHeight, initialWidth, initialHeight);

    double width_scale_ratio = initialWidth / static_cast<double>(displayWidth);
    double height_scale_ratio = initialHeight / static_cast<double>(displayHeight);

    
    if (!displayManager->isCurrentLandscapeOrientation()) {
        width_scale_ratio *= displayManager->getOriginalInitialWidth() / static_cast<double>(initialWidth);
        height_scale_ratio *= displayManager->getOriginalInitialHeight() / static_cast<double>(initialHeight);
    }

    offset_h = mGameSteelingWheel->width() / 2.0 * width_scale_ratio;
    offset_v = mGameSteelingWheel->height() / 2.0 * height_scale_ratio;
    //syslog(LOG_DEBUG, "[GameKeyManager][%s] offset_h = %d, offset_v = %d", __func__, offset_h, offset_v);
}

// 处理walkwidget上的上、下、左、右、左上、左下、右上、右下
void GameKeyManager::onWalkKeyProcess(WalkDirectionType type, bool isPressed)
{
    if ((!mIsVisible) || mIsGameKeyInEditing || mIsMoving || mIsHidenInTray) {
        return;
    }

    DisplayWidget* display = mMainWindow->getDisplayManager()->getFocusedDisplay();
    if (display && mGameSteelingWheel) {
        QPoint pos = _getGameKeyPos(mGameSteelingWheel);
        int offset_h, offset_v;
        _getSteelingWheelSlideOffset(offset_h, offset_v);
        //syslog(LOG_DEBUG, "[GameKeyManager]onWalkKeyProcess, X = %d, Y = %d, offset_h = %d, offset_v = %d", pos.x(), pos.y(), offset_h, offset_v);
        switch(type) {
            case WALK_DIRECTION_CENTER:
                break;
            case WALK_DIRECTION_LEFT:
                //模拟向左滑动
                if (isPressed) {
                    display->sendMouseEvent(MouseEventInfo{Button1_Press, 1, pos.x(), pos.y()});
                    display->sendMouseEvent(MouseEventInfo{Mouse_Motion, 1, pos.x() - offset_h, pos.y()});
                }
                else {
                    display->sendMouseEvent(MouseEventInfo{Button1_Release, 1, pos.x() - offset_h, pos.y()});
                }
                break;
            case WALK_DIRECTION_DOWN:
                //模拟向下滑动
                if (isPressed) {
                    display->sendMouseEvent(MouseEventInfo{Button1_Press, 1, pos.x(), pos.y()});
                    display->sendMouseEvent(MouseEventInfo{Mouse_Motion, 1, pos.x(), pos.y() + offset_v});
                }
                else {
                    display->sendMouseEvent(MouseEventInfo{Button1_Release, 1, pos.x(), pos.y() + offset_v});
                }
                break;
            case WALK_DIRECTION_UP:
                //模拟向上滑动
                if (isPressed) {
                    display->sendMouseEvent(MouseEventInfo{Button1_Press, 1, pos.x(), pos.y()});
                    display->sendMouseEvent(MouseEventInfo{Mouse_Motion, 1, pos.x(), pos.y() - offset_v});
                }
                else {
                    display->sendMouseEvent(MouseEventInfo{Button1_Release, 1, pos.x(), pos.y() - offset_v});
                }
                break;
            case WALK_DIRECTION_RIGHT:
                //模拟向右滑动
                if (isPressed) {
                    display->sendMouseEvent(MouseEventInfo{Button1_Press, 1, pos.x(), pos.y()});
                    display->sendMouseEvent(MouseEventInfo{Mouse_Motion, 1, pos.x() + offset_h, pos.y()});
                }
                else {
                    display->sendMouseEvent(MouseEventInfo{Button1_Release, 1, pos.x() + offset_h, pos.y()});
                }
                break;
            case WALK_DIRECTION_LEFT_UP:
                //模拟向左上方滑动
                display->sendMouseEvent(MouseEventInfo{Button1_Press, 1, pos.x(), pos.y()});
                display->sendMouseEvent(MouseEventInfo{Mouse_Motion, 1, pos.x() - offset_h, pos.y() - offset_v});
                break;
            case WALK_DIRECTION_LEFT_DOWN:
                //模拟向左下方滑动
                display->sendMouseEvent(MouseEventInfo{Button1_Press, 1, pos.x(), pos.y()});
                display->sendMouseEvent(MouseEventInfo{Mouse_Motion, 1, pos.x() - offset_h, pos.y() + offset_v});
                break;
            case WALK_DIRECTION_RIGHT_UP:
                //模拟向右上方滑动
                display->sendMouseEvent(MouseEventInfo{Button1_Press, 1, pos.x(), pos.y()});
                display->sendMouseEvent(MouseEventInfo{Mouse_Motion, 1, pos.x() + offset_h, pos.y() - offset_v});
                break;
            case WALK_DIRECTION_RIGHT_DOWN:
                //模拟向右下方滑动
                display->sendMouseEvent(MouseEventInfo{Button1_Press, 1, pos.x(), pos.y()});
                display->sendMouseEvent(MouseEventInfo{Mouse_Motion, 1, pos.x() + offset_h, pos.y() + offset_v});
                break;
            default:
                break;
        }
    }
}

bool GameKeyManager::initGameSettingsEnable(const QString &packageName)
{
    bool enable = false;
    QString gameConfigFile = "/usr/share/kmre/games.json";
    
    QFile file(gameConfigFile);
    if (file.exists()) {
        if (!file.open(QIODevice::ReadOnly)) {
            syslog(LOG_ERR, "[%s] Failed to open game key config file: '%s'", 
                __func__, gameConfigFile.toStdString().c_str());
        }
        else {
            QString content = file.readAll();
            file.close();

            QJsonParseError json_error;
            const QJsonDocument jsonDocument = QJsonDocument::fromJson(content.toUtf8(), &json_error);
            if (!jsonDocument.isNull() && (json_error.error == QJsonParseError::NoError)) {
                if (jsonDocument.isObject()) {
                    QJsonObject obj = jsonDocument.object();
                    if (obj.contains("games")) {
                        QJsonValue arrayVaule = obj.value("games");
                        if (arrayVaule.isArray()) {
                            QJsonArray array = arrayVaule.toArray();
                            for (int i = 0;i<array.size();i++) {
                                QJsonValue value = array.at(i);
                                QJsonObject child = value.toObject();
                                QString pkgname = child["pkgname"].toString();
                                if (packageName == pkgname) {
                                    enable = true;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return enable;
}

void GameKeyManager::sendEventToMainDisplay(QEvent *event)
{
    DisplayManager *displayManager = mMainWindow->getDisplayManager();
    QApplication::sendEvent(displayManager->getMainDisplayWidget(), event);
}
