#include "protocol.h"

extern "C" bool kyconnect(kyplugin::Host &host) {
	/* 将工厂对象添加到插件中 */
	host.add(new application_example::ProtocolProvider());
	return true;
}
