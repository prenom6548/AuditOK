#include "ukui_previewhub_api.h"

#include "content-preview-page.h"

#include <QCoreApplication>
#include <QLocale>
#include <QString>
#include <QStandardPaths>
#include <QTranslator>

namespace {

void installPreviewHubTranslator()
{
    static bool installed = false;
    if (installed)
        return;

    installed = true;
    const QString fileName = QStringLiteral("ukui-previewhub_") +
        QLocale::system().name() + QStringLiteral(".qm");
    const QString path = QStandardPaths::locate(
        QStandardPaths::GenericDataLocation,
        QStringLiteral("ukui-previewhub/translations/") + fileName);
    if (path.isEmpty())
        return;

    QTranslator *translator = new QTranslator(QCoreApplication::instance());
    if (translator->load(path))
        QCoreApplication::installTranslator(translator);
    else
        delete translator;
}

}

extern "C" {

void *ukui_previewhub_preview_widget_create(void *parent_qwidget)
{
    return ukui_previewhub_preview_widget_create_with_mode(
        parent_qwidget, UKUI_PREVIEWHUB_PREVIEW_MODE_COMPLEX);
}

void *ukui_previewhub_preview_widget_create_with_mode(
    void *parent_qwidget, UkuiPreviewHubPreviewMode mode)
{
    installPreviewHubTranslator();
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
