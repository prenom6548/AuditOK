#ifndef NETBUTTON_H
#define NETBUTTON_H
#include <QPushButton>
#include <QIcon>
#include <QLabel>
#include <QTimer>
#include <QVariantAnimation>

#define MIDDLE_COLOR 178

class RadioItemButton : public QPushButton
{
    Q_OBJECT

public:
    RadioItemButton(QWidget * parent = nullptr);
//    ~RadioItemButton();
    void setButtonIcon(const QIcon &icon);
    void setDefaultPixmap();
    void setActive(const bool &isActive);
    void setOobe(bool enable);
    enum PixmapColor {
        WHITE = 0,
        BLACK,
        GRAY,
        BLUE,
    };
    const QPixmap loadSvg(const QPixmap &source, const PixmapColor &color);
    const QPixmap loadSvgOobe(const QPixmap &source);
    bool containsRedColor(const QPixmap &pixmap);

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);

private:
    bool m_isActivated = false;
    bool m_oobe = false;
    QLabel * m_iconLabel = nullptr;
    QColor m_backgroundColor;

    QPixmap m_pixmap;

    void refreshButtonIcon();

private Q_SLOTS:

    void onPaletteChanged();
};

#endif // NETBUTTON_H

