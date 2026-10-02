#ifndef KOM_BURIEDPOINT_H
#define KOM_BURIEDPOINT_H

#include <QString>
#include <QMap>

namespace kom {

class BuriedPoint {
public:
    enum class BuriedPointPage {
        kMainFrame,                          // 主框架
        kServiceSupport,                     // 服务支持
        kFaultDiagnosis,                     // 故障诊断
        kJunkClean,                          // 垃圾清理
        kDriverController,                   // 驱动管理
        kToolBox,                            // 百宝箱
        kWinDataMigration,                   // 换机助手
        kRemoteAssistanceTool,               // 远程协助工具
        kMonitorService,                     // 监控服务
        kExperienceInformationCollection,    // 用户体验信息收集
        kServiceSupportMini,                 // 服务支持 mini
    };
    enum class EventCode {
        // 主框架
        kJumpTo,                             // 主界面跳转事件(custom_property: jumpTabName)
        kNavigationClick,                    // 主界面导航栏点击事件(custom_property: navigationName)

        // 故障诊断
        kFaultDiagnosis,                     // 诊断结果(custom_property: systemVersion, systemArch, totalNumber, numberOfSuccessful, numberOfFailed)
        kDiagnosisFailed,                    // 诊断失败的项(custom_property: systemVersion, systemArch, detectionItemMark, detectionItemName)
        kFaultRepair,                        // 修复结果(custom_property: systemVersion, systemArch, totalNumber, numberOfSuccessful, numberOfFailed)
        kRepairFailed,                       // 修复失败的项(custom_property: systemVersion, systemArch, errorCode, errorName, errorDescription)

        // 垃圾清理
        kCleanCompleted,                     // 清理完成(custom_property: cleanSize)

        // 服务支持
        kSubmit,                             // 提交(custom_property: submitType)
        kCancelSubmission,                   // 取消提交
        kSwitchTab,                          // 切换 Tab 页(custom_property: tabName)

        // 驱动管理
        kGraphicsCardInformation,            // 获取到的显卡信息(custom_property: vendor, model, name, localDriver, localDriverVersion, vid, pid, toBeInstalledDriver, toBeInstalledDriverVersion)
        kPrinterInformation,                 // 获取到的打印机信息(custom_property: vendor, model, name, localDriver, localDriverVersion, vid, pid, toBeInstalledDriver, toBeInstalledDriverVersion)
        kScannerInformation,                 // 获取到的扫描仪信息(custom_property: vendor, model, name, localDriver, localDriverVersion, vid, pid, toBeInstalledDriver, toBeInstalledDriverVersion)

        // 百宝箱
        kStartApplication,                   // 百宝箱应用启动事件(custom_property: appStartPath)

        // 监控服务
        kMonitorMachineId,                   // 机器 ID(custom_property: machineId)

        // 远程协助工具
        kRemoteAssistanceOperation,          // 远程协助工具运维端事件(custom_property: functionName)
        kRemoteAssistanceClient,             // 远程协助工具客户端事件(custom_property: functionName)

        // 换机助手
        kStartMigration,                     // 开始迁移
        kCancelMigration,                    // 取消迁移
        kEstablishingConnection,             // 建立连接

        // 用户体验信息收集
        kFaultRepairToolRepairSuccessful,    // 故障修复类小工具修复成功(costum_property: name, uuid)

        // 服务支持 mini
        kFaultDiagnosisReport,               // 故障诊断上报(costum_property: applicationName, errorCode, submitTime, taskNumber)
    };

    BuriedPoint() = default;
    ~BuriedPoint() = default;
    static void uploadMessage(enum BuriedPointPage buried_point_page, enum EventCode event_code);
    static void uploadMessage(enum BuriedPointPage buried_point_page, enum EventCode event_code, const QMap<QString, QString> &custom_property);
    static void uploadProgramStartup();

private:
    static QMap<enum BuriedPointPage, QString> buried_point_page_map_;
    static QMap<enum EventCode, QString> event_code_map_;
};

}

#endif
