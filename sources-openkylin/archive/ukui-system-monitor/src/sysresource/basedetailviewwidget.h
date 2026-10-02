#ifndef BASEDETAILVIEWWIDGET_H
#define BASEDETAILVIEWWIDGET_H

#include <QWidget>
#include <QHBoxLayout>
#include <kborderlessbutton.h>
#include <kysdk/applications/klabel.h>
using namespace kdk;

class BaseDetailViewWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BaseDetailViewWidget(QWidget *parent = nullptr);
    ~BaseDetailViewWidget();

    void setTitle(const QString &text);

    void setDetail(const QString &text);

private:
    void initUI();
    void initConnect();

protected:
    QFont m_contentFont;
    QFont m_titleFont;

    QString mTitleText;
    QString mDetailText;
    QLabel *mTitleLabel = nullptr;
    KLabel *mDetailLabel = nullptr;
    KBorderlessButton *mHideBtn = nullptr;

    QVBoxLayout *mMainLayout;

signals:
    void hideDetails(int index);
};

#endif // BASEDETAILVIEWWIDGET_H
