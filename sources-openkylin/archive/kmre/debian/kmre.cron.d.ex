#
# Regular cron jobs for the kmre package
#
0 4	* * *	root	[ -x /usr/bin/kmre_maintenance ] && /usr/bin/kmre_maintenance
