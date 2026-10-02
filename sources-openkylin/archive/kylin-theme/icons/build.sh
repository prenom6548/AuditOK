#!/bin/bash

folder_system="."
folder_default="ukui-icon-theme-default"
folder_fashion="ukui-icon-theme-fashion"
folder_svg="scalable/actions"
folder_16="16x16/actions"
folder_24="24x24/actions"
folder_32="32x32/actions"

function mfile ()
{
 for file in `ls $1`
 do
 if [ -d $1"/"$file ]
 then
  readfile $1"/"$file
 else
  if [ "${file##*-}"x = "symbolic.svg"x ];then
   gtk-encode-symbolic-svg $folder_default"/"$folder_svg"/"$file 16x16 -o $folder_default"/"$folder_16
   gtk-encode-symbolic-svg $folder_default"/"$folder_svg"/"$file 24x24 -o $folder_default"/"$folder_24
   gtk-encode-symbolic-svg $folder_default"/"$folder_svg"/"$file 32x32 -o $folder_default"/"$folder_32
  fi
 fi
 done
}


function hfile ()
{
 for file in `ls $1`
 do
 if [ -d $1"/"$file ]
 then
  readfile $1"/"$file
 else
  if [ "${file##*-}"x = "symbolic.svg"x ];then
   gtk-encode-symbolic-svg $folder_fashion"/"$folder_svg"/"$file 16x16 -o $folder_fashion"/"$folder_16
   gtk-encode-symbolic-svg $folder_fashion"/"$folder_svg"/"$file 24x24 -o $folder_fashion"/"$folder_24
   gtk-encode-symbolic-svg $folder_fashion"/"$folder_svg"/"$file 32x32 -o $folder_fashion"/"$folder_32
  fi
 fi
 done
}

mfile $folder_system"/"$folder_default"/"$folder_svg
hfile $folder_system"/"$folder_fashion"/"$folder_svg

function rfile ()
{
 for file in `ls $1`
 do
 if [ -d $1"/"$file ]
 then
  readfile $1"/"$file
 else
  if [ "${file##*-}"x = "symbolic.symbolic.png"x ];then
    tt="-symbolic.symbolic.png"
    mv $1"/"$file $1"/"${file//$tt}.symbolic.png
  fi

 fi
 done
}
rfile $folder_system"/"$folder_default"/"$folder_16
rfile $folder_system"/"$folder_default"/"$folder_24
rfile $folder_system"/"$folder_default"/"$folder_32
rfile $folder_system"/"$folder_fashion"/"$folder_16
rfile $folder_system"/"$folder_fashion"/"$folder_24
rfile $folder_system"/"$folder_fashion"/"$folder_32

gtk-update-icon-cache $folder_fashion
gtk-update-icon-cache $folder_default
gtk-update-icon-cache "ukui-icon-theme-classical"
