#!/bin/bash 
#!/bin/sh
set -e 

unset DISPLAY

NEED_FACTORY_RESTORE=
for x in $(cat /proc/cmdline); do
    case $x in
        ostree-factory-restore)
            NEED_FACTORY_RESTORE=y
            ;;
        *)
            ;;
    esac
done

if [ x"$NEED_FACTORY_RESTORE" = "xy" ]; then
    /sbin/plymouthd --mode=shutdown --pid-file=/run/plymouth/pid --attach-to-session --kernel-command-line="quiet splash plymouth.debug"
    /bin/plymouth show-splash
    /bin/plymouth display-message --text="请再一次确认是否进行系统出厂还原，是否继续？[y/n]"
    is_sure=$(/bin/plymouth watch-keystroke)
    echo is_sure=$is_sure
    case $is_sure in
            y|Y)
                /bin/plymouth display-message --text="正在还原系统....."
                python3 /usr/sbin/kylin-ostree-restore.py
                sleep 3
                update-grub
                /bin/plymouth display-message --text="系统还原完成....."
                /bin/plymouth quit;;
            n|N)
                /bin/plymouth display-message --text="不恢复！....."
                sleep 10
                /bin/plymouth quit
                # # 注意，reboot重启不了，可用reboot -f
                reboot -f;;
            *) ;;
            esac
fi
