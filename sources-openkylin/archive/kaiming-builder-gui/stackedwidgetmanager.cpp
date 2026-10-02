#include "stackedwidgetmanager.h"

StackedWidgetManager::StackedWidgetManager(QWidget* parent)
    : QStackedWidget(parent)
{}

StackedWidgetManager::~StackedWidgetManager()
{}

StackedPage StackedWidgetManager::addWidget(StackedPage index, QWidget* w)
{
    if (index < StackedPage::END)
    {
        insertWidget((int)index, w);
        return index;
    }

    return StackedPage::END;
}

void StackedWidgetManager::setCurrentPage(StackedPage index)
{
    if (index < StackedPage::END)
    {
        setCurrentIndex((int)index);
    }
}
