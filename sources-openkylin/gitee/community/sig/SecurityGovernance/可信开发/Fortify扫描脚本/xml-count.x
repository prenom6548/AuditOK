#!/bin/bash

report_dir=$(pwd)/report

function xml_count_title(){
	echo -n -e '自研包 \t文件数 \t代码行数 \tCritical \tHigh \tMedium \tLow \tSum \n' >> $report_dir/xml_count
}

function xml_count(){
	xml=$1
	pkg=${xml%.xml*}
	c1=`head -50 $xml | egrep -o '[0-9,]{,9} files' | awk '{print $1}'`
	c2=`head -50 $xml | egrep -o '[0-9,]{,9} LOC' | awk '{print $1}'`
	c3=`egrep '<Friority>Critical</Friority>' $xml |wc -l`
	c4=`egrep '<Friority>High</Friority>' $xml |wc -l`
	c5=`egrep '<Friority>Medium</Friority>' $xml |wc -l`
	c6=`egrep '<Friority>Low</Friority>' $xml |wc -l`
	c7=`egrep '<Friority>[[:graph:]]{,9}</Friority>' $xml |wc -l`
	echo -n -e $pkg '\t'$c1 '\t'$c2 '\t'$c3 '\t'$c4 '\t'$c5 '\t'$c6 '\t'$c7 '\n'  >> $report_dir/xml_count
}

function Category(){
	egrep '^[ ]*<Category>[[:graph:] ]*</Category>[ ]*$' $xml |cut -d '>' -f2|cut -d '<' -f1
}

echo > $report_dir/xml_count
cd $report_dir/xml
xml_count_title
if [ `echo $1 | wc -w` -gt 0 ]
	then xml=$1
	if [[ $xml =~ 'xml' ]]
		then xml_count $xml
	fi
	exit
fi
for xml in `ls -S -r .`
	do if [[ $xml =~ 'xml' ]]
		then xml_count $xml
	fi
done


