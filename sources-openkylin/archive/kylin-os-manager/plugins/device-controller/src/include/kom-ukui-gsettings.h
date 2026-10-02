#ifndef __KOM_UKUI_GSETTINGS_H__
#define __KOM_UKUI_GSETTINGS_H__

#include <memory>

#include <QObject>
#include <QVariant>

namespace kom
{

class UkuiGsettings : public QObject
{
	Q_OBJECT
public:
	static UkuiGsettings *getInstance(void);
	UkuiGsettings(const UkuiGsettings &) = delete;
	UkuiGsettings &operator=(const UkuiGsettings &) = delete;

	QVariant getFontSize(void);

private:
	UkuiGsettings();
	~UkuiGsettings();

	class Impl;
	std::unique_ptr<Impl> m_impl;
	static UkuiGsettings self;

Q_SIGNALS:
	void fontSizeChange(QVariant size);
};

};

#endif
