#ifndef IAPPVOLUMEWIDGET_H
#define IAPPVOLUMEWIDGET_H

#include <QWidget>
#include <QString>

class IAppVolumeWidget {
public:
    IAppVolumeWidget() = default;
    virtual ~IAppVolumeWidget() {}

public:
    virtual void initUi() = 0;

    virtual void showWidget() = 0;

    virtual QWidget* getWidget() = 0;

    virtual void addAppWidget(int) = 0;

    virtual void removeAppWidget(int) = 0;

    virtual void addAppToWidget(const QString&, const QString&) = 0;

    virtual void removeAppfromWidget(const QString&) = 0;
};

#endif // IAPPVOLUMEWIDGET_H
