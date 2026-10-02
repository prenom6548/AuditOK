#include "renamewidget.h"
#include "mainwindow.h"
// #include "xatom-helper.h"
#include "tools.h"
#include <QRegExp>
#include <ukuistylehelper/ukuistylehelper.h>

RenameWidget::RenameWidget(QWidget *parent) : QWidget(parent)
{
    init();
    initAction();
}

void RenameWidget::init()
{
    // 总布局
    m_mainLayout = new QVBoxLayout();
    // 标题栏
    m_titleWidget = new QWidget(this);
    m_titleWidget->setAccessibleName("kylin-recorder_renamewidget_QWidget_m_titleWidget");
    m_titleLayout = new QHBoxLayout();
    titleIcon = new QPushButton();
    titleIcon->setAccessibleName("kylin-recorder_QPushButton_titleIcon");
    titleText = new QLabel();
    titleText->setAccessibleName("kylin-recorder_QLabel_titleText");
    m_titleButtonExit = new QToolButton();
    m_titleButtonExit->setAccessibleName("kylin-recorder_renamewidget_QToolButton_m_titleButtonExit");
    // 重命名界面
    m_bodyLayout = new QVBoxLayout();
    m_checkButtonLayout = new QHBoxLayout();

    m_renameLabel = new QLabel(this);
    m_renameLabel->setAccessibleName("kylin-recorder_QLabel_m_renameLabel");
    m_renameLabel->setText(tr("Rename"));
    m_font = QFont();
    m_font.setBold(true);
    m_renameLabel->setFont(m_font);
    m_renameLayout = new QHBoxLayout();

    m_LineEditLayout = new QHBoxLayout();
    m_renameLineEdit = new QLineEdit(this);
    m_renameLineEdit->setAccessibleName("kylin-recorder_QLineEdit_m_renameLineEdit");
    m_nameLengthCount = new QLabel(m_renameLineEdit);
    m_nameLengthCount->setAccessibleName("kylin-recorder_QLabel_m_nameLengthCount");

    m_wordExceedLabel = new QLabel(this);
    m_wordExceedLabel->setAccessibleName("kylin-recorder_QLabel_m_wordExceedLabel");
    m_wordExceedLabel->setText(tr("The word limit is exceeded"));
    QPalette m_pt;
    m_pt.setColor(QPalette::WindowText, Qt::red);
    m_wordExceedLabel->setPalette(m_pt);
    m_wordExceedLabel->hide();
    m_wordExceedLayout = new QHBoxLayout();

    m_confirmButton = new QPushButton();
    m_confirmButton->setAccessibleName("kylin-recorder_QPushButton_m_confirmButton");
    m_cancelButton = new QPushButton();
    m_cancelButton->setAccessibleName("kylin-recorder_QPushButton_m_cancelButton");

    // 设置按下Enter=确认
    connect(m_renameLineEdit, &QLineEdit::returnPressed, m_confirmButton, &QPushButton::click);
    // 按下确认则改名
    connect(m_confirmButton, &QPushButton::clicked, this, &RenameWidget::renameSlots);
    // 显示 renameLineedit编辑框中的字符数量
    connect(m_renameLineEdit, &QLineEdit::textChanged, this, &RenameWidget::countLengthSlots);

}

void RenameWidget::initAction()
{
    setTitleBar();
    setBodyLayout();
    setmainLayout();
}

void RenameWidget::setTitleBar()
{
    initTitleIcon();
    initButtonExit();
    titleText->setText(tr(m_appShowingName.toLocal8Bit()));
    setTitleLayout();
}

void RenameWidget::initTitleIcon()
{
    titleIcon->setFixedSize(24, 24);
    titleIcon->setIconSize(QSize(26, 26));
    titleIcon->setIcon(QIcon::fromTheme("kylin-recorder"));
    titleIcon->setStyleSheet("QPushButton{border:0px; background:transparent;}");
}

void RenameWidget::initButtonExit()
{
    m_titleButtonExit->setFixedSize(25, 25);
    m_titleButtonExit->setIcon(QIcon::fromTheme("window-close-symbolic"));
    m_titleButtonExit->setProperty("isWindowButton", 0x2);
    m_titleButtonExit->setProperty("useIconHighlightEffect", 0x8);
    m_titleButtonExit->setAutoRaise(true);

    connect(m_titleButtonExit, &QPushButton::clicked, [=](){
        this->close();
    });
}

void RenameWidget::setTitleLayout()
{
    m_titleWidget->setFixedHeight(30);
    m_titleLayout->setSpacing(0);
    m_titleLayout->setMargin(4);
    m_titleLayout->addSpacing(4);
    m_titleLayout->addWidget(titleIcon, 0, Qt::AlignCenter);
    m_titleLayout->addSpacing(8);
    m_titleLayout->addWidget(titleText, 0, Qt::AlignCenter);
    m_titleLayout->addStretch();
    m_titleLayout->addWidget(m_titleButtonExit, 0, Qt::AlignBottom);

    m_titleWidget->setLayout(m_titleLayout);
}

void RenameWidget::setBodyLayout()
{
    m_confirmButton->setText(tr("confirm"));
    m_confirmButton->setFixedWidth(96);
    m_cancelButton->setText(tr("cancle"));
    m_cancelButton->setFixedWidth(96);
    // 设置输入框和限制输入的长度
    m_renameLineEdit->setFixedSize(360, 36);
    m_renameLineEdit->setMaxLength(30);
    m_renameLineEdit->setFocus();
    m_nameLengthCount->setFixedHeight(m_renameLineEdit->height());
    m_nameLengthCount->move(330, 0);

    QPalette m_pt;
    m_pt.setColor(QPalette::Text, QColor(140, 140, 140));
    m_nameLengthCount->setPalette(m_pt);

    m_wordExceedLayout->addWidget(m_wordExceedLabel);
    m_wordExceedLayout->setContentsMargins(35, 0, 0, 0);

    //layout布局
    m_renameLayout->addWidget(m_renameLabel);
    m_renameLayout->setContentsMargins(35, 0, 0, 10);
    m_checkButtonLayout->setSpacing(16);
    m_seatLabel = new QLabel(this);    //占位标签
    m_seatLabel->setAccessibleName("kylin-recorder_QLabel_m_seatLabel");
    m_checkButtonLayout->setContentsMargins(0, 0, 25, 0);
    m_checkButtonLayout->addWidget(m_seatLabel, 2, Qt::AlignCenter);
    m_checkButtonLayout->addWidget(m_cancelButton, 1, Qt::AlignCenter);
    m_checkButtonLayout->addWidget(m_confirmButton, 1, Qt::AlignCenter);

    m_bodyLayout->addStretch();
    m_bodyLayout->addLayout(m_renameLayout);
    m_LineEditLayout->addWidget(m_renameLineEdit);
    m_bodyLayout->addLayout(m_LineEditLayout);
    m_bodyLayout->addLayout(m_wordExceedLayout);
    m_bodyLayout->addStretch();
    m_bodyLayout->addLayout(m_checkButtonLayout);
    m_bodyLayout->addStretch();

    connect(m_cancelButton, &QPushButton::clicked, [=](){
        this->close();
    });
}

void RenameWidget::setmainLayout()
{
    this->setWindowFlag(Qt::Tool);
    this->setFixedSize(420, 230);
    this->setAttribute(Qt::WA_ShowModal, true);
    m_mainLayout->setMargin(0);
    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(m_titleWidget,Qt::AlignTop);
    m_mainLayout->addLayout(m_bodyLayout);
    this->setLayout(m_mainLayout);
    this->move(MainWindow::mutual->geometry().center() - this->rect().center());
}


bool RenameWidget::checkRenameable(QFileInfo *fileinfo, QString new_name)
{
    // 检查根目录下是否存在同名文件
    QString new_file = fileinfo->path() + '/' + new_name + '.' + fileinfo->completeSuffix();
    QFileInfo* new_file_info = new QFileInfo(new_file);  
    return new_file_info->exists();

}

bool RenameWidget::checkLegal(QString new_rename)
{
    /* 文件名或文件夹名中不能出现以下字符：\、/、:、*、?、"、<、>、| 、\n 、\t */
    QRegExp gInvalidName("[\\\\/:\\*\\?\\\"<>\\|\\n\\t]");
    if(new_rename.contains(gInvalidName) || new_rename.contains(" ") || new_rename == ""){
       return false;
    }else{
       return true;
    }
}

void RenameWidget::renameSlots()
{
#ifndef WHITE_TEST
    new_name = m_renameLineEdit->text();
    if(new_name == fileInfo->baseName()){
        // 判断用户是否对标题做出修改，如果未做出修改就点确定，则直接关闭窗口即可，无需更改数据库、源文件等。
        this->close();
    }else{
        if(!(checkLegal(new_name) && m_wordExceedLabel->isHidden())){
                QMessageBox::warning(this, tr("Alert"), tr("Illegal naming!"));
            }else{
            if(!checkRenameable(fileInfo, new_name)){
                    emit DATABASE_INSTANCE.updateNameSignal(*fileInfo, new_name);
                    // 重命名文件
                    QFile file(fileInfo->absoluteFilePath());
                    QString newfilepath = fileInfo->path()+ "/" + new_name + "." +fileInfo->completeSuffix();
                    file.rename(fileInfo->absoluteFilePath(), newfilepath);
                    new_name = new_name + "." +fileInfo->completeSuffix();
                    emit changeLabel(newfilepath);
                    this->close();
                    }else{
                        QMessageBox::information(this, tr("Alert"), tr("The current file name already exists in the default path."));
                    }
        }
    }
#endif
}


void RenameWidget::countLengthSlots()
{
    QString name = m_renameLineEdit->text();
    m_nameLengthCount->setText(QString::number(fixedNameLength - name.length()));
    if(fixedNameLength - name.length() < 0){
        QPalette m_pt;
        m_pt.setColor(QPalette::Text, Qt::red);
        m_nameLengthCount->setPalette(m_pt);
        m_wordExceedLabel->show();
    }else{
        QPalette m_pt;
        m_pt.setColor(QPalette::Text, QColor(140, 140, 140));
        m_nameLengthCount->setPalette(m_pt);
        m_wordExceedLabel->hide();
    }
}

void RenameWidget::setLineEditContent(QString m_filePath)
{
    fileInfo = new QFileInfo(m_filePath);
    m_renameLineEdit->setText(fileInfo->baseName());
    m_renameLineEdit->selectAll();
    this->show();
    kdk::UkuiStyleHelper::self()->removeHeader(this);
}

QString RenameWidget::getNewName()
{
    return new_name;
}
