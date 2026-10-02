#ifndef PICTUREBUTTON_H
#define PICTUREBUTTON_H

#include <QPushButton>

class PictureIconLabel;
class QLabel;
class PictureButton : public QPushButton
{
    Q_OBJECT
public:
    explicit PictureButton(const QString &name, QWidget *parent = nullptr);
    explicit PictureButton(const QString &name, const QPixmap &pixmap, QWidget *parent = nullptr);
    explicit PictureButton(QString name, QStringList iStringList, QWidget *parent = nullptr);
    explicit PictureButton(QString name, QList<QPixmap> iconList, QWidget *parent = nullptr);

    void initUi();
    void setConnect();
    void setRect(QSize size);
    void setPixmap(const QPixmap &pixmap);
    void setBtnClicked(bool status);
    void setStatus(bool status);
    QString getStyle(int border, int radius) const;
    QString getThemeName() const;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    PictureIconLabel * m_iconLabel;
    int m_radius = 8;
    int m_width = 146;
    int m_heigh = 126;
    QString m_clickedBtnStyle;
    QString m_enterBtnStyle;
    QString m_leaveBtnStyle;
    QString m_themeName;
};

#endif // PICTUREBUTTON_H
