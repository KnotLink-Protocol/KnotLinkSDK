INCLUDEPATH += $$PWD/

QT       += network

SOURCES += \
        $$PWD/opensocketquerier.cpp \
        $$PWD/opensocketresponser.cpp \
        $$PWD/signalsender.cpp \
        $$PWD/signalsubscriber.cpp  \
        $$PWD/tcpclient.cpp

HEADERS += \
        $$PWD/opensocketquerier.h \
        $$PWD/opensocketresponser.h \
        $$PWD/signalsender.h \
        $$PWD/signalsubscriber.h  \
        $$PWD/tcpclient.h
