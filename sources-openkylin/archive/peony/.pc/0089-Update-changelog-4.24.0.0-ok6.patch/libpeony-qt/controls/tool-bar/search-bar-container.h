/*
 * Peony-Qt
 *
 * Copyright (C) 2020, Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: MeihongHe <hemeihong@kylinos.cn>
 *
 */

#ifndef SEARCH_BAR_CONTAINER_H
#define SEARCH_BAR_CONTAINER_H

#include <QWidget>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QStringListModel>
#include <QListView>
#include <QStringList>
#include <QTimer>
#include <QProxyStyle>
#include <QPaintEvent>
#include <QVariantAnimation>
#include <QProxyStyle>
#include <QPainter>
#include "idm-dbus-manager.h"
namespace Peony {
class GradientLineEditStyle : public QProxyStyle
{
public:
    void drawPrimitive(PrimitiveElement pe, const QStyleOption *opt,
                       QPainter *p, const QWidget *w) const override {
        if (pe == PE_PanelLineEdit) {
            // 1. 先绘制默认背景
            QProxyStyle::drawPrimitive(pe, opt, p, w);

            // 2. 如果是焦点状态，绘制渐变边框
            if (w && w->hasFocus()) {
                bool isisAiAvailable = Peony::IDMDBusManager::getInstance()->isAiAvailable();
                if(!isisAiAvailable){
                    QProxyStyle::drawPrimitive(pe, opt, p, w);
                    return;
                }
                p->save();

                QLinearGradient grad(opt->rect.left(), opt->rect.top(),
                                     opt->rect.right(), opt->rect.top());
                grad.setColorAt(0, QColor("#E888F8"));
                grad.setColorAt(0.35, QColor("#E888F8"));
                grad.setColorAt(0.74, QColor("#4E75FF"));
                grad.setColorAt(1, QColor("#0036F5"));

                p->setRenderHint(QPainter::Antialiasing);
                p->setPen(QPen(grad, 2));
                p->setBrush(Qt::NoBrush);
                p->drawRoundedRect(opt->rect.adjusted(1,1,-1,-1), 6, 6);

                p->restore();
            }
            return;
        }
        QProxyStyle::drawPrimitive(pe, opt, p, w);
    }
};

class ProgressLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit ProgressLineEdit(QWidget *parent = nullptr);

public Q_SLOTS:
    void updateSearchProgress(bool searching);

protected:
    void paintEvent(QPaintEvent *e) override;

private:
    QVariantAnimation *m_animation = nullptr;
    bool m_searching = false;
    double m_value = 0;
};

class SearchBarContainer : public QWidget
{
    Q_OBJECT
public:
    explicit SearchBarContainer(QWidget *parent = nullptr);

    QSize sizeHint() const override;

    void setPlaceholderText(const QString &content) {
        m_search_box->setPlaceholderText(content);
    }
    void setFocus() {
        m_search_box->setFocus();
    }
    QString text() {
        return m_search_box->text();
    }
    void setText(QString text) {
        m_search_box->setText(text);
    }

    //get user selected index of file type
    int getFilterIndex() {
        return m_filter_box->currentIndex();
    }

    void clearFilter() {
        m_filter_box->setCurrentIndex(0);
    }

    void deselectSearchBox() {
        // Take action in the search bar to refresh search results
        m_search_box->deselect();
    }

    void setSearchBoxHeight(qint32 height) {
        m_search_box->setFixedHeight(height);
    }

Q_SIGNALS:
    void returnPressed();
    void filterUpdate(const int &index);
    void updateSearchProgress(bool searching);
    void updateLastLocationPath();

public Q_SLOTS:
    void onTableClicked(const QModelIndex &index);
    void changeSearchStatus();
    void startSearch();
    void stopSearch();
    void clearSearchBox();

private:
    GradientLineEditStyle *m_lineStyle = nullptr;
    QHBoxLayout *m_layout = nullptr;

    ProgressLineEdit *m_search_box;
    QComboBox *m_filter_box;

    QStringListModel *m_model = nullptr;
    QListView *m_list_view = nullptr;
    QToolButton* m_stopSearchButton = nullptr;

    QTimer m_search_trigger;

    bool m_clear_action = true;
    bool m_searching = false;

    QStringList m_file_type_list = {tr("all"), tr("file folder"), tr("image"),
                                    tr("video"), tr("text file"), tr("audio"), tr("wps file"), tr("others")
                                   };
};

class ToolButtonStyle : public QProxyStyle
{
public:
    static ToolButtonStyle *getStyle();
    ToolButtonStyle() {}

    void drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const override;
};
}


#endif // SEARCH_BAR_CONTAINER_H
