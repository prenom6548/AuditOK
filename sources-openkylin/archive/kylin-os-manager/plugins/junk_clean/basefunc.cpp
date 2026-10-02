#include "basefunc.h"
#include <QProcess>
#include <QString>
#include <QFileInfo>
#include <QDebug>

QHash<QString, enum junk_clean::KernelState>  junk_clean::BaseFunc::kernel_env_;

junk_clean::KernelState junk_clean::BaseFunc::getKernelState(QString path)
{
    KernelState kernel_state{KernelState::kUnknow};

    // 获取垃圾项的内核版本
    QString kernel_version;
    const QFileInfo file_info(path);
    const auto file_name = file_info.fileName();
    if (const auto index = file_name.indexOf('-'); index != -1) {
        kernel_version = file_name.mid(index + 1);
    }

    const auto iter = kernel_env_.find(kernel_version);
    if (iter != kernel_env_.end()) {
        kernel_state = iter.value();
    } else {
        qCritical() << "Junk clean boot partition junk kernel state not obtained: " << path;
    }
    return kernel_state;
}

void junk_clean::BaseFunc::GetKernelEnv()
{
    // 获取已安装的内核包的状态
    QProcess process1;
    process1.setProgram("bash");
    process1.setArguments({"-c", "dpkg -l | grep linux-image"});
    process1.start();
    process1.waitForFinished();

    const auto output = QString::fromUtf8(process1.readAllStandardOutput()).trimmed();
    const auto lines = output.split('\n');
    for (const auto &line: lines) {
        auto elements = line.split(' ');
        elements.removeAll("");
        if (elements.size() < 3) {
            continue;
        }

        QString kernel_version;
        // 从包名中获取内核版本
        const auto package_name = elements.at(1);
        const int first_dash = package_name.indexOf('-');
        const int second_dash = package_name.indexOf('-', first_dash + 1);
        if (second_dash != -1) {
            kernel_version = package_name.mid(second_dash + 1);
        }

        enum KernelState kernel_state;
        // 包的版本
        const auto package_version = elements.at(2);
        auto version_elements = package_version.split('-');
        version_elements.removeAll("");
        if (version_elements.size() < 2) {
            continue;
        }
        // 内部版本号
        const auto internal_version = version_elements.at(1);
        auto internal_version_elements = internal_version.split('.');
        internal_version_elements.removeAll("");
        if (internal_version_elements.size() < 2) {
            continue;
        }
        // 编译版本
        bool have_letter{false};
        const auto compiled_version = internal_version_elements.at(1);
        for (const auto &c: compiled_version) {
            if (c.isLetter()) {
                have_letter = true;
                break;
            }
        }
        if (have_letter) {
            kernel_state = KernelState::kCustomized;
        } else {
            kernel_state = KernelState::kGeneric;
        }

        kernel_env_.insert(kernel_version, kernel_state);
    }

    // 获取正在运行的内核版本
    QProcess process2;
    process2.setProgram("uname");
    process2.setArguments({"-r"});
    process2.start();
    process2.waitForFinished();
    const auto running_kernel_version = QString::fromUtf8(process2.readAllStandardOutput()).trimmed();
    // 如果 key 已经存在，则覆盖掉原有的 value，以此处的值为准
    kernel_env_.insert(running_kernel_version, KernelState::kRunning);
}
