#include "mainwindow.h"
#include <functional>
#include <glib.h>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDir>
#include <QDebug>
#include <QListWidgetItem>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDBusPendingReply>
#include <QDBusPendingCallWatcher>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include "app_item.h"
#include "defines.h"

namespace tool_box {

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent),
      category_tab_bar_(new kdk::KTabBar(kdk::KTabBarStyle::SegmentLight, this)),
      search_line_edit_(new kdk::KSearchLineEdit(this)),
      stacked_widget_(new QStackedWidget(this)),
      feature_tools_list_widget_(new QListWidget(this)),
      debug_tools_list_widget_(new QListWidget(this)),
      troubleshooting_tools_list_widget_(new QListWidget(this)),
      other_tools_list_widget_(new QListWidget(this)),
      blank_feature_tools_widget_(new BlankPage(this)),
      blank_debug_tools_widget_(new BlankPage(this)),
      blank_troubleshooting_tools_widget_(new BlankPage(this)),
      blank_other_tools_widget_(new BlankPage(this)),
      stacked_feature_tools_widget_(new QStackedWidget(this)),
      stacked_debug_tools_widget_(new QStackedWidget(this)),
      stacked_troubleshooting_tools_widget_(new QStackedWidget(this)),
      stacked_other_tools_widget_(new QStackedWidget(this))
{
    qRegisterMetaType<tool_box::AppItem::SyncType>();

    search_line_edit_->setCompleter(nullptr);

    SetUpListWidget(feature_tools_list_widget_);
    SetUpListWidget(debug_tools_list_widget_);
    SetUpListWidget(troubleshooting_tools_list_widget_);
    SetUpListWidget(other_tools_list_widget_);

    // 创建选项卡
    CreateTools();

    // 创建索引对应关系
    if (feature_tools_list_widget_->count()) {
        category_tab_bar_->addTab(tr("Feature tools"));
        AddChildWidgetToStackedWidget(stacked_feature_tools_widget_, feature_tools_list_widget_, blank_feature_tools_widget_);
        stacked_widget_->addWidget(stacked_feature_tools_widget_);
    } else {
        feature_tools_list_widget_->hide();
        blank_feature_tools_widget_->hide();
        stacked_feature_tools_widget_->hide();
    }
    if (debug_tools_list_widget_->count()) {
        category_tab_bar_->addTab(tr("Debug tools"));
        AddChildWidgetToStackedWidget(stacked_debug_tools_widget_, debug_tools_list_widget_, blank_debug_tools_widget_);
        stacked_widget_->addWidget(stacked_debug_tools_widget_);
    } else {
        debug_tools_list_widget_->hide();
        blank_debug_tools_widget_->hide();
        stacked_debug_tools_widget_->hide();
    }
    if (troubleshooting_tools_list_widget_->count()) {
        category_tab_bar_->addTab(tr("Troubleshooting tools"));
        AddChildWidgetToStackedWidget(stacked_troubleshooting_tools_widget_, troubleshooting_tools_list_widget_, blank_troubleshooting_tools_widget_);
        stacked_widget_->addWidget(stacked_troubleshooting_tools_widget_);
    } else {
        troubleshooting_tools_list_widget_->hide();
        blank_troubleshooting_tools_widget_->hide();
        stacked_troubleshooting_tools_widget_->hide();
    }
    if (other_tools_list_widget_->count()) {
        category_tab_bar_->addTab(tr("Other tools"));
        AddChildWidgetToStackedWidget(stacked_other_tools_widget_, other_tools_list_widget_, blank_other_tools_widget_);
        stacked_widget_->addWidget(stacked_other_tools_widget_);
    } else {
        other_tools_list_widget_->hide();
        blank_other_tools_widget_->hide();
        stacked_other_tools_widget_->hide();
    }
    // 默认显示第一项
    if (category_tab_bar_->count() && stacked_widget_->count()) {
        category_tab_bar_->setCurrentIndex(0);
        stacked_widget_->setCurrentIndex(0);
    }

    category_tab_bar_->setFixedSize(category_tab_bar_->count() * 100, 36);
    search_line_edit_->setFixedWidth(240);
    auto *category_tab_bar_hbox_layout = new QHBoxLayout;
    category_tab_bar_hbox_layout->setSpacing(0);
    category_tab_bar_hbox_layout->setContentsMargins(40, 0, 40, 0);
    category_tab_bar_hbox_layout->addWidget(category_tab_bar_);
    category_tab_bar_hbox_layout->addStretch();
    category_tab_bar_hbox_layout->addWidget(search_line_edit_);

    auto *stacked_widget_hbox_layout = new QHBoxLayout;
    stacked_widget_hbox_layout->setSpacing(0);
    stacked_widget_hbox_layout->setContentsMargins(32, 0, 0, 0);
    stacked_widget_hbox_layout->addWidget(stacked_widget_);

    auto *vbox_layout = new QVBoxLayout;
    vbox_layout->setSpacing(0);
    vbox_layout->setContentsMargins(0, 24, 0, 0);
    vbox_layout->addLayout(category_tab_bar_hbox_layout);
    vbox_layout->addSpacing(8);
    vbox_layout->addLayout(stacked_widget_hbox_layout);
    setLayout(vbox_layout);

    connect(category_tab_bar_, &kdk::KTabBar::currentChanged, this, [this](int index) {
        stacked_widget_->setCurrentIndex(index);
    });

    connect(search_line_edit_, &kdk::KSearchLineEdit::textChanged, this, &MainWindow::SearchCategoryTools);
}

MainWindow::~MainWindow() {

}

void MainWindow::SetUpListWidget(QListWidget *list_widget) {
    list_widget->resize(800, 460);
    list_widget->setFocusPolicy(Qt::NoFocus);
    list_widget->setViewMode(QListWidget::ViewMode::IconMode);
    list_widget->setFrameShape(QFrame::NoFrame);
    list_widget->setMovement(QListWidget::Movement::Static);
    list_widget->setFlow(QListWidget::Flow::LeftToRight);
    list_widget->setWrapping(true);
    list_widget->setSpacing(8);
}

void MainWindow::UpdateInterface() {
    if (stacked_widget_->indexOf(stacked_feature_tools_widget_) == -1 && feature_tools_list_widget_->count()) {
        category_tab_bar_->addTab(tr("Feature tools"));
        AddChildWidgetToStackedWidget(stacked_feature_tools_widget_, feature_tools_list_widget_, blank_feature_tools_widget_);
        stacked_widget_->addWidget(stacked_feature_tools_widget_);
    }
    if (stacked_widget_->indexOf(stacked_debug_tools_widget_) == -1 && debug_tools_list_widget_->count()) {
        category_tab_bar_->addTab(tr("Debug tools"));
        AddChildWidgetToStackedWidget(stacked_debug_tools_widget_, debug_tools_list_widget_, blank_debug_tools_widget_);
        stacked_widget_->addWidget(stacked_debug_tools_widget_);
    }
    if (stacked_widget_->indexOf(stacked_troubleshooting_tools_widget_) == -1 && troubleshooting_tools_list_widget_->count()) {
        category_tab_bar_->addTab(tr("Troubleshooting tools"));
        AddChildWidgetToStackedWidget(stacked_troubleshooting_tools_widget_, troubleshooting_tools_list_widget_, blank_troubleshooting_tools_widget_);
        stacked_widget_->addWidget(stacked_troubleshooting_tools_widget_);
    }
    if (stacked_widget_->indexOf(stacked_other_tools_widget_) == -1 && other_tools_list_widget_->count()) {
        category_tab_bar_->addTab(tr("Other tools"));
        AddChildWidgetToStackedWidget(stacked_other_tools_widget_, other_tools_list_widget_, blank_other_tools_widget_);
        stacked_widget_->addWidget(stacked_other_tools_widget_);
    }
    category_tab_bar_->setFixedSize(category_tab_bar_->count() * 100, 36);
}

void MainWindow::AddChildWidgetToStackedWidget(QStackedWidget *stacked_widget, QListWidget *list_widget, BlankPage *blank_page) {
    if (stacked_widget == nullptr || list_widget == nullptr || blank_page == nullptr) {
        return;
    }
    if (stacked_widget->indexOf(list_widget) == -1 && stacked_widget->indexOf(blank_page) == -1 && list_widget->count() > 0) {
        stacked_widget->addWidget(list_widget);
        stacked_widget->addWidget(blank_page);
        stacked_widget->setCurrentIndex(0);
    } else {
        return;
    }
    if (list_widget->isHidden()) {
        list_widget->setHidden(false);
    }
    if (blank_page->isHidden()) {
        blank_page->setHidden(false);
    }
    if (stacked_widget->isHidden()) {
        stacked_widget->setHidden(false);
    }
}

void MainWindow::SearchCategoryTools(const QString &text)
{
    SearchListWidgetTools(stacked_feature_tools_widget_, feature_tools_list_widget_, text);
    SearchListWidgetTools(stacked_debug_tools_widget_, debug_tools_list_widget_, text);
    SearchListWidgetTools(stacked_troubleshooting_tools_widget_, troubleshooting_tools_list_widget_, text);
    SearchListWidgetTools(stacked_other_tools_widget_, other_tools_list_widget_, text);
}

void MainWindow::SearchListWidgetTools(QStackedWidget *stacked_widget, QListWidget *list_widget, const QString &text)
{
    bool search_result = false;

    if (list_widget->count() > 0) {
        for (int i = 0; i < list_widget->count(); ++i) {
            auto *item = list_widget->item(i);
            if (text == "") {
                item->setHidden(false);
                search_result = true;
                continue;
            }
            // 获取并使用对应列表项的widget
            AppItem *app_item = qobject_cast<AppItem*>(list_widget->itemWidget(list_widget->item(i)));
            if (app_item) {
                if (app_item->GetToolName().contains(text, Qt::CaseInsensitive)) {
                    item->setHidden(false);
                    search_result = true;
                } else {
                    item->setHidden(true);
                }
            }
        }
    }

    // 按照搜索结果，切换到对应的页面
    if (search_result) {
        stacked_widget->setCurrentIndex(0);
    } else {
        stacked_widget->setCurrentIndex(1);
    }
}

void MainWindow::SetUpListWidgetItem(QListWidgetItem *item) {
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
    item->setSizeHint(QSize(360, 100));
}

void MainWindow::CreateTools() {
    CreateToolsFromConfFile();
    CreateToolsFromDBus();
}

void MainWindow::CreateToolsFromConfFile() {
    QDir dir("/usr/share/kylin-os-manager/plugins/tool-box/plugins/");
    if (!dir.exists()) {
        qCritical() << "Tool box plugins dir is not exists.";
        return;
    }
    auto files_info = dir.entryInfoList(QDir::Filter::NoDotAndDotDot | QDir::Filter::Files);
    for (const auto &file_info: files_info) {
        g_autoptr(GError) error = NULL;
        g_autoptr(GKeyFile) key_file = g_key_file_new();

        if (!g_key_file_load_from_file(key_file, file_info.absoluteFilePath().toStdString().c_str(), G_KEY_FILE_NONE, &error)) {
            qCritical() << "Tool box load plugin conf file fail: " << error->message;
            continue;
        }
        // 获取名字
        g_autofree gchar *name = g_key_file_get_locale_string(key_file, "ToolBox Entry", "Name", NULL, &error);
        if (name == NULL && error != NULL) {
            qCritical() << "Tool box [" << file_info.fileName() << "] plugin get name fail: " << error->message;
            continue;
        }
        //获取描述
        g_autofree gchar *comment = g_key_file_get_locale_string(key_file, "ToolBox Entry", "Comment", NULL, &error);
        if (comment == NULL && error != NULL) {
            qCritical() << "Tool box [" << file_info.fileName() << "] plugin get comment fail: " << error->message;
            continue;
        }
        // 获取图标
        g_autofree gchar *icon = g_key_file_get_string(key_file, "ToolBox Entry", "Icon", &error);
        if (icon == NULL && error != NULL) {
            qCritical() << "Tool box [" << file_info.fileName() << "] plugin get icon fail: " << error->message;
            continue;
        }
        // 获取执行命令
        g_autofree gchar *exec = g_key_file_get_string(key_file, "ToolBox Entry", "Exec", &error);
        if (exec == NULL && error != NULL) {
            qCritical() << "Tool box [" << file_info.fileName() << "] plugin get exec fail: " << error->message;
            continue;
        }

        auto *app_item = new AppItem(AppItem::Type::kExec, this);
        app_item->setSizeIncrement(360, 100);
        app_item->SetIcon(icon);
        app_item->SetName(name);
        app_item->SetComment(comment);
        app_item->SetExecConfigFilePath(file_info.absoluteFilePath());

        QString command{exec};
        auto command_list = command.split(' ');
        command_list.removeAll("");
        app_item->SetExec(command_list);

        // 获取依赖包
        gsize length = 0;
        gchar **depends = g_key_file_get_string_list(key_file, "ToolBox Entry", "Depends", &length, NULL);
        if (length > 0) {
            QList<QString> depend_packages;
            for (gsize i = 0; i < length; i++) {
                depend_packages.push_back(depends[i]);
            }
            app_item->SetDependPackages(depend_packages);
            g_strfreev(depends);
        } else {
            app_item->SetDependPackages(QList<QString>());
        }

        // 获取用户手册
        g_autoptr(GError) user_manual_error = NULL;
        g_autofree gchar *user_manual = g_key_file_get_string(key_file, "ToolBox Entry", "UserManual", &user_manual_error);
        if (user_manual_error == NULL && user_manual != NULL) {
            app_item->SetUserManualName(user_manual);
        }

        // 分类
        g_autoptr(GError) category_error = NULL;
        g_autofree gchar *category = g_key_file_get_string(key_file, "ToolBox Entry", "Categories", &category_error);
        if (category_error == NULL) {
            if (QString(category) == "Feature") {
                // 特色工具
                app_item->SetCategory(AppItem::Category::kFeature);

                auto *item = new QListWidgetItem(feature_tools_list_widget_);
                SetUpListWidgetItem(item);
                feature_tools_list_widget_->setItemWidget(item, app_item);
            } else if (QString(category) == "Debug") {
                // 运维调试类工具
                app_item->SetCategory(AppItem::Category::kDebug);

                auto *item = new QListWidgetItem(debug_tools_list_widget_);
                SetUpListWidgetItem(item);
                debug_tools_list_widget_->setItemWidget(item, app_item);
            } else if (QString(category) == "Troubleshooting") {
                // 故障修复工具
                app_item->SetCategory(AppItem::Category::kTroubleshooting);

                auto *item = new QListWidgetItem(troubleshooting_tools_list_widget_);
                SetUpListWidgetItem(item);
                troubleshooting_tools_list_widget_->setItemWidget(item, app_item);
            } else {
                // 其他分类
                app_item->SetCategory(AppItem::Category::kOther);

                auto *item = new QListWidgetItem(other_tools_list_widget_);
                SetUpListWidgetItem(item);
                other_tools_list_widget_->setItemWidget(item, app_item);
            }
        } else {
            qCritical() << "Tool box [" << file_info.fileName() << "] plugin get category fail: " << category_error->message;
            // 其他分类
            app_item->SetCategory(AppItem::Category::kOther);

            auto *item = new QListWidgetItem(other_tools_list_widget_);
            SetUpListWidgetItem(item);
            other_tools_list_widget_->setItemWidget(item, app_item);
        }
    }
}

void MainWindow::CreateToolsFromDBus() {
    const auto message = QDBusMessage::createMethodCall(TREASUREBOX_DBUS_NAME,
                                                        TREASUREBOX_DBUS_PATH,
                                                        TREASUREBOX_DBUS_INTERFACE,
                                                        "QueryAllApps");
    auto pend = QDBusConnection::sessionBus().asyncCall(message);
    auto *pend_watcher = new QDBusPendingCallWatcher(pend, this);
    connect(pend_watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *watcher) {
        if (!watcher->isValid() || watcher->isError()) {
            qCritical() << "Tool box DBus create tools fail: share object is not valid.";
            return;
        }

        QDBusPendingReply<QString> reply = *watcher;
        if (!reply.isValid() || reply.isError()) {
            qCritical() << "Tool box DBus create tools fail: get return value fail.";
            return;
        }
        if (reply.count() != 1) {
            qCritical() << "Tool box DBus create tools fail: D-Bus return vaule size error.";
            return;
        }
        auto json_data = reply.argumentAt<0>();

        QJsonParseError error;
        auto doc = QJsonDocument::fromJson(json_data.toLocal8Bit(), &error);
        if (doc.isNull()) {
            qCritical() << "Tool box DBus create tools fail: parse json data error.";
            return;
        }
        if (!doc.isArray()) {
            qCritical() << "Tool box D-Bus create tools fail: json format error.";
            return;
        }
        const auto array = doc.array();
        for (const auto &i: array) {
            if (!i.isObject()) {
                continue;
            }
            const auto obj = i.toObject();
            if (obj.find("uuid") == obj.end() ||
                (obj.find("name") == obj.end() && obj.find("name_zh_cn") == obj.end()) ||
                (obj.find("desc") == obj.end() && obj.find("desc_zh_cn") == obj.end()) ||
                (obj.find("theme_icon") == obj.end() && obj.find("custom_icon") == obj.end()))
            {
                qWarning() << "Tool box D-Bus create tools field lose.";
                continue;
            }

            // 取图标
            QString theme_icon;
            QString custom_icon;
            const auto theme_icon_iter = obj.find("theme_icon");
            if (theme_icon_iter != obj.end()) {
                theme_icon = theme_icon_iter.value().toString();
            }
            const auto custom_icon_iter = obj.find("custom_icon");
            if (custom_icon_iter != obj.end()) {
                custom_icon = custom_icon_iter.value().toString();
            }

            // 取名字和描述
            DBusAppName dbus_app_name;
            DBusAppDesc dbus_app_desc;
            const auto name_iter = obj.find("name");
            if (name_iter != obj.end()) {
                dbus_app_name.name = name_iter.value().toString();
            }
            const auto name_zh_cn_iter = obj.find("name_zh_cn");
            if (name_zh_cn_iter != obj.end()) {
                dbus_app_name.name_zh_cn = name_zh_cn_iter.value().toString();
            }
            const auto name_en_us_iter = obj.find("name_en_us");
            if (name_en_us_iter != obj.end()) {
                dbus_app_name.name_en_us = name_en_us_iter.value().toString();
            }
            const auto name_zh_hk_iter = obj.find("name_zh_hk");
            if (name_zh_hk_iter != obj.end()) {
                dbus_app_name.name_zh_hk = name_zh_hk_iter.value().toString();
            }
            const auto name_bo_cn_iter = obj.find("name_bo_cn");
            if (name_bo_cn_iter != obj.end()) {
                dbus_app_name.name_bo_cn = name_bo_cn_iter.value().toString();
            }
            const auto name_mn_mn_iter = obj.find("name_mn_mn");
            if (name_mn_mn_iter != obj.end()) {
                dbus_app_name.name_mn_mn = name_mn_mn_iter.value().toString();
            }
            const auto desc_iter = obj.find("desc");
            if (desc_iter != obj.end()) {
                dbus_app_desc.desc = desc_iter.value().toString();
            }
            const auto desc_zh_cn_iter = obj.find("desc_zh_cn");
            if (desc_zh_cn_iter != obj.end()) {
                dbus_app_desc.desc_zh_cn = desc_zh_cn_iter.value().toString();
            }
            const auto desc_en_us_iter = obj.find("desc_en_us");
            if (desc_en_us_iter != obj.end()) {
                dbus_app_desc.desc_en_us = desc_en_us_iter.value().toString();
            }
            const auto desc_zh_hk_iter = obj.find("desc_zh_hk");
            if (desc_zh_hk_iter != obj.end()) {
                dbus_app_desc.desc_zh_hk = desc_zh_hk_iter.value().toString();
            }
            const auto desc_bo_cn_iter = obj.find("desc_bo_cn");
            if (desc_bo_cn_iter != obj.end()) {
                dbus_app_desc.desc_bo_cn = desc_bo_cn_iter.value().toString();
            }
            const auto desc_mn_mn_iter = obj.find("desc_mn_mn");
            if (desc_mn_mn_iter != obj.end()) {
                dbus_app_desc.desc_mn_mn = desc_mn_mn_iter.value().toString();
            }

            auto *app_item = new AppItem(AppItem::Type::kDBus, this);
            app_item->SetIcon(theme_icon, custom_icon);
            app_item->SetDBusAppName(dbus_app_name);
            app_item->SetDBusAppDesc(dbus_app_desc);
            app_item->SetDependPackages(QList<QString>());
            app_item->SetDBusUuid(obj["uuid"].toString());

            const auto has_user_manual_iter = obj.find("hasUserManual");
            if (has_user_manual_iter != obj.end()) {
                app_item->SetUserManualName(has_user_manual_iter.value().toString());
            }
            const auto online_iter = obj.find("online");
            if (online_iter != obj.end()) {
                app_item->SetDBusOnline(online_iter.value().toBool());
            }

            // 分类
            const auto category_iter = obj.find("app_type");
            if (category_iter != obj.end()) {
                const auto category = category_iter.value().toString();
                if (category == "Feature") {
                    // 特色工具
                    app_item->SetCategory(AppItem::Category::kFeature);

                    auto *item = new QListWidgetItem(feature_tools_list_widget_);;
                    SetUpListWidgetItem(item);
                    feature_tools_list_widget_->setItemWidget(item, app_item);
                } else if (category == "Debug") {
                    // 运维调试类工具
                    app_item->SetCategory(AppItem::Category::kDebug);

                    auto *item = new QListWidgetItem(debug_tools_list_widget_);
                    SetUpListWidgetItem(item);
                    debug_tools_list_widget_->setItemWidget(item, app_item);
                } else if (category == "Troubleshooting") {
                    // 故障修复工具
                    app_item->SetCategory(AppItem::Category::kTroubleshooting);

                    auto *item = new QListWidgetItem(troubleshooting_tools_list_widget_);
                    SetUpListWidgetItem(item);
                    troubleshooting_tools_list_widget_->setItemWidget(item, app_item);
                } else {
                    // 其他工具
                    app_item->SetCategory(AppItem::Category::kOther);

                    auto *item = new QListWidgetItem(other_tools_list_widget_);
                    SetUpListWidgetItem(item);
                    other_tools_list_widget_->setItemWidget(item, app_item);
                }
            } else {
                // 不包含分类字段，默认放到其他分类中
                app_item->SetCategory(AppItem::Category::kOther);

                auto *item = new QListWidgetItem(other_tools_list_widget_);
                SetUpListWidgetItem(item);
                other_tools_list_widget_->setItemWidget(item, app_item);
            }
        }
        UpdateInterface();
    });
}

}
