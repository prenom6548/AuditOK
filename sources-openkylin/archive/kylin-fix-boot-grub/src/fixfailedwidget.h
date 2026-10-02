#ifndef FIXFAILDWIDGET_H
#define FIXFAILDWIDGET_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFrame>
#include <QPalette>
#include <QPainterPath>

class fixFailedWidget : public QWidget
{
    Q_OBJECT
public:
    explicit fixFailedWidget(QWidget *parent = nullptr);
    ~fixFailedWidget();
    void initUI();

public:
    QVBoxLayout     *m_pVboxLayout = nullptr;

    QWidget         *m_pFailedIconWidget = nullptr;
    QHBoxLayout     *m_pHboxIconLayout = nullptr;
    QLabel          *m_pFailedIconLabel = nullptr;

    QWidget         *m_pFailedTextWidget = nullptr;
    QHBoxLayout     *m_pFailedTextLayout = nullptr;
    QLabel          *m_pFailedTextLabel = nullptr;

    QFrame          *m_pFrameLine = nullptr;
    QWidget         *m_pFailedButtonWidget = nullptr;
    QHBoxLayout     *m_pFailedButtonLayout = nullptr;
    QPushButton     *m_pFailedConfirmButton = nullptr;

private slots:
    void confirmButtonFaildSlots();

signals:

};

#endif // FIXFAILDWIDGET_H
