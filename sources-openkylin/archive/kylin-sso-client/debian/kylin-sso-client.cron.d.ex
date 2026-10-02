#
# Regular cron jobs for the kylin-sso-client package
#
0 4	* * *	root	[ -x /usr/bin/kylin-sso-client_maintenance ] && /usr/bin/kylin-sso-client_maintenance
