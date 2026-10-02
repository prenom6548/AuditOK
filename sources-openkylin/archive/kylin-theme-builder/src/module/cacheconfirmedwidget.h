#ifndef CACHECONFIRMEDWIDGET_H
#define CACHECONFIRMEDWIDGET_H

#include "../fileProcess/fileprocess.h"
#include "../module/infocreatewidget.h"
#include <kaboutdialog.h>
#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>

class cacheConfirmedWidget : public kdk::KDialog
{
    Q_OBJECT
public:
    explicit cacheConfirmedWidget(QWidget *parent = nullptr);
protected:
    void closeEvent(QCloseEvent *event) override;
signals:
    void cacheSave();
    void cacheClean();
    void cacheCancel();
    void visibilityChanged(bool visible);
private:
    QVBoxLayout *m_cacheConfirmedWidget;
    QLabel *m_questionMessage;
    QHBoxLayout *m_cacheConfirmedBtn;
    QPushButton *m_saveCache;
    QPushButton *m_cleanCache;
    QPushButton *m_cancel;

};

#endif // CACHECONFIRMEDWIDGET_H
