#include <QWidget>
#include <QLayout>

class FlowLayout : public QLayout
{
public:
    FlowLayout(QWidget *parent, int margin = 0, int spacing = -1)
        : QLayout(parent)
    {
        setContentsMargins(margin, margin, margin, margin);
        setSpacing(spacing);
    }

    ~FlowLayout()
    {
        QLayoutItem *item;
        while ((item = takeAt(0)))
            delete item;
    }

    void addItem(QLayoutItem *item) override
    {
        itemList.append(item);
    }

    int count() const override
    {
        return itemList.size();
    }

    QLayoutItem *itemAt(int index) const override
    {
        return itemList.value(index);
    }

    QLayoutItem *takeAt(int index) override
    {
        if (index >= 0 && index < itemList.size())
            return itemList.takeAt(index);
        return nullptr;
    }

    Qt::Orientations expandingDirections() const override
    {
        return {};
    }

    bool hasHeightForWidth() const override
    {
        return true;
    }

    int heightForWidth(int width) const override
    {
        int height = doLayout(QRect(0, 0, width, 0), true);
        return height;
    }

    void setGeometry(const QRect &rect) override
    {
        QLayout::setGeometry(rect);
        doLayout(rect, false);
    }

    QSize sizeHint() const override
    {
        return minimumSize();
    }

    QSize minimumSize() const override
    {
        QSize size;
        for (const auto &item : itemList)
            size = size.expandedTo(item->minimumSize());

        size += QSize(2 * margin(), 2 * margin());
        return size;
    }

private:
    int doLayout(const QRect &rect, bool testOnly) const
    {
        int x = rect.x();
        int y = rect.y();
        int lineHeight = 0;

        for (auto item : itemList) {
            QWidget *wid = item->widget();
            int spaceX = spacing();
            int spaceY = spacing();
            int nextX = x + item->sizeHint().width() + spaceX;
            if (nextX - spaceX > rect.right() && lineHeight > 0) {
                x = rect.x();
                y = y + lineHeight + spaceY;
                nextX = x + item->sizeHint().width() + spaceX;
                lineHeight = 0;
            }

            if (!testOnly)
                item->setGeometry(QRect(QPoint(x, y), item->sizeHint()));

            x = nextX;
            lineHeight = qMax(lineHeight, item->sizeHint().height());
        }
        return y + lineHeight - rect.y();
    }

    QList<QLayoutItem *> itemList;
};