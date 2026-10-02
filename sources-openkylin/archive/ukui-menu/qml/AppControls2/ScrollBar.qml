import QtQuick 2.0
import org.ukui.quick.items 1.0 as UkuiItems

UkuiItems.DtThemeScrollBar {
    padding: (hovered || pressed) ? 3 : 5
    // The scroll bar is only shown when the content is too large to fit.
}
