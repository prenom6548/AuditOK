# Backup and Recovery
## Overview
Kylin Backup and Recovery Tool is used to backup and restore system or user data.

It supports to create new backup points, and incremental backup based on a backup point; as well as restore the system to the state at the time of a backup, and partial restore retaining user data.

There are two modes: General, Grub backup and restore.

<br>

## General
### System Backup

![Fig 1 system backup -big](image/1.png)


- **New System Backup** -- The data that not in /backup、/media、/run、/proc、/dev、/sys、/cdrom、/mnt, etc , can be backup.

Click "Start Backup" Button will  enter the page select backup location, you can choose to backup to backup partition or mobile device, or customize other path. As shown in figure 2.

![Fig 2 system backup device choose -big](image/2.png)

Click "Next" button to enter the environment checking page. Inspection, such as backup space is enough, etc . And show check results. See 3,4.
 
![Fig 3 Checking -big](image/3.png)

![Fig 4 Check Result -big](image/4.png)

Click "Next" button to enter the backup preparing page. You can define the backup name in it. As shown in figure 5.

![Fig 5 Preparing -big](image/5.png)

Then click "Next" button to backup. As shown in figure 6.

![Fig 6 Backuping -big](image/6.png)

The backup process can click "cancel" button, cancel the backup operation. 
Backup is completed, enter the backup finish page, show result. As shown in figure 7.

![Fig 7 Backup Finish -big](image/7.png)

Click the "Home Page" button, click "Backup Management >>" tab, pop-up window of system backup management, you can see the backup just in it. As shown in figure 8.

![Fig 8 Backup Management -big](image/8.png)

In the backup management page, select a backup, click the "Delete" button, remove the corresponding backup point.

### System Restore
System restore can restore the system to its previous backup state.

![Fig 9 system restore -big](image/9.png)

Click "Start Restore" button, the system backups list window will pop up. As shown in figure 10.

![Fig 10 select backup -big](image/10.png)

Select the corresponding backup point, click the "Ok" button, enter the system restore environmental checking page. As shown in figure 11.

![Fig 11 checking -big](image/11.png)

Checking success, then click the "Next" button to enter the page of restore, to start the system restore. As shown in figure 12.

![Fig 12 select backup -big](image/12.png)

### Data Backup and Data Recovery
- **Data Backup** -- Backup the directories or files specified by user, and the function is similar to system backup.

![Fig 13 Data backup-big](image/13.png)

Note:``、$()、;、&amp;、| etc. The symbols that can contain and execute system commands or used for continuous execution of system commands are added to the blacklist, and the name of the directory to be backed up is judged before backup. If the symbols in the blacklist are included, backup is not allowed.

Click "Start Backup" and enter the backup path selection page. As shown in figure 14.

![Fig 14 Specify directories or files -big](image/14.png)

Or click "Update Backup" button, pop up data backup list window. As shown in figure 15.

![Fig 15 data backup list -big](image/15.png)

Then, click "Next" button, enter backup checking page --> backup preparing --> backuping --> backup finished. Such as system backup.
- **Update Data Backup** -- On the basis of a data backup, update the backup data.
- **Data Recovery** -- Recovery the data to its previous backup state, and the function is similar to system restore.

![Fig 16 Data recovery-big](image/16.png)

### Log Records
Record all operations on backup and recovery tool, and the interface as shown in Fig 17. 

![Fig 17 Log records-big](image/17.png)

### Ghost Image
Install from ghost image means generate an image file from an installed system, and then use this image file to copy the same state system to other computers. To use this function, a lacol system backup is needed first.

#### Create Ghost Image
The interface as shown in Fig 18.

![Fig 18 Ghost image-big](image/18.png)

Click "Start Ghost" and there will pop up a window to show all local system backups. Users select one of them and click "Ok" button, into the image storage location selection interface, can choose a local or mobile device, as shown in Fig 19.

(Tips: The architecture in name depends on hardware platform.)

![Fig 19 Ghost image position -big](image/19.png)

Then click "Next" button, enter checking page. As shown in figure 20.

![Fig 20 Ghost image checking -big](image/20.png)

If success, then click "Next" button, enter makeing page. As shown in figure 21.

![Fig 21 Ghost image makeing -big](image/21.png)

Image filename is "computer name + architecture + backup name.kyimg", and the backup name removed some symbol. As shown in figure 22.

![Fig 22 Ghost image finished -big](image/22.png)

#### Install from Ghost Image
1) Copy the ghost image (saved in /ghost) to U-disk or other removable storage device.

2) Enter livecd system and insert the removable device.

3) If it doesn't mount automatically, users can manually mount device through terminal:

sudo mount /dev/sdb1 /mnt

Removable device is /dev/sdb1 by general, and it can be seen by command "fdisk -l".

4) Double click the install button to start installation guide. Select "Install from ghost image" in Installation Type, and find the ghost image in the removable device. The follow-up steps can refer to installation manual.

(Tips: The architecture in name depends on hardware platform.)

If there exists data partition when creating ghost image file, it needs to check "Create Data Partition" in the next step.

<br>

## Grub Backup and Recovery

(Tips: The date and time in the figure is for reference only.)

1) Select "Backup and Restore" in grub menu when start-up.  Note: Only machines with backup partitions when the system is installed are supported.

2) Backup or restore are both allowed to select here. If there is an error, restart the system and try again.

- Backup mode: The system will start to backup immediately and the prompt will display on the screen.

For backup mode, is smilar to "Whole Disk Backup" in general mode. If there is no enough space, the backup will fail.

- Restore mode: The system will start to restore to the lastest successful backup state, and the prompt will display on the screen.

For restore mode,  If there is no successful backup in /backup partition, the system will not be restored.

## Q&A
- Can not use backup and recovery tool
In order to use this tool, "Create Backup Partition" must be checked when install system.

<br>

- There is insufficient space on the backup partition
Backup partition (/ backup) size is fixed installation system, should not be too big commonly, as system usage gradually increase, will cause storage is not enough.At this point can be backup to a mobile device, but backup speed will slow down.
Recommended for system only a die erste installation backup to the backup partition, at the back of the backup system use mobile device. Of the more important data into a directory, backup in data backup function.
