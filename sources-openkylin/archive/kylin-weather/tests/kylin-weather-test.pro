TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS += \
    controller \
    global \
    model \
    view

controller.file = controller/controller-test.pro
global.file = global/global-test.pro
model.file = model/model-test.pro
view.file = view/view-test.pro