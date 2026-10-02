#ifndef __DATA_WAREHOUSE_H__
#define __DATA_WAREHOUSE_H__

#include <QString>

class DataWarehouse
{
public:
    static DataWarehouse* getInstance(void);
    ~DataWarehouse();

    enum StartMode {
        MODE_MAIN = 0, // 程序从正常入口启动
        MODE_PLUG = 1, // 程序从小插件启动
        MODE_PLUG_AGAIN,
    };

    /* 运行平台 */
    /* 990 , 9a0 , v100 , v101 , inter */
    QString platform;

    /* 启动是否为 intel 小插件方式 , 该变量标识两种状态 ，0 ---> 正常方式启动 , 1 ---> 小插件启动 */
    StartMode intelPlug;

    /* intel 模式标识 , true ---> 平板 , false ---> 正常模式 */
    bool intelMode;

    /* 输入最大限度 */
    int maxInputNum;

    /* 透明度 */
    double transparency;

protected:

private:
    DataWarehouse();

    void getPlatForm(void);
    void init(void);
};

#endif
