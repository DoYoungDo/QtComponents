#include "mainwindow.h"
#include "appinit.h"

#include <QApplication>


static SimpleAutoReg<APP_LAUNCHED> Launch([](){
    qDebug() << "on app launch";
});

static SimpleAutoReg<APP_READY_EXIT> appExit([](){
    qDebug() << "on app exit";
});

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SimpleInvoker<APP_LAUNCHED>::invoke();

    SimpleInvoker<BEROE_CREATE_WINDOW>::invoke();
    MainWindow w;
    SimpleInvoker<AFTER_CREATE_WINDOW, MainWindow*>::invoke(&w);
    SimpleInvoker<BEROE_SHOW_WINDOW, MainWindow*>::invoke(&w);
    w.show();
    SimpleInvoker<AFTER_SHOW_WINDOW, MainWindow*>::invoke(&w);

    SimpleInvoker<APP_READY_EXIT>::invoke();
    return a.exec();
}

