#
# Regular cron jobs for the kydoctor package
#
0 4	* * *	root	[ -x /usr/bin/kydoctor_maintenance ] && /usr/bin/kydoctor_maintenance
