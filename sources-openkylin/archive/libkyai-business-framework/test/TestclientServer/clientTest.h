#pragma once
#include <gio/gio.h>

#include <string>

#include "testeglue.h"

class ClientTest {
public:
    friend void callBack2(GObject* source_object, GAsyncResult* res, gpointer user_data);
    friend void callBack3Continue(GDBusConnection* conn, const gchar* sender_name, const gchar* object_path,
                                  const gchar* interface_name, const gchar* signal_name, GVariant* parameters,
                                  gpointer user_data);
    friend void callBack3Finesh(GObject* source_object, GAsyncResult* res, gpointer user_data);

    ClientTest() = default;
    ~ClientTest();

    void callMethod1();
    void callMethod2();
    void callMethod3();

    bool connect();
    void destroy();

private:
    GDBusConnection* connection_ = nullptr;
    AiBusinessTest* delegate_ = nullptr;
    const std::string objectPath_ = "/com/kylin/AiBusiness/test";
    const std::string interfaceName_ = "com.kylinos.aisdk.business.test";
};
