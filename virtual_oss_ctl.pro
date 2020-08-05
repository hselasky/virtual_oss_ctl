isEmpty(PREFIX) {
    PREFIX=/usr/local
}
TEMPLATE	= app
QT		+= core gui widgets
CONFIG		+= qt warn_on release

HEADERS		+= virtual_oss_ctl.h
HEADERS		+= virtual_oss_ctl_compressor.h
HEADERS		+= virtual_oss_ctl_connect.h
HEADERS         += virtual_oss_ctl_button.h
HEADERS         += virtual_oss_ctl_buttonmap.h
HEADERS         += virtual_oss_ctl_equalizer.h
HEADERS         += virtual_oss_ctl_groupbox.h
HEADERS         += virtual_oss_ctl_gridlayout.h
HEADERS         += virtual_oss_ctl_mainwindow.h
HEADERS         += virtual_oss_ctl_volume.h

SOURCES		+= virtual_oss_ctl.cpp
SOURCES		+= virtual_oss_ctl_compressor.cpp
SOURCES		+= virtual_oss_ctl_connect.cpp
SOURCES         += virtual_oss_ctl_button.cpp
SOURCES         += virtual_oss_ctl_buttonmap.cpp
SOURCES         += virtual_oss_ctl_equalizer.cpp
SOURCES         += virtual_oss_ctl_groupbox.cpp
SOURCES         += virtual_oss_ctl_gridlayout.cpp
SOURCES         += virtual_oss_ctl_mainwindow.cpp
SOURCES         += virtual_oss_ctl_volume.cpp

RESOURCES	+= virtual_oss_ctl.qrc

TARGET		= virtual_oss_ctl

LIBS		+= -L$${PREFIX}/lib
INCLUDEPATH	+= $${PREFIX}/include

target.path	= $${PREFIX}/bin
INSTALLS	+= target

icons.path	= $${PREFIX}/share/pixmaps
icons.files	= virtual_oss_ctl.png
INSTALLS	+= icons

desktop.path	= $${PREFIX}/share/applications
desktop.files	= virtual_oss_ctl.desktop
INSTALLS	+= desktop

