TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS += \
    bottomlistdelegate \
    collectcity \
    collectitemdelegate \
    hotcity \
    hotcitybtn \
    searchcitylist \
    searchdelegate \
    searchview \
    titlesearchcityview \
    titlesearchlistdelegate \
    bottomlistview

bottomlistdelegate.file = bottomlistdelegate/bottomlistdelegate_test.pro
collectcity.file = collectcity/collectcity_test.pro
collectitemdelegate.file = collectitemdelegate/collectitemdelegate_test.pro
hotcity.file = hotcity/hotcity_test.pro
hotcitybtn.file = hotcitybtn/hotcitybtn_test.pro
searchcitylist.file = searchcitylist/searchcitylist_test.pro
searchdelegate.file = searchdelegate/searchdelegate_test.pro
searchview.file = searchview/searchview_test.pro
titlesearchcityview.file = titlesearchcityview/titlesearchcityview_test.pro
titlesearchlistdelegate.file = titlesearchlistdelegate/titlesearchlistdelegate_test.pro
bottomlistview.file = bottomlistview/bottomlistview_test.pro
