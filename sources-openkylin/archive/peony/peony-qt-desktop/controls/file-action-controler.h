
#ifndef FILEACTIONCONTROLER_H
#define FILEACTIONCONTROLER_H

#include <QObject>
#include <QMenu>
#include <gio/gio.h>
#include "common.h"
namespace Peony {
class DesktopMenu;
}
class FileActionController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList selectedUris READ getSelectedUris WRITE setSelectedUris NOTIFY selectedUrisChanged)
    Q_PROPERTY(QString directryUri READ getDirectryUri WRITE setDirectryUri NOTIFY directryUriChanged)
    Q_PROPERTY(QSize iconSize READ getIconSize WRITE setIconSize NOTIFY iconSizeChanged)
    Q_PROPERTY(QSize gridSize READ getGridSize WRITE setGridSize NOTIFY gridSizeChanged)
    Q_PROPERTY(int emblemSize READ getEmblemSize WRITE setEmblemSize NOTIFY emblemChanged)
    Q_PROPERTY(bool dragDesktopWidgets READ dragDesktopWidgets WRITE setDragDesktopWidgets NOTIFY dragDesktopWidgetsChanged FINAL)
    /// 当前在桌面上“选中”的 ukui 小插件实例 id（左键点插件后用于右键菜单删除等），-1 表示未选中
    Q_PROPERTY(int focusedDesktopWidgetInstanceId READ focusedDesktopWidgetInstanceId WRITE setFocusedDesktopWidgetInstanceId NOTIFY focusedDesktopWidgetInstanceIdChanged FINAL)
    Q_PROPERTY(int clickedScreenId READ clickedScreenId WRITE setClickedScreenId NOTIFY clickedScreenIdChanged)

public:
    static FileActionController *getInstance();
    explicit FileActionController(QObject *parent = nullptr);
    static GAsyncReadyCallback queryZoomLevelAsyncCallback(GObject *obj,
            GAsyncResult *res,
            FileActionController *p_this);
    Q_INVOKABLE QString iconName(QIcon icon) {
        return icon.name();
    }
    Q_INVOKABLE QIcon icon(const QString &uri);
    Q_INVOKABLE bool enableShortcutKeys();
    Q_INVOKABLE void fileCopy(const QStringList &selectedUris);
    Q_INVOKABLE void fileCut(const QStringList &selectedUris);
    Q_INVOKABLE void filePaste(const QStringList &selectedUris);
    Q_INVOKABLE void deleteToTrash(const QStringList &selectedUris);
    Q_INVOKABLE void undo();
    Q_INVOKABLE void redo();
    Q_INVOKABLE void zoomIn();
    Q_INVOKABLE void zoomOut();
    Q_INVOKABLE void editUri(const QString &selectedUri);
    Q_INVOKABLE void remove(const QStringList &selectedUris);
    Q_INVOKABLE void help();
    Q_INVOKABLE void showProperties(const QStringList &selectedUris);
    Q_INVOKABLE void newFolder();
    Q_INVOKABLE void refresh();
    Q_INVOKABLE void invertSelections();
    Q_INVOKABLE void recoverNormalLevel();

    Q_INVOKABLE void setDefaultZoomLevel(ZoomLevel level);
    Q_INVOKABLE ZoomLevel getZoomLevel() const;
    Q_INVOKABLE void editUris(const QStringList &selectedUris);
    Q_INVOKABLE void setShowHidden();
    Q_INVOKABLE void clearClipboard();
    Q_INVOKABLE void setSelectAll();
    Q_INVOKABLE void showMenu(const QStringList &selectedUris, const QPoint &globalPos);
    Q_INVOKABLE void closeMenu();
    Q_INVOKABLE void openFileByUri(QString uri);
    Q_INVOKABLE void openFileByUri(const QStringList &uris);
    Q_INVOKABLE void rename(const QStringList& selections, const QString newName);
    Q_INVOKABLE void updateFileElideStatus(const QString &uri, bool isElided);
    Q_INVOKABLE void setClickedScreenId(const int screenId);
    Q_INVOKABLE QString getAppId(const QString uri);

    void openWindow(const QStringList &uris);
    void finishRefresh();
    QStringList getSelectedUris(){
        return m_newFilesToBeSelected;
    }
    void setSelectedUris(const QStringList &selectedUris){
        m_newFilesToBeSelected = selectedUris;
        Q_EMIT selectedUrisChanged(m_newFilesToBeSelected);
    }

    QString getDirectryUri(){
        return m_directryUri;
    }
    void setDirectryUri(const QString &directryUri){
        m_directryUri = directryUri;
    }

    QSize getIconSize(){
        return m_iconSize;
    }

    void setIconSize(const QSize &newIconSize){
        if (m_iconSize != newIconSize) {
            m_iconSize = newIconSize;
            Q_EMIT iconSizeChanged();
        }
    }

    QSize recalculateGridSize();

    QSize getGridSize(){
        return m_gridSize;
    }

    void setGridSize(const QSize &newGridSize);

    int getEmblemSize(){
        return m_emblemSize;
    }

    void setEmblemSize(const int newEmblemSize){
        if (m_emblemSize != newEmblemSize) {
            m_emblemSize = newEmblemSize;
            Q_EMIT emblemChanged();
        }
    }

    QStringList& urisToEdit() {
        return m_urisToEdit;
    }
    static QString getLanguageSpecificSample();

    bool dragDesktopWidgets() const;
    void setDragDesktopWidgets(bool newDragDesktopWidgets);

    int focusedDesktopWidgetInstanceId() const { return m_focusedDesktopWidgetInstanceId; }
    void setFocusedDesktopWidgetInstanceId(int instanceId);
    int clickedScreenId() const { return m_clickedScreenId; }

    Q_INVOKABLE void setIsOperationByPartition(bool flag)
    {
        operationByPartition = flag;
    }
    bool getIsOperationByPartition()
    {
        return operationByPartition;
    }
public Q_SLOTS:
    void paste(const QStringList &selectedUris);
    void triggerDesktopOrganization();
    void triggerQuitDesktopOrganization();

Q_SIGNALS:
    void selectedUrisChanged(const QStringList &selectedUris);
    void directryUriChanged();
    void iconSizeChanged();
    void gridSizeChanged();
    void emblemChanged();
    void selectAll();
    void edit(QString uri);
    void changeOpacity(double opacity);
    void clickedScreenIdChanged(int screenId);
    void focusedDesktopWidgetInstanceIdChanged();

#ifdef TEST_PEONY_DESKTOP_WIDGETS
    void requestManageWidgets();
#endif
    void dragDesktopWidgetsChanged();
    void desktopOriganization();
    void quitOriganization();

private:
    bool execSharedFileLink(const QString uri);

private:
    QStringList m_newFilesToBeSelected;
    QStringList m_urisToEdit;/* 新建文件/文件夹，可编辑文件名list */
    QString m_directryUri;
    ZoomLevel m_zoom_level = Invalid;
    QSize m_iconSize = QSize(64, 64);
    QSize m_gridSize = QSize(64, 64);
    Peony::DesktopMenu *m_menu = nullptr;
    int m_emblemSize = 16;
    int m_clickedScreenId = 0;
    int m_focusedDesktopWidgetInstanceId = -1;
    bool m_freshing = false;
    bool m_initialized = false;
    bool m_dragDesktopWidgets = false;
    bool operationByPartition = false;
};

#endif // FILEACTIONCONTROLER_H
