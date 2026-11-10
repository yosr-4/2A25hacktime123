QT += core gui widgets sql printsupport

CONFIG += c++17

TARGET = HackTime
TEMPLATE = app

# fichiers sources en UTF-8 (accents, emojis)
msvc: QMAKE_CXXFLAGS += /utf-8

SOURCES += \
    competition.cpp \
    connection.cpp \
    employe.cpp \
    hacktime.cpp \
    main.cpp \
    statsemploye.cpp

HEADERS += \
    competition.h \
    connection.h \
    employe.h \
    hacktime.h \
    statsemploye.h

FORMS += \
    hacktime.ui

RESOURCES += \
    ressources.qrc
