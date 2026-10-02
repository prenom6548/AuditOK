#
# Regular cron jobs for the liblol package
#
0 4	* * *	root	[ -x /usr/bin/liblol_maintenance ] && /usr/bin/liblol_maintenance
