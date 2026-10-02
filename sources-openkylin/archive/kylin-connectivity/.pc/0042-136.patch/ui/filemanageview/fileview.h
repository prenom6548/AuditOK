#ifndef FILEVIEW_H
#define FILEVIEW_H

#include <QWidget>
#include <QListView>
#include <QStandardItemModel>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QMenu>
#include <QAction>
#include <QMap>
#include <kysdk/applications/klistview.h>

#include "fileinfo.h"
#include "publicattributes.hpp"
#include "levitationbutton.h"

class FileView : public kdk::KListView
{
    Q_OBJECT
public:
    FileView(QWidget *parent = nullptr);

    enum ItemSelectMode {
        Default = 0,  // 默认
        Edit,         // 编辑模式
    };

    enum ThumbnailType {
        Image = 0,  // 图片缩略图
        Video,      // 视频缩略图
        None        // 无缩略图
    };

    void setViewMode(QListView::ViewMode mode);
    QListView::ViewMode getViewMode() const;
    void setModelFlag(PublicAttributes::Model model);
    void setViewInfo(const QMap<QString, FileInfo> &map);
    void setViewInfo(const QList<FileInfo> &list);
    void setTheme(PublicAttributes::Theme theme);
    PublicAttributes::Theme getTheme() const;
    void changeFontSize(double fontSize);
    double getFontSize() const;
    void setItemMode(FileView::ItemSelectMode mode, bool isClear = false);
    FileView::ItemSelectMode getItemMode() const;
    void setThumbnailType(ThumbnailType type);
    void setItemThumbnail(QString filePath);
    void setDownloadBtn(bool isShow);
    void setAllowDrag(bool isAllowDrag);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void setInfo(const QMap<QString, FileInfo> &map);
    void setInfo(const QList<FileInfo> &list);
    void openFile(const QModelIndex &index);
    void sendDownloadInfo();
    void updateViewMode();
    QSize getIconSize();
    QIcon getPictureIcon(QString path);
    QIcon getVideoIcon(QString path);

Q_SIGNALS:
    void sigOpenFile(FileInfo::FileType, QString);
    void sigSelectionModels(bool);
    void sigDownFileInfo(QString, QList<FileInfo> &fileList);
    void sigItemSelectAll(bool);
    void sigUploadFileInfo(QString, const QStringList &);
    void sigCtrlAClicked();

private Q_SLOTS:
    void slotDoubleClicked(const QModelIndex &index);
    void slotClicked(const QModelIndex &index);
    void slotDownloadBtnClicked();
    void slotContextMenu(const QPoint &pos);
    void slotOpenTriggered();
    void slotDownloadTriggered();

private:
    QStandardItemModel *m_listModel = nullptr;
    LevitationButton *m_downloadBtn = nullptr;                           // 下载悬浮按钮

    QMap<QString, QStandardItem *> m_itemMap;                            // 用于设置缩略图
    PublicAttributes::Model m_model = PublicAttributes::Model::PcModel;  // 当前设备模式
    QListView::ViewMode m_viewModel = QListView::IconMode;               // 当前视图模式
    PublicAttributes::Theme m_theme = PublicAttributes::Theme::Light;    // 当前主题
    double m_fontSize = 10;
    int m_listItemHeight = 0;
    FileView::ItemSelectMode m_itemMode = FileView::ItemSelectMode::Default;  // 当前item视图模式
    ThumbnailType m_thumbnailType = ThumbnailType::None;                      // 当前缩略图类型
    bool m_isAllowDrag = true;
    bool m_isClicked = false;
};

#endif  // FILEVIEW_H
