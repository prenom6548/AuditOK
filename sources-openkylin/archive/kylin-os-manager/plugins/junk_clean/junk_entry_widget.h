#ifndef KYLIN_OS_MANAGER_PLUGINS_JUNK_CLEAN_JUNK_ENTRY_WIDGET_H
#define KYLIN_OS_MANAGER_PLUGINS_JUNK_CLEAN_JUNK_ENTRY_WIDGET_H

#include <QWidget>
#include <QCheckBox>
#include "defines.h"
#include "text_label.h"

namespace junk_clean {

class JunkEntryWidget: public QWidget {
    Q_OBJECT
public:
    JunkEntryWidget(quint64 mark, enum Type type, const QString &cleaner_mark, QWidget *parent = nullptr);
    Qt::CheckState CheckState() const;
    void SetCheckState(Qt::CheckState check_state) const;
    void SetContent(const QString &content);
    void SetSize(quint64 size);
    void SetDisable(bool ok);
    bool AllowSelect() const;
    quint64 Size() const;
    quint64 Mark() const;
    QString Content() const;
    quint64 FileCount() const;

Q_SIGNALS:
    void sig_CheckBoxStateChanged();

private:
    quint64 CalculateFileCount(const QString &path) const;

    QCheckBox    *select_checkbox_;
    K::TextLabel *path_label_;
    K::TextLabel *size_label_;

    quint64       mark_;
    enum Type     type_;
    QString       cleaner_mark_;
    quint64       size_;
    quint64       file_count_;    // 记录垃圾项包含的文件个数，主要针对垃圾项是目录的情况
};

}

#endif
