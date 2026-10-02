#ifndef WIDGETCONNECT_H
#define WIDGETCONNECT_H

#include "applogic.h"
#include "widgetmigration.h"
#include "widgetcommon.h"
#include <QLineEdit>
#include <kpasswordedit.h>

class WidgetConnect : public WidgetCommon
{
    Q_OBJECT

signals:
    void connectSmbServer(const SmbConf &);
    void startLoad();

public:
    WidgetConnect(QWidget *parent = nullptr);

public slots:
    void connetcResult(int errCode, QVariant var = QVariant());

private slots:
    void onConnbtnClick();
    void textChanged(const QString &str);

private:
    void createAppLogic();
    void initConnect();
    void initPrompt();
    void initUI();
    void initMigration();
    bool eventFilter(QObject *target, QEvent *event);

    bool m_showPrompt = true;
    QString m_projectName = "win-data-migration";
    WidgetMigration *m_widgetMigration = nullptr;
    AppLogic *m_appLogic = nullptr;
    QStandardItemModel *m_itemModel = nullptr;
    QLineEdit *m_lineEditIP = nullptr;
    QLineEdit *m_lineEditWorkGroup = nullptr;
    QLineEdit *m_lineEditUserName = nullptr;
    kdk::KPasswordEdit *m_lineEditPassWord = nullptr;
    QPushButton *m_connbtn = nullptr;
    bool *m_cancelHandle = nullptr;
    bool m_isConnect = false;

    enum Customize { NONE = 0, DEFAULT, ICBC };
    static Customize getCustomize();
    static Customize m_customize;
};

#endif // WIDGETCONNECT_H
