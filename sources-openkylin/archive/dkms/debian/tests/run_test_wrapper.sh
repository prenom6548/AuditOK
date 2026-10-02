#!/bin/sh

KERNEL_VER=
NO_SIGNING_TOOL=
for kver in $(dpkg-query -W -f '${Package}\n' 'linux-headers-*' | sed s/linux-headers-//)
do
	if [ -d "/lib/modules/$kver/build" ]
	then
		KERNEL_VER=$kver
		grep -q "^CONFIG_MODULE_SIG_HASH=" "/lib/modules/$kver/build/.config" ||
			NO_SIGNING_TOOL="--no-signing-tool"
		break
	fi
done

export KERNEL_VER

# Ubuntu specific -- All arches have a signing tool, but not all of them have
# update-secureboot-policy. This argument tells run_test.sh to account for DKMS
# output that appears when update-secureboot-policy is missing.
NO_UPDATE_SB_POLICY=
command -v update-secureboot-policy || NO_UPDATE_SB_POLICY="--no-update-sb-policy"

bash ./run_test.sh $NO_SIGNING_TOOL $NO_UPDATE_SB_POLICY
