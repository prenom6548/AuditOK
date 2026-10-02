#ifndef RADIUSWIDGET_H
#define RADIUSWIDGET_H

#include <QWidget>
#include <QPalette>
namespace kom
{
class RadiusWidget : public QWidget
{
    Q_OBJECT
public:
    RadiusWidget(QWidget *parent = nullptr);
    void setColorRole(QPalette::ColorRole role);
    void setRadius(int r);

private:
    void paintEvent(QPaintEvent *e);
    QPalette::ColorRole m_colorRole;
    int m_radius = 0;
};
} // namespace kom
#endif // RADIUSWIDGET_H
