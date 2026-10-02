#include "kom_buriedpoint.h"
#include <vector>
#include <libkydatacollect.h>
#include <QDebug>

namespace kom {

QMap<enum BuriedPoint::BuriedPointPage, QString> BuriedPoint::buried_point_page_map_ = {
    {BuriedPointPage::kMainFrame, "main_frame"},
    {BuriedPointPage::kServiceSupport, "service_support"},
    {BuriedPointPage::kFaultDiagnosis, "fault_diagnosis"},
    {BuriedPointPage::kJunkClean, "junk_clean"},
    {BuriedPointPage::kDriverController, "driver_controller"},
    {BuriedPointPage::kToolBox, "tool_box"},
    {BuriedPointPage::kWinDataMigration, "win_data_migration"},
    {BuriedPointPage::kRemoteAssistanceTool, "remote_assistance_tool"},
    {BuriedPointPage::kMonitorService, "monitor_service"},
    {BuriedPointPage::kExperienceInformationCollection, "experience_information_collection"},
    {BuriedPointPage::kServiceSupportMini, "service_support_mini"}
};

QMap<enum BuriedPoint::EventCode, QString> BuriedPoint::event_code_map_ = {
    {BuriedPoint::EventCode::kJumpTo, "jump_to"},
    {BuriedPoint::EventCode::kNavigationClick, "navigation_click"},
    {BuriedPoint::EventCode::kFaultDiagnosis, "fault_diagnosis"},
    {BuriedPoint::EventCode::kDiagnosisFailed, "diagnosis_failed"},
    {BuriedPoint::EventCode::kFaultRepair, "fault_repair"},
    {BuriedPoint::EventCode::kRepairFailed, "repair_failed"},
    {BuriedPoint::EventCode::kCleanCompleted, "clean_completed"},
    {BuriedPoint::EventCode::kSubmit, "submit"},
    {BuriedPoint::EventCode::kCancelSubmission, "cancel_submission"},
    {BuriedPoint::EventCode::kSwitchTab, "switch_tab"},
    {BuriedPoint::EventCode::kGraphicsCardInformation, "graphics_card_information"},
    {BuriedPoint::EventCode::kPrinterInformation, "printer_information"},
    {BuriedPoint::EventCode::kScannerInformation, "scanner_information"},
    {BuriedPoint::EventCode::kStartApplication, "start_application"},
    {BuriedPoint::EventCode::kMonitorMachineId, "monitor_machine_id"},
    {BuriedPoint::EventCode::kRemoteAssistanceOperation, "remote_assistance_operation"},
    {BuriedPoint::EventCode::kRemoteAssistanceClient, "remote_assistance_client"},
    {BuriedPoint::EventCode::kStartMigration, "start_migration"},
    {BuriedPoint::EventCode::kCancelMigration, "cancel_migration"},
    {BuriedPoint::EventCode::kEstablishingConnection, "establishing_connection"},
    {BuriedPoint::EventCode::kFaultRepairToolRepairSuccessful, "fault_repair_tool_repair_successful"},
    {BuriedPoint::EventCode::kFaultDiagnosisReport, "fault_diagnosis_report"}
};

void BuriedPoint::uploadMessage(enum BuriedPointPage buried_point_page, enum EventCode event_code, const QMap<QString, QString> &custom_property) {
    if (buried_point_page_map_.find(buried_point_page) == buried_point_page_map_.end()) {
        qCritical() << "Buried point page is not exist.";
        return;
    }
    if (event_code_map_.find(event_code) == event_code_map_.end()) {
        qCritical() << "Buried point event code is not exist.";
        return;
    }

    KTrackData *track_data = kdk_dia_data_init(KEventSourceType::KEVENTSOURCE_DESKTOP, KEventType::KEVENT_CLICK);

    int cursor{0};
    int property_size{custom_property.size()};
    std::vector<KCustomProperty> property(property_size);
    for (auto iter = custom_property.cbegin(); iter != custom_property.cend(); iter++) {
        QByteArray key = iter.key().toUtf8();
        QByteArray value = iter.value().toUtf8();
        property[cursor].key = strdup(key.data());
        property[cursor].value = strdup(value.data());
        cursor++;
    }
    kdk_dia_append_custom_property(track_data, property.data(), property_size);

    QByteArray event_code_utf8 = event_code_map_[event_code].toUtf8();
    QByteArray buried_point_page_utf8 = buried_point_page_map_[buried_point_page].toUtf8();
    kdk_dia_upload_default(track_data,
                           event_code_utf8.data(),
                           buried_point_page_utf8.data());

    for (const auto &p : property) {
        if (p.key) {
            free(p.key);
        }
        if (p.value) {
            free(p.value);
        }
    }

    kdk_dia_data_free(track_data);
}

void BuriedPoint::uploadMessage(enum BuriedPointPage buried_point_page, enum EventCode event_code) {
    uploadMessage(buried_point_page, event_code, QMap<QString, QString>());
}

void BuriedPoint::uploadProgramStartup() {
    kdk_dia_upload_program_startup();
}

}
