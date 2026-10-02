/*
 * Qt5-UKUI's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "kyfiledialogrename.h"

#include <QStackedWidget>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QTextEdit>
#include <QRadioButton>
#include <QButtonGroup>
#include <QCheckBox>
#include <QFileInfo>
#include <QStandardPaths>

#include <QApplication>
#include <QFontMetrics>
#include "file-utils.h"
#include "rename-editor.h"
#include "../kyfiledialog.h"
#include "../debug.h"

#define PEONY_TRUNCATE_NAME_LIMIT 225
#define PEONY_RENAME_LIMIT 255
using namespace UKUIFileDialog;
extern bool isFuseFileSystem(QString path);

MyTextEdit:: MyTextEdit(QWidget *parent) : QTextEdit(parent)
{
    setReadOnly(true);
    setStyleSheet("QTextEdit { border: none; }");
    setWordWrapMode(QTextOption::WrapAnywhere);
    QObject::connect(qApp, &QApplication::fontChanged, this, [&](const QFont &font){
        this->setFont(font);
    },Qt::UniqueConnection);
}

MyTextEdit::~MyTextEdit()
{

}

KyFileDialogRename::KyFileDialogRename(QWidget *parent) : KyFileOperationDialog(parent), Peony::FileOperationErrorHandler()
{
    setFixedWidth(600);
    setFixedHeight(300);
}

KyFileDialogRename::~KyFileDialogRename()
{

    if(m_content){
        delete m_content;
        m_content = nullptr;
    }
    if(m_iconTopLabel){
        delete m_iconTopLabel;
        m_iconTopLabel = nullptr;
    }
}

void KyFileDialogRename::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    if(event->type() == QEvent::Show)
    {
        // Check if m_content and m_iconTopLabel are initialized
        if (!m_content || !m_iconTopLabel) {
            return;
        }
        
        QFontMetrics fm(m_content->fontMetrics());
        QString text = m_content->text();
        int textwidth = m_content->width() - 2*m_content->contentsMargins().left();
        textwidth = qMax(textwidth, 1);
        QRect textrect = fm.boundingRect(0,0,textwidth,0, m_content->alignment() | Qt::TextWordWrap, m_content->text());

        m_iconTopLabel->setFixedHeight((m_content->height() - textrect.height()) / 2);
    }
}


void KyFileDialogRename::handle(Peony::FileOperationError &error)
{
    Peony::ExceptionResponse responseCode = Peony::ExceptionResponse::Cancel;
    QString newName = Peony::FileUtils::getUriBaseName(error.destDirUri);
    newName = Peony::FileUtils::urlDecode(newName);

    auto fsType = Peony::FileUtils::getFsTypeFromFile(m_currentUri);
    QString path = error.destDirUri.left(error.destDirUri.lastIndexOf("/"));

    if(path.startsWith("file://"))
        path = path.remove(0,7);
    if(isFuseFileSystem(path))
        fsType = "fuse.kyfs";
    pDebug << "handle path......" << path << fsType;

    QString byteText(tr("bytes"));
    QString capitalByteText(tr("Bytes"));

    if (fsType.contains("ntfs") || fsType.contains("fuse.kyfs")) {
        byteText = QString(tr("character"));
        capitalByteText = QString(tr("Character"));
    }

    auto stack = new QStackedWidget(this);
    stack->setContentsMargins(20, 0, 20, 20);
    auto layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(stack);
    mainWidget()->setLayout(layout);

    // todo:
    // 根据错误码和文件操作码拼接文件信息
    QString line1;
    QString line2;
    QString line3;

    switch (error.op) {
        case Peony::FileOpRename: {
            line1 = tr("File \"%1\"").arg(newName);
            line2 = tr("Saving failed, the reason is: %1").arg(error.errorCode == G_IO_ERROR_FILENAME_TOO_LONG?
                    tr("The file name is too long. "): error.errorStr) +
                    tr("Please choose the following processing method:");
            break;
        }
    }

    QString line;
    QStringList messages;
    line = qApp->fontMetrics().elidedText(line1, Qt::ElideMiddle, 500);

//    messages.append(line);
//    line.append('\n');
//    line2 = qApp->fontMetrics().elidedText(line2, Qt::ElideMiddle, 600);
//    messages.append(line2);
//    line = qApp->fontMetrics().elidedText(line3, Qt::ElideMiddle, 500);
//    messages.append(line);

    m_labelText = line + line2;

    // page1: save to long file directory
    auto pageOne = new QWidget(this);
//    QRadioButton *skip = new QRadioButton(tr("Skip"), this);
    QRadioButton *cutOff = new QRadioButton(tr("Truncation"), this);
    QRadioButton *save = new QRadioButton(tr("Save to long file name directory"), this);
    QRadioButton *rename = new QRadioButton(tr("Rename"), this);
    QButtonGroup *onePageGroup = new QButtonGroup(this);
    QLabel *specificationLabel = new QLabel(this);
    specificationLabel->setWordWrap(true);

//    onePageGroup->addButton(skip, 0);
    onePageGroup->addButton(rename, 0);
    onePageGroup->addButton(cutOff, 1);
    onePageGroup->addButton(save, 2);

//    save->setVisible();

    QVBoxLayout *oneLayout = new QVBoxLayout();
    m_content = new QLabel(this);
    m_content->setWordWrap(true);

    //int textWidth = (this->width() - 40 - 40 - 24) * 3;
    //QString elidedString = qApp->fontMetrics().elidedText(error.errorStr, Qt::ElideMiddle, textWidth);
    m_content->setText(m_labelText);
//    QHBoxLayout *labelLayout = new QHBoxLayout(this);

    auto labelIcon = new QLabel(this);
    labelIcon->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(16, 16));
//    labelIcon->setGeometry(0, content->y(), labelIcon->size().width(), labelIcon->size().height());
    //labelIcon->setAlignment(Qt::AlignTop);
    labelIcon->setAlignment(Qt::AlignVCenter);
    QFontMetrics fm(qApp->font());

    if(labelIcon &&labelIcon->pixmap() && !labelIcon->pixmap()->size().isNull() && labelIcon->pixmap()->size().height()/labelIcon->devicePixelRatioF() <= fm.height())
        labelIcon->setFixedHeight(fm.height());
    QObject::connect(qApp, &QApplication::fontChanged, this, [&](const QFont &font){
        QFontMetrics fm(qApp->font());
        QString line;
        line = fm.elidedText(line1, Qt::ElideMiddle, 500);
        m_labelText = line + line2;
        m_content->setText(m_labelText);
        m_content->setFont(qApp->font());
        m_content->update();

        if(labelIcon &&labelIcon->pixmap() && !labelIcon->pixmap()->size().isNull() && labelIcon->pixmap()->size().height()/labelIcon->devicePixelRatioF() <= fm.height())
            labelIcon->setFixedHeight(fm.height());
        QString text = m_content->text();
        int textwidth = m_content->width() - 2*m_content->contentsMargins().left();
        textwidth = qMax(textwidth, 1);
        QRect textrect = fm.boundingRect(0,0,textwidth,0, m_content->alignment() | Qt::TextWordWrap, m_content->text());

        m_iconTopLabel->setFixedHeight((m_content->height() - textrect.height()) / 2);
    }, Qt::UniqueConnection);


//    labelLayout->addWidget(labelIcon);
//    labelLayout->addStretch();

    QHBoxLayout *centerLayout = new QHBoxLayout();
//    centerLayout->addWidget(skip);
    centerLayout->addWidget(save);
    centerLayout->addWidget(rename);
    centerLayout->addWidget(cutOff);
    centerLayout->addStretch();
    QCheckBox *allApp = new QCheckBox(tr("All applications"), this);
    allApp->setVisible(false);
    QDialogButtonBox *buttonBoxOne = new QDialogButtonBox(this);
    buttonBoxOne->setAccessibleName(tr("Dialog buttons one"));
    buttonBoxOne->setStandardButtons(QDialogButtonBox::NoButton);
    QPushButton *cancelBt = buttonBoxOne->addButton(tr("Cancel"), QDialogButtonBox::ActionRole);
    QPushButton *applyBt = buttonBoxOne->addButton(tr("OK"), QDialogButtonBox::ActionRole);
    applyBt->setDefault(true);
    applyBt->setProperty("isImportant", true);
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->addWidget(allApp);
    bottomLayout->addStretch();
    bottomLayout->addWidget(buttonBoxOne);
    bottomLayout->setAlignment(buttonBoxOne, Qt::AlignRight);

    oneLayout->addWidget(m_content);
    oneLayout->addLayout(centerLayout);
    oneLayout->addWidget(specificationLabel);
    oneLayout->addLayout(bottomLayout);

    QHBoxLayout *mainLayout = new QHBoxLayout();
    QVBoxLayout *iconLayout = new QVBoxLayout();
    m_iconTopLabel = new QLabel();
    iconLayout->setSpacing(0);
    iconLayout->addWidget(m_iconTopLabel);
    iconLayout->addWidget(labelIcon);
    iconLayout->addStretch();
    mainLayout->addLayout(iconLayout);
//    mainLayout->addWidget(labelIcon);
    mainLayout->addLayout(oneLayout);
    pageOne->setLayout(mainLayout);

    stack->addWidget(pageOne);

    // page2: test rename
    auto page2 = new QWidget(this);
    auto layout2 = new QGridLayout(this);
    auto textEdit = new RenameEditor(this);
    textEdit->setBackgroundRole(QPalette::Button);
    textEdit->setAutoFillBackground(true);
    textEdit->viewport()->setBackgroundRole(QPalette::Button);
    textEdit->viewport()->setAutoFillBackground(true);
//    int height = qApp->fontMetrics().lineSpacing() * 3 + qApp->fontMetrics().descent();
//    textEdit->setFixedHeight(height);

    auto num = new QLabel(this);
    int newNameCount = newName.toLocal8Bit().count();
    if (fsType.contains("ntfs") || fsType.contains("fuse.kyfs")) {
        newNameCount = newName.count();
    }
    QString totalNum =
            QString("<span style=\"color:red;\">%1</span>/%2%3").arg(newNameCount).arg(m_maxLength).arg(capitalByteText);
    num->setText(totalNum);
    auto buttonBox2 = new QDialogButtonBox(this);
    buttonBox2->setAccessibleName(tr("Dialog buttons 2"));
    buttonBox2->setStandardButtons(QDialogButtonBox::NoButton);
    auto cancel2 = buttonBox2->addButton(tr("Cancel"), QDialogButtonBox::ActionRole);
    auto ensure2 = buttonBox2->addButton(tr("OK"), QDialogButtonBox::ActionRole);
    ensure2->setDefault(true);
    ensure2->setEnabled(false);
    ensure2->setProperty("isImportant", true);

    layout2->addWidget(textEdit, 0, 0, 4, 2, Qt::AlignTop);
    layout2->addWidget(num, 4, 1, Qt::AlignBottom|Qt::AlignRight);
    layout2->addWidget(buttonBox2, 5, 1, Qt::AlignBottom|Qt::AlignRight);
    page2->setLayout(layout2);
    stack->addWidget(page2);

    textEdit->setText(newName);
    textEdit->selectAll();
    //textEdit->setFocus();
    auto cursor = textEdit->textCursor();
    cursor.setPosition(0, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    bool isDir = Peony::FileUtils::getFileIsFolder(textEdit->toPlainText());
    bool isDesktopFile = textEdit->toPlainText().endsWith(".desktop");
    bool isSoftLink = Peony::FileUtils::getFileIsSymbolicLink(textEdit->toPlainText());

    if (!isDesktopFile && !isSoftLink && !isDir && textEdit->toPlainText().contains(".") &&
            !textEdit->toPlainText().startsWith(".")) {
        int n = 1;
        if(textEdit->toPlainText().contains(".tar.")) //ex xxx.tar.gz xxx.tar.bz2
            n = 2;
        while(n){
            cursor.movePosition(QTextCursor::WordLeft, QTextCursor::KeepAnchor, 1);
            cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 1);
            --n;
        }
    }

    textEdit->setTextCursor(cursor);
    textEdit->activateWindow();
    //textEdit->setFocus();

    connect(textEdit, &QTextEdit::textChanged, this, [=](){
        QString currentNum ;
        int textCount = textEdit->toPlainText().toLocal8Bit().count();
        if (fsType.contains("ntfs") || fsType.contains("fuse.kyfs")) {
            textCount = textEdit->toPlainText().count();
            pDebug << "textCount 11111" << textCount;

        }

        if (textCount > m_maxLength) {
            ensure2->setEnabled(false);
            currentNum = QString("<span style=\"color:red;\">%1</span>").arg(textCount);
        } else {
            currentNum = QString("<span style=\"color:\">%1</span>").arg(textCount);
            ensure2->setEnabled(true);
        }
        QString totalNum = QString("%1/%2%3").arg(currentNum).arg(m_maxLength).arg(capitalByteText);
        num->setText(totalNum);
    });

    // page3: text truncation
    QWidget *truncationWidget = new QWidget(this);
    QRadioButton *frontTruncation = new QRadioButton(tr("Front truncation"), this);
    QRadioButton *laterTruncation = new QRadioButton(tr("Post truncation"), this);
    QButtonGroup *truncationGroup = new QButtonGroup(this);
    truncationGroup->addButton(laterTruncation, 0);
    truncationGroup->addButton(frontTruncation, 1);
    laterTruncation->setChecked(true);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    buttonBox->setAccessibleName(tr("Dialog buttons"));
    buttonBox->setStandardButtons(QDialogButtonBox::NoButton);
    QPushButton *cancelButton = buttonBox->addButton(tr("Cancel"), QDialogButtonBox::ActionRole);
    QPushButton *okButton = buttonBox->addButton(tr("OK"), QDialogButtonBox::ActionRole);
    okButton->setDefault(true);
    okButton->setProperty("isImportant", true);

    QHBoxLayout *pageThreebottomLayout = new QHBoxLayout();
    pageThreebottomLayout->addWidget(frontTruncation);
    pageThreebottomLayout->addWidget(laterTruncation);
    pageThreebottomLayout->addStretch();
    pageThreebottomLayout->addWidget(buttonBox);

    MyTextEdit *truncatedLabel = new MyTextEdit(this);
    truncatedLabel->setText(newName);

    QVBoxLayout *cutOffLayout = new QVBoxLayout(truncationWidget);
    cutOffLayout->addWidget(truncatedLabel);
    cutOffLayout->addStretch();
    cutOffLayout->addLayout(pageThreebottomLayout);
    truncationWidget->setLayout(cutOffLayout);
    stack->addWidget(truncationWidget);
    stack->setCurrentWidget(pageOne);

    connect(cancelButton, &QPushButton::clicked, this, &KyFileDialogRename::reject);
    connect(okButton, &QPushButton::clicked, this, [=, &error]{
        int id = truncationGroup->checkedId();
        error.respValue.insert("cateType", id);
        if (Qt::CheckState::Checked == allApp->checkState()) {
            error.respCode = Peony::ExceptionResponse::TruncateAll;
        } else {
            error.respCode = Peony::ExceptionResponse::TruncateOne;
        }
        Q_EMIT changeFileName(m_truncateFileName);
        accept();
    });

    connect(truncationGroup, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked),
            this, [=](QAbstractButton* button) {
        const int id = truncationGroup->id(button);
        const QString destName = truncateDestFileName(newName, id, fsType);
        truncatedLabel->setText(destName);
    });
    Q_EMIT truncationGroup->buttonClicked(laterTruncation);

//    m_currentWidget = page2;

    QString renameClickText = "<a href=\"  \" style=\"color: #3D6BE5;text-decoration: none;\">"
          + tr("modify the name")
          + "</a>" + tr(".");
    QString renameText = tr("Explanation: When renaming a file name, ensure it is within %1 %2 and ").arg(m_maxLength).arg(byteText);
    renameText = renameText + renameClickText;

    QString truncateClickText = "<a href=\" \" style=\"color: #3D6BE5;text-decoration: none;\">"
          + tr("truncate interval")
           + "</a>" + tr(".");
    QString truncateText = tr("Explanation: Truncate the portion of the file name that exceeds %1 %2 and select ").arg(PEONY_TRUNCATE_NAME_LIMIT).arg(byteText);
    truncateText = truncateText + truncateClickText;

    QString downloadPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    QString pathText = QString(tr("Description: Save long file names to \"") + downloadPath + "/扩展" + tr("\"."));

    connect(onePageGroup, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked),
            this, [=](QAbstractButton* button) {
        const int id = onePageGroup->id(button);
        switch(id) {
        case 0:{
            specificationLabel->setText(renameText);
            m_currentWidget = page2;
            break;
        }
        case 1:{
            specificationLabel->setText(truncateText);
            m_currentWidget = truncationWidget;
            break;
        }
        case 2:{
            specificationLabel->setText(pathText);
            break;
        }

        };
    });
//    Q_EMIT onePageGroup->buttonClicked(skip);

    connect(specificationLabel, &QLabel::linkActivated, this, [=]() {
        const int id = onePageGroup->checkedId();
        pDebug << "linkActivated......." << id;

        if (1 == id) {
            QRect textRect = fontMetrics().boundingRect(truncatedLabel->toPlainText());
            int len = textRect.width()/(this->width() - stack->contentsMargins().left() - stack->contentsMargins().right());
            int totalHeight = qApp->fontMetrics().lineSpacing() *(len+1) + qApp->fontMetrics().descent();
            totalHeight = totalHeight + 150 > this->height() ? totalHeight + 150 : this->height();
//            this->setFixedHeight(totalHeight);
            stack->setCurrentWidget(truncationWidget);
        } else if (0 == id) {
            stack->setCurrentWidget(page2);
            textEdit->setFocus();
        }
        stack->setCurrentWidget(m_currentWidget);
    });

    if (QGSettings::isSchemaInstalled("org.ukui.style")) {
        QGSettings *settings = new QGSettings("org.ukui.style", QByteArray(), this);
        connect(settings, &QGSettings::changed, this, [=](const QString &key) {
            if("iconThemeName" == key){
                labelIcon->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(16, 16));
                //renameIcon->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(24, 24));
            } else if ("systemFont" == key || "systemFontSize" == key) {
                if (0 != stack->currentIndex()) {
                    QFontMetrics font = qApp->fontMetrics();
                    QRect textRect = fontMetrics().boundingRect(truncatedLabel->toPlainText());
                    int len = textRect.width()/(this->width() - stack->contentsMargins().left() - stack->contentsMargins().right());
                    int totalHeight = qApp->fontMetrics().lineSpacing() *(len+1) + qApp->fontMetrics().descent();
                    totalHeight = totalHeight + 150 > this->height() ? totalHeight + 150 : this->height();
//                    this->setFixedHeight(totalHeight);
                }
//                int height = font.lineSpacing() * 3 + font.descent();
//                textEdit->setFixedHeight(height);
//                textEdit->setTextCursor(cursor);

//                QStringList messages;
//                QString line;
//                line = qApp->fontMetrics().elidedText(line1, Qt::ElideMiddle, 500);
//                messages.append(line);
//                line = qApp->fontMetrics().elidedText(line2, Qt::ElideMiddle, 500);
//                messages.append(line);
//                line = qApp->fontMetrics().elidedText(line3, Qt::ElideMiddle, 500);
//                messages.append(line);
//                auto labelText = messages.join('\n');
                m_content->setText(m_labelText);
                this->repaint();
            }
        });
    }

    connect(cancelBt, &QPushButton::clicked, this, &KyFileDialogRename::reject);
    connect(applyBt, &QPushButton::clicked, this, [=, &error]{
        const int id = onePageGroup->checkedId();
        switch(id) {
        case 0:{
            stack->setCurrentWidget(page2);
            break;
        }

        case 1:{
            QRect textRect = fontMetrics().boundingRect(truncatedLabel->toPlainText());
            int len = textRect.width()/(this->width() - stack->contentsMargins().left() - stack->contentsMargins().right());
            int totalHeight = qApp->fontMetrics().lineSpacing() *(len+1) + qApp->fontMetrics().descent();
            totalHeight = totalHeight + 150 > this->height() ? totalHeight + 150 : this->height();
//            this->setFixedHeight(totalHeight);
            stack->setCurrentWidget(truncationWidget);
            break;
        }
        case 2:{
            if (Qt::CheckState::Checked == allApp->checkState()) {
                error.respCode = Peony::ExceptionResponse::SaveAll;
            } else {
                error.respCode = Peony::ExceptionResponse::SaveOne;
            }
            accept();
            break;
        }
        }
    });
    connect(cancel2, &QPushButton::clicked, this, &KyFileDialogRename::reject);
    connect(ensure2, &QPushButton::clicked, this, [=, &error]{
        error.respValue.insert("newName", textEdit->toPlainText());
        error.respCode = Peony::ExceptionResponse::RenameOne;
        pDebug << "ensure2 clicked...." << newName << textEdit->toPlainText();
        Q_EMIT changeFileName(textEdit->toPlainText());
        accept();
    });

    connect(this, &KyFileDialogRename::rejected, this, [&](){
        pDebug << "reject.......";
        m_dialogAccept = false;
    });
    connect(this, &KyFileDialogRename::accepted, this, [&](){
        pDebug << "accepted.......";

        m_dialogAccept = true;
    });
    connect(textEdit, &RenameEditor::returnPressed, ensure2, &QPushButton::click);

    connect(this, &KyFileDialogRename::rejected, this, [&](){
       if(!save->isVisible()) {
           Q_EMIT muchLongNameReject();
       }
    });

    if(fsType.contains("ext")){
        save->setVisible(true);
        save->setChecked(true);
        specificationLabel->setText(pathText);
    }
    else{
        save->setVisible(false);
        rename->setChecked(true);
        specificationLabel->setText(renameText);
        m_currentWidget = page2;
    }


    if (!exec()) {
        pDebug << "responseCode......" << responseCode;
        error.respCode = responseCode;
    }
}

const QString KyFileDialogRename::truncateDestFileName(const QString &uri, const int cateType, QString fsType)
{
    /*调试代码
    {
        QString uri("file:///home/tanjing/下载/扩展/少时诵诗书所所所所所所所所所所所所所所所所所所所所所所所所所所所所所所所所所所所所所所所所所所所所所所所所所所所所所所所所所所所少时诵诗书多多多多的点点滴滴多多多多多多多长城长城错错错错错错错错错错错错错错错错错错错的点点滴滴多多多多多多多就还好还或或或或或能解决军军军军军军军军军军军军军军军军军军军军军军军军军合并报表不不不不不不不不不不不不不不不不不不不不不不不不不不不不不不不不不不不不不不不接口可扩扩扩扩扩不不不.png");
        auto newName = Peony::FileUtils::getNonSuffixedBaseNameFromUri(uri);
        QString baseName = Peony::FileUtils::getUriBaseName(uri);
        baseName = Peony::FileUtils::urlDecode(baseName);
        auto destDirUri = Peony::FileUtils::getParentUri(uri);
        auto fsType = Peony::FileUtils::getFsTypeFromFile(uri);
        bool setLimitBytes = true;
        if (fsType.contains("ntfs")) {
            setLimitBytes = false;
        }
        auto suffix = baseName;
        suffix = suffix.remove(newName);
        newName.remove(suffix);
        pDebug << "newName0000......" << newName;
        pDebug << "baseName 000000......" << baseName;
        pDebug << "suffix 000000......" << suffix;
    }
    */


    QFileInfo file(uri);
    QString newName;//去掉后缀的名字// = Peony::FileUtils::getNonSuffixedBaseNameFromUri(uri);
    {
        QString suffixedBaseName = uri;
        int index = suffixedBaseName.lastIndexOf(".");
        if (index != -1) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
            QString suffix = suffixedBaseName.chopped(suffixedBaseName.size() - index);
            pDebug << "suffix 00000000000000000" << suffix;
            if (suffix == ".gz" || suffix == ".xz" || suffix == ".bz"
                    || suffix == ".bz2" || suffix == ".Z" ||
                    suffix == ".sit") {
                int secondIndex = suffixedBaseName.lastIndexOf('.');
                suffixedBaseName.chop(suffixedBaseName.size() - secondIndex);
            }
            else
                suffixedBaseName = suffix;
#else
            suffixedBaseName.chop(suffixedBaseName.size() - index);
#endif
        }
        newName = suffixedBaseName;
    }

    QString baseName = uri;

    //auto fsType = Peony::FileUtils::getFsTypeFromFile(uri);
    bool setLimitBytes = true;
    if (fsType.contains("ntfs") || fsType.contains("fuse.kyfs")) {
        setLimitBytes = false;
    }
    auto suffix = baseName;
    suffix = suffix.remove(newName);
    newName.remove(suffix);
    pDebug << "newName 11111......" << newName;
//    pDebug << "baseName 22222......" << baseName;
//    pDebug << "suffix 22222......" << suffix;

    pDebug << "setLimitBytes......" << fsType << setLimitBytes << newName.length() << newName.count() << newName.toLocal8Bit().count();
    QString truncatedText;
    QString destName;
    if (setLimitBytes) {
        bool useForceChop = false;
        if (suffix.toLocal8Bit().count() > PEONY_TRUNCATE_NAME_LIMIT) {
            qWarning()<<"suffix too long:"<<uri<<"use force chop instead";
            useForceChop = true;
        } else if (newName == baseName) {
            qWarning()<<"failed to truncate suffix of"<<uri<<", use force chop instead";
            useForceChop = true;
        } else if (file.isDir()) {
            useForceChop = true;
        }
        if (useForceChop) {
            newName = baseName;
            int len = newName.length();
            if (TurnCateType::Post == cateType) {
                while (newName.toLocal8Bit().count() > PEONY_TRUNCATE_NAME_LIMIT) {
                    newName.chop(1);
                }
                truncatedText = baseName.right(len - newName.length());
                destName = newName + "<s>" + truncatedText + "</s>";
            } else if (TurnCateType::Front == cateType) {
                while (newName.toLocal8Bit().count() > PEONY_TRUNCATE_NAME_LIMIT) {
                    newName.remove(0,1);
                }
                truncatedText = baseName.left(len - newName.length());
                destName = "<s>" + truncatedText + "</s>" + newName;
            }
        } else {
            QString tmp = newName;
            int len = tmp.length();
            int limitBytes = PEONY_TRUNCATE_NAME_LIMIT - suffix.toLocal8Bit().count();
            if (TurnCateType::Post == cateType) {
                while (newName.toLocal8Bit().count() > limitBytes) {
                    newName.chop(1);
                }
                truncatedText = tmp.right(len - newName.length());
                destName = newName + "<s>" + truncatedText + "</s>";
            } else if (TurnCateType::Front == cateType){
                while (newName.toLocal8Bit().count() > limitBytes) {
                    newName.remove(0,1);
                }
                truncatedText = tmp.left(len - newName.length());
                destName = "<s>" + truncatedText + "</s>" + newName;
            }
            destName = destName + suffix;
        }
    } else {
        bool useForceChop = false;
        if (suffix.length() > PEONY_TRUNCATE_NAME_LIMIT) {
            qWarning()<<"suffix too long:"<<uri<<"use force chop instead";
            useForceChop = true;
        } else if (newName == baseName) {
            qWarning()<<"failed to truncate suffix of"<<uri<<", use force chop instead";
            useForceChop = true;
        } else if (file.isDir()) {
            useForceChop = true;
        }
        pDebug << "useForceChop......" << useForceChop;
        if (useForceChop) {
            newName = baseName;
//            while (newName.length() > PEONY_TRUNCATE_NAME_LIMIT) {
//                newName.chop(1);
//            }
            int len = baseName.length();

            if (TurnCateType::Post == cateType) {
                while (newName.count() > PEONY_TRUNCATE_NAME_LIMIT) {
                    newName.chop(1);
                }
                truncatedText = baseName.right(len - newName.length());
                destName = newName + "<s>" + truncatedText + "</s>";
            } else if (TurnCateType::Front == cateType) {
                while (newName.count() > PEONY_TRUNCATE_NAME_LIMIT) {
                    newName.remove(0,1);
                }
                truncatedText = baseName.left(len - newName.length());
                destName = "<s>" + truncatedText + "</s>" + newName;
            }

//            truncatedText = baseName.right(len - newName.length());
//            destName = newName + "<s>" + truncatedText + "</s>";
        } else {
            QString tmp = newName;
            int limitBytes = PEONY_TRUNCATE_NAME_LIMIT - suffix.length();
            int len = tmp.length();

            if (TurnCateType::Post == cateType) {
                while (newName.length() > limitBytes) {
                    newName.chop(1);
                }
                truncatedText = tmp.right(len - newName.length());
                destName = newName + "<s>" + truncatedText + "</s>";
            } else if (TurnCateType::Front == cateType){
                while (newName.length() > limitBytes) {
                    newName.remove(0,1);
                }

                truncatedText = tmp.left(len - newName.length());
                destName = "<s>" + truncatedText + "</s>" + newName;
            }

//            truncatedText = tmp.right(len - newName.length());
//            destName = newName + "<s>" + truncatedText + "</s>";
            destName = destName + suffix;
        }
    }
    m_truncateFileName = newName + suffix;

    pDebug << "m_truncateFileName......." << m_truncateFileName;
    pDebug << "destName......" << destName;
    return destName;
}

bool KyFileDialogRename::dialogResult(){
    return m_dialogAccept;
}

void KyFileDialogRename::setCurrentUri(QString uri)
{
    if(m_currentUri != uri)
        m_currentUri = uri;
}

QString KyFileDialogRename::currentUri()
{
    return m_currentUri;
}

void KyFileDialogRename::setMaxLength(int maxLength)
{
    if(m_maxLength != maxLength)
        m_maxLength = maxLength;
}
