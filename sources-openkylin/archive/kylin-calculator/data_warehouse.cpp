#include "data_warehouse.h"

#include <libkysysinfo.h>

#include <QDebug>

DataWarehouse::DataWarehouse()
{
    init();
}

DataWarehouse::~DataWarehouse() {}

DataWarehouse *DataWarehouse::getInstance(void)
{
    static DataWarehouse *instance = NULL;
    if (instance == NULL) {
        instance = new DataWarehouse();
        return instance;
    }

    return instance;
}

void DataWarehouse::init(void)
{
    this->platform.clear();
    this->getPlatForm();

    this->intelPlug = StartMode::MODE_PLUG;
    this->intelMode = false;
    this->maxInputNum = 35;

    return;
}

void DataWarehouse::getPlatForm(void)
{
    char *projectName = kdk_system_get_projectName();
    if (projectName == NULL) {
        return;
    }

    if (QString(projectName) == QString("V10SP1-Intel") || QString(projectName) == QString("V10SP1-edu")) {
        this->platform = QString("intel");
    }

    free(projectName);

    qDebug() << "Info : check run platform is " << this->platform;

    return;
}
