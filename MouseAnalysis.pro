QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets quickwidgets qml quick

CONFIG += c++17
RC_ICONS  = logo.ico

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    component/clickablewidget.cpp \
    component/commandprocess.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    component/clickablewidget.h \
    component/commandprocess.h \
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
