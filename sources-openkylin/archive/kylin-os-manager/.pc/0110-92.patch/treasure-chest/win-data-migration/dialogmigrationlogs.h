#ifndef DIALOGMIGRATIONLOGS_H
#define DIALOGMIGRATIONLOGS_H

#include "constant.h"
#include <kwidget.h>
#include <QStandardPaths>
#include <QStandardItemModel>
#include <QTextEdit>
#include "dialogviewdelegate.h"

class DialogMigrationLogsCommon : public kdk::KWidget
{
    Q_OBJECT
public:
    DialogMigrationLogsCommon(QWidget *parent = nullptr);
    virtual void updateAndActive(QString fileName = "") = 0;

protected:
    QString m_logPath =
        QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/kylin-os-manager/win-data-migration/";
};

class DialogMigrationLogs : public DialogMigrationLogsCommon
{
    Q_OBJECT
public:
    static DialogMigrationLogs *getInstance();
    void updateAndActive(QString fileName = "");
    void updateView();

private:
    void updateViewPrivate();
    DialogMigrationLogs(QWidget *parent = nullptr);
    static DialogMigrationLogs *m_instance;
    QStandardItemModel *m_model = nullptr;
    DialogViewDelegate *m_deleget = nullptr;

private slots:
    void onTreeViewItemClick(const QModelIndex &index);
};

class DialogMigrationLogsChild : public DialogMigrationLogsCommon
{
    Q_OBJECT
public:
    static DialogMigrationLogsChild *getInstance();
    void updateAndActive(QString fileName = "");

private:
    DialogMigrationLogsChild(QWidget *parent = nullptr);
    static DialogMigrationLogsChild *m_instance;
    QTextEdit *m_textEdit = nullptr;
};

class DialogHowToUse : public DialogMigrationLogsCommon
{
    Q_OBJECT
public:
    static DialogHowToUse *getInstance();
    void updateAndActive(QString fileName = "");

private:
    void resizeEvent(QResizeEvent *e);
    DialogHowToUse(QWidget *parent = nullptr);
    static DialogHowToUse *m_instance;
    QLabel *m_labelPix = nullptr;
    QTextEdit *m_textEdit = nullptr;
    QPixmap m_pix = QPixmap(":/res/howtouse.png");
};

#endif // DIALOGMIGRATIONLOGS_H
