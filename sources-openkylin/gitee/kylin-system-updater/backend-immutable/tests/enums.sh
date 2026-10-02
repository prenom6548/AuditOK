
# D-Bus session bus名称
bus_name="com.kylin.systemupgrade"

# D-Bus对象路径
object_path="/com/kylin/systemupgrade"

# D-Bus接口名称
interface_name="com.kylin.systemupgrade.interface"

# D-Bus信号名称
update_method="UpdateDetect"
update_finished_signal="UpdateDetectFinished"

download_method="UpdateDownloadAll"
download_finished_signal="UpdateDownloadFinished"

deploy_method="DeployLatestUpdate reboot"
deploy_finished_signal="DeployUpdatFinished"

CMD_PREFIX=""
test_tmpdir="/tmp/ostree-auto-test"

BUILD_MASTER_BRACH="testos/buildmaster/newbranch"
file_to_monitor="/var/log/kylin-system-updater/kylin-system-updater.log.1"
http_port="8889"
process_name="python3 -m http.server "$http_port

metadata_string="--add-metadata=download-size='8000000000' --add-metadata=install-size='111220202' --add-metadata=update-name_zh_CN='系统更新'  --add-metadata=update-name_en_US='dddddddddd' --add-metadata=description='GalaxyKylinV10SP1SystemUpdate' --add-metadata-string='icon=/usr/share/kylin-update-desktop-config/icon/kylin-update-desktop-system.png' --add-metadata-string=changelog='initialcommit' --add-metadata-string=total_download_size='800MB' --add-metadata-string=total_install_size='1.2GB' --add-metadata-string=update-type='kylin-update-desktop-system'"

TAIL_LINE_NUM="2"

updater_program=$(pwd)/../kylin-system-updater 

PUSH_DIR="/opt/kylin-software-properties"
PUSH_FILE="$PUSH_DIR/ostree-important.list"
OS_DATA="/tmp/ostree-auto-test/osdata"