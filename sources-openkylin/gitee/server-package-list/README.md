# openKylin 服务器版本镜像软件包列表

本仓库用于维护 openKylin 服务器版本镜像的软件包列表，并提供可直接用于镜像构建的 profile 渲染结果。

## 目标

本仓库主要面向两个目标：

1. 维护 openKylin 不同镜像形态所需的软件包列表，包括基础系统、服务器、桌面、平台适配等不同组合。
2. 展示 Server 镜像的核心软件包构成，方便社区开发者了解当前设计并参与共建。

## 文档

- [Server 镜像核心软件包列表](./server-package-list.md)

该文档用于展示 Server 镜像的核心能力和软件包构成。实际构建输入仍以 `package-lists/*.list` 为准。

## 镜像构建包列表

`package-lists/` 目录提供轻量化的镜像构建包列表管理方式：`*.list` 维护包集合，`profiles.conf` 维护包集合之间的依赖关系。

包列表可以按照基础能力、镜像形态、硬件平台或特定场景拆分，并通过 `profiles.conf` 组合成最终镜像 profile。随着后续真实硬件、桌面环境或更多场景加入，可以继续扩展新的 list 和 profile。

当前仅允许生成以 `.img` 结尾的镜像 profile：

```bash
scripts/render-profile.sh normal-qemu-riscv64.img
scripts/render-profile.sh server-qemu-riscv64.img
```

生成结果位于 `generated/` 目录，可作为镜像构建工具的包列表输入：

| 生成文件 | 用途 |
| --- | --- |
| [normal-qemu-riscv64.img.packages](./generated/normal-qemu-riscv64.img.packages) | RISC-V 普通 CLI 镜像完整包列表。 |
| [server-qemu-riscv64.img.packages](./generated/server-qemu-riscv64.img.packages) | RISC-V Server 镜像完整包列表。 |

## 说明

- `package-lists/*.list` 是维护入口。
- `generated/*.packages` 是渲染结果，可直接作为镜像构建工具输入。
- `server-package-list.md` 是 Server 镜像核心软件包展示文档。
- 欢迎社区开发者共同参与服务器版本软件包列表的共建工作。
