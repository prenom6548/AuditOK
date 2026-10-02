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
#include "setupcodec.h"

#include "global/global.h"

#include <QDebug>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QVBoxLayout>
#include <QIntValidator>

using namespace Global;

SetupCodec::SetupCodec(QWidget *parent) :
    ThemeWidget(parent)
{
    initLayout();
    QIntValidator *ivd = new QIntValidator;
    ivd->setRange(1, 32);
    m_threadEdit->setValidator(ivd);
    m_threadEdit->setFixedHeight(30);

    initData();
    initConnect();
    connectThemeSetting();
}

SetupCodec::~SetupCodec()
{
    if (m_threadEdit->validator()) {
        delete m_threadEdit->validator();
    }
}

void SetupCodec::initData()
{
    QStringList items;
    if(!unsupportedVDPAUCOPY){
        items << tr("default") << tr("no") << "vdpau" << "vdpau-copy" << "vaapi" << "vaapi-copy";
    } else{
        items << tr("default") << tr("no") << "vdpau" << "vaapi" << "vaapi-copy";
    }
    m_comboBoxDecoder->addItems(items);
    m_comboBoxDecoder->setCurrentText(tr(g_settings->value("General/video_decoder").toString().toStdString().c_str()));

    m_label1->setText(tr("Video output"));
    m_comboBoxOutput->addItem(tr("auto"));
    m_comboBoxOutput->addItem("x11");
    m_comboBoxOutput->addItem("xv");
    m_comboBoxOutput->addItem("vdpau");
    m_comboBoxOutput->addItem("vaapi");
    if (g_config->hardwareType() != GlobalConfig::LONGXIN_INTEGRATED_GRAPHICS)
        m_comboBoxOutput->addItem("gpu");
    m_comboBoxOutput->setCurrentText(g_settings->value("General/video_output").toString());

    m_label2->setText(tr("Video decoder"));

    m_label3->setText(tr("Decode threads"));
    int threads = g_settings->value("General/video_decode_threads").toInt();
    m_threadEdit->setText(QString::number(threads > 0 ? threads : 4));
}

void SetupCodec::initConnect()
{
    connect(m_comboBoxDecoder, &QComboBox::currentTextChanged, this, [this](QString decoder) {
        g_config->videoDecoder.second = decoder;
    });
    connect(m_comboBoxOutput, &QComboBox::currentTextChanged, this, [this](QString decoder) {
        g_config->videoOutput.second = decoder;
    });
    connect(m_threadEdit, &QLineEdit::textChanged, this, [this](QString text){
        g_config->videoDecodeThreads.second = text.toInt();
    });
}

void SetupCodec::setBlackTheme()
{
    m_label1->setStyleSheet(QString("color:rgb(249,249,249);"));
    m_label2->setStyleSheet(QString("color:rgb(249,249,249);"));
    m_label3->setStyleSheet(QString("color:rgb(249,249,249);"));

//    m_comboBoxOutput->setStyleSheet(QString("color:rgb(249,249,249);background-color:rgb(64,64,64);"));
//    m_comboBoxDecoder->setStyleSheet(QString("color:rgb(249,249,249);background-color:rgb(64,64,64);"));
//    m_threadEdit->setStyleSheet(QString("color:rgb(249,249,249);background-color:rgb(64,64,64);"));
//    ui->cb_ADecoder->setStyleSheet(QString("color:rgb(249,249,249);background-color:rgb(64,64,64);"));
}

void SetupCodec::setDefaultTheme()
{
    m_label1->setStyleSheet(QString("color:rgb(38,38,38);"));
    m_label2->setStyleSheet(QString("color:rgb(38,38,38);"));
    m_label3->setStyleSheet(QString("color:rgb(38,38,38);"));

//    m_comboBoxOutput->setStyleSheet(QString("color:rgb(38,38,38);background-color:rgb(240,240,240);"));
//    m_comboBoxDecoder->setStyleSheet(QString("color:rgb(38,38,38);background-color:rgb(240,240,240);"));
//    m_threadEdit->setStyleSheet(QString("color:rgb(38,38,38);background-color:rgb(240,240,240);"));
    //    ui->cb_ADecoder->setStyleSheet(QString("color:rgb(38,38,38);background-color:rgb(240,240,240);"));
}

void SetupCodec::setWidgetFont(QString family, int size)
{
    QFont f(family);
    f.setPointSize(size + 2);
    m_label1->setFont(f);
    m_label2->setFont(f);
    m_label3->setFont(f);

    f.setPointSize(size);
    m_comboBoxDecoder->setFont(f);
    m_comboBoxOutput->setFont(f);
    m_threadEdit->setFont(f);
}

void SetupCodec::initLayout()
{
    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setContentsMargins(30, 10, 24, 8);
    lay->setSpacing(12);

    m_label1 = new QLabel;
    m_label1->setAccessibleName("kylin-video_QLabel_m_label1");
    lay->addWidget(m_label1);

    m_comboBoxOutput = new QComboBox;
    m_comboBoxOutput->setAccessibleName("kylin-video_setupcodec_QComboBox_m_comboBoxOutput");
    lay->addWidget(m_comboBoxOutput);

    m_label1->hide();
    m_comboBoxOutput->hide();

    if (g_config->videoOutputType() == GlobalConfig::VO_WID) {
        m_label1->show();
        m_comboBoxOutput->show();
    }

    m_label2 = new QLabel;
    m_label2->setAccessibleName("kylin-video_QLabel_m_label2");
    lay->addWidget(m_label2);

    m_comboBoxDecoder = new QComboBox;
    m_comboBoxDecoder->setAccessibleName("kylin-video_setupcodec_QComboBox_m_comboBoxDecoder");
    lay->addWidget(m_comboBoxDecoder);

    m_label3 = new QLabel;
    m_label3->setAccessibleName("kylin-video_QLabel_m_label3");
    lay->addWidget(m_label3);

    m_threadEdit = new QLineEdit;
    m_threadEdit->setAccessibleName("kylin-video_setupcodec_QLineEdit_m_threadEdit");
    lay->addWidget(m_threadEdit);

    lay->addStretch();
}
