#ifndef TABLETMODEDESKTOPWIDGET_H
#define TABLETMODEDESKTOPWIDGET_H

#include <QFrame>

class QButtonGroup;
class PictureButton;
class QGSettings;
class TabletModeDesktopWidget: public QFrame
{
    Q_OBJECT

public:
    explicit TabletModeDesktopWidget(QWidget* parent = nullptr);
    ~TabletModeDesktopWidget() = default;

private:
    void initLayout();
    void initTabletModeDesktopWidget();
    void initConnect();
    void initButtonGroup();
    QWidget* createTabletTextWidget() const;
    QWidget* createTabletPictureWidget() const;
    void setButtonChecked(PictureButton* button);

    void setTabletModeGsettings(const QString& type);

    bool getCurrentTabletMode() const;

    QButtonGroup* m_buttonGroup;
    QGSettings* m_tabletModeGsettings = nullptr;

};
#endif // TABLETMODEDESKTOPWIDGET_H
