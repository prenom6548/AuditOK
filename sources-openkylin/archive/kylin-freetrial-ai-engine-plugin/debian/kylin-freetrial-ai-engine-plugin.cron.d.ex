#
# Regular cron jobs for the kylin-freetrial-ai-engine-plugin package
#
0 4	* * *	root	[ -x /usr/bin/kylin-freetrial-ai-engine-plugin_maintenance ] && /usr/bin/kylin-freetrial-ai-engine-plugin_maintenance
