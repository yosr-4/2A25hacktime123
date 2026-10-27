QT       += core gui widgets sql printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG   += c++17
TARGET    = HackTimeAtelier
TEMPLATE  = app

# Source files are UTF-8 (emojis / accents)
msvc: QMAKE_CXXFLAGS += /utf-8

SOURCES += \
    main.cpp \
    connexion.cpp \
    atelier.cpp \
    statschart.cpp \
    ateliermainwindow.cpp

HEADERS += \
    connexion.h \
    atelier.h \
    statschart.h \
    ateliermainwindow.h

FORMS += \
    ateliermainwindow.ui
