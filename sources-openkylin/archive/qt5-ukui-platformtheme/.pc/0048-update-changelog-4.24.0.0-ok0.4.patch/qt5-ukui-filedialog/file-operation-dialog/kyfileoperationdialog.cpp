#include "kyfileoperationdialog.h"
using namespace UKUIFileDialog;

KyFileOperationDialog::KyFileOperationDialog(QWidget *parent) : kdk::KDialog(parent)
{
    setWindowIcon(nullptr);
    minimumButton()->setVisible(false);
    closeButton()->setVisible(true);
//    setFixedWidth(536);
//    setFixedHeight(192);
}

