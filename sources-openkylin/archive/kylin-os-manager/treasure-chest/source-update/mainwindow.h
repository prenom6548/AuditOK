#ifndef KYLIN_OS_MANAGER_TREASURE_CHEST_SOURCE_UPDATE
#define KYLIN_OS_MANAGER_TREASURE_CHEST_SOURCE_UPDATE

#include <kwidget.h>
#include <QPushButton>
#include <QTimer>
#include "text_label.h"

namespace source_update {

class MainWindow: public kdk::KWidget {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);

private Q_SLOTS:
    void on_Exec();
    void on_UpdateSourceFinish(int exit_code);

private:
    // 弹气泡弹窗
    void BubblePopup(bool ok);

    K::TextLabel *desc_label_;
    QPushButton *exec_btn_;
};

}

#endif
