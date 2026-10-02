#ifndef MESSAGEBOXUTILS_H
#define MESSAGEBOXUTILS_H

#include <QWidget>

enum class MessageBoxResult {
    Yes,    // 用户选择“是”
    No,     // 用户选择“否”
    Cancel  // 用户选择“取消”
};
class MessageBoxUtils
{

public:
    static void QMESSAGE_BOX_INFORMATION(QWidget* q_parent, const QString& typelabel, const QString& message, const QString& label);
    static void QMESSAGE_BOX_WARNING(QWidget* q_parent, const QString& typelabel, const QString& message, const QString& label);
    static void QMESSAGE_BOX_CRITICAL(QWidget* q_parent, const QString& typelabel, const QString& message, const QString& label);
    static bool QMESSAGE_BOX_WARNING_CANCEL(QWidget* q_parent, const QString& typelabel, const QString& message,
                                            const QString& label_yes, const QString& label_no);
    static MessageBoxResult QMESSAGE_BOX_WARNING_CANCEL2(QWidget* q_parent, const QString& typelabel, const QString& message,
                                            const QString& label_yes, const QString& label_no, const QString &label_cancel);
};

#endif // MESSAGEBOXUTILS_H
