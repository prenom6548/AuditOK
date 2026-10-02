#ifndef AREAUI_H
#define AREAUI_H

#include "languageframe.h"
#include "languagefeatureitem.h"
#include "addlanguagedialog.h"
#include "addinputmethoddialog.h"
#include "installlanguagefeaturesdialog.h"
#include "languagerestartbar.h"
#include "settinggroup.h"
#include "comboxwidget.h"
#include "lightlabel.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QComboBox>
#include "ukcccommon.h"
#include <klabel.h>
#include <kpushbutton.h>

using namespace ukcc;
using namespace kdk;

class AreaUi : public QWidget
{
    Q_OBJECT

public:
    enum ComboBoxOrder {
        COUNTRY = 0,
        CALENDAR,
        FIRSTDAY,
        DATEFORMAT,
#ifdef Nile
        SHORTDATEFORMAT,
        LONGDATEFORMAT,
#endif
        TIMEFORMAT
    };
    AreaUi(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    virtual ~AreaUi();
    void initUi();
    void initLanguageFormat();
    void initSystemLanguage();
    void settingForIntel();

public:
    inline QComboBox *countryComboBox()
    {
        if (countryWidget) {
            return countryWidget->comboBox();
        } else {
            return nullptr;
        }
    }

    inline QComboBox *calendarComboBox()
    {
        if (calendarWidget) {
            return calendarWidget->comboBox();
        } else {
            return nullptr;
        }
    }

    inline QComboBox *firstDayComboBox()
    {
        if (dayWidget) {
            return dayWidget->comboBox();
        } else {
            return nullptr;
        }
    }

    inline QComboBox *dateComboBox()
    {
        if (shortDateWidget) {
            return shortDateWidget->comboBox();
        } else {
            return nullptr;
        }
    }

    inline QComboBox *shortDateComboBox()
    {
        if (shortDateWidget) {
            return shortDateWidget->comboBox();
        } else {
            return nullptr;
        }
    }

    inline QComboBox *longDateComboBox()
    {
        if (longDateWidget) {
            return longDateWidget->comboBox();
        } else {
            return nullptr;
        }
    }

    inline QComboBox *timeComboBox()
    {
        if (timeWidget) {
            return timeWidget->comboBox();
        } else {
            return nullptr;
        }
    }

    inline KPushButton *addbutton()
    {
        return addLanguageBtn;
    }

    inline AddLanguageDialog *languageDialog()
    {
        return addLanguageDialog;
    }

    inline AddInputMethodDialog *inputMethodDialog()
    {
        return addInputMehtodDialog;
    }

    // 获取语言特性安装弹窗
    inline InstallLanguageFeaturesDialog *installLanguageFeaturesDialog()
    {
        return installLanguageFeaturesDlg;
    }

    inline UkccFrame* formatDateFrame() {
        return m_formatDateFrame;
    }

    inline KLabel* formatDateValueLabel() {
        return m_formatDateValueLabel;
    }

    inline KLabel* getLanguageTitleLable() {
        return languageTitleLable;
    }

    inline SettingGroup* getlanguageGroup() {
        return languageGroup;
    }

    // 获取当前语言下拉框
    inline QComboBox *currentLanguageComboBox() {
        return currentLanguageCombo;
    }

    // 更新当前语言下拉框内容
    void updateCurrentLanguageCombo(const QStringList &languageCodes,
                                    const QStringList &languageNames,
                                    const QString &currentLanguageCode);
    // 设置当前语言选中项
    void setCurrentLanguageCode(const QString &languageCode);
    // 显示语言重启提示
    void showLanguageRestartTip();
    // 隐藏语言重启提示
    void hideLanguageRestartTip();
    // 更新语言重启提示样式
    void updateLanguageRestartStyle();
    // 添加语言展示项
    void addShowLanguage(const QString &languageCode, const QString &languageName, bool isCurrent = false);
    // 添加带特性的语言展示项
    LanguageFrame *addShowLanguageWithFeatures(const QString &languageCode,
                                               const QString &languageName,
                                               bool isCurrent = false);
    // 折叠所有语言展示项
    void collapseAllLanguageFrames();
    // 查找指定语言展示项
    LanguageFrame *languageFrame(const QString &languageCode) const;
    // 移除指定语言展示项
    void removeLanguageFrame(const QString &languageCode);
    void clearShowLanguage();
    void reloadLanguageFrameIcon();

protected:
    // 处理重启链接点击事件
    bool eventFilter(QObject *watched, QEvent *event) override;

Q_SIGNALS:
    void languageChanged(QString languageCode);
    void showLanguageListRemoved(QString languageCode, QString languageName);
    // 请求删除指定语言
    void languageDeleteRequested(QString languageCode, QString languageName);
    // 请求安装指定语言特性
    void languageFeatureInstallRequested(const QString &languageCode, LanguageFeatureType type);
    // 请求立即重启系统
    void languageRestartRequested();

private:
    QVBoxLayout *uiLayout       = nullptr;
    QVBoxLayout *formatLayout   = nullptr;
    QVBoxLayout *LanguageLayout = nullptr;

private:
    KLabel   *formatTitleLabel = nullptr;
    SettingGroup *formatGroup      = nullptr;
    ComboxWidget *countryWidget    = nullptr;
    ComboxWidget *calendarWidget   = nullptr;
    ComboxWidget *dayWidget        = nullptr;
    ComboxWidget *shortDateWidget  = nullptr;
    ComboxWidget *longDateWidget   = nullptr;
    ComboxWidget *timeWidget       = nullptr;

private:
    KLabel           *languageTitleLable       = nullptr;
    LanguageRestartBar *languageRestartBar       = nullptr;
    QWidget          *languageRestartSpacer      = nullptr;
    KLabel           *languageRestartIcon        = nullptr;
    KLabel           *languageRestartTipLabel  = nullptr;
    KLabel           *languageRestartLink        = nullptr;
    SettingGroup     *languageGroup            = nullptr;
    UkccFrame        *currentLanguageFrame       = nullptr;
    QComboBox        *currentLanguageCombo       = nullptr;
    UkccFrame        *moreLanguageFrame          = nullptr;
    KPushButton      *addLanguageBtn             = nullptr;
    AddLanguageDialog    *addLanguageDialog    = nullptr;
    AddInputMethodDialog *addInputMehtodDialog = nullptr;
    InstallLanguageFeaturesDialog *installLanguageFeaturesDlg = nullptr;
    UkccFrame* m_formatDateFrame = nullptr;
    KLabel* m_formatDateNameLabel = nullptr;
    KLabel* m_formatDateValueLabel = nullptr;

private:
    QList<LanguageFrame*> languageFrameList = {};
};

#endif // AREAUI_H
