#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <dbus-1.0/dbus/dbus.h>
#include <stdlib.h>
#include <sys/types.h>

#include "notifications.h"

int main(void)
{
	DBusConnection *connection;
	DBusError error;
	DBusMessage *message;
	DBusMessageIter iter;
	char *sigvalue;
	
	dbus_error_init(&error);

	/*connect to the deamon bus*/
	connection=dbus_bus_get(DBUS_BUS_SYSTEM, &error);
	if(!connection)
	{
	fprintf(stderr, "Failed to get a system DBus connection: %s\n", error.message);
	exit(1);
	}

	/*add a rule for which message we want to see*/
	dbus_bus_add_match(connection,
						"type='signal',interface='com.kylin.systemupgrade.interface'",
						&error);//,member='RebootLogoutRequired'
	dbus_connection_flush(connection);//阻塞
	if (dbus_error_is_set(&error))
	{
	fprintf(stderr, "Match Error (%s)\n", error.message);
	exit(1);
	}

	/*loop listening for signals being emmitted*/
	while(true)
	{
		//noblocking read of the next available message
		dbus_connection_read_write(connection, 0);
		message= dbus_connection_pop_message(connection);//等待消息
		
		//没接收到等一秒钟继续
		if(NULL == message)
		{
			sleep(1);	
			continue;
		}

		//check if the message is the message we want to get
		if(dbus_message_is_signal(message,"com.kylin.systemupgrade.interface", "RebootLogoutRequired"))
		{
		printf("notifications is ought to be send\n");
			if(!dbus_message_iter_init(message,&iter))
				fprintf(stderr,"MessageHas no Param");
			else if(dbus_message_iter_get_arg_type(&iter)!= DBUS_TYPE_STRING)
            	fprintf(stderr,"Param is not string");
			else
			{
				/*try to get the content of signal*/
                dbus_message_iter_get_basic(&iter,&sigvalue);
            	printf("Recetive Singal Value : %s\n",sigvalue);
				notifications(sigvalue);
				//此处添加notifications函数		
			}
		}//注意此处是持续监听的，可以让沈亚峰进行每三十分钟发送一次信号的操作?
	}
	//free the message
	dbus_message_unref(message);
	return 0;
}


