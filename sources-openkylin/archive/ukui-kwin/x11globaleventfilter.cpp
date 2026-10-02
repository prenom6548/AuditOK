#include "x11globaleventfilter.h"

namespace KWin {

X11GlobalEventFilter::X11GlobalEventFilter()
{
    X11EventDispatch::self()->registerGlobalEventFilter(this);
    LibinputEventDispatch::self()->registerGlobalEventFilter(this);
}

X11GlobalEventFilter::~X11GlobalEventFilter()
{
    X11EventDispatch::self()->unregisterGlobalEventFilter(this);
    LibinputEventDispatch::self()->unregisterGlobalEventFilter(this);
}

}
