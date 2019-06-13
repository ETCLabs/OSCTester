# ETCOSCTester
# (c) 2019 ETC Inc

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = OSCTester
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++11

# Extract version from Git tag/description
GIT_COMMAND = git --git-dir $$shell_quote($$PWD/.git) --work-tree $$shell_quote($$PWD)
GIT_TAG = $$system($$GIT_COMMAND describe --always --tags)

SOURCES += \
        src/main.cpp \
    src/OSCParser.cpp \
    src/OSCTesterMainWin.cpp

HEADERS += \
    src/OSCParser.h \
    src/OSCTesterMainWin.h

FORMS += \
    ui/OSCTesterMainWin.ui

win32:RC_ICONS += res/icon.ico

CONFIG(debug, debug|release) {
    DESTDIR = $${OUT_PWD}/debug
}
CONFIG(release, debug|release) {
    DESTDIR = $${OUT_PWD}/release
}

# Extract the version using GIT tags
GIT_COMMAND = git --git-dir $$shell_quote($$PWD/.git) --work-tree $$shell_quote($$PWD)
GIT_VERSION = $$system($$GIT_COMMAND describe --always --tags)
GIT_DATE_DAY = $$system($$GIT_COMMAND show -s --date=format:\"%a\" --format=\"%cd\" $$GIT_VERSION)
GIT_DATE_DATE = $$system($$GIT_COMMAND show -s --date=format:\"%d\" --format=\"%cd\" $$GIT_VERSION)
GIT_DATE_MONTH = $$system($$GIT_COMMAND show -s --date=format:\"%b\" --format=\"%cd\" $$GIT_VERSION)
GIT_DATE_YEAR = $$system($$GIT_COMMAND show -s --date=format:\"%Y\" --format=\"%cd\" $$GIT_VERSION)
GIT_TAG = $$system($$GIT_COMMAND describe --abbrev=0 --always --tags)
GIT_SHA1 = $$system($$GIT_COMMAND rev-parse --short HEAD)

DEFINES += VERSION=\\\"$$GIT_VERSION\\\"
DEFINES += GIT_CURRENT_SHA1=\\\"$$GIT_VERSION\\\"
DEFINES += GIT_DATE_DAY=\\\"$$GIT_DATE_DAY\\\"
DEFINES += GIT_DATE_DATE=\\\"$$GIT_DATE_DATE\\\"
DEFINES += GIT_DATE_MONTH=\\\"$$GIT_DATE_MONTH\\\"
DEFINES += GIT_DATE_YEAR=\\\"$$GIT_DATE_YEAR\\\"


# Automatically handle installer generation

isEmpty(TARGET_EXT) {
    win32 {
        TARGET_CUSTOM_EXT = .exe
    }
    macx {
        TARGET_CUSTOM_EXT = .app
    }
} else {
    TARGET_CUSTOM_EXT = $${TARGET_EXT}
}

win32 {
    QMAKE_CXXFLAGS += /Zi
    QMAKE_LFLAGS += /INCREMENTAL:NO /Debug
}

win32 {
    PRODUCT_VERSION = "$$GIT_VERSION"

    DEPLOY_DIR = $$shell_quote($$system_path($${_PRO_FILE_PWD_}/install/deploy))
    DEPLOY_TARGET = $$shell_quote($$system_path($${OUT_PWD}/release/$${TARGET}$${TARGET_CUSTOM_EXT}))

    PRE_DEPLOY_COMMAND += $${QMAKE_DEL_FILE} $${DEPLOY_DIR}\*.* /S /Q $$escape_expand(\\n\\t)
    PRE_DEPLOY_COMMAND += $$QMAKE_COPY $${DEPLOY_TARGET} $${DEPLOY_DIR} $$escape_expand(\\n\\t)

    DEPLOY_COMMAND = windeployqt
    DEPLOY_OPT = --dir $${DEPLOY_DIR}

    DEPLOY_INSTALLER = makensis /DPRODUCT_VERSION="$${PRODUCT_VERSION}" $$shell_quote($$system_path($${_PRO_FILE_PWD_}/install/win/install.nsi))
}
macx {
    VERSION = $$system(echo $$GIT_VERSION | sed 's/[a-zA-Z]//')

    DEPLOY_DIR = $${_PRO_FILE_PWD_}/install/mac
    DEPLOY_TARGET = $${OUT_PWD}/release/$${TARGET}$${TARGET_CUSTOM_EXT}

    DEPLOY_COMMAND = macdeployqt
    DEPLOY_CLEANUP = $${QMAKE_DEL_FILE} $${DEPLOY_DIR}/StringTheory*.dmg

    DEPLOY_INSTALLER = $${_PRO_FILE_PWD_}/install/mac/create-dmg --volname "StringTheory_Installer" --volicon "$${_PRO_FILE_PWD_}/res/icon.icns"
    DEPLOY_INSTALLER += --background "$${_PRO_FILE_PWD_}/res/mac_install_bg.png" --window-pos 200 120 --window-size 800 400 --icon-size 100 --icon $${TARGET}$${TARGET_CUSTOM_EXT} 200 190 --hide-extension $${TARGET}$${TARGET_CUSTOM_EXT} --app-drop-link 600 185
    DEPLOY_INSTALLER += $${DEPLOY_DIR}/StringTheory_$${VERSION}.dmg $${OUT_PWD}/$${TARGET}$${TARGET_CUSTOM_EXT}
}

CONFIG( release , debug | release) {
    QMAKE_POST_LINK += $${PRE_DEPLOY_COMMAND} $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $${DEPLOY_COMMAND} $${DEPLOY_TARGET} $${DEPLOY_OPT} $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $${DEPLOY_CLEANUP} $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $${DEPLOY_INSTALLER} $$escape_expand(\\n\\t)
}
