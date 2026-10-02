# PAG dynamic wallpaper rendering component
# Integrates libpag (Apache-2.0) into peony-qt-desktop as a QQuickItem.
# Requires: libpag-dev, libtgfx-dev installed to /usr/include/pag and /usr/include/tgfx.

INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/Drawable.h \
    $$PWD/GPUDrawable.h \
    $$PWD/PAGRenderThread.h \
    $$PWD/PAGView.h

SOURCES += \
    $$PWD/PAGRenderThread.cpp \
    $$PWD/PAGView.cpp

LIBS += -lpag -ltgfx -lffavc
