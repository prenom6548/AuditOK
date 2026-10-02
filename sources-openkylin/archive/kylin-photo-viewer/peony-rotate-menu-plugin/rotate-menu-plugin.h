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

#ifndef ROTATEMENUPLUGIN_H
#define ROTATEMENUPLUGIN_H // ROTATEMENUPLUGIN_H

#include "peony-rotate-menu-plugin_global.h"
#include <menu-plugin-iface.h>
#include <file-info.h>
#include <gio/gio.h>
#include <QAction>
#include <QFileInfo>
#include <QTranslator>
#include <QLocale>
#include <QApplication>

#include <QUrl>
#include <QProgressDialog>
#include <QProgressBar>
#include "../codec/kylinimagecodec/kylinimagecodec.h"
#include "rotation.h"

namespace Peony {

class PEONYQTENGRAMPAMENUPLUGINSHARED_EXPORT RotateMenuPlugin: public QObject, public MenuPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID MenuPluginInterface_iid FILE "common.json")
    Q_INTERFACES(Peony::MenuPluginInterface)
public:
    explicit RotateMenuPlugin(QObject *parent = nullptr);

    PluginInterface::PluginType pluginType() override {return PluginInterface::MenuPlugin;}
    const QString name() override {return tr("Peony-Qt rotate Extension");}
    const QString description() override {return tr("rotate Menu Extension");}
    const QIcon icon() override {return QIcon::fromTheme("ukui-rotate-symbolic");}
    void setEnable(bool enable) override {m_enable = enable;}
    bool isEnable() override {return m_enable;}

    QString testPlugin() override {return "test rotate";}
    QList<QAction *> menuActions(Types types, const QString &uri, const QStringList &selectionUris) override;


    //图片旋转
    void urlToPath(const QStringList &Uris, QWidget *win);
    void is_have_image(const QStringList &Uris);        //是否包含图片
    bool is_write( QString imagePath);              //是否可写、是否是图片
    void initProcessBar(const QStringList &Uris);       //初始化进度条
    void updateProgress();                              //更新进度条
    void rotateFinish();                                //恢复默认值
    void movieRotateFinish();                           //动图是否保存完成
    void closeProgress();                               //关闭进度条

Q_SIGNALS:
    void sigImageRotate(QStringList imagePaths, bool way);

private:
    bool m_enable;
    void *m_securityFd = nullptr;

    QStringList imagePaths;                             //待旋转的图片列表
    QProgressDialog *m_dialog = nullptr;                //进度框
    QProgressBar *m_progressBar = nullptr;              //进度条
    bool isShowProcessBar = false;                      //是否显示进度条
    bool isHaveImage = false;                           //是否包含图片

    QThread *workerThread = nullptr;                    //工作线程
    Rotation *rotateProcessor = nullptr;                //旋转操作

    QStringList m_file_type_list = {"exr", "jfi", "jif", "jp2", "j2k", "jng", "wbmp", "xbm",
                                    "pnm", "tiff","tif", "webp", "bmp", "dib", "png", "jpg", "jpe",
                                    "jpeg", "pbm", "pgm", "ppm", "svg",  "ico", "xpm", "sr",
                                    "tga"};    // "psd","ras",暂不支持 "gif", "apng", 和看图同步 暂不支持

};
}

#endif // ROTATEMENUPLUGIN_H
