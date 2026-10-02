#ifndef KYLIN_OS_MANAGER_PLUGINS_JUNK_CLEAN_CLEAN_UP_ENTRY_WIDGET_H
#define KYLIN_OS_MANAGER_PLUGINS_JUNK_CLEAN_CLEAN_UP_ENTRY_WIDGET_H

#include <QWidget>
#include <QCheckBox>
#include <QVBoxLayout>
#include <ktoolbutton.h>
#include "defines.h"
#include "text_label.h"

namespace junk_clean {

class CleanUpEntryWidget: public QWidget {
    Q_OBJECT
public:
    enum class SpecialSize {
        kScanning,
        kVeryClean,
    };

    CleanUpEntryWidget(const QString &mark, enum Type type, enum Level level, QWidget *parent = nullptr);
    void SetName(const QString &name);
    void SetDescribe(const QString &describe);
    void SetSize(quint64 size);
    void SetSize(enum SpecialSize special_size);
    void ShowCheckBox(bool show) const;
    void ShowExpandBtn(bool show);
    void SetCheckState(Qt::CheckState check_state);
    Qt::CheckState CheckState() const;
    void SetCheckable(bool checkable);
    QString Mark() const;
    enum Type Type() const;
    enum Level Level() const;
    quint64 Size() const;
    void Reset();

Q_SIGNALS:
    void sig_Expand(bool is_expand);
    void sig_CheckBoxStateChanged();

private Q_SLOTS:
    void on_ExpandOrCloseBtnClicked();

private:
    QString mark_;
    enum Type type_;
    enum Level level_;
    QCheckBox *select_checkbox_;
    QLabel *select_checkbox_seat_label_;
    QVBoxLayout *select_checkbox_seat_vbox_layout_;
    K::TextLabel *name_label_;
    K::TextLabel *describe_label_;
    K::TextLabel *state_label_;
    kdk::KToolButton *expand_or_close_btn_;
    QLabel *expand_or_close_seat_label_;
    QVBoxLayout *expand_or_close_seat_vbox_layout_;
    bool is_expand_;
    quint64 size_;
};

}

#endif
