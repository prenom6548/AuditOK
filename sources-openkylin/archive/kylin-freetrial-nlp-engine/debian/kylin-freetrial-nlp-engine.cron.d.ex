#
# Regular cron jobs for the kylin-freetrial-nlp-engine package
#
0 4	* * *	root	[ -x /usr/bin/kylin-freetrial-nlp-engine_maintenance ] && /usr/bin/kylin-freetrial-nlp-engine_maintenance
