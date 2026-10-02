#ifndef OUTPUTWIDGET_H
#define OUTPUTWIDGET_H

#include <QWidget>
#include <QScopedPointer>

class OutputWidgetPrivate;

/**
 * @brief 构建过程输出界面
 */
class OutputWidget : public QWidget
{
    Q_OBJECT

public:
    OutputWidget(QWidget *parent = nullptr);
    virtual ~OutputWidget();

    void startBuild(QString yamlPath);

signals:
    void back();
    void backOutputClicked();
    void backHomeOutputClicked();

private:
    Q_DECLARE_PRIVATE(OutputWidget)
    QScopedPointer<OutputWidgetPrivate> d_ptr;
};

#endif // OUTPUTWIDGET_H
