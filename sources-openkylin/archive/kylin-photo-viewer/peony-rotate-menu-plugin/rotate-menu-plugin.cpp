/* Copyright (C) 2020, KylinSoft Co., Ltd.
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

#include "rotate-menu-plugin.h"

typedef char** (*GetFileProtectedFunc)(int*);

using namespace Peony;

RotateMenuPlugin::RotateMenuPlugin(QObject *parent) : QObject (parent)
{
    QTranslator *t = new QTranslator(this);
    qDebug()<<"system().name:"<<QLocale::system().name();
    qDebug()<<"translate:"<<t->load(":/translations/peony-rotate-menu_"+QLocale::system().name());
    QApplication::installTranslator(t);
}

QList<QAction*> RotateMenuPlugin::menuActions(Types types, const QString &uri, const QStringList &selectionUris)
{
    QList<QAction*> actions;
    if (types == MenuPluginInterface::DirectoryView || types == MenuPluginInterface::DesktopWindow)
    {
        if (! QFile::exists("/usr/bin/kylin-photo-viewer")){
            return actions;
        }
        if (! selectionUris.isEmpty()) {
            auto info = FileInfo::fromUri(selectionUris.first());
            //special type mountable, return
            qDebug()<<"info isVirtual:"<<info->isVirtual()<<info->mimeType();
            if (selectionUris.first().startsWith("computer:///") ||
                selectionUris.first().startsWith("trash:///") ||
                selectionUris.first().startsWith("recent:///") ||
                selectionUris.first().startsWith("filesafe:///") ||
                info->isVirtual())
            {
                return actions;
            }

            is_have_image(selectionUris);
            if (isHaveImage)
            {
                //旋转线程
                workerThread = new  QThread();
                rotateProcessor = new Rotation();
                rotateProcessor->moveToThread(workerThread);
                //连接信号和槽
                connect(this,&RotateMenuPlugin::sigImageRotate,rotateProcessor,&Rotation::rotateAndSaveImage);
                connect(rotateProcessor, &Rotation::allImageFinishSig,this,&RotateMenuPlugin::rotateFinish );
                connect(rotateProcessor, &Rotation::allImageFinishSig, workerThread, &QThread::quit);
                connect(workerThread, &QThread::finished, workerThread, &QThread::deleteLater);
                connect(KylinImageCodec::getSignalObj(), &KylinImageCodecSignals::saveMovieFinish,this,&RotateMenuPlugin::movieRotateFinish,Qt::UniqueConnection);

                //向右旋转
                QAction *rotateRightAction = new QAction(QIcon::fromTheme("object-rotate-right-symbolic"),tr("Rotate right"), this);
                actions<<rotateRightAction;
                connect(rotateRightAction, &QAction::triggered, this,[=](){
                    urlToPath(selectionUris,rotateRightAction->parentWidget()->parentWidget());
                    workerThread->start();
                    //进度条初始化
                    if(isShowProcessBar){
                        initProcessBar(selectionUris);
                        connect(rotateProcessor, &Rotation::imageFinishSig, this, &RotateMenuPlugin::updateProgress);
                    }
                    // 发送图片路径到旋转线程
                    Q_EMIT sigImageRotate(imagePaths, true);
                });

                //向左旋转
                QAction *rotateLeftAction = new QAction(QIcon::fromTheme("object-rotate-left-symbolic"),tr("Rotate left"), this);
                actions<<rotateLeftAction;
                connect(rotateLeftAction, &QAction::triggered, [=](){
                    urlToPath(selectionUris,rotateLeftAction->parentWidget()->parentWidget());
                    workerThread->start();
                    //进度条初始化
                    if(isShowProcessBar){
                        initProcessBar(selectionUris);
                        connect(rotateProcessor, &Rotation::imageFinishSig, this, &RotateMenuPlugin::updateProgress);
                    }
                    // 发送图片路径到旋转线程
                    Q_EMIT sigImageRotate(imagePaths, false);
                });
            }      
        }
    }
    rotateFinish();
    return actions;
}


//url转为path
void RotateMenuPlugin::urlToPath(const QStringList &Uris, QWidget *win){
    //是否显示进度条
    if(Uris.length() >= 4){
        isShowProcessBar = true;
    }

    for(auto url:Uris){
        //转为path
        QString imagePath = QUrl(url).toLocalFile();
        QFileInfo fileInfo (imagePath);
        imagePaths.append(imagePath);
#ifndef UNUI
        if(!fileInfo.exists()){
            QMessageBox::critical(win,tr("Error"),
                                  tr("The file does not exist: %1").arg(fileInfo.fileName()));
        }
        if (!fileInfo.isWritable()){
            QMessageBox::warning(win,tr("Permission Warning"),
                                 tr("The file does not have write permission and cannot be modified: %1").arg(fileInfo.fileName()));
        }
        if(!m_file_type_list.contains(fileInfo.suffix())){
            QMessageBox::warning(win, tr("Prompt"),
                                 tr("The file format is not supported for modification at the moment: %1").arg(fileInfo.fileName()));
        }
#endif

        //判断图片是否被添加到防篡改列表
        if (!m_securityFd) {
            m_securityFd = dlopen("/usr/lib/kysdk/kysdk-security/libkyfilesec.so", RTLD_LAZY);
        }
        if (m_securityFd) {
            GetFileProtectedFunc kdk_get_file_protected = (GetFileProtectedFunc)dlsym(m_securityFd, "kdk_get_file_protected");
            if (kdk_get_file_protected == NULL) {
                qDebug()<< "找不到符号:kdk_get_file_protected";
            } else {
                int len;
                char** p = kdk_get_file_protected(&len);
                if (p != nullptr) {
                    for (int i = 0; i < len; i++) {
                        std::string str = p[i];
                        if (imagePath.toStdString().data() == str) {
                            QMessageBox::warning(win, tr("Prompt"),
                                                 tr("The file has been added to the tamper proof list: %1").arg(fileInfo.fileName()));
                        }
                    }
                    for (int i = 0; i < len; i++) {
                        free(p[i]);
                    }
                    free(p);
                }
            }
        }

    }
}

void RotateMenuPlugin::is_have_image(const QStringList &Uris)
{
    isHaveImage = false;
    if(Uris.isEmpty()){
        return;
    }
    // 检查所有文件是否都是支持的文件格式
    for(auto url:Uris){
        QString path = QUrl(url).toLocalFile();
        QFileInfo file(path);
        // 如果是目录或者文件扩展名不在支持的列表中，直接返回
        if(file.isDir() || !m_file_type_list.contains(file.suffix().toLower())){
            return;
        }
    }
    // 所有文件都通过了检查，都是支持的图片格式
    isHaveImage = true;
}

//进度条初始化
void RotateMenuPlugin::initProcessBar(const QStringList &Uris)
{
        m_dialog = new QProgressDialog(nullptr);
        m_dialog->setWindowTitle(tr("Image Processing"));
        m_dialog->setWindowIcon(QIcon::fromTheme("kylin-photo-viewer"));
        m_dialog->setLabelText(tr("Batch Rotation in Progress..."));
        m_dialog->setFixedSize(300,80);
        m_dialog->setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
        m_dialog->setCancelButtonText(nullptr);

        m_progressBar = new QProgressBar(m_dialog);
        m_dialog->setBar(m_progressBar);
    m_dialog->setMinimumSize(300,80);
    m_progressBar->setRange(0, Uris.length());
    m_progressBar->setValue(0);

    connect(KylinImageCodec::getSignalObj(), &KylinImageCodecSignals::saveMovieFinish,this,&RotateMenuPlugin::updateProgress,Qt::UniqueConnection);
    connect(m_dialog, &QProgressDialog::canceled, this,&RotateMenuPlugin::closeProgress);
#ifndef UNUI
    m_dialog->show();
#endif
}

void RotateMenuPlugin::closeProgress(){
#ifndef UNUI
    if ( m_dialog->wasCanceled()) {
        rotateProcessor->m_isClosed = true;
        if(m_dialog){
            disconnect(KylinImageCodec::getSignalObj(), &KylinImageCodecSignals::saveMovieFinish,this,&RotateMenuPlugin::updateProgress);
            disconnect(KylinImageCodec::getSignalObj(), &KylinImageCodecSignals::saveMovieFinish,this,&RotateMenuPlugin::movieRotateFinish);
            disconnect(rotateProcessor, &Rotation::imageFinishSig, this, &RotateMenuPlugin::updateProgress);
            m_dialog->deleteLater();
            m_dialog = nullptr;
        }
    }
#endif
}

//进度条更新
void RotateMenuPlugin::updateProgress()
{   
    if( m_progressBar != nullptr && m_dialog != nullptr ){
        m_progressBar->setValue( m_progressBar->value() + 1);
    }

    if (m_progressBar->value() == m_progressBar->maximum()){
        disconnect(KylinImageCodec::getSignalObj(), &KylinImageCodecSignals::saveMovieFinish,this,&RotateMenuPlugin::updateProgress);
        if(m_dialog){
            disconnect(KylinImageCodec::getSignalObj(), &KylinImageCodecSignals::saveMovieFinish,this,&RotateMenuPlugin::updateProgress);
            disconnect(KylinImageCodec::getSignalObj(), &KylinImageCodecSignals::saveMovieFinish,this,&RotateMenuPlugin::movieRotateFinish);
            disconnect(rotateProcessor, &Rotation::imageFinishSig, this, &RotateMenuPlugin::updateProgress);
            m_dialog->deleteLater();
            m_dialog = nullptr;
        }
    }
}

void RotateMenuPlugin::rotateFinish()
{
    imagePaths.clear();
    isHaveImage = false;
    isShowProcessBar = false;
}

void RotateMenuPlugin::movieRotateFinish()
{
    rotateProcessor->m_movieRotate = true;
}
