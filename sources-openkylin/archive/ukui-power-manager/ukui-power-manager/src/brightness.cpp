#include "brightness.h"
#include "error.h"

Brightness::Brightness(QObject *parent) : QObject(parent)
{
    qDebug() << "Brightness" << SUCCESS;
}

Brightness::~Brightness()
{

}
