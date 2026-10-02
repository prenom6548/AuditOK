#
# Regular cron jobs for the dpkg-wrapper package
#
0 4	* * *	root	[ -x /usr/bin/dpkg-wrapper_maintenance ] && /usr/bin/dpkg-wrapper_maintenance
