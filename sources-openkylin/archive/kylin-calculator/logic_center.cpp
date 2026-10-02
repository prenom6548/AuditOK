#include "logic_center.h"

LogicCenter::LogicCenter()
{
    this->init();
    this->establishInterrupt();
}

LogicCenter::~LogicCenter() {}

LogicCenter *LogicCenter::getInstance(void)
{
    static LogicCenter *ptr = nullptr;

    if (ptr == nullptr) {
        ptr = new LogicCenter();
        ptr->setObjectName("kylin-calculator_LogicCenter_ptr");
    }

    return ptr;
}

void LogicCenter::init(void)
{
    /* 注意 : 数据仓库需在界面前进行实例 , 界面实例时会访问数据仓库中的数据 */
    DataWarehouse::getInstance();

    /* 走 main 函数 , 说明为正常启动 */
    DataWarehouse::getInstance()->intelPlug = DataWarehouse::StartMode::MODE_MAIN;

    /* 实例主界面 */
    this->m_mainWin = MainWindow::getInstance();

    return;
}

void LogicCenter::establishInterrupt(void)
{
    connect(this->m_mainWin, &MainWindow::sigTranparencyChange, this, &LogicCenter::slotTranparencyChange);

    return;
}

void LogicCenter::slotTranparencyChange(void)
{
    MainWindow::getInstance()->update();

    return;
}
