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
#include <QWidget>
#include <QHBoxLayout>
#include <QHeaderView>
#include "custom_message_box.h"

#include "device_monitor.h"
#include "mainwindow.h"
#include "size_para.h"
#include "custom_list_view.h"
#include "job_manager_window.h"
#include "gsettings_monitor.h"
#include "job_properties_window.h"
#include "ukui_printer.h"

JobManagerModel::JobManagerModel(const QString printerName, QStandardItemModel *parent)
    : QStandardItemModel(parent) ,
      m_printerName(printerName)
{
    QList<UkuiUsbPrinter> printers;
    QString printerUri{QString()};
    UkuiPrinterManager::getInstance().enumPrinters(printers);

    for (auto printer : printers) {
        if (printer.name == printerName) {
            printerUri = printer.device_uri;
            break;
        }
    }

    m_pageInfoHelper = new PageInfoHelper(printerUri);
    this->setRowCount(0);
    this->setColumnCount(jobList.size());
    for (int i = 0; i < jobList.size(); i++) {
        setHeaderData(i, Qt::Horizontal, jobList.at(i));
        this->horizontalHeaderItem(i)->setToolTip(jobList.at(i));
    }
    connect(m_pageInfoHelper, &PageInfoHelper::pageInfoUpdate, this, &JobManagerModel::setPageInfo);
    m_pageInfoHelper->startWorker();
}

void JobManagerModel::setPageInfo(int jobId, int jobImpressionsCompleted)
{
    m_jobId = jobId;
    m_jobImpressionsCompleted = jobImpressionsCompleted;
    qDebug() << m_printerName << " page info " << m_jobId << " " << m_jobImpressionsCompleted;
}

QString JobManagerModel::getJobTitleTranslation(const QString &title)
{
    if (title == "Test Page") {
        return tr("Test Page");
    }
    if (title == "untitled") {
        return tr("untitled");
    }
    return title;
}

bool JobManagerModel::setShowData(const QModelIndex &index, const QString &value, int role)
{
    this->itemFromIndex(index)->setToolTip(value);
    return this->QStandardItemModel::setData(index, value, role);
}

bool JobManagerModel::addJob(int row, const PrintJob &job)
{
    int i = 0;
    // 在表头插，这样最前面的就是最新的
    this->insertRow(row);
    this->setShowData(index(row, i++), QString::number(job.id));
    this->setShowData(index(row, i++), job.user == "unknown" ? tr("unknown") : job.user);
    this->setShowData(index(row, i++), getJobTitleTranslation(job.title));
    this->setShowData(index(row, i++), job.printerName);
    this->setShowData(index(row, i++), QString::number(job.size)+"kb");
    this->setShowData(index(row, i++), GsettingsMonitor::getInstance()->sysDateStyleGet(job.createTime));
    if (job.jobState == IPP_JSTATE_PROCESSING && m_jobImpressionsCompleted != -1) {
        this->setShowData(index(row, i++), tr("Printing page %1").arg(m_jobImpressionsCompleted + 1));
    } else {
        this->setShowData(index(row, i++), jobStatusStrList.at(job.jobState));
    }
    this->setData(index(row, 0), QVariant::fromValue(job.jobState), Qt::UserRole);
    return true;
}

bool JobManagerModel::removeJob(int row)
{
    this->removeRow(row);
    return true;
}

bool JobManagerModel::updateJob(int row, const PrintJob &job)
{
    int i = 0;
    this->setShowData(index(row, i++), QString::number(job.id));
    this->setShowData(index(row, i++), job.user == "unknown" ? tr("unknown") : job.user);
    this->setShowData(index(row, i++), getJobTitleTranslation(job.title));
    this->setShowData(index(row, i++), job.printerName);
    this->setShowData(index(row, i++), QString::number(job.size)+"kb");
    this->setShowData(index(row, i++), GsettingsMonitor::getInstance()->sysDateStyleGet(job.createTime));
    if (job.jobState == IPP_JSTATE_PROCESSING && m_jobImpressionsCompleted != -1) {
        this->setShowData(index(row, i++), tr("Printing page %1").arg(m_jobImpressionsCompleted + 1));
    } else {
        this->setShowData(index(row, i++), jobStatusStrList.at(job.jobState));
    }
    this->setData(index(row, 0), QVariant::fromValue(job.jobState), Qt::UserRole);
    return true;
}

bool JobManagerModel::updateJobs(QList<PrintJob> jobs)
{
    // remove update
    
    for (int i = 0; i < this->rowCount();) {
        int jobid = this->data(index(i,0)).toInt();
        bool check = false;
        for (auto job : jobs) {
            if (jobid == job.id) {
                check = true;
                updateJob(i, job);
            }
        }

        if (check == false) {
            removeJob(i);
        } else {
            i++;
        }
    }

    // add
    for (auto job : jobs) {
        int jobid = job.id;
        if (this->rowCount() == 0) {
            addJob(0, job);
            continue;
        }

        bool check = false;
        for (int i = 0; i < this->rowCount(); i++) {
            if (job.id > this->data(index(i,0)).toInt()) {
                check = true;
                addJob(i, job);
                break;
            } else if (jobid == this->data(index(i,0)).toInt()) {
                break;   
            }
        }

    }
    return true;
}

int JobManagerModel::getStatus(const QModelIndex &index)
{
    int status = index.data(Qt::UserRole).value<int>();
    return status;
}

// ToDo: 接口先留出来
void JobManagerModel::resetFont(QFont font)
{
    for (int i = 0; i < this->rowCount(); i++) {
        for (int j = 0; j < this->columnCount(); j++) {
            this->item(i,j)->setFont(font);
        }
    }
    return ;
}

JobManagerModel::~JobManagerModel()
{
    qDebug() << "im in JobManagerModel::~JobManagerModel()";
    m_pageInfoHelper->finished();
}


// JobMenu
JobMenu::JobMenu(QWidget *parent)
    : QMenu(parent)
{
    QList<QAction *> actions;
    this->m_actionCancel = new QAction(this);
    this->m_actionCancel->setText(tr("Cancel print"));
    this->m_actionDelete = new QAction(this);
    this->m_actionDelete->setText(tr("Delete print"));
    this->m_actionHold = new QAction(this);
    this->m_actionHold->setText(tr("Hold print"));
    this->m_actionRelease = new QAction(this);
    this->m_actionRelease->setText(tr("Release print"));
    this->m_actionReprint = new QAction(this);
    this->m_actionReprint->setText(tr("Reprint"));
    this->m_actionMove= new QAction(this);
    this->m_actionMove->setText(tr("Use other printer..."));
    this->m_actionProperties= new QAction(this);
    this->m_actionProperties->setText(tr("Job properties"));
    {
        m_moveMenu = new QMenu();
        QList<QAction *> actions;
        QList<UkuiUsbPrinter> lis;
        UkuiPrinterManager::getInstance().enumPrinters(lis);
        for (auto info : lis) {
            QAction *act = new QAction(m_moveMenu);
            act->setText(info.name);
            actions << act;
        }
        m_moveMenu->addActions(actions);
        m_actionMove->setMenu(m_moveMenu);
        this->addMenu(m_moveMenu);
    }
    // QMenu = new moveMenu();
    actions << this->m_actionCancel
            << this->m_actionDelete
            << this->m_actionHold
            << this->m_actionRelease
            << this->m_actionReprint
            << this->m_actionMove
            << this->m_actionProperties
            ;

    this->addActions(actions);
}

JobMenu::~JobMenu()
{

}


// JobManagerWindow
JobManagerWindow::JobManagerWindow(QString printerName, UkuiQueueStatus status, QWidget *parent)
    : CustomWidget(parent, true),
      m_printerName(printerName),
      m_status(status)
{
    QString makeObjectName = printerName == "" ? QString("job list") : printerName + " " + QString("job list");
    this->setObjectName(makeObjectName);
    this->initUi();
    this->initConnect();
    this->updateModel();
    this->initTimer();
}

JobManagerWindow::~JobManagerWindow()
{
    qDebug() << "JobManagerWindow::~JobManagerWindow()";
}

void JobManagerWindow::updateModel()
{
    m_jobs.clear();
    getJobs(m_jobs, m_printerName, m_status);
    m_model->updateJobs(m_jobs);
}

bool JobManagerWindow::compareJobs(const QList<PrintJob> &a, const QList<PrintJob> &b) const
{
    if (a.size() != b.size()) return false;
    for (int i = 0; i < a.size(); i++) {
        PrintJob aa = a.at(i);
        PrintJob bb = b.at(i);
        if (
            aa.id       != bb.id
         || aa.jobState != bb.jobState
        ) {
            return false;
        }
    }
    return true;
}

void JobManagerWindow::getJobs(QList<PrintJob> &jobls, const QString &printerName, const UkuiQueueStatus &status)
{

    if (printerName.isEmpty()) {
        QList<UkuiUsbPrinter> printerList;
        UkuiPrinterManager::getInstance().enumPrinters(printerList);
        for (auto info : printerList) {
            UkuiPrinterManager::getInstance().getPrintQueue(info.name, status, jobls);
        }
    }
    else {
        UkuiPrinterManager::getInstance().getPrintQueue(printerName, status, jobls);
    }
    return ;
}

void JobManagerWindow::initView()
{
    m_view.reset(new CustomListView());

    // 设置右键触发customContextMenuRequested信号
    m_view.get()->setContextMenuPolicy(Qt::CustomContextMenu);

    // 最后一格拉伸到最后
    // m_view.get()->horizontalHeader()->setStretchLastSection(true);
    // this->view->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    this->m_model = new JobManagerModel(m_printerName);
    m_view.get()->setModel(this->m_model);
}

void JobManagerWindow::initTimer()
{
    this->m_timer = new QTimer(this);
    connect(this->m_timer, &QTimer::timeout, this, [=](){
        emit updateModel();
    });
    this->m_timer->start(1000);
}

void JobManagerWindow::initUi()
{
    this->setWidgetName(m_printerName.isEmpty() ? tr("Printer") : m_printerName);
    this->setMinimumSize(820,420);
    this->initView();
    this->initMenu();

    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(m_view.get());
    layout->setContentsMargins(24,16,24,24);
    this->baseBar()->setLayout(layout);
    setAttribute(Qt::WA_DeleteOnClose);
}

void JobManagerWindow::initConnect()
{
    connect(m_view.get(), &QTableView::customContextMenuRequested, this, &JobManagerWindow::ShowContextMenu);
    connect(GsettingsMonitor::getInstance(), &GsettingsMonitor::sysDateStyleUpdate, this, [=](){
        updateModel();
    });
}

void JobManagerWindow::initMenu()
{
    // menu = new JobMenu(this);
}

void JobManagerWindow::ShowContextMenu(const QPoint& pos)
{
    this->m_selectList = m_view.get()->selectionModel()->selectedIndexes();
    // selectList = this->view->selectionModel()->selectedRows(0);
    // qDebug() << this->selectList.size();
    // for(auto select : this->selectList) {
    //     qDebug() << select.row()<<" "<<select.data().toString();
    // }
    // qDebug() << pos;
    // index = view->indexAt(view->mapFromGlobal(QCursor::pos()));
    // qDebug() << index.row() << index.column();
    JobMenu menu;
    connect(&menu, &QMenu::triggered, this, &JobManagerWindow::triggerMenu);
    int idx = -1;
    // 多行暂不处理
    if ((m_view.get()->indexAt(pos).row() < 0 && this->m_selectList.size() == 0)
     || (this->m_selectList.size() / jobList.size() != 1)) {
        ;
    }
    else if (this->m_selectList.size() / jobList.size() == 1) {
        QString stu = this->m_selectList.at(6).data().toString();
        idx =  m_model->getStatus(this->m_selectList.at(0));
        qDebug() << stu << " " << idx; 
    }

    bool existFile = true;
    for (int i = 0; i < m_selectList.size(); i+= jobList.size()) {
        int jobId = m_selectList.at(i).data().toInt();
        PrintJob inJobs;
        if (!getJob(inJobs, jobId)) {
            break;
        }
        int id = m_selectList.at(i).data().toInt();
        if (id != inJobs.id) {
            break;
        }
        if (existFile == true) {
            existFile = inJobs.preserved;
        }
    }
    this->setMenuActionEnabled(menu, idx, existFile);
	menu.exec(QCursor::pos());
}

void JobManagerWindow::setMenuActionEnabled(JobMenu &menu, const int &status, bool existFile)
{
    menu.m_actionCancel->setEnabled(false);
    menu.m_actionDelete->setEnabled(false);
    menu.m_actionHold->setEnabled(false);
    menu.m_actionRelease->setEnabled(false);
    menu.m_actionReprint->setEnabled(false);
    menu.m_actionMove->setEnabled(false);
    menu.m_actionProperties->setEnabled(false);

    switch (status)
    {
    case -1:
        break;
    case IPP_JSTATE_PENDING:
        menu.m_actionCancel->setEnabled(true);
        menu.m_actionHold->setEnabled(true);
        menu.m_actionMove->setEnabled(true);
        menu.m_actionProperties->setEnabled(true);
        break;
    case IPP_JSTATE_HELD:
        menu.m_actionCancel->setEnabled(true);
        menu.m_actionRelease->setEnabled(true);
        menu.m_actionProperties->setEnabled(true);
        break;
    case IPP_JSTATE_PROCESSING:
        menu.m_actionCancel->setEnabled(true);
        menu.m_actionProperties->setEnabled(true);
        break;
    case IPP_JSTATE_STOPPED:
        menu.m_actionProperties->setEnabled(true);
        break;
    case IPP_JSTATE_CANCELED:
        menu.m_actionDelete->setEnabled(true);
        menu.m_actionProperties->setEnabled(true);
        break;
    case IPP_JSTATE_ABORTED:
        menu.m_actionProperties->setEnabled(true);
        break;
    case IPP_JSTATE_COMPLETED:
        menu.m_actionDelete->setEnabled(true);
        menu.m_actionReprint->setEnabled(true);
        menu.m_actionProperties->setEnabled(true);
        break;
    default:
        break;
    }
    if (menu.m_actionReprint->isEnabled() == true) {
        menu.m_actionReprint->setEnabled(existFile);
    }
    return ;
}

bool JobManagerWindow::getJob(PrintJob &job, const int &id) const
{
    for (auto j : this->m_jobs) {
        if (j.id == id) {
            job = j;
            return true;
        }
    }
    return false;
}

void JobManagerWindow::triggerMenu(QAction *action)
{
    QString str = action->text();
    // qDebug() << str;
    int clickOk = 0;
    for (int i = 0; i < m_selectList.size(); i+= jobList.size()) {
        int jobId = m_selectList.at(i).data().toInt();
        PrintJob inJobs;
        if (!getJob(inJobs, jobId)) {
            break;
        }
        QString printerName = m_selectList.at(i + 3).data().toString();
        int jobState =  m_model->getStatus(this->m_selectList.at(i));
        if (printerName != inJobs.printerName
         || jobState != inJobs.jobState) {
            break;
        }
        clickOk = 1;
        if(tr("Cancel print") == str) {
            UkuiPrinterManager::getInstance().cancelJob(printerName, jobId);
        }
        else if(tr("Delete print") == str) {
            UkuiPrinterManager::getInstance().deleteJob(printerName, jobId);
        }
        else if(tr("Hold print") == str) {
            UkuiPrinterManager::getInstance().holdJob(jobId);
        }
        else if(tr("Release print") == str) {
            UkuiPrinterManager::getInstance().releaseJob(jobId);
        }
        else if(tr("Reprint") == str) {
            UkuiPrinterManager::getInstance().restartJob(jobId);
        }
        else if(tr("Job properties") == str) {
            JobPropertiesWindow *w = new JobPropertiesWindow(jobId, this);
            w->show();

        }
        // move to 指定打印机
        else {
            if (printerName == str) {
                clickOk = -1;
                break;
            }
            UkuiPrinterManager::getInstance().moveJob(printerName, str, jobId);
        }
    }
    if (clickOk == 1) {
        updateModel();
    }
    else if (clickOk == 0){
        CustomMessageBox Msg(CustomMessageBox::Warning, tr("Warning"), tr("Set error: Job status has been updated!"), CustomMessageBox::NoButton, this);
        Msg.addButton(tr("Sure"), CustomMessageBox::YesRole);
        Msg.exec();
    }
    else if (clickOk == -1) {
        CustomMessageBox Msg(CustomMessageBox::Warning, tr("Warning"), tr("Cannot move job to itself!"), CustomMessageBox::NoButton, this);
        Msg.addButton(tr("Sure"), CustomMessageBox::YesRole);
        Msg.exec();
    }
    else {
        qDebug() << "Some error in triggerMenu!";
    }
    return ;
}

void JobManagerWindow::showThis()
{
    if (this->isHidden()) {
        this->show();
    }
    else {
        this->activateWindow();
    }
}

void JobManagerWindow::hideThis()
{
    if (this->isHidden()) {
        return ;
    }
    else {
        this->hide();
    }
}

JobManagerWindow *JobManagerWindow::getInstance(QString printerName, UkuiQueueStatus status, QWidget *parent)
{
    QString makeObjectName = printerName == "" ? QString("job list") : printerName + " " + QString("job list");
    QWidgetList lswid = QApplication::allWidgets();
    for (auto wid : lswid) {
        // qDebug () << wid->objectName();
        if (wid->objectName() == makeObjectName) {
            return static_cast<JobManagerWindow *>(wid);
        }
    }
    JobManagerWindow *p = new JobManagerWindow(printerName, status, parent);
    return p;
}
