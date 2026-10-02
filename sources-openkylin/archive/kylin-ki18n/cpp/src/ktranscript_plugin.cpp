// SPDX-FileCopyrightText: 2026 KI18n Rust Backend
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "KLocalizedString/ktranscript_export.h"
#include "KLocalizedString/ktranscript_p.h"

#include <QByteArray>
#include <QGlobalStatic>
#include <QHash>
#include <QStringList>
#include <QVariant>
#include <QVector>

extern "C" {

void ki18n_rs_string_free(char *ptr);

int ki18n_rs_ktranscript_eval(const char *const *argv,
                              size_t argc,
                              const char *lang,
                              const char *ctry,
                              const char *msgctxt,
                              const char *const *dyn_keys,
                              const char *const *dyn_vals,
                              size_t dyn_count,
                              const char *msgid,
                              const char *const *subs,
                              size_t subs_count,
                              const char *const *vals,
                              size_t vals_count,
                              const char *msgstrf,
                              const char *const *mod_paths,
                              const char *const *mod_langs,
                              size_t mod_count,
                              char **out_result,
                              char **out_error,
                              int *out_fallback);
}

namespace {

QString variantToTranscriptArg(const QVariant &v)
{
    switch (v.userType()) {
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::LongLong:
    case QVariant::ULongLong:
        return QString::number(v.toLongLong());
    case QVariant::Double:
        return QString::number(v.toDouble(), 'g', 18);
    case QVariant::Bool:
        return v.toBool() ? QStringLiteral("true") : QStringLiteral("false");
    default:
        break;
    }
    if (v.canConvert<QString>()) {
        return v.toString();
    }
    return v.toString();
}

} // namespace

class KTranscriptRust final : public KTranscript
{
public:
    QString eval(const QList<QVariant> &argv,
                 const QString &lang,
                 const QString &ctry,
                 const QString &msgctxt,
                 const QHash<QString, QString> &dynctxt,
                 const QString &msgid,
                 const QStringList &subs,
                 const QList<QVariant> &vals,
                 const QString &ftrans,
                 QList<QStringList> &mods,
                 QString &error,
                 bool &fallback) override
    {
        error.clear();
        fallback = false;

        QVector<QByteArray> argvUtf8;
        QVector<const char *> argvPtrs;
        argvUtf8.reserve(argv.size());
        argvPtrs.reserve(argv.size());
        for (const QVariant &a : argv) {
            const QString s = variantToTranscriptArg(a);
            argvUtf8.push_back(s.toUtf8());
            argvPtrs.push_back(argvUtf8.last().constData());
        }

        const QByteArray langB = lang.toUtf8();
        const QByteArray ctryB = ctry.toUtf8();
        const QByteArray msgctxtB = msgctxt.toUtf8();
        const QByteArray msgidB = msgid.toUtf8();
        const QByteArray ftransB = ftrans.toUtf8();

        QVector<QByteArray> subsUtf8;
        QVector<const char *> subsPtrs;
        for (const QString &s : subs) {
            subsUtf8.push_back(s.toUtf8());
            subsPtrs.push_back(subsUtf8.last().constData());
        }

        QVector<QByteArray> valsUtf8;
        QVector<const char *> valsPtrs;
        for (const QVariant &v : vals) {
            const QString s = variantToTranscriptArg(v);
            valsUtf8.push_back(s.toUtf8());
            valsPtrs.push_back(valsUtf8.last().constData());
        }

        QVector<QByteArray> dynKeyUtf8;
        QVector<QByteArray> dynValUtf8;
        QVector<const char *> dynKeyPtrs;
        QVector<const char *> dynValPtrs;
        dynKeyUtf8.reserve(dynctxt.size());
        dynValUtf8.reserve(dynctxt.size());
        for (auto it = dynctxt.constBegin(); it != dynctxt.constEnd(); ++it) {
            dynKeyUtf8.push_back(it.key().toUtf8());
            dynValUtf8.push_back(it.value().toUtf8());
            dynKeyPtrs.push_back(dynKeyUtf8.last().constData());
            dynValPtrs.push_back(dynValUtf8.last().constData());
        }

        QVector<QByteArray> modPathUtf8;
        QVector<QByteArray> modLangUtf8;
        QVector<const char *> modPathPtrs;
        QVector<const char *> modLangPtrs;
        for (const QStringList &mod : mods) {
            if (mod.size() < 2) {
                continue;
            }
            modPathUtf8.push_back(mod.at(0).toUtf8());
            modLangUtf8.push_back(mod.at(1).toUtf8());
            modPathPtrs.push_back(modPathUtf8.last().constData());
            modLangPtrs.push_back(modLangUtf8.last().constData());
        }
        const size_t mod_count = size_t(modPathPtrs.size());

        char *out_result = nullptr;
        char *out_err = nullptr;
        int out_fb = 0;

        const int rc = ki18n_rs_ktranscript_eval(argvPtrs.constData(),
                                                 size_t(argvPtrs.size()),
                                                 langB.constData(),
                                                 ctryB.constData(),
                                                 msgctxtB.constData(),
                                                 dynKeyPtrs.isEmpty() ? nullptr : dynKeyPtrs.constData(),
                                                 dynValPtrs.isEmpty() ? nullptr : dynValPtrs.constData(),
                                                 size_t(dynKeyPtrs.size()),
                                                 msgidB.constData(),
                                                 subsPtrs.isEmpty() ? nullptr : subsPtrs.constData(),
                                                 size_t(subsPtrs.size()),
                                                 valsPtrs.isEmpty() ? nullptr : valsPtrs.constData(),
                                                 size_t(valsPtrs.size()),
                                                 ftransB.constData(),
                                                 mod_count ? modPathPtrs.constData() : nullptr,
                                                 mod_count ? modLangPtrs.constData() : nullptr,
                                                 mod_count,
                                                 &out_result,
                                                 &out_err,
                                                 &out_fb);

        mods.clear();

        if (rc != 0) {
            if (out_err) {
                error = QString::fromUtf8(out_err);
                ki18n_rs_string_free(out_err);
            } else {
                error = QStringLiteral("ki18n_rs_ktranscript_eval failed");
            }
            if (out_result) {
                ki18n_rs_string_free(out_result);
            }
            return {};
        }

        if (out_err) {
            error = QString::fromUtf8(out_err);
            ki18n_rs_string_free(out_err);
            if (out_result) {
                ki18n_rs_string_free(out_result);
            }
            return {};
        }

        fallback = (out_fb != 0);
        QString ret;
        if (out_result) {
            ret = QString::fromUtf8(out_result);
            ki18n_rs_string_free(out_result);
        }
        return ret;
    }

    QStringList postCalls(const QString &) override
    {
        // Full-string transcript in Rust runs forall hooks in `apply_transcript_with_fallback`.
        // Pure C++ KLocalizedString + persistent QJSEngine would queue names here; not needed
        // for the Rust-backed message path.
        return {};
    }
};

Q_GLOBAL_STATIC(KTranscriptRust, s_ktranscript_rust)

extern "C" {

KTRANSCRIPT_EXPORT KTranscript *load_transcript()
{
    return s_ktranscript_rust();
}

} // extern "C"
