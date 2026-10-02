#ifndef UBRCONFIG_H
#define UBRCONFIG_H

#include "ui_ubrconfig.h"
#include <KCModule>
#include <QWidget>

namespace Ui
{
class UBRConfig;
}

class UBRConfig : public KCModule
{
    Q_OBJECT

public:
    explicit UBRConfig(QWidget *parent = nullptr, const QVariantList &args = QVariantList());
    ~UBRConfig();

    void save() override;

private:
    Ui::UBRConfig *ui;
};

#endif // UBRCONFIG_H
