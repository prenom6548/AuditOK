#
# Regular cron jobs for the kylin-kmre-modules-dkms package
#
0 4	* * *	root	[ -x /usr/bin/kylin-kmre-modules-dkms_maintenance ] && /usr/bin/kylin-kmre-modules-dkms_maintenance
