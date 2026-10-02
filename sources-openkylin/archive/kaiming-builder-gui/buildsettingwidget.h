#ifndef BUILDSETTINGWIDGET_H
#define BUILDSETTINGWIDGET_H


#include <QWidget>
#include <QScopedPointer>
#include <QCheckBox>
#include <logic/BuildSettingManager.h>

class BuildSettingWidgetPrivate;

class BuildSettingWidget : public QWidget
{
    Q_OBJECT
public:
    BuildSettingWidget(QWidget *parent = nullptr, BuildSettingManager *buildSettingManager = nullptr);
    virtual ~BuildSettingWidget();

    QString getCommandString();

signals:
    void back();
    void next();

private:
    Q_DECLARE_PRIVATE(BuildSettingWidget)
    QScopedPointer<BuildSettingWidgetPrivate> d_ptr;
};

#endif // BUILDSETTINGWIDGET_H
