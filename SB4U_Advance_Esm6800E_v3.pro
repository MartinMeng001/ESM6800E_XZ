QT -= gui
QT += network

CONFIG += c++11 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        Annuniator/annuniatortcp.cpp \
        Annuniator/datamgr_fromannuniator.cpp \
        Annuniator/itemannuniatorconnection.cpp \
        Annuniator/manageannuniatorconnections.cpp \
        Annuniator/tcpserver4annuniator.cpp \
        AnnuniatorStatus/annuniatorstatus.cpp \
        AnnuniatorStatus/annuniatortime.cpp \
        AnnuniatorStatus/eventflagbuffer.cpp \
        AnnuniatorStatus/status4gnetwork.cpp \
        AnnuniatorStatus/usage4gnetwork.cpp \
        Configfile/userinfofile.cpp \
        Driver/driver_gpio.cpp \
        Driver/yiyuan_ec20_4gadapter.cpp \
        Log/loggerworker.cpp \
        Platform4G/datamgr_fromplatform.cpp \
        Platform4G/platformtcp.cpp \
        Platform4G/platformtcpv2.cpp \
        Platform808/datamgr_to808platform.cpp \
        Platform808/platform808tcp.cpp \
        Platform808/platform808tcpv2.cpp \
        PlatformAsClient/comanddatamanager.cpp \
        PlatformAsClient/commanddataitem.cpp \
        PlatformAsClient/dataitem_platformclient.cpp \
        PlatformAsClient/itemplatformconnection.cpp \
        PlatformAsClient/manageplatformconnections.cpp \
        PlatformAsClient/tcpserver4platform.cpp \
        Protocol/protocol808platform.cpp \
        Protocol/protocolannuniator4u.cpp \
        Protocol/protocolfor4gserver.cpp \
        RemoteControl/pulse315m.cpp \
        RemoteControl/regionmanager.cpp \
        Worker/worker_4g.cpp \
        Worker/worker_annuniatorstatus.cpp \
        Worker/worker_forgpio.cpp \
        Worker/worker_fromannuniator.cpp \
        Worker/worker_fromplatform.cpp \
        Worker/worker_fromplatformclient.cpp \
        Worker/worker_remotecontrol.cpp \
        Worker/worker_to808platform.cpp \
        main.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    Annuniator/annuniatortcp.h \
    Annuniator/datamgr_fromannuniator.h \
    Annuniator/itemannuniatorconnection.h \
    Annuniator/manageannuniatorconnections.h \
    Annuniator/tcpserver4annuniator.h \
    AnnuniatorStatus/annuniatorstatus.h \
    AnnuniatorStatus/annuniatortime.h \
    AnnuniatorStatus/event_custom.hpp \
    AnnuniatorStatus/eventflagbuffer.h \
    AnnuniatorStatus/status4gnetwork.h \
    AnnuniatorStatus/usage4gnetwork.h \
    Configfile/userinfofile.h \
    Driver/driver_4g.h \
    Driver/driver_gpio.h \
    Driver/yiyuan_ec20_4gadapter.h \
    Log/loggerworker.h \
    Platform4G/datamgr_fromplatform.h \
    Platform4G/platformtcp.h \
    Platform4G/platformtcpv2.h \
    Platform808/datamgr_to808platform.h \
    Platform808/platform808tcp.h \
    Platform808/platform808tcpv2.h \
    PlatformAsClient/comanddatamanager.h \
    PlatformAsClient/commanddataitem.h \
    PlatformAsClient/dataitem_platformclient.h \
    PlatformAsClient/itemplatformconnection.h \
    PlatformAsClient/manageplatformconnections.h \
    PlatformAsClient/tcpserver4platform.h \
    Protocol/protocol808platform.h \
    Protocol/protocolannuniator4u.h \
    Protocol/protocolfor4gserver.h \
    RemoteControl/pulse315m.h \
    RemoteControl/regionmanager.h \
    Worker/worker_4g.h \
    Worker/worker_annuniatorstatus.h \
    Worker/worker_forgpio.h \
    Worker/worker_fromannuniator.h \
    Worker/worker_fromplatform.h \
    Worker/worker_fromplatformclient.h \
    Worker/worker_remotecontrol.h \
    Worker/worker_to808platform.h
