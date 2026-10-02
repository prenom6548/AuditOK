/*
 * Copyright 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "languagecode.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <libintl.h>

LanguageCode::LanguageCode() {
  m_iso639_2data = readAlpha3ToNameMap(ISOCODES_ISO639_2_JSON, "639-2");
  m_iso639_3data = readAlpha3ToNameMap(ISOCODES_ISO639_3_JSON, "639-3");
  m_iso639_5data = readAlpha3ToNameMap(ISOCODES_ISO639_5_JSON, "639-5");
}

QString LanguageCode::codeToLocale(const QString &code) {

  auto value = m_iso639_2data.value(code);
  if (!value.isEmpty()) {
    return translateDomain("iso_639-2", value.toUtf8().constData());
  }
  value = m_iso639_3data.value(code);
  if (!value.isEmpty()) {
    return translateDomain("iso_639-3", value.toUtf8().constData());
  }
  value = m_iso639_5data.value(code);
  if (!value.isEmpty()) {
    return translateDomain("iso_639-5", value.toUtf8().constData());
  }
  return value;
}

void LanguageCode::addDomain(const QString &domain, QString dir) {
  if (domains_.count(domain)) {
    return;
  }
  if (dir.isEmpty()) {
    dir = LOCALE_INSTALL_FULL_DIR;
  }
  bindtextdomain(domain.toUtf8().constData(), dir.toUtf8().constData());
  bind_textdomain_codeset(domain.toUtf8().constData(), "UTF-8");
  domains_.insert(domain);
}

QString LanguageCode::translateDomain(const QString &domain, const QString &s) {
  addDomain(domain);
  return ::dgettext(domain.toUtf8().constData(), s.toUtf8().constData());
}

QMap<QString, QString> LanguageCode::readAlpha3ToNameMap(QString name,
                                                         QString base) {
  QMap<QString, QString> map;
  QFile file(name);
  file.open(QIODevice::ReadOnly);
  auto data = file.readAll();
  QJsonParseError error;
  auto document = QJsonDocument::fromJson(data, &error);
  if (error.error != QJsonParseError::NoError || !document.isObject()) {
    return {};
  }

  auto object = document.object();
  auto iso = object.value(base);
  if (!iso.isArray()) {
    return {};
  }
  const auto array = iso.toArray();
  for (const auto &item : array) {
    if (!item.isObject()) {
      continue;
    }
    auto alpha3 = item.toObject().value("alpha_3").toString();
    auto bibliographic = item.toObject().value("bibliographic").toString();
    auto name = item.toObject().value("name").toString();
    if (alpha3.isEmpty() || name.isEmpty()) {
      continue;
    }
    map.insert(alpha3, name);
    if (!bibliographic.isEmpty()) {
      map.insert(bibliographic, name);
    }
  }
  return map;
}
