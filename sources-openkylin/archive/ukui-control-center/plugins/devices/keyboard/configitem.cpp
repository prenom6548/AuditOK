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

#include "configitem.h"
#include "configvarianthelper.h"
#include "listoptionitem.h"
#include <QtGui/qwindowdefs.h>

class IntegerOptionItem : public ConfigItem {
  Q_OBJECT
  Q_PROPERTY(QString name READ name CONSTANT)
  Q_PROPERTY(QString type READ type CONSTANT)
  Q_PROPERTY(QString desc READ desc CONSTANT)
  Q_PROPERTY(int value READ value WRITE setValue NOTIFY valueNotify)
  Q_PROPERTY(int max READ max CONSTANT)
  Q_PROPERTY(int min READ min CONSTANT)

public:
  IntegerOptionItem(const FcitxQtConfigOption &option, const QString &path,
                    QObject *parent)
      : ConfigItem(path, parent),
        m_defaultValue(option.defaultValue().variant().toString().toInt()),
        m_name(option.name()), m_desc(option.description()) {
    if (option.properties().contains("IntMax")) {
      auto max = option.properties().value("IntMax");
      m_max = max.toString().toInt();
    }
    if (option.properties().contains("IntMin")) {
      auto min = option.properties().value("IntMin");
      m_min = min.toString().toInt();
    }
  }

  void readValueFrom(const QVariantMap &map) override {
    m_value = readVariant(map, path()).toString().toInt();
    qDebug() << "value:" << m_value << m_name;
  };

  void writeValueTo(QVariantMap &map) override {
    writeVariant(map, path(), QString::number(m_value));
  };

  void restoreToDefault() override { setValue(m_defaultValue); };

  QString name() const { return m_name; }
  QString type() const { return m_type; }
  QString desc() const { return m_desc; }
  int value() const { return m_value; }
  int max() const { return m_max; }
  int min() const { return m_min; }

  void setValue(int value) {
    qDebug() << "value:" << value << ", name:" << m_name;
    if (m_value == value) {
      return;
    }
    m_value = value;
    emit valueChanged();
  }

private:
  int m_value = 10;
  int m_max = 0;
  int m_min = 0;
  int m_defaultValue;
  QString m_name;
  QString m_desc;
  QString m_type = "Integer";

Q_SIGNALS:
  void valueNotify(int value);
};

class StringOptionItem : public ConfigItem {
  Q_OBJECT
  Q_PROPERTY(QString name READ name CONSTANT)
  Q_PROPERTY(QString type READ type CONSTANT)
  Q_PROPERTY(QString desc READ desc CONSTANT)
  Q_PROPERTY(QString value READ value WRITE setValue NOTIFY valueNotify)

public:
  StringOptionItem(const FcitxQtConfigOption &option, const QString &path,
                   QObject *parent)
      : ConfigItem(path, parent),
        m_defaultValue(option.defaultValue().variant().toString()),
        m_name(option.name()), m_desc(option.description()) {}

  void readValueFrom(const QVariantMap &map) override {
    m_value = readString(map, path());
  };

  void writeValueTo(QVariantMap &map) override {
    writeVariant(map, path(), m_value);
  };

  void restoreToDefault() override { setValue(m_defaultValue); };

  QString name() const { return m_name; }

  QString type() const { return m_type; }

  QString value() const { return m_value; }

  QString desc() const { return m_desc; }

  void setValue(QString value) {
    qDebug() << "value:" << value << ", name:" << m_name;
    if (m_value == value) {
      return;
    }
    m_value = value;
    emit valueChanged();
  }

private:
  QString m_value;
  QString m_defaultValue;
  QString m_name;
  QString m_desc;
  QString m_type = "String";

Q_SIGNALS:
  void valueNotify(bool value);
};

class KeyListOptionItem : public ConfigItem {
  Q_OBJECT
  Q_PROPERTY(QString name READ name CONSTANT)
  Q_PROPERTY(QString type READ type CONSTANT)
  Q_PROPERTY(QString desc READ desc CONSTANT)
  Q_PROPERTY(QString value READ value WRITE setValue NOTIFY valueNotify)
  Q_PROPERTY(bool allowModifierLess READ allowModifierLess CONSTANT)
  Q_PROPERTY(bool allowModifierOnly READ allowModifierOnly CONSTANT)

public:
  KeyListOptionItem(const FcitxQtConfigOption &option, const QString &path,
                    QObject *parent)
      : ConfigItem(path, parent), m_name(option.name()),
        m_desc(option.description()) {

    m_allowModifierLess =
        readString(option.properties(), "ListConstrain/AllowModifierLess") ==
        "True";

    m_allowModifierOnly =
        readString(option.properties(), "ListConstrain/AllowModifierOnly") ==
        "True";

    auto variant = option.defaultValue().variant();
    QVariantMap map;
    if (variant.canConvert<QDBusArgument>()) {
      auto argument = qvariant_cast<QDBusArgument>(variant);
      argument >> map;
    }
    m_defaultValue = readValue(map, "");
    if (m_defaultValue.isEmpty()) {
      m_value = "";
    } else {
      m_value = m_defaultValue.front();
    }
  }

  void readValueFrom(const QVariantMap &map) override {
    auto values = readValue(map, path());
    if (values.isEmpty()) {
      m_value = "";
    } else {
      m_value = values.front();
    }
  };

  void writeValueTo(QVariantMap &map) override {
    int i = 0;
    for (auto &key : m_defaultValue) {
      if (i == 0) {
        writeVariant(map, QString("%1/%2").arg(path()).arg(i), m_value);
      } else {
        writeVariant(map, QString("%1/%2").arg(path()).arg(i), key);
      }
      i++;
    }
    if (m_defaultValue.empty()) {
      if (!m_value.isEmpty()) {
        writeVariant(map, QString("%1/%2").arg(path()).arg(0), m_value);
      } else {
        writeVariant(map, path(), QVariantMap());
      }
    }
  };

  void restoreToDefault() override {
    if (m_defaultValue.isEmpty()) {
      setValue("");
    } else {
      setValue(m_defaultValue.front());
    }
  };

  QString name() const { return m_name; }

  QString type() const { return m_type; }

  QString value() const { return m_value; }

  QString desc() const { return m_desc; }

  bool allowModifierLess() const { return m_allowModifierLess; }

  bool allowModifierOnly() const { return m_allowModifierOnly; }

  void setValue(QString value) {
    if (m_value == value) {
      return;
    }
    m_value = value;
    emit valueChanged();
  }

private:
  QList<QString> readValue(const QVariantMap &map, const QString &path) {
    int i = 0;
    QList<QString> keys;
    while (true) {
      auto value = readString(
          map,
          QString("%1%2%3").arg(path).arg(path.isEmpty() ? "" : "/").arg(i));
      if (value.isNull()) {
        break;
      }
      keys << QString(value.toUtf8().constData());
      i++;
    }
    return keys;
  }

private:
  QString m_value;
  QString m_name;
  QString m_desc;
  QString m_type = "KeyList";

  bool m_allowModifierLess = false;
  bool m_allowModifierOnly = false;
  QList<QString> m_defaultValue;

Q_SIGNALS:
  void valueNotify(bool value);
};

class KeyOptionItem : public ConfigItem {
  Q_OBJECT
  Q_PROPERTY(QString name READ name CONSTANT)
  Q_PROPERTY(QString type READ type CONSTANT)
  Q_PROPERTY(QString desc READ desc CONSTANT)
  Q_PROPERTY(QString value READ value WRITE setValue)
  Q_PROPERTY(bool allowModifierLess READ allowModifierLess CONSTANT)
  Q_PROPERTY(bool allowModifierOnly READ allowModifierOnly CONSTANT)

public:
  explicit KeyOptionItem(const FcitxQtConfigOption &option, const QString &path,
                         QObject *parent)
      : ConfigItem(path, parent),
        m_defaultValue(
            option.defaultValue().variant().toString().toUtf8().constData()),
        m_name(option.name()), m_desc(option.description()) {
    m_allowModifierLess = readBool(option.properties(), "AllowModifierLess");
    m_allowModifierOnly = readBool(option.properties(), "AllowModifierOnly");
  }

  void readValueFrom(const QVariantMap &map) override {
    m_value = readString(map, path());
  };

  void writeValueTo(QVariantMap &map) override {
    writeVariant(map, path(), m_value);
  };

  void restoreToDefault() override { setValue(m_defaultValue); };

  QString name() const { return m_name; }

  QString type() const { return m_type; }

  QString value() const { return m_value; }

  QString desc() const { return m_desc; }

  bool allowModifierLess() const { return m_allowModifierLess; }

  bool allowModifierOnly() const { return m_allowModifierOnly; }

  void setValue(QString value) {
    qDebug() << "value:" << value << ", name:" << m_name;
    if (m_value != value) {
      m_value = value;
      emit valueChanged();
    }
  }

private:
  QString m_value = "";
  QString m_defaultValue;
  QString m_name;
  QString m_desc;
  QString m_type = "Key";

  bool m_allowModifierLess = false;
  bool m_allowModifierOnly = false;
};

class EnumOptionItem : public ConfigItem {
  Q_OBJECT
  Q_PROPERTY(QString name READ name CONSTANT)
  Q_PROPERTY(QString type READ type CONSTANT)
  Q_PROPERTY(QString desc READ desc CONSTANT)
  Q_PROPERTY(QList<QString> values READ values CONSTANT)
  Q_PROPERTY(QStringList texts READ texts CONSTANT)
  Q_PROPERTY(QList<QString> subConfigPaths READ subConfigPaths CONSTANT)
  Q_PROPERTY(int index READ index WRITE setIndex NOTIFY indexNotify)

public:
  EnumOptionItem(const FcitxQtConfigOption &option, const QString &path,
                 QObject *parent)
      : ConfigItem(path, parent), m_name(option.name()),
        m_desc(option.description()) {
    m_defaultValue = option.defaultValue().variant().toString();
    int i = 0;
    while (true) {
      auto value = readString(option.properties(), QString("Enum/%1").arg(i));
      if (value.isNull()) {
        break;
      }
      auto text =
          readString(option.properties(), QString("EnumI18n/%1").arg(i));
      if (text.isEmpty()) {
        text = value;
      }
      auto subConfigPath =
          readString(option.properties(), QString("SubConfigPath/%1").arg(i));

      m_values << value;
      m_texts << text;
      m_subConfigPaths << subConfigPath;
      i++;
    }
  }

private:
  void readValueFrom(const QVariantMap &map) override {
    auto value = readString(map, path());
    auto idx = m_values.indexOf(value);
    if (idx < 0) {
      idx = m_values.indexOf(m_defaultValue);
    }
    m_index = idx;
  };

  void writeValueTo(QVariantMap &map) override {
    writeVariant(map, path(), m_values[m_index]);
  };

  void restoreToDefault() override {
    auto idx = m_values.indexOf(m_defaultValue);
    setIndex(idx);
  };

  QString name() const { return m_name; }

  QString type() const { return m_type; }

  int index() const { return m_index; }

  QList<QString> values() const { return m_values; }

  QStringList texts() const { return m_texts; }

  QList<QString> subConfigPaths() const { return m_subConfigPaths; }

  QString desc() const { return m_desc; }

  void setIndex(int value) {
    qDebug() << "value:" << value << ", name:" << m_name;
    if (m_index == value) {
      return;
    }
    m_index = value;
    emit valueChanged();
  }

private:
  QString m_name;
  QString m_desc;
  QString m_type = "Enum";
  QString m_defaultValue;

  int m_index = -1;
  QList<QString> m_texts;
  QList<QString> m_values;
  QList<QString> m_subConfigPaths;

Q_SIGNALS:
  void indexNotify(int value);
};

class BooleanOptionItem : public ConfigItem {
  Q_OBJECT
  Q_PROPERTY(QString name READ name CONSTANT)
  Q_PROPERTY(QString type READ type CONSTANT)
  Q_PROPERTY(QString desc READ desc CONSTANT)
  Q_PROPERTY(bool value READ value WRITE setValue NOTIFY valueNotify)

public:
  BooleanOptionItem(const FcitxQtConfigOption &option, const QString &path,
                    QObject *parent)
      : ConfigItem(path, parent),
        m_defaultValue(option.defaultValue().variant().toString() == "True"),
        m_name(option.name()), m_desc(option.description()) {}

  void readValueFrom(const QVariantMap &map) override {
    m_value = readBool(map, path());
    qDebug() << "value:" << m_value << m_name;
  };

  void writeValueTo(QVariantMap &map) override {
    QString value = m_value ? "True" : "False";
    writeVariant(map, path(), value);
  };

  void restoreToDefault() override {
    qDebug() << "restoreToDefault, m_value:" << m_value << ", name:" << m_name
             << ", m_defaultValue:" << m_defaultValue;
    setValue(m_defaultValue);
  };

  QString name() const { return m_name; }

  QString type() const { return m_type; }

  bool value() const { return m_value; }

  QString desc() const { return m_desc; }

  void setValue(bool value) {
    qDebug() << "value:" << value << ", name:" << m_name;
    if (m_value == value) {
      return;
    }
    m_value = value;
    emit valueChanged();
  }

private:
  bool m_value = true;
  bool m_defaultValue;
  QString m_name;
  QString m_desc;
  QString m_type = "Boolean";

Q_SIGNALS:
  void valueNotify(bool value);
};

class ExternalOptionItem : public ConfigItem {
  Q_OBJECT
  Q_PROPERTY(QString name READ name CONSTANT)
  Q_PROPERTY(QString uri READ uri CONSTANT)
  Q_PROPERTY(QString type READ type CONSTANT)
  Q_PROPERTY(QString desc READ desc CONSTANT)
  Q_PROPERTY(bool launchSubConfig READ launchSubConfig CONSTANT)

public slots:
  void handleLaunchExtenalConfig(quint64 windowId = 0) {
    if (!m_launchSubConfig) {
      {
        WId wid = reinterpret_cast<WId>(windowId);
        QFileInfo pathToWrapper(FCITX5_QT_GUI_WRAPPER);
        QDir dirToWrapper = pathToWrapper.dir();
        if (m_uri.startsWith("fcitx://config/addon/")) {
          qDebug() << "handleLaunchExtenalConfig()"
                   << ",uri:" << m_uri;

          QLatin1String qt5Wrapper("fcitx5-qt5-gui-wrapper");
          QLatin1String qt6Wrapper("fcitx5-qt6-gui-wrapper");

          QString qt5WrapperPath = dirToWrapper.filePath(qt5Wrapper);
          QString qt6WrapperPath = dirToWrapper.filePath(qt6Wrapper);

          for (QString &wrapperPath :
               {std::ref(qt6WrapperPath), std::ref(qt5WrapperPath)}) {
            if (!QFileInfo(wrapperPath).isExecutable()) {
              QString fileName = QFileInfo(wrapperPath).fileName();

              QString newWrapperPath =
                  FCITX_INSTALL_LIBEXECDIR + QDir::separator() + fileName;
              wrapperPath = newWrapperPath;
            }
          }

          QString wrapperToUse;
          for (const QString &wrapperPath :
               {std::cref(qt6WrapperPath), std::cref(qt5WrapperPath)}) {
            QStringList args;
            args << QLatin1String("--test");
            args << m_uri;
            int exit_status = QProcess::execute(wrapperPath, args);
            if (exit_status == 0) {
              wrapperToUse = wrapperPath;
              break;
            }
          }
          if (wrapperToUse.isEmpty()) {
            qWarning() << "handleLaunchExtenalConfig()"
                       << ",wrapperToUse is empty";
            return;
          }

          QStringList args;
          if (wid) {
            args << "-w";
            args << QString::number(wid);
          }
          args << m_uri;
          qDebug() << "handleLaunchExtenalConfig(), args:" << args
                   << "，wrapperToUse:" << wrapperToUse;

          QProcess::startDetached(wrapperToUse, args);
        } else {
          qWarning() << "handleLaunchExtenalConfig() m_uri:" << m_uri
                     << ",not a addon, will launch it directly.";
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
          QStringList args = QProcess::splitCommand(m_uri);
          QString program = args.takeFirst();
          QProcess::startDetached(program, args);
#else
          QProcess::startDetached(uri);
#endif
        }
      }
    } else {
      qWarning() << "handleLaunchExtenalConfig() m_launchSubConfig:"
                 << m_launchSubConfig << ",will not launch external config.";
    }
  }

public:
  ExternalOptionItem(const FcitxQtConfigOption &option, const QString &path,
                     QObject *parent)
      : ConfigItem(path, parent),
        m_uri(readString(option.properties(), "External")),
        m_name(option.name()), m_desc(option.description()),
        m_launchSubConfig(readBool(option.properties(), "LaunchSubConfig")) {}

  void readValueFrom(const QVariantMap &map) override {}

  void writeValueTo(QVariantMap &map) override {}

  void restoreToDefault() override {};

  QString uri() const { return m_uri; }

  QString name() const { return m_name; }

  QString type() const { return m_type; }

  QString desc() const { return m_desc; }

  bool launchSubConfig() const { return m_launchSubConfig; }

private:
  QString m_uri;
  QString m_name;
  QString m_desc;
  QString m_type = "External";
  bool m_launchSubConfig;
  const QString FCITX5_QT_GUI_WRAPPER = "/usr/libexec/fcitx5-qt5-gui-wrapper";
  const QString FCITX_INSTALL_LIBEXECDIR = "/usr/local/libexec";

Q_SIGNALS:
  void valueNotify(bool value);
};

std::unique_ptr<ConfigItem>
ConfigItem::create(const FcitxQtConfigOption &option, const QString &path,
                   QObject *parent) {

  std::unique_ptr<ConfigItem> result = nullptr;
  if (option.type() == "Integer") {
    auto item = std::make_unique<IntegerOptionItem>(option, path, parent);
    result = std::move(item);
  } else if (option.type() == "String") {
    const auto isFont = readBool(option.properties(), "Font");
    const auto isEnum = readBool(option.properties(), "IsEnum");
    if (isFont) {
      // TODO: Font Option
    } else if (isEnum) {
      auto item = std::make_unique<EnumOptionItem>(option, path, parent);
      result = std::move(item);
    } else {
      auto item = std::make_unique<StringOptionItem>(option, path, parent);
      result = std::move(item);
    }
  } else if (option.type() == "Boolean") {
    auto item = std::make_unique<BooleanOptionItem>(option, path, parent);
    result = std::move(item);
  } else if (option.type() == "List|Key") {
    auto item = std::make_unique<KeyListOptionItem>(option, path, parent);
    result = std::move(item);
  } else if (option.type() == "Integer") {
    auto item = std::make_unique<IntegerOptionItem>(option, path, parent);
    result = std::move(item);
  } else if (option.type() == "Enum") {
    auto item = std::make_unique<EnumOptionItem>(option, path, parent);
    result = std::move(item);
  } else if (option.type() == "Key") {
    auto item = std::make_unique<KeyOptionItem>(option, path, parent);
    result = std::move(item);
  } else if (option.type() == "External") {
    auto item = std::make_unique<ExternalOptionItem>(option, path, parent);
    result = std::move(item);
  } else if (option.type().startsWith("List|")) {
    auto item = std::make_unique<ListOptionItem>(option, path, parent);
    result = std::move(item);
  }
  return result;
}
#include "configitem.moc"
