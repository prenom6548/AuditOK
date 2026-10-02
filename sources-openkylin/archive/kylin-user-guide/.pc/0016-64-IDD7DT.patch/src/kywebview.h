#ifndef KYWEBVIEW_H
#define KYWEBVIEW_H

#include <QWebEngineView>
#include <QWebView>

class KyWebView : public QWebEngineView
{
    Q_OBJECT
public:
    explicit KyWebView(QWidget* parent = nullptr);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
};

#endif // KYWEBVIEW_H
