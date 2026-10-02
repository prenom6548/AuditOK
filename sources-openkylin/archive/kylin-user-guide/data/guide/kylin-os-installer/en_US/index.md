# System Installation
## Installation Preparation
* A hard disk can be divided into multiple partitions and they are independent of each other. So accessing different partition is like accessing different hard disk.

* Please backup the important data to other storage devices before installing system.

<br>

## Install Steps
### Boot Up
Insert the installation CD/U-disk and reboot.

According to the reminder at bootup, select to boot from installation device.

The system supports "Try without Installation" mode.

### Installation
Double click the installation icon on the desktop.

1) Select language.

![Fig 1 Language select-big](image/1.png)

2) Select time zone.

![Fig 2 Time zone select-big](image/2.png)

4) User configuration

![Fig 6 Create account-big](image/6.png)

5) Installation method

- "Quick Install": Format the whole disk and partition it automatically.

- "Custom Install": Customize partitions. 

Select "Quick Install" here.

![Fig 7 Install options-big](image/7.png)

![Fig 8 Format prompt-big](image/8.png)

7) Click "Start Installation" to start copying files to the disk.

When installation completed, click "Restart" to reboot the system.

<br>

## Custom Install
Here will show all mounted disks by list. Click "Create partition table" and then start to create partition.

Select free space > click "add".

![Fig 9 Creat partition-big](image/9.png)

1) /boot partition & / partition: "Used to" - ext4, "Mount point" -- /boot, /.

2) /swap partition: 2x the size of memory, "Used to " - linux-swap

3) /backup partition & /data partition: "Used to" - ext4, "Mount point" -- /backup, /data.

