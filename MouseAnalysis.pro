QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets quickwidgets qml quick quickcontrols2

CONFIG += c++17
RC_ICONS  = logo.ico

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# 检查是否是 release 构建模式
CONFIG(release, debug|release) {
    DEFINES += REDIRECT_DEBUG_OUTPUT
}



SOURCES += \
    component/canvaseitembase.cpp \
    component/clickablewidget.cpp \
    component/comboxitemdelegate.cpp \
    component/commandprocess.cpp \
    component/common.cpp \
    component/configsaver.cpp \
    component/csvparser.cpp \
    component/imagelist.cpp \
    component/mygraphicsview.cpp \
    component/settingsdialog.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    component/canvaseitembase.h \
    component/clickablewidget.h \
    component/comboxitemdelegate.h \
    component/commandprocess.h \
    component/common.h \
    component/configsaver.h \
    component/csvparser.h \
    component/imagelist.h \
    component/mygraphicsview.h \
    component/settingsdialog.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


DISTFILES +=

RESOURCES += \
    assets.qrc
