#!/bin/bash
cssName=$1
cssPath=${cssName##*/}
qssPath=${cssName%/*}
name=$(sed -r 's/(-)(\w)/\U\2/g'<<<${cssPath%.*})
qssName="$qssPath/$name.qml"
rm $qssName
touch $qssName

function gradient() {
if [[ "$line" == *"deg"* ]]; then
	#分离出渐变开始色，渐变结束色，背景色
	first=${1#*rgba}
	second=${first#*rgba}
	color1=${first%%r*}
	color2=${second%%r*}
	color3=${second#*rgba}
	color1=$(sed -r 's/,/\/255,/g'<<<${color1%%)*})
	color2=$(sed -r 's/,/\/255,/g'<<<${color2%%)*})
	color3=$(sed -r 's/,/\/255,/g'<<<${color3%%)*})
	color1Final="Qt.rgba$color1"")"
	color2Final="Qt.rgba$color2"")"
	color3Final="Qt.rgba$color3"")"

	#分离出参数名并转为驼峰命名
	name=$(sed -r 's/--//g'<<<${1%:*})
	name=$(sed -r 's/(-)(\w)/\U\2/g'<<<$name)

	#分离出渐变色渐变角度
	deg=${1#*(}
	deg=${deg%%deg*}
    deg=$[deg + 270]

	echo "    property var $name"" : GradientColor {" | sed 's/kerror/kError/g' | sed 's/kbrand/kBrand/g' | sed 's/linkvisited/linkVisited/g' | sed 's/ksuccess/kSuccess/g' | sed 's/kwarning/kWarning/g' | sed 's/kcomponent/kComponent/g' | sed 's/kcontain/kContain/g' | sed 's/kline/kLine/g' | sed 's/KSelect/kSelect/g' | sed 's/kselect/kSelect/g'>> $qssName
	echo "        gradientDeg : $deg" >> $qssName
	echo "        gradientStart : $color1Final" >> $qssName
	echo "        gradientEnd : $color2Final" >> $qssName
	echo "        gradientBackground : $color3Final" >> $qssName
	echo "    }" >> $qssName

else
	#分离出渐变前景色，背景色
	first=${1#*rgba}
	second=${first%),*}
	color1="Qt.rgba$second"
	color1Final=$(sed -r 's/,/\/255,/g'<<<${color1})
	third=${first#*rgba}
	color2=$(sed -r 's/,/\/255,/g'<<<${third%%;*})
	color2Final="Qt.rgba$color2"

	#分离出参数名并转为驼峰命名
	name=$(sed -r 's/--//g'<<<${1%:*})
	name=$(sed -r 's/(-)(\w)/\U\2/g'<<<$name)

	#分离出渐变色渐变角度
	deg=${1#*(}
	deg=${deg%%deg*}

	echo "    property var $name"" : GradientColor {" | sed 's/kerror/kError/g' | sed 's/kbrand/kBrand/g' | sed 's/linkvisited/linkVisited/g' | sed 's/ksuccess/kSuccess/g' | sed 's/kwarning/kWarning/g' | sed 's/kcomponent/kComponent/g' | sed 's/kcontain/kContain/g' | sed 's/kline/kLine/g' | sed 's/KSelect/kSelect/g' | sed 's/kselect/kSelect/g' >> $qssName
	echo "        gradientDeg : 0" >> $qssName
	echo "        gradientStart : $color1Final" >> $qssName
	echo "        gradientEnd : $color1Final" >> $qssName
	echo "        gradientBackground : $color2Final" >> $qssName
	echo "    }" >> $qssName
fi
}


function shadow() {
#分离出参数名并转为驼峰命名
name=$(sed -r 's/--//g'<<<${1%:*})
name=$(sed -r 's/(-)(\w)/\U\2/g'<<<$name)
data=${1#*:}
shadow=${data#*rgba}
shadow1=${data%%)*}
test=${shadow#*),}
shadow2=${test%)*}

echo "    property var $name"" : ShadowData {" | sed 's/kshadow/kShadow/g' >> $qssName
#判断阴影是普通阴影还是双层阴影
if [[ "$shadow" == *"rgba"* ]]; then
	echoShadow  "$shadow1"
	echo "        isMultiShadow : true" >> $qssName
	echo "        childShadow : ShadowData {" >> $qssName
	echoShadow  "$shadow2"
	echo "        isMultiShadow : false" >> $qssName
	echo "        }" >> $qssName
else
	echoShadow  "$shadow1"
	echo "        isMultiShadow : false" >> $qssName
fi
echo "    }" >> $qssName
}

#输出阴影参数
function echoShadow() {
	echo "        xOffset : ""${1%%px*}" >> $qssName
	yOff=${1#*px}
	echo "        yOffset : ""${yOff%%px*}" >> $qssName
	radius=${yOff#*px}
	echo "        blurRadius : ""${radius%%px*}" >> $qssName
	spread=${radius#*px}
	echo "        spread : ""${spread%%px*}" >> $qssName
	color=${spread#*rgba}
	echo "        shadowColor : Qt.rgba""$color"")" | sed 's/,/\/255,/g' >> $qssName
}


echo "import QtQml 2.15" >> $qssName
echo "import QtQuick 2.15" >> $qssName
echo "import org.ukui.quick.platform 1.0" >> $qssName
echo "DtThemeDefinition {" >> $qssName
echo "    id: dtTheme" >> $qssName

#将渐变色变量存储到字符串中
while read line
do
	if [[ "$line" == *"linear-gradient"* ]]; then
		str+="$line"
	fi
done < $1

while read line
do

    if [[ "$line" == *"rgb"* ]]; then

		if [[ "$line" != *"linear-gradient"* ]]; then
			if [[ "$line" == *"--kshadow-"* ]]; then
				shadow "$line"
			else
				if [[ "$line" != *"-shadow:"* ]]; then
        			echo $line | sed 's/--/    property color /g' | sed -r 's/(-)(\w)/\U\2/g' | sed 's/;//g' | sed 's/kgray/kGray/g' | sed 's/kcomponent/kComponent/g' | sed 's/kline/kLine/g' | sed 's/kfont/kFont/g' | sed 's/rgba/Qt.rgba/g' | sed 's/alternatebase/alternateBase/g' | sed 's/midlight/midLight/g' | sed 's/kwarning/kWarning/g' | sed 's/kline/kLine/g' | sed 's/windowtext/windowText/g' | sed 's/kbrand/kBrand/g' | sed 's/ksuccess/kSuccess/g' | sed 's/highlightedtext/highlightedText/g' | sed 's/tooltipbase/toolTipBase/g' | sed 's/brighttext/brightText/g' | sed 's/tooltiptext/toolTipText/g' | sed 's/buttontext/buttonText/g' | sed 's/placeholdertext/placeholderText/g' | sed 's/norole/noRole/g' | sed 's/kblack/kBlack/g' | sed 's/linkvisited/linkVisited/g' | sed 's/kerror/kError/g' | sed 's/kmodalmask/kModalMask/g' | sed 's/kwhite/kWhite/g' | sed 's/,/\/255,/g' | sed 's/highlightedtext/highlightedText/g' | sed 's/kdivider/kDivider/g' | sed 's/korange/kOrange/g' | sed 's/kgreen/kGreen/g' | sed 's/kred/kRed/g' | sed 's/Placeholdertext/PlaceholderText/g' | sed 's/KFont/kFont/g' | sed 's/kmenu/kMenu/g' | sed 's/KSelect/kSelect/g' | sed 's/kselect/kSelect/g'>> $qssName
				else
					shadow "$line"
				fi
			fi
		else
			gradient "$line"
		fi
    else
		if [[ "$line" == *"var"* ]]; then
			#判断使用var绑定的变量是否为渐变色变量
			varName=${line#*(}
			varName=${varName%)*}
			if [[ "$str" == *"$varName"* ]]; then
				echo $line | sed 's/var(--//g'| sed 's/--/    property var /g' | sed 's/body {//g' | sed 's/KGray/kGray/g' | sed -r 's/(-)(\w)/\U\2/g' | sed 's/)//g' | sed 's/;//g' | sed 's/kcomponent/kComponent/g' | sed 's/kline/kLine/g' | sed 's/kcontain/kContain/g' | sed 's/kerror/kError/g' | sed 's/tooltipbase/toolTipBase/g' | sed 's/KFont/kFont/g' | sed 's/kfont/kFont/g' | sed 's/windowtext/windowText/g' | sed 's/highlightedtext/highlightedText/g' | sed 's/norole/noRole/g' | sed 's/alternatebase/alternateBase/g' | sed 's/brighttext/brightText/g' | sed 's/midlight/midLight/g' | sed 's/KBlack/kBlack/g' | sed 's/KWhite/kWhite/g' | sed 's/kbrand/kBrand/g' | sed 's/KBrand/kBrand/g' | sed 's/tooltiptext/toolTipText/g' | sed 's/buttontext/buttonText/g' | sed 's/ButtonDisable/buttonDisable/g' | sed 's/ksuccess/kSuccess/g' | sed 's/kwarning/kWarning/g' | sed 's/kerror/kError/g' | sed 's/kdivider/kDivider/g' | sed 's/korange/kOrange/g' | sed 's/kgreen/kGreen/g' | sed 's/kred/kRed/g' | sed 's/KOrange/kOrange/g' | sed 's/KGreen/kGreen/g' | sed 's/KRed/kRed/g' | sed 's/KSelect/kSelect/g' | sed 's/kselect/kSelect/g' | sed 's/kmenu/kMenu/g' | sed 's/KMenu/kMenu/g' | sed 's/Placeholdertext/PlaceholderText/g' | sed 's/kgray/kGray/g' | sed 's/kblack/kBlack/g' | sed 's/kwhite/kWhite/g' >> $qssName
			else
				echo $line | sed 's/var(--//g'| sed 's/--/    property color /g' | sed 's/body {//g' | sed 's/KGray/kGray/g' | sed -r 's/(-)(\w)/\U\2/g' | sed 's/)//g' | sed 's/;//g' | sed 's/kcomponent/kComponent/g' | sed 's/kline/kLine/g' | sed 's/kcontain/kContain/g' | sed 's/kerror/kError/g' | sed 's/tooltipbase/toolTipBase/g' | sed 's/KFont/kFont/g' | sed 's/kfont/kFont/g' | sed 's/windowtext/windowText/g' | sed 's/highlightedtext/highlightedText/g' | sed 's/norole/noRole/g' | sed 's/alternatebase/alternateBase/g' | sed 's/brighttext/brightText/g' | sed 's/midlight/midLight/g' | sed 's/KBlack/kBlack/g' | sed 's/KWhite/kWhite/g' | sed 's/kbrand/kBrand/g' | sed 's/KBrand/kBrand/g' | sed 's/tooltiptext/toolTipText/g' | sed 's/buttontext/buttonText/g' | sed 's/ButtonDisable/buttonDisable/g' | sed 's/ksuccess/kSuccess/g' | sed 's/kwarning/kWarning/g' | sed 's/kerror/kError/g' | sed 's/kdivider/kDivider/g' | sed 's/korange/kOrange/g' | sed 's/kgreen/kGreen/g' | sed 's/kred/kRed/g' | sed 's/KOrange/kOrange/g' | sed 's/KGreen/kGreen/g' | sed 's/KRed/kRed/g' | sed 's/KSelect/kSelect/g' | sed 's/kselect/kSelect/g' | sed 's/kmenu/kMenu/g' | sed 's/KMenu/kMenu/g' | sed 's/Placeholdertext/PlaceholderText/g' | sed 's/kgray/kGray/g' | sed 's/kblack/kBlack/g' | sed 's/kwhite/kWhite/g' >> $qssName
			fi
		fi
    fi
done < $1
echo "}" >> $qssName

