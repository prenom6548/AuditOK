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

#include "popupDialog.h"
#include "widgetstyle.h"
#include "UI/mainwidget.h"
#include "xatom-helper.h"
#include "UI/globalsignal.h"
#include <ukui-log4qt.h>
#include <ukuistylehelper/ukuistylehelper.h>
#include <ukuiwindowhelper/ukuiwindowhelper.h>

#define PT_12 12
#define PT_11 11

QRegExp gInvalidName("[\\\\/:\\*\\?\\\"&<>\\|]");/* 文件名或文件夹名中不能出现以下字符：\、/、:、*、?、"、&、<、>、|  */

PopupDialog::PopupDialog(QWidget *parent) : KDialog(parent)
{
    inpupdialog();
    dlgcolor();
}

void PopupDialog::closeDialog()
{
    this->close();
    enterLineEdit->clear();
}

void PopupDialog::slotTextChanged(QString text)
{
    KyInfo() << "text = " << text;
//    if (!nameIsValid(text) && text != "") {
//        QMessageBox::warning(Widget::mutual,tr("Prompt information"),tr("Could not contain characters: \ / : * ? \" & < > |"), QMessageBox::Ok);

//        enterLineEdit->cursorBackward(true, 1);
//        enterLineEdit->del();
//    }

//    enterLineEdit->setLabelNumber(15 - enterLineEdit->text().length());
    if (text.length() == 15) {
        tips->setText(tr("Reached upper character limit"));
        tips->setStyleSheet("QLabel{color:#F44E50;}");
    } else {
        tips->setText("");
    }
}

void PopupDialog::inpupdialog()
{
    this->setWindowTitle(tr("Music Player"));
    this->setWindowIcon("kylin-music");
    this->setFixedSize(376, 222);
    kdk::UkuiStyleHelper::self()->removeHeader(this);
    this->setWindowModality(Qt::ApplicationModal); //弹出自定义对话框时主界面不可操作
    testLayout = new QVBoxLayout();


    titleLayout = new QHBoxLayout();
    titleLab = new QLabel(this);
    titleLab->setAccessibleName("kylin-music_QLabel_titleLab");
    closeBtn = new QToolButton(this);
    closeBtn->setAccessibleName("kylin-music_popupdialog_QToolButton_closeBtn");
    closeBtn->setFixedSize(30,30);
    closeBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));
    closeBtn->setProperty("isWindowButton", 0x2);
    closeBtn->setProperty("useIconHighlightEffect", 0x8);
    closeBtn->setAutoRaise(true);
    closeBtn->hide();

    titleLab->setText(tr("Prompt information"));

    titleLayout->addWidget(titleLab,Qt::AlignLeft);
    titleLayout->addWidget(closeBtn,0,Qt::AlignRight);

    titleLayout->setMargin(0);
    titleLayout->setSpacing(0);

    enterLineEdit = new QLineEdit;
    enterLineEdit->setAccessibleName("kylin-music_QLineEdit_enterLineEdit");
    enterLineEdit->setFixedSize(328, 40);
    enterLineEdit->setMaxLength(15);

    tips = new QLabel(this);
    tips->setAccessibleName("kylin-music_QLabel_tips");
    tips->setFixedHeight(40);


    confirmBtn = new QPushButton(this);
    confirmBtn->setAccessibleName("kylin-music_QPushButton_confirmBtn");
    cancelBtn = new QPushButton(this);
    cancelBtn->setAccessibleName("kylin-music_QLabelQPushButton_cancelBtn");
    btnLayout = new QHBoxLayout();

    stackWid = new QStackedWidget();
    stackWid->setAccessibleName("kylin-music_popupdialog_QStackedWidget_stackWid");
    stackWid->addWidget(enterLineEdit);

    stackWid->setFixedSize(328,40);
    QString message_confirmBtn = tr("Confirm");
    confirmBtn->setText(message_confirmBtn);
    confirmBtn->setFixedSize(105,36);
    confirmBtn->setProperty("isImportant", true);
    confirmBtn->setEnabled(false);
    this->displayBtn(confirmBtn, message_confirmBtn);

    QString message_cancelBtn = tr("Cancel");
    cancelBtn->setText(message_cancelBtn);
    cancelBtn->setFixedSize(105,36);
    this->displayBtn(cancelBtn, message_cancelBtn);

    btnLayout->addWidget(cancelBtn, 0, Qt::AlignRight);
    btnLayout->addWidget(confirmBtn, Qt::AlignRight);
    btnLayout->setSpacing(16);
    btnLayout->setMargin(0);

    enterLineEdit->setPlaceholderText(tr("Please input playlist name:"));

    testLayout->setMargin(0);
    testLayout->setSpacing(0);
    testLayout->addLayout(titleLayout);
    testLayout->addSpacing(8);
    testLayout->addWidget(stackWid);
    testLayout->addSpacing(2);
    testLayout->addWidget(tips);
    testLayout->addSpacing(14);
    testLayout->addLayout(btnLayout);
    testLayout->setContentsMargins(24, 16, 24, 30);

    this->mainWidget()->setLayout(testLayout);


//    connect(addSongListBtn, SIGNAL(clicked(bool)), this, SLOT(addSongList()));
    connect(enterLineEdit,SIGNAL(textChanged(QString)),this,SLOT(slotTextChanged(QString)));
    connect(closeBtn,SIGNAL(clicked(bool)),this,SLOT(closeDialog()));
    connect(cancelBtn,SIGNAL(clicked(bool)),this,SLOT(closeDialog()));
    connect(enterLineEdit,SIGNAL(textChanged(QString)),this,SLOT(slotConfirmBtnStatus()));
}

void PopupDialog::displayBtn(QPushButton *btn, QString btnTooltip)
{
#ifndef WHITE_TEST
    QLocale locale;
    if(locale.language()==QLocale::Tibetan)
    {
        btn->setText(btnTooltip);
        QFontMetrics  fontMetrics(btn->font());
        int fontSize = fontMetrics.width(btnTooltip);
        QString tmpStr = btnTooltip;
        if (fontSize > confirmBtn->width()) {
            tmpStr = btnTooltip.left(btnTooltip.length() * (btn->width()) / fontSize   - 7)  + "...";
            btn->setText(tmpStr);
            btn->setToolTip(btnTooltip);
        } else {
            btn->setText(btnTooltip);
        }
    }
#endif
}

void PopupDialog::slotConfirmBtnStatus()
{
    if(enterLineEdit->text().isEmpty())
    {
        confirmBtn->setEnabled(false);
    } else
    {
        confirmBtn->setEnabled(true);
    }
}

void PopupDialog::slotLableSetFontSize(int size)
{
    //默认大小12px,换算成pt为9
    double lableBaseFontSize = PT_11;//魔鬼数字，自行处理
    double nowFontSize = lableBaseFontSize * double(size) / 11;//11为系统默认大小，魔鬼数字，自行处理
    QFont font;
    font.setPointSizeF(nowFontSize);
    tips->setFont(font);

    QString message_confirmBtn = tr("Confirm");
    confirmBtn->setText(message_confirmBtn);
    this->displayBtn(confirmBtn, message_confirmBtn);
    QString message_cancelBtn = tr("Cancel");
    cancelBtn->setText(message_cancelBtn);
    this->displayBtn(cancelBtn, message_cancelBtn);
}

void PopupDialog::dlgcolor()
{
    return;

    if(WidgetStyle::themeColor == 1 ) {
        this->setStyleSheet("#TitleBar{background-color:#252526;}");

    } else if(WidgetStyle::themeColor == 0) {
        this->setStyleSheet("#TitleBar{background-color:#FFFFFF;}");
    }
}

bool PopupDialog::nameIsValid(QString textName)
{
    if (nullptr == textName) {
        return false;
    }

    return !textName.contains(gInvalidName);
}


MusicInfoDialog::MusicInfoDialog(QWidget* parent, musicDataStruct date) :
    KDialog(parent)
{
    this->setAutoFillBackground(true);
    this->setBackgroundRole(QPalette::Base);
    this->setWindowTitle(tr("Music Player"));
    this->setWindowIcon("kylin-music");
    this->setWindowFlags(Qt::Tool);
    kdk::UkuiStyleHelper::self()->removeHeader(this);

    UkuiWindowHelper* helper = new UkuiWindowHelper(this);
    helper->setSkipSwitcher(true);
    helper->setSkipTaskBar(true);

    musicDate = date;
    initStyle();
}

MusicInfoDialog::~MusicInfoDialog()
{
}

QString MusicInfoDialog::feed(QString str,int maxWidth)
{

        QString strText = str;
        int AutoIndex =1;
        QFont font ;
        font.setPointSize(13);
        QFontMetrics fm(font);
        qDebug()<<mainWig->width();
        if(!strText.isEmpty()){
            for(int i =1;i<strText.size()+1;i++){
                if(fm.width(strText.left(i))>(maxWidth +80)*AutoIndex){
                    qDebug()<<"111";
                    strText.insert(i - 1,"\n");
                    AutoIndex++;
                }
            }
        }
        return strText;
}
void MusicInfoDialog::initStyle()
{
    this->setFixedSize(376,435);
    mainVLayout = new QVBoxLayout(this);
    songNameLabel = new QLabel(this);
    songNameLabel->setAccessibleName("kylin-music_QLabel_songNameLabel");
    singerLabel = new QLabel(this);
    singerLabel->setAccessibleName("kylin-music_QLabel_singerLabel");
    albumLabel = new QLabel(this);
    albumLabel->setAccessibleName("kylin-music_QLabel_albumLabel");
    filePathLabel = new QLabel(this);
    filePathLabel->setAccessibleName("kylin-music_QLabel_filePathLabel");
    fileTypeLabel = new QLabel(this);
    fileTypeLabel->setAccessibleName("kylin-music_QLabel_fileTypeLabel");
    fileTimeLabel = new QLabel(this);
    fileTimeLabel->setAccessibleName("kylin-music_QLabel_fileTimeLabel");
    fileSizeLable = new QLabel(this);
    fileSizeLable->setAccessibleName("kylin-music_QLabel_fileSizeLable");
    musicInfoLabel = new QLabel(this);
    musicInfoLabel->setAccessibleName("kylin-music_QLabel_musicInfoLabel");
    musicInfoLabel->setText(tr("  Music Information"));
    musicInfoLabel->setFixedHeight(24);
    QFont infoTitleFont = musicInfoLabel->font();
    infoTitleFont.setPointSize(16);
    infoTitleFont.setBold(true);
    musicInfoLabel->setFont(infoTitleFont);

    QFont font;
    font.setPointSize(13);
    songNameLabel->setFixedHeight(28);
    singerLabel->setFixedHeight(28);
    albumLabel->setFixedHeight(28);
    fileTypeLabel->setFixedHeight(28);
    fileSizeLable->setFixedHeight(28);
    fileTimeLabel->setFixedHeight(28);
    filePathLabel->setFixedHeight(28);
    filePathLabel->setWordWrap(true);
    songNameLabel->setText(tr("Song Name : "));
    singerLabel->setText(tr("Singer : "));
    albumLabel->setText(tr("Album : "));
    fileTypeLabel->setText(tr("File Type : "));
    fileSizeLable->setText(tr("File Size : "));
    fileTimeLabel->setText(tr("File Time : "));
    filePathLabel->setText(tr("File Path : "));
//    filePathLabel->setAlignment(Qt::AlignTop);
//    filePathLabel->setToolTip(musicDate.filepath);
    //filePathLabel->setWordWrap(true); 这句很重要
   // filePathLabel->setAlignment(Qt::AlignLeft);

    buttonLayout = new QHBoxLayout();
    cancelBtn = new QPushButton(this);
    cancelBtn->setAccessibleName("kylin-music_QPushButton_cancelBtn");
    confirmBtn = new QPushButton(this);
    confirmBtn->setAccessibleName("kylin-music_QPushButton_confirmBtn");
    btnWidget = new QWidget(this);
    btnWidget->setAccessibleName("kylin-music_musicinfodialog_QWidget_btnWidget");
    QFormLayout *mainFormLayout = new QFormLayout(this);
    //main = new QWidget(this);
   // mainFormLayout->setRowWrapPolicy(QFormLayout::WrapAllRows);
    mainWig = new QWidget(this);
    mainWig->setAccessibleName("kylin-music_popupdialog_QWidget_mainWig");
    mainWig->setLayout(mainFormLayout);
    //mainFormLayout->addWidget(songNameLabel,0,0);
    songNameLabel->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
//    mainFormLayout->setAlignment(main,Qt::AlignTop);
    mainFormLayout->setWidget(0,QFormLayout::LabelRole,songNameLabel);
    //mainFormLayout->setSpacing( 5);
    QLabel *name = new QLabel(this);
    name->setAccessibleName("kylin-music_QLabel_name");
    name->setWordWrap(true);
    QString m_name = feed(musicDate.title,mainWig->width());
    name->setText(m_name);
    name->setToolTip(musicDate.title);
    name->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
    CustomLabel *singer = new CustomLabel(musicDate.singer);
    singer->setAccessibleName("kylin-music_popupdialog_CustomLabel_singer");
    singer->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
    CustomLabel *album = new CustomLabel(musicDate.album);
    album->setAccessibleName("kylin-music_popupdialog_CustomLabel_album");
    album->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
    CustomLabel *filetype = new CustomLabel(musicDate.filetype);
    filetype->setAccessibleName("kylin-music_popupdialog_CustomLabel_filetype");
    filetype->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
    CustomLabel *size = new CustomLabel(musicDate.size);
    size->setAccessibleName("kylin-music_popupdialog_CustomLabel_size");
    size->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
    CustomLabel *time = new CustomLabel(musicDate.time);
    time->setAccessibleName("kylin-music_popupdialog_CustomLabel_time");
    time->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
    mainFormLayout->setWidget(0,QFormLayout::FieldRole,name);
    mainFormLayout->setWidget(1,QFormLayout::LabelRole,singerLabel);
    mainFormLayout->setWidget(1,QFormLayout::FieldRole,singer);
    mainFormLayout->setWidget(2,QFormLayout::LabelRole,albumLabel);
    mainFormLayout->setWidget(2,QFormLayout::FieldRole,album);
    mainFormLayout->setWidget(3,QFormLayout::LabelRole,fileTypeLabel);
    mainFormLayout->setWidget(3,QFormLayout::FieldRole,filetype);
    mainFormLayout->setWidget(4,QFormLayout::LabelRole,fileSizeLable);
    mainFormLayout->setWidget(4,QFormLayout::FieldRole,size);
    mainFormLayout->setWidget(5,QFormLayout::LabelRole,fileTimeLabel);
    mainFormLayout->setWidget(5,QFormLayout::FieldRole,time);
    mainFormLayout->setWidget(6,QFormLayout::LabelRole,filePathLabel);
    pathLabel = new QLabel(this);
    pathLabel->setAccessibleName("kylin-music_QLabel_pathLabel");

//    QString m_path = feed(musicDate.filepath,mainWig->width());

    pathLabel->setText(musicDate.filepath);
    pathLabel->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
    //pathLabel->setAlignment();
    qDebug()<<"路径是"<<musicDate.filepath;
    pathLabel->setToolTip(musicDate.filepath);
    pathLabel->setWordWrap(true);
    pathLabel->setMinimumWidth(200);
    mainFormLayout->setWidget(6,QFormLayout::FieldRole,pathLabel);
    mainFormLayout->setSpacing(3);


    connect(cancelBtn,&QPushButton::clicked,this,&MusicInfoDialog::close);
    connect(confirmBtn,&QPushButton::clicked,this,&MusicInfoDialog::close);



    confirmBtn->setText(tr("Confirm"));
    confirmBtn->setFixedSize(105,36);
    cancelBtn->setText(tr("Cancel"));
    cancelBtn->setFixedSize(105,36);
    confirmBtn->setProperty("isImportant", true);

    btnWidget->setLayout(buttonLayout);

    buttonLayout->setAlignment(Qt::AlignRight);
    buttonLayout->addWidget(cancelBtn);
    buttonLayout->addWidget(confirmBtn);
    buttonLayout->setMargin(0);
    buttonLayout->setSpacing(16);

    //QWidget *containerWidget = mainWig;
    qDebug()<<mainWig->height()<<this->height();
//        if(mainWig->height() >= this->height()){

//            QScrollArea *filePathScrollArea = new QScrollArea(this);
//            filePathScrollArea->setWidgetResizable(true);
//            filePathScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//            filePathScrollArea->setWidget(mainWig);
//            filePathScrollArea->setFrameShape(QFrame::NoFrame);
//            filePathScrollArea->setFrameShadow(QFrame::Plain);
//            filePathScrollArea->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
//            containerWidget = filePathScrollArea;
//        }
    mainVLayout->setAlignment(Qt::AlignTop);
    mainVLayout->addWidget(musicInfoLabel);
    mainVLayout->addSpacing(5);

    QFontMetrics m(font);
    qDebug()<<"宽度是"<<m.width(musicDate.filepath);
    //if(m.width(musicDate.filepath) >= 360){ //当路径宽度大于等于360时候，加上滚动条
        filePathScrollArea = new QScrollArea(this);
        filePathScrollArea->setAccessibleName("kylin-music_popupdialog_QScrollArea_filePathScrollArea");
        filePathScrollArea->setWidgetResizable(true);
        filePathScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        filePathScrollArea->setWidget(mainWig);
        filePathScrollArea->setFrameShape(QFrame::NoFrame);
        filePathScrollArea->setFrameShadow(QFrame::Plain);
        filePathScrollArea->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
        mainVLayout->addWidget(filePathScrollArea);
    //}else{
    //    mainVLayout->addWidget(mainWig);
    //}

    mainVLayout->addSpacing(30);
    mainVLayout->addWidget(btnWidget);
    //mainVLayout->addWidget(filePathScrollArea);

    mainVLayout->setSpacing(6);
    mainVLayout->setContentsMargins(20,20,24,22); // 为了和主题qmessagebox间距一致，而不参照UI图间距
    this->mainWidget()->setLayout(mainVLayout);
}

void MusicInfoDialog::slotLableSetFontSize(int size)
{
    //默认大小12px,换算成pt为9
    double lableBaseFontSize = PT_12;//魔鬼数字，自行处理
    double nowFontSize = lableBaseFontSize * double(size) / 11;//11为系统默认大小，魔鬼数字，自行处理
    QFont font;
    font.setPointSizeF(nowFontSize);
    musicInfoLabel->setFont(font);
    if (pathLabel) {
        pathLabel->setFont(font);
        pathLabel->setText(musicDate.filepath);
        pathLabel->adjustSize();
        pathLabel->update();

    }
}

void PopupDialog::confirmCheck()
{
    QString text = enterLineEdit->text();
    if (!nameIsValid(text) && text != "") {
#ifndef WHITE_TEST
        QMessageBox::warning(Widget::mutual,tr("Prompt information"),tr("Could not contain characters: \ / : * ? \" & < > |"), QMessageBox::Ok);
#endif
        return;
    }
    Q_EMIT accessConfirmCheck();

}
