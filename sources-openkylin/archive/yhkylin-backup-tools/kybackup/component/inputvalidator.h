#ifndef INPUTVALIDATOR_H
#define INPUTVALIDATOR_H

#include <QRegExpValidator>

class InputValidator : public QRegExpValidator
{
    Q_OBJECT
public:
    InputValidator(const QRegExp &rx, QObject *parent = nullptr);
    virtual ~InputValidator();

public:
    virtual QValidator::State validate(QString &input, int &pos) const override;

signals:
    void checked(bool valid, QString input, int pos, QChar c) const;
};

#endif // INPUTVALIDATOR_H
