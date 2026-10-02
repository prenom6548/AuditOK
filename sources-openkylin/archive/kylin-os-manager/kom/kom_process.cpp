#include "kom_process.h"
#include <QPointer>
#include <QDebug>
#include <QMutex>
#include <QMutexLocker>
#include "libkypackages.h"

namespace kom {

static QPointer<KomProcess> g_instance;    // 类的实例指针
static kdk_cmd_node *g_kdk_cmd_node;       // sdk 表示命令的结构体指针

// 存储标准输出
static QByteArray g_standard_out;
static QMutex g_standard_output_mutex;

// 存储标准输入
static QByteArray g_standard_err;
static QMutex g_standard_err_mutex;

static void update_callback(const char *output) {
    if (g_instance.isNull()) {
        return;
    }

    QByteArray op(output);
    QMutexLocker locker(&g_standard_output_mutex);
    g_standard_out.append(op);
    // 测试过程中发现，sdk 调用该回调函数一整行一整行的传递命令的输出，并且移除了行末尾的换行符
    // 接口调用者有时需要根据换行符分割来逐行分析输出，所以此处追加换行符，保持命令的原样输出
    g_standard_out.append('\n');

    qInfo() << "[KomProcess] Received standard output: " << QString(op).toUtf8();

    Q_EMIT g_instance->readyReadStandardOutput();
}

static void finished_callback(int state, const char *standard_output, const char *standard_err) {
    if (g_instance.isNull()) {
        return;
    }

    if (state) {
        // 出错
        QByteArray op(standard_err);
        if (!op.isEmpty()) {
            QMutexLocker locker(&g_standard_err_mutex);
            g_standard_err.append(op);
            qInfo() << "[KomProcess] Command ended received standard error: " << QString(op).toUtf8();
            Q_EMIT g_instance->readyReadStandardError();
        }

        Q_EMIT g_instance->finished(state, QProcess::ExitStatus::NormalExit);
    } else {
        // 正常退出
        QByteArray op(standard_output);
        if (!op.isEmpty()) {
            QMutexLocker locker(&g_standard_output_mutex);
            g_standard_out.append(op);
            qInfo() << "[KomProcess] Command ended received standard output: " << QString(op).toUtf8();
            Q_EMIT g_instance->readyReadStandardOutput();
        }

        Q_EMIT g_instance->finished(state, QProcess::ExitStatus::NormalExit);
    }

    qInfo() << "[KomProcess] Command ended";

    if (g_kdk_cmd_node) {
        kdk_package_free_cmd(g_kdk_cmd_node);
        g_kdk_cmd_node = NULL;
    }
}

KomProcess::KomProcess(QObject *parent)
    : QObject(parent)
{
    g_kdk_cmd_node = NULL;

    qRegisterMetaType<QProcess::ExitStatus>();
}

void KomProcess::start(const QString &program, const QStringList &arguments) {
    g_instance = this;

    QString command(program);
    for (const auto &arg: arguments) {
        command.append(" ");
        command.append(arg);
    }
    command = command.trimmed();

    qInfo() << "[KomProcess] Start running command: " << command;

    auto cmd = command.toLocal8Bit();
    g_kdk_cmd_node = kdk_package_cmd_init(cmd.data());
    g_kdk_cmd_node->update = update_callback;
    g_kdk_cmd_node->finished = finished_callback;

    if (kdk_package_launch_cmd(g_kdk_cmd_node)) {
        kdk_package_free_cmd(g_kdk_cmd_node);
        g_kdk_cmd_node = NULL;

        qCritical() << "[KomProcess] Command execution failed";

        Q_EMIT errorOccurred(QProcess::ProcessError::FailedToStart);
        return;
    }

    Q_EMIT started();
}

void KomProcess::close() {
    if (g_kdk_cmd_node == NULL) {
        return;
    }

    kdk_package_cmd_close(g_kdk_cmd_node);
    kdk_package_free_cmd(g_kdk_cmd_node);
    g_kdk_cmd_node = NULL;
}

QByteArray KomProcess::readAllStandardOutput() {
    QMutexLocker locker(&g_standard_output_mutex);

    QByteArray standard_output;
    standard_output.swap(g_standard_out);

    return standard_output;
}

QByteArray KomProcess::readAllStandardError() {
    QMutexLocker locker(&g_standard_err_mutex);

    QByteArray standard_err;
    standard_err.swap(g_standard_err);

    return standard_err;
}

}
