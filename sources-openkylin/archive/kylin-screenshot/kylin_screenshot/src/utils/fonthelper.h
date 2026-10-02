#ifndef FONTHELPER_H
#define FONTHELPER_H

#include <QFontDatabase>
#include <QStringList>
#include <QDir>
#include <QProcess>

// 扫描用户字体目录，返回用户安装的字体族名（fc-list 失败返回空，白名单兜底）
static inline QStringList userFontFamilies()
{
    QStringList result;
    QProcess fc;
    fc.start(QStringLiteral("fc-list"));
    if (!fc.waitForFinished(3000))
        return result;
    const QString output = QString::fromUtf8(fc.readAllStandardOutput());
    const QString home = QDir::homePath();
    const QStringList userDirs = {
        home + QLatin1String("/.fonts"),
        home + QLatin1String("/.local/share/fonts"),
    };
    const QStringList lines = output.split(QLatin1Char('\n'));
    for (const QString &line : lines) {
        if (line.trimmed().isEmpty())
            continue;
        bool isUser = false;
        for (const QString &d : userDirs) {
            if (line.startsWith(d)) {
                isUser = true;
                break;
            }
        }
        if (!isUser)
            continue;
        int colon = line.indexOf(QLatin1Char(':'));
        if (colon < 0)
            continue;
        // 族名可能带逗号别名，取第一个；再去掉 :style= 后缀
        QString family = line.mid(colon + 1).section(QLatin1Char(','), 0, 0).trimmed();
        int stylePos = family.indexOf(QLatin1String(":style="));
        if (stylePos >= 0)
            family = family.left(stylePos).trimmed();
        if (!family.isEmpty())
            result.append(family);
    }
    result.removeDuplicates();
    return result;
}

// 系统自带精选字体：白名单（存在才加入）
static inline QStringList systemWhiteListFamilies(QFontDatabase &db)
{
    QStringList result;
    static const QStringList whiteList = {
        // 中文
        "Noto Sans CJK SC", "Noto Serif CJK SC",
        "文泉驿正黑", "文泉驿微米黑",
        "黑体", "SimHei", "宋体", "SimSun",
        "华文黑体", "华文宋体", "华文楷体", "华文仿宋",
        "国标黑体", "国标宋体", "国标楷体", "国标仿宋",
        // 用户常用中文（LXGW 系列兜底，fc-list 失效时亦有）
        "霞鹜文楷等宽 GB", "霞鹜漫黑", "霞鹜文楷",
        "小赖字体等宽", "平方洒脱体",
        // 英文（常见者 + 等宽）
        "DejaVu Sans", "Liberation Sans",
        "DejaVu Serif", "Liberation Serif",
        "DejaVu Sans Mono", "Cascadia Code",
    };
    const QStringList all = db.families();
    for (const QString &w : whiteList) {
        if (all.contains(w))
            result.append(w);
    }
    return result;
}

// 最终字体列表：系统精选白名单 + 用户字体（仅保留支持中文的用户字体，避免英文泛滥）
static inline QStringList filteredFontFamilies(QFontDatabase &db)
{
    QStringList result = systemWhiteListFamilies(db);
    const QStringList user = userFontFamilies();
    for (const QString &f : user) {
        if (result.contains(f))
            continue;
        // 用户字体仅保留支持中文者（Cascadia/Inconsolata 等纯英文用户字体不入列）
        const QList<QFontDatabase::WritingSystem> ws = db.writingSystems(f);
        if (ws.contains(QFontDatabase::SimplifiedChinese)
            || ws.contains(QFontDatabase::TraditionalChinese))
            result.append(f);
    }
    return result;
}

#endif // FONTHELPER_H