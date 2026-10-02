#include "fault_diagnosis.h"

extern "C"
bool kyconnect(kyplugin::Host &host) {
    host.add(new fault_diagnosis::FaultDiagnosisProvider);
    return true;
}
