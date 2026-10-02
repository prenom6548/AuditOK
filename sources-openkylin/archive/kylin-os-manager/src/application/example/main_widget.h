#ifndef KYLIN_OS_MANAGER_SRC_APPLICATION_EXAMPLE_MAIN_WIDGET_H
#define KYLIN_OS_MANAGER_SRC_APPLICATION_EXAMPLE_MAIN_WIDGET_H

#include <QWidget>

namespace application_example {

class MainWindow : public QWidget {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
};

}

#endif
