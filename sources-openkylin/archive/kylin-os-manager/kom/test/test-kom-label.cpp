#include <QApplication>
#include <QColor>
#include <QLabel>

#include "kom-label.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	kom::KomLabel *label = new kom::KomLabel;
	label->setFixedWidth(280);
	label->setSecondNumColor(QColor("#F3222D"));
	label->setThirdNumColor(QColor("#ffaa00"));
	label->setText("一共检测了 123 项，其中 9 项异常，13 项警告");
	label->show();

	app.exec();
}
