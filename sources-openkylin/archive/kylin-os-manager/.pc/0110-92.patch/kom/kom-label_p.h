#ifndef __KOM_LABEL_P_H__
#define __KOM_LABEL_P_H__

#include <QtGlobal>
#include <QColor>


#include "kom-label.h"

namespace kom
{
class KomLabelPrivate
{
    Q_DECLARE_PUBLIC(KomLabel)
public:
    KomLabelPrivate(KomLabel *q);
    virtual ~KomLabelPrivate();

    void fitText(void);
    QString transHtml(QString &text);
    double adaptFontSize(void);
    void setAlignment(Qt::Alignment a);

    KomLabel *q_ptr;
    QLabel *m_label;
    QString m_text;
    bool m_isBold;
    double m_fontSize;
    QColor m_firstNumColor;
    QColor m_secondNumColor;
    QColor m_thirdNumColor;
};

} // namespace kom

#endif
