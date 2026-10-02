#
# Regular cron jobs for the peony-vfs-kylin-kmre package
#
0 4	* * *	root	[ -x /usr/bin/peony-vfs-kylin-kmre_maintenance ] && /usr/bin/peony-vfs-kylin-kmre_maintenance
