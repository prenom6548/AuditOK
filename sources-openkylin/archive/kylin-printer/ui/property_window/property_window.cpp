/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 */
#include <QHBoxLayout>
#include "custom_message_box.h"

#ifdef USE_SELF_CONTAINED_DEP
    #include "knavigationbar.h"
#else
    #include <kysdk/applications/knavigationbar.h>
#endif // USE_SELF_CONTAINED_DEP

#include "property_window.h"
#include "property_list_window.h"
#include "mainwindow.h"
#include "select_ppd_dialog.h"
#include "dictionary_for_chinese_printer.h"
#include "property_itemwidget.h"
#include "common.h"
#include "ink_info_widget.h"
#include "ink_info_helper.h"
#include "information_collector.h"
#include "kwre_info_widget.h"


QString getDebPackageName(ppd_file_t *m_ppd) {
    if (m_ppd) {
        QString name = QString(m_ppd->pcfilename);
        name.remove(".ppd");

        ppd_attr_t *attr = ppdFindAttr(m_ppd, "DebPackageName", NULL);
        if (attr && attr->value) {
            name = QString(attr->value);
        }
        return name;
    }
    return QString();
}

QString getProductName(ppd_file_t *m_ppd) {
    QString printerWineName = QString(m_ppd->product);
    if (printerWineName.at(0) == '(') {
        printerWineName.remove(0, 1);
    }
    if (printerWineName.back() == ')') {
        printerWineName.chop(1);
    }
    return printerWineName;
}

bool isKwreDriver(ppd_file_t *m_ppd) {
    QString printerWineName = QString(m_ppd->nickname);
    return printerWineName.contains("(LingXi)");
}

PropertyWindow::PropertyWindow(const UkuiUsbPrinter &printerInfo, QWidget *parent) :
    m_currentPrinterInfo(printerInfo),
    CustomWidget(parent, true)
{
    initUi();
    setAttribute(Qt::WA_DeleteOnClose);
    addTab(tr("base"), PropertyListWindow::BASE_PAGE);


    InkInfoHelper *h = new InkInfoHelper(m_currentPrinterInfo);
    QEventLoop loop;
    connect(h, &InkInfoHelper::finished, &loop, &QEventLoop::quit);
    connect(h, &InkInfoHelper::succeed, [=]() {
        h->finished();
    });
    h->startWorker();
    loop.exec();

    if (m_currentPrinterInfo.inkOptions.size()) {
        addTab(tr("Ink"), PropertyListWindow::INK_PAGE);
    }

    g_printerManger.startModifyAttr(m_currentPrinterInfo.name,m_currentPrinterInfo,cupsinfo);
    if (isKwreDriver(cupsinfo.m_ppd)) {
        addTab(tr("Print Settings"), PropertyListWindow::KWRE_PAGE);
    }

    if (m_currentPrinterInfo.flexibleOptions.size()) {
        addTab(tr("advanced"), PropertyListWindow::ADVANCED_PAGE);
    }

    auto index = m_navigationBar->model()->item(0)->index();
    m_navigationBar->listview()->setCurrentIndex(index);

    for (auto &it : m_currentPrinterInfo.flexibleOptions)
    {
        getZhPropertyfromEn(it);
    }
    insertItemListLayout();
    initConnect();
    QPoint temp = QPoint(parent->mapToGlobal(QPoint(0,0)).x() + parent->width()/2,parent->mapToGlobal(QPoint(0,0)).y() + parent->height()/2);
    this->move(temp - this->rect().center());
}

void PropertyWindow::initUi()
{
    setWidgetName(tr("PrinterProperty"));
    setLayoutType(kdk::HorizontalType);
    this->layout()->setSizeConstraint(QLayout::SizeConstraint::SetFixedSize);

    windowButtonBar()->menuButton()->hide();
    windowButtonBar()->maximumButton()->hide();
    windowButtonBar()->minimumButton()->hide();
    baseBar()->setFixedSize(820 - 198,540);
    sideBar()->setFixedSize(198,540);
    iconBar()->setFixedWidth(198);
    m_navigationBar.reset(new kdk::KNavigationBar(iconBar()));

    m_leftLayout.reset(new QHBoxLayout());
    m_leftLayout->setContentsMargins(11, 16, 16, 0);
    m_leftLayout->setSpacing(0);

    m_leftLayout->addWidget(m_navigationBar.get());
    sideBar()->setLayout(m_leftLayout.get());

    m_propertyListWindow.reset(new PropertyListWindow());
    setShowPage(m_navigationBar->model()->index(0, 0));
    m_rightLayout.reset(new QHBoxLayout());
    m_rightLayout->setMargin(0);
    m_rightLayout->setSpacing(0);
    m_rightLayout->addWidget(m_propertyListWindow.get());
    baseBar()->setLayout(m_rightLayout.get());
}

PropertyWindow::~PropertyWindow()
{

}

void PropertyWindow::insertItemListLayout()
{
    m_itemMap.reset(new QMap<QString, ProperytItemWidget *>);

    initBasePage();
    initAdvancedPage();
    initInkPage();
    initKwrePage();
}

void PropertyWindow::initBasePage()
{
    createItem(PropertyListWindow::BASE_PAGE, g_propertyList.find("Name").value(),     tr("Name"));
    createItem(PropertyListWindow::BASE_PAGE, g_propertyList.find("Location").value(), tr("Location"));
    createItem(PropertyListWindow::BASE_PAGE, g_propertyList.find("Status").value(),   tr("Status"));
    auto item = createItem(PropertyListWindow::BASE_PAGE, g_propertyList.find("Driver").value(),   tr("Driver"));
    if (item) {
        connect(item, &ProperytItemWidget::thirdOperateButtonClicked, this, &PropertyWindow::selectPPDDialogShow);
        item->setThirdButtonText(tr("Modify"));
    }
    createItem(PropertyListWindow::BASE_PAGE, g_propertyList.find("URI").value(),      tr("URI"));
}

void PropertyWindow::initAdvancedPage()
{
    if (!m_currentPrinterInfo.flexibleOptions.size()) {
        return ;
    }

    for(auto cur : m_currentPrinterInfo.flexibleOptions)
    {
        qDebug() << "当前项展示名：" << cur.keyword.displayName;
        auto item = createItem(PropertyListWindow::ADVANCED_PAGE, PropertyBoxStyle::TEXT_WITH_COMBOBOX, cur.keyword.displayName);
        if (item) {
            item->setObjectName(cur.keyword.name);
            connect(item,  &ProperytItemWidget::showProperyChanged, this, &PropertyWindow::checkOptionConflicts);
        }
    }
}

void PropertyWindow::initInkPage()
{
    for (auto inkOption : m_currentPrinterInfo.inkOptions) {
        qDebug() << inkOption;
        auto page = m_propertyListWindow.get()->getListWidget(PropertyListWindow::INK_PAGE);
        InkInfoWidget *item = new InkInfoWidget(page);
        QListWidgetItem *listWidgetItem = new QListWidgetItem(page);
        listWidgetItem->setFlags(Qt::ItemIsSelectable);
        listWidgetItem->setSizeHint(QSize(m_propertyListWindow.get()->getStackedWidget()->width(), 64));
        item->setMainLabel(inkOption.name + tr(" (level: %1%)").arg(inkOption.level));
        item->setInformativeLabel(inkOption.type);
        item->setColor(inkOption.color);
        page->addItem(listWidgetItem);
        page->setItemWidget(listWidgetItem, item);
    }
}

void PropertyWindow::initKwrePage()
{
    auto page = m_propertyListWindow.get()->getListWidget(PropertyListWindow::KWRE_PAGE);
    KwreInfoWidget *item = new KwreInfoWidget(page);
    QListWidgetItem *listWidgetItem = new QListWidgetItem(page);
    listWidgetItem->setFlags(Qt::ItemIsSelectable);
    listWidgetItem->setSizeHint(QSize(m_propertyListWindow.get()->getStackedWidget()->width(), 60));
    item->setFixedSize(m_propertyListWindow.get()->getStackedWidget()->width(), 60);
    item->setMainLabel(tr("Set print orientation, page order, paper source, etc."));
    item->setButtonText(tr("Printing Preferences"));
    page->addItem(listWidgetItem);
    page->setItemWidget(listWidgetItem, item);
    connect(item->getButton(), &CustomPushButton::clicked, this, [=](){
        // /opt/kylin-wine/printer_engine/bin/setprinter /opt/kylin-wine/printer_drv/kyocera-taskalfa-2020-gx/run.sh "Kyocera TASKalfa 2320 GX" 1
        QStringList command;
        command << "/opt/kylin-wine/printer_engine/bin/setprinter"
                << QString("/opt/kylin-wine/printer_drv/%1/run.sh").arg(getDebPackageName(cupsinfo.m_ppd))
                << getProductName(cupsinfo.m_ppd)
                << "1";
        runCommand(command);
    });

    // 创建分隔线项
    QListWidgetItem *separatorItem = new QListWidgetItem;
    separatorItem->setFlags(Qt::NoItemFlags); // 设置为不可选择
    separatorItem->setSizeHint(QSize(0, 2));  // 设置高度
    // 创建分隔线控件
    QLabel *line = new QLabel;
    line->setFixedHeight(2);
    line->setStyleSheet("background-color: #e5e5e5;");
    page->addItem(separatorItem);
    page->setItemWidget(separatorItem, line);


    KwreInfoWidget *item1 = new KwreInfoWidget(page);
    QListWidgetItem *listWidgetItem1 = new QListWidgetItem(page);
    listWidgetItem1->setFlags(Qt::ItemIsSelectable);
    listWidgetItem1->setSizeHint(QSize(m_propertyListWindow.get()->getStackedWidget()->width(), 60));
    item1->setFixedSize(m_propertyListWindow.get()->getStackedWidget()->width(), 60);
    item1->setMainLabel(tr("Printer port, sharing and other property configurations"));
    item1->setButtonText(tr("Device properties"));
    page->addItem(listWidgetItem1);
    page->setItemWidget(listWidgetItem1, item1);
    connect(item1->getButton(), &CustomPushButton::clicked, this, [=](){
        // /opt/kylin-wine/printer_engine/bin/setprinter /opt/kylin-wine/printer_drv/kyocera-taskalfa-2020-gx/run.sh "Kyocera TASKalfa 2320 GX" 2
        QStringList command;
        command << "/opt/kylin-wine/printer_engine/bin/setprinter"
                << QString("/opt/kylin-wine/printer_drv/%1/run.sh").arg(getDebPackageName(cupsinfo.m_ppd))
                << getProductName(cupsinfo.m_ppd)
                << "2";
        runCommand(command);
    });


    // 创建分隔线项
    QListWidgetItem *separatorItem1 = new QListWidgetItem;
    separatorItem1->setFlags(Qt::NoItemFlags); // 设置为不可选择
    separatorItem1->setSizeHint(QSize(0, 2));  // 设置高度
    // 创建分隔线控件
    QLabel *line1 = new QLabel;
    line1->setFixedHeight(2);
    line1->setStyleSheet("background-color: #e5e5e5;");
    page->addItem(separatorItem1);
    page->setItemWidget(separatorItem1, line1);

}

void PropertyWindow::onShow()
{
    needSaveDriver = false;
    for (auto &it : m_currentPrinterInfo.flexibleOptions)
    {
        getZhPropertyfromEn(it);
    }

    updateItemListLayout(m_currentPrinterInfo);

    if(!isActiveWindow())
    {
        hide();
        show();
        activateWindow();
    }

    show();
    raise();
    activateWindow();

    setShowPage(m_navigationBar->model()->index(0, 0));
}

void PropertyWindow::updateItemListLayout(const UkuiUsbPrinter& currentPrinterInfo)
{
    for (auto i = m_itemMap->begin(); i != m_itemMap->end(); i++) {
        QString propertyName = i.key();
        if (!propertyName.compare("Name")||!propertyName.compare(tr("Name"))) {
            i.value()->setSecondProperty(currentPrinterInfo.name);
        }
        else if (!propertyName.compare("Location")||!propertyName.compare(tr("Location")))
        {
            i.value()->setSecondProperty(currentPrinterInfo.location);
        }
        else if (!propertyName.compare("Status")||!propertyName.compare(tr("Status")))
        {
            if(!currentPrinterInfo.connect_status.compare(tr("Idle")))
            {
                i.value()->setSecondProperty(tr("Idle"));
            }
            else if(!currentPrinterInfo.connect_status.compare(tr("Printing")))
            {
                i.value()->setSecondProperty(tr("Printing"));
            }
            else if(!currentPrinterInfo.connect_status.compare(tr("Stopped")))
            {
                i.value()->setSecondProperty(tr("Stopped"));
            }
            else
            {
                qWarning() << "打印机状态异常：" << currentPrinterInfo.connect_status;
                i.value()->setSecondProperty(tr("Unknow"));
            }
        }
        else if (!propertyName.compare("Driver")||!propertyName.compare(tr("Driver"))) {
            i.value()->setSecondProperty(currentPrinterInfo.make_and_model);
        }
        else if (!propertyName.compare("URI")||!propertyName.compare(tr("URI")))
        {
            i.value()->setSecondProperty(currentPrinterInfo.device_uri);
        }
        else {
            qDebug() << propertyName;
            disconnect(i.value(),  &ProperytItemWidget::showProperyChanged, this, &PropertyWindow::checkOptionConflicts);

            for(auto cur : m_currentPrinterInfo.flexibleOptions)
            {
                if (!propertyName.compare(cur.keyword.displayName))
                {
                    qDebug() << cur.current.name;
                    for (auto l : cur.supported) {
                        qDebug() << l.displayName;
                    }
                    i.value()->setSecondProperty(cur.current, cur.supported);
                    break;
                }
            }
            connect(i.value(),  &ProperytItemWidget::showProperyChanged, this, &PropertyWindow::checkOptionConflicts);
        }
    }
}

void PropertyWindow::checkOptionConflicts(QString optionName, QString curVal)
{
    qDebug() << optionName;
    qDebug() << curVal;

    QMap<QString, QString> conflicts;

    int res = g_printerManger.modifyPrinterAttr(optionName, curVal, conflicts,cupsinfo);
    if(-1 == res)
    {
        updateItemListLayout(m_currentPrinterInfo);

        CustomMessageBox Msg(CustomMessageBox::Warning, tr("Warning"), tr("Error happened, all options restored!."), CustomMessageBox::NoButton, this);
        Msg.addButton(tr("Confirm"), CustomMessageBox::YesRole);
        Msg.exec();
        return;
    }
    else
    {
        setAllConflicts(optionName,conflicts);
    }
}

void PropertyWindow::setAllConflicts(QString option, QMap<QString, QString> conflicts)
{
    QString optionDisplayName;
    for(auto cur : m_currentPrinterInfo.flexibleOptions)
    {
        if (option == cur.keyword.name) {
            optionDisplayName = cur.keyword.displayName;
            break;
        }
    }

    qDebug() << optionDisplayName;

    for (auto i = m_itemMap->begin(); i != m_itemMap->end(); i++) {
        i.value()->showWarning("");
    }

    //g_printerManger.getConflictResults(option, conflictsList); //获取当前修改项的冲突项

    if(conflicts.size())
    {
        for (auto i = m_itemMap->begin(); i != m_itemMap->end(); i++)//冲突项1置为红色
        {
            for(auto it : conflicts)
            {
                if (i.value()->getSecondComboBoxContent().contains(it))
                {
                    i.value()->showWarning(tr("Conflict with \"%1\"").arg(optionDisplayName));
                    break;
                }
            }
        }

        m_propertyListWindow.get()->getAcceptButton()->setEnabled(false);
    }
    else
    {
        m_propertyListWindow.get()->getAcceptButton()->setEnabled(true);
    }
}

void PropertyWindow::initConnect()
{
    connect(m_navigationBar->listview(), &QListView::clicked, this, &PropertyWindow::setShowPage);
    connect(m_propertyListWindow.get(),  &PropertyListWindow::saveRequest,  this, &PropertyWindow::doSave);
    connect(m_propertyListWindow.get(),  &PropertyListWindow::closeRequest, this, &kdk::KDialog::close);
}

void PropertyWindow::setShowPage(const QModelIndex &index)
{
    if (!index.isValid()) {
        return ;
    }
    
    PropertyListWindow::PAGE page = index.data(Qt::UserRole + 1).value<PropertyListWindow::PAGE>();
    m_propertyListWindow.get()->getStackedWidget()->setCurrentIndex(page);
    m_propertyListWindow.get()->getNameLabel()->setText(index.data().toString());
    switch (page)
    {
    case PropertyListWindow::BASE_PAGE:
    case PropertyListWindow::ADVANCED_PAGE:
        m_propertyListWindow.get()->getRefreshButton()->setVisible(false);
        break;
    case PropertyListWindow::INK_PAGE:
        m_propertyListWindow.get()->getRefreshButton()->setVisible(true);
    default:
        break;
    }
}

void PropertyWindow::selectPPDDialogShow()
{
//    SelectPpdDialog ppdDialog(this);
    m_pSelectPPDDialog.reset(new SelectPpdDialog(this));
    connect(m_pSelectPPDDialog.get(),SIGNAL(chosenPPDName(QString)),this,SLOT(updatePPD(QString)));
    m_pSelectPPDDialog.get()->onShow();
//    m_pSelectPPDDialog.get()->exec();
}

void PropertyWindow::doSave()
{
    bool needSaveUri = false;

    if (needSaveDriver == false) {
        for (auto i = m_itemMap->begin(); i != m_itemMap->end(); i++)
        {
            QString propertyName = i.key();
            QString propertyValue = i.value()->getProperty();
            if (!propertyName.compare("Name")||!propertyName.compare(tr("Name")))
            {
                m_currentPrinterInfo.name = propertyValue;
            }
            else if (!propertyName.compare("Location")||!propertyName.compare(tr("Location")))
            {
                m_currentPrinterInfo.location = propertyValue;
            }
            else if (!propertyName.compare("Status")||!propertyName.compare(tr("Status")))
            {
                m_currentPrinterInfo.connect_status = propertyValue;
            }
            else if (!propertyName.compare("Driver")||!propertyName.compare(tr("Driver")))
            {
                m_currentPrinterInfo.make_and_model = propertyValue;
            }
            else if (!propertyName.compare("URI")||!propertyName.compare(tr("URI")))
            {
                if (m_currentPrinterInfo.device_uri != propertyValue) {
                    m_currentPrinterInfo.device_uri = propertyValue;
                    needSaveUri = true;
                    InformationCollector::getInstance().addMessage(QString("change printer uri"));
                }
            }
            else
            {
                for (auto &it : m_currentPrinterInfo.flexibleOptions)
                {
                    if(!propertyName.compare(it.keyword.displayName, Qt::CaseInsensitive) )
                    {
                        it.current.name = i.value()->getSecondComboBoxContent();
                    }
                }
            }
        }
    }

    qDebug() << "needSaveUri is : " << needSaveUri;
    g_printerManger.endModifyAttr(m_currentPrinterInfo, !needSaveDriver, needSaveUri,cupsinfo);
    qDebug() << "修改成功";
    emit MainWindow::getInstance()->updateMainWin();
    this->close();
}

void PropertyWindow::updatePPD(QString text)
{
    needSaveDriver = true;
    emit MainWindow::getInstance()->modifyPPD(text);
    QList<UkuiUsbPrinter> allPrintersVector;
    UkuiPrinterManager::getInstance().enumPrinters(allPrintersVector);
    for(auto printer : allPrintersVector)
    {
        if(printer.name == m_currentPrinterInfo.name)
        {
            m_currentPrinterInfo = printer;
        }
    }

    ProperytItemWidget *w = findItem(tr("Driver"));
    if (w != nullptr) {
        w->setSecondProperty(m_currentPrinterInfo.make_and_model);
    }

    return;
}

void PropertyWindow::getZhPropertyfromEn(SelectableProperty& enProperty)
{
    DictionaryForChinesePrinter dict;
    for (auto &cur: dict.getPropertyDict())
    {
        if (!(enProperty.keyword.displayName.trimmed()).compare(cur.at(0),Qt::CaseInsensitive))
        {
            enProperty.keyword.displayName = cur.at(1);
        }
        if (!(enProperty.current.displayName.trimmed()).compare(cur.at(0),Qt::CaseInsensitive))
        {
            enProperty.current.displayName = cur.at(1);
        }
        for (auto &it : enProperty.supported)
        {
            if (!(it.displayName.trimmed()).compare(cur.at(0),Qt::CaseInsensitive))
            {
                it.displayName = cur.at(1);
            }
        }
    }
    return ;
}

ProperytItemWidget *PropertyWindow::createItem(int index, PropertyBoxStyle style, const QString &propertyName)
{
    if (propertyName.isEmpty() || findItem(propertyName) != nullptr) {
        return nullptr;
    }
    auto page = m_propertyListWindow.get()->getListWidget(index);

    ProperytItemWidget *item = new ProperytItemWidget(propertyName,style,page);
    QListWidgetItem *listWidgetItem = new QListWidgetItem(page);
    listWidgetItem->setFlags(Qt::ItemIsSelectable);
    listWidgetItem->setSizeHint(QSize(m_propertyListWindow.get()->getStackedWidget()->width(), 44));
    page->addItem(listWidgetItem);
    page->setItemWidget(listWidgetItem, item);

    m_itemMap->insert(propertyName, item);

    return item;
}

ProperytItemWidget *PropertyWindow::findItem(const QString &propertyName)
{
    auto res = m_itemMap->find(propertyName);
    if (res == m_itemMap->end()) {
        return nullptr;
    }
    return res.value();
}

void PropertyWindow::addTab(const QString &name, PropertyListWindow::PAGE page)
{
    auto item = new QStandardItem(name);
    item->setData(page, Qt ::UserRole + 1);
    m_navigationBar->addItem(item);
}
