QT += \
    core \
    gui \
    widgets

TARGET = Haralug

TEMPLATE = app

DEFINES += \
    QT_DEPRECATED_WARNINGS

SOURCES += \
    main.cpp \
    Crypto.cpp \
    MainWindow.cpp \
    TaskManager.cpp \
    Settings.cpp \
    ThreadPool.cpp \
    Utils.cpp \
    PasswordDialog.cpp \
    PasswordGenerator.cpp \
    GeneratePasswordDialog.cpp \
    TaskTableModel.cpp \
    AboutDialog.cpp \
    TaskProgressItemDelegate.cpp

HEADERS += \
    Crypto.h \
    MainWindow.h \
    TaskManager.h \
    Settings.h \
    ThreadPool.h \
    Utils.h \
    PasswordDialog.h \
    PasswordGenerator.h \
    GeneratePasswordDialog.h \
    TaskTableModel.h \
    AboutDialog.h \
    TaskProgressItemDelegate.h

FORMS += \
    MainWindow.ui \
    PasswordDialog.ui \
    GeneratePasswordDialog.ui \
    AboutDialog.ui

RESOURCES += \
    resources.qrc

LIBS += \
    -lcrypto

win32 {
    RC_FILE = Haralug.rc
}
