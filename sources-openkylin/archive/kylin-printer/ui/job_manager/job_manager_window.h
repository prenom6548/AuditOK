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
#ifndef JOB_MANAGER_WINDOW_H
#define JOB_MANAGER_WINDOW_H

#include <QWidget>
#include <QStandardItemModel>
#include <QMenu>
#include <QTableWidget>
#include "custom_widget.h"
#include "ukui_printer.h"
#include "device_information.h"
#include "custom_list_view.h"
#include "page_info_helper.h"


/*
|-----------------------------------------------------------------------------|
| status     | printer can do                                                 |
|-----------------------------------------------------------------------------|
| -          | cancel | delete | hold | release | move | reprint | properties |
| pending    | Y      |        | Y    |         | Y    |         | Y          |
| held       | Y      |        |      | Y       |      |         | Y          |
| processing | Y      |        |      |         |      |         | Y          |
| canceled   |        | Y      |      |         |      |         | Y          |
| completed  |        | Y      |      |         |      | Y       | Y          |
|-----------------------------------------------------------------------------|
*/

// about jobStatusStrList, u can see ipp_jstate_e in ipp.h for help
#ifndef JOB_FLAG
#define JOB_FLAG                                   \
QStringList jobList = {                            \
    QObject::tr("id"),                             \
    QObject::tr("user"),                           \
    QObject::tr("title"),                          \
    QObject::tr("printer name"),                   \
    QObject::tr("size"),                           \
    QObject::tr("create time"),                    \
    QObject::tr("job state"),                      \
};                                                 \
QStringList jobStatusStrList = {                   \
    "",                                            \
    "",                                            \
    "",                                            \
    QObject::tr("Job is waiting to be printed"),   \
    QObject::tr("Job is held for printing"),       \
    QObject::tr("Job is currently printing"),      \
    QObject::tr("Job has been stopped"),           \
    QObject::tr("Job has been canceled"),          \
    QObject::tr("Job has aborted due to error"),   \
    QObject::tr("Job has completed successfully"), \
};
#endif

class JobManagerModel : public QStandardItemModel
{
    Q_OBJECT
private:
    JOB_FLAG
    QString m_printerName{QString()};
    int m_jobId{-1};
    int m_jobImpressionsCompleted{-1};
    PageInfoHelper *m_pageInfoHelper{nullptr};
    void setPageInfo(int jobId, int jobImpressionsCompleted);
public:
    explicit JobManagerModel(const QString printerName, QStandardItemModel *parent = nullptr);
    ~JobManagerModel();
    bool setShowData(const QModelIndex &index, const QString &value, int role = Qt::EditRole);
    bool addJob(int row, const PrintJob &job);
    bool removeJob(int row);
    bool updateJob(int row, const PrintJob &job);
    bool updateJobs(QList<PrintJob> jobs);
    void resetFont(QFont font);
    QString getJobTitleTranslation(const QString &title);
    int getStatus(const QModelIndex &index);
};

class JobMenu : public QMenu
{
    Q_OBJECT
private:

public:
    explicit JobMenu(QWidget *parent = nullptr);
    ~JobMenu();
    QAction *m_actionCancel;
    QAction *m_actionDelete;
    QAction *m_actionHold;
    QAction *m_actionRelease;
    QAction *m_actionReprint;
    QAction *m_actionMove;
    QAction *m_actionProperties;

    QMenu *m_moveMenu;
};


class JobManagerWindow : public CustomWidget
{
    Q_OBJECT
private:
    JOB_FLAG
    QTimer *m_timer;
    QString m_printerName;
    UkuiQueueStatus m_status;
    QList<PrintJob> m_jobs;
    std::unique_ptr<CustomListView> m_view{nullptr};
    JobManagerModel *m_model;
    QModelIndexList m_selectList;
    void getJobs(QList<PrintJob> &jobls, const QString &printerName, const UkuiQueueStatus &status);
    bool getJob(PrintJob &job, const int &id) const;
    bool compareJobs(const QList<PrintJob> &a, const QList<PrintJob> &b) const;
    void initView();
    void initTimer();
    void initMenu();
    void initUi();
    void initConnect();
    void ShowContextMenu(const QPoint &pos);
    void triggerMenu(QAction *action);
    void updateModel();
    void setMenuActionEnabled(JobMenu &menu, const int &status, bool existFile);
    explicit JobManagerWindow(QString printerName = QString(),
                              UkuiQueueStatus status = UkuiQueueStatus::UKUI_PRINTER_QUEUE_ALL,
                              QWidget *parent = nullptr);
public:
    static JobManagerWindow *getInstance(QString printerName = QString(),
                                        UkuiQueueStatus status = UkuiQueueStatus::UKUI_PRINTER_QUEUE_ALL,
                                        QWidget *parent = nullptr);
    void showThis();
    void hideThis();
    ~JobManagerWindow();
};

#endif // JOB_MANAGER_WINDOW_H
