TARGET = LH_LCoreReboot
TEMPLATE = lib
DEFINES += LH_LCOREREBOOT_LIBRARY
DEFINES += VERSION=1.01

include(../Plugins.pri)

HEADERS += \
    $$PLUGIN_HEADERS

SOURCES += \
    $$PLUGIN_SOURCES

win32 {
    SOURCES += LH_QtPlugin_LCoreReboot.cpp
    HEADERS += LH_QtPlugin_LCoreReboot.h
    LIBS += -lpsapi -luser32 -lshell32 -ladvapi32
}
