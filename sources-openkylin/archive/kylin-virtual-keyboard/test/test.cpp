#include <QApplication>
#include <QQuickView>

#include <cassert>

#include "animation/animationfactory.h"
#include "animation/disabledanimator.h"
#include "animation/enabledanimator.h"
#include "animation/expansionanimationfactory.h"
#include "animation/floatanimationfactory.h"
#include "appinputareamanager.h"
#include "ipc/dbusservice.h"
#include "ipc/fcitxvirtualkeyboardserviceproxy.h"
#include "localsettings/viewlocalsettings.h"
#include "screenmanager.h"
#include "virtualkeyboard/virtualkeyboardmanager.h"
#include "virtualkeyboardentry/virtualkeyboardtrayicon.h"
#include "virtualkeyboardsettings/virtualkeyboardsettings.h"

void testGetPrimaryScreenGeometry() {
    auto rect = ScreenManager::getPrimaryScreenGeometry();
    assert(!rect.isEmpty());
}

void testGetPrimaryScreenSize() {
    auto rect = ScreenManager::getPrimaryScreenSize();
    assert(!rect.isEmpty());
}

void testVirtualKeyboardSettings() {
    auto &settings = VirtualKeyboardSettings::getInstance();
    assert(settings.isAnimationEnabled());

    settings.updateFloatButtonAvailability(true);
    assert(settings.isFloatButtonEnabled());
    settings.emitFloatButtonAvailabilityChanged();

    settings.updateFloatButtonAvailability(false);
    assert(!settings.isFloatButtonEnabled());
    settings.emitFloatButtonAvailabilityChanged();

    auto scaleFactor = settings.calculateVirtualKeyboardScaleFactor();
    assert(scaleFactor > 0.1);
}

std::unique_ptr<AnimationFactory> createFloatModeAnimationFactory() {
    return std::unique_ptr<AnimationFactory>(new FloatAnimationFactory());
}

std::unique_ptr<AnimationFactory> createExpansionModeAnimationFactory() {
    return std::unique_ptr<AnimationFactory>(new ExpansionAnimationFactory());
}

void testAnimator(Animator *animator) {
    auto *view = new QQuickView();

    animator->playShowAnimation(view, {0, 0, 100, 100});
    animator->playHideAnimation(view, {0, 0, 0, 0});
    animator->playFlipAnimation(view, {0, 0, 200, 200});
}

void testVirtualKeyobardAnimation() {
    auto floatModeEnabledAnimator =
        std::unique_ptr<EnabledAnimator>(new EnabledAnimator(
            []() { return true; }, createFloatModeAnimationFactory()));
    testAnimator(floatModeEnabledAnimator.get());

    auto expansionModeEnabledAnimator =
        std::unique_ptr<EnabledAnimator>(new EnabledAnimator(
            []() { return false; }, createExpansionModeAnimationFactory()));
    testAnimator(expansionModeEnabledAnimator.get());

    auto disabledAnimator =
        std::unique_ptr<DisabledAnimator>(new DisabledAnimator());
    testAnimator(disabledAnimator.get());
}

void testAppInputAreaManager() {
    auto appInputAreaManager =
        std::unique_ptr<AppInputAreaManager>(new AppInputAreaManager());

    appInputAreaManager->raiseInputArea({100, 100, 100, 100});
    appInputAreaManager->fallInputArea();
}

void testVirtualKeyboardProxy() {
    auto proxy = std::unique_ptr<FcitxVirtualKeyboardServiceProxy>(
        new FcitxVirtualKeyboardServiceProxy());

    proxy->showVirtualKeyboard();
    proxy->hideVirtualKeyboard();
}

void testViewLocalSettings() {
    ViewLocalSettings localSetting("test", "test");
    localSetting.setValue("test", "a", "1");
    localSetting.saveSettingsAsync();
    localSetting.getValue("test", "a", "1");
    localSetting.getValue("test", "a");
    localSetting.remove("a");

    localSetting.setValue("test", "b", "2");
    localSetting.saveSettingsAsync();
    localSetting.getValue("test", "b", "2");
    localSetting.getValue("test", "b");
    localSetting.remove("b");
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    testGetPrimaryScreenGeometry();

    testGetPrimaryScreenSize();

    testVirtualKeyboardSettings();

    testVirtualKeyobardAnimation();

    testAppInputAreaManager();

    testVirtualKeyboardProxy();

    testViewLocalSettings();

    return 0;
}
