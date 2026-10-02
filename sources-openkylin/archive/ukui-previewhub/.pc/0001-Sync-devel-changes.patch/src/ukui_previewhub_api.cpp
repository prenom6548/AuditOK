#include "ukui_previewhub_api.h"

#include "content-preview-page.h"

#include <QString>

extern "C" {

void *ukui_previewhub_preview_widget_create(void *parent_qwidget)
{
    QWidget *parent = static_cast<QWidget *>(parent_qwidget);
    return static_cast<void *>(new ContentPreviewPage(parent));
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
