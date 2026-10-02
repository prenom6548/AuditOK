#ifndef K_ICON_LABEL_H
#define K_ICON_LABEL_H

#include <QLabel>
#include <QTimer>
#include <QColor>
#include <QIcon>

namespace K {

class IconLabel: public QLabel {
    Q_OBJECT
public:
    enum class BackgroundMode {
        kCircularMode
    };

    IconLabel(QWidget *parent = nullptr);
    ~IconLabel();
    void Loading(bool is_loading);
    void SetBackgroundColor(const QColor &color, BackgroundMode background_mode = BackgroundMode::kCircularMode);
    void SetIcon(const QIcon &icon, double reduction_ratio = 0);    // reduction_ratio 为显示图片的矩形比当前控件缩小的比例，值为 0 - 1 之间
    void SetPixmap(const QPixmap &pixmap, double reduction_ratio = 0);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QColor background_color_;
    BackgroundMode background_mode_;
    QIcon icon_;
    double icon_reduction_ratio_;
    QPixmap pixmap_;
    double pixmap_reduction_ratio_;
    QTimer *timer_;
    int start_angle_;
    bool is_loading_;
};

}

#endif
