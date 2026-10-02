#ifndef __CALCULATOR_H__
#define __CALCULATOR_H__

#include <QWidget>
#include "plug_interface.h"

class Calculator : public QWidget , public KySmallPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(KySmallPluginInterface)
    Q_PLUGIN_METADATA(IID SP_PLUGIN_IID FILE "Calculator.json")

public:
    const QString name() const;               /* 名称 */
    const QString nameCN() const;             /* 中文名 */
    const QString description() const;        /* 描述 */
    int sortNum() const;                      /* 排序 */
    QWidget *createWidget(QWidget *parent);   /* 创建窗体 */

protected:

private:

};


#endif
