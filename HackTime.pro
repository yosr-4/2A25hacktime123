QT += core gui widgets sql
CONFIG += c++17

TARGET = HackTime
TEMPLATE = app

# Support UTF-8 source files (French accents and Arabic comments)
msvc: QMAKE_CXXFLAGS += /utf-8

SOURCES += \
    competition.cpp \
    connection.cpp \
    hacktime.cpp \
    interface.cpp \
    main.cpp

HEADERS += \
    competition.h \
    connection.h \
    hacktime.h \
    interface.h

FORMS += \
    hacktime.ui \
    interface.ui

RESOURCES += ressources.qrc
