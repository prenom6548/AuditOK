/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

#include "shreddialog.h"
#include "kalertdialog.h"
#include "buried_point.hpp"

// ShredDialog::ShredDialog(ShredManager *plugin, QDialog *parent)
//     :QDialog(parent)
ShredDialog::ShredDialog(QWidget *parent) : KWidget(parent), mousePressed(false)
{

    //首先设置大小
    this->setFixedSize(672, 260);
    //设置icon和标题
    QIcon icon(QIcon::fromTheme("kylin-file-crush"));
    QPixmap pix = icon.pixmap(icon.actualSize(QSize(24, 24)));
    this->setIcon("kylin-file-crush");
    this->setWidgetName(tr("File Shredder"));
    // iconBar()->iconLabel()->setPixmap(pix);
    // iconBar()->nameLabel()->setText(tr("FileShredder"));
    this->setWindowTitle(tr("File Shredder"));
    this->setWindowIcon(QIcon::fromTheme("kylin-file-crush"));

    //    this->setWindowTitle(tr("Kylin Shred Manager"));
    //    this->setWindowIcon(QIcon(":/model/res/plugin/shredder.png"));
    //设置最大化和最小化不可见
    windowButtonBar()->maximumButton()->setVisible(false);
    windowButtonBar()->minimumButton()->setVisible(false);
    windowButtonBar()->menuButton()->setVisible(false);
    sideBar()->hide();
    //设置主布局
    mLabelTipIcon = new QLabel(this);
    mLabelTipIcon->setFixedSize(24, 24);
    if (QIcon::hasThemeIcon("dialog-info")) {
        mLabelTipIcon->setPixmap(QIcon::fromTheme("dialog-info").pixmap(QSize(24, 24)));
    } else {
        qWarning() << "ShredDialog::ShredDialog 无系统Icon！";
    }

    mLabelTipText = new QLabel(this);
    mLabelFilePath = new QLineEdit(this);
    mLabelFilePath->setReadOnly(true);
    mLabelFilePath->setPlaceholderText(tr("No file selected to be shredded"));

    shred_btn = new QPushButton();
    shred_btn->resize(96, 36);
    shred_btn->setObjectName("blackButton");
    shred_btn->setFocusPolicy(Qt::NoFocus);
    shred_btn->setProperty("isImportant", true);
    cacel_btn = new QPushButton();
    cacel_btn->resize(96, 36);
    cacel_btn->setObjectName("blackButton");
    cacel_btn->setFocusPolicy(Qt::NoFocus);
    mBtnAddFile = new QPushButton();
    mBtnAddFile->resize(96, 36);
    mBtnAddFile->setFocusPolicy(Qt::NoFocus);

    tipLabel = new QLabel();
    //    tipLabel->setFixedSize(400,34);
    //    tipLabel->setWordWrap(true);
    tipLabel->setIndent(5);
    tipLabel->setAlignment(Qt::AlignLeft);
    QPalette pal;
    QColor color = palette().color(QPalette::PlaceholderText);
    pal.setColor(QPalette::WindowText, color);
    tipLabel->setPalette(pal);

    QHBoxLayout *layout1 = new QHBoxLayout();
    layout1->setContentsMargins(0, 0, 0, 0);
    layout1->setSpacing(0);
    layout1->setMargin(0);
    layout1->addWidget(mLabelTipIcon);
    layout1->addSpacing(8);
    layout1->addWidget(mLabelTipText);
    layout1->addStretch();

    QHBoxLayout *layout2 = new QHBoxLayout();
    layout2->setContentsMargins(0, 0, 0, 0);
    layout2->setSpacing(0);
    layout2->setMargin(0);
    layout2->addWidget(mLabelFilePath);

    QHBoxLayout *layout3 = new QHBoxLayout();
    layout3->setContentsMargins(0, 0, 0, 0);
    layout3->setSpacing(0);
    layout3->setMargin(0);
    layout3->addWidget(tipLabel);
    layout3->addStretch();

    QHBoxLayout *layout4 = new QHBoxLayout();
    layout4->setContentsMargins(0, 0, 0, 0);
    layout4->setSpacing(0);
    layout4->setMargin(0);
    layout4->addWidget(cacel_btn);
    layout4->addStretch();
    layout4->addWidget(mBtnAddFile);
    layout4->addSpacing(16);
    layout4->addWidget(shred_btn);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(24, 0, 24, 24);
    layout->setSpacing(0);
    //    layout->addStretch();
    layout->addLayout(layout1);
    layout->addSpacing(17);
    layout->addLayout(layout2);
    layout->addSpacing(8);
    layout->addLayout(layout3);
    layout->addSpacing(14);
    layout->addLayout(layout4);
    //    layout->addStretch();
    baseBar()->setLayout(layout);

    //    progressbar->setVisible(false);

    alertDialog = new KAlertDialog(this);
    connect(alertDialog, &KAlertDialog::sigCloseDialog, [this]() {
        //        qDebug() << "接收到警示框的文件提示信号";
        mLabelFilePath->setText("");
    });

    sureDialog = new KSureDialog(this);
    connect(sureDialog, SIGNAL(sigCloseDialog(bool)), this, SLOT(slotSureDialog(bool)));

    this->setLanguage();
    this->initConnect();
    this->moveCenter();
}

ShredDialog::~ShredDialog()
{
    //    if (shredSettings != NULL)
    //    {
    //        shredSettings->sync();
    //        delete shredSettings;
    //        shredSettings = NULL;
    //    }
}

void ShredDialog::setLanguage()
{
    mLabelTipText->setText(tr("The file is completely shredded and cannot be recovered"));
    shred_btn->setText(tr("Shred File"));
    cacel_btn->setText(tr("Deselect"));
    mBtnAddFile->setText(tr("Add File"));
    tipLabel->setText(tr("Note:File shredding cannot be cancelled, please exercise caution!"));
    //    barLabel->setText(tr("Shattering..."));
}

void ShredDialog::initConnect()
{
    connect(mBtnAddFile, SIGNAL(clicked()), this, SLOT(onSelectButtonClicked()));
    connect(shred_btn, SIGNAL(clicked()), this, SLOT(onShredButtonClicked()));
    connect(cacel_btn, SIGNAL(clicked()), this, SLOT(onCacelButtonClicked()));
    //    connect(title_bar, SIGNAL(showMinDialog()), this, SLOT(onMinButtonClicked()));
    connect(windowButtonBar()->closeButton(), SIGNAL(clicked()), this, SLOT(onCloseButtonClicked()));
}

void ShredDialog::onCloseButtonClicked()
{
    this->close();
}

// void ShredDialog::progressbarFlash()
//{
//     if(i == 100){
//         progressbar->setInvertedAppearance(!(progressbar->invertedAppearance()));
//         i = 0;
//         progressbar->setValue(i);
//     }else{
//         progressbar->setValue(i);
//         i++;
//     }
// }

void ShredDialog::onSelectButtonClicked()
{
    QString fileName = QFileDialog::getOpenFileName(0, tr("Select file"), QDir::homePath(), tr("All Files(*)"));
    if (!fileName.isEmpty()) {
        mLabelFilePath->setText(fileName);

        if (QIcon::hasThemeIcon("dialog-info")) {
            mLabelTipIcon->setPixmap(QIcon::fromTheme("dialog-info").pixmap(QSize(24, 24)));
        } else {
            qWarning() << "ShredDialog::ShredDialog 无系统Icon！";
        }
        mLabelTipText->setText(tr("The file is completely shredded and cannot be recovered"));
    }



    /*QStringList fileNameList;
    QString fileName;
    QFileDialog* fd = new QFileDialog(this);
//    fd->setStyleSheet("QFileDialog{background-color:blue;}");
    fd->resize(500, 471);
//    fd->setFilter(tr("Allfile(*)"));
    fd->setFilter(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
//    fd->setFilter( "Allfile(*.*);;mp3file(*.mp3);;wmafile(*.wma);;wavefile(*.wav)");
    fd->setViewMode(QFileDialog::List);//设置浏览模式，有 列表（list） 模式和 详细信息（detail）两种方式
    if (fd->exec() == QDialog::Accepted)
    {
        fileNameList = fd->selectedFiles();
        fileName = fileNameList[0];
        select_edit->setText(fileName);
    }
    else
        fd->close();*/
}
void ShredDialog::slotSureDialog(bool isSure)
{
    if (isSure) {
        kdk::kabase::BuriedPoint buriedPoint;
        if (buriedPoint.functionBuriedPoint(kdk::kabase::AppName::KylinOsManager,
                                            kdk::kabase::BuriedPoint::PT::KylinOsManagerFileShredding)) {
            qCritical() << "buried point fail!pt:BaseInfo";
        }

        mLabelTipText->setText(tr("File Shredding ..."));
        if (QIcon::hasThemeIcon("dialog-warning")) {
            mLabelTipIcon->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(QSize(24, 24)));
        } else {
            qWarning() << "ShredDialog::slotSureDialog  无系统图标！";
        }

        myThread = new ShredQThread(mLabelFilePath->text());
        thread = new QThread();
        myThread->moveToThread(thread);
        connect(myThread, &ShredQThread::success, this, [=] {
            qDebug() << "ShredDialog::slotSureDialog success!";
            cacel_btn->setVisible(true);
            shred_btn->setDisabled(false);
            //            progressbar->setVisible(false);
            //            barLabel->setVisible(false);
            //            toolkits->alertMSG(this->frameGeometry().topLeft().x(), this->frameGeometry().topLeft().y(),
            //            tr("Shred successfully!"));
            if (QIcon::hasThemeIcon("ukui-dialog-success")) {
                mLabelTipIcon->setPixmap(QIcon::fromTheme("ukui-dialog-success").pixmap(QSize(24, 24)));
            } else {
                qDebug() << "ShredDialog::slotSureDialog  无系统图标！";
            }
            mLabelTipText->setText(tr("Shred successfully!"));
            mLabelFilePath->setText("");
            thread->exit();
            //            myTimer.stop();
        });

        connect(myThread, &ShredQThread::failed, this, [=] {
            qDebug() << "ShredDialog::slotSureDialog failed!";
            cacel_btn->setVisible(true);
            shred_btn->setDisabled(false);
            //            progressbar->setVisible(false);
            //            barLabel->setVisible(false);
            //            toolkits->alertMSG(this->frameGeometry().topLeft().x(), this->frameGeometry().topLeft().y(),
            //            tr("Shred failed!"));
            mLabelTipText->setText(tr("Shred failed!"));
            mLabelFilePath->setText("");
            thread->exit();
            // myTimer.stop();
        });

        connect(thread, &QThread::started, myThread, &ShredQThread::run);

        connect(thread, &QThread::finished, this, [=] {
            thread->deleteLater();
            qDebug() << "ShredQThread thread finished......";
        });

        thread->start();
    }
}

void ShredDialog::onShredButtonClicked()
{

    if (mLabelFilePath->text().isEmpty()) {
        if (alertDialog) {
            alertDialog->setAlertText(tr("Please select the file to shred!"));
            alertDialog->show();
        }
    } else if (!mLabelFilePath->text().contains("/")) {
        if (alertDialog) {
            alertDialog->setAlertText(tr("You did not select a file with permissions!"));
            alertDialog->show();
        }
    } else {
        sureDialog->exec();
    }
}

void ShredDialog::onCacelButtonClicked()
{
    mLabelFilePath->clear();
}

void ShredDialog::closeEvent(QCloseEvent *event)
{
    event->accept();
    //  emit SignalClose();
}

void ShredDialog::moveCenter()
{
    QPoint pos = QCursor::pos();
    QRect primaryGeometry;
    for (QScreen *screen : qApp->screens()) {
        if (screen->geometry().contains(pos)) {
            primaryGeometry = screen->geometry();
        }
    }

    if (primaryGeometry.isEmpty()) {
        primaryGeometry = qApp->primaryScreen()->geometry();
    }

    this->move(primaryGeometry.x() + (primaryGeometry.width() - this->width()) / 2,
               primaryGeometry.y() + (primaryGeometry.height() - this->height()) / 2);
}

void ShredDialog::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        this->dragPosition = event->globalPos() - frameGeometry().topLeft();
        this->mousePressed = true;
    }
    QWidget::mousePressEvent(event);
}

void ShredDialog::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        this->mousePressed = false;
    }

    QWidget::mouseReleaseEvent(event);
}

void ShredDialog::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    //    QPainterPath path;
    //    path.setFillRule(Qt::WindingFill);
    //    path.addRoundRect(10,10,this->width()-20,this->height()-20,5,5);
    //    QPainter painter(this);
    //    painter.setRenderHint(QPainter::Antialiasing,true);
    //    painter.fillPath(path,QBrush(Qt::white));
    //    QColor color(0,0,0,50);
    //    for(int i = 0 ; i < 10 ; ++i)
    //    {
    //        QPainterPath path;
    //        path.setFillRule(Qt::WindingFill);
    //        path.addRoundRect(10-i,10-i,this->width()-(10-i)*2,this->height()-(10-i)*2,5,5);
    //        color.setAlpha(150 - qSqrt(i)*50);
    //        painter.setPen(color);
    //        painter.drawPath(path);
    //    }

    QWidget::paintEvent(event);
}

void ShredDialog::mouseMoveEvent(QMouseEvent *event)
{
    if (this->mousePressed) {
        move(event->globalPos() - this->dragPosition);
    }

    QWidget::mouseMoveEvent(event);
}
