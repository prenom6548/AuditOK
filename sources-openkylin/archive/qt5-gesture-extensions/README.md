# qt5-gesture-extensions

    qt5-gesture-extensions are some dynamic libraries, provide some gesture extensions 

## Depends

    debhelper-compat (= 12),
    qtbase5-dev-tools,
    qttools5-dev-tools,
    qtbase5-dev.
    qt5-qmake

## Build&Install

```bash
    mkdir build

    cd build

    qmake ..

    make

    sudo make install # install
```

## Include

If you want to run-time loading, please call registerWidget, is a `c-style` interface

or if you want to load at compile time, just `#include <qt5-gesture-extensions/qt5-gesture-extensions.h>`

## Interface

    c-style interface:

```C++
    extern "C" void registerWidget(QWidget* widget, QObject* parent);
    extern "C" void unregisterWidget(QWidget* widget, QObject* parent);
```

    c++ interface:
```C++
namespace UkuiGestureExtension {
class QT5GESTUREEXTENSIONS_EXPORT GestureExtensionManager : public QObject
{
public:
    ~GestureExtensionManager();
    /*!
     * \brief registerWidget
     * \details register the widget to all extension
    */
    void registerWidget(QWidget* widget);
    /*!
     * \brief unregisterWidget
     * \details unregister the widget to all extension
    */
    void unregisterWidget(QWidget* widget);

    static GestureExtensionManager* getInstance(QObject* parent = nullptr);
};

} // namespace
```

you can use 
```C++
    bool QObject::setProperty(const char *name, const QVariant &value);
```
to ban some extension for your widget, such as:
```C++
    setProperty("notUseSlideGesture", true); // ban Slide List View Extension for your widget
```
## Extensions List

    * SlideListViewGesture
        make your QAbstractItemView can scroll when you touch
        property:notUseSlideGesture

## Core Logic

    PlantformTheme will called
```C++
    void registerWidget(QWidget* widget);
```
    to register all widgets to Gesture Extensions Manager, Manager will register all widget to all of extensions:
```C++
    void GestureExtensionManagerPrivate::registerWidget(QWidget* widget)
    {
        for (auto begin = m_extensions.begin(); begin != m_extensions.end(); ++begin) {
            if ((*begin)->isWidgetUseExtension(widget))
                (*begin)->registerWidget(widget);
        }
    }
```

## Slide Gesture Extension

    The Slide Gesture Extension inherited from gesture extension, it will install the event filter for the registered widget.
```C++
    void SlideGesture::registerWidget(QWidget *widget)
    {
        if (!widget) {
            qDebug() << __FILE__ << __FUNCTION__ << " widget is empty";
            return;
        }

        widget->installEventFilter(this);
    }
```
    In the event filter, install QScroller on the scrollarea that wants to support touch scrolling, and ignore the non-scrollarea

## TODO

    * Load extension from extension dir
