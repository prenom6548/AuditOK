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
#include "information.h"
#include "sizedate.h"
#include "kyutils.h"
Information::Information(QWidget *parent) : QWidget(parent)
{
    this->setObjectName("Kylin-photo-viewer_QWidget_informationWindow");
    this->setAccessibleName("Kylin-photo-viewer_QWidget_informationWindow");
    this->setAccessibleDescription("information window");

    if (m_local.system().name() == "en_US") {
        this->resize(INFOR_SIZE.width() + 7, INFOR_SIZE.height());
    } else if (m_local.system().name() == "zh_CN") {
        this->resize(INFOR_SIZE);
    } else {
        this->resize(INFOR_SIZE.width() + 7, INFOR_SIZE.height());
    }
    //    this->adjustSize();
    m_ft.setBold(true);
    m_ft.setPixelSize(14);
    m_ftContent.setPixelSize(14);
    m_widName = new QLabel(this);
    m_widName->setObjectName("Kylin-photo-viewer_QLabel_widName");
    m_widName->setAccessibleName("Kylin-photo-viewer_QLabel_widName");
    m_widName->setAccessibleDescription("information widget name");
    m_widName->setAttribute(Qt::WA_TranslucentBackground);
    //    m_widName->setFont(m_ft);
    //    m_widName->setAttribute();
    m_widName->setText(tr("Info"));
    //    m_widName->setText(tr("信息"));

    m_name = new QLabel(this);
    m_name->setObjectName("Kylin-photo-viewer_QLabel_name");
    m_name->setAccessibleName("Kylin-photo-viewer_QLabel_name");
    m_name->setAccessibleDescription("name tag");
    m_name->setAttribute(Qt::WA_TranslucentBackground);
    m_name->setText(tr("Name"));
    //    m_name->setFont(m_ftContent);
    //    m_name->setText(tr("名称"));

    m_format = new QLabel(this);
    m_format->setObjectName("Kylin-photo-viewer_QLabel_format");
    m_format->setAccessibleName("Kylin-photo-viewer_QLabel_format");
    m_format->setAccessibleDescription("format tag");
    m_format->setAttribute(Qt::WA_TranslucentBackground);
    //    m_format->setFixedHeight(16);
    m_format->setText(tr("Type"));
    //    m_format->setFont(m_ftContent);
    //    m_format->setText(tr("格式"));

    m_storageSize = new QLabel(this);
    m_storageSize->setObjectName("Kylin-photo-viewer_QLabel_storageSize");
    m_storageSize->setAccessibleName("Kylin-photo-viewer_QLabel_storageSize");
    m_storageSize->setAccessibleDescription("storage size tag");
    m_storageSize->setAttribute(Qt::WA_TranslucentBackground);
    m_storageSize->setText(tr("Capacity"));
    //    storageSize->setText(tr("大小"));
    //    m_storageSize->setFont(m_ftContent);

    m_pixelSize = new QLabel(this);
    m_pixelSize->setObjectName("Kylin-photo-viewer_QLabel_pixelSize");
    m_pixelSize->setAccessibleName("Kylin-photo-viewer_QLabel_pixelSize");
    m_pixelSize->setAccessibleDescription("pixel size tag");
    m_pixelSize->setAttribute(Qt::WA_TranslucentBackground);
    m_pixelSize->setText(tr("Size"));
    //    pixelSize->setText(tr("尺寸"));
    //    m_pixelSize->setFont(m_ftContent);

    m_colorSpace = new QLabel(this);
    m_colorSpace->setObjectName("Kylin-photo-viewer_QLabel_colorSpace");
    m_colorSpace->setAccessibleName("Kylin-photo-viewer_QLabel_colorSpace");
    m_colorSpace->setAccessibleDescription("color space tag");
    m_colorSpace->setAttribute(Qt::WA_TranslucentBackground);
    m_colorSpace->setText(tr("Color"));
    //    colorSpace->setText(tr("颜色空间"));
    //    m_colorSpace->setFont(m_ftContent);

    m_creationTime = new QLabel(this);
    m_creationTime->setObjectName("Kylin-photo-viewer_QLabel_creationTime");
    m_creationTime->setAccessibleName("Kylin-photo-viewer_QLabel_creationTime");
    m_creationTime->setAccessibleDescription("creation time tag");
    m_creationTime->setAttribute(Qt::WA_TranslucentBackground);
    m_creationTime->setText(tr("Created"));
    m_creationTime->hide();
    //    creationTime->setText(tr("创建时间"));
    //    m_creationTime->setFont(m_ftContent);

    m_revisionTime = new QLabel(this);
    m_revisionTime->setObjectName("Kylin-photo-viewer_QLabel_revisionTime");
    m_revisionTime->setAccessibleName("Kylin-photo-viewer_QLabel_revisionTime");
    m_revisionTime->setAccessibleDescription("modification time tag");
    m_revisionTime->setAttribute(Qt::WA_TranslucentBackground);
    m_revisionTime->setText(tr("Modified"));
    //    revisionTime->setText(tr("修改时间"));
    //    m_revisionTime->setFont(m_ftContent);

    m_nameC = new QLabel(this);
    m_nameC->setObjectName("Kylin-photo-viewer_QLabel_nameContent");
    m_nameC->setAccessibleName("Kylin-photo-viewer_QLabel_nameContent");
    m_nameC->setAccessibleDescription("name content");
    m_nameC->setAttribute(Qt::WA_TranslucentBackground);
    //    m_nameC->setFont(m_ftContent);
    m_nameC->setWordWrap(true);

    m_formatC = new QLabel(this);
    m_formatC->setObjectName("Kylin-photo-viewer_QLabel_formatContent");
    m_formatC->setAccessibleName("Kylin-photo-viewer_QLabel_formatContent");
    m_formatC->setAccessibleDescription("format content");
    m_formatC->setAttribute(Qt::WA_TranslucentBackground);
    //    m_formatC->setFont(m_ftContent);

    m_storageSizeC = new QLabel(this);
    m_storageSizeC->setObjectName("Kylin-photo-viewer_QLabel_storageSizeContent");
    m_storageSizeC->setAccessibleName("Kylin-photo-viewer_QLabel_storageSizeContent");
    m_storageSizeC->setAccessibleDescription("storage size content");
    m_storageSizeC->setAttribute(Qt::WA_TranslucentBackground);
    //    m_storageSizeC->setFont(m_ftContent);

    m_pixelSizeC = new QLabel(this);
    m_pixelSizeC->setObjectName("Kylin-photo-viewer_QLabel_pixelSizeContent");
    m_pixelSizeC->setAccessibleName("Kylin-photo-viewer_QLabel_pixelSizeContent");
    m_pixelSizeC->setAccessibleDescription("pixel size content");
    m_pixelSizeC->setAttribute(Qt::WA_TranslucentBackground);
    //    m_pixelSizeC->setFont(m_ftContent);

    m_colorSpaceC = new QLabel(this);
    m_colorSpaceC->setObjectName("Kylin-photo-viewer_QLabel_colorSpaceContent");
    m_colorSpaceC->setAccessibleName("Kylin-photo-viewer_QLabel_colorSpaceContent");
    m_colorSpaceC->setAccessibleDescription("color space content");
    m_colorSpaceC->setAttribute(Qt::WA_TranslucentBackground);
    //    m_colorSpaceC->setFont(m_ftContent);

    m_creationTimeC = new QLabel(this);
    m_creationTimeC->setObjectName("Kylin-photo-viewer_QLabel_creationTimeContent");
    m_creationTimeC->setAccessibleName("Kylin-photo-viewer_QLabel_creationTimeContent");
    m_creationTimeC->setAccessibleDescription("creation time content");
    m_creationTimeC->setAttribute(Qt::WA_TranslucentBackground);
    m_creationTimeC->hide();
    //    m_creationTimeC->setFont(m_ftContent);

    m_revisionTimeC = new QLabel(this);
    m_revisionTimeC->setObjectName("Kylin-photo-viewer_QLabel_revisionTimeContent");
    m_revisionTimeC->setAccessibleName("Kylin-photo-viewer_QLabel_revisionTimeContent");
    m_revisionTimeC->setAccessibleDescription("modification time content");
    m_revisionTimeC->setAttribute(Qt::WA_TranslucentBackground);
    //    m_revisionTimeC->setFont(m_ftContent);
    //布局
    m_inforWid = new QWidget(this);
    m_inforWid->setObjectName("Kylin-photo-viewer_QWidget_information");
    m_inforWid->setAccessibleName("Kylin-photo-viewer_QWidget_information");
    m_inforWid->setAccessibleDescription("information widget");
    m_gdLayout = new QGridLayout(this);
}
//为各控件设置text
void Information::contentText(QFileInfo info, QString sizeImage, QString spaceColor, QString realFormat)
{
    QString imageSize;

    QString size = QString::number(info.size());
    char sdkSize[1024];

    if (float(info.size()) / 1024 >= 1024) {
        if (kdkVolumeBaseCharacterConvert(size.toLocal8Bit().data(), KDK_MEGABYTE, sdkSize) != 0) {
            QString Size = QString("%1").arg(QString::asprintf("%.1f", float(info.size()) / 1024 / 1024));
            imageSize = Size + "MB";
        } else {
            imageSize = QString::fromUtf8(sdkSize);
        }
    } else {
        if (kdkVolumeBaseCharacterConvert(size.toLocal8Bit().data(), KDK_KILOBYTE, sdkSize) != 0) {
            QString Size = QString("%1").arg(QString::asprintf("%.1f", float(info.size()) / 1024));
            imageSize = Size + "KB";
        } else {
            imageSize = QString::fromUtf8(sdkSize);
        }
    }
    if (sizeImage == "-") {
        imageSize = "0.0KB";
    }
    m_creationTimeC->setText(info.birthTime().toString("yyyy.MM.dd hh:mm"));
    m_revisionTimeC->setText(info.lastModified().toString("yyyy.MM.dd hh:mm"));
    QString nameContant = AutoFeed(info.completeBaseName());
    m_nameC->setText(nameContant);
    m_formatC->setText(realFormat.toUpper());
    m_storageSizeC->setText(imageSize);
    m_pixelSizeC->setText(sizeImage);
    m_colorSpaceC->setText(spaceColor);

    this->layout();
}
//布局--需要根据图片名字是否是两行来重设大小
void Information::layout()
{
    if (!m_nameC->text().isEmpty()) {
        if (m_linenum) {
            this->layoutS(2);
        } else {
            this->layoutS(0);
        }
    } else if (m_nameC->text() == "") {
        this->layoutS(0);
    }
}

void Information::layoutS(int line)
{
    m_gdLayout->addWidget(m_widName, 0, 0, 1, 2);
    m_gdLayout->addWidget(m_name, 1, 0, 1, 2, Qt::AlignTop);
    m_gdLayout->addWidget(m_nameC, 1, 3, 1, 4, Qt::AlignTop);
    m_gdLayout->addWidget(m_format, 2 + line, 0, 1, 2);
    m_gdLayout->addWidget(m_formatC, 2 + line, 3, 1, 4);
    m_gdLayout->addWidget(m_storageSize, 3 + line, 0, 1, 2);
    m_gdLayout->addWidget(m_storageSizeC, 3 + line, 3, 1, 4);
    m_gdLayout->addWidget(m_pixelSize, 4 + line, 0, 1, 2);
    m_gdLayout->addWidget(m_pixelSizeC, 4 + line, 3, 1, 4);
    m_gdLayout->addWidget(m_colorSpace, 5 + line, 0, 1, 2);
    m_gdLayout->addWidget(m_colorSpaceC, 5 + line, 3, 1, 4);
//    m_gdLayout->addWidget(m_creationTime, 6 + line, 0, 1, 2);
//    m_gdLayout->addWidget(m_creationTimeC, 6 + line, 3, 1, 4);
    m_gdLayout->addWidget(m_revisionTime, 6 + line, 0, 1, 2);
    m_gdLayout->addWidget(m_revisionTimeC, 6 + line, 3, 1, 4);
    m_gdLayout->setContentsMargins(12, 10 - line * 3, 10, 16 - line * 3);
    m_inforWid->setLayout(m_gdLayout);
    m_inforWid->resize(this->width(), this->height());
}
//由重命名触发的更新信息栏名字函数
void Information::updateName(QFileInfo newFile)
{
    QString nameContant = AutoFeed(newFile.completeBaseName());
    m_nameC->setText(nameContant);
    this->layout();
}
//文字过长时，最大换两行后显示...
QString Information::AutoFeed(QString text)
{
    QString strText = text;
    int AntoIndex = 1;
    QFontMetrics fm(m_nameC->font());
    int width = fm.width(m_revisionTimeC->text());
    //手动处理图片名字，来进行换行（支持中英文，数字，故替换掉原先的方法）
    if (!strText.isEmpty()) {
        for (int i = 1; i < strText.size() + 1; i++) {
            if (fm.width(strText.left(i)) <= width) {
                strText = text;
                m_lineInTwo = false;
            }
            //超过width长度，加换行符换行
            if (fm.width(strText.left(i)) > width * AntoIndex) {
                AntoIndex++;
                strText.insert(i - 1, "\n");
            }
            if (fm.width(strText.left(i)) > width && fm.width(strText.left(i)) <= 2 * (width - 6)) {
                m_lineInTwo = true;
            }
            //超过两行减6x2，加上...
            if (fm.width(strText.left(i)) > 2 * (width - 6)) {
                strText.insert(i - 1, "…");
                strText = strText.mid(0, i );
                m_lineInTwo = false;
                continue;
            }
        }
    }
    //判断窗口大小和是否显示tooltips，解决tooltips设置一次后一直显示的问题
    if (strText.contains("…")) {
        m_linenum = true;
        m_nameC->setToolTip(text);
    } else {
        m_nameC->setToolTip("");
        if (m_lineInTwo) {
            m_linenum = true;
        } else {
            m_linenum = false;
        }
    }

    return strText;
}

void Information::changeEverySize(double fontSize, double detio, QSize sizeWid)
{
    QFont font;
    font.setPointSizeF(fontSize);
    m_widName->setFont(font);
    m_name->setFont(font);
    m_nameC->setFont(font);
    m_format->setFont(font);
    m_formatC->setFont(font);
    m_storageSize->setFont(font);
    m_storageSizeC->setFont(font);
    m_pixelSize->setFont(font);
    m_pixelSizeC->setFont(font);
    m_colorSpace->setFont(font);
    m_colorSpaceC->setFont(font);
    m_creationTime->setFont(font);
    m_creationTimeC->setFont(font);
    m_revisionTime->setFont(font);
    m_revisionTimeC->setFont(font);
    m_inforWid->resize(sizeWid * detio);
    this->resize(m_inforWid->size());
}
