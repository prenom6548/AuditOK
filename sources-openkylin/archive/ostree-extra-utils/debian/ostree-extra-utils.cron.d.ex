#
# Regular cron jobs for the ostree-extra-utils package
#
0 4	* * *	root	[ -x /usr/bin/ostree-extra-utils_maintenance ] && /usr/bin/ostree-extra-utils_maintenance
