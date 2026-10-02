/*
 * UKUI Intelligent Data Manager
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include <QApplication>
#include <singleapplication.h>
#include <QCommandLineParser>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "model/sqlitemodel.h"
#include "model/fileitemmodel.h"
#include "model/modelcontroller.h"
#include "model/previewhelper.h"
#include "model/searchresultmodel.h"
#include "model/spacessettings.h"
#include "database/databasemanager.h"
#include "thumbnailmanager.h"
#include "conditioneditor.h"
#include "windowhelper.h"

#include "file/fileop/fileoperationmanager.h"

#include <peony-qt/global-settings.h>

#include <QSql>

#include <QWidget>

#include <QQuickWindow>

#include <QTranslator>
#include <QLocale>
#include <QTimer>

#include <KWindowEffects>
#include <ukuistylehelper/ukuistylehelper.h>
#include <ukuistylehelper/xatom-helper.h>
#include <kinputdialog.h>

int main(int argc, char *argv[])
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QIcon::setFallbackThemeName("hicolor");
#else
#endif

    QCoreApplication::setApplicationName("ukui-intelligent-data-manager");
    QCoreApplication::setOrganizationName("org.ukui");
    kdk::QtSingleApplication app("ukui-intelligent-data-manager", argc, argv);
    app.setApplicationDisplayName(QObject::tr("UKUI Intelligent Data Manager"));

    QTranslator ukuiidm;
    if (ukuiidm.load("/usr/share/ukui/idm/ukui-intelligent-data-manager_"+QLocale::system().name()))
        app.installTranslator(&ukuiidm);

    if (app.isRunning()) {
        QString message = app.arguments().join(' ');
        qDebug() << "send message" << message;
        if (app.sendMessage(message)) {
            return 0;
        } else {
            qWarning() << "failed to send massage to running program";
        }
    }

    QTranslator qt;
    if (qt.load("/usr/share/qt5/translations/qt_"+QLocale::system().name()))
        app.installTranslator(&qt);
    QTranslator qtbase;
    if (qtbase.load("/usr/share/qt5/translations/qtbase_"+QLocale::system().name()))
        app.installTranslator(&qtbase);
    QTranslator qqc1;
    if (qqc1.load("/usr/share/qt5/translations/qtquickcontrols_"+QLocale::system().name()))
        app.installTranslator(&qqc1);
    QTranslator qqc2;
    if (qqc2.load("/usr/share/qt5/translations/qtquickcontrols2_"+QLocale::system().name()))
        app.installTranslator(&qqc2);
    QTranslator libpeony;
    if (libpeony.load("/usr/share/libpeony-qt/libpeony-qt_"+QLocale::system().name()))
        app.installTranslator(&libpeony);
    QTranslator kdkwidgets;
    if (kdkwidgets.load(":/translations/gui_" + QLocale::system().name() + ".qm"))
        app.installTranslator(&kdkwidgets);

    QCommandLineParser parser;
    parser.addHelpOption();
    QCommandLineOption showOption(QStringList()<<"s"<<"show", QObject::tr("display window"));
    QCommandLineOption quitOption(QStringList()<<"q"<<"quit", QObject::tr("exit"));
    parser.addOption(showOption);
    parser.addOption(quitOption);

    bool showWindow = false;
    parser.process(app);
    if (parser.isSet(quitOption)) {
        app.quit();
        return 0;
    }
    if (parser.isSet(showOption)) {
        showWindow = true;
    }

    app.setQuitOnLastWindowClosed(false);

    // 注册 QSharedPointer<UKUI::IDM::FileInfo>
    qRegisterMetaType<QSharedPointer<UKUI::IDM::FileInfo>>("QSharedPointer<UKUI::IDM::FileInfo>");

    // 注册 QList<QSharedPointer<UKUI::IDM::FileInfo>>
    qRegisterMetaType<QList<QSharedPointer<UKUI::IDM::FileInfo>>>("QList<QSharedPointer<UKUI::IDM::FileInfo>>");

    Peony::GlobalSettings::getInstance();
    DatabaseManager dbManager;

    QQmlApplicationEngine engine;

    qmlRegisterType<ModelController>("ModelController", 1, 0, "ModelController");
    qmlRegisterType<FileItemModel>("FileItemModel", 1, 0, "FileItemModel");
    qmlRegisterType<FileItemProxySortFilterModel>("FileItemProxySortFilterModel", 1, 0, "FileItemProxySortFilterModel");
    qmlRegisterType<ThumbnailManager>("ThumbnailManager", 1, 0, "ThumbnailManager");
    qmlRegisterType<PreviewHelper>("PreviewHelper", 1, 0, "PreviewHelper");
    qmlRegisterType<QWindow>("QWindow", 1, 0, "QWindow");
    qmlRegisterType<SearchResultProxyModel>("SearchResultProxyModel", 1, 0, "SearchResultProxyModel");
    qmlRegisterType<SpacesSettings>("SpacesSettings", 1, 0, "SpacesSettings");
    //qmlRegisterType<SqliteModel>("SqliteModels", 1, 0, "SqliteModel");

    static WindowHelper *windowHelper = nullptr;
    static ThumbnailManager *thumbnailManager = new ThumbnailManager(&app);

    auto context = engine.rootContext();

    context->setContextProperty("thumbnailManager", thumbnailManager);

    auto mc = ModelController::globalInstance();
    mc->setParent(&app);
    context->setContextProperty("modelController", mc);
    QObject::connect(mc, &ModelController::setSpaceConditionRequest, &app, [=](int spaceId, const QString &spaceName){
        auto editor = new ConditionEditor(spaceId, spaceName, ConditionEditor::Edit);
        editor->createWinId();
        editor->windowHandle()->setTransientParent(windowHelper->getRootWindow());
        QObject::connect(editor, &ConditionEditor::conditionChanged, mc, &ModelController::spaceConditionsChanged);
        editor->show();
        if (windowHelper) {
            editor->windowHandle()->setTransientParent(windowHelper->getRootWindow());
        }
    });
    QObject::connect(mc, &ModelController::createSpaceRequest, &app, [=]{
        auto editor = new ConditionEditor(-1, nullptr, ConditionEditor::Create);
        editor->createWinId();
        editor->windowHandle()->setTransientParent(windowHelper->getRootWindow());
        QObject::connect(editor, &ConditionEditor::conditionCreatedRequest, mc, [=](const QString &spaceName, const QVariant &conditionsDescription){
            mc->onSpaceCreateRequest(spaceName, conditionsDescription, false);
        });
        editor->show();
        if (windowHelper) {
            editor->windowHandle()->setTransientParent(windowHelper->getRootWindow());
        }
    });
    QObject::connect(mc, &ModelController::editSearchRequested, &app, [=]{
        auto editor = new ConditionEditor(-1, nullptr, ConditionEditor::Search);
        editor->createWinId();
        editor->windowHandle()->setTransientParent(windowHelper->getRootWindow());
        QObject::connect(editor, &ConditionEditor::conditionCreatedRequest, mc, &ModelController::onSpaceCreateRequestWithFiles);
        editor->show();
        if (windowHelper) {
            editor->windowHandle()->setTransientParent(windowHelper->getRootWindow());
        }
    });

    auto fileopmgr = FileOperationManager::globalInstance();
    QObject::connect(mc, &ModelController::showMenuRequested, fileopmgr, &FileOperationManager::showMenu);
    QObject::connect(mc, &ModelController::openFileRequest, fileopmgr, &FileOperationManager::openFile);

    auto previewHelper = PreviewHelper::globalInstance();
    context->setContextProperty("previewHelper", previewHelper);

    auto searchModel = SearchResultProxyModel::globalInstance();
    context->setContextProperty("searchModel", searchModel);

    context->setContextProperty("spacesSettings", SpacesSettings::globalInstance());

    QObject::connect(mc, &ModelController::renameSpaceRequested, &app, [=](int spaceId){
        QString currentName = mc->getProxyModelBySpaceId(spaceId)->spaceName();
        QString newName = FileOperationManager::getNewSpaceName(currentName, windowHelper->getRootWindow());
        if (!newName.isEmpty()) {
            mc->renameSpace(spaceId, newName);
        }
    });

    QObject::connect(mc, &ModelController::searchSaveRequested, &app, [=]{
        bool ok = false;
        QString spaceName = FileOperationManager::getNewSpaceName(nullptr, windowHelper->getRootWindow(), &ok);
        if (ok) {
            QList<QSharedPointer<Condition>> conditions;
            auto reversedConditions = SearchResultModel::globalInstance()->getReversedConditions();
            auto editableConditions = searchModel->getSearchConditions();

            conditions << reversedConditions;
            conditions << editableConditions;

            QVariant conditionsDescription = ConditionEditor::pharseConditions(conditions);

            QList<QSharedPointer<UKUI::IDM::FileInfo>> infos;
            auto nameMatchFiles = SearchResultModel::globalInstance()->getNameMatchFiles();
            auto contentMatchFiles = SearchResultModel::globalInstance()->getContentMatchFiles();
            infos << nameMatchFiles;
            infos << contentMatchFiles;

            mc->onSpaceCreateRequestWithFiles(spaceName, conditionsDescription, true, infos);
        }
    });

    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [mc, fileopmgr, url](QObject *obj, const QUrl &objUrl) {
        qDebug() << "objectCreated" << obj->metaObject()->className() << objUrl;
        auto window = qobject_cast<QWindow *>(obj);

        if (window) {
            windowHelper = new WindowHelper(window);
            QIcon windowIcon;
            windowIcon.addFile(":/data/ukui-idm.png", QSize(64, 64));
            QIcon themeIcon = QIcon::fromTheme("ukui-intelligent-data-manager", windowIcon);
            window->setIcon(themeIcon);
            if (windowHelper->getIsWayland())
                windowHelper->removeHeaderBarWayland();
            windowHelper->activeWindow();

            QObject::connect(window, SIGNAL(systemMoveRequest(QPointF)), windowHelper, SLOT(handleSystemMoveRequest(QPointF)));
            QObject::connect(window, SIGNAL(optionMenuRequest(QPointF)), windowHelper, SLOT(handleOptionMenuRequest(QPointF)));

            fileopmgr->setRootWindow(window);
            mc->setRootWindow(window);
        }

        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    if (showWindow)
        engine.load(url);
    else
        mc->startLoadModel();

    QObject::connect(&app, &kdk::QtSingleApplication::messageReceived, &app, [&](const QString &message){
        QStringList arguments = message.split(" ");
        qDebug() << "message recived" << message << "arguments" << arguments;
        parser.process(arguments);
        if (parser.isSet(quitOption)) {
            qDebug() << "quit";
            app.closeAllWindows();
            app.quit();
        }
        if (parser.isSet(showOption)) {
            qDebug() << "show option";
            if (windowHelper) {
                qDebug() << "request active window";
                windowHelper->activeWindow();
            } else {
                engine.load(url);
            }
        }
    });

    return app.exec();
}
