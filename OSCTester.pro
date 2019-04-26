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

TARGET_CUSTOM_EXT = .exe
DEPLOY_DIR = $$shell_quote($$system_path($${_PRO_FILE_PWD_}/install/deploy))
DEPLOY_TARGET = $$shell_quote($$system_path($${DESTDIR}/$${TARGET}$${TARGET_CUSTOM_EXT}))

PRE_DEPLOY_COMMAND =  $$sprintf($${QMAKE_MKDIR_CMD}, $$shell_path($${DEPLOY_DIR})) $$escape_expand(\\n\\t)
PRE_DEPLOY_COMMAND += $$QMAKE_DEL_FILE $${DEPLOY_DIR}\*.* /S /Q $$escape_expand(\\n\\t)
PRE_DEPLOY_COMMAND += $$QMAKE_COPY $${DEPLOY_TARGET} $${DEPLOY_DIR} $$escape_expand(\\n\\t)
DEPLOY_COMMAND = windeployqt
DEPLOY_OPT = --dir $${DEPLOY_DIR}
DEPLOY_INSTALLER = makensis /DPRODUCT_VERSION="$${GIT_TAG}" $$shell_quote($$system_path($${_PRO_FILE_PWD_}/install/install.nsi))

win32 {
    QMAKE_CXXFLAGS += /Zi
    QMAKE_LFLAGS += /INCREMENTAL:NO /Debug
}

CONFIG(release, debug|release) {
    QMAKE_POST_LINK += $${PRE_DEPLOY_COMMAND} $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $${DEPLOY_COMMAND} $${DEPLOY_TARGET} $${DEPLOY_OPT} $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $${DEPLOY_CLEANUP} $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $${DEPLOY_INSTALLER} $$escape_expand(\\n\\t)
}
