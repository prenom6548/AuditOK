#ifndef KYLIN_OS_MANAGER_PLUGINS_PLUGIN_TEMPLATE_MAINWINDOW_H
#define KYLIN_OS_MANAGER_PLUGINS_PLUGIN_TEMPLATE_MAINWINDOW_H

#include <QWidget>

namespace application_example {

class MainWindow: public QWidget {
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();
};

}

#endif
