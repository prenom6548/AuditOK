#ifndef TIPDISPLAYWIN_H
#define TIPDISPLAYWIN_H

#include <QObject>
#include <QWidget>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>

class TipDisplayWin : public QWidget
{
    Q_OBJECT
public:
    TipDisplayWin(QWidget *parent = nullptr);
    void setAllowDrag(bool isAllowDrag);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

Q_SIGNALS:
    void sigUploadFileInfo(QString, const QStringList &);

private:
    bool m_isAllowDrag = true;
};

#endif  // TIPDISPLAYWIN_H
