#ifndef SEARCH_LINE_EDIT_TEST_H
#define SEARCH_LINE_EDIT_TEST_H

#include <QObject>

class SearchLineEditTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testSearchLineEditEmitsImmediateThenDebounce();
    void testSearchLineEditPaintFocusAndRadiusPaths();
    void testSearchBarWidgetSetTextAndClear();
    void testSearchBarWidgetReSearchEmitsCurrentText();
    void testSearchBarWidgetPaintAndSettingsSignal();
    void testLineEditStyleDrawPrimitiveBranches();
};

#endif // SEARCH_LINE_EDIT_TEST_H
