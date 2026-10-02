#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>

#include "mainwindow.h"

namespace application_example {

MainWindow::MainWindow(QWidget *parent)
	: QWidget(parent)
{
	QHBoxLayout *hLayout = new QHBoxLayout;

	QPushButton *pushButton = new QPushButton("Click me to test", this);
	hLayout->addStretch();
	hLayout->addWidget(pushButton);
	hLayout->addStretch();

	setLayout(hLayout);

	connect(pushButton, &QPushButton::clicked, this, [=]() {
		QMessageBox messageBox(QMessageBox::Information,
				       "kylin os manager plugin example",
				       "Welcome to use the kylin os manager plugin mechanism. The plugin function is normal, please rest assured to use it");
		messageBox.exec();
	});
}

MainWindow::~MainWindow() {
	/* nothing to do */
}

}
