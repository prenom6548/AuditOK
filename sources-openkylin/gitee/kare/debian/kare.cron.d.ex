#
# Regular cron jobs for the kare package
#
0 4	* * *	root	[ -x /usr/bin/kare_maintenance ] && /usr/bin/kare_maintenance
