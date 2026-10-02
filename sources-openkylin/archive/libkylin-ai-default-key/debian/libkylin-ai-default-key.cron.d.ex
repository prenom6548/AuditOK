#
# Regular cron jobs for the libkylin-ai-default-key package
#
0 4	* * *	root	[ -x /usr/bin/libkylin-ai-default-key_maintenance ] && /usr/bin/libkylin-ai-default-key_maintenance
