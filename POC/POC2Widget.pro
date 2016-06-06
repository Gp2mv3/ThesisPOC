#-------------------------------------------------
#
# Project created by QtCreator 2016-05-29T12:36:23
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = POC2Widget
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    logger.cpp \
    vlrWrapper.cpp \
    lib/bb.c \
    lib/bbs.c \
    lib/bls.c \
    lib/hash.c \
    lib/vlr.c \
    networkdevicehandler.cpp \
    keys.cpp

HEADERS  += mainwindow.h \
    logger.h \
    vlrWrapper.h \
    lib/inc/bb.h \
    lib/inc/bbs.h \
    lib/inc/bls.h \
    lib/inc/hash.h \
    lib/inc/sig.h \
    lib/inc/vlr.h \
    networkdevicehandler.h \
    keys.h

FORMS    += mainwindow.ui

CONFIG += mobility
MOBILITY = 


package.files = params
android {
    package.path = /assets
} else {
    package.path = /
}
INSTALLS += package

#Others (ARM Android): Ignore incompatibles when compiling for x86
android {

LIBS += -L$$PWD/../../DevEnv/libs/pbc-0.5.14/arm/lib/ -lpbc
INCLUDEPATH += $$PWD/../../DevEnv/libs/pbc-0.5.14/arm/include
DEPENDPATH += $$PWD/../../DevEnv/libs/pbc-0.5.14/arm/include
PRE_TARGETDEPS += $$PWD/../../DevEnv/libs/pbc-0.5.14/arm/lib/libpbc.a

LIBS += -L$$PWD/../../DevEnv/libs/openssl-1.0.2g/ -lcrypto_1_0_0
INCLUDEPATH += $$PWD/../../DevEnv/libs/openssl-1.0.2g/include
DEPENDPATH += $$PWD/../../DevEnv/libs/openssl-1.0.2g/include


LIBS += -L$$PWD/../../DevEnv/libs/gmp-master/armeabi-v7a/ -lgmp
INCLUDEPATH += $$PWD/../../DevEnv/libs/gmp-master/armeabi-v7a
DEPENDPATH += $$PWD/../../DevEnv/libs/gmp-master/armeabi-v7a

} else {
#x86
LIBS += -lpbc -lgmp -lcrypto
}

contains(ANDROID_TARGET_ARCH,armeabi-v7a) {
    ANDROID_EXTRA_LIBS = /home/gp2mv3/Documents/EPL/ELEC22M/Memoire/Anonymity/Mobile/POC2Widget/../../DevEnv/libs/gmp-master/armeabi-v7a/libgmpxx.so /home/gp2mv3/Documents/EPL/ELEC22M/Memoire/Anonymity/Mobile/POC2Widget/../../DevEnv/libs/gmp-master/armeabi-v7a/libgmp.so /home/gp2mv3/Documents/EPL/ELEC22M/Memoire/Anonymity/Mobile/POC2Widget/../../DevEnv/libs/openssl-1.0.2g/libssl_1_0_0.so /home/gp2mv3/Documents/EPL/ELEC22M/Memoire/Anonymity/Mobile/POC2Widget/../../DevEnv/libs/openssl-1.0.2g/libcrypto_1_0_0.so
}

DISTFILES += \
    deployment.pri \
    android/AndroidManifest.xml \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradlew \
    android/res/values/libs.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew.bat

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
