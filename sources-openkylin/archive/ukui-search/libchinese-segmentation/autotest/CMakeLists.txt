set(CMAKE_AUTOMOC on)
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Test REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Test REQUIRED)

include_directories(
    ..
    ../storage-base
    ../storage-base/cedar
    ../storage-base/darts-clone
    ../cppjieba
    ../cppjieba/limonp
    ../pinyin4cpp
    ../Traditional-Chinese-Simplified-conversion
    ${Qt${QT_VERSION_MAJOR}Core_INCLUDE_DIRS}
    ${Qt${QT_VERSION_MAJOR}Test_INCLUDE_DIRS}
)

add_executable(traditional2SimplifiedTrieTest
    traditional2simplified-trie-test.cpp
    ../Traditional-Chinese-Simplified-conversion/Traditional2Simplified_trie.cpp
    ../storage-base/storage-base.cpp
)
target_compile_definitions(traditional2SimplifiedTrieTest PRIVATE
    CHINESESEGMENTATION_LIBRARY
)
target_compile_options(traditional2SimplifiedTrieTest PRIVATE
    -UDICT_INSTALL_PATH
    -DDICT_INSTALL_PATH="${CMAKE_CURRENT_SOURCE_DIR}/fixtures/traditional"
)
target_link_libraries(traditional2SimplifiedTrieTest PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
)
add_test(traditional2SimplifiedTrieTest ${CMAKE_BINARY_DIR}/libchinese-segmentation/autotest/traditional2SimplifiedTrieTest)

add_executable(traditional2SimplifiedTest
    traditional2simplified-test.cpp
    ../Traditional-to-Simplified.cpp
    ../Traditional-Chinese-Simplified-conversion/Traditional2Simplified_trie.cpp
    ../storage-base/storage-base.cpp
)
target_compile_definitions(traditional2SimplifiedTest PRIVATE
    CHINESESEGMENTATION_LIBRARY
    DICT_INSTALL_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../Traditional-Chinese-Simplified-conversion/dict"
)
target_link_libraries(traditional2SimplifiedTest PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
)
add_test(traditional2SimplifiedTest ${CMAKE_BINARY_DIR}/libchinese-segmentation/autotest/traditional2SimplifiedTest)

add_executable(pinyin4cppTrieTest
    pinyin4cpp-trie-test.cpp
    ../pinyin4cpp/pinyin4cpp-trie.cpp
    ../pinyin4cpp/pinyin4cpp_dataTrie.cpp
    ../storage-base/storage-base.cpp
)
target_compile_definitions(pinyin4cppTrieTest PRIVATE
    CHINESESEGMENTATION_LIBRARY
)
target_compile_options(pinyin4cppTrieTest PRIVATE
    -UDICT_INSTALL_PATH
    -DDICT_INSTALL_PATH="${CMAKE_CURRENT_SOURCE_DIR}/fixtures/pinyin4cpp"
)
target_link_libraries(pinyin4cppTrieTest PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
)
add_test(pinyin4cppTrieTest ${CMAKE_BINARY_DIR}/libchinese-segmentation/autotest/pinyin4cppTrieTest)

add_executable(pinyin4cppDictTrieTest
    pinyin4cpp-dicttrie-test.cpp
    ../pinyin4cpp/pinyin4cpp_dictTrie.cpp
    ../pinyin4cpp/pinyin4cpp_dataTrie.cpp
)
target_compile_definitions(pinyin4cppDictTrieTest PRIVATE
    CHINESESEGMENTATION_LIBRARY
)
target_link_libraries(pinyin4cppDictTrieTest PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
)
add_test(pinyin4cppDictTrieTest ${CMAKE_BINARY_DIR}/libchinese-segmentation/autotest/pinyin4cppDictTrieTest)

add_executable(storageBaseWhiteboxTest
    storage-base-whitebox-test.cpp
)
target_compile_definitions(storageBaseWhiteboxTest PRIVATE
    CHINESESEGMENTATION_LIBRARY
)
target_link_libraries(storageBaseWhiteboxTest PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
)
add_test(storageBaseWhiteboxTest ${CMAKE_BINARY_DIR}/libchinese-segmentation/autotest/storageBaseWhiteboxTest)

add_executable(hanziToPinyinTest
    hanzi-to-pinyin-test.cpp
    ../hanzi-to-pinyin.cpp
    ../chinese-segmentation.cpp
    ../pinyin4cpp/pinyin4cpp-trie.cpp
    ../pinyin4cpp/pinyin4cpp_dataTrie.cpp
    ../storage-base/storage-base.cpp
    ../cppjieba/idf-trie/idf-trie.cpp
    ../cppjieba/segment-trie/segment-trie.cpp
)
target_compile_definitions(hanziToPinyinTest PRIVATE
    CHINESESEGMENTATION_LIBRARY
    DICT_INSTALL_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../dict"
)
target_link_libraries(hanziToPinyinTest PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
)
add_test(hanziToPinyinTest ${CMAKE_BINARY_DIR}/libchinese-segmentation/autotest/hanziToPinyinTest)

add_executable(chineseSegmentationTest
    chinese-segmentation-test.cpp
    ../chinese-segmentation.cpp
    ../storage-base/storage-base.cpp
    ../cppjieba/idf-trie/idf-trie.cpp
    ../cppjieba/segment-trie/segment-trie.cpp
)
target_compile_definitions(chineseSegmentationTest PRIVATE
    CHINESESEGMENTATION_LIBRARY
    DICT_INSTALL_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../dict"
)
target_link_libraries(chineseSegmentationTest PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
)
add_test(chineseSegmentationTest ${CMAKE_BINARY_DIR}/libchinese-segmentation/autotest/chineseSegmentationTest)

add_executable(cppJiebaWhiteboxTest
    cppjieba-whitebox-test.cpp
    ../storage-base/storage-base.cpp
    ../cppjieba/idf-trie/idf-trie.cpp
    ../cppjieba/segment-trie/segment-trie.cpp
)
target_compile_definitions(cppJiebaWhiteboxTest PRIVATE
    CHINESESEGMENTATION_LIBRARY
    DICT_INSTALL_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../dict"
)
target_link_libraries(cppJiebaWhiteboxTest PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
)
add_test(cppJiebaWhiteboxTest ${CMAKE_BINARY_DIR}/libchinese-segmentation/autotest/cppJiebaWhiteboxTest)

add_executable(cppJiebaFixtureDictTest
    cppjieba-fixture-dict-test.cpp
    ../storage-base/storage-base.cpp
    ../cppjieba/idf-trie/idf-trie.cpp
    ../cppjieba/segment-trie/segment-trie.cpp
)
target_compile_definitions(cppJiebaFixtureDictTest PRIVATE
    CHINESESEGMENTATION_LIBRARY
)
target_compile_options(cppJiebaFixtureDictTest PRIVATE
    -UDICT_INSTALL_PATH
    -DDICT_INSTALL_PATH="${CMAKE_CURRENT_SOURCE_DIR}/fixtures/cppjieba"
)
target_link_libraries(cppJiebaFixtureDictTest PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
)
add_test(cppJiebaFixtureDictTest ${CMAKE_BINARY_DIR}/libchinese-segmentation/autotest/cppJiebaFixtureDictTest)
