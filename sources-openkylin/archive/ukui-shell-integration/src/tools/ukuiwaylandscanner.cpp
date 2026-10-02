// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <vector>

#include <QCoreApplication>
#include <QFile>
#include <QXmlStreamReader>

class Scanner
{
public:
    explicit Scanner()
    {
    }
    ~Scanner()
    {
        delete m_xml;
    }

    bool parseArguments(int argc, char **argv);
    void printUsage();
    bool process();
    void printErrors();

private:
    struct WaylandEnumEntry
    {
        QByteArray name;
        QByteArray value;
        QByteArray summary;
    };

    struct WaylandEnum
    {
        QByteArray name;

        std::vector<WaylandEnumEntry> entries;
    };

    struct WaylandArgument
    {
        QByteArray name;
        QByteArray type;
        QByteArray interface;
        QByteArray summary;
        bool allowNull;
    };

    struct WaylandEvent
    {
        bool request;
        QByteArray name;
        QByteArray type;
        std::vector<WaylandArgument> arguments;
    };

    struct WaylandInterface
    {
        QByteArray name;
        int version;

        std::vector<WaylandEnum> enums;
        std::vector<WaylandEvent> events;
        std::vector<WaylandEvent> requests;
    };

    bool parseOption(const QByteArray &str);

    QByteArray byteArrayValue(const QXmlStreamReader &xml, const char *name);
    int intValue(const QXmlStreamReader &xml, const char *name, int defaultValue = 0);
    bool boolValue(const QXmlStreamReader &xml, const char *name);
    WaylandEvent readEvent(QXmlStreamReader &xml, bool request);
    Scanner::WaylandEnum readEnum(QXmlStreamReader &xml);
    Scanner::WaylandInterface readInterface(QXmlStreamReader &xml);
    QByteArray waylandToCType(const QByteArray &waylandType, const QByteArray &interface);
    QByteArray waylandToQtType(const QByteArray &waylandType, const QByteArray &interface, bool cStyleArray);
    const Scanner::WaylandArgument *newIdArgument(const std::vector<WaylandArgument> &arguments);

    void printEvent(const WaylandEvent &e, bool omitNames = false, bool withResource = false);
    void printEventHandlerSignature(const WaylandEvent &e, const char *interfaceName);
    void printEnums(const std::vector<WaylandEnum> &enums);

    QByteArray stripInterfaceName(const QByteArray &name);
    bool ignoreInterface(const QByteArray &name);

    enum Option {
        ClientHeader,
        ClientCode,
    } m_option;

    QByteArray m_protocolName;
    QByteArray m_protocolFilePath;
    QByteArray m_scannerName;
    QByteArray m_headerPath;
    QByteArray m_prefix;
    QVector<QByteArray> m_includes;
    QXmlStreamReader *m_xml = nullptr;
};

bool Scanner::parseArguments(int argc, char **argv)
{
    QVector<QByteArray> args;
    args.reserve(argc);
    for (int i = 0; i < argc; ++i)
        args << QByteArray(argv[i]);

    m_scannerName = args[0];

    if (argc <= 2 || !parseOption(args[1]))
        return false;

    m_protocolFilePath = args[2];

    if (argc > 3 && !args[3].startsWith('-')) {
        // legacy positional arguments
        m_headerPath = args[3];
        if (argc == 5)
            m_prefix = args[4];
    } else {
        // --header-path=<path> (14 characters)
        // --prefix=<prefix> (9 characters)
        // --add-include=<include> (14 characters)
        for (int pos = 3; pos < argc; pos++) {
            const QByteArray &option = args[pos];
            if (option.startsWith("--header-path=")) {
                m_headerPath = option.mid(14);
            } else if (option.startsWith("--prefix=")) {
                m_prefix = option.mid(10);
            } else if (option.startsWith("--add-include=")) {
                auto include = option.mid(14);
                if (!include.isEmpty())
                    m_includes << include;
            } else {
                return false;
            }
        }
    }

    return true;
}

void Scanner::printUsage()
{
    fprintf(stderr, "Usage: %s [client-header|client-code] specfile [--header-path=<path>] [--prefix=<prefix>] [--add-include=<include>]\n", m_scannerName.constData());
}

bool Scanner::parseOption(const QByteArray &str)
{
    if (str == "client-header")
        m_option = ClientHeader;
    else if (str == "client-code")
        m_option = ClientCode;
    else
        return false;

    return true;
}

QByteArray Scanner::byteArrayValue(const QXmlStreamReader &xml, const char *name)
{
    if (xml.attributes().hasAttribute(name))
        return xml.attributes().value(name).toUtf8();
    return QByteArray();
}

int Scanner::intValue(const QXmlStreamReader &xml, const char *name, int defaultValue)
{
    bool ok;
    int result = byteArrayValue(xml, name).toInt(&ok);
    return ok ? result : defaultValue;
}

bool Scanner::boolValue(const QXmlStreamReader &xml, const char *name)
{
    return byteArrayValue(xml, name) == "true";
}

Scanner::WaylandEvent Scanner::readEvent(QXmlStreamReader &xml, bool request)
{
    WaylandEvent event = {
        .request = request,
        .name = byteArrayValue(xml, "name"),
        .type = byteArrayValue(xml, "type"),
        .arguments = {},
    };
    while (xml.readNextStartElement()) {
        if (xml.name() == QLatin1String("arg")) {
            WaylandArgument argument = {
                .name = byteArrayValue(xml, "name"),
                .type = byteArrayValue(xml, "type"),
                .interface = byteArrayValue(xml, "interface"),
                .summary = byteArrayValue(xml, "summary"),
                .allowNull = boolValue(xml, "allowNull"),
            };
            event.arguments.push_back(std::move(argument));
        }

        xml.skipCurrentElement();
    }
    return event;
}

Scanner::WaylandEnum Scanner::readEnum(QXmlStreamReader &xml)
{
    WaylandEnum result = {
        .name = byteArrayValue(xml, "name"),
        .entries = {},
    };

    while (xml.readNextStartElement()) {
        if (xml.name() == QLatin1String("entry")) {
            WaylandEnumEntry entry = {
                .name = byteArrayValue(xml, "name"),
                .value = byteArrayValue(xml, "value"),
                .summary = byteArrayValue(xml, "summary"),
            };
            result.entries.push_back(std::move(entry));
        }

        xml.skipCurrentElement();
    }

    return result;
}

Scanner::WaylandInterface Scanner::readInterface(QXmlStreamReader &xml)
{
    WaylandInterface interface = {
        .name = byteArrayValue(xml, "name"),
        .version = intValue(xml, "version", 1),
        .enums = {},
        .events = {},
        .requests = {},
    };

    while (xml.readNextStartElement()) {
        if (xml.name() == QLatin1String("event"))
            interface.events.push_back(readEvent(xml, false));
        else if (xml.name() == QLatin1String("request"))
            interface.requests.push_back(readEvent(xml, true));
        else if (xml.name() == QLatin1String("enum"))
            interface.enums.push_back(readEnum(xml));
        else
            xml.skipCurrentElement();
    }

    return interface;
}

QByteArray Scanner::waylandToCType(const QByteArray &waylandType, const QByteArray &interface)
{
    if (waylandType == "string")
        return "const char *";
    else if (waylandType == "int")
        return "int32_t";
    else if (waylandType == "uint")
        return "uint32_t";
    else if (waylandType == "fixed")
        return "wl_fixed_t";
    else if (waylandType == "fd")
        return "int32_t";
    else if (waylandType == "array")
        return "wl_array *";
    else if (waylandType == "object" || waylandType == "new_id") {
        if (interface.isEmpty())
            return "struct ::wl_object *";
        return "struct ::" + interface + " *";
    }
    return waylandType;
}

QByteArray Scanner::waylandToQtType(const QByteArray &waylandType, const QByteArray &interface, bool cStyleArray)
{
    if (waylandType == "string")
        return "const QString &";
    else if (waylandType == "array")
        return cStyleArray ? "wl_array *" : "const QByteArray &";
    else
        return waylandToCType(waylandType, interface);
}

const Scanner::WaylandArgument *Scanner::newIdArgument(const std::vector<WaylandArgument> &arguments)
{
    for (const WaylandArgument &a : arguments) {
        if (a.type == "new_id")
            return &a;
    }
    return nullptr;
}

void Scanner::printEvent(const WaylandEvent &e, bool omitNames, bool withResource)
{
    printf("%s(", e.name.constData());
    bool needsComma = false;
    for (const WaylandArgument &a : e.arguments) {
        bool isNewId = a.type == "new_id";
        if (isNewId && (a.interface.isEmpty() != e.request))
            continue;
        if (needsComma)
            printf(", ");
        needsComma = true;
        if (isNewId && e.request) {
            printf("const struct ::wl_interface *%s, uint32_t%s", omitNames ? "" : "interface", omitNames ? "" : " version");
            continue;
        }

        QByteArray qtType = waylandToQtType(a.type, a.interface, e.request == false);
        printf("%s%s%s", qtType.constData(), qtType.endsWith("&") || qtType.endsWith("*") ? "" : " ", a.name.constData());
    }
    printf(")");
}

void Scanner::printEventHandlerSignature(const WaylandEvent &e, const char *interfaceName)
{
    printf("handle_%s(void *data, ", e.name.constData());
    printf("struct ::%s *object", interfaceName);
    for (const WaylandArgument &a : e.arguments) {
        printf(", ");
        bool isNewId = a.type == "new_id";
        QByteArray cType = waylandToCType(a.type, a.interface);
        printf("%s%s%s", cType.constData(), cType.endsWith("*") ? "" : " ", a.name.constData());
    }
    printf(")");
}

void Scanner::printEnums(const std::vector<WaylandEnum> &enums)
{
    for (const WaylandEnum &e : enums) {
        printf("\n");
        printf("    enum %s {\n", e.name.constData());
        for (const WaylandEnumEntry &entry : e.entries) {
            printf("        %s_%s = %s,", e.name.constData(), entry.name.constData(), entry.value.constData());
            if (!entry.summary.isNull())
                printf(" // %s", entry.summary.constData());
            printf("\n");
        }
        printf("    };\n");
    }
}

QByteArray Scanner::stripInterfaceName(const QByteArray &name)
{
    if (!m_prefix.isEmpty() && name.startsWith(m_prefix))
        return name.mid(m_prefix.size());
    if (name.startsWith("qt_") || name.startsWith("wl_"))
        return name.mid(3);

    return name;
}

bool Scanner::ignoreInterface(const QByteArray &name)
{
    return name == "wl_display";
}

bool Scanner::process()
{
    QFile file(m_protocolFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        fprintf(stderr, "Unable to open file %s\n", m_protocolFilePath.constData());
        return false;
    }

    m_xml = new QXmlStreamReader(&file);
    if (!m_xml->readNextStartElement())
        return false;

    if (m_xml->name() != QLatin1String("protocol")) {
        m_xml->raiseError(QStringLiteral("The file is not a wayland protocol file."));
        return false;
    }

    m_protocolName = byteArrayValue(*m_xml, "name");

    if (m_protocolName.isEmpty()) {
        m_xml->raiseError(QStringLiteral("Missing protocol name."));
        return false;
    }

    // We should convert - to _ so that the preprocessor wont generate code which will lead to unexpected behavior
    // However, the wayland-scanner doesn't do so we will do the same for now
    QByteArray preProcessorProtocolName = QByteArray(m_protocolName).toUpper();

    std::vector<WaylandInterface> interfaces;

    while (m_xml->readNextStartElement()) {
        if (m_xml->name() == QLatin1String("interface"))
            interfaces.push_back(readInterface(*m_xml));
        else
            m_xml->skipCurrentElement();
    }

    if (m_xml->hasError())
        return false;

    printf("// This file was generated by ukuiwaylandscanner\n");
    printf("// source file is %s\n\n", qPrintable(m_protocolFilePath));

    for (auto b : qAsConst(m_includes))
        printf("#include %s\n", b.constData());

    if (m_option == ClientHeader) {
        printf("#pragma once\n");
        printf("\n");
        if (m_headerPath.isEmpty())
            printf("#include \"wayland-%s-client-protocol.h\"\n", QByteArray(m_protocolName).replace('_', '-').constData());
        else
            printf("#include <%s/wayland-%s-client-protocol.h>\n", m_headerPath.constData(), QByteArray(m_protocolName).replace('_', '-').constData());
        printf("#include <QByteArray>\n");
        printf("#include <QString>\n");
        printf("\n");
        printf("struct wl_registry;\n");
        printf("\n");
        printf("QT_WARNING_PUSH\n");
        printf("QT_WARNING_DISABLE_GCC(\"-Wmissing-field-initializers\")\n");

        printf("\n");
        printf("namespace UKUI\n{\n");

        bool needsNewLine = false;
        for (const WaylandInterface &interface : interfaces) {
            if (ignoreInterface(interface.name))
                continue;

            if (needsNewLine)
                printf("\n");
            needsNewLine = true;

            const char *interfaceName = interface.name.constData();

            QByteArray stripped = stripInterfaceName(interface.name);
            const char *interfaceNameStripped = stripped.constData();

            printf("class %s\n{\n", interfaceName);
            printf("public:\n");
            printf("    %s(struct ::wl_registry *registry, int id, int version);\n", interfaceName);
            printf("    %s(struct ::%s *object);\n", interfaceName, interfaceName);
            printf("    %s();\n", interfaceName);
            printf("\n");
            printf("    virtual ~%s();\n", interfaceName);
            printf("\n");
            printf("    void init(struct ::wl_registry *registry, int id, int version);\n");
            printf("    void init(struct ::%s *object);\n", interfaceName);
            printf("\n");
            printf("    struct ::%s *object()\n    {\n        return m_%s;\n    }\n", interfaceName, interfaceName);
            printf("    const struct ::%s *object() const\n    {\n        return m_%s;\n    }\n", interfaceName, interfaceName);
            printf("    static %s *fromObject(struct ::%s *object);\n", interfaceName, interfaceName);
            printf("\n");
            printf("    bool isInitialized() const;\n");
            printf("\n");
            printf("    static const struct ::wl_interface *interface();\n");

            printEnums(interface.enums);

            if (!interface.requests.empty()) {
                printf("\n");
                for (const WaylandEvent &e : interface.requests) {
                    const WaylandArgument *new_id = newIdArgument(e.arguments);
                    QByteArray new_id_str = "void ";
                    if (new_id) {
                        if (new_id->interface.isEmpty())
                            new_id_str = "void *";
                        else
                            new_id_str = "struct ::" + new_id->interface + " *";
                    }
                    printf("    %s", new_id_str.constData());
                    printEvent(e);
                    printf(";\n");
                }
            }

            bool hasEvents = !interface.events.empty();

            if (hasEvents) {
                printf("\n");
                printf("protected:\n");
                for (const WaylandEvent &e : interface.events) {
                    printf("    virtual void %s_", interfaceNameStripped);
                    printEvent(e);
                    printf(";\n");
                }
            }

            printf("\n");
            printf("private:\n");
            if (hasEvents) {
                printf("    void init_listener();\n");
                printf("    static const struct %s_listener m_%s_listener;\n", interfaceName, interfaceName);
                for (const WaylandEvent &e : interface.events) {
                    printf("    static void ");

                    printEventHandlerSignature(e, interfaceName);
                    printf(";\n");
                }
            }
            printf("    struct ::%s *m_%s;\n", interfaceName, interfaceName);
            printf("};\n");
        }
        printf("}\n");
        printf("\n");
        printf("QT_WARNING_POP\n");
    }

    if (m_option == ClientCode) {
        if (m_headerPath.isEmpty())
            printf("#include \"ukui-%s.h\"\n", QByteArray(m_protocolName).replace('_', '-').constData());
        else
            printf("#include <%s/ukui-%s.h>\n", m_headerPath.constData(), QByteArray(m_protocolName).replace('_', '-').constData());
        printf("\n");
        printf("QT_WARNING_PUSH\n");
        printf("QT_WARNING_DISABLE_GCC(\"-Wmissing-field-initializers\")\n");
        printf("\n");
        printf("namespace UKUI\n{\n");
        printf("\n");

        // wl_registry_bind is part of the protocol, so we can't use that... instead we use core
        // libwayland API to do the same thing a wayland-scanner generated wl_registry_bind would.
        printf("static inline void *wlRegistryBind(struct ::wl_registry *registry, uint32_t name, const struct ::wl_interface *interface, uint32_t version)\n");
        printf("{\n");
        printf("    const uint32_t bindOpCode = 0;\n");
        printf("#if (WAYLAND_VERSION_MAJOR == 1 && WAYLAND_VERSION_MINOR > 10) || WAYLAND_VERSION_MAJOR > 1\n");
        printf("    return (void *) wl_proxy_marshal_constructor_versioned((struct wl_proxy *) registry,\n");
        printf("        bindOpCode, interface, version, name, interface->name, version, nullptr);\n");
        printf("#else\n");
        printf("    return (void *) wl_proxy_marshal_constructor((struct wl_proxy *) registry,\n");
        printf("        bindOpCode, interface, name, interface->name, version, nullptr);\n");
        printf("#endif\n");
        printf("}\n");
        printf("\n");

        bool needsNewLine = false;
        for (const WaylandInterface &interface : interfaces) {

            if (ignoreInterface(interface.name))
                continue;

            if (needsNewLine)
                printf("\n");
            needsNewLine = true;

            const char *interfaceName = interface.name.constData();

            QByteArray stripped = stripInterfaceName(interface.name);
            const char *interfaceNameStripped = stripped.constData();

            bool hasEvents = !interface.events.empty();

            printf("%s::%s(struct ::wl_registry *registry, int id, int version)\n", interfaceName, interfaceName);
            printf("{\n");
            printf("    init(registry, id, version);\n");
            printf("}\n");
            printf("\n");

            printf("%s::%s(struct ::%s *obj)\n", interfaceName, interfaceName, interfaceName);
            printf("    : m_%s(obj)\n", interfaceName);
            printf("{\n");
            if (hasEvents)
                printf("    init_listener();\n");
            printf("}\n");
            printf("\n");

            printf("%s::%s()\n", interfaceName, interfaceName);
            printf("    : m_%s(nullptr)\n", interfaceName);
            printf("{\n");
            printf("}\n");
            printf("\n");

            printf("%s::~%s()\n", interfaceName, interfaceName);
            printf("{\n");
            printf("}\n");
            printf("\n");

            printf("void %s::init(struct ::wl_registry *registry, int id, int version)\n", interfaceName);
            printf("{\n");
            printf("    m_%s = static_cast<struct ::%s *>(wlRegistryBind(registry, id, &%s_interface, version));\n", interfaceName, interfaceName, interfaceName);
            if (hasEvents)
                printf("    init_listener();\n");
            printf("}\n");
            printf("\n");

            printf("void %s::init(struct ::%s *obj)\n", interfaceName, interfaceName);
            printf("{\n");
            printf("    m_%s = obj;\n", interfaceName);
            if (hasEvents)
                printf("    init_listener();\n");
            printf("}\n");
            printf("\n");

            printf("%s *%s::fromObject(struct ::%s *object)\n", interfaceName, interfaceName, interfaceName);
            printf("{\n");
            if (hasEvents) {
                printf("    if (wl_proxy_get_listener((struct ::wl_proxy *)object) != (void *)&m_%s_listener)\n", interfaceName);
                printf("        return nullptr;\n");
            }
            printf("    return static_cast<%s *>(%s_get_user_data(object));\n", interfaceName, interfaceName);
            printf("}\n");
            printf("\n");

            printf("bool %s::isInitialized() const\n", interfaceName);
            printf("{\n");
            printf("    return m_%s != nullptr;\n", interfaceName);
            printf("}\n");
            printf("\n");

            printf("const struct wl_interface *%s::interface()\n", interfaceName);
            printf("{\n");
            printf("    return &::%s_interface;\n", interfaceName);
            printf("}\n");

            for (const WaylandEvent &e : interface.requests) {
                printf("\n");
                const WaylandArgument *new_id = newIdArgument(e.arguments);
                QByteArray new_id_str = "void ";
                if (new_id) {
                    if (new_id->interface.isEmpty())
                        new_id_str = "void *";
                    else
                        new_id_str = "struct ::" + new_id->interface + " *";
                }
                printf("%s%s::", new_id_str.constData(), interfaceName);
                printEvent(e);
                printf("\n");
                printf("{\n");
                for (const WaylandArgument &a : e.arguments) {
                    if (a.type != "array")
                        continue;
                    QByteArray array = a.name + "_data";
                    const char *arrayName = array.constData();
                    const char *variableName = a.name.constData();
                    printf("    struct wl_array %s;\n", arrayName);
                    printf("    %s.size = %s.size();\n", arrayName, variableName);
                    printf("    %s.data = static_cast<void *>(const_cast<char *>(%s.constData()));\n", arrayName, variableName);
                    printf("    %s.alloc = 0;\n", arrayName);
                    printf("\n");
                }
                int actualArgumentCount = new_id ? int(e.arguments.size()) - 1 : int(e.arguments.size());
                printf("    %s%s_%s(", new_id ? "return " : "", interfaceName, e.name.constData());
                printf("m_%s%s", interfaceName, actualArgumentCount > 0 ? ", " : "");
                bool needsComma = false;
                for (const WaylandArgument &a : e.arguments) {
                    bool isNewId = a.type == "new_id";
                    if (isNewId && !a.interface.isEmpty())
                        continue;
                    if (needsComma)
                        printf(", ");
                    needsComma = true;
                    if (isNewId) {
                        printf("interface, version");
                    } else {
                        QByteArray cType = waylandToCType(a.type, a.interface);
                        QByteArray qtType = waylandToQtType(a.type, a.interface, e.request);
                        if (a.type == "string")
                            printf("%s.toUtf8().constData()", a.name.constData());
                        else if (a.type == "array")
                            printf("&%s_data", a.name.constData());
                        else if (cType == qtType)
                            printf("%s", a.name.constData());
                    }
                }
                printf(");\n");
                if (e.type == "destructor")
                    printf("    m_%s = nullptr;\n", interfaceName);
                printf("}\n");
            }

            if (hasEvents) {
                printf("\n");
                for (const WaylandEvent &e : interface.events) {
                    printf("void %s::%s_", interfaceName, interfaceNameStripped);
                    printEvent(e, true);
                    printf("\n");
                    printf("{\n");
                    printf("}\n");
                    printf("\n");
                    printf("void %s::", interfaceName);
                    printEventHandlerSignature(e, interfaceName);
                    printf("\n");
                    printf("{\n");
                    printf("    static_cast<%s *>(data)->%s_%s(", interfaceName, interfaceNameStripped, e.name.constData());
                    bool needsComma = false;
                    for (const WaylandArgument &a : e.arguments) {
                        if (needsComma)
                            printf(", ");
                        needsComma = true;
                        const char *argumentName = a.name.constData();
                        if (a.type == "string")
                            printf("QString::fromUtf8(%s)", argumentName);
                        else
                            printf("%s", argumentName);
                    }
                    printf(");\n");

                    printf("}\n");
                    printf("\n");
                }
                printf("const struct %s_listener %s::m_%s_listener = {\n", interfaceName, interfaceName, interfaceName);
                for (const WaylandEvent &e : interface.events) {
                    printf("    %s::handle_%s,\n", interfaceName, e.name.constData());
                }
                printf("};\n");
                printf("\n");

                printf("void %s::init_listener()\n", interfaceName);
                printf("{\n");
                printf("    %s_add_listener(m_%s, &m_%s_listener, this);\n", interfaceName, interfaceName, interfaceName);
                printf("}\n");
            }
        }
        printf("}\n");
        printf("\n");
        printf("QT_WARNING_POP\n");
    }

    return true;
}

void Scanner::printErrors()
{
    if (m_xml->hasError())
        fprintf(stderr, "XML error: %s\nLine %lld, column %lld\n", m_xml->errorString().toLocal8Bit().constData(), m_xml->lineNumber(), m_xml->columnNumber());
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    Scanner scanner;

    if (!scanner.parseArguments(argc, argv)) {
        scanner.printUsage();
        return EXIT_FAILURE;
    }

    if (!scanner.process()) {
        scanner.printErrors();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
