#! /bin/bash
# Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
#
# Authors:
#  Kobe Lee    lixiang@kylinos.cn
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 3.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

set -e

script_path=`dirname $0`

# generate protobuf source
cd $script_path/communication/protobuf/
proto_file_list=(`ls *.proto`)

for protofile in "${proto_file_list[@]}"
do
	/usr/bin/protoc -I=./ --cpp_out=./ "${protofile}"
done

echo "Generate protobuf source finished."

# generate qbus source
cd $script_path/dbus/
/usr/bin/qdbusxml2cpp cn.kylinos.Kmre.Window.xml -i metatypes.h -a window_dbus

echo "Generate qdbus source finished."

exit 0
