#include <QGSettings>
#include <QStringList>

#include "kom-ukui-gsettings.h"

namespace kom
{

UkuiGsettings UkuiGsettings::self;

class UkuiGsettings::Impl
{
public:
	Impl()
	{
#ifdef __v10__
		if (QGSettings::isSchemaInstalled("org.mate.interface"))
			m_fontSize.reset(new QGSettings("org.mate.interface"));
#else
		if (QGSettings::isSchemaInstalled("org.ukui.style"))
			m_fontSize.reset(new QGSettings("org.ukui.style"));
#endif
	}

	QVariant getFontSize(void)
	{
#ifdef __v10__
		if (m_fontSize && m_fontSize->keys().contains("fontName")) {
			QVariant fontSize = m_fontSize->get("fontName");
			QStringList items = fontSize.toString().trimmed().split(" ");
			items.removeAll(" ");
			if (!items.isEmpty())
				return items.takeLast();
		}
#else
		if (m_fontSize && m_fontSize->keys().contains("systemFontSize"))
			return m_fontSize->get("systemFontSize");
#endif
		return QVariant("");
	}

public:
	std::unique_ptr<QGSettings> m_fontSize;
};

UkuiGsettings::UkuiGsettings()
{
	m_impl.reset(new Impl());

	connect(m_impl->m_fontSize.get(), &QGSettings::changed, this, [=](const QString &key){
#ifdef __v10__
		if (key == "fontName")
			Q_EMIT fontSizeChange(m_impl->getFontSize());
#else
		if (key == "systemFontSize")
			Q_EMIT fontSizeChange(m_impl->getFontSize());

#endif
	});
}

UkuiGsettings::~UkuiGsettings()
{
	/* nothing to do */
}

UkuiGsettings *UkuiGsettings::getInstance(void)
{
	return &self;
}

QVariant UkuiGsettings::getFontSize(void)
{
	return m_impl->getFontSize();
}

}
