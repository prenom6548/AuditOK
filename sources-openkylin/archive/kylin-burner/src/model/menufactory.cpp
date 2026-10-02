/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#include "menufactory.h"
#include "src/frame/logger.h"
#include "src/po/translation.h"
#include "src/model/logicmain.h"
#include "src/data/datacdrom.h"
#include "src/view/aboutdialog.h"
#include "src/common/utils.h"
#include "src/view/mainwindow.h"
#include "src/view/modaldialog.h"

#include <QApplication>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QFileInfo>
#include <QDebug>
#include <QDBusMessage>

#include "kaboutdialog.h"
#include "kmenubutton.h"

using namespace kdk;

MenuFactory *MenuFactory::m_oInstance = nullptr;
QMutex       MenuFactory::m_oMutex;

MenuFactory::MenuFactory(QObject *parent) :
    QObject(parent),
    m_oActionSetting(new QAction(/*QIcon::fromTheme("ukui-icon-setting"),*/ _("Setting"), this)),
    m_oActionDataSetting(new QAction(/*QIcon::fromTheme("ukui-icon-allsetting"),*/ _("Data Setting"), this)),
    m_oActionFileFilter(new QAction(_("File Filter"), this)),
    m_oActionFileFilterView(new QAction(/*QIcon::fromTheme("view-filter-symbolic"),*/ _("View Filter File(s)"), this)),
    m_oActionDiskCheck(new QAction(_("Disk Check"), this)),
    m_oActionBurnerSetting(new QAction(/*QIcon::fromTheme("drive-cdrom"),*/ _("Burner Setting"), this)),
    m_oActionEject(new QAction(/*QIcon::fromTheme("media-eject-symbolic"),*/ _("Eject"), this)),
    m_oActionEarse(new QAction(/*QIcon::fromTheme("accessories-system-cleaner"),*/ _("Earse"), this)),
    m_oActionVersion(new QAction(/*QIcon::fromTheme("burner"),*/ _("Version"), this)),
    m_oActionAbout(new QAction(/*QIcon::fromTheme("burner"),*/ _("About"), this)),
    m_oActionHelp(new QAction(/*QIcon::fromTheme("help"),*/ _("Help"), this)),
    m_oActionExit(new QAction(/*QIcon::fromTheme("exit-symbolic"),*/ _("Exit"), this)),
    m_oSubEject(new QMenu()),
    m_oSubErase(new QMenu())
{

    QMenu *menu;
    auto createMenu = [](const QString &accessibleName) {
        QMenu *newMenu = new QMenu();
        newMenu->setAccessibleName(accessibleName);
        return newMenu;
    };

    m_oSubEject->setAccessibleName("kylin-burner_QMenu_MenuFactory_m_oSubEject");
    m_oSubErase->setAccessibleName("kylin-burner_QMenu_MenuFactory_m_oSubErase");

    m_oEjectActions << m_oSubEject->addAction(QIcon::fromTheme("media-eject-symbolic"), _("Eject All"), this, SIGNAL(eject(bool)));

    connect(m_oActionSetting, &QAction::triggered, this, &MenuFactory::setting);
    connect(m_oActionDataSetting, &QAction::triggered, this, &MenuFactory::slotsDataSetting);
    connect(m_oActionFileFilter, &QAction::triggered, this, &MenuFactory::fileFilter);
    connect(m_oActionFileFilterView, &QAction::triggered, this, &MenuFactory::viewFilterFile);
    connect(m_oActionDiskCheck, &QAction::triggered, this, &MenuFactory::slotsDiskCheck);
    connect(m_oActionBurnerSetting, &QAction::triggered, this, &MenuFactory::burnSetting);

    connect(m_oActionEject, &QAction::triggered, [this](bool checked){
       emit this->eject(checked);
    });
    connect(m_oActionEarse, &QAction::triggered, [this](bool checked){
       emit this->earse(checked);
    });

    connect(m_oActionVersion, &QAction::triggered, this, &MenuFactory::version);
    connect(m_oActionAbout,&QAction::triggered,this,&MenuFactory::showabout);
    connect(m_oActionHelp, &QAction::triggered, this, &MenuFactory::slotsHelp);
    connect(m_oActionExit, &QAction::triggered, [=](){
        g_user_signal->sigCloseApp();
    });
    connect(g_user_signal, &GlobalUserSignal::sigChangeDiscDisplayInfo, [=](void* cdrom, QString displayInfo){
       newEarse(cdrom);
    });

    kdk::KMenuButton* menuButton = new kdk::KMenuButton();
    QAction *feedbackAction = menuButton->feedbackAction();
    feedbackAction->setVisible(true);

    // common actions
    m_oCommonActions << m_oActionHelp << m_oActionAbout << feedbackAction << m_oActionExit;
    connect(this, &MenuFactory::setSetting, m_oActionBurnerSetting, &QAction::setEnabled);
    connect(this, &MenuFactory::setViewFilterFile, m_oActionFileFilterView, &QAction::setEnabled);
    // create iso
    menu = createMenu("kylin-burner_QMenu_MenuFactory_menuCreateISO");

    menu->addAction(m_oActionDataSetting);
    menu->addAction(m_oActionFileFilter);
    menu->addAction(m_oActionFileFilterView);
    menu->addSeparator();
    menu->addAction(m_oActionBurnerSetting);
    menu->addAction(m_oActionEject);
    menu->addAction(m_oActionEarse);
    menu->addSeparator();
    menu->addActions(m_oCommonActions);
    m_oMenus << menu;

    // burn data
    menu = createMenu("kylin-burner_QMenu_MenuFactory_menuBurnData");
    menu->addAction(m_oActionDataSetting);
    menu->addAction(m_oActionFileFilter);
    menu->addAction(m_oActionFileFilterView);
    menu->addSeparator();
    menu->addAction(m_oActionBurnerSetting);
    menu->addAction(m_oActionEject);
    menu->addAction(m_oActionEarse);
    menu->addAction(m_oActionDiskCheck);
    menu->addSeparator();
    menu->addActions(m_oCommonActions);
    m_oMenus << menu; // burn data same as add burn

    // burn image
    menu = createMenu("kylin-burner_QMenu_MenuFactory_menuBurnImage");
    menu->addAction(m_oActionBurnerSetting);
    menu->addAction(m_oActionEject);
    menu->addAction(m_oActionEarse);
    menu->addAction(m_oActionDiskCheck);
    menu->addSeparator();
    menu->addActions(m_oCommonActions);
    m_oMenus << menu; // burn data same as add burn

    // default is disabled for eject and earse
    m_oActionEject->setEnabled(false);
    m_oActionEarse->setEnabled(false);
}

MenuFactory *MenuFactory::instance()
{
    if (nullptr == m_oInstance)
    {
        QMutexLocker locker(&m_oMutex);
        if (nullptr == m_oInstance)
        {
            m_oInstance = new MenuFactory();
        }
    }
    return m_oInstance;
}

QMenu *MenuFactory::getMenu(int i)
{
    //m_oMenus有3个menu，m_oMenus[0]对应creatISO，m_oMenus[1]对应burnData,m_oMenus[2]对应burnImage.
    //在枚举Busness中添加了值，这里也得改，否则会导致返回值为NULL
    //bug92741 【刻录】点击刻录镜像界面后点击菜单栏，菜单栏打不开;
    switch(i)
    {
    case 0:
        //CreateISO
        return m_oMenus.at(0);
    case 1:
        //BurnData
    case 2:
        //BurnUDFData
    case 3:
        //AddBurn
        return m_oMenus.at(1);
    case 4:
    case 5:
    case 6:
        return m_oMenus.at(2);
    default:
        break;
    }
    return nullptr;

}

void MenuFactory::newCDROM(void *cdrom)
{
    if (!cdrom)
        return;

    DataCDROM* device = (DataCDROM *)cdrom;

   //光驱是不存在的,m_oSeting类被delete，后面调用isEjected会调用m_oSetting成员函数出错
    if ((device->m_is_using <= 0) || (!device->isCDROM()))
    {
        return;
    }

    if (!device->isEjected())
    {
        m_oActionEject->setEnabled(true);
    }

    connect(this, SIGNAL(eject(bool)), device, SLOT(eject()));

    if (-1 == m_oEjects.indexOf(cdrom))
    {
        for (int i = 0; i < m_oEjects.size(); ++i)
        {
            DataCDROM *indexDevice = static_cast<DataCDROM *>(m_oEjects.at(i));
            if (!indexDevice)
                continue;

            if (device->device() == indexDevice->device())
                return;
        }

        m_oEjectActions << m_oSubEject->addAction(device->mediumIcon(),
                                                  device->display(),
                                                  device, SLOT(eject()));

        m_oEjectActions.last()->setToolTip(device->mDisplay());

        if (device->isEjected())
        {
            m_oEjectActions.last()->setEnabled(false);
        }

        //connect(device, &DataCDROM::eject, m_oEjectActions.last(), &QAction::setEnabled);
        connect(device, SIGNAL(eject(bool)), this, SLOT(checkEject(bool)));
        m_oEjects << cdrom;
    }

    if (m_oEjectActions.size() > 2)
    {
        device = static_cast<DataCDROM *>(m_oEjects.first());
        disconnect(device, SIGNAL(eject(bool)), m_oActionEject, SLOT(setEnabled(bool)));
        m_oActionEject->setMenu(m_oSubEject);
    }
    else
    {
        connect(device, SIGNAL(eject(bool)), m_oActionEject, SLOT(setEnabled(bool)));
    }
}

void MenuFactory::checkEject(bool checked)
{
    Q_UNUSED(checked);
    int i;
    for (i = 0; i < m_oEjects.size(); ++i)
    {
        DataCDROM *device = static_cast<DataCDROM *>(m_oEjects.at(i));
        if (!device->isEjected())
            break;
    }

    if (i == m_oEjects.size()){
        m_oActionEject->setEnabled(false);
    }else{
        m_oActionEject->setEnabled(true);
    }
}


/*
 * LogicMain中信号appendableMedium()、finishedMedium()和noMedium()更新可删除Action
 * 注意：在noMedium信号调用时光驱可能被拔出
 */
void MenuFactory::newEarse(void *c)
{
    int idx;
    DataCDROM *cdrom;

    idx = m_oErases.indexOf(c);
    cdrom = static_cast<DataCDROM *>(c);

    //为了防止cdrom被delete后还调用成员函数cdrom->mediumStatus()造成段错误
    //RW光盘 && 没有在m_oEreases注册
    if (cdrom->mediumIsErasable() && -1 == idx)
    {
        m_oEraseActions << m_oSubErase->addAction(cdrom->mediumIcon(),
                                                  cdrom->mDisplay(),
                                                  cdrom, SLOT(earse()));
        m_oErases << cdrom;
    }
    else if(cdrom->mediumIsErasable() && idx > -1){

        // 由于光盘容量或光盘描述发送信息更改，需先删除原来的QAtion、DataCDROM对象，再重新添加
        m_oErases.takeAt(idx);
        QAction *tmp = m_oEraseActions.takeAt(idx);
        m_oSubErase->removeAction(tmp);
        m_oEraseActions << m_oSubErase->addAction(cdrom->mediumIcon(),
                                                  cdrom->mDisplay(),
                                                  cdrom, SLOT(earse()));
        m_oErases << cdrom;
    }
//逻辑对吗？还得再看
    if (m_oEraseActions.size())
        m_oActionEarse->setMenu(m_oSubErase);
    m_oActionEarse->setEnabled(m_oEraseActions.size());
}

void MenuFactory::removeEarse(void *c)
{

    int idx;
    QAction *tmp;

    idx = m_oErases.indexOf(c);

    //光驱已在m_oEreases注册过的，移除
    if (-1 != idx)
    {
        tmp = m_oEraseActions.takeAt(idx);
        m_oSubErase->removeAction(tmp);
        delete tmp;
        m_oErases.takeAt(idx);
    }
    //逻辑对吗？还得再看
    if (m_oEraseActions.size())
        m_oActionEarse->setMenu(m_oSubErase);
    m_oActionEarse->setEnabled(m_oEraseActions.size());
}


void MenuFactory::takeCDROM(void * cdrom)
{

    DataCDROM *current_cdrom = static_cast<DataCDROM *>(cdrom);
    int index = m_oEjects.indexOf(cdrom);

    //光驱m_oEjects在已注册过
    if (-1 != index)
    {
        m_oEjects.takeAt(index);
        //m_oEjectActions have eject all action
        delete m_oEjectActions.takeAt(index + 1);
    }

    checkEject(true);

    index = 0;
    index = m_oErases.indexOf(cdrom);

    if (-1 != index)
    {
        m_oErases.takeAt(index);
        delete m_oEraseActions.takeAt(index);
    }

    if (current_cdrom && (current_cdrom->getUsedTimes() == 0))
    {
        connect(current_cdrom,&DataCDROM::destroyed,current_cdrom,[&](){current_cdrom = nullptr;},Qt::UniqueConnection);
        current_cdrom->deleteLater();
    }
}

void MenuFactory::showabout(bool checked){
    Q_UNUSED(checked);

    AboutDialog *about = new AboutDialog(MainWindow::Instance());
    about->exec();
    about->deleteLater();
}

void MenuFactory::slotsDataSetting(bool checked)
{
    Q_UNUSED(checked);

    ModalDialog::instance()->showPage(ModalDialog::DataSetting);
    ModalDialog::instance()->show();
}

void MenuFactory::slotsDiskCheck(bool checked)
{
    Q_UNUSED(checked);

    ModalDialog::instance()->showPage(ModalDialog::DiskCheck);
    ModalDialog::instance()->show();
}

void MenuFactory::slotsHelp(bool checked)
{
    Q_UNUSED(checked);

    QDBusMessage msg = QDBusMessage::createMethodCall( QString("com.kylinUserGuide.hotel_%1")
                                          .arg(QString::number(getuid())),"/", "com.guide.hotel",
                                          "showGuide");
    msg << "kylin-burner";
    msg.setDelayedReply(false);

    QDBusConnection::sessionBus().call(msg);
}

MenuFactory::~MenuFactory()
{
    QMenu *menu;

    while (m_oMenus.size())
    {
        menu = m_oMenus.takeFirst();
        if (menu) delete menu;
    }
    m_oMenus.clear();
    delete m_oSubEject;
    delete m_oSubErase;
}
