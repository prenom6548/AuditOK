#include "mainwindow.h"
#include "protocol.h"

namespace application_example {

Protocol::~Protocol() {
	/* 手动申请的一些资源需要在此处释放 */
}

std::string Protocol::name() {
    return "ApplicationExample";
}

std::string Protocol::i18nName() {
    return "应用插件示例";
}

QWidget *Protocol::createWidget() {
    MainWindow *w = new MainWindow;
    return w;
}

std::string Protocol::icon() {
	return "ukui-full-backup-symbolic";
}

int Protocol::sort() {
	/*
	 * 给个大值，以便在最后一个位置显示
	 * 此处的顺序不能与已有的顺序重复，否则会覆盖当前顺序的插件
	 * 例如：现在麒麟管家有 4 个标签页，则顺序应从 5 开始
	 */
	return 999;
}

KomApplicationInterface *ProtocolProvider::create() const {
	/* 相当于是个工厂方法，实例化上边的类, new 的 Protocol 实例由麒麟管家负责释放，届时会触发上述的 ~Protocol 函数 */
	return new Protocol();
}

}
