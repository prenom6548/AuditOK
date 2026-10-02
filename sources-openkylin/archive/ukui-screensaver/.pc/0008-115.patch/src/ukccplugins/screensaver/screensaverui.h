#ifndef SCREENSAVERUI_H
#define SCREENSAVERUI_H

#include <widgets/settinggroup.h>
#include <widgets/switchwidget.h>
#include <widgets/pushbuttonwidget.h>
#include <widgets/comboxwidget.h>
#include "screensaverpluginso.h"
#include "previewwidget.h"
#include <QHBoxLayout>
#include <QWidget>
#include <QRadioButton>
#include <QLineEdit>
#include <QFileDialog>
#include <QButtonGroup>
#include <QFileSystemWatcher>
#include <kysdk/applications/klabel.h>

using namespace kdk;

class ScreensaverUi : public QWidget
{
    Q_OBJECT
public:
    ScreensaverUi();
    ~ScreensaverUi();
    void setIdleTimeOptions(const QStringList &textList, const QList<int> &dataList);
    void setScreensaverOptions(const QStringList &textList, const QStringList &dataList);
    void setSwitchTimeOptions(const QStringList &textList, const QList<int> &dataList);
    void updatePreview(QWidget *w);
    void setSourcePath(const QString &text);
    void setSwitchRandom(const bool &b);
    void setSwitchTime(const int &time);
    void setCustomText(const QString &text);
    void setTextCentered(const bool &b);
    void setShowUkuiTime(const bool &b);
    void setShowCustomTime(const bool &b);
    void setLockscreen(const bool &b);

public:
    void setIdleTime(int time);
    void setScreensaver(QString type);

private:
    void initUi();
    void initPreview();
    void initSetting();
    void initSourceFrame();
    void initSwitchModeFrame();
    void initSwitchTimeFrame();
    void initCustomTextFrame();
    void initTextPositionFrame();
    void initBreakTimeFrame();
    void initScreenlockFrame();
    void sendSignals();

private:
    QVBoxLayout *uiLayout         = nullptr;

private:
    KLabel    *screensaverTitle = nullptr;
    UkccFrame     *previewFrame     = nullptr;
    QHBoxLayout   *previewLayout    = nullptr;
    PreviewWidget *previewWidget    = nullptr;
    SettingGroup  *screensaverGroup = nullptr;
    ComboxWidget  *idleTimeWidget   = nullptr;
    ComboxWidget  *programWidget    = nullptr;
    SettingGroup  *customizeGroup   = nullptr;

private:
    QLineEdit     *sourcePathLine     = nullptr;
    QPushButton   *sourceButton       = nullptr;
    QButtonGroup  *switchModeGroup    = nullptr;
    QComboBox     *switchTimeComboBox = nullptr;
    QLineEdit     *textLineEdit       = nullptr;
    QButtonGroup  *textPositionGroup  = nullptr;
    KSwitchButton *showCustomTimeBtn  = nullptr;
    KSwitchButton *showUkuiTimeBtn    = nullptr;
    KSwitchButton *lockButton         = nullptr;

Q_SIGNALS:
    void idleTimeChanged(int time);
    void screensaverChanged(QString type);
    void customPathChanged(QString path);
    void switchRandomChanged(bool b);
    void switchTimeChanged(int time);
    void customTextChanged(QString text);
    void textCenteredChanged(bool b);
    void showUkuiTimeChanged(bool b);
    void showCustomChanged(bool b);
    void lockscreenChanged(bool b);
};

#endif // SCREENSAVERUI_H
