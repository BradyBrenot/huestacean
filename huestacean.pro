#################
# NB: ONLY used for building Android. Use cmake for everything else
#################

QT += quick
QT += network
QT += androidextras
CONFIG += c++14
QMAKE_CXXFLAGS += -std=c++14

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += src/main.cpp \
    src/huebridge.cpp \
    src/bridgediscovery.cpp \
    src/huestacean.cpp \
    src/utility.cpp \
    src/entertainment.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    include/huebridge.h \
    include/bridgediscovery.h \
    include/huestacean.h \
    include/utility.h \
    include/entertainment.h

DISTFILES += \
    README.md \
    LICENSE \
    apache-2.0.txt \
    lgpl-3.txt \
    license-screen_capture_lite.txt \
    android/AndroidManifest.xml \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradlew \
    android/res/values/libs.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew.bat \
    android/src/com/huestacean/Huestacean.java

INCLUDEPATH += include

#mbedtls
INCLUDEPATH += C:/hueproj/huestacean/mbedtls/include

CONFIG(debug, debug|release) {
    LIBS += -LC:/Users/Brady/AndroidStudioProjects/MyApplication/mbedtls/.externalNativeBuild/cmake/debug/armeabi-v7a/library -lmbedtls -lmbedx509 -lmbedcrypto
}

CONFIG(release, debug|release) {
    LIBS += -LC:/Users/Brady/AndroidStudioProjects/MyApplication/mbedtls/.externalNativeBuild/cmake/debug/armeabi-v7a/library -lmbedtls -lmbedx509 -lmbedcrypto
}

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
