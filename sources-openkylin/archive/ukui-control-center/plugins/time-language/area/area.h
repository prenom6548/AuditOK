/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef AREA_H
#define AREA_H

#include "areaui.h"
#include "shell/interface.h"
#include "interface/mthread.h"
#include "settinggroup.h"
#include "comboxwidget.h"
#include "languagefeatureitem.h"
#include "languagefeatureconfig.h"
#include "languageinstall.h"
#include <QWidget>
#include <QObject>
#include <QtPlugin>
#include <QDBusInterface>
#include <QDBusReply>
#include <QMap>
#include <QStringList>
#include <QEvent>
#include <kysdk/kysdk-system/libkydate.h>
using namespace kdk;
#include "ukcccommon.h"
using namespace ukcc;

/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

typedef QPair <QString, QString> pairString;
typedef QList <QPair<QString, QString>> pairStringList;

typedef struct InteractiveInfo
{
    QString listKey;
    QString key;
    QString setkey;
    pairStringList list;
}InteractiveInfo;

class UkccArgEvent : public QEvent
{
public:
    const static Type TYPE = static_cast<Type>(QEvent::User + 0x10);
    // 保存外部传入的插件参数
    explicit UkccArgEvent(const QString &s) : QEvent(TYPE), mstr(s) {}
    // 获取插件参数字符串
    QString str() const { return mstr; }

private:
    QString mstr;
};

class Area : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ukcc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    Area();
    ~Area();
    QString plugini18nName()   Q_DECL_OVERRIDE;
    int pluginTypes()       Q_DECL_OVERRIDE;
    QWidget * pluginUi()   Q_DECL_OVERRIDE;
    const QString name() const  Q_DECL_OVERRIDE;
    bool isShowOnHomePage() const Q_DECL_OVERRIDE;
    QIcon icon() const Q_DECL_OVERRIDE;
    bool isEnable() const Q_DECL_OVERRIDE;

protected:
    // 处理外部传入的插件事件
    bool event(QEvent *event) Q_DECL_OVERRIDE;

public:
    InteractiveInfo initInteractiveInfo(int orderNum);
    void initComboBox(QComboBox *comboBox, InteractiveInfo info);
    void initContent();
    void initCountry();
    void initCalendar();
    void initFirstDay();
    void initDateFormat(bool firstLoad = true);
    void initTimeFormat(bool firstLoad = false);
    void initLanguage();
    void initAddLanguage();
    void initDateComboBox();
    void initConnect();
    void connectToServer();
    QString showLanguageName(QString languageCode);
    // 应用语言特性安装确认结果
    void applyInstallLanguageResult(const InstallLanguageResult &result);
    // 刷新当前语言下拉框
    void refreshLanguageCombo(const QString &currentLanguageOverride = QString());
    // 响应语言特性手动安装请求
    void onLanguageFeatureInstallRequested(const QString &languageCode, LanguageFeatureType type);
    // 启动单项语言特性安装
    void startLanguageFeatureInstall(const QString &languageCode,
                                     LanguageFeatureType type,
                                     LanguageFeatureItem *item,
                                     const InstallLanguageResult &result);
    // 安装语言字体特性
    bool installLanguageFontFeature(const InstallLanguageResult &result);
    // 安装输入法特性
    bool installInputMethodFeature(const InstallLanguageResult &result);
    // 卸载语言字体特性
    bool uninstallLanguageFontFeature(const QString &languageCode);
    // 卸载输入法特性
    bool uninstallInputMethodFeature(const QString &inputMethodId);
    // 卸载语言相关软件包
    void uninstallLanguagePackages(const QString &languageCode);
    // 确认并删除语言展示项
    void onLanguageDeleteRequested(const QString &languageCode, const QString &languageName);
    // 恢复语言展示项和特性状态
    void restoreLanguageFrame(const QString &languageCode, const QString &languageName, bool isCurrent);
    // 保存语言特性安装状态
    void saveLanguageFeatureConfig(const QString &languageCode,
                                   const InstallLanguageResult &result,
                                   LanguageFeatureState fontState,
                                   LanguageFeatureState inputMethodState);
    // 更新已保存的单项语言特性状态
    void updateSavedFeatureState(const QString &languageCode,
                                 LanguageFeatureType type,
                                 LanguageFeatureState state);
    // 删除语言特性保存配置
    void removeLanguageFeatureConfig(const QString &languageCode);
    // 读取已保存的语言特性安装结果
    InstallLanguageResult savedLanguageFeatureResult(const QString &languageCode,
                                                     const QString &languageName = QString()) const;
    // 查询语言字体特性状态
    LanguageFeatureState queryLanguageFontFeatureState(const InstallLanguageResult &result,
                                                       LanguageFeatureState fallbackState) const;
    // 查询输入法特性状态
    LanguageFeatureState queryInputMethodFeatureState(const InstallLanguageResult &result) const;
    // 判断语言是否缺少可安装特性包
    bool hasMissingLanguageFeaturePackage(const QString &languageCode) const;
    // 更新添加语言列表状态
    void updateAddLanguageRows();
    // 查询网络是否可用
    bool isNetworkAvailable() const;
    void showMessageBox(const int flag);
    void initLanguageFormat();
    void updateShortDate();

public Q_SLOTS:
    void dataChanged(QString key);

private:
    // 延后提示语言重启
    void promptLanguageRestartLater(const QString &languageCode = QString());
    // 结束语言特性安装后的重启提示等待
    void finishLanguageRestartPromptInstall(const QString &languageCode);
    // 显示语言重启确认弹窗
    bool showLanguageRestartMessageBox();
    // 展开当前语言的特性安装入口
    void showCurrentLanguageFeatures();
    // 忽略本次主动写入触发的数据变化
    void ignoreNextDataChanged(const QString &key);
    // 记录语言重启提示状态
    void setLanguageRestartPromptPending(const QString &languageCode, bool pending);
    // 判断语言重启提示是否待处理
    bool isLanguageRestartPromptPending(const QString &languageCode);
    // 判断语言重启是否已生效
    bool isLanguageRestartEffective(const QString &languageCode) const;
    // 判断语言功能状态是否允许提示重启
    bool isLanguageRestartPromptReady(const LanguageFeatureOption &option,
                                      const InstallLanguageResult &result,
                                      LanguageFeatureState fontState,
                                      LanguageFeatureState inputMethodState) const;
    // 恢复待展示的语言重启提示
    void updatePendingLanguageRestartTip(const QString &languageCode,
                                         const LanguageFeatureOption &option,
                                         const InstallLanguageResult &result,
                                         LanguageFeatureState fontState,
                                         LanguageFeatureState inputMethodState);

private slots:
    void cloudChangedSlot(const QString &key);
    void TimeChanged(QString time);
    void refreshLongDate(QString date);
    void refreshShortDate(QString date);
    void setVisibleBySecurity();
    void setEnableBySecurity();
    // 处理语言特性安装完成
    void onLanguageFeatureInstallFinished(const LanguageFeatureInstallTask &task,
                                          const PackageInstallResult &packageResult);

private:
    AreaUi          *areaWidget          = nullptr;
    QDBusConnection *areaInerfaceConnect = nullptr;
    QDBusInterface  *areaInterface       = nullptr;
    QString         changeKey            = "";
    QStringList     m_pendingSelfChangedKeys;
    bool            mFirstLoad           = true;
    QString         pluginName           = "";
    int             pluginType           = 0;
    QString m_LunarDate;
    bool m_shortDateNeedChaned = true;
    QMap<QString, InstallLanguageResult> m_pendingInstallResults;
    QMap<QString, int> m_pendingRestartPromptInstallCounts;
    LanguageInstall *m_languageInstall = nullptr;
};

#endif // AREA_H
