OTHER_FILES +=\
$${PWD}/lib/a.param \
$${PWD}/lib/d201.param \
$${PWD}/params/


myfiles.files = \
$${PWD}/lib/a.param \
$${PWD}/lib/d201.param \
$${PWD}/params/

deployment.files += $${PWD}/params/a.param
deployment.files += $${PWD}/params/d201.param

#deployment.files += MyFile2

deployment.path = /assets
INSTALLS += deployment

unix:!android {
    isEmpty(target.path) {
        qnx {
            target.path = /tmp/$${TARGET}/bin
            myfiles.path = /tmp/$${TARGET}/bin

        } else {
            target.path = /opt/$${TARGET}/bin
            myfiles.path = /opt/$${TARGET}/bin
        }
        export(target.path)
        export(myfiles.path)
    }
    INSTALLS += target myfiles
    QMAKE_BUNDLE_DATA  += myfiles
}

export(QMAKE_BUNDLE_DATA)
export(INSTALLS)

DISTFILES += \
    $$PWD/lib/a.param \
    $$PWD/lib/d201.param
