#
# Regular cron jobs for the kylin-mfa package
#
0 4	* * *	root	[ -x /usr/bin/kylin-mfa_maintenance ] && /usr/bin/kylin-mfa_maintenance
