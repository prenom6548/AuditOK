#ifndef KYLIN_OS_MANAGER_PLUGINS_JUNK_CLEAN_CLEAN_UP_GROUP_WIDGET_H
#define KYLIN_OS_MANAGER_PLUGINS_JUNK_CLEAN_CLEAN_UP_GROUP_WIDGET_H

#include <QWidget>
#include <ktoolbutton.h>
#include "defines.h"
#include "icon_label.h"
#include "text_label.h"

namespace junk_clean {

class CleanUpGroupWidget: public QWidget {
    Q_OBJECT
public:
    CleanUpGroupWidget(enum Type type, QWidget *parent = nullptr);
    void SetDescribe(const QString &describe);
    void SetDescribe(int total_items, quint64 total_size, quint64 select_size);
    void SetExpand(bool is_expand);
    enum Type Type() const;

Q_SIGNALS:
    void sig_Expand(bool is_expand);

private Q_SLOTS:
    void on_ExpandOrCloseBtnClicked();

private:
    QString TypeToString(enum Type type) const;
    QString TypeIcon(enum Type type) const;

    enum junk_clean::Type type_;
    K::IconLabel *icon_label_;
    K::TextLabel *category_label_;
    K::TextLabel *describe_label_;
    kdk::KToolButton *expand_or_close_btn_;
    bool is_expand_;
};

}

#endif
