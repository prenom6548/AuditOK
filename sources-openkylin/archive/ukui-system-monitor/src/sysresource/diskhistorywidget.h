#ifndef DISKHISTORYWIDGET_H
#define DISKHISTORYWIDGET_H

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QGSettings>
#include <QMouseEvent>
#include <kysdk/applications/klabel.h>
using namespace kdk;

class DiskHistoryWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DiskHistoryWidget(QWidget *parent = nullptr);
    ~DiskHistoryWidget();

    void setClickedFlags(bool clicked);
protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *watched, QEvent *event) Q_DECL_OVERRIDE;

private:
    void initUI();
    void initStyleTheme();
    void onThemeFontChange(float fFontSize);

private:
    float m_curFontSize;
    QGSettings *m_pStyleSettings = nullptr;

    QVBoxLayout *m_pMainVLayout = nullptr;
    QHBoxLayout *m_pReadHLayout = nullptr;
    QHBoxLayout *m_pWriteHLayout = nullptr;
    KLabel *m_pDiskTitleLabel = nullptr;
    QLabel *m_pIconReadLabel = nullptr;
    QLabel *m_pIconWriteLabel = nullptr;
    KLabel *m_pReadTitleLabel = nullptr;
    KLabel *m_pWriteTitleLabel = nullptr;
    KLabel *m_pReadValueLabel = nullptr;
    KLabel *m_pWriteValueLabel = nullptr;
    bool m_clicked = false;
    bool m_hover = false;
    qreal transparent = 0.4;

public slots:
    void themeChangedSlot(const QString &key);
    void onUpdateDiskData();

signals:
    void clicked();
};

#endif // DISKHISTORYWIDGET_H
