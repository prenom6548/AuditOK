/*
 * Copyright 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "subsysteminstallmodule.h"

#include <QApplication>
#include <QDebug>
#include <QLabel>
#include <QTimer>
#include <QTranslator>

SubSystemInstallModule::SubSystemInstallModule(QObject *parent)
    : QObject(parent) {
    QTranslator *translator = new QTranslator(this);
    translator->load("/usr/share/kylin-ai-subsystem-plugin/translations/" +
                     QLocale::system().name());
    QApplication::installTranslator(translator);

    pluginName = tr("AI Models");
    pluginType = COMMONINFO;
}

QString SubSystemInstallModule::plugini18nName() { return pluginName; }

int SubSystemInstallModule::pluginTypes() { return pluginType; }

QWidget *SubSystemInstallModule::pluginUi() {
    if (mFirstLoad) {
        initUI();
        connectSignals();
        mFirstLoad = false;
        QTimer::singleShot(100,
                           [this]() { subSystemWidget->continueLoading(); });
    }
    return mainFrame;
}

bool SubSystemInstallModule::isEnable() const { return true; }

const QString SubSystemInstallModule::name() const { return "aisubsystem"; }

bool SubSystemInstallModule::isShowOnHomePage() const { return true; }

QIcon SubSystemInstallModule::icon() const {
    return QIcon::fromTheme("ukui-ai-module-manager-symbolic");
}

QString SubSystemInstallModule::translationPath() const {
    return "/usr/share/kylin-ai-subsystem-plugin/translations/%1.ts";
}

void SubSystemInstallModule::initUI() {
    mainFrame = new QFrame();
    mainFrame->setAccessibleName(
        "kylin-ai-subsystem-plugin_QFrame_subsystemMainInterface");
    mainVLayout = new QVBoxLayout();
    mainVLayout->setSpacing(0);
    mainVLayout->setContentsMargins(0, 0, 0, 0);
    mainVLayout->setObjectName(
        "kylin-ai-subsystem-plugin_QVBoxLayout_subsystemMainLayout");
    mainFrame->setLayout(mainVLayout);

    subSystemWidget = new SubSystemWidget(mainFrame);

    mainWidget = subSystemWidget->mainWidget();
    if (mainWidget) {
        mainVLayout->addWidget(mainWidget);
    }

    loadingPluginText = new QLabel(mainFrame);
    loadingPluginText->setContentsMargins(0, 20, 0, 0);
    loadingPluginText->setText(tr("loading plugin"));
    loadingPluginText->setAlignment(Qt::AlignCenter);
    loadingPluginText->hide();

    pluginVLayout = new QVBoxLayout();
    pluginVLayout->setSpacing(0);
    pluginVLayout->setContentsMargins(0, 0, 0, 0);

    mainVLayout->addWidget(loadingPluginText);
    mainVLayout->addLayout(pluginVLayout);
    mainVLayout->addStretch();
}

void SubSystemInstallModule::connectSignals() {
    connect(subSystemWidget, &SubSystemWidget::uninstallPkgSuccess, this,
            [this]() {
                if (modelConfigWidget) {
                    modelConfigWidget->hide();
                }
            });

    connect(subSystemWidget, &SubSystemWidget::modelConfigLoaded, this,
            [this]() {
                modelConfigWidget = subSystemWidget->modelConfigWidget();
                modelConfigWidget->setContentsMargins(0, 40, 0, 0);
                if (modelConfigWidget) {
                    pluginVLayout->addWidget(modelConfigWidget);
                    if (!subSystemWidget->runtimeProcessExists()) {
                        modelConfigWidget->hide();
                    }
                }

                computePowerSharingWidget =
                    subSystemWidget->computePowerSharingWidget();
                if (computePowerSharingWidget) {
                    pluginVLayout->addWidget(computePowerSharingWidget);
                    if (!subSystemWidget->runtimeProcessExists()) {
                        computePowerSharingWidget->hide();
                    }
                }

                loadingPluginText->hide();
            });

    connect(subSystemWidget, &SubSystemWidget::loadingPlugin, this,
            [this]() { loadingPluginText->show(); });
}

void SubSystemInstallModule::initSearchText() {
    //~ contents_path /aisubsystem/AI SubSystem
    tr("AI SubSystem");
    //~ contents_path /aisubsystem/Model Configuration
    tr("Model Configuration");
}
