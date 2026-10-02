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

#ifndef GAME_KEY_MANAGER_H
#define GAME_KEY_MANAGER_H

#include <QList>
#include <mutex>
#include "common.h"
#include "singlekey.h"
#include "steelingwheel.h"
#include "settingspanel.h"

class KmreWindow;

class GameKeyManager : public QObject
{
    Q_OBJECT

public:
    explicit GameKeyManager(KmreWindow *window, bool enable);
    ~GameKeyManager();

    bool isVisible(){return mIsVisible;}
    bool isSettingsPanelVisible(){
        if (mSettingsPanel) {
            return mSettingsPanel->isVisible();
        }
        return false;
    }

    QSize getEmptySpace();
    QPoint getGlobalPos(QPoint pos);
    double getTransparency(){return mTransparency;}
    void showGameKeys();
    void showGameKeysAfterMove();
    void showGameKeysAfterShownFromTray();
    void hideGameKeysWhileMove();
    void hideGameKeysWhileHidenInTray();
    void hideGameKeysWhileRotation();
    bool isGameKeyExist(const QString &strKey);
    bool isGameKeyFocused(QWindow *win);
    void updateGameKeyPos();
    void updateGameKeySize();
    void updateGameKeySizeAndPos();
    void enableGameKeyEdit(bool enable);
    void sendEventToMainDisplay(QEvent *event);
    bool hasSteelingWheel(){return !!mGameSteelingWheel;}
    QString getGameKeyConfigPath(){return mXmlDirectory;}
    QRect getGameKeyValidRect();
    static bool initGameSettingsEnable(const QString &packageName);

signals:
    void windowActive();

public slots:
    void addKey(void);
    void deleteKey(int idx);

    void addSteelingWheel(void);
    void deleteSteelingWheel(void);

    void hideGameKeys();
    void saveGameKeys();
    void removeGameKeys();
    void updateTransparency(double ratio);

    void onKeyPressed(QString strKey);
    void onKeyRelease(QString strKey);
    void onWalkKeyProcess(WalkDirectionType type, bool isPressed);

private:
    KmreWindow *mMainWindow = nullptr;
    QString mXmlDirectory;

    bool mIsGameKeyInitialed;
    bool mEnableGameKey;
    bool mIsVisible;
    bool mIsMoving;
    bool mIsHidenInTray;
    bool mIsGameKeyInEditing;
    double mTransparency;

    int mKeyIndex;
    int mKeyOrientation;
    int mMouseClickOrder[4] = {0, 0, 0, 0}; //对应multitouch序号依次为2,3,4,5
    QList<SingleKey*> mGameKeys;
    std::mutex mKeyLock;
    SteelingWheel* mGameSteelingWheel;
    std::mutex mWheelLock;
    SettingsPanel* mSettingsPanel;

private:
    void initGameKeys();
    void initSettingsPanel();
    void setSettingsPanelVisible(bool visible);
    void updateSettingsPanelPos();

    void saveGameKeysConfig();
    void writeGameKeysConfigToFile();
    void clearGameKeysConfigFile();
    void initGameKeyTransparency();

    bool _addKey(QString key, double x, double y, bool show);
    bool _addSteelingWheel(double x, double y, double w, double h, bool show);
    bool isGameKeyStored(const QString &key);
    void _showGameKeys();
    void _showGameKeys(bool show);
    QPoint _getGameKeyPos(QWidget *key);
    void _getSteelingWheelSlideOffset(int &offset_h, int &offset_v);
    void _checkGameKeyWithSteelingWheel();
    void createGameKeysXmlDir();
    
};
#endif // GAME_KEY_MANAGER_H
