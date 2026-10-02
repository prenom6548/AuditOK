#
# Regular cron jobs for the onnxruntime-backend package
#
0 4	* * *	root	[ -x /usr/bin/onnxruntime-backend_maintenance ] && /usr/bin/onnxruntime-backend_maintenance
