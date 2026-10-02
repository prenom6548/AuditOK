#include <QApplication>
#include "custom_combo_box.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    CustomComboBox *b = new CustomComboBox();

    b->setDefaultPreItem("1");
    b->setDefaultPreItem("2", true);

    b->setDefaultAfterItem("3");
    // b->setDefaultAfterItem("4", true);
    b->setDefaultAfterItem("5");

    // b->addItem("aaa");

    // b->clear();

    b->addItem("aaa");

    b->show();

    app.exec();
}