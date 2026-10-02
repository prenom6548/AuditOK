#ifndef CAMERAFILTERWIDGET_H
#define CAMERAFILTERWIDGET_H


#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>
#include <QToolButton>
#include <QPainterPath>

class cameraFilterWidget : public QWidget
{
    Q_OBJECT
public:
    cameraFilterWidget();
    ~cameraFilterWidget();
    QPushButton *original = nullptr;
    QPushButton *warm = nullptr;
    QPushButton *cool = nullptr;
    QPushButton *grayscale = nullptr;
    QWidget *mainWin = nullptr;
    QHBoxLayout *mainWinLayout = nullptr;
protected:
    void paintEvent(QPaintEvent *event);

Q_SIGNALS:
    void filter_emit(int);

private:
    QHBoxLayout *filterLayout = nullptr;
    int width;
    int height;
    void filterOriginalSlot();
    void filterWarmSlot();
    void filterCoolSlot();
    void filterGrayscaleSlot();
};

#endif // CAMERAFILTERWIDGET_H
