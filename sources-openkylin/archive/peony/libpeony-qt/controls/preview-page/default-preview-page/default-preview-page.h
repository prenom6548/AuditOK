/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef DEFAULTPREVIEWPAGE_H
#define DEFAULTPREVIEWPAGE_H

#include "peony-core_global.h"
#include "preview-page-plugin-iface.h"
#include <QStackedWidget>

#include <memory>
#include <QLabel>
#include <QMap>
#include <QDBusInterface>
#include <QScrollArea>
#include<QPushButton>
#include <QMutex>
#include <QCache>
class QVBoxLayout;
class QPushButton;
class QFormLayout;

#include <QLayout>
#include <QRect>
#include <QList>
#include <QStyle>

class labelFlowLayout : public QLayout
{
public:
    explicit labelFlowLayout(QWidget *parent, int margin = 0, int hSpacing = -1, int vSpacing = -1);
    explicit labelFlowLayout(int margin = 0, int hSpacing = -1, int vSpacing = -1);
    ~labelFlowLayout();

    void addItem(QLayoutItem *item) override;
    int horizontalSpacing() const;
    int verticalSpacing() const;
    Qt::Orientations expandingDirections() const override;
    bool hasHeightForWidth() const override;
    int heightForWidth(int width) const override;
    int count() const override;
    QLayoutItem *itemAt(int index) const override;
    QSize minimumSize() const override;
    void setGeometry(const QRect &rect) override;
    QSize sizeHint() const override;
    QLayoutItem *takeAt(int index) override;
    void clear();
private:
    int doLayout(const QRect &rect, bool testOnly) const;
    int smartSpacing(QStyle::PixelMetric pm) const;

    QList<QLayoutItem *> itemList;
    int m_hSpace;
    int m_vSpace;
};

class IntelligentLabelButton : public QPushButton {
    Q_OBJECT
public:
    explicit IntelligentLabelButton(const QString &text, QWidget *parent = nullptr)
        : QPushButton(parent), m_fullText(text) {
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        updateText();
    }

protected:
    void resizeEvent(QResizeEvent *event) override {
        QPushButton::resizeEvent(event);
        updateText();
    }

private:
    void updateText() {
        QFontMetrics fm(font());
        QString elidedText = fm.elidedText(m_fullText, Qt::ElideRight, width() - 12); // 保留边距
        if(m_fullText!=elidedText)
            setToolTip(m_fullText);
        QPushButton::setText(elidedText);
    }

    QString m_fullText;
};

namespace Peony {

class FileInfo;
class FileWatcher;
class IconContainer;
class FilePreviewPage;
/*!
 * \brief The DefaultPreviewPage class
 * \todo
 * Implement a preview page containing file thumbnail and descriptions.
 */
class PEONYCORESHARED_EXPORT DefaultPreviewPage : public QStackedWidget, public PreviewPageIface
{
    Q_OBJECT
public:
    explicit DefaultPreviewPage(QWidget *parent = nullptr);
    ~DefaultPreviewPage() override;

    void prepare(const QString &uri, PreviewType type) override;
    void prepare(const QString &uri) override;
    void startPreview() override;
    void cancel() override;
    void closePreviewPage() override;

public Q_SLOTS:
    void updateDateFormat(QString dateFormat);

protected:
    bool eventFilter(QObject *obj, QEvent *ev) override;
    void paintEvent(QPaintEvent *e) override;

private:
    void updatePreviewIconSize(int previewWidth);

    QString m_current_uri;
    PreviewType m_current_type;

    QWidget *m_empty_tab_widget;
    FilePreviewPage *m_preview_tab_widget;

    std::shared_ptr<FileInfo> m_info;
    std::shared_ptr<FileWatcher> m_watcher;

    bool m_support = true;
    bool m_is_picture = false;
    QString m_date_format = "";
    QDBusInterface  *mDbusDateServer;
    QScrollArea* m_scrollArea;
};

class FileCountOperation;
class FilePreviewPage : public QFrame
{
    friend class DefaultPreviewPage;
    Q_OBJECT
private:
    explicit FilePreviewPage(QWidget *parent = nullptr);
    ~FilePreviewPage();

public:
    void wrapData(QLabel *p_label, const QString &text);
    void updateForm(QSize size);
    void addSeparator();
    void setLabelProperties(QLabel *label);
    void setSpaceName(const QString& spaceName);
    QLabel *getLabel(const QString& text);

private:
    void fetchLabelsAndSummaryAsync(const QString &path);
    void updateLabelsAndSummary(const QStringList &labels, const QString &summary);
    void setIsPreviewBySearch(bool flage);

private Q_SLOTS:
    void updateInfo(FileInfo *info);
    void countAsync(const QString &uri);
    void updateCount();
    void cancel();

    void resizeIcon(QSize size);
    void onLabelsAndSummaryFetched(const QStringList &labels,  const QString &summary);
    void openInNewWindow(const QString &uri);

protected Q_SLOTS:
    void resetCount();
    void onPreparedOne(const QString &uri, quint64 size) {
        m_file_count++;
        if (uri.contains("/.")) {
            m_hidden_count++;
        }
        m_total_size += size;
        this->updateCount();
    }
    void onCountDone();

private:
    FileCountOperation *m_count_op = nullptr;
    quint64 m_file_count = 0;
    quint64 m_hidden_count = 0;
    quint64 m_total_size = 0;

    QVBoxLayout *m_layout;
    IconContainer *m_icon;
    QFormLayout *m_form;
    labelFlowLayout *m_intelligentLabelLayout;
    QWidget *m_lableContainer;
    QLabel *m_display_name_label;
    QLabel *m_type_label;
    QLabel *m_file_count_label;
    QLabel *m_total_size_label;
    QLabel *m_time_modified_label;
    QLabel *m_time_access_label;
    QLabel *m_time_create_label;
    QLabel *m_file_summery_label;
    QLabel *m_location_label;

    //image
    QLabel *m_image_size;
    QLabel *m_image_format;
    QMap<QLabel*, QString> m_form_label_map;
    QString m_currentFileUri;
    QString m_spaceName;
    QLabel* m_removeLabel;
    std::atomic_bool m_destroyed{false};
    static QMutex m_cacheMutex;
    static QCache<QString, QPair<QStringList, QString>> m_labelSummaryCache;
    std::shared_ptr<FileWatcher> m_thumbnail_watcher;
    bool m_isDir = false;
};
}

#endif // DEFAULTPREVIEWPAGE_H
