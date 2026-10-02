#ifndef BASEWIDGET_H
#define BASEWIDGET_H

#include <QWidget>
#include <QGSettings>
#include <QVBoxLayout>
#include <QScrollArea>

#ifdef KY_SDK_WAYLANDHELPER
#include <ukuiwindowhelper/ukuiwindowhelper.h>
#endif

class baseWidget : public QWidget
{
    Q_OBJECT
public:
    explicit baseWidget(QWidget *parent = nullptr);
    virtual ~baseWidget();
    void initOpacityGSettings();
    void showBaseWindow();
    void initUi();
    void setMaxHeight(int height);
    void updateSize();
    void setupWindowEffects();
    void setPanelPosition(int pos);

    QVBoxLayout * mainWindowVboxLayout;
    QTimer *m_hideTimer;

protected:
    void paintEvent(QPaintEvent * event);
    bool eventFilter(QObject *watched, QEvent *event);

private:
    QGSettings *m_gsTransOpacity = nullptr;
    qreal m_curTransOpacity = 1;

    QScrollArea *m_scrollArea;
    QWidget *m_contentWidget;
    int m_maxHeight = 600;
    int m_panelPosition = 0;
    
#ifdef KY_SDK_WAYLANDHELPER
    UkuiWindowHelper* m_windowHelper = nullptr;
#endif
};

#endif // BASEWIDGET_H
