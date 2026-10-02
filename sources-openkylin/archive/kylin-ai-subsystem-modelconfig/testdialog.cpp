#include "testdialog.h"
#include "aisubsystem.h"
#include <QWidget>
TestDialog::TestDialog(QWidget *parent)
  : QDialog(parent)
{
    this->setFixedSize(800,600);
    QHBoxLayout *layout = new QHBoxLayout;
    TestAiSubSystem *test = new TestAiSubSystem(this);
    layout->addWidget(test->createModelConfigWidget());
    this->setLayout(layout);
}
