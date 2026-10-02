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
#include <QStandardPaths>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <xkbcommon/xkbcommon.h>
#include <ukuivirtualkeyboard.h>
#include "qxkbcommon_p.h"

xkb_keymap *keymap = nullptr;
xkb_state *state = nullptr;
xkb_mod_index_t shiftIndex = 0;
xkb_mod_index_t controlIndex = 0;
xkb_mod_index_t altIndex = 0;
using ScanCodeKey = std::pair<uint, int>; // group/layout and QtKey
QMap<ScanCodeKey, uint> scanCodesByQtKey;

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

int QtKeyboard::createAnonymousFile(size_t size)
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);
    if (path.isEmpty())
        return -1;

    QByteArray name = QFile::encodeName(path + QStringLiteral("/qtwayland-XXXXXX"));

    int fd = mkstemp(name.data());
    if (fd < 0)
        return -1;

    long flags = fcntl(fd, F_GETFD);
    if (flags == -1 || fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1) {
        close(fd);
        fd = -1;
    }
    unlink(name.constData());

    if (fd < 0)
        return -1;

    if (ftruncate(fd, size) < 0) {
        close(fd);
        return -1;
    }

    return fd;
}

void QtKeyboard::maybeUpdateXkbScanCodeTable()
{
    if (!scanCodesByQtKey.isEmpty() || !state)
        return; // No need to update if the table is already populated or state is not set

    // This function is a placeholder for any future updates to the XKB scan code table.
    // Currently, it does nothing but can be extended as needed.
    xkb_keymap_key_for_each(
        keymap, [](xkb_keymap *keymap, xkb_keycode_t keycode, void *d) {
            auto *scanCodesByQtKey = static_cast<QMap<ScanCodeKey, uint> *>(d);
            uint numLayouts = xkb_keymap_num_layouts_for_key(keymap, keycode);
            for (uint layout = 0; layout < numLayouts; ++layout) {
                const xkb_keysym_t *syms = nullptr;
                xkb_keymap_key_get_syms_by_level(keymap, keycode, layout, 0, &syms);
                if (!syms)
                    continue;
                Qt::KeyboardModifiers mods = {};
                int qtKey = QXkbCommon::keysymToQtKey(syms[0], mods, nullptr, 0, false, false);
                if (qtKey != 0)
                    scanCodesByQtKey->insert({layout, qtKey}, keycode);
            }
        },
        &scanCodesByQtKey);
    shiftIndex = xkb_keymap_mod_get_index(keymap, XKB_MOD_NAME_SHIFT);
    controlIndex = xkb_keymap_mod_get_index(keymap, XKB_MOD_NAME_CTRL);
    altIndex = xkb_keymap_mod_get_index(keymap, XKB_MOD_NAME_ALT);
}

uint32_t QtKeyboard::keyToScanCode(uint32_t key)
{
    // Convert a Qt key to an XKB scan code
    uint scanCode = 0;
    maybeUpdateXkbScanCodeTable();
    return scanCodesByQtKey.value({0, key}, 0);
}

uint32_t QtKeyboard::toWaylandCode(uint32_t key)
{
    const uint offset = 8;
    if (key < offset) {
        qWarning() << "Key" << key << "is less than offset" << offset;
        return 0; // Invalid key
    }
    return key - offset;
}

QtKeyboard::QtKeyboard(QObject *parent)
    : FakeKeyboard(parent)
{
    m_capslockStateHelper = CapslockStateHelper::instance(this);
    connect(m_capslockStateHelper, &CapslockStateHelper::capslockChanged, this, [=](bool state) {
        qDebug() << " CapsLock changed !!! " << " state = " << state;
        m_capsState = state;
    });
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
    if (m_capsState && c.isLetter()){
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
       sendKey(Qt::Key_CapsLock);
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

void QtKeyboard::sendKey(Qt::Key key, bool shift)
{
    struct xkb_context *ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (!ctx) {
        qWarning() << "Failed to create xkb context";
        xkb_context_unref(ctx);
        return ;
    }

    struct xkb_rule_names names;
    names.rules = "evdev";
    names.model = "pc105";
    names.layout = "us";
    names.variant = "";
    names.options = "";

    keymap = xkb_keymap_new_from_names(ctx, &names, XKB_KEYMAP_COMPILE_NO_FLAGS);
    if (!keymap) {
        qWarning() << "Failed to create xkb keymap";
        xkb_context_unref(ctx);
        xkb_keymap_unref(keymap);
        return ;
    }

    char *keymap_str = xkb_keymap_get_as_string(keymap, XKB_KEYMAP_FORMAT_TEXT_V1);
    if (!keymap_str) {
        qWarning("Failed to compile global XKB keymap");
        xkb_context_unref(ctx);
        xkb_keymap_unref(keymap);
        return ;
    }
    state = xkb_state_new(keymap);

    UkuiVirtualKeyboard *keyboard = new UkuiVirtualKeyboard(keymap_str);
    if (shift) {
        uint32_t shift_scanCode = keyToScanCode(Qt::Key_Shift);
        xkb_state_update_key(state, shift_scanCode, XKB_KEY_DOWN);

        uint32_t modsDepressed = xkb_state_serialize_mods(state, XKB_STATE_MODS_DEPRESSED);
        uint32_t modsLatched = xkb_state_serialize_mods(state, XKB_STATE_MODS_LATCHED);
        uint32_t modsLocked = xkb_state_serialize_mods(state, XKB_STATE_MODS_LOCKED);

        keyboard->modifiers(modsDepressed, modsLatched, modsLocked);
    }

    keyboard->keyPress(toWaylandCode(keyToScanCode(key)));
    keyboard->keyRelease(toWaylandCode(keyToScanCode(key)));

    xkb_state_unref(state);
    xkb_keymap_unref(keymap);
    xkb_context_unref(ctx);
    free(keymap_str);
}
