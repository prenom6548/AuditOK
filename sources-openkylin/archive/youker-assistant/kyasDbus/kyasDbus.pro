TEMPLATE = aux

inst1.files += systemdaemon/
inst1.files += sessiondaemon/
inst1.path = /usr/lib/python3/dist-packages/kylin-assistant-daemon/

inst2.files += systemdaemon/conf/com.kylin.assistant.systemdaemon.conf
inst2.path = /etc/dbus-1/system.d/
inst3.files += systemdaemon/conf/com.kylin.assistant.systemdaemon.service
inst3.path = /usr/share/dbus-1/system-services/
inst4.files += systemdaemon/conf/com.kylin.assistant.systemdaemon.policy
inst4.path = /usr/share/polkit-1/actions/
inst5.files += sessiondaemon/conf/com.kylin.assistant.sessiondaemon.service
inst5.path = /usr/share/dbus-1/services/

INSTALLS += inst1 \
            inst2 \
            inst3 \
            inst4 \
            inst5 \
