/*
 *  * Copyright (C) 2024, KylinSoft Co., Ltd.
 *  *
 *  * This program is free software: you can redistribute it and/or modify
 *  * it under the terms of the GNU General Public License as published by
 *  * the Free Software Foundation, either version 3 of the License, or
 *  * (at your option) any later version.
 *  *
 *  * This program is distributed in the hope that it will be useful,
 *  * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  * GNU General Public License for more details.
 *  *
 *  * You should have received a copy of the GNU General Public License
 *  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  *
 *  * Authors: like <like1@kylinos.cn>
 *
 */
#include <QtTest>
#include <QObject>
#include "../src/filechooser/kyfiledialog.h"
#include "../src/filechooser/kyfiledialogprivate.h"
#include "../src/filechooser/uikyfiledialog.h"
#include "../src/filechooser/pathbar.h"
#include "../src/filechooser/sidebar.h"
#include "../src/filechooser/menutoolbutoon.h"
#include "../src/ukuidesktopportal.h"
#include "../src/ukuifilechooserportal.h"


// 定义函数指针类型，与库中函数的签名匹配
typedef void (*FunctionInLibType)(void);

Q_DECLARE_METATYPE(FileChooserPortal::Filter)
Q_DECLARE_METATYPE(FileChooserPortal::Filters)
Q_DECLARE_METATYPE(FileChooserPortal::FilterList)
Q_DECLARE_METATYPE(FileChooserPortal::FilterListList)

class LibFunctionTest : public QObject
{
    Q_OBJECT
public:
private Q_SLOTS:
    // 测试函数，用于测试相关功能
    void testFunctionInLib();
private:
    KyNativeFileDialog *kydialog;
    UKUIDesktopPortal *ukuidesktopportal;
    FileChooserPortal *filechooserportal;
};

// 测试函数实现
void LibFunctionTest::testFunctionInLib()
{
    qApp->setProperty("Language", "zh_CN");
    QWidget w;
    QDBusObjectPath path;
    QVariantMap testMap, result;
    testMap.insert("accept_label", "nihao");
    testMap.insert("modal", true);
    testMap.insert("multiple", false);
    testMap.insert("directory", false);
    testMap.insert("current_folder", "/");
    testMap.insert("current_name", "nihao");

    FileChooserPortal::Filter Filter;
    FileChooserPortal::FilterList FilterList;
    FileChooserPortal::Filters flist1, flist2;
    FileChooserPortal::FilterListList flistlist;

    Filter.type = 0;
    Filter.filterString = QString("*");
    flist1.append(Filter);
    Filter.type = 0;
    Filter.filterString = QString("*.txt");
    flist1.append(Filter);
    FilterList.userVisibleName = QString("全部");
    FilterList.filters = flist1;
    flistlist.append(FilterList);

    Filter.type = 0;
    Filter.filterString = QString("*.jpg");
    flist2.append(Filter);
    Filter.type = 0;
    Filter.filterString = QString("*.img");
    flist2.append(Filter);
    FilterList.userVisibleName = QString("图片");
    FilterList.filters = flist2;
    flistlist.append(FilterList);
    QVariant filterr;
    filterr.setValue(flistlist);
    testMap.insert("filters", filterr);


    ukuidesktopportal = new UKUIDesktopPortal;
    filechooserportal = new FileChooserPortal(ukuidesktopportal);
    filechooserportal->OpenFile(path, "", "wayland:nihao", "", testMap, result);
    filechooserportal->SaveFile(path, "", "x11:test", "", testMap, result);
    filechooserportal->SaveFiles(path, "", "", "", testMap, result);


    kydialog = new KyNativeFileDialog(&w);
    kydialog->setDirectory(QString("file://%1").arg(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)));
    kydialog->setViewMode(KyNativeFileDialog::List);
    kydialog->setAcceptMode(QFileDialog::AcceptSave);
    kydialog->setCurrentInputName("test");
    kydialog->setLabelText(QFileDialog::FileName, "test");
    kydialog->setFileMode(QFileDialog::Directory);
    kydialog->setFileMode(QFileDialog::AnyFile);
    kydialog->setLabelText(QFileDialog::Accept,"是确认");
    kydialog->setLabelText(QFileDialog::Reject,"不确认");
    kydialog->setLabelText(QFileDialog::LookIn,"测试");
    kydialog->setCurrentSortOrder (Qt::AscendingOrder);
    kydialog->setCurrentSortColumn (10);
    kydialog->beginSwitchView("Icon View");
    kydialog->selectFile("test");
    qDebug()<<kydialog->labelText(QFileDialog::Accept)<<kydialog->labelText(QFileDialog::Reject)<<kydialog->labelText(QFileDialog::LookIn)
           <<kydialog->getCurrentAllFileUris()<<kydialog->nameFilters()<<kydialog->selectedNameFilter()<<kydialog->selectNameFilterIndex()
          <<kydialog->viewMode();
    kydialog->getCurrentSelectionFileInfos();
    kydialog->options();
    QStringList nameFilters;
    nameFilters << "txt";
    nameFilters << "*";
    kydialog->setNameFilters(nameFilters);
    kydialog->exec();

}
QTEST_MAIN(LibFunctionTest)

#include "test-settings.moc"


