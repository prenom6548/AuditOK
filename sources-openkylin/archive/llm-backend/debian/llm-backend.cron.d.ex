#
# Regular cron jobs for the llm-backend package
#
0 4	* * *	root	[ -x /usr/bin/llm-backend_maintenance ] && /usr/bin/llm-backend_maintenance
