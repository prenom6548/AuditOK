TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS += \
    dataparser \
    datarequest \
    #dbus \
    iplocation \
    network \
    searchmarch \
    titlesearchmodel

dataparser.file = dataparser/dataparser-test.pro
datarequest.file = datarequest/datarequest-test.pro
#dbus.file = dbus/dbus-test.pro
iplocation.file = iplocation/iplocation-test.pro
network.file = network/network-test.pro
searchmarch.file = searchmarch/searchmarch-test.pro
titlesearchmodel.file = titlesearchmodel/titlesearchmodel-test.pro
