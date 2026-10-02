#ifndef DIALOGMESSAGE_H
#define DIALOGMESSAGE_H

#include "applogic.h"
#include "dialogcommon.h"
#include <kdialog.h>
#include <kprogressbar.h>
#include <QStandardPaths>
#include <kwidget.h>

class DialogMessage : public DialogCommon
{
    Q_OBJECT

signals:
    void resave();

public:
    DialogMessage(QWidget *parent = nullptr);

public slots:
    void connetcResult(int errCode, QVariant var = QVariant());
    void saveProgressValue(int num);
    void saveProgressMax(int num);
    void downloadingFile(const QString &str);
    void onCancelClick();
    void onViewDetailsClick();
    void onResaveClick();
    void setCancelHandle(bool *cancelHandle);
    void clearAndShow();
    void cancelFileMigrate();
protected:
    void closeEvent(QCloseEvent *event) override;
private:
    void initUI();
    void initWidgetErrList();
    void errorDialog(const QString &err);
    DialogCommon *m_dialogErrList = nullptr;
    QStandardItemModel *m_listViewModel = nullptr;
    QLabel *m_icon = nullptr;
    QLabel *m_labelMessage = nullptr;
    kdk::KProgressBar *m_progressBar = nullptr;
    QPushButton *m_cancel = nullptr;
    QPushButton *m_viewDetails = nullptr;
    bool *m_cancelHandle = nullptr;
    QString m_logPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/kylin-os-manager/win-data-migration/";
    SmbUrlInfoList m_errList;
};

#endif // DIALOGMESSAGE_H
