# Peony 
## Overview
The peony can view files and folders on the system categorically, supporting common operations for files and folders, and its main interface is shown in figure below.

![Fig 1 Peony-big](image/1.png)

## Basic Function
### File Name

(1)The maximum length of the system file name can be 255 characters, usually by letters, numbers, "."(dotted), "_"(underlined) and "-"(minus).

(2)"-"cannot be the first letter of a file name.

(3)File names cannot contain the "/"symbol; because "/"is in the operating system directory tree and indicates a separator symbol in the root directory or path.

### Path
(1)When using a file in the current directory, users can quote the file name directly; if users want to use a file in another directory, users must specify the directory where the file is located.

(2)The absolute path is certain and the relative path changes with the user's working directory.

**Absolute path** —— A path from the root directory, such as "/home/kylin/test"

**Relative path** —— The path from the current directory.For example, when located in the /home directory, the relative path of the test file is "kylin/test"

There are "."and ".."files under each directory.

(3)The "."file representing the current directory and the ".."file representing the directory level above the current directory.When located in the /etc directory, the relative path of the test file is represented as... "/home/kylin/test"

### File Type
The system supports file types such as those in table below. 

|File type| Description
| :------------ | :------------ |
| General files| Includes text files, data files, executable binaries, etc
| Contents files(contents)|The system views the directory as a special file and uses it to form a hierarchical tree structure of the file system
| Equipment files(Character device file/block device file)|The system uses it to identify individual device drivers, which the kernel uses to communicate with hardware devices
|Symbolic link|Stored data is the path to a file in the file system;When a symbolic link file is invoked, the system will automatically access the paths saved in the file

<br>

## Window Composition
The Peony window can be divided into the toolbar and address bar, the folder label preview area, the sidebar, the window area and status bar, and the preview window in six sections.

![Fig 2 Window partitioning diagram-big](image/2.png)

### Toolbar And Address Bar
The functions corresponding to the icons on the toolbar are shown in table below.

|Icons|Description|Icons|Description
| :------------ | :------------ | :------------ | :------------ |
|![](image/icon1.png)|New folder	|![](image/icon7.png)|Command line
|![](image/icon2.png)|Return to previous level|![](image/icon8.png)|Retreat
|![](image/icon3.png)|Search folders,files, etc. with advanced search functions|![](image/icon9.png)|Select the view mode (small icon, medium icon, large icon)
|![](image/icon4.png)|Select the sorting method (name, modification date, etc.)|![](image/icon10.png)|Advanced features
|![](image/icon5.png)|Minimize|![](image/icon11.png)|Maximize
|![](image/icon6.png)|Close||||

<br>

### Folder Tag Preview Area
Users can view open folders through the folder tab preview area and are able to add additional folders by clicking the "![](image/icon12-o.png)"icon to add additional folders.

![Fig 3 Folder Label Preview Area](image/3.png)

### Sidebar
The sidebar lists the directory hierarchy for all files, providing a view of the different types of folder directories in users operating system.External mobile devices, remotely connected shared devices are also shown here.

![Fig 4 Peony Sidebar](image/4.png)

### Window Area
The window area lists the subdirectories, files under the current directory node.In the sidebar list, click on a directory, the contents of which should be displayed here, the contents should be displayed here, with the file icon highlighted to indicate that the file is selected.

![Fig 5 window area-big](image/5.png)

### Status Bar
(1)When entering a directory, the number of files in the current location is displayed.

(2)When a folder is selected, the number of files in that folder is displayed.

(3)When a file is selected, the type and size of that file is displayed.

(4)The slider in the bottom right corner is a zoom bar that can be dragged to adjust the file size.

![Fig 6 Status bar-big](image/6.png)

### Preview Window
Click on the preview icon in the upper right corner of the preview window to preview the details of the file, to the picture file as an example.In the preview window users can view the image name, type, size, creation time, resolution and other information, and underneath the image information, users can also rotate, edit, delete and other operations on the image.

|Icons|Description
| :------------ | :------------ |
|![](image/icon14.png)|Document Preview Only|

<br>

![Fig 7 Preview window](image/7.png)
<br>

## Main Function
### View 
Users can use the peony to view and manage files on local files, local storage devices (such as external hard drives), file servers and network shares.

In the peony , double-click any folder to view its contents (by default application);Or right-click a folder to open it in a new tab or new window.

#### View mode
By default, the system displays all files and directories in the form of icons.

The user can select the file view mode by clicking the "![](image/icon16-o.png)"icon to select the file view mode, the icon can be set to: large icon, small icon, list, details, tiled five modes.

![Fig 8 View Mode](image/8.png)

In the icon view, files in the peony are displayed as <big icon + file name>, like Fig 1.

![Fig 9 Icon view-big](image/9.png)

In the list view, files in the  peony are displayed as <small icon + file name + file information>.

![Fig 10 List view-big](image/10.png)

#### Sort By
Files can be sorted in different ways while browsing. The way the files are arranged depends on the folder view method currently in use, and the user can change it by clicking the "![](image/icon4.png)"icon on the toolbar to change it.

![Fig 11 Sorted by-big](image/11.png)

The various documents are sorted as follows:

(1)Sort by name: Alphabetical order by file name.

(2)Sort by modification date: Sort by the date and time the file was last changed; by default it is sorted from oldest to newest.

(3)Sort by type: files are sorted alphabetically by file type; similar files are grouped together and then sorted by name.

(4)Sort by size: Sort by file size; by default it is sorted from smallest to largest.

(5)Sort by ascending/descending: Sort in order from smallest to largest or from largest to smallest, depending on the corresponding type.

(6)Marker Filtering: Filter out files and folders in the current directory that have been marked with the same color.

![Fig 12 Mark sort-big](image/12.png)

### Search and Filter Functions
The File Browser provides users with search and filter functions.

The search function, i.e., clicking on the “” icon button, you can switch to the search input box.

Type in the search box and press enter, or wait a moment (less than 1 second) to search for files in the current directory.

![Fig 13 Search Function-big](image/13.png)

Filtering function: In the current directory or the searched files, users can customize the conditions, according to the name corresponds to the keywords, the type corresponds to the file type, the file size corresponds to the size of the file to be searched, the modification time corresponds to a certain period of time and so on for filtering. The filter function can only filter files in the current directory or after searching, it does not include the filtering of sub-folders in the current directory.

Display all filter boxes and the filtering criteria as shown below:

![Fig 14 Filter box-big](image/14.png)

### Common Operation
(1)Copy
* Way 1:Check, right-click > "Copy"> Target, right-click > "Paste"
* Way 2:Check, Ctrl+C > Target, Ctrl+V
* Way 3:Drag from the folder window where the project is located to the destination folder window

In way 3, if both folders are on the same hard disk device on the computer, the item is moved;If dragged from a USB stick to the system folder, the item will be copied.To drag copy on the same device, users need to hold down "Ctrl"while dragging.

(2)Move
* Way 1:Check, right-click > "Cut"> Target, right-click > "Paste"
* Way 2:Check, Ctrl+X > Target, Ctrl+V

(3)Delete

Delete to recycling bin:
* Way 1:Check, right-click > "Delete"
* Way 2:Selected, Delete
* Way 3:Select it and drag it to the "Trash"on the desktop

If the deleted files are on a removable device, eject the device without emptying the Trash, the deleted files on the removable device may not be visible on other operating systems, but they still exist;When the device is reinserted into the system used to delete this file, it will be visible in the Trash.

Delete permanently:
* Way 1:Delete again in the "Trash"
* Way 2:Selected, Shift+Delete

(4)Rename
* Way 1:Check, right-click > "Rename"
* Way 2:Selected, F2

To undo a rename, press Ctrl+Z to restore it.
### Formatting And Unloading Equipment
In the sidebar, for the device accessing the system, right-click and the menu appears as shown in figure below.

![Fig 16 Mobile device right-click menu](image/16.png)

* Unload/pop:Both of these serve the purpose of offloading mobile devices.The difference is that the device still exists in the system after uninstallation (unmounted state) and can no longer be found in the system when ejected.The "![](image/icon18-o.png)"button on the right side of the device is the pop-up role.
* Formatting:System default format is FAT32 file system, users can change to Ext2/3/4 or NTFS format;To use this feature, users need to uninstall the device first.
* Rename: It supports renaming mobile devices (e.g., USB flash drives, removable hard disks and other devices) and renaming partitions, some partitions are not supported for the time being (e.g., file system partitions, data disks, etc.).
* Encrypt Disk/(Cancel Encryption, Change Password, Lock Disk): Encrypt the current disk for higher security file protection. For the encrypted disk, the right-click menu will show Cancel Encryption, Change Password, Lock Disk. Cancel Encryption: Cancel the encryption of the disk, in addition, it may lead to the loss of files in the disk, please save the files first. Change Password: Change the password that you entered when encrypting the disk, enter the original password and re-enter the new password to change the encryption password. Lock Disk: Lock the disk, after the disk is locked, you need to verify the encryption password when you want to access the files in the disk, in addition, during the process of locking the disk, it may lead to the loss of the files being used, please save the files first.

![Fig 17 Formatted](image/17.png)

### Disk Encryption
Disk encryption is primarily used to encrypt and protect disk data, preventing the leakage of private data stored on disks. It mainly includes functions such as encrypting disks, continuing disk encryption, locking disks, unlocking disks, changing passwords, and canceling encryption.

* Encrypt Disk: Encrypts non-encrypted disk partitions, turning them into encrypted disk partitions. Encrypted disks need to be in an unlocked state to mount the disk and read/write disk data.
* Continue Disk Encryption: For disks that are not fully encrypted (e.g., interrupted unexpectedly during encryption), you can choose to continue disk encryption to complete the encryption process.
* Lock Disk: Closes an open encrypted partition. After locking, the disk cannot be mounted or read/written.
* Unlock Disk: Opens an encrypted partition, automatically mounts the disk, and allows reading/writing of disk data.
* Change Password: Changes the disk encryption password. After changing the password, the new password must be used for operations such as unlocking.
* Cancel Encryption: Cancels the encryption of an encrypted disk partition, turning it into a regular non-encrypted disk.

Note:

* During disk encryption/decryption, avoid shutting down, powering off, or removing the disk to prevent the risk of disk damage and data loss. Also, minimize read/write operations on the disk being encrypted/decrypted during the process.

#### (1) Operations on Disks Other Than Data and System Disks
##### Encrypt Disk

In the sidebar, right-click on an unencrypted device connected to the system, and a menu will appear as shown below.

![Fig endisk Mobile device right-click menu](image/endisk.png)

Click on "Encrypt Disk," and a disk encryption dialog box will pop up, as shown below.

![Fig endisk1 disk encryption](image/endisk1.png)

Select "Format and Encrypt" or "Encrypt with Data Retention," click "Confirm," and a password box will appear. You can choose password unlock, TPM+PIN unlock, or TPM automatic unlock (if TPM is supported), as shown below.

![Fig endisk2 disk encryption](image/endisk2.png) ![Fig endisk3 Mobile device right-click menu](image/endisk3.png) ![Fig endisk4 Mobile device right-click menu](image/endisk4.png)

Set different passwords or PINs based on your selection. You can choose a backup key save path (default path if not selected). Click "Confirm," and an authorization dialog box will appear, as shown below.

![Fig endisk5 disk encryption](image/endisk5.png)

After confirmation, enter the password for authorization. Encryption will begin after authorization, and a dialog box will pop up indicating that the disk has been encrypted, as shown below.

![Fig endisk6 disk encryption](image/endisk6.png)

##### Continue Disk Encryption

In the sidebar, right-click on a device connected to the system that is not fully encrypted, and a menu will appear as shown below.

![Fig cendisk disk encryption](image/cendisk.png)

Click on "Continue Disk Encryption," and a dialog box for continuing disk encryption will appear, as shown below.

![Fig cendisk1 disk encryption](image/cendisk1.png)

Enter the previously set encryption password, click "Confirm," and then enter password permissions based on the prompts. A dialog box will pop up indicating that the disk has been encrypted upon completion.

##### Lock Disk

In the sidebar, right-click on an encrypted and unlocked device connected to the system, and a menu will appear as shown below.

![Fig cdisk Mobile device right-click menu](image/cdisk.png)

Click on "Lock Disk," and a dialog box for locking the disk will appear, as shown below.

![Fig sdendisk1 lock disk](image/sdendisk1.png)

After clicking "Confirm," an authorization dialog box will appear. Enter the password for authorization, and the disk will be locked. A dialog box will pop up indicating that the disk has been locked upon completion.

##### Unlock Disk

In the sidebar, right-click on an encrypted and locked device connected to the system, and a menu will appear as shown below.

![Fig cdisk Mobile device right-click menu](image/cdisk1.png)

Click on "Unlock Disk," and a dialog box for unlocking the disk will appear, as shown below.

![Fig jsndisk1 unlock disk](image/jsendisk.png)

Enter the password and click "Confirm" to unlock the disk. A dialog box will pop up indicating that the disk has been unlocked upon completion.

##### Change Password

In the sidebar, right-click on an encrypted device connected to the system, and a menu will appear as shown below.

![Fig cdisk Mobile device right-click menu](image/cdisk.png)

Click on "Change Password," and a dialog box for changing the password will appear, as shown below.

![Fig ggendisk1 Change the password for the encrypted disk](image/ggendisk1.png)

Enter the old and new passwords, click "Confirm" to change the password. A dialog box will pop up indicating that the disk password has been changed upon completion.

##### Cancel Encryption

In the sidebar, right-click on an encrypted device connected to the system, and a menu will appear as shown below.

![Fig cdisk Mobile device right-click menu](image/cdisk.png)

Click on "Cancel Encryption," and a dialog box for canceling encryption will appear, as shown below.

![Fig qxendisk1 cancel encryption](image/qxendisk1.png)

After clicking "Confirm," an authorization window will pop up. Enter the password for authorization, and then a window for entering the disk password will appear. After entering the password and clicking "Confirm," decryption will begin, displaying a progress bar and real-time decryption progress. A dialog box will pop up indicating that disk encryption has been canceled upon completion.

#### (2) Operations on Data or System Disks
##### Encrypt Disk

In the sidebar, right-click on an unencrypted device connected to the system, and a menu will appear as shown below.

![Fig xtdisk Right-click menu of the system disk](image/xtdisk.png)

Click on "Start Encryption," and a dialog box will pop up prompting the user to confirm whether to encrypt the system disk and restart the system, a process that cannot be canceled. After successful encryption, the system cannot revert to a lower version, as shown below.

![Fig xtendisk1 disk encryption](image/xtendisk1.png)

After clicking "Confirm," an operation authorization window will pop up. Enter the password for authorization. After successful authorization, initialization will begin. Upon completion, a dialog box will pop up indicating that initialization has been completed. Click "Confirm" to restart the system. After restarting, the logged-in user will automatically see an encryption box. Follow the relevant operations to complete disk encryption, as shown below.

![Fig xtendisk2 disk encryption](image/xtendisk2.png)

You can choose password unlock, TPM+PIN unlock, or TPM automatic unlock (if TPM is supported). Set different passwords or PINs based on your selection. After confirmation, enter the password for authorization. Encryption will begin after authorization, displaying real-time encryption progress. A dialog box will pop up indicating that the disk has been encrypted upon completion.

##### Continue Encryption

Due to the longer encryption process for system or data disks, if encryption is interrupted unexpectedly during data retention, restarting the system or logging out and re-entering the system will bring up the disk encryption dialog box. Follow the prompts to continue disk encryption.

##### Change Password

Refer to the password change operation for disks other than data and system disks.

##### Cancel Encryption

In the sidebar, right-click on an encrypted device connected to the system, and a menu will appear as shown below.

![Fig xtqxdisk Right-click menu of the system disk](image/xtqxdisk.png)

Click on "Cancel Encryption," and a dialog box for canceling encryption initialization will appear, reminding the user that encryption of the system or data disk will be canceled and the system will restart, a process that cannot be canceled, as shown below.

![Fig xtqxendisk1 cancel encryption](image/xtqxendisk1.png)

After clicking "Confirm," an operation authorization window will pop up. Enter the password for authorization. After successful authorization, initialization will begin. Upon completion, a dialog box will pop up indicating that initialization has been completed. Click "Confirm" to restart the system. After restarting, decryption will be automatically completed in the character terminal, as shown below.

![Fig xtqxendisk2 cancel encryption](image/xtqxendisk2.png)

### Access Network
Used to share files over a LAN.

Exp:Right-click "Music", select "Share", and a dialog box will pop up.18.Users can set information and permissions for shared folders.

![Fig 18 Share option-big](image/18.png)

(1)After setting the sharing option, clicking Confirm will set the sharing and the window will close automatically, or not set if canceled.

(2)On another system on the same LAN, open a peony , select "Browse Network"and find the host name of the shared file.When opened, users can see the file that was shared. Double-click the file to bring up the connection prompt box.

![Fig 19 Connect-tip-big](image/19.png)

(3)After connecting, the contents of the shared file can be seen, and the connected host is also displayed in the sidebar.

![Fig 20 Shared folder contents-big](image/20.png)

(4)If users want to stop sharing the folder, users can right-click the folder again, and uncheck the sharing.



### Intelligent Space

Intelligent Space is a specialized view designed for intelligent user data management. Leveraging the capabilities of an AI subsystem, it currently offers AI-powered features such as file categorization by labels, summary previews, and semantic content search. Unlike traditional file manager views, Smart Space allows users to focus on their data within a dedicated interface called the Smart Space.

#### Space Management

##### （1）Create  Space

To create a new space by filtering conditions: Click the + button on the right side of the smart space in the sidebar of the main interface of the document management, and a space creation interface will pop up. Enter the name of the new space, add the filtering conditions, and then click the Create button to create a new smart space.

![Fig 28 create space](image/28.png)

![Fig 29 space name ](image/29.png)

![Fig 30 space view](image/30.png)

To create a new space through search: Open the main interface of the document management, click to focus on the search box, enter keywords to jump to the search interface for search, wait for the search to complete and display the search results, and click "Save as Space" to save the search results as space.

![Fig 31 search to create space](image/31.png)

![Fig 32 search space view](image/32.png)

##### （2）Delete Space

Right-click on the corresponding space in the peony sidebar or right-click on the corresponding space in the space view. A menu item will pop up. Click the "Delete Space" option, and a secondary confirmation window will appear. Click "OK" to delete the corresponding space.

![Fig 33 delete space side bar](image/33.png)

![Fig 34 delete space by space view](image/34.png)

##### （3）Rename Space

Right-click on the corresponding space in the Peony sidebar or right-click on the corresponding space in the space view. A menu item will pop up. Click the Rename option, and a rename text editing window will appear. Change the name and click OK to rename the corresponding space.

![Fig 35 rename space](image/35.png)

![Fig 36 rename space in view](image/36.png)



##### （4）edit and modify the space conditions to dynamically update the file

Right-click the button corresponding to the space in the peony sidebar and click the "Edit Space Conditions" option, or click "Edit Space" in the upper right corner of the space view. A "Edit Space Conditions" window will pop up. In this window, you can change the filtering conditions. After clicking "Apply", the files in the space will be dynamically updated based on the modified filtering conditions. Take the following figure as an example to filter the condition that the file name contains "newly created". After clicking "OK", the files with the file name containing "newly created" will be consolidated into the space.

![Fig 37 edit space condition](image/37.png)

![Fig 38 edit condition](image/38.png)



#### File operations within the space

##### 1.Manually add files into space

Right-click on the blank area of the space file view, and a menu will pop up. Click on the sub-item of the Add File menu, and a File Explorer window will pop up. Select the corresponding file, and you can manually add the file to this space. At the same time, it is compatible with document management, supporting dragging files into the space view and manually adding the file to the space.

![Fig 39 add file](image/39.png)

![Fig 40 file dialog](image/40.png)

![Fig 41 user add to space](image/41.png)

##### 2.Save As

In the Space File view, select the file, right-click the file to pop up the menu, click the "Save File As" menu item, and File Explorer will pop up. Select the folder path and the file name to be saved as. You can then save the file as a copy.

![Fig 42 save as](image/42.png)

![Fig 43 file dialog](image/43.png)

![Fig 44 save as file](image/44.png)

##### 3.Move file to another space

In the Space File view, select the file, right-click the file to pop up the menu, click Move to Space menu item, and a secondary menu will pop up. Click the menu sub-item corresponding to the space name to move the file from the current space to the selected menu item space.

![Fig 45 move to space](image/45.png)

![Fig 46 move to space view](image/46.png)

##### 4.Drag-and-drop file copying to create a copy is compatible with peony interaction

![Fig 47 Drag](image/47.png)

![Fig 48 Dragged](image/48.png)

##### 5.Summary extraction

 When previewing a file, display a summary of the file's content and output a descriptive text about the file, with a word count of approximately 20 to 50 words.

![Fig 49 summary](image/49.png)

#### 6.Intelligent Label

After the file index is created, Intelligent label will be automatically generated. When creating a new space, the existing smart tags will be displayed. Files that match the content of the selected smart tags can be aggregated in the smart space. And it supports dynamic updates, as well as previewing the label information of Spaces and files.

![Fig 50 labels](image/50.png)

![Fig 51 file label](image/51.png)

#### 7.Intelligent Sorting

After clicking to enter the corresponding space view, you can select "Intelligent Sorting" from the menu options in the upper right corner. The intelligent sorting rule prioritizes files based on their usage frequency, specifically the number of times they have been opened. If multiple files have been opened the same number of times, the sorting will further compare their most recent access times, with files accessed more recently taking precedence.

![52 Intelligent Sorting](image/52.png)

#### 8.Sort by Addition Method 

Click to enter the corresponding space view, and you can select the "Sort by Addition Method" option from the menu in the upper right corner. Addition methods can be categorized into manual addition and automatic addition. Manually added files are those that users manually upload or drag and drop into the space. Automatically added files are those that are automatically aggregated, updated, and included in the space based on set filtering criteria. When a specific addition method is selected, files added via that method will be prioritized in the sorting order.

![52 Addition method](image/53.png)

## Advanced Settings

Click the "![](image/icon10.png)"icon on the toolbar to enter the setup interface.

Advanced settings are the peony preferences.21,Preferences are divided into three parts: editing tools, setup items, and manual instructions.

(1)Edit: copy, paste, cut and delete

(2)Settings: Keep Allow, Show Hidden, Show File Extension, Show Create Time, Show Relative Time, Forbid thumbnailing, Index Settings, Resident in Backend,Parallel Operations, Set samba password, Open each folder in a new window, Plugin manager Settings, Connect to Server, Show Network, Local Disk Show Unmount Opt

(3)Manuals and Instructions: Help, About, Feedback

|Icons|Description|Icons|Description
| :------------ | :------------ | :------------ | :------------ |
|![](image/icon20.png)|	Reproduction of selected projects|![](image/icon21.png)|	Paste selected items
|![](image/icon22.png)|Cut the selected item|![](image/icon23.png)|Trash, empty all items


<br>

![Fig 21 Preferences](image/21.png)

The content that can be set contains multiple aspects, and the functions corresponding to each tab are briefly described in table below.

|Tabs|Description
| :------------ | :------------ |
|Keep Allow|Place the peony window on the top level for display
|Show Hidden|Whether the file starting with "." is displayed
|Show File Extension|Whether the file extension is displayed
|Show Create Time|Whether the file creation time is displayed
|Show Relative Time|Whether the file creation time is displayed
|Forbid thumbnailing|Whether to display the preview thumbnails of pictures, videos, and wps office files
|Resident in Backend|Whether the file manager program has been kept in the background without completely exiting
|Parallel Operations|Whether multiple file operations can be performed simultaneously
|Set samba password|You can set the samba password for the current user
|Open each folder in a new window|Whether a new window opens when the folder is opened
|Plugin manager Settings|Manage whether the plugin for the file manager is loaded
|Connect to Server|Displays the interface for connecting to the remote server
|Show Network|Whether sidebar network entries and computer view network entries are displayed
|Help|Jump to User Manual
|About|Show details of the peony 
|Feedback|Open the Feedback window of kylin-os-manager, you can submit suggestions or comments about the file manager

<br>

## Appendix
### 1 Shortcuts

|Shortcut key|Function|
| :------------ | :------------ |
| Ctrl+C|Copy |
| Ctrl+X|Cut|
| Ctrl+V|Paste|
| Delete|Delete|
| Shift+Delete|Permanent deletion|
| Ctrl+Z |Revoke|
| Ctrl+A|All|
| F2 or Alt+E| Rename|
| Alt+D or F4| Active address field |
| Ctrl+E or Ctrl+F or F3 | Activate the search box in peony|
| Ctrl+P | Show/hide the preview Window |
| Ctrl+Shift+N | New folder |
| Home | Keyboard focus fixed focus content area first item |
| End | Keyboard focus fixed focus content area last item |
| Ctrl+Mouse Wheel | Adjust the content area view mode |
| Backspace | Return to the previous folder |
| Alt+ ← | Return by history |
| Alt+ → | Go with the history |
| Ctrl+N | Create a peony window |
| Ctrl+W | Close the peony Windows on the desktop one by one |
| F11 | Maximize/restore, peony window |
| Ctrl+D or Delete | Move the selected item to the recycle bin |
| Shift+F10 | Open/close the right-click menu |
| Shift+ ←  →  ↑ ↓ | Select multiple items in a row in peony or desktop |
| Alt+Enter | Displays the properties of the selected item |
| Alt+ ↑| Return to the previous directory |
| Ctrl+H | Show hidden files |
| Alt+F4 | Close peony window |
| Ctrl+T | New tab page |
| Ctrl+Tab or Ctrl+Shift+Tab | Switch tabs |
| Ctrl+Shift+L | Reverse selection |
| Ctrl+Q | Exit the peony application |
| Ctrl+R or F5| Refresh current file view |

### 2 Wildcard Descriptions

|Wildcard|Description
| :------------ | :------------ |
|Asterisks（*）|Match zero or more characters|
|question mark（?）|Match any of the characters|
|[abl A-F]|Match any of the characters listed in square brackets, for example a, b, l or any of the uppercase characters from A to F|

### 3 Brief description of the system section

（1）/bin:Store command files that can be used by regular users.

（2）/boot:Contains files used by the kernel and other system programs when they start.

（3）/dev:The directory where the device files are located. The device is managed as a file in the operating system and can be manipulated as an operating file.

（4）/etc:The system's configuration file.

（5）/home:The location of the user's home directory, where user files are saved, including configuration files, documents, etc.

（6）/lib:Contains many shared library files that are used by programs in /bin.

（7）/opt:Store the files and programs that can be optionally installed, mainly by third-party developers to install their packages.

（8）/root:The home directory of the system administrator (root or superuser).

（9）/usr:Includes files and directories that are directly relevant to system users, and some major applications are also stored in this directory.

（10）/var:Contains some documents that change frequently. Such as false offline (spool) directories, file log directories, lock files, and temporary files.
