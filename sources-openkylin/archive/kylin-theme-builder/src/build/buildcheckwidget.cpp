#include "buildcheckwidget.h"

buildCheckWidget::buildCheckWidget(QWidget *parent)
    : QWidget(parent)
{
}

// 编译前拷贝必须文件
void buildCheckWidget::buildCopy(){

    m_debSourceFormat = ":/resource/debian/source/format";
    m_debRules = ":/resource/debian/rules";
    m_debCopyRight = ":/resource/debian/copyright";
    m_debCopyPostinst = ":/resource/debian/postinst";
    m_debCopyPostrm = ":/resource/debian/postrm";

    QString m_debPath = QDir::homePath() + "/.cache/theme-build/" + FileProcess::g_date + "/src";
    QDir m_debSource = m_debPath +"/debian/source";
//    qDebug()<<m_debSource;
    m_debSource.mkdir(m_debSource.absolutePath());

    if (QFile::copy(m_debSourceFormat,m_debPath+"/debian/source/format") &&
        QFile::copy(m_debRules,m_debPath+"/debian/rules") &&
        QFile::copy(m_debCopyRight,m_debPath+"/debian/copyright") &&
        QFile::copy(m_debCopyPostinst,m_debPath+"/debian/postinst") &&
        QFile::copy(m_debCopyPostrm,m_debPath+"/debian/postrm")){
        qDebug()<<"COPY SUCCESS";

    } else {
        qDebug()<<"COPY Failed";
        qDebug()<<QFile::copy(m_debSourceFormat,m_debPath+"/debian/source/format") ;
        qDebug()<<QFile::copy(m_debRules,m_debPath+"/debian/rules") ;
        qDebug()<<QFile::copy(m_debCopyRight,m_debPath+"/debian/copyright");
        qDebug()<<QFile::copy(m_debCopyPostinst,m_debPath+"/debian/postinst");
        qDebug()<<QFile::copy(m_debCopyPostrm,m_debPath+"/debian/postrm");
    }

}

//全局主题打包
void buildCheckWidget::buildForGlobal(){
    buildForIcon();
    buildForCursor();
    buildForPlymouth();
    buildForGrub();
    m_option = new gsettingOption;

    //拷贝default.json
    QFile::copy(m_debPath+"/globalTheme/json.txt",m_debPath+"/globalTheme/"+FileProcess::g_themeENName+".json");
    //修改conf
    QString m_themeConf = m_debPath+"/globalTheme/theme.conf";

    QString m_themeName = "Name\[zh_CN\]=寻光";
    QString m_themeNewName = "Name\[zh_CN\]="+FileProcess::g_themeENName;

    QFile file(m_themeConf);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        qDebug() << "Failed to open file.";
    }
    QTextStream in(&file);
    QString fileContent = in.readAll();

    // 查找目标字段并替换
    int index1 = fileContent.indexOf(m_themeName);

    if (index1 != -1) {
        fileContent.replace(index1, m_themeName.length(), m_themeNewName);
        file.resize(0);
        QTextStream out(&file);
        out << fileContent;
        qDebug() << "Name field updated.";
    } else {
        qDebug() << "Name field not found.";
    }

    QString m_themeWallpaper = "wallPaperPath=/usr/share/backgrounds/1-openkylin.jpg";
    QString m_themeNewWallpaper;
    if(FileProcess::g_isSetup){
        m_themeNewWallpaper= "wallPaperPath=/usr/share/backgrounds/1-openkylin.jpg";
    }else{
        m_themeNewWallpaper= "wallPaperPath=/usr/share/backgrounds/kylin-theme-builder-focal-"+FileProcess::g_themeENName+".jpg";
    }

//    QDir directory(QDir::homePath() + "/.cache/theme-build/" + FileProcess::g_themeENName + "/src/globalTheme");
//    QStringList fileList = directory.entryList(QDir::Files);
//    if (fileList.contains("1-openkylin.jpg")) {
//        m_themeNewWallpaper = "wallPaperPath=/usr/share/backgrounds/kylin-theme-builder-focal-"+FileProcess::g_themeENName+".jpg";
//    }

    int index2 = fileContent.indexOf(m_themeWallpaper);
    if (index2 != -1) {
        fileContent.replace(index2, m_themeWallpaper.length(), m_themeNewWallpaper);
        file.resize(0);
        QTextStream out(&file);
        out << fileContent;
        qDebug() << "Name field updated.";
    } else {
        qDebug() << "Name field not found.";
    }

    QString m_themeWidgetTheme = "widgetStyleName=default";
    QString m_themeNewidgetTheme = "widgetStyleName="+FileProcess::g_themeENName;

    int index3 = fileContent.indexOf(m_themeWidgetTheme);
    if (index3 != -1) {
        fileContent.replace(index3, m_themeWidgetTheme.length(), m_themeNewidgetTheme);
        file.resize(0);
        QTextStream out(&file);
        out << fileContent;
        qDebug() << "Name field updated.";
    } else {
        qDebug() << "Name field not found.";
    }

    QString m_themeIconTheme = "iconThemeName=ukui-icon-theme-default";
    QString m_themeNewIconTheme = "iconThemeName="+FileProcess::g_themeENName;

    int index4 = fileContent.indexOf(m_themeIconTheme);
    if (index4 != -1) {
        fileContent.replace(index4, m_themeIconTheme.length(), m_themeNewIconTheme);
        file.resize(0);
        QTextStream out(&file);
        out << fileContent;
        qDebug() << "Name field updated.";
    } else {
        qDebug() << "Name field not found.";
    }

    QString m_themeCursorTheme = "cursorThemeName=dark-sense";
    QString m_themeNewCursoTheme = "cursorThemeName="+FileProcess::g_themeENName+"-cursor";

    int index5 = fileContent.indexOf(m_themeCursorTheme);
    if (index5 != -1) {
        fileContent.replace(index5, m_themeCursorTheme.length(), m_themeNewCursoTheme);
        file.resize(0);
        QTextStream out(&file);
        out << fileContent;
        qDebug() << "Name field updated.";
    } else {
        qDebug() << "Name field not found.";
    }
    int startIndex = fileContent.indexOf("accentRole=");
    if (startIndex != -1) {
        int lineStartIndex = fileContent.lastIndexOf('\n', startIndex) + 1;
        int lineEndIndex = fileContent.indexOf('\n', startIndex);

        if (lineEndIndex == -1) {
            lineEndIndex = fileContent.size();
        }


        QString lineContent = fileContent.mid(lineStartIndex, lineEndIndex - lineStartIndex).trimmed();
        lineContent.remove("accentRole=");
        m_option->accentRole = lineContent;
    }

    initSetupShell();
    initSetupSchemas();
    file.close();

    //写入postinst文件
    QFile(m_debPath+"/debian/postinst").setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
                                                       QFile::ReadUser | QFile::WriteUser | QFile::ExeUser |
                                                       QFile::ReadGroup | QFile::WriteGroup | QFile::ExeGroup |
                                                       QFile::ReadOther | QFile::WriteOther | QFile::ExeOther);
    QFile m_postinst(m_debPath+"/debian/postinst");

    QTextStream m_gsettinginstStream(&m_postinst);
    if (m_postinst.open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text)) {
        m_postinst.seek(m_postinst.size());

        // 在最后一行写入文本
        QTextStream postinstStream(&m_postinst);

        //如果是默认安装，则将配置文件放到各个用户下.config/ukui/目录下
        if(FileProcess::g_isSetup){

            postinstStream << endl;
            //通用文件，如果存在，则跳过安装，不需要再次安装，避免装包重复报错
            postinstStream << "source_file1=\"/usr/share/config/globaltheme/"+FileProcess::g_themeENName+"/theme-builder-change.conf\""<< endl;
            postinstStream << "source_file2=\"/usr/share/config/globaltheme/"+FileProcess::g_themeENName+"/kylin-theme-builder-change.desktop\""<< endl;
            postinstStream << "source_file3=\"/usr/share/config/globaltheme/"+FileProcess::g_themeENName+"/40_kylin-theme-builder.gschema.override\""<< endl;
            postinstStream << "source_file4=\"/usr/share/config/globaltheme/"+FileProcess::g_themeENName+"/kylin-theme-builder-change\""<< endl;
            postinstStream << "source_file5=\"/usr/share/config/globaltheme/"+FileProcess::g_themeENName+"/kylin-theme-builder.xml\""<< endl;

            postinstStream << "target_path1=\"/usr/share/config/globaltheme/\""<< endl;
            postinstStream << "target_path2=\"/etc/xdg/autostart/\""<< endl;
            postinstStream << "target_path3=\"/usr/share/glib-2.0/schemas/\""<< endl;
            postinstStream << "target_path4=\"/usr/bin/\""<< endl;
            postinstStream << "target_path5=\"/usr/share/ukui-background-properties/\""<< endl;

            postinstStream << "[ ! -f \"$target_path1/theme-builder-change.conf\" ] && cp \"$source_file1\" \"$target_path1\"" << endl;
            postinstStream << "[ ! -f \"$target_path2/kylin-theme-builder-change.desktop\" ] && cp \"$source_file2\" \"$target_path2\" && chmod +x \"$target_path2/kylin-theme-builder-change.desktop\"" << endl;
            postinstStream << "[ ! -f \"$target_path3/40_kylin-theme-builder.gschema.override\" ] && cp \"$source_file3\" \"$target_path3\"" << endl;
            postinstStream << "[ ! -f \"$target_path4/kylin-theme-builder-change\" ] && cp \"$source_file4\" \"$target_path4\" && chmod +x \"$target_path4/kylin-theme-builder-change\"" << endl;
            postinstStream << "[ ! -f \"$target_path5/kylin-theme-builder.xml\" ] && cp \"$source_file5\" \"$target_path5\"" << endl;

            postinstStream << endl << "CONFIG_FILE=\"/usr/share/config/globaltheme/"+FileProcess::g_themeENName+"/theme-builder-change.conf\"" << endl;
            postinstStream << "usernames=$(grep '/bin/bash$' /etc/passwd | cut -d: -f1)" << endl;
            postinstStream << "filtered_usernames=$(echo \"$usernames\" | awk '!/root/')" << endl;
            postinstStream << "for username in $filtered_usernames; do" << endl;
            postinstStream << "    echo \"Handling users: $username\"" << endl;
            postinstStream << "    if [ ! -d \"/home/$username/.config/ukui/\" ]; then" << endl;
            postinstStream << "        echo \"A directory has been created: /home/$username/.config/ukui/\"" << endl;
            postinstStream << "    else" << endl;
            postinstStream << "        echo \"The directory already exists: /home/$username/.config/ukui/\"" << endl;
            postinstStream << "    fi" << endl;
            postinstStream << "    cp \"$CONFIG_FILE\" \"/home/$username/.config/ukui/theme-builder-change.conf\"" << endl;
            postinstStream << "done" << endl;

            postinstStream << "rm \"$CONFIG_FILE\"" << endl;

            //将1-warty-paper.jpg改名为2-warty-paper.jpg
            postinstStream << "if [ ! -f \"/usr/share/backgrounds/2-warty-final-ubuntukylin.jpg\" ]; then" << endl;
            postinstStream << "mv /usr/share/backgrounds/1-openkylin.jpg /usr/share/backgrounds/2-warty-final-ubuntukylin.jpg" << endl;
            postinstStream << "else" << endl;
            postinstStream << "rm /usr/share/backgrounds/1-openkylin.jpg" << endl;
            postinstStream << "fi" << endl;

            postinstStream << "cp /usr/share/config/globaltheme/"+FileProcess::g_themeENName+"/1-openkylin.jpg /usr/share/backgrounds/1-openkylin.jpg" << endl;
            postinstStream << "if [ -f \""<<QDir::homePath()<<  "/.config/ukui/wallpaper.xml\"]; then"<< endl;
            postinstStream << "rm " <<QDir::homePath() + "/.config/ukui/wallpaper.xml"<< endl;
            postinstStream << "fi" << endl;

        }else{
            qDebug()<<"g_isSetup is false";
            //非默认的情况，壁纸需要新添加一个壁纸，且添加一个新的xml文件
            postinstStream << endl<< "cp /usr/share/config/globaltheme/"+FileProcess::g_themeENName+"/1-openkylin.jpg /usr/share/backgrounds/kylin-theme-builder-focal-"<<FileProcess::g_themeENName<<".jpg"<< endl;
            //xml文件修改
            postinstStream << "cp /usr/share/config/globaltheme/"+FileProcess::g_themeENName+"/kylin-theme-builder-focal.xml /usr/share/ukui-background-properties/kylin-theme-builder-focal-"<<FileProcess::g_themeENName<<".xml"<<endl;
            postinstStream << "if [ -f \""<<QDir::homePath()<<  "/.config/ukui/wallpaper.xml\" ]; then"<< endl;
            postinstStream << "rm " <<QDir::homePath() + "/.config/ukui/wallpaper.xml"<< endl;
            postinstStream << "fi" << endl;
        }

        m_gsettinginstStream.flush();
        m_postinst.close();

    } else {
        qDebug()<< "can't open!!!!!!!!!";
    }
    QFile(m_debPath+"/debian/postrm").setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
                                                       QFile::ReadUser | QFile::WriteUser | QFile::ExeUser |
                                                       QFile::ReadGroup | QFile::WriteGroup | QFile::ExeGroup |
                                                       QFile::ReadOther | QFile::WriteOther | QFile::ExeOther);
    QFile m_postrm(m_debPath+"/debian/postrm");

    QTextStream m_postrmStreamall(&m_postrm);
    if (m_postrm.open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text)) {
        m_postrm.seek(m_postrm.size());
        QTextStream postrmStream(&m_postrm);
        if(FileProcess::g_isSetup){
            postrmStream<<endl;
            postrmStream << "source_file1=\"/usr/share/config/globaltheme/theme-builder-change.conf\""<< endl;
            postrmStream << "source_file2=\"/etc/xdg/autostart/kylin-theme-builder-change.desktop\""<< endl;
            postrmStream << "source_file3=\"/usr/share/glib-2.0/schemas/40_kylin-theme-builder.gschema.override\""<< endl;
            postrmStream << "source_file4=\"/usr/bin/kylin-theme-builder-change\""<< endl;
            postrmStream << "source_file5=\"/usr/share/ukui-background-properties/wallpaper/kylin-theme-builder.xml\""<< endl;
            postrmStream<<endl;
            postrmStream << "[ -f \"$source_file1\" ] && rm \"$source_file1\"" << endl;
            postrmStream << "[ -f \"$source_file2\" ] && rm \"$source_file2\"" << endl;
            postrmStream << "[ -f \"$source_file3\" ] && rm \"$source_file3\"" << endl;
            postrmStream << "[ -f \"$source_file4\" ] && rm \"$source_file4\"" << endl;
            postrmStream << "[ -f \"$source_file5\" ] && rm \"$source_file5\"" << endl;

            postrmStream << endl << "CONFIG_FILE=\"/usr/share/config/globaltheme/theme-builder-change.conf\"" << endl;
            postrmStream << "usernames=$(grep '/bin/bash$' /etc/passwd | cut -d: -f1)" << endl;
            postrmStream << "filtered_usernames=$(echo \"$usernames\" | awk '!/root/')" << endl;
            postrmStream << "for username in $filtered_usernames; do" << endl;
            postrmStream << "    echo \"Handling users: $username\"" << endl;
            postrmStream << "    if [ ! -f \"/home/$username/.config/ukui/theme-builder-change.conf\" ]; then" << endl;
            postrmStream << "        rm \"/home/$username/.config/ukui/ukui/theme-builder-change.conf\"" << endl;
            postrmStream << "    fi" << endl;
            postrmStream << "done" << endl;

            postrmStream << "if [ ! -f \"/usr/share/backgrounds/2-warty-final-ubuntukylin.jpg\" ]; then" << endl;
            postrmStream << "cp /usr/share/backgrounds/2-warty-final-ubuntukylin.jpg /usr/share/backgrounds/1-openkylin.jpg" << endl;
            postrmStream << "rm /usr/share/backgrounds/2-warty-final-ubuntukylin.jpg" << endl;
            postrmStream << "fi" << endl;

            postrmStream << "if [ -f \""<<QDir::homePath()<<  "/.config/ukui/wallpaper.xml\" ]; then"<< endl;
            postrmStream << "rm " <<QDir::homePath() + "/.config/ukui/wallpaper.xml"<< endl;
            postrmStream << "fi" << endl;

        } else {
            //仅删除focal
            postrmStream << "if [ -f /usr/share/backgrounds/kylin-theme-builder-focal-" <<FileProcess::g_themeENName<<".jpg ]; then"<< endl;
            postrmStream << "rm /usr/share/backgrounds/kylin-theme-builder-focal-"<<FileProcess::g_themeENName<<".jpg"<< endl;
            postrmStream << "fi" << endl;

            postrmStream << "if [ -f /usr/share/ukui-background-properties/kylin-theme-builder-focal-" <<FileProcess::g_themeENName<<".xml ]; then"<< endl;
            postrmStream << "rm /usr/share/ukui-background-properties/kylin-theme-builder-focal-" <<FileProcess::g_themeENName<<".xml"<< endl;
            postrmStream << "fi" << endl;
        }



        m_postrmStreamall.flush();
        m_postrm.close();
    } else {
        qDebug()<< "can't open!!!!!!!!!";
    }
}

//图标打包
void buildCheckWidget::buildForIcon(){

    //创建打包目录
    QString m_buildIconThemePath = m_debPath+"/iconTheme/"+FileProcess::g_themeENName;
    QDir m_buildIconThemeDir = m_buildIconThemePath;
    m_buildIconThemeDir.mkdir(m_buildIconThemeDir.absolutePath());

    QFile m_iconIndex(m_buildIconThemePath+"/index.theme");
    QTextStream m_iconIndexStream(&m_iconIndex);
    //写入icon index.theme文件
    if (m_iconIndex.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
//        qDebug()<< m_iconIndex;
        m_iconIndexStream << QStringLiteral("[Icon Theme]");
        m_iconIndexStream << endl << QStringLiteral("Name=")+ FileProcess::g_themeENName;
        m_iconIndexStream << endl << QStringLiteral("Comment=Icon theme for UKUI ,from moka.");
        m_iconIndexStream << endl << QStringLiteral("Inherits=ukui-icon-theme-default,gnome,hicolor,Adwaita");
        m_iconIndexStream << endl;

        m_iconIndexStream.flush();
        m_iconIndex.close();
    }

    //mimetype写入icon index.theme文件
    if (m_iconIndex.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        m_iconIndexStream << endl << QStringLiteral("# Directory list");
        m_iconIndexStream << endl << QStringLiteral("Directories=16x16/apps,24x24/apps,32x32/apps,48x48/apps,"
                                                    "64x64/apps,96x96/apps,128x128/apps,256x256/apps,scalable/apps"
                                                    "16x16/devices,24x24/devices,32x32/devices,48x48/devices,"
                                                    "64x64/devices,96x96/devices,128x128/devices,256x256/devices,scalable/devices"
                                                    "16x16/places,24x24/places,32x32/places,48x48/places,"
                                                    "64x64/places,96x96/places,128x128/places,256x256/places,scalable/places"
                                                    "16x16/mimetypes,24x24/mimetypes,32x32/mimetypes,48x48/mimetypes,"
                                                    "64x64/mimetypes,96x96/mimetypes,128x128/mimetypes,256x256/mimetypes,scalable/mimetypes");
        m_iconIndexStream << endl;
        m_iconIndexStream << endl << QStringLiteral("[scalable/mimetypes]");
        m_iconIndexStream << endl << QStringLiteral("Context=Mimetypes");
        m_iconIndexStream << endl << QStringLiteral("Size=16");
        m_iconIndexStream << endl << QStringLiteral("MinSize=16");
        m_iconIndexStream << endl << QStringLiteral("MaxSize=256");
        m_iconIndexStream << endl << QStringLiteral("Type=Scalable");
        m_iconIndexStream << endl;

        int array[] = {16, 24, 32, 48, 64, 96, 128, 256};
        int length = sizeof(array) / sizeof(array[0]);

        for (int i = 0; i < length; i++) {
//            qDebug()<< m_iconIndex;
            m_iconIndexStream << endl << QStringLiteral("[")+QString::number(array[i])+QStringLiteral("x")+QString::number(array[i])+QStringLiteral("/mimetypes]");
            m_iconIndexStream << endl << QStringLiteral("Context=Mimetypes");
            m_iconIndexStream << endl << QStringLiteral("Size=")+QString::number(array[i]);
            m_iconIndexStream << endl << QStringLiteral("Type=Fixed");
            m_iconIndexStream << endl;
        }
        m_iconIndexStream.flush();
        m_iconIndex.close();
    }

    // 创建对应目录，拷贝mimetypes文件
    QString m_iconThemeScalablePath = m_buildIconThemePath + "/scalable";
    QString m_iconThemeScalableMimePath = m_iconThemeScalablePath + "/mimetypes";
    QDir m_buildSystemIcondirDir = m_iconThemeScalablePath;
    QDir m_iconThemeScalableMimeDir = m_iconThemeScalableMimePath;
    m_buildSystemIcondirDir.mkdir(m_buildSystemIcondirDir.absolutePath());
    m_iconThemeScalableMimeDir.mkdir(m_iconThemeScalableMimeDir.absolutePath());
    QFile::copy(":/resource/mimetypes/application-x-desktop.svg",m_iconThemeScalableMimePath+"/application-x-desktop.svg");

    // 遍历scalable/mimetype目录
    QStringList m_mimeIconNameFilters;
    m_mimeIconNameFilters << "*.svg";
    m_iconThemeScalableMimeDir.setNameFilters(m_mimeIconNameFilters);

    QFileInfoList m_svgMimeFileInfoList = m_iconThemeScalableMimeDir.entryInfoList(QDir::Files);
    foreach (QFileInfo m_svgMimeFileInfo, m_svgMimeFileInfoList) {
        //处理svg转png
        //遍历、创建尺寸目录，并拷贝生成png
        int array[] = {16, 24, 32, 48, 64, 96, 128, 256};
        int length = sizeof(array) / sizeof(array[0]);

        for (int i = 0; i < length; i++) {

            // 初始化一个 SVG 渲染器
            QSvgRenderer svgRenderer(m_svgMimeFileInfo.absoluteFilePath());

            // 创建一个和 SVG 同样大小的 QImage 对象，使用透明背景
            QImage image(QSize(array[i],array[i]), QImage::Format_ARGB32);
            image.fill(Qt::transparent);

            // 创建一个 QPainter 对象，用于在 QImage 对象上绘图
            QPainter painter(&image);
            // 使用 SVG 渲染器在 QPainter 上渲染图像
            svgRenderer.render(&painter, QRectF(0, 0, array[i], array[i]));
            //生成尺寸目录
            QString m_imageDirThemeSizePath = m_buildIconThemePath +"/" + QString::number(array[i]) + "x" + QString::number(array[i]);
            QString m_imageDirThemeMimePath = m_imageDirThemeSizePath+ "/mimetypes";
            QDir m_imageThemeSizeDir = m_imageDirThemeSizePath;
            QDir m_imageDirThemeMimeDir = m_imageDirThemeMimePath;
            m_imageThemeSizeDir.mkdir(m_imageThemeSizeDir.absolutePath());
            m_imageDirThemeMimeDir.mkdir(m_imageDirThemeMimeDir.absolutePath());

            // 将 QImage 对象保存为 PNG 文件
            QString m_fileName = m_svgMimeFileInfo.baseName();
            image.save(m_imageDirThemeMimePath+"/"+m_fileName+".png");
        }
    }

    // 增加图标种类目录在这里增加数组项
    QStringList m_iconTypeList;
    m_iconTypeList << "appicon" << "systemicon" << "deviceicon" << "placeicon";

    // 遍历QStringList
    foreach (const QString &m_iconType, m_iconTypeList) {
//        qDebug() << m_iconType;

        QString m_type;
        QString m_typeContext;
        if (m_iconType == "appicon" ||m_iconType == "systemicon"){
            m_type = "apps";
            m_typeContext = "Applications";
        } else if(m_iconType == "deviceicon"){
            m_type = "devices";
            m_typeContext = "Devices";
        } else if(m_iconType == "placeicon"){
            m_type = "places";
            m_typeContext = "Places";
        }

        QString m_buildIconPath = QDir::homePath() + "/.cache/theme-build/" + FileProcess::g_date + "/src/iconTheme/"+m_iconType+"/";
        QDir m_buildIconDir = m_buildIconPath;

        // 设置过滤器，仅获取 .svg 文件
        QStringList m_iconNameFilters;
        m_iconNameFilters << "*.svg";
        m_buildIconDir.setNameFilters(m_iconNameFilters);

        // 获取源目录中的文件列表
        QStringList m_iconfiles = m_buildIconDir.entryList(QDir::Files);

        if(!m_iconfiles.isEmpty()){

            //app类型写入icon index.theme文件
            if (m_iconIndex.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
                m_iconIndexStream << endl;
//                m_iconIndexStream << endl << QStringLiteral("# Applications");
                m_iconIndexStream << endl << QStringLiteral("#")+m_type;
                m_iconIndexStream << endl << QStringLiteral("[scalable/")+m_type+QStringLiteral("]");
                m_iconIndexStream << endl << QStringLiteral("Context=")+m_typeContext;
                m_iconIndexStream << endl << QStringLiteral("Size=16");
                m_iconIndexStream << endl << QStringLiteral("MinSize=16");
                m_iconIndexStream << endl << QStringLiteral("MaxSize=256");
                m_iconIndexStream << endl << QStringLiteral("Type=Scalable");
                m_iconIndexStream << endl;

                int array[] = {16, 24, 32, 48, 64, 96, 128, 256};
                int length = sizeof(array) / sizeof(array[0]);

                for (int i = 0; i < length; i++) {
//                    qDebug()<< m_iconIndex;
                    m_iconIndexStream << endl << QStringLiteral("[")+QString::number(array[i])+QStringLiteral("x")+
                                         QString::number(array[i])+QStringLiteral("/")+m_type+QStringLiteral("]");
                    m_iconIndexStream << endl << QStringLiteral("Context=")+m_typeContext;
                    m_iconIndexStream << endl << QStringLiteral("Size=")+QString::number(array[i]);
                    m_iconIndexStream << endl << QStringLiteral("Type=Fixed");
                    m_iconIndexStream << endl;
                }
                m_iconIndexStream.flush();
                m_iconIndex.close();
            }

            // 创建app目录
            QString m_iconThemeScalableTypePath = m_iconThemeScalablePath + "/" +m_type;
            QDir m_iconThemeScalableTypeDir = m_iconThemeScalableTypePath;
            m_iconThemeScalableTypeDir.mkdir(m_iconThemeScalableTypeDir.absolutePath());

            // 遍历appicon文件列表并拷贝到目标目录apps
            foreach (const QString &m_iconFile, m_iconfiles) {
                QString sourceFilePath = m_buildIconDir.filePath(m_iconFile);
                QString destinationFilePath = m_iconThemeScalableTypeDir.filePath(m_iconFile);

                QFile sourceFile(sourceFilePath);
                QFile destinationFile(destinationFilePath);

                // 如果源文件可以打开，并且目标文件可以创建
                if (sourceFile.open(QIODevice::ReadOnly) && destinationFile.open(QIODevice::WriteOnly)) {
                    // 从源文件读取数据
                    QByteArray data = sourceFile.readAll();

                    // 将数据写入目标文件
                    destinationFile.write(data);

                    // 关闭文件
                    sourceFile.close();
                    destinationFile.close();
                }
            }

            QStringList m_svgFilters;
            m_svgFilters << "*.svg"; // 指定扩展名为 .svg

            m_iconThemeScalableTypeDir.setNameFilters(m_svgFilters);
            m_iconThemeScalableTypeDir.setFilter(QDir::Files | QDir::NoSymLinks);

            QFileInfoList m_svgFileInfoList = m_iconThemeScalableTypeDir.entryInfoList();

            // 遍历获取文件名
            foreach (QFileInfo m_svgFileInfo, m_svgFileInfoList) {

                //处理svg转png
                //遍历、创建尺寸目录，并拷贝生成png
                int array[] = {16, 24, 32, 48, 64, 96, 128, 256};
                int length = sizeof(array) / sizeof(array[0]);

                for (int i = 0; i < length; i++) {

                    // 初始化一个 SVG 渲染器
                    QSvgRenderer svgRenderer(m_svgFileInfo.absoluteFilePath());

                    // 创建一个和 SVG 同样大小的 QImage 对象，使用透明背景
                    QImage image(QSize(array[i],array[i]), QImage::Format_ARGB32);
                    image.fill(Qt::transparent);

                    // 创建一个 QPainter 对象，用于在 QImage 对象上绘图
                    QPainter painter(&image);
                    // 使用 SVG 渲染器在 QPainter 上渲染图像
                    svgRenderer.render(&painter, QRectF(0, 0, array[i], array[i]));
                    //生成尺寸目录
                    QString m_imageDirThemeSizePath = m_buildIconThemePath +"/" + QString::number(array[i]) + "x" + QString::number(array[i]);
                    QString m_imageDirThemeAppsPath = m_imageDirThemeSizePath+ "/"+m_type;
                    QDir m_imageThemeSizeDir = m_imageDirThemeSizePath;
                    QDir m_imageDirThemeAppDir = m_imageDirThemeAppsPath;
                    m_imageThemeSizeDir.mkdir(m_imageThemeSizeDir.absolutePath());
                    m_imageDirThemeAppDir.mkdir(m_imageDirThemeAppDir.absolutePath());

                    // 将 QImage 对象保存为 PNG 文件
                    QString m_fileName = m_svgFileInfo.baseName();
                    image.save(m_imageDirThemeAppsPath+"/"+m_fileName+".png");
                }
            }
        }else{
            qDebug()<<"svg images doesn't exits!!!";
        }


    }

}

//光标打包
void buildCheckWidget::buildForCursor(){
    //创建打包目录
    QString m_buildcursorThemePath = m_debPath+"/cursorTheme/"+ FileProcess::g_themeENName + "-cursor";
    QDir m_buildCursorThemeDir = m_buildcursorThemePath;
    m_buildCursorThemeDir.mkdir(m_buildCursorThemeDir.absolutePath());

    //写入icon index.theme文件
    QFile m_cursorIndex(m_buildcursorThemePath+"/index.theme");
    QTextStream m_cursorIndexStream(&m_cursorIndex);
    if (m_cursorIndex.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
//        qDebug()<< m_cursorIndex;
        m_cursorIndexStream << QStringLiteral("[Icon Theme]");
        m_cursorIndexStream << endl << QStringLiteral("Name=") + FileProcess::g_themeENName + QStringLiteral("-cursor");
        m_cursorIndexStream << endl << QStringLiteral("Comment=cursor theme");
        m_cursorIndexStream << endl;

        m_cursorIndexStream.flush();
        m_cursorIndex.close();
    }
    //写入icon cursor.theme文件
    QFile m_cursorTheme(m_buildcursorThemePath+"/cursor.theme");
    QTextStream m_cursorThemeStream(&m_cursorTheme);
    if (m_cursorTheme.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
//        qDebug()<< m_cursorTheme;
        m_cursorThemeStream << QStringLiteral("[Icon Theme]");
        m_cursorThemeStream << endl << QStringLiteral("Inherits=") + FileProcess::g_themeENName + QStringLiteral("-cursor");
        m_cursorThemeStream << endl;

        m_cursorIndexStream.flush();
        m_cursorTheme.close();
    }
    //编写cursor.sh
    QFile m_cursorShell(m_buildcursorThemePath+"/cursor.sh");
    QTextStream m_cursorShellStream(&m_cursorShell);
    if(!m_cursorShell.exists()){
        if (m_cursorShell.open(QIODevice::WriteOnly |  QIODevice::Append | QIODevice::Text)) {
//            qDebug()<< m_cursorShell;
            m_cursorShellStream << QStringLiteral("#!/bin/bash");
            m_cursorShellStream << endl << QStringLiteral("  mkdir cursors");

            m_cursorIndexStream.flush();
            m_cursorShell.close();
            m_cursorShell.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
                                         QFile::ReadUser | QFile::WriteUser | QFile::ExeUser |
                                         QFile::ReadGroup | QFile::WriteGroup | QFile::ExeGroup |
                                         QFile::ReadOther | QFile::WriteOther | QFile::ExeOther);
        }
    }
    //处理svg转png，获取svg文件
    QString m_cursorIconPath = QDir::homePath() + "/.cache/theme-build/" + FileProcess::g_date + "/src/cursorTheme/cursor/";
    QString m_timeCursorIconPath = QDir::homePath() + "/.cache/theme-build/" + FileProcess::g_date + "/src/cursorTheme/timecursor/";
    QDir m_cursorIconDir = m_cursorIconPath;
    QDir m_timeCursorIconDir = m_timeCursorIconPath;

    // 设置过滤器，仅获取 .svg 文件
    QStringList m_cursorIconNameFilters;
    m_cursorIconNameFilters << "*.svg";
    m_cursorIconDir.setNameFilters(m_cursorIconNameFilters);
    QStringList m_timeCursorIconNameFilters;
    m_timeCursorIconNameFilters << "*.svg";
    m_timeCursorIconDir.setNameFilters(m_timeCursorIconNameFilters);

    // 获取源目录中的文件列表
    QStringList m_cursorIconfiles = m_cursorIconDir.entryList(QDir::Files);
    QStringList m_timeCursorIconfiles = m_timeCursorIconDir.entryList(QDir::Files);

    if(!m_cursorIconfiles.isEmpty()||!m_timeCursorIconfiles.isEmpty()){

        QString m_cursorThemeSVGPath = m_buildcursorThemePath + "/svg";
        QDir m_cursorThemeSVGDir = m_cursorThemeSVGPath;
        m_cursorThemeSVGDir.mkdir(m_cursorThemeSVGDir.absolutePath());
        // 遍历文件列表并拷贝到目标目录
        foreach (const QString &m_cursorIconfile, m_cursorIconfiles) {
            QString sourceFilePath = m_cursorIconDir.filePath(m_cursorIconfile);
            QString destinationFilePath = m_cursorThemeSVGDir.filePath(m_cursorIconfile);

            QFile sourceFile(sourceFilePath);
            QFile destinationFile(destinationFilePath);

            // 如果源文件可以打开，并且目标文件可以创建
            if (sourceFile.open(QIODevice::ReadOnly) && destinationFile.open(QIODevice::WriteOnly)) {
                // 从源文件读取数据
                QByteArray data = sourceFile.readAll();

                // 将数据写入目标文件
                destinationFile.write(data);

                // 关闭文件
                sourceFile.close();
                destinationFile.close();
            }
        }
        foreach (const QString &m_timeCursorIconfile, m_timeCursorIconfiles) {
            QString sourceFilePath = m_timeCursorIconDir.filePath(m_timeCursorIconfile);
            QString destinationFilePath = m_cursorThemeSVGDir.filePath(m_timeCursorIconfile);

            QFile sourceFile(sourceFilePath);
            QFile destinationFile(destinationFilePath);

            if (sourceFile.open(QIODevice::ReadOnly) && destinationFile.open(QIODevice::WriteOnly)) {
                QByteArray data = sourceFile.readAll();
                destinationFile.write(data);
                sourceFile.close();
                destinationFile.close();
            }
        }

        QStringList m_svgFilters;
        m_svgFilters << "*.svg"; // 指定扩展名为 .svg

        m_cursorThemeSVGDir.setNameFilters(m_svgFilters);
        m_cursorThemeSVGDir.setFilter(QDir::Files | QDir::NoSymLinks);

        QFileInfoList m_svgFileInfoList = m_cursorThemeSVGDir.entryInfoList();

        // 遍历获取文件名
        foreach (QFileInfo m_svgFileInfo, m_svgFileInfoList) {
            QString m_fileName = m_svgFileInfo.fileName();
            QString m_deleteFileNameConclusion = m_fileName.chopped(4);
            QString m_fileNameTimeCursor = m_fileName.left(m_fileName.lastIndexOf('_'));

            //遍历、创建尺寸目录，并拷贝生成png
            int array[] = {24, 32, 48, 64};
            int length = sizeof(array) / sizeof(array[0]);

            for (int i = 0; i < length; i++) {

                // 初始化一个 SVG 渲染器
                QSvgRenderer svgRenderer(m_svgFileInfo.absoluteFilePath());

                // 创建一个和 SVG 同样大小的 QImage 对象，使用透明背景
                QImage image(QSize(array[i],array[i]), QImage::Format_ARGB32);
                image.fill(Qt::transparent);

                // 创建一个 QPainter 对象，用于在 QImage 对象上绘图
                QPainter painter(&image);
                // 使用 SVG 渲染器在 QPainter 上渲染图像
                svgRenderer.render(&painter, QRectF(0, 0, array[i], array[i]));

                //生成尺寸目录
                QString m_imageDirThemeSizePath = m_buildcursorThemePath +"/" + QString::number(array[i]) + "x" + QString::number(array[i]);
                QDir m_imageThemeSizeDir = m_imageDirThemeSizePath;
                m_imageThemeSizeDir.mkdir(m_imageThemeSizeDir.absolutePath());

                // 将 QImage 对象保存为 PNG 文件
                QString m_fileNameBase = m_svgFileInfo.baseName();
                image.save(m_imageDirThemeSizePath+"/"+m_fileNameBase+".png");

                if(m_fileNameTimeCursor != "watch" && m_fileNameTimeCursor != "left_ptr_watch"){

                    if (m_cursorShell.open(QIODevice::WriteOnly | QIODevice::Append  | QIODevice::Text)) {
                        qDebug()<< m_cursorShell;
                        m_cursorShellStream << endl << QStringLiteral("  echo '")+QString::number(array[i])+QStringLiteral(" ")
                                               +QString::number(array[i]/2)+QStringLiteral(" ")+QString::number(array[i]/2)
                                               +QStringLiteral(" ")+QString::number(array[i])+QStringLiteral("x")+QString::number(array[i])
                                               +QStringLiteral("/")+m_deleteFileNameConclusion+QStringLiteral(".png 0 '>> cursors/")
                                               +m_deleteFileNameConclusion+QStringLiteral(".in");

                        m_cursorShellStream.flush();
                        m_cursorShell.close();
                    } else {
                        qDebug()<< m_cursorShell;
                    }
                } else {

                    if (m_cursorShell.open(QIODevice::WriteOnly | QIODevice::Append  | QIODevice::Text)) {
                        qDebug()<< m_cursorShell;
                        m_cursorShellStream << endl << QStringLiteral("  echo '")+QString::number(array[i])+QStringLiteral(" ")
                                               +QString::number(array[i]/2)+QStringLiteral(" ")+QString::number(array[i]/2)
                                               +QStringLiteral(" ")+QString::number(array[i])+QStringLiteral("x")+QString::number(array[i])
                                               +QStringLiteral("/")+m_deleteFileNameConclusion+QStringLiteral(".png 50 '>> cursors/")
                                               +m_fileNameTimeCursor+QStringLiteral(".in");

                        m_cursorShellStream.flush();
                        m_cursorShell.close();
                    } else {
                        qDebug()<< m_cursorShell;
                    }
                }
            }

            //处理光标文件生成
            if(m_fileNameTimeCursor != "watch" && m_fileNameTimeCursor != "left_ptr_watch"){
                if (m_cursorShell.open(QIODevice::WriteOnly | QIODevice::Append  | QIODevice::Text)) {
                    qDebug()<< m_cursorShell;
                    m_cursorShellStream << endl << QStringLiteral("  xcursorgen cursors/")+m_deleteFileNameConclusion+QStringLiteral(".in cursors/")+m_deleteFileNameConclusion;

                    m_cursorShellStream.flush();
                    m_cursorShell.close();
                } else {
                    qDebug()<< m_cursorShell;
                }
            } else {
                if (m_cursorShell.open(QIODevice::WriteOnly | QIODevice::Append  | QIODevice::Text)) {
                    qDebug()<< m_cursorShell;
                    m_cursorShellStream << endl << QStringLiteral("  xcursorgen cursors/")+m_fileNameTimeCursor+QStringLiteral(".in cursors/")+m_fileNameTimeCursor;

                    m_cursorShellStream.flush();
                    m_cursorShell.close();
                } else {
                    qDebug()<< m_cursorShell;
                }
            }

            //处理软连接
            if(m_deleteFileNameConclusion == "left_ptr"){
                if (m_cursorShell.open(QIODevice::WriteOnly | QIODevice::Append  | QIODevice::Text)) {
                    qDebug()<< m_cursorShell;
                    m_cursorShellStream << endl << QStringLiteral("  cd cursors/");
                    m_cursorShellStream << endl << QStringLiteral("  ln -s left_ptr move");
                    m_cursorShellStream << endl << QStringLiteral("  ln -s left_ptr dnd-move");
                    m_cursorShellStream << endl << QStringLiteral("  ln -s left_ptr grabbing");
                    m_cursorShellStream << endl << QStringLiteral("  ln -s left_ptr top_left_arrow");
                    m_cursorShellStream << endl << QStringLiteral("  ln -s left_ptr arrow");
                    m_cursorShellStream << endl << QStringLiteral("  cd ..");

                    m_cursorShellStream.flush();
                    m_cursorShell.close();
                } else {
                    qDebug()<< m_cursorShell;
                }
            } else if(m_deleteFileNameConclusion == "circle"){
                if (m_cursorShell.open(QIODevice::WriteOnly | QIODevice::Append  | QIODevice::Text)) {
                    qDebug()<< m_cursorShell;
                    m_cursorShellStream << endl << QStringLiteral("  cd cursors/");
                    m_cursorShellStream << endl << QStringLiteral("  ln -s circle crossed_circle");
                    m_cursorShellStream << endl << QStringLiteral("  cd ..");

                    m_cursorShellStream.flush();
                    m_cursorShell.close();
                } else {
                    qDebug()<< m_cursorShell;
                }

            } else if(m_deleteFileNameConclusion == "cross"){
                if (m_cursorShell.open(QIODevice::WriteOnly | QIODevice::Append  | QIODevice::Text)) {
                    qDebug()<< m_cursorShell;
                    m_cursorShellStream << endl << QStringLiteral("  cd cursors/");
                    m_cursorShellStream << endl << QStringLiteral("  ln -s cross crosshair");
                    m_cursorShellStream << endl << QStringLiteral("  ln -s cross cross_reverse");
                    m_cursorShellStream << endl << QStringLiteral("  ln -s cross diamond_cross");
                    m_cursorShellStream << endl << QStringLiteral("  cd ..");

                    m_cursorShellStream.flush();
                    m_cursorShell.close();
                } else {
                    qDebug()<< m_cursorShell;
                }

            } else if(m_deleteFileNameConclusion == "dnd-none"){
                if (m_cursorShell.open(QIODevice::WriteOnly | QIODevice::Append  | QIODevice::Text)) {
                    qDebug()<< m_cursorShell;
                    m_cursorShellStream << endl << QStringLiteral("  cd cursors/");
                    m_cursorShellStream << endl << QStringLiteral("  ln -s dnd-none kylin-move");
                    m_cursorShellStream << endl << QStringLiteral("  cd ..");

                    m_cursorShellStream.flush();
                    m_cursorShell.close();
                } else {
                    qDebug()<< m_cursorShell;
                }

            } else if(m_deleteFileNameConclusion == "dnd-copy"){
                if (m_cursorShell.open(QIODevice::WriteOnly | QIODevice::Append  | QIODevice::Text)) {
                    qDebug()<< m_cursorShell;
                    m_cursorShellStream << endl << QStringLiteral("  cd cursors/");
                    m_cursorShellStream << endl << QStringLiteral("  ln -s dnd-copy copy");
                    m_cursorShellStream << endl << QStringLiteral("  cd ..");

                    m_cursorShellStream.flush();
                    m_cursorShell.close();
                } else {
                    qDebug()<< m_cursorShell;
                }

            } else if(m_deleteFileNameConclusion == "dnd-link"){
                if (m_cursorShell.open(QIODevice::WriteOnly | QIODevice::Append  | QIODevice::Text)) {
                    qDebug()<< m_cursorShell;
                    m_cursorShellStream << endl << QStringLiteral("  cd cursors/");
                    m_cursorShellStream << endl << QStringLiteral("  ln -s dnd-link copy");
                    m_cursorShellStream << endl << QStringLiteral("  cd ..");

                    m_cursorShellStream.flush();
                    m_cursorShell.close();
                } else {
                    qDebug()<< m_cursorShell;
                }

            } else if(m_deleteFileNameConclusion == "dnd-ask"){
                if (m_cursorShell.open(QIODevice::WriteOnly | QIODevice::Append  | QIODevice::Text)) {
                    qDebug()<< m_cursorShell;
                    m_cursorShellStream << endl << QStringLiteral("  cd cursors/");
                    m_cursorShellStream << endl << QStringLiteral("  ln -s dnd-ask question_arrow");
                    m_cursorShellStream << endl << QStringLiteral("  cd ..");

                    m_cursorShellStream.flush();
                    m_cursorShell.close();
                } else {
                    qDebug()<< m_cursorShell;
                }

            } else if(m_deleteFileNameConclusion == "h_double_arrow"){
                if (m_cursorShell.open(QIODevice::WriteOnly | QIODevice::Append  | QIODevice::Text)) {
                    qDebug()<< m_cursorShell;
                    m_cursorShellStream << endl << QStringLiteral("  cd cursors/");
                    m_cursorShellStream << endl << QStringLiteral("  ln -s h_double_arrow left_side");
                    m_cursorShellStream << endl << QStringLiteral("  ln -s h_double_arrow right_side");
                    m_cursorShellStream << endl << QStringLiteral("  cd ..");

                    m_cursorShellStream.flush();
                    m_cursorShell.close();
                } else {
                    qDebug()<< m_cursorShell;
                }

            } else if(m_deleteFileNameConclusion == "v_double_arrow"){
                if (m_cursorShell.open(QIODevice::WriteOnly | QIODevice::Append  | QIODevice::Text)) {
                    qDebug()<< m_cursorShell;
                    m_cursorShellStream << endl << QStringLiteral("  cd cursors/");
                    m_cursorShellStream << endl << QStringLiteral("  ln -s v_double_arrow top_side");
                    m_cursorShellStream << endl << QStringLiteral("  ln -s v_double_arrow bottom_side");
                    m_cursorShellStream << endl << QStringLiteral("  cd ..");

                    m_cursorShellStream.flush();
                    m_cursorShell.close();
                } else {
                    qDebug()<< m_cursorShell;
                }

            } else if(m_deleteFileNameConclusion == "ul_angle"){
                if (m_cursorShell.open(QIODevice::WriteOnly | QIODevice::Append  | QIODevice::Text)) {
                    qDebug()<< m_cursorShell;
                    m_cursorShellStream << endl << QStringLiteral("  cd cursors/");
                    m_cursorShellStream << endl << QStringLiteral("  ln -s ul_angle top_left_corner");
                    m_cursorShellStream << endl << QStringLiteral("  ln -s ul_angle bd_double_arrow");
                    m_cursorShellStream << endl << QStringLiteral("  ln -s ul_angle bottom_right_corner");
                    m_cursorShellStream << endl << QStringLiteral("  ln -s ul_angle lr_angle");
                    m_cursorShellStream << endl << QStringLiteral("  cd ..");

                    m_cursorShellStream.flush();
                    m_cursorShell.close();
                } else {
                    qDebug()<< m_cursorShell;
                }

            } else if(m_deleteFileNameConclusion == "ur_angle"){
                if (m_cursorShell.open(QIODevice::WriteOnly | QIODevice::Append  | QIODevice::Text)) {
                    qDebug()<< m_cursorShell;
                    m_cursorShellStream << endl << QStringLiteral("  cd cursors/");
                    m_cursorShellStream << endl << QStringLiteral("  ln -s ur_angle top_right_corner");
                    m_cursorShellStream << endl << QStringLiteral("  ln -s ur_angle fd_double_arrow");
                    m_cursorShellStream << endl << QStringLiteral("  ln -s ur_angle bottom_left_corner");
                    m_cursorShellStream << endl << QStringLiteral("  ln -s ur_angle ll_angle");
                    m_cursorShellStream << endl << QStringLiteral("  cd ..");

                    m_cursorShellStream.flush();
                    m_cursorShell.close();
                } else {
                    qDebug()<< m_cursorShell;
                }

            }

        }

        //多余文件处理
        if (m_cursorShell.open(QIODevice::WriteOnly | QIODevice::Append  | QIODevice::Text)) {
            qDebug()<< m_cursorShell;
            m_cursorShellStream << endl << QStringLiteral("  rm cursors/*.in");

            m_cursorShellStream.flush();
            m_cursorShell.close();
        } else {
            qDebug()<< m_cursorShell;
        }
    }else{
        qDebug()<<"svg images doesn't exits!!!";
    }


}

//开机动画打包
void buildCheckWidget::buildForPlymouth(){
    QString m_buildPlymouthPath = QDir::homePath()+ "/.cache/theme-build/" + FileProcess::g_date + "/src/plymouthTheme/other/";
    QDir m_buildPlymouthThemeDir = m_buildPlymouthPath;
    m_buildPlymouthThemeDir.mkdir(m_buildPlymouthThemeDir.absolutePath());
    //拷贝文件
    QFile::copy(":/resource/plymouth-other/logo.script", m_buildPlymouthPath + FileProcess::g_themeENName+"-logo.script");
    QFile::copy(":/resource/plymouth-other/uk.png",m_buildPlymouthPath+"/uk.png");
    QFile::copy(":/resource/plymouth-other/password_field.png",m_buildPlymouthPath+"/password_field.png");
    QFile::copy(":/resource/plymouth-other/progress_dot_off.png",m_buildPlymouthPath+"/progress_dot_off.png");

    //写入plymouth文件
    QFile m_themeplymouth(m_buildPlymouthPath + FileProcess::g_themeENName+"-logo.plymouth");
    QTextStream m_themeplymouthStream(&m_themeplymouth);
    if (m_themeplymouth.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
//        qDebug()<< m_themeplymouth;
        m_themeplymouthStream << QStringLiteral("[Plymouth Theme]");
        m_themeplymouthStream << endl << QStringLiteral("Name=My Theme");
        m_themeplymouthStream << endl << QStringLiteral("Description=This theme is a template to build on");
        m_themeplymouthStream << endl << QStringLiteral("ModuleName=script");
        m_themeplymouthStream << endl;
        m_themeplymouthStream << endl << QStringLiteral("[script]");
        m_themeplymouthStream << endl << QStringLiteral("ImageDir=/usr/share/plymouth/themes/")+FileProcess::g_themeENName+QStringLiteral("-logo");
        m_themeplymouthStream << endl << QStringLiteral("ScriptFile=/usr/share/plymouth/themes/")+FileProcess::g_themeENName
                                 +QStringLiteral("-logo/")+FileProcess::g_themeENName+QStringLiteral("-logo.script");
        m_themeplymouthStream.flush();
        m_themeplymouth.close();
    }
    //写入postinst文件
    QFile(m_debPath+"/debian/postinst").setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
                                                       QFile::ReadUser | QFile::WriteUser | QFile::ExeUser |
                                                       QFile::ReadGroup | QFile::WriteGroup | QFile::ExeGroup |
                                                       QFile::ReadOther | QFile::WriteOther | QFile::ExeOther);
    QFile m_postinst(m_debPath+"/debian/postinst");
    // 读取文件内容
    QStringList lines;

    QTextStream m_plymouthpostinstStream(&m_postinst);
    if (m_postinst.open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text)) {
        //处理多次编译
        m_plymouthpostinstStream.seek(0);
        while (!m_plymouthpostinstStream.atEnd()) {
            QString line = m_plymouthpostinstStream.readLine();
            // 如果行不包含特定文本，则添加到新的行列表
            if (!line.contains("update-alternatives")) {
                lines << line;
            }
        }
         m_postinst.resize(0);
         foreach (const QString &line, lines) {
             if (line == "#!/bin/sh"){
                 m_plymouthpostinstStream << line;
             } else {
                 m_plymouthpostinstStream << endl << line;
             }
         }
         m_plymouthpostinstStream << endl << QStringLiteral("update-alternatives --remove-all default.plymouth");
         m_plymouthpostinstStream << endl << QStringLiteral("update-alternatives"
                                                    " --install /usr/share/plymouth/themes/default.plymouth default.plymouth "
                                                    "/usr/share/plymouth/themes/")+FileProcess::g_themeENName+QStringLiteral("-logo/")
                                                    +FileProcess::g_themeENName+QStringLiteral("-logo.plymouth 150");
         m_plymouthpostinstStream.flush();
         m_postinst.close();
     } else {
         qDebug()<< "can't open!!!!!!!!!";
     }
}

//grub打包
void buildCheckWidget::buildForGrub(){
    QString m_buildGrubPath = QDir::homePath()+ "/.cache/theme-build/" + FileProcess::g_date + "/src/grubTheme/";
    if(QFile::exists(m_buildGrubPath+"background-"+FileProcess::g_themeENName+".png")){
        QFile::remove(m_buildGrubPath+"background-"+FileProcess::g_themeENName+".png");
    }
    //拷贝grub背景
    QFile::copy(m_buildGrubPath+"background.png",m_buildGrubPath+"background-"+FileProcess::g_themeENName+".png");

    //写入postinst文件
    QFile(m_debPath+"/debian/postinst").setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
                                                       QFile::ReadUser | QFile::WriteUser | QFile::ExeUser |
                                                       QFile::ReadGroup | QFile::WriteGroup | QFile::ExeGroup |
                                                       QFile::ReadOther | QFile::WriteOther | QFile::ExeOther);
    QFile m_postinst(m_debPath+"/debian/postinst");
    QTextStream m_grubpostinstStream(&m_postinst);
    QStringList lines;
    if (m_postinst.open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text)) {
//        qDebug()<< m_postinst;
        //处理多次编译
        m_grubpostinstStream.seek(0);
        QStringList linesToRemove; // 用于存储需要移除的行
        bool skipNextLines = false; // 标志是否需要跳过接下来的四行

        while (!m_grubpostinstStream.atEnd()) {
            QString line = m_grubpostinstStream.readLine();

            if (skipNextLines) {
                // 如果需要跳过接下来的四行，则将这四行添加到移除列表中，并将标志重置为false
                linesToRemove << line;
                if (linesToRemove.size() == 5) {
                    skipNextLines = false;
                }
            } else if (line.contains("file_path='/usr/share/grub/themes/UKUI/theme.txt'")) {
                // 如果当前行包含特定文本，则将这一行和接下来的四行都标记为需要移除
                linesToRemove << line;
                skipNextLines = true;
            } else {
                // 如果当前行既不是需要移除的行也不是其后的四行，则添加到新行列表中
                lines << line;
            }
        }
        m_postinst.resize(0);
        foreach (const QString &line, lines) {
            if (line == "#!/bin/sh"){
                m_grubpostinstStream << line;
            } else {
                m_grubpostinstStream << endl << line;
            }
        }
        m_grubpostinstStream << endl << QStringLiteral("file_path='/usr/share/grub/themes/UKUI/theme.txt'");
        m_grubpostinstStream << endl << QStringLiteral("line_number=$(grep -n 'desktop-image:' $file_path | cut -d ':' -f 1)");
        m_grubpostinstStream << endl << QStringLiteral("if [ -n '$line_number' ]; then");
        m_grubpostinstStream << endl << QStringLiteral("  sed -i '/^desktop-image:/s|\".*\"|\"'background-")+FileProcess::g_themeENName+(".png'\"|' \"$file_path\"");
        m_grubpostinstStream << endl << QStringLiteral("fi");

        m_grubpostinstStream.flush();
        m_postinst.close();
    }

}

void buildCheckWidget::initSetupShell()
{
    //将接收到的gsetting值写入shell
    QString m_setupshellpath = m_debPath+"/setupConfig/kylin-theme-builder-change";
    QFile file(m_setupshellpath);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        qDebug() << "Failed to open file.";
    }
    QTextStream in(&file);
    QString fileContent = in.readAll();
    int startIndex = fileContent.indexOf("gsettings set org.ukui.style theme-color");
    startIndex += 40;
    fileContent.insert(startIndex, " " + m_option->accentRole);

    file.close();
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << fileContent;
        file.close();

        return ;
    } else {
        qDebug() << "无法写入文件：" << m_setupshellpath;
        return  ;
    }
}

void buildCheckWidget::initSetupSchemas()
{

    //将接收到的gsetting值写入xml
    QString m_setupschemapath = m_debPath+"/setupConfig/40_kylin-theme-builder.gschema.override";
    QFile file(m_setupschemapath);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        qDebug() << "Failed to open file.";
    }
    QTextStream in(&file);
    QString fileContent = in.readAll();
    int startIndex = fileContent.indexOf("theme-color=");
    startIndex += 12;
    fileContent.insert(startIndex,  "'"+m_option->accentRole+"'");

    file.close();
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << fileContent;
        file.close();

        return ;
    } else {
        qDebug() << "无法写入文件：" << m_setupschemapath;
        return  ;
    }
}

//编译检查
void buildCheckWidget::buildCheck(QDir m_buildPath){
    //获取基础资源文件
//    QDir appicons = ":/resource/appicons/";
//    QDir systemIcons = ":/resource/systemicons";
//    QCryptographicHash hash(QCryptographicHash::Md5);

//    QStringList appIconFiles = appicons.entryList(QDir::Files);
//    foreach(QString appIconFile, appIconFiles){
//        appIconInfo[appIconFile] = appIconInfoID;
//        appIconInfoID++;
//    }
//    QStringList systemIconFiles = systemIcons.entryList(QDir::Files);
//    foreach(QString systemIconFile, systemIconFiles){
//        systemIconInfo[systemIconFile] = systemIconInfoID;
//        systemIconInfoID++;
//    }


//    if (m_buildPath.exists()) {
//        qDebug() << "开始遍历目录：" << m_buildPath.absolutePath();
//        buildCheckResult(m_buildPath);

//        for (auto it = appIconInfo.begin(); it != appIconInfo.end(); ++it) {
//            const QString appIconInfo = it.key();

//            // 检查是否 keyA 存在于 QMap B 中
//            if (!cacheIconInfo.contains(keyA)) {
//                // 如果 keyA 不存在于 QMap B 中，将其添加到 result
//                warning[keyA] = it.value();
//            }
//        }

//        if(buildCheckState == 0){
            buildCopy();

            if(FileProcess::g_createThemeType == "globalTheme"){
                buildForGlobal();
            } else if(FileProcess::g_createThemeType == "iconTheme"){
                buildForIcon();
            } else if(FileProcess::g_createThemeType == "cursorTheme"){
                buildForCursor();
            } else if(FileProcess::g_createThemeType == "plymouthTheme"){
                buildForPlymouth();
            } else if(FileProcess::g_createThemeType == "grubTheme"){
                buildForGrub();
            }

            buildWidget *m_buildWidget = new buildWidget();
            m_buildWidget->show();

            connect(m_buildWidget, &buildWidget::gohomesignals, this,[=](){
                //发送返回主界面信号给infocreatewidget
                emit gohomesignals();
            });
//        } else {
//            this->setFixedSize(424,480);
//            this->setWindowModality(Qt::ApplicationModal);

//            m_buildCheckWidget = new QVBoxLayout();
//            m_buildWarningMessage = new QLabel();
//            m_buildWarningMessage->setText("导出时检测到以下问题，请调整后重新导出:");
//            m_warningMessageList = new QLabel();
//            m_warningMessageList->setText("缺少以下素材：");

//            m_buildCheckBtnWidget = new QHBoxLayout();
//            m_buildBtn = new QPushButton();
//            m_buildBtn->setText("继续导出");
//            m_exitBuildBtn = new QPushButton();
//            m_exitBuildBtn->setText("返回");

//            m_buildCheckBtnWidget->addWidget(m_buildBtn);
//            m_buildCheckBtnWidget->addWidget(m_exitBuildBtn);
//            m_buildCheckWidget->addWidget(m_buildWarningMessage);
//            m_buildCheckWidget->addWidget(m_warningMessageList);
//            m_buildCheckWidget->addLayout(m_buildCheckBtnWidget);

//            this->setLayout(m_buildCheckWidget);
//            this->show();
//        }
//    } else {
//        qDebug() << "目录不存在：" << m_buildPath.absolutePath();
//    }
}

void buildCheckWidget::buildCheckResult(QDir m_buildPath){

    QString path = m_buildPath.absolutePath();
    QDir m_buildThemePath = path + "/" + FileProcess::g_createThemeType;
    // 设置过滤器，只获取文件和目录
    m_buildThemePath.setFilter(QDir::Files|QDir::Dirs| QDir::NoDotAndDotDot);

    // 获取目录下的所有文件和目录
    QFileInfoList fileList = m_buildThemePath.entryInfoList();
//    qDebug()<<fileList;
    // 遍历文件和目录
    foreach(const QFileInfo& fileInfo, fileList)
    {
//        qDebug()<<fileInfo;
        if (fileInfo.isFile()){
            if(FileCheck::isLegalIconFile(fileInfo.absoluteFilePath())){
//                cacheIconInfo[fileInfo] = cacheIconInfoID;
//                cacheIconInfoID++;

            } else {
                qDebug() << "warning:" << fileInfo.absoluteFilePath();
                buildCheckState--;
            }
        } else if (fileInfo.isDir()){
            buildCheckResult(QDir(fileInfo.absoluteFilePath()));
        } else {
            qDebug() << "warning:" << fileInfo.absoluteFilePath();
            buildCheckState--;
        }
    }
}
