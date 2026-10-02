wlcctrl(1)

# NAME

wlcctrl - A command line utility for kylin-wayland-compositor

# SYNOPSIS

wlcctrl [command] [options...]

# GLOBAL OPTIONS

*--help*
	Show a help message and quit.

*-v, --version*
	Show a wlcctrl version and quit.

*--exec* <str>
	Execute a shell command.

*--sleep* <int_ms>
	Set sleep time in milliseconds. Can be combined with other commands.

	example:
		wlcctrl --keydown a --sleep 1000 --keyup a
		wlcctrl --mousepress 1 --sleep 200 --mouserelease 1

*--path* <str>
	Set screenshot save path (PNG format). Uses last value when set multiple times.

	with *--windowcapture*:
		Set the window screenshot saved path.
	with *--workspacecapture*:
		Set the workspace screenshot saved path.
	with *--fullscreencapture*:
		Set the screen/output screenshot saved path.

	example: wlcctrl --windowcapture uuid --path "/home/kylin/xxx.png"

*-o* <o_uuid>
	Specify output device by UUID. Uses last value when set multiple times.

# KEYBOARD OPTIONS

*--keystring* <string>
	Send a string to be typed into the focused client. No commands can chain after 'keystring'.

*--key* <key>
	Send key events. Single key or key combination.

	Note: CapsLock(58), NumLock(69), ScrollLock(70) may not switch states normally.
	For reliable lock key functionality, use ydotool instead.

	Key mappings:
		"PrintScreen" -> "sys_req"
		"esc" -> "escape"
		"ctrl" -> "control_l"
		"shift" -> "shift_l"
		"alt" -> "alt_l"
		"winleft" -> "super_l"
		"enter" -> "return"

	Valid keys: escape, return, shift_l, shift_r, control_l, control_r,
	meta_l, meta_r, alt_l, alt_r, page_up, page_down, menu, caps_lock,
	pause, break, print, sys_req, scroll_lock, num_lock, etc.

	Multiple keys separated by '+'.

	examples:
		wlcctrl --key control_l+alt_l+t
		wlcctrl --key ctrl+alt+t
		wlcctrl --key super_l

*--keyup* <key>
	Release a single key.

*--keydown* <key>
	Press a single key.

	with *--keyup*:
		Indicate pressing and releasing a key.
	otherwise: Indicate the state of a key being pressed individually.
	The command defaults to releasing the key upon completion.

# POINTER OPTIONS

*--mousebutton* <button>
	Click mouse button.

	Button codes:
		1 = BTN_LEFT
		2 = BTN_RIGHT
		3 = BTN_MIDDLE
		4 = BTN_EXTRA
		5 = BTN_SIDE
		6 = BTN_FORWARD
		7 = BTN_BACK

	example:
		wlcctrl --mousebutton 2

*--mousepress* [button]
	Press a mouse button.

	Button codes:
		1 = BTN_LEFT
		2 = BTN_RIGHT
		3 = BTN_MIDDLE
		4 = BTN_EXTRA
		5 = BTN_SIDE
		6 = BTN_FORWARD
		7 = BTN_BACK

	with *--mouserelease*
		Indicate pressing and releasing a mouse button.
	width *--mousemove*
		Can be combined into a drag-and-drop action.
	otherwise: Indicate the state of an individual mouse button being pressed.
	The command defaults to releasing the mouse button upon completion.

	example: Click a mouse button.
		wlcctrl --mousepress 2 --mouserelease 2
	example: Complete a drag-and-drop action.
		./wlcctrl --mousemove 500,600 --mousepress 1 --mousemove 100,200 --mouserelease 1

*--mouserelease* [button]
	Release a mouse button.

	Button codes: BTN_LEFT->1 | BTN_RIGHT->2 | BTN_MIDDLE->3 | BTN_EXTRA->4 |
	BTN_SIDE->5 | BTN_FORWARD->6 | BTN_BACK->7

*--mousemove* <dx,dy>
	Move cursor. dx = right displacement, dy = down displacement.
	Only non-negative numbers allowed. Units: pixels.

	example:
		wlcctrl --mousemove 100,500

*--scroll* [*-x* <dx>] [*-y* <dy>]
	Scroll cursor. Negative numbers allowed.

	examples:
		wlcctrl --scroll -y 10
		wlcctrl --scroll -x 10 -y 20

*--getmouselocation*
	Get the current mouse position.

# TOPLEVEL OPTIONS

*-l, --list*
	List the names of all known toplevel.

*--windowmove* <w_uuid> [options...]
	Move the window to a specified position.

	*-x* <dx>
		_dx_ is the displacement in positive-right direction.
	*-y* <dy>
		_dy_ is the displacement in the positive-downward direction.
	Only non-negative numbers are allowed.

	example: wlcctrl --windowmove 182fcd8f-6136-413b-aa1b-c77b066ebe23 -x 200 -y 400

*--windowsize* <w_uuid> [options...]
	Resize the window.
	_w_uuid_: This is the UUID of the toplevel device.

	*-w* <width>
		_width_ is the desired window width.
	*-h* <height>
		_height_ is the desired window height.
	Units are pixels.

	example: wlcctrl --windowsize 182fcd8f-6136-413b-aa1b-c77b066ebe23 -w 800 -h 600

*--windowminimize* <w_uuid>
	Minimize window.
	_w_uuid_ is the UUID of the toplevel.

*--windowmaximize* <w_uuid>
	Maximize window.
	The default output device is the primary display.
	_w_uuid_ is the UUID of the toplevel.

	with *-o*
		Set the output device for the action.
	example: wlcctrl --windowmaximize <w_uuid> -o <o_uuid>

*--windowfullscreen* <w_uuid>
	Fullscreen window.
	The default output device is the primary display.
	_w_uuid_ is the UUID of the toplevel.

	with *-o*
		Set the output device for the action.
	example: wlcctrl --windowfullscreen <w_uuid> -o <o_uuid>

*--windowenter* <w_uuid:uuid>
	Add the window into the specified workspace.
	_w_uuid_ is the UUID of the toplevel.
	_uuid_ is the workspace that window to be entered.

*--windowleave* <w_uuid:uuid>
	Remove the window from the specified workspace.
	_w_uuid_ is the UUID of the toplevel.
	_uuid_ is the workspace that window to be left.

*--movewindowtoworkspace* <w_uuid:uuid>
	Move the window to the specified workspace.
	_w_uuid_ is the UUID of the toplevel.
	_uuid_ is the workspace that window moved to.

*--movewindowtooutput* <w_uuid:o_uuid>
	Move the window to the specified output device.
	_w_uuid_ is the UUID of the toplevel.
	_o_uuid_ is the UUID of the output device.

*--windowmap* <w_uuid>
	Restore window.
	_w_uuid_ is the UUID of the toplevel.

*--windowclose* <w_uuid>
	Close window.
	_w_uuid_ is the UUID of the toplevel.

*--windowkill* <app_id>
	Force close all windows of the specified app_id.

*--windowactivate* <w_uuid>
	Activate the specified window.
	_w_uuid_ is the UUID of the toplevel.

*--windowcapture* <w_uuid>
	Take a screenshot of a window. Saved by default in the current working directory.
	_w_uuid_ is the UUID of the toplevel.

*--getactivewindow*
	Get the current active window.

*--getwindowname* <w_uuid>
	Query the app ID, title, icon and pid of a specific window.
	_w_uuid_ is the UUID of the toplevel.

*--getwindowgeometry* <w_uuid>
	Query the geometry of a specific window.
	_w_uuid_ is the UUID of the toplevel.

*--search* <name>
	Obtain the window UUID list by checking if the regular expression pattern matches the window app_id, icon and title.

# OUTPUT OPTIONS

*--outputs*
	List the name of all known outputs.

*--getdisplaygeometry* <o_uuid>
	Get the geometry of a specified output.
	_o_uuid_ is the UUID of the output device.

*--fullscreencapture*
	Take a fullscreen screenshot. Saved the file to the current working directory by default.
	By default, capture a fullscreen image of the primary display.

	example: wlcctrl --fullscreencapture

	with *-o*
		Set the output device for the action.
	example: wlcctrl --fullscreencapture -o <o_uuid>

*--setarea* <x,y,width,height>
	Area screenshot. Set the position and size of the rectangular region.
	The default output device is the primary display.

	_x_ is X coordinate of the screenshot area in output coordinate.
	_y_ is Y coordinate of the screenshot area in output coordinate.
	_width_ is Width of the screenshot area.
	_height_ is Height of the screenshot area.

	with *-o*
		Set the output device for the area.
	with *--path*
		Set the screenshot save path.
	example: wlcctrl --setarea 10,40,500,100 --path "/home/kylin/xxx.png"
		wlcctrl --setarea 10,40,500,500 --path "/home/kylin/xxx111.png" -o d4d40b7b-5e2e-396a-eaa2-7b5efea5a3ec

*--getpixelcolor* <x,y>
	Get the RGBA value of a pixel at a specified position.
	The default output device is the primary display.
	_x_ is X coordinate of the point in output coordinate.
	_y_ is Y coordinate of the point in output coordinate.

	with *-o*
		Set the output device for the pixel location.
	example: wlcctrl --getpixelcolor 100,3

# WORKSPACE OPTIONS

*--workspace*
	List the name of all known workspaces.

*--workspacecapture* <uuid:o_uuid>
	Take a screenshot of a workspace. Saved the file to the current working directory by default.
	_uuid_ is the workspace that needs to be captured.
	_o_uuid_ is the UUID of the output device.

*--workspaceadd* <num>
	Add new workspaces. If _num_ is unspecified, a workspace will be added by default.

*--workspaceremove* <uuid>
	Remove the specified workspace.
	_uuid_: The workspace that needs to be operated on.

*--workspaceactive* <uuid>
	Activate the specified workspace.
	_uuid_ is the workspace that needs to be operated on.
