# SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
# SPDX-License-Identifier: GPL-3.0-or-later

# KF5 开发文件：头文件、CMake 包、pri、qlogging、可选 QCH。需 KI18N_RS_FULL_INSTALL=ON。

if(NOT KI18N_RS_FULL_INSTALL)
    return()
endif()

find_package(ECM 5.92 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)
include(KDEInstallDirs)
include(CMakePackageConfigHelpers)
include(ECMGeneratePriFile)

# 与根目录 multiarch 安装一致（KDEInstallDirs 可能覆盖）
set(CMAKE_INSTALL_LIBDIR "/usr/lib/${ARCH_TRIPLET}" CACHE STRING "" FORCE)

set(ECM_MKSPECS_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/qt5/mkspecs/modules" CACHE PATH "" FORCE)

set(CMAKECONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/KF5I18n")

# --- 与 ecm_generate_headers 等价的转发头（一行 #include）---
set(_ki18n_fwd "${CMAKE_BINARY_DIR}/kf5-devel/include/KF5/KI18n")
set(_ld_fwd "${CMAKE_BINARY_DIR}/kf5-devel/include/KF5/KI18nLocaleData")
file(MAKE_DIRECTORY "${_ki18n_fwd}" "${_ld_fwd}")

# 分号须写成 \;，否则 set() 会拆成两个列表项，转发头会变成 #include "KLocalizedString" 自包含死循环。
set(_fwd_pairs
    "KLazyLocalizedString\;klazylocalizedstring.h"
    "KLocalizedContext\;klocalizedcontext.h"
    "KLocalizedString\;klocalizedstring.h"
    "KLocalizedTranslator\;klocalizedtranslator.h"
    "KuitMarkup\;kuitmarkup.h"
    "KuitSetup\;kuitsetup.h"
)
foreach(_pair IN LISTS _fwd_pairs)
    string(REGEX REPLACE "^([^;]*);(.*)$" "\\1" _camel "${_pair}")
    string(REGEX REPLACE "^([^;]*);(.*)$" "\\2" _real "${_pair}")
    file(WRITE "${_ki18n_fwd}/${_camel}" "#include \"${_real}\"\n")
endforeach()

foreach(_entry IN ITEMS "KCountry;kcountry.h" "KCountrySubdivision;kcountrysubdivision.h" "KTimeZone;ktimezone.h")
    string(REGEX REPLACE "^([^;]*);(.*)$" "\\1" _camel "${_entry}")
    string(REGEX REPLACE "^([^;]*);(.*)$" "\\2" _real "${_entry}")
    file(WRITE "${_ld_fwd}/${_camel}" "#include \"${_real}\"\n")
endforeach()

# --- 安装头文件 ---
foreach(_h IN ITEMS
        klazylocalizedstring.h klocalizedcontext.h klocalizedstring.h
        klocalizedtranslator.h kuitmarkup.h kuitsetup.h kcatalog.h)
    install(FILES "${CMAKE_SOURCE_DIR}/cpp/include/KLocalizedString/${_h}"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/KF5/KI18n")
endforeach()

install(FILES
    "${CMAKE_SOURCE_DIR}/cpp/include/KLocalizedString/kcatalog_p.h"
    "${CMAKE_SOURCE_DIR}/cpp/include/KLocalizedString/ktranscript_p.h"
    "${CMAKE_SOURCE_DIR}/cpp/include/KLocalizedString/ktranscript_export.h"
    "${CMAKE_SOURCE_DIR}/cpp/include/ki18n_export.h"
    "${CMAKE_SOURCE_DIR}/cmake/ki18n_version.h"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/KF5/KI18n")

file(GLOB _ki18n_fwd_files LIST_DIRECTORIES false "${_ki18n_fwd}/*")
install(FILES ${_ki18n_fwd_files} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/KF5/KI18n")

install(FILES
    "${KI18N_LOCALEDATA_HEADERS_DIR}/kcountry.h"
    "${KI18N_LOCALEDATA_HEADERS_DIR}/kcountrysubdivision.h"
    "${KI18N_LOCALEDATA_HEADERS_DIR}/ktimezone.h"
    "${CMAKE_SOURCE_DIR}/cpp/include/ki18nlocaledata_export.h"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/KF5/KI18nLocaleData")

file(GLOB _ld_fwd_files LIST_DIRECTORIES false "${_ld_fwd}/*")
install(FILES ${_ld_fwd_files} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/KF5/KI18nLocaleData")

# --- QCH（可选）；PACKAGE_INCLUDE_QCHTARGETS 供 KF5I18nConfig.cmake.in 使用 ---
set(PACKAGE_INCLUDE_QCHTARGETS "")
if(KI18N_RS_BUILD_QCH)
    if(NOT EXISTS "${KI18N_DATA_DOCS_DIR}/programmers-guide.md")
        message(FATAL_ERROR "KI18N_RS_BUILD_QCH: missing ${KI18N_DATA_DOCS_DIR}/programmers-guide.md (see ki18n-data/docs/)")
    endif()
    include(FeatureSummary)
    include(ECMAddQch)
    find_package(Qt5 COMPONENTS Core REQUIRED)

    set(_qch_sources
        "${CMAKE_SOURCE_DIR}/cpp/include/KLocalizedString/klazylocalizedstring.h"
        "${CMAKE_SOURCE_DIR}/cpp/include/KLocalizedString/klocalizedcontext.h"
        "${CMAKE_SOURCE_DIR}/cpp/include/KLocalizedString/klocalizedstring.h"
        "${CMAKE_SOURCE_DIR}/cpp/include/KLocalizedString/klocalizedtranslator.h"
        "${CMAKE_SOURCE_DIR}/cpp/include/KLocalizedString/kuitmarkup.h"
        "${CMAKE_SOURCE_DIR}/cpp/include/KLocalizedString/kuitsetup.h"
        "${CMAKE_SOURCE_DIR}/cpp/include/KLocalizedString/kcatalog.h"
        "${CMAKE_SOURCE_DIR}/cpp/include/ki18n_export.h"
        "${KI18N_LOCALEDATA_HEADERS_DIR}/kcountry.h"
        "${KI18N_LOCALEDATA_HEADERS_DIR}/kcountrysubdivision.h"
        "${KI18N_LOCALEDATA_HEADERS_DIR}/ktimezone.h"
        "${CMAKE_SOURCE_DIR}/cpp/include/ki18nlocaledata_export.h"
        "${KI18N_DATA_DOCS_DIR}/programmers-guide.md"
        "${KI18N_DATA_DOCS_DIR}/translators-guide.md"
    )
    foreach(_pair IN LISTS _fwd_pairs)
        string(REGEX REPLACE "^([^;]*);(.*)$" "\\1" _camel "${_pair}")
        list(APPEND _qch_sources "${_ki18n_fwd}/${_camel}")
    endforeach()
    foreach(_c IN ITEMS KCountry KCountrySubdivision KTimeZone)
        list(APPEND _qch_sources "${_ld_fwd}/${_c}")
    endforeach()

    ecm_add_qch(
        KF5I18n_QCH
        NAME KI18n
        BASE_NAME KF5I18n
        VERSION 5.92.0
        ORG_DOMAIN org.kde
        SOURCES ${_qch_sources}
        MD_MAINPAGE "${KI18N_DATA_DOCS_DIR}/README.md"
        LINK_QCHS Qt5Core_QCH
        INCLUDE_DIRS
            "${CMAKE_BINARY_DIR}/kf5-devel/include/KF5/KI18n"
            "${CMAKE_BINARY_DIR}/kf5-devel/include/KF5/KI18nLocaleData"
            "${CMAKE_SOURCE_DIR}/cpp/include/KLocalizedString"
            "${CMAKE_SOURCE_DIR}/cpp/include"
            "${KI18N_LOCALEDATA_HEADERS_DIR}"
        BLANK_MACROS
            KI18N_EXPORT
            KI18N_DEPRECATED_EXPORT
            KI18N_DEPRECATED
            "KI18N_DEPRECATED_VERSION(x, y, t)"
            KI18NLOCALEDATA_EXPORT
            KI18NLOCALEDATA_DEPRECATED_EXPORT
            KI18NLOCALEDATA_DEPRECATED
            "KI18NLOCALEDATA_DEPRECATED_VERSION(x, y, t)"
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
    )
    ecm_install_qch_export(
        TARGETS KF5I18n_QCH
        FILE "${CMAKE_BINARY_DIR}/cmake/KF5I18nQchTargets.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF5I18nQchTargets.cmake\")")
endif()

# 勿先用 configure_file(@ONLY) 处理 .in：会误清空 @PACKAGE_INIT@。
configure_package_config_file(
    "${CMAKE_SOURCE_DIR}/cmake/KF5I18nConfig.cmake.in"
    "${CMAKE_BINARY_DIR}/cmake/KF5I18nConfig.cmake"
    INSTALL_DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
)

write_basic_package_version_file(
    "${CMAKE_BINARY_DIR}/cmake/KF5I18nConfigVersion.cmake"
    VERSION 5.92.0
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_BINARY_DIR}/cmake/KF5I18nConfig.cmake"
    "${CMAKE_BINARY_DIR}/cmake/KF5I18nConfigVersion.cmake"
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
)

install(EXPORT KF5I18nTargets
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    FILE KF5I18nTargets.cmake
    NAMESPACE KF5::
)

install(FILES
    "${CMAKE_BINARY_DIR}/cmake/KF5I18nMacros.cmake"
    "${KI18N_DATA_DIR}/cmake/kf5i18nuic.cmake"
    "${KI18N_DATA_DIR}/cmake/build-pofiles.cmake"
    "${KI18N_DATA_DIR}/cmake/build-tsfiles.cmake"
    "${KI18N_DATA_DIR}/cmake/ts-pmap-compile.py"
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
)

if(EXISTS "${KI18N_QLOGGING_DIR}/ki18n.categories")
    install(FILES
        "${KI18N_QLOGGING_DIR}/ki18n.categories"
        "${KI18N_QLOGGING_DIR}/ki18n.renamecategories"
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/qlogging-categories5"
    )
endif()

ecm_generate_pri_file(
    BASE_NAME KI18n
    LIB_NAME KF5I18n
    DEPS "core"
    VERSION 5.92.0
    FILENAME_VAR _ki18n_pri
    INCLUDE_INSTALL_DIRS "${CMAKE_INSTALL_INCLUDEDIR}/KF5/KI18n"
    LIB_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}"
)
install(FILES "${_ki18n_pri}" DESTINATION "${ECM_MKSPECS_INSTALL_DIR}")
