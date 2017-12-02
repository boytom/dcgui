#-------------------------------------------------
#
# Project created by QtCreator 2014-01-06T21:12:42
#
#-------------------------------------------------

QT       += core gui svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = dcgui
TEMPLATE = app
#-fwide-exec-charset=utf-32Ҳ���ÿ��ַ�������BOM��Linux��
unix:QMAKE_CXXFLAGS += -std=gnu++14 -finput-charset=gb18030 -fexec-charset=utf-8  #-municode -DUNICODE -D_UNICODE
unix:QMAKE_LFLAGS += -Wl,-rpath,/home/whg/soft/gcc/lib64
win32: {

QMAKE_CXXFLAGS_DEBUG +=-D_DEBUG #microsoft crt debug need this macro being defined
QMAKE_CFLAGS_DEBUG +=-D_DEBUG #microsoft crt debug need this macro being defined

#MSDN https://msdn.microsoft.com/en-us/library/mt708818.aspx
#MSDN ˵��ͦ�ã�/execution-charset֧�� gb18030��������֧�� utf-16
#MSDN ˵��ͦ�ã�/source-charsetֻ֧�� gb2312��������֧�� gb18030
#https://docs.microsoft.com/en-us/cpp/build/reference/source-charset-set-source-character-set
QMAKE_CXXFLAGS += -DUNICODE -D_UNICODE -Zc:wchar_t -execution-charset:gb18030 -source-charset:gb2312
#QMAKE_CXXFLAGS += -DUNICODE -D_UNICODE -Zc:wchar_t -execution-charset:utf-8 -source-charset:gb2312
QMAKE_LFLAGS +=/ENTRY:"wmainCRTStartup"
QMAKE_LIBS += user32.lib gdi32.lib
INCLUDEPATH=include
RC_FILE=icon-dcgui.rc
}

SOURCES += main.cpp\
        mainwindow.cpp \
    comparethread.cpp \
    import.cc \
    xaspectratiopixmaplabel.cc \
    xitemdialog.cc \
    itemhelper.cc \
    charicon.cc \
    xwindowdragger.cc \
    xaboutdialog.cc \
    xpaydialog.cc

HEADERS  += mainwindow.h \
    comparethread.h \
    vc_c_api.h \
    import.h \
    xaspectratiopixmaplabel.h \
    xitemdialog.h \
    itemhelper.h \
    charicon.h \
    xwindowdragger.h \
    xaboutdialog.h \
    xpaydialog.h

FORMS    += mainwindow.ui

RESOURCES += \
    dcgui.qrc