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
#include "qtkeyboard.h"
#include <QDebug>
#include <QWidget>
#include <QKeyEvent>
#include <QDir>
#include <QApplication>
#include <QCoreApplication>
#include "plasma-shell-manager.h"

QMap<QChar,int> m_specialSymbolMap = {
    {' ',   Qt::Key_Space},
    {',',   Qt::Key_Comma},
    {'.',   Qt::Key_Period},
    {'\'',  Qt::Key_QuoteLeft},
    {'@',   Qt::Key_At},
    {'#',   Qt::Key_NumberSign},
    {'$',   Qt::Key_Dollar},
    {'%',   Qt::Key_Percent},
    {'&',   Qt::Key_Ampersand},
    {'*',   Qt::Key_Asterisk},
    {'(',   Qt::Key_ParenLeft},
    {')',   Qt::Key_ParenRight},
    {'-',   Qt::Key_Minus},
    {'+',   Qt::Key_Plus},
    {'!',   Qt::Key_Exclam},
    {'"',   Qt::Key_QuoteDbl},
    {'<',   Qt::Key_Less},
    {'>',   Qt::Key_Greater},
    {':',   Qt::Key_Colon},
    {';',   Qt::Key_Semicolon},
    {'/',   Qt::Key_Slash},
    {'?',   Qt::Key_Question},
    {'=',   Qt::Key_Equal},
    {'.',   Qt::Key_Period}, /*XK_kana_middledot*/
    {'~',   Qt::Key_AsciiTilde},
    {'`',   Qt::Key_QuoteLeft},
    {'|',   Qt::Key_Bar},
    {'^',   Qt::Key_AsciiCircum},
    {'{',   Qt::Key_BraceLeft},
    {'}',   Qt::Key_BraceRight},
    {'[',   Qt::Key_BracketLeft},
    {']',   Qt::Key_BracketRight},
    {'_',   Qt::Key_Underscore},
    {'\\',  Qt::Key_Backslash},
};

QMap<FuncKey::FUNCKEY, int> m_funckeyMap = {
    {FuncKey::SPACE,     Qt::Key_Space},
    {FuncKey::BACKSPACE, Qt::Key_Backspace},
    {FuncKey::ENTER,     Qt::Key_Enter},
    {FuncKey::HOME,      Qt::Key_Home},
    {FuncKey::END,       Qt::Key_End},
    {FuncKey::PGUP,      Qt::Key_PageUp},
    {FuncKey::PGDN,      Qt::Key_PageDown},
    {FuncKey::INSERT,    Qt::Key_Insert},
    {FuncKey::DELETE,    Qt::Key_Delete},
    {FuncKey::UP,        Qt::Key_Up},
    {FuncKey::DOWN,      Qt::Key_Down},
    {FuncKey::LEFT,      Qt::Key_Left},
    {FuncKey::RIGHT,     Qt::Key_Right}
};

QMap<Modifier::MOD, Qt::KeyboardModifier> m_modifierMap = {
    {Modifier::CTRL,    Qt::ControlModifier},
    {Modifier::ALT,     Qt::AltModifier},
    {Modifier::SUPER,   Qt::MetaModifier},
    {Modifier::SHIFT,   Qt::ShiftModifier}
};

QVector<QChar> m_shiftKeyVec = {'~', '!', '@', '#', '$', '%', '^', '&', '*',
                              '(', ')', '_', '+', '{', '}', '|', ':', '"',
                              '>', '?'};

#define DRM_DIR "/sys/class/leds/"
#define CAPSLOCK_STATUS "capslock_state"

/**
 * @brief 判断大写键状态
 * @return true: 大写锁定
 */
bool QtKeyboard::checkCapsLockState()
{
    QDir ledDir(DRM_DIR);
    QStringList leds = ledDir.entryList(QDir::Dirs);
    QString capsFile;

    for(int i = 0;i<leds.count();i++){
        if(leds.at(i).contains("capslock"))
            capsFile = leds.at(i);
    }
    QFile drmStatusFile(DRM_DIR + capsFile + "/brightness");
    if(drmStatusFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&drmStatusFile);
        QString status = in.readLine();

        if(status == "0") {
            return false;
        }else{
            return true;
        }
    }

    return false;
}

QtKeyboard::QtKeyboard(QObject *parent)
    : FakeKeyboard(parent)
{

}

QtKeyboard::~QtKeyboard()
{

}

void QtKeyboard::addModifier(Modifier::MOD mod)
{
    modList.push_back(mod);
}

void QtKeyboard::removeModifier(Modifier::MOD mod)
{
    modList.removeOne(mod);
}

bool QtKeyboard::hasModifier(Modifier::MOD mod)
{
    return modList.contains(mod);
}

QList<Modifier::MOD> QtKeyboard::getAllModifier()
{
    return modList;
}

void QtKeyboard::clearModifier()
{
    modList.clear();
}


void QtKeyboard::onKeyPressed(QChar c)
{
    /*判断大写锁定打开时，转换字母大小写状态，与x11keyboard类逻辑保持一致*/
    if(checkCapsLockState() && c.isLetter()){
        if(c.isUpper()){
            c = c.toLower();
        }
        else if(c.isLower()){
            c = c.toUpper();
        }
    }

    if(c>='A' && c<='Z'){
        sendKey(c.toLatin1(),c);
    }else if(c>='a' && c<='z'){
        sendKey(c.toLatin1() - 32,c);
    }else if(c >= '0' && c<='9'){
        sendKey(c.toLatin1(),c);
    }else if(m_specialSymbolMap.contains(c)){
        sendKey(m_specialSymbolMap[c],c);
    }else {
        sendKey(c.toLatin1(),c);
    }
}

void QtKeyboard::onKeyPressed(FuncKey::FUNCKEY key)
{
    int keysym = m_funckeyMap[key];
    /*这里的text根据实际按键得到的QEvent中的text内容打印*/
    if(key == FuncKey::SPACE){
        sendKey(keysym," ");
    }else if(key == FuncKey::BACKSPACE){
        sendKey(keysym,"\b");
    }else if(key == FuncKey::ENTER){
        sendKey(keysym,"\r");
    }else if(key == FuncKey::INSERT){
       sendKey(keysym,"\u007F");
    }else if(key == FuncKey::CAPSLOCK){
       PlasmaShellManager::getInstance()->setKeyPressed(58);
    }else{
       sendKey(keysym,"");
    }
}

void QtKeyboard::sendKey(const unsigned int keysym,const QString text)
{
    Qt::KeyboardModifiers modifier = Qt::NoModifier;
    for(auto mod : modList){
        modifier = modifier | m_modifierMap[mod];
    }

    QWidget *objfous = QApplication::focusWidget();

    if(objfous){
        QKeyEvent event1(QEvent::KeyPress, keysym
                         , modifier, text
                         , true, 1);
        QKeyEvent event2(QEvent::KeyRelease, keysym
                         , modifier, text
                         , true, 1);

        QCoreApplication::sendEvent(objfous, &event1);
        QCoreApplication::sendEvent(objfous, &event2);
    }

}
