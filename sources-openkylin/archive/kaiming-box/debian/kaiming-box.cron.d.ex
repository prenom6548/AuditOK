#
# Regular cron jobs for the kaiming-box package
#
0 4	* * *	root	[ -x /usr/bin/kaiming-box_maintenance ] && /usr/bin/kaiming-box_maintenance
