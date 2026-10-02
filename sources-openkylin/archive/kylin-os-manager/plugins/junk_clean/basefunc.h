#ifndef BASEFUNC_H
#define BASEFUNC_H

#include<QHash>

namespace junk_clean {

enum class KernelState {
    kUnknow,
    kRunning,       // 运行中
    kCustomized,    // 定制
    kGeneric,       // 通用
};

class BaseFunc
{
public:
    static KernelState getKernelState(QString path);
    static void GetKernelEnv();
private:
    static QHash<QString, enum KernelState>  kernel_env_;
};
}


#endif // BASEFUNC_H
