#
# Regular cron jobs for the kmre-apk-installer package
#
0 4	* * *	root	[ -x /usr/bin/kmre-apk-installer_maintenance ] && /usr/bin/kmre-apk-installer_maintenance
