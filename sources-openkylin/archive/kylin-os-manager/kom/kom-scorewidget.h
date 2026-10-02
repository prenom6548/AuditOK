#ifndef __KOM_SCORE_WIDGET_H__
#define __KOM_SCORE_WIDGET_H__

#include <QWidget>
#include <QLabel>

namespace kom {

class ScoreWidget : public QWidget
{
    Q_OBJECT
public:
    ScoreWidget(QWidget *parent = nullptr);
    void setStarPix(QImage light , QImage dark);
    int getScore();

private:
    bool eventFilter(QObject *watched, QEvent *event);
    void setPix(int i = 5);
    QLabel *m_l1;
    QLabel *m_l2;
    QLabel *m_l3;
    QLabel *m_l4;
    QLabel *m_l5;
    QImage m_lightPix;
    QImage m_darkPix;
    int m_score = 5;
};

} // namespace kom

#endif
