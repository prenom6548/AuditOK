#
# Regular cron jobs for the kylin-burner package
#
0 4	* * *	root	[ -x /usr/bin/kylin-burner_maintenance ] && /usr/bin/kylin-burner_maintenance
