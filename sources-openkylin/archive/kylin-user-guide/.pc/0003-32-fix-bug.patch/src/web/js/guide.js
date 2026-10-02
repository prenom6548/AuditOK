require.config({
    paths: {
        "marked": "/usr/share/javascript/marked/marked.min",
    }
});
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
	switchTheme(icon_theme);
});

// 函数需要写在页面上，导入方式不可以，具体原因暂未知
new QWebChannel(qt.webChannelTransport, function(channel) {
    window.bridge = channel.objects.bridge;
    bridge.mdPathOtherChanged.connect(updateattribute);
    bridge.mdContentChanged.connect(updatecontent);
    bridge.highColorChange.connect(updatehighcolor);
})

// 首页app加载
var updateattribute=function(value)
{
    getapp(value);
	appSizeChange(isTabletmode);
    // resizeEvent();
}
// 二级页面
var updatecontent=function(value)
{
    getcontent(value);
}

// 强调色
var updatehighcolor=function(value)
{
    changeNavHighlight(value);
}


function getapp(infoListStr){
    // alert(infoListStr)
    var systemParaList = new Array(8); 
    var systemList = ['kylin-user-desktop','ukui-control-center','hot-key','kylin-os-installer.desktop',
    'system-file-manager','utilities-terminal','kylin-system-monitor','peony','kylin-kmre'];

    var infoList = infoListStr.split(",");
    for(var i=0;i<infoList.length;i++){
        var value = infoList[i];
        // 名称|图片名称|路径|标题
        var valueList = value.split("|");
        // alert(valueList)
        var pngname = valueList[1];
        var mdPath = valueList[2];
        var info = valueList[3];
        var dirname = valueList[0];
        // /usr/share/kylin-user-guide/data/guide-ubuntukylin/xournal/zh_CN/index.md
        NameAndDir = getdirforapp(mdPath,info);
            
        if (navigator.language == "zh-CN" || navigator.language == "zh" || navigator.language == "zh") {
            var realname = NameAndDir.slice(0, NameAndDir.indexOf("|"));
            var dir = NameAndDir.slice(NameAndDir.indexOf("|") + 1, NameAndDir.length)
            // 这里对隐私与声明进行单独处理
            if ('银河麒麟最终用户使用许可协议和隐私政策声明' == realname) {
                realname = '隐私与声明';
            }
        } else {
            var dir = NameAndDir
            realname = dirname.replace(dirname[0], dirname[0].toUpperCase())
            if (realname.search("-") !== -1) {
                realname = realname.split("-")
                if (realname.length == 2) {
                    realname[1] = realname[1].replace(realname[1][0], realname[1][0].toUpperCase())
                }
                else if (realname.length == 3) {
                    realname[1] = realname[1].replace(realname[1][0], realname[1][0].toUpperCase())
                    realname[2] = realname[2].replace(realname[2][0], realname[2][0].toUpperCase())
                }
                else if (realname.length == 4) {
                    realname[1] = realname[1].replace(realname[1][0], realname[1][0].toUpperCase())
                    realname[2] = realname[2].replace(realname[2][0], realname[2][0].toUpperCase())
                    realname[3] = realname[3].replace(realname[3][0], realname[3][0].toUpperCase())
                }
                realname = realname.join(" ")
            }
        }

        var para = createSystemElement(dir,dirname,realname,pngname);

        // 判断类型(应用工具/影音图像/办公娱乐/其他)
        var element = document.getElementById("app");
        var systemElement = document.getElementById("system");
        var officeElement = document.getElementById("office-entertainment-app");
        var otherElement = document.getElementById("other-app");
        var audioElement = document.getElementById("audio-video-app");
        // var para = document.createElement("div")
        // var node1 = document.createElement("br")
        // var node2 = document.createElement("img")
        // var node3 = document.createElement("span")
        // var apptext = document.createTextNode(realname)
        // var attr = document.createAttribute("onclick")
        // attr.value = "onclickButton('" + dirname + "')"
        // node3.appendChild(apptext)
        // node3.style.lineHeight = "10px"
        // node2.src = "file:////usr/share/kylin-user-guide/data/" + dir + "/" + dirname + "/" + pngname + ""
        // node2.alt = dirname
        // para.id = dirname;
        // para.style.marginLeft = "26px";
        // para.className = "system-app";
        // para.setAttributeNode(attr)
        // para.appendChild(node2)
        // para.appendChild(node1)
        // para.appendChild(node3)

        var para = createSystemElement(dir,dirname,realname,pngname);

        otherList = ['Services-and-Supports', "kylin-os"];
        audioList = ['kylin-music', 'pictures', 'kylin-video', 'kylin-recorder', 'kylin-screenshot', 'kylin-scanner', 'kylin-camera'];
        officeList = ['evolution', 'reversi', 'mines', 'tetravex', 'atril', 'kylin-calculator'];
        // systemList = ['kylin-kmre'];

        if (systemList.includes(dirname)) {
            // 固定系统组件顺序  --- 使用数组
            switch (dirname) 
            { 
                case 'kylin-os-installer.desktop':
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
                case 'kylin-kmre':
                    systemParaList[7] = para;
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

    }

    // alert(systemParaList)
    for(var i=0;i<systemParaList.length;i++){
        if(systemParaList[i]==null||systemParaList[i]==""){
            continue;
        }else{
            systemElement.appendChild(systemParaList[i]);
        }
    }
    // tt==null || tt==""
    // systemElement.appendChild(para);

    // 开机引导添加至最后
    // var para;
    // if (navigator.language == "zh-CN" || navigator.language == "zh" || navigator.language == "bo-CN") {
    //     para = createSystemElement('guide','kylin-new-function-introduction',
    //                                 '新功能介绍','kylin-new-function-introduction.png');
    // }else{
    //     para = createSystemElement('guide','kylin-new-function-introduction',
    //                                 'new function introduce','kylin-new-function-introduction.png')
    // }
    // otherElement.appendChild(para);
    
}

function getcontent(value){

    var valueList = value.split("|");
    var str = valueList[0];
    var mdPath = valueList[1];
    var mdDate = valueList[2];
    var mdData = "";
    valueList = valueList.slice(3);
    mdData = valueList.join("|");
    
    var m2ht = getDocTop(mdPath, mdData)
    var html = m2ht.html
    var hlist = m2ht.hlist
    var info = m2ht.info

    var hlist_str = "";
    // try {
            // window.guideWebkit.js_changeNavHighlight()
            // alert('')
            var iconName = mdPath.split("/").slice(0, mdPath.split("/").length - 2)
            iconName.push(iconName[iconName.length - 1] + ".png")
            iconName = iconName.join("/")
        // } catch (err) {
        //     console.log(err);
        //     var mdPath = window.guideWebkit.js_getIndexMdFilePath(str)
        //     var mdDate = window.guideWebkit.js_getIndexMdFileContent(mdPath)
        //     var mdData = window.guideWebkit.js_getFileSystemInformation(mdPath)
        //     var iconName = mdPath.split("/").slice(0, mdPath.split("/").length - 2)
        //     iconName.push(iconName[iconName.length - 1] + ".png")
        //     iconName = iconName.join("/")
        // }
        setTimeout(function () {
            document.getElementById("mainUI").style.display = "none";
            document.getElementById("pageContent").style.display = "inline";
            try {
                var m2ht = getDocTop(mdPath, mdData)
            } catch (err) {
                console.log(err);
                m2ht = getDocTop(mdPath, mdData);
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
                //console.log(hlist[i].type+"======"+name_text);
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
                    //console.log(name_text.length);
                    if (name_text.length >= 11) {
                        hlist_str += "<dt>\n" + "<a class='name2' onclick=onclickA('list_" + hlist[i].id + "')" + " id=" + "list_" + hlist[i].id + " href=\'#" + hlist[i].id + "\'>" + name_text + "</a> \n</dt>";
                    }
                    else {
                        hlist_str += "<dt>\n" + "<a class='name2' onclick=onclickA('list_" + hlist[i].id + "')" + " id=" + "list_" + hlist[i].id + " href=\'#" + hlist[i].id + "\'>" + name_text + "</a> \n</dt>";
                    }
                    //console.log(hlist_str);
                }
            }
            hlist_str = "<dl>" + hlist_str + "</dl>";
            //   console.log(hlist_str);
            str_name = str.replace(/-/g, "_");
            if (navigator.language == "zh-CN" || navigator.language == "zh" || navigator.language == "bo-CN") {
                var update = "更新时间："
                var goTop = "返回顶部"
            }
            else {
                var update = "Update time:"
                var goTop = "Go Top"
            }
            html_new = "<div class='top_title'>"
                // <img id='logo' src='"
                // +iconName
                // +"'>"           
                + "<div id='top_name'><h1><a name='paga_top'>" + info.title + "</a></h1>"
                + "<p  class='date'>"
                + update
                + mdDate
                + "</p>"
                + "</div></div>"
                + html
                + "<HR style='margin-top: 20px'>"
                + "<div class='paga_top' >"
                + "<img src='file:////usr/share/kylin-user-guide/data/icon-go-top.png' " + "/>"
                + "<a style='text-decoration: none;'onclick=onclickA('') href='#paga_top'>"
                + goTop
                + "</a>"
                + "</div>"

            // bridge.js_changeNavHighlight()

            document.getElementById("hlist").innerHTML = hlist_str
            document.getElementById("content").scrollTop = 0;
            document.getElementById("content").innerHTML = html_new


            $(window).scroll(scrollFun)
            onclickA("list_h1")

            var ScrollBarHeight = window.innerHeight;
        }, 1);
        // setTimeout(function () {
            
        // }, 1);
}

function getdirforapp(mdPath,info){
    // alert(navigator.language);
    if (navigator.language == "zh-CN" || navigator.language == "zh") {
        if (mdPath.search("/guide/") > 0) {
            dirforapp = "guide"
        }
        else if (mdPath.search("/guide-ubuntukylin/") > 0) {
            dirforapp = "guide-ubuntukylin"
        }
        if (info.search("帮助手册") < 0) {
            // alert("帮助手册")
            return info + "|" + dirforapp;
        }
        else {
            //console.log(info.title.slice(0,info.title.search("帮助手册")))
            return info.slice(0, info.search("帮助手册")) + "|" + dirforapp
        }
    } else {
        if (mdPath.search("/guide/") > 0) {
            dirforapp = "guide"
        }
        else if (mdPath.search("/guide-ubuntukylin/") > 0) {
            dirforapp = "guide-ubuntukylin"
        }
        return dirforapp
    }
}

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
    userinfo: "帐户信息|userinfo",
    biometrics: "登录选项|biometrics",
    cloudaccount: "云帐户|cloudaccount",
    date: "时间和日期|date",
    area: "区域语言|area",
    backup: "备份|backup",
    upgrade: "更新|upgrade",
    securitycenter: "安全中心|securitycenter",
    defaultapp: "默认应用|defaultapp",
    autoboot: "开机启动|autoboot",
    search: "全局搜索|搜索|search",
}

//手册dbus跳转页面js接口函数
function qt_jumpApp(appName) {
    // 判断是否为新功能介绍
    if("kylin-new-function-introduction" == appName){
       jumpIntroduce('ukui-new-function-introduction -nocheck')
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
                                // 多个相同标题，选中第一个
                                // onclickA("list_" + $heading.attr("id"));
                                // 滚动导航栏位置
                                aim = "list_" + $heading.attr("id");
                                // var t = $("#list_" + $heading.attr("id")).offset().top;// 获取需要跳转到标签的top值
                                // document.getElementById('hlist').scrollTop = t;
                                break;
                            }
                        }

                        if (aim.search("list_") >= 0) {
                            // document.getElementById('hlist').scrollTop = 0;
                            titleTOp = document.getElementById(aim).offsetTop;
                            // alert(mm);
                            // var titleTOp = $("#" + aim).offset().top;// 获取需要跳转到标签的top值
                            // alert(t)
                            document.getElementById('hlist').scrollTop = titleTOp;
                            document.getElementById(aim).click()
                        }
                        // } else {
                        //     var t = $("#" + aim).offset().top;// 获取需要跳转到标签的top值
                        //     document.getElementById('hlist').scrollTop = t;
                        //     $(window).scrollTop(t);// 跳转到指定位置
                        // }
                    }, 100);
                } else {
                    var target = html_list[list[1]]
                    if (target.search("list_") >= 0) {
                        document.getElementById(target).click()
                    } else {
                        var t = $("#" + target).offset().top;// 获取需要跳转到标签的top值
                        $(window).scrollTop(t);// 跳转到指定位置
                    }
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
    document.getElementById("mainUI").style.display = "none";
    document.getElementById("pageContent").style.display = "inline";
    bridge.js_getPageContent(str);
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
}

function getapp_name(qpp) {
    bridge.js_getIndexMdFileInfo(qpp);
}

// function addhtmlapp() {
//     var test = window.guideWebkit.js_getIntoFilename();
//     var systemName = window.guideWebkit.JudgmentSystem();
//     // if (systemName !== "Kylin") {
//     //     document.getElementById("support").style.display = "none";
//     //     document.getElementById("sup").style.display = "none";
//     //     document.getElementById("kylinOS").style.display = "none";
//     // }
//     // if(window.guideWebkit.js_getCpuArchitecture()){
//     //     document.getElementById("kydroid").style.display="none";
//     // }

//     var element = document.getElementById("app");
//     // 判断类型(应用工具/影音图像/办公娱乐/其他)
//     var systemElement = document.getElementById("system");
//     var officeElement = document.getElementById("office-entertainment-app");
//     var otherElement = document.getElementById("other-app");
//     var audioElement = document.getElementById("audio-video-app");
//     for (i = 0; i < test.length; i++) {
//         var dirname = test[i].slice(0, test[i].indexOf("|"))
//         var pngname = test[i].slice(test[i].indexOf("|") + 1, test[i].length)
//         if (dirname == "software-compatibility" || dirname == "technical-assistance"
//             || dirname == "kylin-user-desktop" || dirname == "ukui-control-center"
//             || dirname == "hot-key"
//             || dirname == "hardware-compatibility"
//             || dirname == "kylin-os-installer.desktop"
//             || dirname == "system-file-manager"
//             || dirname == "ukui-system-monitor"
//             || dirname == "utilities-terminal"
//             || dirname == "kylin-system-monitor"
//             || dirname == "peony"
//         ) {
//             continue
//         }
//         else {
//             if (navigator.language == "zh-CN" || navigator.language == "bo-CN") {
//                 var NameAndDir = getapp_name(dirname);
//                 var realname = NameAndDir.slice(0, NameAndDir.indexOf("|"));
//                 var dir = NameAndDir.slice(NameAndDir.indexOf("|") + 1, NameAndDir.length)
//                 // 这里对隐私与声明进行单独处理
//                 if ('银河麒麟最终用户使用许可协议和隐私政策声明' == realname) {
//                     realname = '隐私与声明';
//                 }
//             }
//             else {
//                 var NameAndDir = getapp_name(dirname);
//                 var realname = NameAndDir.slice(0, NameAndDir.indexOf("|"));
//                 var dir = NameAndDir.slice(NameAndDir.indexOf("|") + 1, NameAndDir.length)

//                 if (realname.search("-") >= 0)
//                     realname = (realname.split("-").map(item => item[0].toUpperCase() + item.slice(1, item.length))).join(" ")
//                 realname = (realname.split(" ").map(item => item[0].toUpperCase() + item.slice(1, item.length))).join(" ")
//             }

//             var para = createSystemElement(dir,dirname,realname,pngname);

//             otherList = ['Services-and-Supports', "kylin-os"];
//             audioList = ['kylin-music', 'pictures', 'kylin-video', 'kylin-recorder', 'kylin-screenshot', 'kylin-scanner', 'kylin-camera'];
//             officeList = ['evolution', 'reversi', 'mines', 'tetravex', 'atril', 'kylin-calculator'];
//             // systemList = ['kylin-kmre'];
//             // systemList = ['kylin-user-desktop','ukui-control-center','hot-key'];

//             // if (systemList.includes(dirname)){
//             //     systemElement.appendChild(para);
//             // } else 
//             if (otherList.includes(dirname)) {
//                 otherElement.appendChild(para);
//             } else if (audioList.includes(dirname)) {
//                 audioElement.appendChild(para);
//             } else if (officeList.includes(dirname)) {
//                 officeElement.appendChild(para);
//             } else {
//                 element.appendChild(para);
//             }

//         }
//     }
//     // 开机引导添加至最后
//     var para;
//     if (navigator.language == "zh-CN" || navigator.language == "bo-CN") {
//         para = createSystemElement('guide','kylin-new-function-introduction',
//                                     '新功能介绍','kylin-new-function-introduction.png');
//     }else{
//         para = createSystemElement('guide','kylin-new-function-introduction',
//                                     'new function introduce','kylin-new-function-introduction.png')
//     }
//     para
//     otherElement.appendChild(para);
// }

function addhtmlapp(info) {
    var test = info.split(",");
    var infoList = new Array();
    for (i = 0; i < test.length; i++) {
        var dirname = test[i].slice(0, test[i].indexOf("|"))
        var pngname = test[i].slice(test[i].indexOf("|") + 1, test[i].length)
        if (dirname == "software-compatibility" || dirname == "technical-assistance"
            // || dirname == "kylin-user-desktop" || dirname == "ukui-control-center"
            // || dirname == "hot-key"
            || dirname == "hardware-compatibility"
            // || dirname == "kylin-os-installer.desktop"
            // || dirname == "system-file-manager"
            // || dirname == "ukui-system-monitor"
            // || dirname == "utilities-terminal"
            // || dirname == "kylin-system-monitor"
            // || dirname == "peony"
        ) {
            continue
        } else {
            // 聚合所有的信息
            infoList.push(dirname+"|"+pngname);
            // getapp_name(dirname+"|"+pngname);
        }
    }
    getapp_name(infoList.join())
}

function createSystemElement(dir,dirname,realname,pngname){
    var para = document.createElement("div")
    var node1 = document.createElement("br")
    var node2 = document.createElement("img")
    var node3 = document.createElement("div")
    var apptext = document.createTextNode(realname)
    var attr = document.createAttribute("onclick")
    if('kylin-new-function-introduction' == dirname){
        attr.value = "jumpIntroduce('ukui-new-function-introduction -nocheck')";
    }else{
        attr.value = "onclickButton('" + dirname + "')"
    }
    node3.appendChild(apptext)
    node3.style.lineHeight = "22px"
    node3.style.paddingLeft = "15px"
    node3.style.paddingRight = "15px"
    node2.src = "file:////usr/share/kylin-user-guide/data/" + dir + "/" + dirname + "/" + pngname + ""
    node2.alt = dirname
    para.id = dirname;
    para.style.marginLeft = "26px";
    para.className = "system-app";
    para.setAttributeNode(attr)
    para.appendChild(node2)
    para.appendChild(node1)
    para.appendChild(node3)
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

var icon_theme
function switchTheme(themeName){
    setTimeout(function () {
        icon_theme = themeName;
        apps = document.getElementsByClassName('system-app');
        for(let i = 0 ; i < apps.length ; i++){
             //alert(apps[i].children[0].src);
            // alert('file:////usr/share/icons/'+icon_theme+'/scalable/apps/'+apps[i].id+'.svg');
            var theme_src = 'file:////usr/share/icons/'+icon_theme+'/scalable/apps/'+apps[i].id.trim()+'.svg';
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
    //alert()		
    imgObj.onload = function () {
        successCallback(img_src,app);
    }
    imgObj.onerror = function () {
        //successCallback(flag);
		//alert(1);
		errorCallback(app);
    }
}

function successCallback(img_src,app){
    app.children[0].src = img_src;   
}

function errorCallback(app){
	//app.children[0].src = "file:////usr/share/kylin-user-guide/data/" + apps[i].id.trim() + "/" + apps[i].id.trim() + "/" + apps[i].id.trim() + ".png";

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
    if (navigator.language == "zh-CN" || navigator.language == "zh" || navigator.language == "bo-CN") {
        if ("2" == style)
            window.location.href = "index-ubuntukylin_black.html"
        else
            window.location.href = "index-ubuntukylin.html"
    }
    else {
        if ("2" == style)
            window.location.href = "index-ubuntukylin_en_US_black.html"
        else
            window.location.href = "index-ubuntukylin_en_US.html"
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

    bridge.js_changeNavHighlight()

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
        $("#" + old_str).removeAttr("style")
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
            hlist.push({ "id": id, "text": text, "type": type });
            //console.log(id);
            //console.log(text);
            //console.log(type);
        }
        return "<" + type + " id=" + id + " text=" + text + " class='html_h5'>" + text + "</" + type + ">\n";
    };

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
            return '<h5  style="text-align: left;font-family: Noto Sans SC; font-size: 12px;color=#333333;"><img style="max-height:98%;max-width:98%;"  src=\"' + hrefX2 + '\" data-src=\"' + href + '\" alt=' + text + '/><br> ' + text + '</h5>';
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

    renderer.link = function (href, title, text) {
        return '<a href="" onclick="bridge.js_openUrl(\''+href+'\')" target="_blank">'+text+'</a>';
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
window.onresize = function () {
    // var windowsWidth = window.innerHeight;
    // // 左侧导航栏宽度
    // oTop = document.getElementById("hlist");
    // oTop.style.width = 120 / windowsWidth * 100 + "%";
    // // 右侧内容宽度
    // oBottom = document.getElementById("content");
    // oBottom.style.width = (windowsWidth - 120) / windowsWidth * 100 + "%";
    // // alert(windowsWidth);
    // // alert((windowsWidth - 120) / windowsWidth * 100);
    // alert(oTop.style.width);
    // alert(oBottom.style.width);
    // 侧边栏拖动功能暂时停用
    // if (normalWidth > window.innerWidth || normalHeight < window.innerHeight) {
    //     //这里是发生放大缩小之后执行的方法
    //     // 重绘
    //     document.getElementBy
    // Id("content").innerHTML += "<div id='line' onmousedown='dragScrollBar()' style='position:fixed;top:0;left:24%;height:" + window.innerHeight + "px;width:4px;background:#F5F5F5;opacity:0;cursor:w-resize;'></div>";
    //     // 设置事件
    //     dragScrollBar();
    // }

    resizeEvent();

}

function resizeEvent() {
    // 优化窗口放大时，左边宽度
    var contentWidth = document.documentElement.offsetWidth;
    var leftNavWidth = 260;
    document.getElementById("content").style.width = (contentWidth - leftNavWidth) + "px";
    // alert((contentWidth - 240) + "px");
}

function jumpIntroduce(cmd){
    if(cmd.length != 0)
        bridge.js_executeCommand(cmd);
}

//强调色默认蓝色 #3790FA
highColor = ""
function changeNavHighlight(color){
    if("" != color){
        highColor = color;
        $("#" + old_str).css({ "backgroundColor": highColor, "color": "white" })
    }
}
isTabletmode="0"
function appSizeChange(tabletmode){
	isTabletmode=tabletmode
    if("1"==tabletmode){
        // 平板模式
        $("div.system-app").attr("style","width: 160px; height: 160px;");
        $("div.system-app img").attr("style","width: 96px; height: 96px;");
    }else{
        $("div.system-app").attr("style","width: 120px; height: 120px;");
        $("div.system-app img").attr("style","width: 64px; height: 64px;");
    }
}