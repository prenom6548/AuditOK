kylin-wlcom-wrapper(1)

# NAME

kylin-wlcom-wrapper - Wrapper to manage kylin-wlcom.target lifecycle in user session

# SYNOPSIS

*kylin-wlcom-wrapper*

# DESCRIPTION

*kylin-wlcom-wrapper* is a systemd user session manager designed for Kylin OS Wayland compositor sessions.
It cleans up stale environment variables (such as DISPLAY, WAYLAND_DISPLAY, and XDG_\*), starts the _kylin-wlcom.target_ unit,
and monitors its state to ensure clean session startup and reliable exit.

This program is typically started by a display manager (e.g., LightDM) at user login.

# OPTIONS

None. The program takes no command-line arguments.

# LOGGING

All messages are sent to the systemd journal. You can use the following command to view the logs:

	journalctl --user -t kylin-wlcom-wrapper

# EXAMPLE

In a display manager's session script:

	exec /usr/bin/kylin-wlcom-wrapper

# FILES

*/usr/lib/systemd/user/kylin-wlcom.target*  
The systemd user target unit managed by this wrapper.

# SEE ALSO

*systemd*(1), *systemctl*(1)

# AUTHORS

Kylin OS Development Team