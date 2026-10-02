#
# Regular cron jobs for the openkylin-token-center package
#
0 4	* * *	root	[ -x /usr/bin/openkylin-token-center_maintenance ] && /usr/bin/openkylin-token-center_maintenance
