include_directories(../)

add_library(ukuifilemetadata_ffmpegextractor MODULE
        ffmpeg-extractor.cpp
        )
target_link_libraries(ukuifilemetadata_ffmpegextractor
        ukui-file-metadata
        PkgConfig::AVCODEC
        PkgConfig::AVFORMAT
        PkgConfig::AVUTIL
        PkgConfig::SWSCALE
        )
set_target_properties(ukuifilemetadata_ffmpegextractor PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/ukuifilemetadata")
install(
        TARGETS ukuifilemetadata_ffmpegextractor
        DESTINATION "${PLUGIN_INSTALL_DIR}")

#
#office (binary)
#

add_library(ukuifilemetadata_officeextractor MODULE office-extractor.cpp binary-parser.cpp)
target_link_libraries(ukuifilemetadata_officeextractor
        ukui-file-metadata)
set_target_properties(ukuifilemetadata_officeextractor PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/ukuifilemetadata")
install(
        TARGETS ukuifilemetadata_officeextractor
        DESTINATION "${PLUGIN_INSTALL_DIR}")

#
#office2007
#

add_library(ukuifilemetadata_office2007extractor MODULE office2007-extractor.cpp)
target_link_libraries(ukuifilemetadata_office2007extractor
        ukui-file-metadata)
set_target_properties(ukuifilemetadata_office2007extractor PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/ukuifilemetadata")
install(
        TARGETS ukuifilemetadata_office2007extractor
        DESTINATION "${PLUGIN_INSTALL_DIR}")

#
#text
#

add_library(ukuifilemetadata_textextractor MODULE text-extractor.cpp)
target_link_libraries(ukuifilemetadata_textextractor
        ukui-file-metadata
        PkgConfig::UCHARDET
        )
set_target_properties(ukuifilemetadata_textextractor PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/ukuifilemetadata")
install(
        TARGETS ukuifilemetadata_textextractor
        DESTINATION "${PLUGIN_INSTALL_DIR}")

#
#pdf
#

add_library(ukuifilemetadata_pdfextractor MODULE pdf-extractor.cpp)
target_link_libraries(ukuifilemetadata_pdfextractor
        ukui-file-metadata
        PkgConfig::POPPLER
        )
set_target_properties(ukuifilemetadata_pdfextractor PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/ukuifilemetadata")
install(
        TARGETS ukuifilemetadata_pdfextractor
        DESTINATION "${PLUGIN_INSTALL_DIR}")

#
#uof
#

add_library(ukuifilemetadata_uofextractor MODULE uof-extractor.cpp)
target_link_libraries(ukuifilemetadata_uofextractor
        ukui-file-metadata)
set_target_properties(ukuifilemetadata_uofextractor PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/ukuifilemetadata")
install(
        TARGETS ukuifilemetadata_uofextractor
        DESTINATION "${PLUGIN_INSTALL_DIR}")

#
#ofd
#

add_library(ukuifilemetadata_ofdextractor MODULE ofd-extractor.cpp)
target_link_libraries(ukuifilemetadata_ofdextractor
        ukui-file-metadata)
set_target_properties(ukuifilemetadata_ofdextractor PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/ukuifilemetadata")
install(
        TARGETS ukuifilemetadata_ofdextractor
        DESTINATION "${PLUGIN_INSTALL_DIR}")

#
#png
#

add_library(ukuifilemetadata_pngextractor MODULE png-extractor.cpp)
target_link_libraries(ukuifilemetadata_pngextractor
        ukui-file-metadata)
set_target_properties(ukuifilemetadata_pngextractor PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/ukuifilemetadata")
install(
        TARGETS ukuifilemetadata_pngextractor
        DESTINATION "${PLUGIN_INSTALL_DIR}")

#
#taglib
#

    add_library(ukuifilemetadata_taglibextractor MODULE taglib-extractor.cpp)
    target_link_libraries( ukuifilemetadata_taglibextractor
            ukui-file-metadata
            PkgConfig::TAGLIB
    )

    set_target_properties(ukuifilemetadata_taglibextractor PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/ukuifilemetadata")
    install(
            TARGETS ukuifilemetadata_taglibextractor
            DESTINATION "${PLUGIN_INSTALL_DIR}")

#
#image
#

add_library(ukuifilemetadata_imageextractor MODULE image-extractor.cpp)
target_link_libraries(ukuifilemetadata_imageextractor
        ukui-file-metadata)
set_target_properties(ukuifilemetadata_imageextractor PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/ukuifilemetadata")
install(
        TARGETS ukuifilemetadata_imageextractor
        DESTINATION "${PLUGIN_INSTALL_DIR}")

#
#bookmarks
#

add_library(ukuifilemetadata_bookmarksextractor MODULE bookmarks-extractor.cpp)
target_link_libraries(ukuifilemetadata_bookmarksextractor
        ukui-file-metadata)
set_target_properties(ukuifilemetadata_bookmarksextractor PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/ukuifilemetadata")
install(
        TARGETS ukuifilemetadata_bookmarksextractor
        DESTINATION "${PLUGIN_INSTALL_DIR}")
