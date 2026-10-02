#
# Regular cron jobs for the kylin-ai-cloud-model package
#
0 4	* * *	root	[ -x /usr/bin/kylin-ai-cloud-model_maintenance ] && /usr/bin/kylin-ai-cloud-model_maintenance
