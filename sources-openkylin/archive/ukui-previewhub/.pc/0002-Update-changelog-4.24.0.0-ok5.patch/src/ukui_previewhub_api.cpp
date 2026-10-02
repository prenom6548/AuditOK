#include "ukui_previewhub_api.h"

#include "content-preview-page.h"

#include <QString>

extern "C" {

void *ukui_previewhub_preview_widget_create(void *parent_qwidget)
{
    return ukui_previewhub_preview_widget_create_with_mode(
        parent_qwidget, UKUI_PREVIEWHUB_PREVIEW_MODE_COMPLEX);
}

void *ukui_previewhub_preview_widget_create_with_mode(
    void *parent_qwidget, UkuiPreviewHubPreviewMode mode)
{
    QWidget *parent = static_cast<QWidget *>(parent_qwidget);
    const bool simplePreview = mode == UKUI_PREVIEWHUB_PREVIEW_MODE_SIMPLE;
    return static_cast<void *>(new ContentPreviewPage(parent, simplePreview));
}

void ukui_previewhub_preview_widget_destroy(void *widget)
{
    if (!widget)
        return;
    delete static_cast<ContentPreviewPage *>(widget);
}

void ukui_previewhub_preview_prepare(void *widget, const char *uri_utf8)
{
    if (!widget || !uri_utf8)
        return;
    static_cast<ContentPreviewPage *>(widget)->prepare(QString::fromUtf8(uri_utf8));
}

void ukui_previewhub_preview_start(void *widget)
{
    if (!widget)
        return;
    static_cast<ContentPreviewPage *>(widget)->startPreview();
}

void ukui_previewhub_preview_cancel(void *widget)
{
    if (!widget)
        return;
    static_cast<ContentPreviewPage *>(widget)->cancel();
}

}
