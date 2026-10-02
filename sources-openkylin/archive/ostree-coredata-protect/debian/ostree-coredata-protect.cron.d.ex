#
# Regular cron jobs for the ostree-coredata-protect package.
#
0 4	* * *	root	[ -x /usr/bin/ostree-coredata-protect_maintenance ] && /usr/bin/ostree-coredata-protect_maintenance
