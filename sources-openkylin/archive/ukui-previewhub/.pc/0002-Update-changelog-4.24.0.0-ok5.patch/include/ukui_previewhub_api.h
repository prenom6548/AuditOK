#ifndef UKUI_PREVIEWHUB_API_H
#define UKUI_PREVIEWHUB_API_H

#include "ukui_previewhub_export.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Controls how much image-preview UI is shown. */
typedef enum UkuiPreviewHubPreviewMode {
    UKUI_PREVIEWHUB_PREVIEW_MODE_SIMPLE = 0,
    UKUI_PREVIEWHUB_PREVIEW_MODE_COMPLEX = 1
} UkuiPreviewHubPreviewMode;

/**
 * Create the preview widget. Cast the return value to QWidget* in C++.
 * @param parent_qwidget Parent QWidget*, or nullptr for a top-level preview widget.
 */
UKUIPREVIEWHUB_EXPORT void *ukui_previewhub_preview_widget_create(void *parent_qwidget);

/**
 * Create the preview widget with the requested presentation mode.
 * The legacy create function uses complex mode.
 */
UKUIPREVIEWHUB_EXPORT void *ukui_previewhub_preview_widget_create_with_mode(
    void *parent_qwidget, UkuiPreviewHubPreviewMode mode);

UKUIPREVIEWHUB_EXPORT void ukui_previewhub_preview_widget_destroy(void *widget);

/**
 * @param uri_utf8 File URI as UTF-8, typically "file:///path/to/file".
 */
UKUIPREVIEWHUB_EXPORT void ukui_previewhub_preview_prepare(void *widget, const char *uri_utf8);

UKUIPREVIEWHUB_EXPORT void ukui_previewhub_preview_start(void *widget);

UKUIPREVIEWHUB_EXPORT void ukui_previewhub_preview_cancel(void *widget);

#ifdef __cplusplus
}

#  include <QWidget>
#  include <QString>

namespace UkuiPreviewHub {

enum class PreviewMode {
    Simple = UKUI_PREVIEWHUB_PREVIEW_MODE_SIMPLE,
    Complex = UKUI_PREVIEWHUB_PREVIEW_MODE_COMPLEX
};

/** Returns a QWidget* that hosts the shared preview implementation. */
inline QWidget *createPreviewWidget(QWidget *parent = nullptr,
                                    PreviewMode mode = PreviewMode::Complex)
{
    return static_cast<QWidget *>(ukui_previewhub_preview_widget_create_with_mode(
        parent, static_cast<UkuiPreviewHubPreviewMode>(mode)));
}

inline void destroyPreviewWidget(QWidget *widget)
{
    ukui_previewhub_preview_widget_destroy(widget);
}

inline void preparePreview(QWidget *widget, const QString &uri)
{
    ukui_previewhub_preview_prepare(widget, uri.toUtf8().constData());
}

inline void startPreview(QWidget *widget)
{
    ukui_previewhub_preview_start(widget);
}

inline void cancelPreview(QWidget *widget)
{
    ukui_previewhub_preview_cancel(widget);
}

} // namespace UkuiPreviewHub

#endif

#endif
