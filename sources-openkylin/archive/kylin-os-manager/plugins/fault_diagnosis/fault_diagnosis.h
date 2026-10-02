#ifndef KYLIN_OS_MANAGER_PLUGIN_FAULT_DIAGNOSIS_FAULT_DIAGNOSIS_H
#define KYLIN_OS_MANAGER_PLUGIN_FAULT_DIAGNOSIS_FAULT_DIAGNOSIS_H

#include <string>
#include <QWidget>
#include <QScopedPointer>
#include <QStackedWidget>
#include <QThread>
#include <QCloseEvent>
#include "kom_application_interface.h"
#include "home_page_widget.h"
#include "defines.h"

namespace fault_diagnosis {

class MainWindow;

class FaultDiagnosis: public QObject, public KomApplicationInterface {
    Q_OBJECT
public:
    FaultDiagnosis();
    virtual ~FaultDiagnosis();
    virtual void init(void (*frame_callback)(const char * function_name, ...)) override;
    virtual void communication(enum CommunicationType communication_type, QVariant data) override;
    virtual std::string name() override;
    virtual std::string i18nName() override;
    virtual std::string icon() override;
    virtual int sort() override;
    virtual QWidget *createWidget() override;

Q_SIGNALS:
    void sig_RepairSpecificError(QString module, QString path, QString error_code, QString error_data);
    void sig_DiagnosisSpecificType(QString type);

private:
    enum StackedWidget {
        kHomePage,
        kMainWindow,
        kRepairSpecificError
    };

    QStackedWidget *                         stacked_widget_;
    MainWindow *                             mainwindow_;
    HomePageWidget *                         home_page_;
    QThread *                                thread_;                       // 此线程跟随整个进程一起退出，堆内存进程结束后由操作系统来回收
    QHash<QString, enum DiagnosisType>       diagnosis_type_map_;           // 记录支持的指定检测类型
    QHash<QString, enum SpecificErrorModule> specific_error_module_map_;    // 记录支持的错误修复组件

    bool thread_started_;
    bool start_diagnosis_cache_;
};

class FaultDiagnosisProvider: public KomApplicationProvider {
public:
    KomApplicationInterface *create() const override;
};

}

Q_DECLARE_METATYPE(QCloseEvent *)

#endif
