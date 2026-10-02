#
# Regular cron jobs for the kylincameralibs package
#
0 4	* * *	root	[ -x /usr/bin/kylincameralibs_maintenance ] && /usr/bin/kylincameralibs_maintenance
