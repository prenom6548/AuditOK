#ifndef APPMANAGERITEMWIDGET_H
#define APPMANAGERITEMWIDGET_H

#include <QFrame>

#include "ISelectComboxItem.h"
#include "AppManagerSelectComboxItem.h"
#include "AppManagerSliderItem.h"
#include "ISelectComboxItem.h"
#include "BaseType.h"

using namespace UkuiAudioFramwork;

class AppManagerItemWidget;
typedef struct AppManagerItem
{
    uint32_t index;
    QString iconName;
    std::vector<int> inputStreamVec;
    std::vector<int> outputStreamVec;
    AppManagerItemWidget* wid;
} AppManagerItem;


class AppManagerItemWidget : public QWidget
{
    Q_OBJECT
public:
    AppManagerItemWidget(QLabel*, const QString&, QWidget* parent = nullptr);
    ~AppManagerItemWidget() = default;

public:
    void setRange(const VolumeSliderType&, int, int);

    void setValue(const VolumeSliderType&, int);

    void setMuted(const VolumeSliderType&, bool);

    void setCurrentIndex(const NodeType&, int);

    int findData(const NodeType&, const QVariant&);

    void insertItem(const NodeType&, const QString&, const QVariant& = QVariant());

    void removeItem(const NodeType&, int);

    void setEnabled(const NodeType&, bool);

    IVolumeSliderItem* getSliderItem() const;

    std::unordered_map<NodeType, ISelectComboxItem*> getSelectComboxItems() const;

Q_SIGNALS:
    void closeWindowSignals();

private Q_SLOTS:
    void closeWindowSlots();

private:
    void createItems();

    void initUi();

    void initSlots();

private:
    QString m_name {};
    QLabel* m_pTitileLabel {};
    QPushButton* m_pConfirmBtn {};

    std::unordered_map<VolumeSliderType, QString> m_volumeKeys {
        {VolumeSliderType::VOLUME_SLIDER_SINKINPUT, tr("Output Volume")},
    };
    std::unordered_map<NodeType, QStringList> m_selectKeys {
        {NodeType::OUTPUT, QStringList{std::initializer_list<QString>{tr("Output Device"), QObject::tr("This case does not support setting the output device")}}},
        {NodeType::INPUT, QStringList{std::initializer_list<QString>{tr("Input Device"), QObject::tr("This case does not support setting the input device")}}},
    };

    std::unordered_map<VolumeSliderType, IVolumeSliderItem*> m_volumeSliderItemMap;
    std::unordered_map<NodeType, ISelectComboxItem*> m_selectComboxItemMap;
};

#endif // APPMANAGERITEMWIDGET_H
