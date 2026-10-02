#include <QApplication>
#include <gtest/gtest.h>
#include <QHBoxLayout>
#include <QWidget>
#include <QListWidget>
#include <QDebug>
#include "../gesture-extension-manager/gesture-extension-manager.h"

using namespace UkuiGestureExtension;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QWidget* widget = nullptr;



    GestureExtensionManager::getInstance()->registerWidget(widget);

    widget = new QWidget();
    widget->setProperty("notUseSlideGesture", true);
    GestureExtensionManager::getInstance()->registerWidget(widget);
    EXPECT_FALSE(qApp->testAttribute(Qt::AA_SynthesizeTouchForUnhandledMouseEvents));

    qDebug() << widget->inherits("QAbstractScrollArea");
    widget->setProperty("notUseSlideGesture", false);

    GestureExtensionManager::getInstance()->unregisterWidget(widget);

    QListWidget* lwidget = new QListWidget();

    qDebug() << lwidget->inherits("QAbstractScrollArea");
    GestureExtensionManager::getInstance()->registerWidget(lwidget);

    GestureExtensionManager::getInstance()->unregisterWidget(lwidget);

    GestureExtensionManager::getInstance()->release();

    delete widget;
    widget = nullptr;

    return 0;
}
