#include "globalsignal.h"

GlobalSignal *GlobalSignal::instance = new GlobalSignal;

GlobalSignal::GlobalSignal(QObject *parent) : QObject(parent) {

}

GlobalSignal::~GlobalSignal() {

}

GlobalSignal *GlobalSignal::getInstance() {
    return instance;
}


