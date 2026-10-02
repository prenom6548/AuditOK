/*
 * Qt5-UKUI's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * Authors: Jing Tan <tanjing@kylinos.cn>
 *
 */

#ifndef KYBUTTON_H
#define KYBUTTON_H

#include <QObject>
#include <QColor>

namespace UKUIGlobalDTConfig {
class GlobalDTConfig;
}

namespace UKUISDKStyle {

class KyButton : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int styleType READ styleType WRITE setStyleType NOTIFY styleTypeChanged)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool hovered READ hovered WRITE setHovered NOTIFY hoveredChanged)
    Q_PROPERTY(bool pressed READ pressed WRITE setPressed NOTIFY pressedChanged)
    Q_PROPERTY(bool hasIcon READ hasIcon WRITE setHasIcon NOTIFY hasIconChanged)
    Q_PROPERTY(QColor contentColor READ contentColor NOTIFY contentColorChanged)
    Q_PROPERTY(QColor borderColor READ borderColor NOTIFY borderColorChanged)

public:
    enum StyleType {
        StyleBorderless = 0,
        StyleBorder = 1
    };
    Q_ENUM(StyleType)

    explicit KyButton(QObject *parent = nullptr);

    int styleType() const { return m_styleType; }
    void setStyleType(int styleType);

    bool enabled() const { return m_enabled; }
    void setEnabled(bool enabled);

    bool hovered() const { return m_hovered; }
    void setHovered(bool hovered);

    bool pressed() const { return m_pressed; }
    void setPressed(bool pressed);

    bool hasIcon() const { return m_hasIcon; }
    void setHasIcon(bool hasIcon);

    QColor contentColor() const { return m_contentColor; }
    QColor borderColor() const { return m_borderColor; }

    Q_INVOKABLE void refreshPalette();

signals:
    void styleTypeChanged();
    void enabledChanged();
    void hoveredChanged();
    void pressedChanged();
    void hasIconChanged();
    void contentColorChanged();
    void borderColorChanged();

private:
    void updateContentColor();
    void connectTheme();

    UKUIGlobalDTConfig::GlobalDTConfig *m_dt = nullptr;

    QColor m_disableColor;
    QColor m_hoverColor;
    QColor m_clickColor;
    QColor m_hoverEndColor;
    QColor m_clickEndColor;
    QColor m_iconNormalColor;
    QColor m_textNormalColor;
    QColor m_borderTextNormalColor;
    QColor m_borderLineNormalColor;

    int m_styleType = StyleBorderless;
    bool m_enabled = true;
    bool m_hovered = false;
    bool m_pressed = false;
    bool m_hasIcon = false;
    QColor m_contentColor;
    QColor m_borderColor;
};

} // namespace UKUISDKStyle

#endif // KYBUTTON_H
