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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "mediainfodialog.h"
#include "ui_mediainfodialog.h"

#include <QDebug>
#include <QScrollBar>
#include <QMouseEvent>

#include <KF5/KWindowSystem/kwindoweffects.h>

#include <ukui-log4qt.h>
#include "global/global.h"
#include "global/globalsignal.h"

using namespace Global;

MediaInfoDialog::MediaInfoDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MediaInfoDialog)
{
    ui->setupUi(this);
    setFixedSize(380, 355);
    this->setWindowModality(Qt::WindowModal);

    ui->btn_icon->setFixedSize(QSize(26,26));
    ui->btn_icon->setIconSize(QSize(26,26));
    ui->btn_icon->setIcon(QIcon::fromTheme("kylin-video"));
    ui->btn_icon->setStyleSheet("QPushButton{border:0px;background:transparent;}"
                                "QPushButton::hover{border:0px;background:transparent;}"
                                "QPushButton::pressed{border:0px;background:transparent;}");

    ui->btn_close->setFixedSize(30, 30);
    ui->btn_close->setProperty("isWindowButton", 0x2);
    ui->btn_close->setProperty("useIconHighlightEffect", 0x8);
    ui->btn_close->setFlat(true);
    ui->btn_close->setIcon(QIcon::fromTheme("window-close-symbolic"));
    ui->btn_close->setToolTip(tr("Close"));
    connect(ui->btn_close, &QPushButton::clicked, [&](){accept();});

    ui->btn_ok->setProperty("isImportant", true);

    ui->m_title->setText(tr("title:"));
    ui->m_title->setFixedHeight(30);

    //ui->v_title->setWordWrap(true);
    ui->v_title->setTextInteractionFlags(Qt::TextSelectableByMouse);
    ui->v_title->setCursor(Qt::IBeamCursor);

    ui->m_type->setText(tr("type:"));
    ui->m_type->setFixedHeight(30);

    ui->v_type->setTextInteractionFlags(Qt::TextSelectableByMouse);
    ui->v_type->setCursor(Qt::IBeamCursor);

    ui->m_size->setText(tr("size:"));
    ui->m_size->setFixedHeight(30);

    ui->v_size->setWordWrap(true);
    ui->v_size->setTextInteractionFlags(Qt::TextSelectableByMouse);
    ui->v_size->setCursor(Qt::IBeamCursor);

    ui->m_duration->setText(tr("duration:"));
    ui->m_duration->setFixedHeight(30);
    ui->v_duration->setTextInteractionFlags(Qt::TextSelectableByMouse);
    ui->v_duration->setCursor(Qt::IBeamCursor);

    ui->m_path->setText(tr("path:"));
    ui->m_path->setFixedHeight(30);

  //  ui->v_path->setWordWrap(true);
    ui->v_path->setTextInteractionFlags(Qt::TextSelectableByMouse);
    ui->v_path->setCursor(Qt::IBeamCursor);

    ui->scrollArea->setFixedHeight(165);
    ui->scrollArea->setFixedWidth(332);
    ui->scrollArea->setWidgetResizable(true);
    ui->scrollArea->setFrameShape(QFrame::NoFrame);
//    ui->scrollArea->setStyleSheet("QScrollArea {background-color:transparent;}");
//    ui->scrollArea->viewport()->setStyleSheet("background-color:transparent;");
    ui->scrollArea->verticalScrollBar()->setProperty("drawScrollBarGroove", false);
    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scrollArea->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);

    installEventFilter(this);
    connect(g_gsettings, &QGSettings::changed, this, &MediaInfoDialog::gSettingChanged);
    gSettingChanged("systemFont");
}

MediaInfoDialog::~MediaInfoDialog()
{
    delete ui;
}

void MediaInfoDialog::updateDate()
{
    QFontMetrics font(ui->v_title->font());
    // 实际显示区域的宽度
    int nLabelSize = ui->scrollArea->width() - findMaxSize() - 40;
    // 处理标题、路径这两个较长的字符串，实现在固定的长度换行（对于长英文、数字、混合，QLabel无法准确实现自动换行）
    QString qstrTitle = ProcessText(font, m_title, nLabelSize);
    ui->v_title->setText(qstrTitle);

    ui->v_type->setText(m_type);
    ui->v_size->setText(m_size);
    ui->v_duration->setText((m_duration));

    QString qstrFilePath = ProcessText(font, m_filePath, nLabelSize);
    ui->v_path->setText(qstrFilePath);
}


void MediaInfoDialog::showEvent(QShowEvent *event)
{
    return QDialog::showEvent(event);
}

void MediaInfoDialog::on_btn_ok_clicked()
{
    hide();
}

void MediaInfoDialog::gSettingChanged(QString key)
{
    if (key == "systemFontSize" || key == "systemFont") {
        // 字体改变
        QString fontFamily = g_gsettings->get("systemFont").toString();
        double fontSize = g_gsettings->get("systemFontSize").toDouble();
        qDebug() << "font_size: " << fontSize;
        QFont f(fontFamily);
        f.setPointSizeF(fontSize + 2);
        ui->label_2->setFont(f);
        f.setPointSizeF(fontSize);
        ui->m_title->setFont(f);
        ui->m_type->setFont(f);
        ui->m_size->setFont(f);
        ui->m_duration->setFont(f);
        ui->m_path->setFont(f);
        ui->v_title->setFont(f);
        ui->v_type->setFont(f);
        ui->v_size->setFont(f);
        ui->v_duration->setFont(f);
        ui->v_path->setFont(f);
        updateDate();
    }
}


QString MediaInfoDialog::ProcessText(const QFontMetrics &font, const QString &text, int nLabelSize)
{
    int nTextSize = font.width(text);
    if(nTextSize > nLabelSize){
        int nPos = 0;
        long nOffset = 0;
        for(int i = 0; i < text.size(); i++){
            nOffset += font.width(text.at(i));
            if(nOffset > nLabelSize){
                nPos = i;
                break;
            }
        }
        QString qstrLeftData = text.left(nPos);
        QString qstrMidData = text.mid(nPos);
        return qstrLeftData + "\n" + ProcessText(font, qstrMidData, nLabelSize);
    }
    return text;
}

int MediaInfoDialog::findMaxSize()
{
    QFontMetrics font(ui->v_title->font());
    int sizeTitle = font.width(tr("title:"));
    int sizeType = font.width(tr("type:"));
    int sizeSize = font.width(tr("size:"));
    int sizeDuration = font.width(tr("duration:"));
    int sizePath = font.width(tr("path:"));

    int tempMax1 = sizeTitle >= sizeType ? sizeTitle : sizeType;
    int tempMax2 = sizeSize >= sizeDuration ? sizeSize : sizeDuration;
    int max = tempMax1 >= tempMax2 ? tempMax1 :tempMax2;
    return max >= sizePath ? max : sizePath;
}
