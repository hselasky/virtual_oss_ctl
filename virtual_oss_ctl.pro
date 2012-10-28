TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		+= virtual_oss_ctl.h
SOURCES		+= virtual_oss_ctl.cpp

RESOURCES	+= virtual_oss_ctl.qrc

TARGET		= virtual_oss_ctl

LIBS		+= -L$${PREFIX}/lib -lcuse4bsd
INCLUDEPATH	+= $${PREFIX}/include

target.path	= $${PREFIX}/bin
INSTALLS	+= target

icons.path	= $${PREFIX}/share/pixmaps
icons.files	= virtual_oss_ctl.png
INSTALLS	+= icons

desktop.path	= $${PREFIX}/share/applications
desktop.files	= virtual_oss_ctl.desktop
INSTALLS	+= desktop
