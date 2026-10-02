#!/bin/bash

project_path=$(cd `dirname $0`; pwd)
cd $project_path

sudo cp org.kylin.printer.gschema.xml /usr/share/glib-2.0/schemas
sudo glib-compile-schemas /usr/share/glib-2.0/schemas
