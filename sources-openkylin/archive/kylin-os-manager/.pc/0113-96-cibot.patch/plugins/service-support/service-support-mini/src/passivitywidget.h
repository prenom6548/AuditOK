#ifndef PASSIVITY_WIDGET_H
#define PASSIVITY_WIDGET_H

#include <QWidget>
#include <QCheckBox>
#include <QPushButton>
#include <QProcess>
#include <kballontip.h>
#include <kdialog.h>
#include "feedbackcommon.h"

class PassivityWidget: public QWidget, public FeedBackCommon {
    Q_OBJECT
public:
    PassivityWidget(QWidget *parent = nullptr);
    void run();

private Q_SLOTS:
    void creatFinish(FeedBackFinishType type, QString str);
    void creatFinishNoUI(FeedBackFinishType type, QString str);
    void exportLogsFinish(int code);
    void on_UpdateBlacklist(bool ok);

private:
    void initUI();
    void initDialog();
    void onButtonClick();
    void onDialogButtonClick();
    void exportLogs();
    void onthemeChange();
    void showMessageBox(const QString &msg, kdk::TipType type, QWidget *parent = nullptr);
    QString ExecName(const QString &exec);

    QCheckBox *m_logCheckbox = nullptr;
    QCheckBox *m_restartApp = nullptr;
    QPushButton *m_submit = nullptr;
    QProcess *m_exportLogsProc = nullptr;
    kdk::KDialog *m_dialog = nullptr;
    QPushButton *m_dialogSubmit = nullptr;
    /**
     * @brief 部分 desktop 的名字与二进制的名字不一致，此处对这部分二进制做一下映射。
     * @note <二进制名, desktop 名>
     */
    QHash<QString, QString> exec_desktop_map_;
};

#endif
