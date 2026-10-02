# ukui-window-switch

ukui-window-switch is a MultitaskView runs in Kylin Operating System.

## Introduction

MultitaskView supports both PC and Pad.

 - PC

	MultitaskView provides a convenient and fast interactive interface, which makes it convenient to switch between virtual desktops and locate the window you want, so as to improve your production efficiency and working efficiency.

 - Pad

	MultitaskView is convenient to manage the opened windows, it also improves interactive experience through different animations. In Pad mode, you can choose to wake up or close any windows shown in MultitaskView.

## Features

Common features of multitaskView:

	1. Loaded as a plugin of kwin;  
	2. Run with compositor;

 - PC

	The following are the features of the MultitaskView in PC mode:
	1. Add the functions of creating, deleting, exchanging virtual desktop and managing window content in virtual desktop;
	2. The window is of equal height and slides up and down, and one line places the window with the greatest possibility;
	3. Support small window dragging to different virtual desktops;
	4. Dual screen mode adaptation;

 - Pad

	1. Display the contents of the window with equal height and width, and can only slide left and right;
	2. Add all clear function;
	3. Support sliding up, closing, arousing, closing, arousing and other dynamic effects;
	4. Adapt to different system modes and desktop states;

## Requirements 

Please use the version after 0430 for ukui panel and kwin.

## Installation

Obtain the package (ukui-window-switch) of MultitaskView from the warehouse source and execute "sudo apt install ukui-window-switch". 

Please ensure that the installed version meets the installation conditions of the new version!!!!

## Usage
[Open operation mode]
	1. Slide from the bottom of the screen to bring up the MultitaskView;
	2. Click the second button on the left side of the taskbar to evoke the MultitaskView;
	3. Win + Tab shortcut key evokes MultitaskView;
[Close operation mode]
	1. When the MultitaskView is open, click ESC to close the MultitaskView;
	2. When the MultitaskView is open, click the blank space of the MultitaskView to close the MultitaskView;
	3. When the MultitaskView is open, use the shortcut key "menu + Tab" to close the MultitaskView;
	4. In PC mode only and MultitaskView is on, click virtual desktop to close MultitaskView;
	5. When the MultitaskView is open, click the thumbnail window to close the MultitaskView;
	6. Passively trigger to close the MultitaskView. When a new window is activated, the MultitaskView will be closed automatically;

## Development

The MultitaskView is divided into two parts: the view module is developed with QML language and the backend is developed with C++ language.
The view module is divided into PC mode and Pad mode.

The development and commissioning shall be carried out according to the following steps:

1. cd ukui-window-switch
2. mkdir build
3. cd build
4. cmake ..
5. sudo make install
6. kwin --replace &

## Debuglog 

The log is stored in the log of kwin. The viewing method is as follows:

1. cd ~/.log (or cd .config/kwin/log)
2. tail -f kwin_0.log

## Changelog 

You can view the record of each change in the changelog in Debian directory.

## FAQ

1. The MultitaskView always shows no application window?
	Please ensure that the current version of kwin is installed correctly
2. Shortcut keys cannot evoke MultitaskView?
	rm -rf ~/.config/kglobalshortcutsrc
	reboot
3. Does the MultitaskView provide a DBUS interface and how to use it?
	Please use d-feet
	In the session, find "org.kde.kwin" -> "/MultitaskView" -> org.kde.kWin.MultitaskView->show()
	Please ensure that the installed versions of kwin and ukui panel support the new version of multitask


