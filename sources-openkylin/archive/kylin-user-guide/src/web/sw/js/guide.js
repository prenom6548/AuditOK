/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

require.config({
    paths: {
        "marked": "/usr/share/kylin-user-guide/web/js/marked.min",
    }
});

var icon_theme;

require(['marked'], function (markedSetCustom) {

    //    alert('add marked');
    var marked = require('marked');
    marked.setOptions({
        renderer: new marked.Renderer(),
        gfm: true,
        tables: true,
        breaks: false,
        pedantic: false,
        sanitize: false,
        smartLists: true,
        smartypants: false
    });

    console.log(marked('I am using __markdown__.'));
    addhtmlapp();
    icon_theme = window.guideWebkit.js_getIconThemeName();
	switchTheme(icon_theme);
});

//readFile = function (fileName, callback) {
//  var xhr = new XMLHttpRequest();
//  xhr.open('GET', fileName);
//  xhr.onload = function () {
//    if (xhr.responseText != '') {
//      callback(xhr.responseText);
//    }
//  };
//  xhr.send();
//};

var html_list = {
    // onboard: "list_h1",
    // atril: "list_h2",
    // indicator_china_weather: "list_h4",
    // kylin_recorder: "list_h5",
    // kylin_usb_creator: "list_h6",
    // kylin_screenshot: "list_h7",
    // kylin_calculator: "list_h8",
    // kylin_camera: "list_h9",
    // kylin_scanner: "list_h10",
    // ukui_notebook: "h14",
    // ukui_clock: "h13",
    // gnome_tetravex: "list_h1",
    // gnome_mines: "list_h2",
    // iagno: "list_h3",
    error_code: "list_h18",
};

var jump_dict = {
    display: "显示器|display",
    audio: "声音|audio",
    power: "电源|power",
    notice: "通知|notice",
    vino: "远程桌面|vino",
    about: "关于|about",
    bluetooth: "蓝牙|bluetooth",
    printer: "打印机|printer",
    mouse: "鼠标|mouse",
    touchpad: "触控板|touchpad",
    keyboard: "键盘|keyboard",
    shortcut: "快捷键|shortcut",
    touchscreen: "触摸屏|touchscreen",
    projection: "多屏协同|projection",
    netconnect: "有线网络|wiredconnect",
    wlanconnect: "无线局域网|wlanconnect",
    proxy: "代理|proxy",
    vpn: "vpn|vpn",
    mobilehotspot: "移动热点|mobilehotspot",
    wallpaper: "背景|background",
    theme: "主题|theme",
    screenlock: "锁屏|screenlock",
    screensaver: "屏保|screensaver",
    fonts: "字体|fonts",
    userinfo: "账户信息|userinfo",
    loginoptions: "登录选项|biometrics|loginoptions",
    cloudaccount: "云帐户|cloudaccount",
    kylinid: "麒麟id|kylinid",
    date: "时间和日期|date",
    area: "区域语言|area",
    backup: "备份|backup",
    upgrade: "更新|upgrade",
    securitycenter: "安全中心|securitycenter",
    defaultapp: "默认应用|defaultapp",
    autoboot: "开机启动|autostart",
    search: "全局搜索|搜索|search",
    panel: "任务栏|panel",
    boot: "系统启动|boot",
    maintainmode: "维护模式|maintainmode",
    aisubsystem: "ai模块管理|aimodulemanager",
    accessibility: "辅助功能|accessibility"
}

var security_jump_dict = {
    securityscan:  "安全体检|securityscan",
    accountprotection: "账户保护|accountprotection",
    firewall: "防火墙|firewall",
    virusdefense: "病毒防护|virusprotection",
    privacysecurity: "隐私安全|privacysecurity",
    securitytools: "安全工具|securitytools",
    trustmeasure:"可信度量|trustmeasure",
    vulnerabilityrepair: "漏洞修复|vulnerabilityrepair"
}

var manager_jump_dict = {
    toolboxfilecrush:  "文件粉碎机|kylinfilecrush",
    toolboxnetworkconnectivitydetectiontools: "网络连通性检测工具|networkconnectivitydetectiontools",
    toolboxdfeet: "d-feet|d-feet",
    toolboxdiskusageanalyzer: "磁盘使用情况分析器|diskusageanalyzer"
}

//手册dbus跳转页面js接口函数
function qt_jumpApp(appName) {
    // 判断是否为新功能介绍
    if("kylin-new-function-introduction" == appName){
       jumpIntroduce('ukui-new-function-introduction','-nocheck');
       return; 
    }
    // 跳转页面后展示顶端内容
    $(window).scrollTop(0);
    //去掉传过来字段两端的空格
    appName = appName.trim()
    //判断是否是需要跳转标题的字段
    if (appName.search("/") >= 0) {
        //处理字段可能开头就是"/"的情况
        if (appName.indexOf("/") == 0) {
            appName = appName.slice(1, appName.length)
        }
        //截取跳转的app手册内容和显示app内容后的标题
        var list = appName.split("/")
        if (list.length == 2) {

            //跳转app手册内容页面
            onclickButton(list[0])

            //将“-”转换成“_”
            if (list[1].search("-") >= 0) {
                list[1] = list[1].replace(/-/g, "_");
            }

            //页面的资源都加载完成后跳转需要的标题
            $(document).ready(function () {
                // 控制面板走新逻辑
                if ("ukui-control-center" == list[0]) {
                    // 中文状态取值、英文暂时不管
                    setTimeout(function () {
                        var key = (list[1].replace(" ", "").toLowerCase());
                        var value = jump_dict[key];
                        // 检索匹配
                        // 只做三级标题的匹配
                        for (let heading of $('h3[id]')) {
                            const $heading = $(heading);
                            //去除空格
                            var title = $heading.text().replace(" ", "");
                            //英文模式下转小写
                            title = title.toLowerCase();
                            if (value.split('|').includes(title)) {
                                // 滚动导航栏位置
                                aim = "list_" + $heading.attr("id");
                                break;
                            }
                        }

                        if (aim.search("list_") >= 0) {
                            titleTOp = document.getElementById(aim).offsetTop;
                            document.getElementById('hlist').scrollTop = titleTOp;
                            document.getElementById(aim).click()
                        }
                    }, 300);
                } else if ("ksc-defender" == list[0]) {
                    // 中文状态取值、英文暂时不管
                    setTimeout(function () {
                        var key = (list[1].replace(" ", "").toLowerCase());
                        var value = security_jump_dict[key];
                        // 检索匹配
                        // 安全中心只需适配二级标题
                        for (let heading of $('h2[id]')) {
                            const $heading = $(heading);
                            //去除空格
                            var title = $heading.text().replace(" ", "");
                            //英文模式下转小写
                            title = title.toLowerCase();
                            if (value.split('|').includes(title)) {
                                aim = "list_" + $heading.attr("id");
                                break;
                            }
                        }
                        if (aim.search("list_") >= 0) {
                            titleTOp = document.getElementById(aim).offsetTop;
                            document.getElementById('hlist').scrollTop = titleTOp;
                            document.getElementById(aim).click()
                        }
                    }, 300);
                } else if ("kylin-os-manager" == list[0]) {
                    // 中文状态取值、英文暂时不管
                    setTimeout(function () {
                        var key = (list[1].replace(" ", "").toLowerCase());
                        var value = manager_jump_dict[key];
                        // 检索匹配
                        for (let heading of $('h3[id]')) {
                            const $heading = $(heading);
                            //去除空格
                            var title = $heading.text().replace(" ", "");
                            //英文模式下转小写
                            title = title.toLowerCase();
                            if (value.split('|').includes(title)) {
                                aim = "list_" + $heading.attr("id");
                                break;
                            }
                        }
                        if (aim.search("list_") >= 0) {
                            titleTOp = document.getElementById(aim).offsetTop;
                            document.getElementById('hlist').scrollTop = titleTOp;
                            document.getElementById(aim).click()
                        }
                    }, 300);
                } else {
		    setTimeout(function () {
		        var target = html_list[list[1]]
		        if (target.search("list_") >= 0) {
		            document.getElementById(target).click()
		        } else {
		            var t = $("#" + target).offset().top;// 获取需要跳转到标签的top值
		            $(window).scrollTop(t);// 跳转到指定位置
		        }
                    }, 300);
                }
            });
        }
    }
    else {
        onclickButton(appName);
        document.getElementById('hlist').scrollTop = 0;
    }
}


// 默认侧边栏展示比例
var oriScrollbarW = 24;
function onclickButton(str) {
    setTimeout(function () {
        window.guideWebkit.js_showBackbtn();
    }, 1);
    window.guideWebkit.js_buriedPoint(str);
    try {
        document.getElementById("mainUI").style.display = "none";
        document.getElementById("pageContent").style.display = "inline";
        var mdPath = window.guideWebkit.js_getIndexMdFilePathOther(str)
        var mdDate = window.guideWebkit.js_getIndexMdFileContent(mdPath)
        var mdData = window.guideWebkit.js_getFileSystemInformation(mdPath)
        window.guideWebkit.js_changeNavHighlight()
        var iconName = mdPath.split("/").slice(0, mdPath.split("/").length - 2)
        iconName.push(iconName[iconName.length - 1] + ".png")
        iconName = iconName.join("/")
        var m2ht = getDocTop(mdPath, mdDate)
    } catch (err) {
        console.log(err);
        document.getElementById("mainUI").style.display = "none";
        document.getElementById("pageContent").style.display = "inline";
        var mdPath = window.guideWebkit.js_getIndexMdFilePathOther(str)
        var mdDate = window.guideWebkit.js_getIndexMdFileContent(mdPath)
        var mdData = window.guideWebkit.js_getFileSystemInformation(mdPath)
        var iconName = mdPath.split("/").slice(0, mdPath.split("/").length - 2)
        iconName.push(iconName[iconName.length - 1] + ".png")
        iconName = iconName.join("/")
        m2ht = getDocTop(mdPath, mdDate);
    }
        var html = m2ht.html
        var hlist = m2ht.hlist
        var info = m2ht.info
        var hlist_str = "";
        for (i = 0; i < hlist.length; i++) {
            var pattren = /[0-9]/;
            var name_text = "";
            for (j = 0; j < hlist[i].text.length; j++) {
                if (hlist[i].text[j] != "." && !pattren.test(hlist[i].text[j])) {
                    name_text += hlist[i].text[j];
                }
            }
            if (hlist[i].type == "h1")
                hlist_str += "<dt>\n" + "<a class='name1' id=" + "list_" + hlist[i].id + " href=\'#" + hlist[i].id + "\'>" + name_text + "</a> \n</dt>";
            else if (hlist[i].type == "h2")
                if (name_text.length >= 11) {
                    hlist_str += "<dt>\n" + "<a class='name2'  onclick=onclickA('list_" + hlist[i].id + "')" + " id=" + "list_" + hlist[i].id + " href=\'#" + hlist[i].id + "\'>" + name_text + "</a> \n</dt>";
                }
                else {
                    hlist_str += "<dt>\n" + "<a class='name2'  onclick=onclickA('list_" + hlist[i].id + "')" + " id=" + "list_" + hlist[i].id + " href=\'#" + hlist[i].id + "\'>" + name_text + "</a> \n</dt>";
                }
            else if (hlist[i].type == "h3") {
                if (name_text.length >= 11) {
                    hlist_str += "<dt>\n" + "<a class='name2' onclick=onclickA('list_" + hlist[i].id + "')" + " id=" + "list_" + hlist[i].id + " href=\'#" + hlist[i].id + "\'>" + name_text + "</a> \n</dt>";
                }
                else {
                    hlist_str += "<dt>\n" + "<a class='name2' onclick=onclickA('list_" + hlist[i].id + "')" + " id=" + "list_" + hlist[i].id + " href=\'#" + hlist[i].id + "\'>" + name_text + "</a> \n</dt>";
                }
            }
        }
        hlist_str = "<dl>" + hlist_str + "</dl>";
        str_name = str.replace(/-/g, "_");
        if (navigator.language == "zh-CN" || navigator.language == "bo-CN" || navigator.language != "en-US") {
            var update = "更新时间："
            var goTop = "返回顶部"
        }
        else {
            var update = "Update time:"
            var goTop = "Go Top"
        }
        html_new = "<div class='top_title'>"       
            + "<div id='top_name'><h1><a name='page_top'>" + info.title + "</a></h1>"
            + "<p  class='date'>"
            + update
            + mdData
            + "</p>"
            + "</div></div>"
            + html
            + "<HR style='margin-top: 20px'>"
            + "<div class='page_top' >"
            + "<img src='file:////usr/share/kylin-user-guide/data/icon-go-top.png' " + "/>"
            + "<a style='text-decoration: none;'onclick=onclickA('') href='#page_top'>"
            + goTop
            + "</a>"
            + "</div>"

        document.getElementById("hlist").innerHTML = hlist_str
        document.getElementById("content").scrollTop = 0;
        document.getElementById("content").innerHTML = html_new


        $(window).scroll(scrollFun)
        onclickA("list_h1")

        var ScrollBarHeight = window.innerHeight;
    
}

// home 返回顶部
$(document).keydown(function (event) {
    if (event.keyCode == 36) {
        // 跳转页面后展示顶端内容
        $(window).scrollTop(0);
        document.getElementById("hlist").scrollTop = 0;
        onclickA('')
        return;
    }
});


function scrollFun() {
    setTimeout(function () {
        for (let heading of $('h2[id],h3[id]')) {
            const $heading = $(heading);
            // console.log($heading.length+"==="+$heading.text()+"==="+($heading.offset().top - $(document).scrollTop()))
            if ($heading.offset().top - $(document).scrollTop() > 10) {
                break;
            }
            // console.log($("#list_"+$heading.attr("id")).offset().top+"===="+$(window).height())
            onclickA("list_" + $heading.attr("id"))
            // if ($("#list_"+$heading.attr("id")).top - $(document).scrollTop()> $(window).height()) {
            //             $("#hlist").scrollTop($("#list_"+$heading.attr("id")).top);
            //         } 
            }
    }, 1);
}

var scrollTimeout; 
window.addEventListener('scroll', function() {
	clearTimeout(scrollTimeout); 
	scrollTimeout = setTimeout(function() {
	// 执行滚动停止后的操作
	window.guideWebkit.js_repaint();
	}, 50); 
    // 设置滚动停止的等待时间（毫秒） 
});

function getapp_name(qpp) {
    var dirforapp
    var mdPath = window.guideWebkit.js_getIndexMdFilePathOther(qpp)
    var mdDate = window.guideWebkit.js_getIndexMdFileContent(mdPath)

    console.log(mdPath)

    if (mdPath.search("/guide/") > 0) {
        dirforapp = "guide"
    }
    else if (mdPath.search("/guide-ubuntukylin/") > 0) {
        dirforapp = "guide-ubuntukylin"
    }

    var info = window.guideWebkit.js_getIndexMdFileTitle(mdPath)
    // console.log(window.guideWebkit.js_getIndexMdFileTitle(mdPath))  
    //console.log(info+"======"+qpp)
    if (info.search("帮助手册") < 0) {
        return info + "|" + dirforapp;
    }
    else {
        //console.log(info.title.slice(0,info.title.search("帮助手册")))
        return info.slice(0, info.search("帮助手册")) + "|" + dirforapp
    }
}

function addhtmlapp() {
    var test = window.guideWebkit.js_getIntoFilename();
    var systemName = window.guideWebkit.JudgmentSystem();

    var systemParaList = new Array(9); 
    var systemList = ['kylin-user-desktop','ukui-control-center','hot-key','kylin-os-installer',
    'system-file-manager','utilities-terminal','kylin-system-monitor', 'ukui-system-monitor' ,'peony','kylin-kmre','kylin-activation'];


    var element = document.getElementById("app");
    // 判断类型(应用工具/影音图像/办公娱乐/其他)
    var systemElement = document.getElementById("system");
    var officeElement = document.getElementById("office-entertainment-app");
    var otherElement = document.getElementById("other-app");
    var audioElement = document.getElementById("audio-video-app");
    for (i = 0; i < test.length; i++) {
        var dirname = test[i].slice(0, test[i].indexOf("|"))
        var pngname = test[i].slice(test[i].indexOf("|") + 1, test[i].length)
        // if (dirname == "software-compatibility" || dirname == "technical-assistance"
        //     || dirname == "kylin-user-desktop" || dirname == "ukui-control-center"
        //     || dirname == "hot-key"
        //     || dirname == "hardware-compatibility"
        //     || dirname == "kylin-os-installer.desktop"
        //     || dirname == "system-file-manager"
        //     || dirname == "ukui-system-monitor"
        //     || dirname == "utilities-terminal"
        //     || dirname == "kylin-system-monitor"
        //     || dirname == "peony"
        // ) {
        //     continue
        // }
        // else {
            if (navigator.language == "zh-CN" || navigator.language == "bo-CN"  || navigator.language != "en-US") {
                var NameAndDir = getapp_name(dirname);
                var realname = NameAndDir.slice(0, NameAndDir.indexOf("|"));
                var dir = NameAndDir.slice(NameAndDir.indexOf("|") + 1, NameAndDir.length)
                // 这里对隐私与声明进行单独处理
                if ('银河麒麟最终用户使用许可协议和隐私政策声明' == realname) {
                    realname = '隐私与声明';
                }
                realname = realname.replace(" ", "");
            } else {
                var NameAndDir = getapp_name(dirname);
                var realname = NameAndDir.slice(0, NameAndDir.indexOf("|"));
                var dir = NameAndDir.slice(NameAndDir.indexOf("|") + 1, NameAndDir.length)
                if ('Kylin Operating System License Agreement and Privacy Statement' == realname) {
                    realname = 'Privacy and Statement';
                }
                if (realname.search("-") >= 0)
                    realname = (realname.split("-").map(item => item[0].toUpperCase() + item.slice(1, item.length))).join(" ")
                realname = (realname.split(" ").map(item => item[0].toUpperCase() + item.slice(1, item.length))).join(" ")
            }

            var para = createSystemElement(dir,dirname,realname,pngname);

            var systemElement = document.getElementById("system");
            otherList = ['kylin-service-support', 'Services-and-Supports', "kylin-os"];
            audioList = ['kylin-music', 'pictures', 'kylin-video', 'kylin-recorder', 'kylin-screenshot', 'kylin-scanner', 'kylin-camera', 'kylin-photo-viewer', 'kylin-screencap', 'kolourpaint'];
            officeList = ['evolution', 'reversi', 'mines', 'tetravex', 'atril', 'kylin-calculator'];

            if (systemList.includes(dirname)) {
                // 固定系统组件顺序  --- 使用数组
                switch (dirname) 
                { 
                    case 'kylin-os-installer':
                        systemParaList[0] = para;
                        break; 
                    case "kylin-user-desktop":
                        systemParaList[1] = para;
                        break; 
                    case 'ukui-control-center':
                        systemParaList[2] = para;
                        break; 
                    case 'hot-key':
                        systemParaList[3] = para;
                        break; 
                    case 'system-file-manager':
                        systemParaList[4] = para;
                        break; 
                    case 'peony':
                        systemParaList[4] = para;
                        break; 
                    case 'utilities-terminal':
                        systemParaList[5] = para;
                        break; 
                    case 'kylin-system-monitor':
                        systemParaList[6] = para;
                        break; 
                    case 'ukui-system-monitor':
                        systemParaList[6] = para;
                        break; 
                    case 'kylin-kmre':
                        systemParaList[7] = para;
                        break; 
                    case 'kylin-activation':
                        systemParaList[8] = para;
                        break;
                    default:
                        systemParaList.push(para)
                }
            } else if (otherList.includes(dirname)) {
                otherElement.appendChild(para);
            } else if (audioList.includes(dirname)) {
                audioElement.appendChild(para);
            } else if (officeList.includes(dirname)) {
                officeElement.appendChild(para);
            } else {
                element.appendChild(para);
            }

        // }
    }

    for(var i=0;i<systemParaList.length;i++){
        if(systemParaList[i]==null||systemParaList[i]==""){
            continue;
        }else{
            systemElement.appendChild(systemParaList[i]);
        }
    }

    // 开机引导添加至最后
    var para;
    if (navigator.language == "zh-CN" || navigator.language == "zh" || navigator.language == "zh-HK" || navigator.language == "kk" || navigator.language == "ug"|| navigator.language == "ky"
                                || navigator.language == "bo-CN" || navigator.language == "mn-MN" || navigator.language == "vi-VN" || navigator.language == "th-TH" || navigator.language == "ms-MY") {
        para = createSystemElement('guide','kylin-new-function-introduction',
                                    '新功能介绍','kylin-new-function-introduction.png');
    }else{
        para = createSystemElement('guide','kylin-new-function-introduction',
                                    'New Function Introduce','kylin-new-function-introduction.png')
    }
    para
    otherElement.appendChild(para);
}

function createSystemElement(dir,dirname,realname,pngname){
    var para = document.createElement("div")
    var node1 = document.createElement("br")
    var node2 = document.createElement("img")
    var node3 = document.createElement("div")
    var apptext = document.createTextNode(realname)
    var attr = document.createAttribute("onclick")
    if('kylin-new-function-introduction' == dirname){
        attr.value = "jumpIntroduce('ukui-new-function-introduction','-nocheck')";
    }else{
        attr.value = "onclickButton('" + dirname + "')"
    }
    node3.appendChild(apptext)
    node3.style.lineHeight = "22px"
    node3.style.paddingLeft = "15px"
    node3.style.paddingRight = "15px"
    var themeIcon = 'file:////usr/share/icons/'+icon_theme+'/scalable/apps/'+dirname.trim()+'.svg';
    //node2.src = "file:////usr/share/kylin-user-guide/data/" + dir + "/" + dirname + "/" + pngname + ""
    // node2.alt = dirname
    node2.hidden = true
    para.id = dirname;
    para.style.marginLeft = "26px";
    para.className = "system-app";
    para.setAttributeNode(attr)
    para.appendChild(node2)
    para.appendChild(node1)
    para.appendChild(node3)
    checkImg(themeIcon, para);
    return para
}

var style_name
function SwitchStyle(id) {
    var css = document.getElementById("css")
    if (id == 1) {
        css.setAttribute("href", "css/guide-white.css");
        style_name = 1
        clearAllDtAttr()
    } else {
        css.setAttribute("href", "css/guide-black.css");
        style_name = 2
        clearAllDtAttr()
    }
}

function switchTheme(themeName){
    setTimeout(function () {
        icon_theme = themeName;
        apps = document.getElementsByClassName('system-app');
        for(let i = 0 ; i < apps.length ; i++){
            // var theme_src = 'file:////usr/share/icons/'+icon_theme+'/scalable/apps/'+apps[i].id.trim()+'.svg';
            // 锯齿问题改用 96*96图标
            var theme_src = 'file:////usr/share/icons/'+icon_theme+'/96x96/apps/'+apps[i].id.trim()+'.png';
			checkImg(theme_src,apps[i])
//			if(checkImg(theme_src)){
//				alert(1)
//				apps[i].children[0].src = theme_src;   
//			}
          //  var imgObj = new Image()
      //      imgObj.src = theme_src
            // alert(theme_src)
  //           imgObj.onload = function () {
//				alert(1)                
				//a();							
//apps[i].children[0].src = theme_src;    
 //            }
             //imgObj.onerror = function () {
            //	 alert(0)
            //}
            // if(fso.FileExists(theme_src)){
            //     apps[i].children[0].src = theme_src;    
            //     alert(1)
            // }else{
            //     alert(0)
            // }
            // break;
        }
    }, 1);
        // alert(themeName);
}

function checkImg(img_src,app){
    var imgObj = new Image();
    imgObj.src = img_src;
    imgObj.onload = function () {
        successCallback(img_src,app);
    }
    imgObj.onerror = function () {
        errorCallback(img_src,app);
    }
}

function successCallback(img_src,app){
    app.children[0].src = img_src;
    app.children[0].hidden = false;
}

function errorCallback(img_src,app){
    console.log("iconFileNotExist:", img_src);
    if (img_src.includes("/hicolor/")) {
	var firstPath = 'file:////opt/system/resource/kylin/kylin-user-guide/data/guide/' + app.id.trim() + '/' + app.id.trim() + '.png';
        var fallbackPath = 'file:////usr/share/kylin-user-guide/data/guide/' + app.id.trim() + '/' + app.id.trim() + '.png';
            
        app.children[0].onerror = function() {
            this.src = fallbackPath;
        };
            
        app.children[0].onload = function() {
            this.hidden = false;
        };
            
        app.children[0].src = firstPath;
        app.children[0].hidden = false;
    } else {
        var themeIcon = 'file:////usr/share/icons/hicolor/scalable/apps/'+ app.id.trim() + '.svg';
        checkImg(themeIcon, app);
    }
}

function changebuttonicon(val) {
    for (let i = 0; i < 3; i++) {
        let buttonName = "button" + i.toString()
        if (arrows_png[buttonName] == 0) {
            if (val == 1) {
                document.getElementById(buttonName).style.backgroundImage = 'url("/usr/share/icons/ukui-icon-theme-default/scalable/actions/pan-down-symbolic.svg")';
            }
            if (val == 2) {
                document.getElementById(buttonName).style.backgroundImage = 'url("/usr/share/kylin-user-guide/data/ukui-down-symbolic.svg")';

            }
        }
        if (arrows_png["button" + i.toString()] == 1) {
            if (val == 1) {
                document.getElementById(buttonName).style.backgroundImage = 'url("/usr/share/icons/ukui-icon-theme-default/scalable/actions/pan-end-symbolic.svg")';
            }
            if (val == 2) {
                document.getElementById(buttonName).style.backgroundImage = 'url("/usr/share/kylin-user-guide/data/ukui-end-symbolic.svg")';
            }
        }
        document.getElementById(buttonName).style.backgroundSize = "24px 24px";
    }
}

function clearAllDtAttr() {
    var dt_list = document.getElementsByTagName("dt")
    if (dt_list.length > 0) {
        for (let i = 0; i < dt_list.length; i++) {
            let elem = dt_list[i].children
            if (elem[0].hasAttribute("style")) {
                elem[0].removeAttribute("style")
                Refresh_the_content_interface()
            }

        }
    }
}

function goBackMainUI_ubuntu(style) {
    if (navigator.language == "zh-CN" || navigator.language == "zh" || navigator.language == "zh-HK" || navigator.language == "kk" || navigator.language == "ug"|| navigator.language == "ky"
				|| navigator.language == "bo-CN" || navigator.language == "mn-MN" || navigator.language == "vi-VN" || navigator.language == "th-TH" || navigator.language == "ms-MY") {
        if ("2" == style)
            window.location.href = "index-ubuntukylin_black_sw.html"
        else
            window.location.href = "index-ubuntukylin_sw.html"
    }
    else {
        if ("2" == style)
            window.location.href = "index-ubuntukylin_en_US_black_sw.html"
        else
            window.location.href = "index-ubuntukylin_en_US_sw.html"
    }
}

var arrows_div = {
    system: 0,
    app: 0,
    support: 0,
}

var arrows_big_png = {
    width: 0,
    height: 0,
}

var arrows_png = {
    button1: 0,
    button2: 0,
    button3: 0,
}

function change_arrows(button_name) {
    if (arrows_png[button_name] == 0) {
        if (style_name == 1) {
            document.getElementById(button_name).style.backgroundImage = "url(" + "/usr/share/icons/ukui-icon-theme-default/scalable/actions/pan-end-symbolic.svg" + ")";
        }
        if (style_name == 2) {
            document.getElementById(button_name).style.backgroundImage = "url(" + "/usr/share/kylin-user-guide/data/ukui-end-symbolic.svg" + ")";
        }
        document.getElementById(button_name).style.backgroundSize = "24x 24px";
        arrows_png[button_name] = 1;
        return
    }
    if (arrows_png[button_name] == 1) {
        if (style_name == 1) {
            document.getElementById(button_name).style.backgroundImage = "url(" + "/usr/share/icons/ukui-icon-theme-default/scalable/actions/pan-down-symbolic.svg" + ")";
        }
        if (style_name == 2) {
            document.getElementById(button_name).style.backgroundImage = "url(" + "/usr/share/kylin-user-guide/data/ukui-down-symbolic.svg" + ")";
        }
        document.getElementById(button_name).style.backgroundSize = "24x 24px";
        arrows_png[button_name] = 0;
        return
    }
}

function onclickarrows(str) {
    //console.log(str)
    if (str == 'system') {
        if (arrows_div[str] == 0) {
            document.getElementById(str).style.display = "none"
            arrows_div.system = 1;
        }
        else {
            document.getElementById(str).style.display = "inline"
            arrows_div.system = 0;
        }
    }
    else if (str == 'app') {
        if (arrows_div[str] == 0) {
            document.getElementById(str).style.display = "none"
            arrows_div.app = 1;
        }
        else {
            document.getElementById(str).style.display = "inline"
            arrows_div.app = 0;
        }
    }
    else if (str == 'support') {
        if (arrows_div[str] == 0) {
            document.getElementById(str).style.display = "none"
            arrows_div.support = 1;
        }
        else {
            document.getElementById(str).style.display = "inline"
            arrows_div.support = 0;
        }
    }

}

var old_str = "";
function onclickA(str) {
    resizeEvent();

    // 返回首页逻辑调整
    if (str == "") {
        str = "list_h1";
        // // alert(1);
        // document.getElementById("hlist").scrollTop = 0;
        // // onclickA("list_h1")
        // $("#list_h1").css({ "backgroundColor": "#3D6BE5", "color": "white" })
        // old_str = "list_h1";
    } else {
        for (let heading of $('h2[id],h3[id]')) {
            const $heading = $(heading);
            if ($heading.offset().top - $(document).scrollTop() > 20) {
                break;
            }
            // 检测页面滚动后当前页面距离顶部最近的标题
            str = "list_" + $heading.attr("id");
        }
    }
    //console.log(old_str,str) 
    if (old_str != "") {
        $("#" + old_str).removeAttr("style");
    }
    if (str != "") {
        $("#" + str).css({ "backgroundColor": highColor, "color": "white" })
        old_str = str
    }
    if (str == "") {
        document.getElementById("hlist").scrollTop = 0;
    }
}

function Refresh_the_content_interface() {
    if (old_str != "") {
        document.getElementById(old_str).click()
    }
    resizeEvent();
}

function getDocTop(mdFile, mdData) {
    var hlist = [];
    var info = {};
    var html = '';

    var path = mdFile.slice(0, mdFile.lastIndexOf('/') + 1);
    var count = 0;
    var renderer = new marked.Renderer();
    renderer.heading = function (text, level) {
        var id = 'h' + count;
        count++;
        if (level == 1) {
            var title = text.split('|')[0];
            var logo = text.split('|')[1];
            logo = path + logo;
            //console.log(logo);
            info = { "title": title, "logo": logo };
            return "";
        }
        if (level == 2) {
            text = text.split('|')[0];
            //console.log(text);
        }
        var type = 'h' + level;
        if (level == 2 || level == 3) {
            text.trim();
            if(2 == text.length){
                text = text.charAt(0)+" "+text.charAt(1);
            }
            hlist.push({ "id": id, "text": text, "type": type });
            //console.log(id);
            //console.log(text);
            //console.log(type);
        }
        return "<" + type + " id=" + id + " text=" + text + " class='html_h5'>" + text + "</" + type + ">\n";
    };

    renderer.listitem = function (text) {
        // alert(text)
        var text = " ● " + text + "\n"
        return "<div>" +text+ "</div>";
    }

    // 内容页面图片展示
    renderer.image = function (href, title, text) {
        var hrefX2 = href;
        //console.log(hrefX2);
        //         if (devicePixelRatio >= 1.5 && href.indexOf('.svg') == -1) {
        //             var path = href.split('.');
        //             var ext = path.pop();
        // //            hrefX2 = `${path.join('.')}x2.${ext}`;
        //             hrefX2 =  path.join('.')+ "x2." + ext;
        //         }
        if (text == "")
            //            return `<img src="${hrefX2}" data-src="${href}" alt="${text}" />`;
            return '<img style="max-height:98%;max-width:98%;" src=\"' + hrefX2 + '\" data-src=\"' + href + '\" alt=' + text + '/>';
        else {
            if (text.lastIndexOf("-big") < 0) {
                //                return `<h5  style="text-align: center;"><img src="${hrefX2}" data-src="${href}" alt="${text}" /><br>${text}</h5>`;
                return '<h5  style="text-align: left;font-family: Noto Sans SC; font-size: 12px;color=#333333;"><img style="max-height:98%;max-width:98%;"  src=\"' + hrefX2 + '\" data-src=\"' + href + '\" alt=' + text + '/><br> ' + text + '</h5>';
            }
            else {
                text1 = text.slice(0, text.lastIndexOf("-big"))
                //路径需要用双引号，单引号无法解析相对路径
                //                return `<h5 style="text-align: center;"><img  style="height:98%;width:98%;" src="${hrefX2}" data-src="${href}" alt="${text}" /><br>${text1}</h5>`;
                var size = window.guideWebkit.js_getBigPngSize(path + hrefX2);
                if (size[0] != 0 && size[1] != 0)
                    return '<h5 style="text-align: left;font-family: Noto Sans SC; font-size: 12px;color=#333333;"><img  style="height:98%;width:98%;max-width:' + size[0] + 'px;max-height:' + size[1] + 'px;" src=\"' + hrefX2 + '\"" data-src=\"' + href + '\" alt=' + text + '/><br>' + text1 + '</h5>';
                else
                    return '<h5 style="text-align: left;font-family: Noto Sans SC; font-size: 12px;color=#333333;"><img  style="height:98%;width:98%;" src=\"' + hrefX2 + '\"" data-src=\"' + href + '\" alt=' + text + '/><br>' + text1 + '</h5>';
            }
        }
    };
    renderer.table = function (header, body) {
        //console.log(header,body)
        if (body) body = '<tbody>' + body + '</tbody>';
        return '<div align="left"style="font-family:Noto Sans SC;font-size: 12px;"><table border=black cellspacing="0" width="90%" style="margin:0px">\n'
            + '<thead>\n'
            + header
            + '</thead>\n'
            + body
            + '</table>\n</div>';
    }
    html = marked(mdData, { "renderer": renderer }).replace(/src="/g, '$&' + path);
    return { "html": html, "hlist": hlist, "info": info };
}

//  function $(id) {
//     return document.getElementById(id)
// }

function dragScrollBar() {
    // var    oPageWidth = $("box"),
    // alert(1);
    // 左侧导航栏宽度
    oTop = document.getElementById("hlist");
    // 右侧内容宽度
    oBottom = document.getElementById("content");
    //页面宽度
    oPageWidth = oTop.offsetWidth + oBottom.offsetWidth;
    oScrollBar = document.getElementById("line");
    // 左侧最小宽度
    lMinWidth = 196;
    // 右侧最小宽度（注意内外边距）
    rMinWidth = 476;
    oScrollBar.onmousedown = function (e) {
        var disX = (e || event).clientX;
        oScrollBar.left = oScrollBar.offsetLeft;
        document.onmousemove = function (e) {
            var iT = oScrollBar.left + ((e || event).clientX - disX);
            var e = e || window.event,
                tarnameb = e.target || e.srcElement;
            var maxT = oPageWidth - oScrollBar.offsetWidth;
            oScrollBar.style.margin = 0;
            iT < lMinWidth && (iT = lMinWidth);
            iT > maxT - rMinWidth && (iT = maxT - rMinWidth);
            // 优化成百分比
            oriScrollbarW = oScrollBar.style.left = oTop.style.width = (iT / oPageWidth) * 100 + "%";
            oBottom.style.width = (oPageWidth - iT) / oPageWidth * 100 - 6 + "%";
            // 如果实时更新则可以采用像素跟随鼠标
            // oScrollBar.style.left = oTop.style.width = iT + "px";
            // oBottom.style.width =     oPageWidth - iT) + "px";
            // $("msg").innerText = 'top.width:' + oScrollBar.style.width + '---bottom.width:' + oBottom.style.width + '---oScrollBar.offsetLeft:' + oScrollBar.offsetLeft + '---disX:' + disX + '---tarnameb:' + tarnameb.tagName;
            return false
        };
        document.onmouseup = function () {
            document.onmousemove = null;
            document.onmouseup = null;
            oScrollBar.releaseCapture && oScrollBar.releaseCapture()
        };
        oScrollBar.setCapture && oScrollBar.setCapture();
        return false
    };
};

var normalWidth = window.innerWidth;
var windowsWidth = $(window).width();
var normalHeight = window.innerHeight;

var isSw = window.guideWebkit.js_judgmentSW();

if(isSw){
    $(window).resize(function() {
        clearTimeout(window.resizedFinished);
        window.resizedFinished = setTimeout(function(){
            console.log('Resizing finished.');
            resizeEvent();
        }, 100);
    });
}else{
    window.onresize = function () {
        resizeEvent();
    }
}

function resizeEvent() {
    // 优化窗口放大时，左边宽度
    var contentWidth = document.documentElement.offsetWidth;
    var leftNavWidth = 260;
    document.getElementById("content").style.width = (contentWidth - leftNavWidth) + "px";
    // alert((contentWidth - 240) + "px");
}

function jumpIntroduce(cmd,args){
    window.guideWebkit.js_buriedPoint(cmd);
    if(cmd.length != 0)
        window.guideWebkit.js_executeCommand(cmd,args);
}

//强调色默认蓝色
highColor = "#3790FA"
function changeNavHighlight(color){
    if("" != color){
        highColor = color;
        $("#" + old_str).css({ "backgroundColor": highColor, "color": "white" })
    }
}
