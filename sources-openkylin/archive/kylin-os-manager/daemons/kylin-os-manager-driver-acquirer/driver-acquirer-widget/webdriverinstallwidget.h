#ifndef KYLIN_OS_MANAGER_WEBDRIVERINSTALLWIDGET_H
#define KYLIN_OS_MANAGER_WEBDRIVERINSTALLWIDGET_H

#include <QString>
#include <QStringList>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QUrl>
#include <QMessageBox>
#include <QPushButton>
#include <QVariant>
#include <kdialog.h>

class WebDriverInstallWidgetPrivate;

class Q_DECL_EXPORT WebDriverInstallWidget : public kdk::KDialog
{
    Q_OBJECT
public:
    explicit WebDriverInstallWidget(const QStringList&, QWidget* parent = nullptr);
    explicit WebDriverInstallWidget(const QString& type, const QString& token, QWidget* parent = nullptr);
    ~WebDriverInstallWidget();

private:
    Q_DECLARE_PRIVATE(WebDriverInstallWidget)
    WebDriverInstallWidgetPrivate*const d_ptr;
};



#endif //KYLIN_OS_MANAGER_WEBDRIVERINSTALLWIDGET_H
