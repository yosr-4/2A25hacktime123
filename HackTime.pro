QT += core gui widgets sql printsupport

# Borne Arduino : nécessite le module Qt Serial Port.
# S'il n'est pas installé, l'application compile quand même (sans la borne).
qtHaveModule(serialport) {
    QT += serialport
} else {
    DEFINES += SANS_SERIALPORT
    message("Module Qt Serial Port absent : borne Arduino desactivee")
}

CONFIG += c++17

TARGET = HackTime
TEMPLATE = app

# fichiers sources en UTF-8 (accents, emojis)
msvc: QMAKE_CXXFLAGS += /utf-8

SOURCES += \
    arduino.cpp \
    competition.cpp \
    connection.cpp \
    employe.cpp \
    hacktime.cpp \
    main.cpp \
    participant.cpp \
    participantborne.cpp \
    participantui.cpp \
    statsemploye.cpp

HEADERS += \
    arduino.h \
    competition.h \
    connection.h \
    employe.h \
    hacktime.h \
    participant.h \
    participantborne.h \
    participantui.h \
    statsemploye.h

FORMS += \
    hacktime.ui

RESOURCES += \
    ressources.qrc
