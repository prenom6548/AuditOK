# SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-2-Clause

if (QT_VERSION_MAJOR EQUAL "6")
    set(ITEMS_SHADER_FILES
        shadowedrectangle.vert
        shadowedrectangle.frag
        shadowedrectangle_lowpower.frag
        shadowedborderrectangle.frag
        shadowedborderrectangle_lowpower.frag
        shadowedtexture.frag
        shadowedtexture_lowpower.frag
        shadowedbordertexture.frag
        shadowedbordertexture_lowpower.frag
    )

    qt6_add_shaders(${PROJECT_NAME} "shaders6"
        BATCHABLE
        PRECOMPILE
        OPTIMIZED
        PREFIX "/org/ukui/quick/shaders/"
        FILES ${ITEMS_SHADER_FILES}
    )

    if(TARGET ukui-quick-items-test-plugin)
        qt6_add_shaders(ukui-quick-items-test-plugin "shaders6-test"
            BATCHABLE
            PRECOMPILE
            OPTIMIZED
            PREFIX "/org/ukui/quick/shaders/"
            FILES ${ITEMS_SHADER_FILES}
        )
    endif()
endif()
