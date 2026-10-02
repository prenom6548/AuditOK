#
# Regular cron jobs for the wemeet package
#
0 4	* * *	root	[ -x /usr/bin/wemeet_maintenance ] && /usr/bin/wemeet_maintenance
