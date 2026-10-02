#
# Regular cron jobs for the libkylin-kmre package
#
0 4	* * *	root	[ -x /usr/bin/libkylin-kmre_maintenance ] && /usr/bin/libkylin-kmre_maintenance
