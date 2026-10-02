#include "custombuttonstyle.h"
#include <QVariant>

CustomButtonStyle::CustomButtonStyle(QWidget *parent) : QPushButton(parent)
{
    QColor defaultColor(Qt::white);
    setProperty("setTextDefaultColor" , QVariant(defaultColor));
}
