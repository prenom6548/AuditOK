#!/bin/bash

raw_dir=$(pwd)
src_tar_dir=$raw_dir/src_tar2
src_folder_dir=$raw_dir/src_folder
report_dir=$raw_dir/report
rule_dir=$raw_dir/rule
patch_dir=$raw_dir/patch
template_default_dir='/home/sx/Fortify-360-3.0.0-Analyzers_and_Apps-Linux-x64/Core/config/reports'
template_user_dir='/home/sx/.fortify/config/AWB360-5.10/reports/'

function error_msg(){
    echo $1
    exit
}

function init(){
    mkdir -p $src_tar_dir
    mkdir -p $src_folder_dir
    mkdir -p $report_dir/fpr/0
    mkdir -p $report_dir/fpr/1
    mkdir -p $report_dir/pdf
    mkdir -p $report_dir/xml
    mkdir -p $report_dir/rtf
    if [[ -e $report_dir/multisca ]]
        then echo > $report_dir/multisca
    fi
}

function init_rule_opt(){
	if [[ $1 == 'default' ]]
		then rule_opt=''
		else rule_opt="$1"
	fi
}

function help(){
    echo -e '\tfortify_sca.x decompress xxx.tar'
    echo -e '\tfortify_sca.x 0|1 xxx.tar|xxx default|"-rules /path_to_rule -no-default-rules"'
    echo -e '\tfortify_sca.x patch 0 xxx default|"-rules /path_to_rule -no-default-rules" xxx(patched_dir)'
}

function decompress(){
	tar=$1
	pkg=${tar%.tar*}
    cd $src_folder_dir
    mkdir -p $pkg
	cd $pkg
	if [ `find . -maxdepth 1 -type d |wc -w` -eq 1 ]
		then tar -xf $src_tar_dir/$tar -C .
	fi
	a=`find . -maxdepth 1 -type d`
	b=${a#*./}		
	cd $b
	if [ `find . -maxdepth 1 -type d |wc -w` -eq 1 ]
		then dpkg-source -x *.dsc
	fi		
	a=`find . -maxdepth 1 -type d`
	pkg_dir=${a#*./}
	cd $pkg_dir
}

function install_dep(){
	echo y | debuild > ./tmp1 2> ./tmp2
	a=`grep 'dpkg-checkbuilddeps' ./tmp1 | sed 's/([><= 0-9a-z.-]*)//g'`
	dep=${a##*:}
    if [[ `echo $2 | wc -w` -gt 0 ]]
	    then sudo apt install -y $dep
    fi
}

function pre_make(){
    install_dep
	if [ `ls | grep 'setup.py'| wc -w` -gt 0 ] 
		then return
	fi	
	if [ `ls | grep 'autogen.sh'| wc -w` -gt 0 ] 
		then ./autogen.sh
	fi	
	if [ `ls | grep 'autogen.sh'| wc -w` -eq 0 -a `find . -maxdepth 1 -name 'configure'| wc -w` -eq 0 -a  `ls | grep 'configure.ac'| wc -w` -gt 0 ] 
		then autoreconf -i -f
	fi	
	if [ `find . -maxdepth 1 -name 'configure'| wc -w` -gt 0 ] 
		then ./configure
	fi	
	if [ `ls | grep '.pro'| wc -w` -gt 0 ] 
		then qmake
	fi	
	if [ `ls | grep 'CMakeLists.txt'| wc -w` -gt 0 ] 
		then cmake .
	fi	
}

function fpr_2_pdf(){
	#template_defaults=$template_default_dir/DeveloperWorkbook.xml
	template_user=$template_user_dir'/detail1.xml'
	fpr=$1
	pkg=${fpr%.fpr*}
	ReportGenerator -format pdf -f $3/$pkg.pdf -source $2/$fpr -template "$template_user"
}

function fpr_2_rtf(){
	#template_defaults=$template_default_dir/DeveloperWorkbook.xml
	template_user=$template_user_dir'/detail1.xml'
	fpr=$1
	pkg=${fpr%.fpr*}
	ReportGenerator -format rtf -f $3/$pkg.rtf -source $2/$fpr -template "$template_user"
}

function fpr_2_xml(){
	#template_defaults=$template_default_dir/DeveloperWorkbook.xml
	template_user=$template_user_dir'/detail1.xml'
	fpr=$1
	pkg=${fpr%.fpr*}
	ReportGenerator -format xml -f $3/$pkg.xml -source $2/$fpr -template "$template_user"
}

function multisca0(){
	template_user=$template_user_dir'/detail1.xml'
	exclude_dir="./third_party"
	sourceanalyzer -b dr -clean	
	sourceanalyzer -b dr -nc gcc `find . -type f -name '*.h'| grep -v "/3rdparty/"`
	sourceanalyzer -b dr -nc gcc `find . -type f -name '*.c'| grep -v "/3rdparty/"`
	sourceanalyzer -b dr -nc gcc `find . -type f -name '*.cc'| grep -v "/3rdparty/"`
	sourceanalyzer -b dr -nc gcc `find . -type f -name '*.cpp'| grep -v "/3rdparty/"`
	sourceanalyzer -b dr -nc gcc `find . -type f -name '*.cxx'| grep -v "/3rdparty/"`
	sourceanalyzer -b dr $rule_opt -Xmx3g -Xms3g -scan -f $report_dir/fpr/0/$1.fpr
	# sourceanalyzer -b dr -nc gcc *.c -exclude logviewlib/*.c logviewlib/*.cc logviewlib/*.cpp logviewlib/*.cxx logviewlib/*.h 
	sourceanalyzer -b dr $rule_opt -Xmx3g -Xms3g -scan -f $report_dir/fpr/0/$1.fpr
	fpr_2_pdf $1.fpr $report_dir/fpr/0 $report_dir/pdf
	fpr_2_xml $1.fpr $report_dir/fpr/0 $report_dir/xml
	fpr_2_rtf $1.fpr $report_dir/fpr/0 $report_dir/rtf
	python3 $raw_dir/xml_issuecount.py basic $report_dir/xml/$1.xml 
}

function multisca1(){
    pre_make
	sourceanalyzer -b dr -clean	
	if [ `find . -maxdepth 1 -name '[Mm]akefile'| wc -w` -gt 0 ] 
		then make -j8
        ret=$?
        if [[ ! $ret -eq 0 ]]
	        then echo  -n -e $1 '\t' $ret >> $report_dir/multisca
            exit
        fi
        make -j8 clean
        sourceanalyzer -b dr make -j8
		sourceanalyzer -b dr $rule_opt -scan -f $report_dir/fpr/1/$1.fpr
	fi	
	echo  -n -e $1 '\t' $? >> $report_dir/multisca
}

function multisca_decompress(){
    init
    decompress $2
}

function multisca0_patch(){
	template_user=$template_user_dir'/detail_patch1.xml'
	sourceanalyzer -b dr -clean	
	sourceanalyzer -b dr -nc gcc `find . -type f -name '*.h'`
	sourceanalyzer -b dr -nc gcc `find . -type f -name '*.c'`
	sourceanalyzer -b dr -nc gcc `find . -type f -name '*.cc'`
	sourceanalyzer -b dr -nc gcc `find . -type f -name '*.cpp'`
	sourceanalyzer -b dr -nc gcc `find . -type f -name '*.cxx'`
	sourceanalyzer -b dr $rule_opt -Xmx3g -Xms3g -scan -f $report_dir/fpr/0/$1_$2.fpr
	cd $src_folder_dir/$2
	sourceanalyzer -b dr -clean	
	sourceanalyzer -b dr -nc gcc `find . -type f -name '*.h'`
	sourceanalyzer -b dr -nc gcc `find . -type f -name '*.c'`
	sourceanalyzer -b dr -nc gcc `find . -type f -name '*.cc'`
	sourceanalyzer -b dr -nc gcc `find . -type f -name '*.cpp'`
	sourceanalyzer -b dr -nc gcc `find . -type f -name '*.cxx'`
	sourceanalyzer -b dr $rule_opt -Xmx3g -Xms3g -scan -f $report_dir/fpr/0/$1_$2.fpr
	fpr_2_pdf $1_$2.fpr $report_dir/fpr/0 $report_dir/pdf
	fpr_2_xml $1_$2.fpr $report_dir/fpr/0 $report_dir/xml
	python3 $raw_dir/xml_issuecount.py patch $report_dir/xml/$1_$2.xml 
}

function multisca_tar(){
    init
    decompress $2
    multisca$1 $pkg
}

function multisca_folder(){
    init
    if [ ! -e $src_folder_dir/$2 ]
        then error_msg "dir does not exist"
    fi
    cd $src_folder_dir/$2
	a=`find . -maxdepth 1 -type d`
	pkg_dir=${a#*./}
	cd $pkg_dir
    multisca$1 $2
}

function multisca_folder_patch(){
    init
    if [ ! -e $src_folder_dir/$2 ]
        then error_msg "dir $2 does not exist"
    fi
    if [ ! -e $src_folder_dir/$4 ]
        then error_msg "dir $4 does not exist"
    fi
    cd $src_folder_dir/$2
    multisca$1_patch $2 $4
}

if [[ $1 == '-h' ]]
    then help
fi
if [[ ($1 == 'decompress') && ($2 =~ 'tar') ]]
    then decompress $2
fi
if [[ (($1 == '0') || ($1 == '1')) && (`echo $2 | wc -w` -gt 0) ]]
    then init_rule_opt "$3"
	if [[ $2 =~ 'tar' ]]
        then multisca_tar $1 $2
        else multisca_folder $1 $2
    fi
fi
if [[ $1 == 'patch' ]]
	then if [[ (($2 == '0') || ($2 == '1')) && (`echo $5 | wc -w` -gt 0) ]]
		then init_rule_opt "$4"
		multisca_folder_patch $2 $3 $4 $5
	fi
fi






