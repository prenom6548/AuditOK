# Quick reference

- The official Libvirt docker image.

- Maintained by: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv).

- Where to get help: [openKylin RISC-V SIG](https://gitee.com/openkylin/riscv), [openKylin](https://gitee.com/openkylin/community).

# Libvirt | openKylin
Current Libvirt docker images are built on the [openKylin](https://factory.openkylin.top/kif/default/index). This repository is free to use and exempted from per-user rate limits.

Libvirt provides a portable, long term stable C API for managing the
virtualization technologies provided by many operating systems. It
includes support for QEMU, KVM, Xen, LXC, bhyve, Virtuozzo, VMware
vCenter and ESX, VMware Desktop, Hyper-V, VirtualBox and the POWER
Hypervisor.

Learn more about Libvirt on [libvirt Website](https://libvirt.org)⁠.

# Supported tags and respective Dockerfile links
The tag of each `libvirt` docker image is consist of the version of `libvirt` and the version of basic image. The details are as follows

| Tag                                                                                                                               | Currently                                 | Architectures |
|-----------------------------------------------------------------------------------------------------------------------------------|-------------------------------------------|---------------|
|[11.9.0-ok20](https://gitee.com/openkylin/openkylin-docker-images/blob/master/Cloud/libvirt/11.9.0/2.0/Dockerfile) | libvirt 11.9.0 on openKylin 2.0 | amd64,arm64,riscv64 |

# Usage
In this usage, users can select the corresponding `{Tag}` based on their requirements.

- Pull the `openkylin/libvirt` image from docker

	```bash
    # From Docker Hub
	docker pull openkylin/libvirt:{Tag}
	# From Quay
	docker pull quay.io/openkylin/libvirt:{Tag}
	```

- Start a libvirt instance

    ```
    docker run -it --rm openkylin/libvirt:{Tag}
    ```
    The `openkylin/libvirt` image is used to verify the integration between the upstream libvirt version and openKylin. 

# Question and answering
If you have any questions or want to use some special features, please submit an issue or a pull request on [openkylin-docker-images](https://gitee.com/openkylin/openkylin-docker-images).
