/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#include "utils.h"
#include <QWidget>
#include <random>


/**
 * @brief 移到鼠标所在屏幕中央。兼容990
 */
Utils::Utils(QObject *parent):QObject(parent)
{

}

void Utils::centerToScreen(QWidget *widget)
{
    if (!widget)

       return;

     QDesktopWidget* m = QApplication::desktop();

     QRect desk_rect = m->screenGeometry(m->screenNumber(QCursor::pos()));

     int desk_x = desk_rect.width();

     int desk_y = desk_rect.height();

     int x = widget->width();

     int y = widget->height();

     widget->move(desk_x / 2 - x / 2 + desk_rect.left(), desk_y / 2 - y / 2 + desk_rect.top());
}
QRect Utils::centerToScreenQRect(QWidget *widget)
{
    if (!widget)

       return QRect();

     QDesktopWidget* m = QApplication::desktop();

     QRect desk_rect = m->screenGeometry(m->screenNumber(QCursor::pos()));

     int desk_x = desk_rect.width();

     int desk_y = desk_rect.height();

     int x = widget->width();

     int y = widget->height();
     QRect result = QRect(desk_x / 2 - x / 2 + desk_rect.left(), desk_y / 2 - y / 2 + desk_rect.top(),x,y);
     return result;
}
QString Utils::loadFontFamilyFromTTF()
{
    static QString font;
    static bool loaded = false;
    if(!loaded)
    {
        loaded = true;
        int loadedFontID = QFontDatabase::addApplicationFont(":/image/DFPKingGothicGB-Semibold-2.ttf");
        QStringList loadedFontFamilies = QFontDatabase::applicationFontFamilies(loadedFontID);
        if(!loadedFontFamilies.empty())
            font = loadedFontFamilies.at(0);
    }
    return font;
}

QString Utils::loadFontHuaKangJinGangHeiRegularTTF()
{
    static QString font;
    static bool loaded = false;
    if(!loaded)
    {
        loaded = true;
        int loadedFontID = QFontDatabase::addApplicationFont(":/image/HuaKangJinGangHei-Regular-2.ttf");
        QStringList loadedFontFamilies = QFontDatabase::applicationFontFamilies(loadedFontID);
        if(!loadedFontFamilies.empty())
            font = loadedFontFamilies.at(0);
    }
    return font;
}

QString Utils::getRandomId()
{
    //time(0) 随机种子
    static std::default_random_engine e(time(0));
    //5位数
    static std::uniform_int_distribution<unsigned> u(10000, 99999);
    //转字符串
    return QString::number(u(e));
}
/*
 * 单位变双位
 * Integer to character
 */
QString Utils::changeNumToStr(int alarmHour)
{
    QString str;
    if (alarmHour < 10) {
        QString hours_str = QString::number(alarmHour);
        str = "0"+hours_str;
    } else {
        str = QString::number(alarmHour);
    }
    return str;
}

void Utils::resetText(QLabel* label , int pageIndex)
{
    // pageIndex : 0 表示无闹钟界面；1表示编辑闹钟页面；2表示倒计时页面
    QFontMetrics fontWidth(label->font());//得到每个字符的宽度
    QString item_name = label->text();
    QString show_name = "";
    if (label->toolTip() != "")
    {
        item_name = label->toolTip();
    }
    if(pageIndex == 0 || pageIndex == 1){
        show_name = fontWidth.elidedText(item_name, Qt::ElideRight, label->width() - 25);
    } else{
        show_name = fontWidth.elidedText(item_name, Qt::ElideRight, label->width() - 55);
    }
    label->setText(show_name);
    if (show_name.endsWith("…"))
        label->setToolTip(item_name);
    else
        label->setToolTip("");
}

void Utils::setBtnBackgroundColorTransparent(QPushButton *btn)
{
    QPalette pale =btn->palette();
    QColor color = pale.color(QPalette::Button);
    QColor ColorPlaceholderText3 = QColor(255,255,255,0);
    QBrush brush;
    brush.setColor(ColorPlaceholderText3);
    pale.setBrush(QPalette::Button, brush);
    btn->setPalette(pale);
}

void Utils::setBtnHighlightColorTransparent(QPushButton *btn)
{
    QPalette pale =btn->palette();
    QColor color = pale.color(QPalette::Button);
    QColor ColorPlaceholderText3 = QColor(255,255,255,0);
    QBrush brush;
    brush.setColor(ColorPlaceholderText3);
    pale.setBrush(QPalette::Highlight, brush);
    btn->setPalette(pale);
}

QIcon Utils::getQIcon(QString themeUrl, QString localUrl)
{
    QIcon icon = QIcon::fromTheme(themeUrl);
    if(icon.isNull()){
        qWarning()<<"dbq-empty url"<<themeUrl;
        icon = QIcon(localUrl);
    }
    return icon;
}
QString Utils::getOmitStr(QString str, int sizeLimit)
{
    if(str.length()>sizeLimit){
        str = str.left(sizeLimit);
        str = str+"...";
    }
    return str;
}

QString Utils::getOmitStr(const QString &str, const QFont &font, int pixelWidthLimit)
{
    if (str.isEmpty()) {
           return str;
       }

       QFontMetrics fm(font);
       const QString ellipsis = "...";
       const int ellipsisWidth = fm.horizontalAdvance(ellipsis);
       const int availableWidth = pixelWidthLimit - ellipsisWidth;

       if (availableWidth <= 0) {
           return ellipsis;
       }

       int totalWidth = fm.horizontalAdvance(str);
       if (totalWidth <= pixelWidthLimit) {
           return str;
       }

       int currentWidth = 0;
       int truncateIndex = 0;
       const int strLength = str.length();

       for (; truncateIndex < strLength; ++truncateIndex) {
           QChar ch = str.at(truncateIndex);
           int charWidth = fm.horizontalAdvance(ch);
           if (currentWidth + charWidth > availableWidth) {
               break;
           }
           currentWidth += charWidth;
       }

       QString result = str.left(truncateIndex) + ellipsis;
       qInfo() << "截断后的字符串：" << result << "，显示宽度：" << fm.horizontalAdvance(result);
       return result;
}

bool Utils::checkLocalChina()
{
    return QLocale().name()=="zh_CN";
}
bool Utils::checkLocalUs()
{
    return QLocale().name()=="en_US";
}

bool Utils::checkLocalBo()
{
    return QLocale().name()=="bo_CN";
}
int Utils::handelColorRange(int value)
{
    if(value<0){
        return 0;
    }else if(value>255){
        return 255;
    }else{
        return value;
    }
}

QString Utils::getClockVersion()
{
    //return VERSION;
    FILE *pp = NULL;
       char *line = NULL;
       size_t len = 0;
       ssize_t read;
       char *q = NULL;
       QString version = tr("none");

       pp = popen("dpkg -l ukui-clock", "r");
       if(NULL == pp)
           return version;

       while((read = getline(&line, &len, pp)) != -1){
           q = strrchr(line, '\n');
           *q = '\0';

           QString content = line;
           QStringList list = content.split(" ");

           list.removeAll("");

           if (list.size() >= 3)
               version = list.at(2);
       }

       free(line);
       pclose(pp);
       return version;
}
