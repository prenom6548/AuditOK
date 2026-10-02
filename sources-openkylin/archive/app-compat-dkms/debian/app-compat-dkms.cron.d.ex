#
# Regular cron jobs for the app-compat-dkms package
#
0 4	* * *	root	[ -x /usr/bin/app-compat-dkms_maintenance ] && /usr/bin/app-compat-dkms_maintenance
