# UKUI Intelligent Data Manager

## [中文README](docs/zh_CN/README.md)

## Introduction
UKUI Intelligent Data Manager (ukui-idm) is a project which intends to provide an intelligent user interface for local files management. To distinguish the traditional file manager, such as peony, caja, nautilus, ukui-idm allow users focus their data on a special view. We call that view as 'Intelligent Space'.

Unlike the regular directory, an intelligent space may contain several files from different folder. Intelligent space will list those files with some user-defined condition. User can create an intelligent space and space condition is also editable.

There are 2 default spaces for every user, "Work" and "Download". "Work" space will find user document in computer and list them together, and "Download" will retrieve the downloaded files from web browser, netdisks, chat tools, etc...

## Build Depends(project with development files)
- cmake
- pkg-config
- glib2.0(libglib2.0-dev)
- peony(libpeony-dev)
- Qt/QtQuick(qtbase5-dev,qtdeclarative5-dev)
- ukui-file-metadata
- ukui-quick
- kysdk-filesystem
