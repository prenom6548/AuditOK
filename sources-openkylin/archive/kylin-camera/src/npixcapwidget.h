#ifndef NPIXCAPWIDGET_H
#define NPIXCAPWIDGET_H


#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>
#include <QPainterPath>
#include "util/custombuttonstyle.h"

class NPixCapWidget : public QWidget
{
    Q_OBJECT
public:
    NPixCapWidget();
    ~NPixCapWidget();
    CustomButtonStyle *close = nullptr;
    CustomButtonStyle *capture3 = nullptr;
    CustomButtonStyle *capture5 = nullptr;
    CustomButtonStyle *capture10 = nullptr;
    CustomButtonStyle *capture20 = nullptr;
    QWidget *mainWin = nullptr;
    QHBoxLayout *mainWinLayout = nullptr;
protected:
    void paintEvent(QPaintEvent *event);

Q_SIGNALS:
    void cap_emit(int);

private:
    QHBoxLayout *captureLayout = nullptr;
    int width;
    int height;
    QHBoxLayout *layout = nullptr;
    QWidget *widget = nullptr;
    void capCloseSlot();
    void cap3Slot();
    void cap5Slot();
    void cap10Slot();
    void cap20Slot();
};


#endif // NPIXCAPWIDGET_H
