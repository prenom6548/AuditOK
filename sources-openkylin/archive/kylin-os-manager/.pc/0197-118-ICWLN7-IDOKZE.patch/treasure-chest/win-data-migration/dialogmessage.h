#ifndef DIALOGMESSAGE_H
#define DIALOGMESSAGE_H

#include "applogic.h"
#include "dialogcommon.h"
#include <kdialog.h>
#include <kprogressbar.h>
#include <QStandardPaths>
#include <QMessageBox>
#include <kwidget.h>
#include "common/text_label.h"
#include <QTextBrowser>

class DialogMessageBox : public kdk::KWidget
{
    Q_OBJECT
private:
    QPushButton *m_iconBtn = nullptr;
    QPushButton *m_cannelBtn = nullptr;
    QTextBrowser *m_browser = nullptr;
public:
    DialogMessageBox(QWidget *parent = nullptr);
    void setText(QString strText);
    bool isStarMigration =false;
protected:
    void closeEvent(QCloseEvent *event) override;
signals:
    void signalWidgetClose();
};

class DialogMessage : public DialogCommon
{
    Q_OBJECT

signals:
    void resave(bool);
    void selectiveSource(int type);
    void signalCannelGetData();

public:
    DialogMessage(QWidget *parent = nullptr);
    ~DialogMessage();
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
    void showInteractionBox(int type, QString str = "");
    void showTimeOutTip();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onBox2rejected();

private:
    void initUI();
    void initInteractionBox();
    void initWidgetErrList();
    void errorDialog(const QString &err);
    DialogCommon *m_dialogErrList = nullptr;
    QStandardItemModel *m_listViewModel = nullptr;
    QLabel *m_icon = nullptr;
    K::TextLabel *m_labelMessage = nullptr;
    kdk::KProgressBar *m_progressBar = nullptr;
    QPushButton *m_cancel = nullptr;
    QPushButton *m_viewDetails = nullptr;
    bool *m_cancelHandle = nullptr;
    QString m_logPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/kylin-os-manager/win-data-migration/";
    SmbUrlInfoList m_errList;
    QMessageBox *m_interactionBox1 = nullptr;
    QMessageBox *m_interactionBox2 = nullptr;
    DialogMessageBox *m_interactionWidget2 = nullptr;
    bool m_userRejected = true;

    QMessageBox *m_messageBoxTip = nullptr;
};

#endif // DIALOGMESSAGE_H
