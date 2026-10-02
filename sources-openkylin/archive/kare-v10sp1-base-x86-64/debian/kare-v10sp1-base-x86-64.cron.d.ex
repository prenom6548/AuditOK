#
# Regular cron jobs for the kare-v10sp1-base-x86-64 package
#
0 4	* * *	root	[ -x /usr/bin/kare-v10sp1-base-x86-64_maintenance ] && /usr/bin/kare-v10sp1-base-x86-64_maintenance
