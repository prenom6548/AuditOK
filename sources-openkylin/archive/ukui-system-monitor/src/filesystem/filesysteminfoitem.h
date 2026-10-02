#ifndef FILESYSTEMINFOITEM_H
#define FILESYSTEMINFOITEM_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <qgsettings.h>
#include <QColor>
#include <QPalette>
#include <QProgressBar>

#include <kysdk/applications/klabel.h>
using namespace kdk;

#include "filesystemdata.h"

class FileSystemInfoItem : public QPushButton
{
    Q_OBJECT
public:
    explicit FileSystemInfoItem(QWidget *parent = nullptr);
    virtual ~FileSystemInfoItem();

public slots:
    void onUpdateInfo(FileSystemData& fsData);

signals:
    void dblClicked(QString strMountUrl);

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *watched, QEvent *event);

private:
    void initUI();
    void initConnections();
    void initStyleTheme();
    void onThemeStyleChange(QString strStyleName);
    void adjustContentSize();

private:
    QHBoxLayout *m_mainLayout = nullptr;
    QVBoxLayout *m_contentLayout = nullptr;
    QHBoxLayout *m_contentMidLayout = nullptr;

    KLabel *m_labelDevName = nullptr;
    KLabel *m_labelUsedCapacity = nullptr;
    KLabel *m_labelOther = nullptr;
    KLabel *m_iconDev = nullptr;
    QProgressBar *m_progressBar = nullptr;

    QPalette m_peLabel;
    QGSettings *m_styleSettings = nullptr;
    FileSystemData m_fsData;
    int mBoardRadius = 6;
};

#endif // FILESYSTEMINFOITEM_H
