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

#include <QtTest>
#include <QObject>
#include <QDir>
#include <QStandardPaths>

#include "fileitemmodel.h"
#include "fileitemproxysortfiltermodel.h"
#include "modelcontroller.h"
#include "databasemanager.h"

class ModelTest : public QObject
{
    Q_OBJECT
public:
    ModelTest() {}

public Q_SLOTS:
    void onModelCreated(FileItemProxySortFilterModel *model);

private Q_SLOTS:
    void initTestCase();
    void test1ControllerInit();
    void test2ModelInit();
    void test3RecentModel();
    void test4InsertFiles();
    void cleanupTestCase();

private:
    ModelController *controller;
    QList<FileItemProxySortFilterModel *> models;
};

QTEST_MAIN(ModelTest)
#include "modeltest.moc"

void ModelTest::onModelCreated(FileItemProxySortFilterModel *model)
{
    models << model;
}

void ModelTest::initTestCase()
{
    QDir d(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
    d.mkpath(d.absolutePath());
    DatabaseManager manager("modeltest.db");
    auto spaces = manager.getAllSpaceNames();
    d.cd("/bin");
    QStringList paths;
    for (auto entry : d.entryList()) {
        paths << d.filePath(entry);
    }
    for (auto space : spaces) {
        for (auto file : paths) {
            manager.addFileToSpace(file, space);
        }
        // UserAddedFile空间
        manager.addFileToSpace("/home", space, true);
    }
}

void ModelTest::test1ControllerInit()
{
    controller = new ModelController("modeltest.db");
    connect(controller, &ModelController::modelCreated, this, &ModelTest::onModelCreated);
    QEventLoop l;
    connect(controller, &ModelController::allModelCreated, &l, &QEventLoop::quit);
    controller->startLoadModel();
    l.exec();
}

void ModelTest::test2ModelInit()
{
    QVERIFY(models.count() > 0);
    for (auto model : models) {
        QEventLoop l;
        connect(model, &FileItemProxySortFilterModel::filesLoaded, &l, &QEventLoop::quit);
        model->startLoadFiles();
        l.exec();
        qDebug() << model->getSpaceId() << model->getSpaceName();
        qDebug() << model->rowCount();
        qDebug() << model->index(model->rowCount() - 1, 0).data();
    }
}

void ModelTest::test3RecentModel()
{
    FileItemProxySortFilterModel proxyModel(new FileItemModel);
    QEventLoop loop;
    connect(&proxyModel, &FileItemProxySortFilterModel::filesLoaded, this, [&]{
        qDebug() << proxyModel.rowCount();
        loop.quit();
    });
    proxyModel.startLoadRecentFiles();
    loop.exec();
}

void ModelTest::test4InsertFiles()
{
    QVERIFY(models.count() > 0);
    auto model = models.first();
    auto sourceModel = qobject_cast<FileItemModel *>(model->sourceModel());
    auto infos = {UKUI::IDM::FileInfo::fromPath("/"), UKUI::IDM::FileInfo::fromPath("/home")};

    auto infosAdded = sourceModel->insertFiles(infos, false);
    QVERIFY(infosAdded.count() == 1);
    infosAdded = sourceModel->insertFiles(infos, true);
    QVERIFY(infosAdded.count() == 0);
    sourceModel->removeFile("/");

    infosAdded = sourceModel->insertFiles(infos, true);
    QVERIFY(infosAdded.count() == 1);
    infosAdded = sourceModel->insertFiles(infos, false);
    QVERIFY(infosAdded.count() == 0);
    sourceModel->removeFile("/");
}

void ModelTest::cleanupTestCase()
{
    for (auto model : models) {
        delete model;
    }
    delete controller;
    QDir d(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
    d.removeRecursively();
}
