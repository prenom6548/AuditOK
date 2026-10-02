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

#ifndef LANGUAGECODE_H
#define LANGUAGECODE_H

#include <QMap>
#include <QString>
#include <unordered_set>
class LanguageCode {
public:
  LanguageCode();

  QString codeToLocale(const QString &code);

private:
  QMap<QString, QString> m_iso639_2data;
  QMap<QString, QString> m_iso639_3data;
  QMap<QString, QString> m_iso639_5data;

private:
  void addDomain(const QString &domain, QString dir = "");
  QString translateDomain(const QString &domain, const QString &s);
  QMap<QString, QString> readAlpha3ToNameMap(QString name, QString base);
  std::unordered_set<QString> domains_;
  const QString ISOCODES_ISO639_2_JSON =
      "/usr/share/iso-codes/json/iso_639-2.json";
  const QString ISOCODES_ISO639_3_JSON =
      "/usr/share/iso-codes/json/iso_639-3.json";
  const QString ISOCODES_ISO639_5_JSON =
      "/usr/share/iso-codes/json/iso_639-5.json";
  const QString LOCALE_INSTALL_FULL_DIR = "/usr/share/locale";
};

#endif // LANGUAGECODE_H
