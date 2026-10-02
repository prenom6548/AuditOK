#! /bin/bash

dir=(./ukui-icon-theme-default ./ukui-icon-theme-classical ./ukui-icon-theme-fashion)
dirPath=""
item=""

function check(){  
    for file in ` ls $1 `  
    do  
        if [ -d $1"/"$file ]   
        then
            check $1"/"$file  
        else
	    if [ "$item"x != "index.theme"x ] && [ "$item"x != "icon-theme.cache"x ]
            then
                # 这里是对文件是否重名判断
                find $dirPath"/"$item -name $file | tail -n +2

                # 下面是对文件权限判断
                if [ -x $file ] && [ -f $file ];
                then
                    chmod 664 $file	
                fi
	    fi
        fi  
    done  
} 

for ((i=0; i<=2; i++))
do
    dirPath="${dir[$i]}"
    # 缓存文件检查
    gtk-update-icon-cache $dirPath

    for item in `ls $dirPath`; do
        # 对文件包含空格检查
        find $dirPath"/"$item -name '* *'
        check $dirPath"/"$item
    done
done
