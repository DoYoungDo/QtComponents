#include "mainwindow.h"
#include "appinit.h"

#include <QApplication>


static SimpleAutoReg<APP_LAUNCHED> Launch([](){
    qDebug() << "on app launch";
});

static SimpleAutoReg<APP_READY_EXIT, bool> appExit([](){
    qDebug() << "on app exit";
    return false;
});
static SimpleAutoReg<APP_READY_EXIT, bool> appExit2([](){
    qDebug() << "on app2 exit";
    return true;
});

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SimpleInvoker<APP_LAUNCHED, void>::invoke();

    SimpleInvoker<BEROE_CREATE_WINDOW, void>::invoke();
    MainWindow w;
    SimpleInvoker<AFTER_CREATE_WINDOW, void, MainWindow*>::invoke(&w);
    SimpleInvoker<BEROE_SHOW_WINDOW, void, MainWindow*>::invoke(&w);
    w.show();
    SimpleInvoker<AFTER_SHOW_WINDOW, void, MainWindow*>::invoke(&w);

    bool stop = SimpleInvoker<APP_READY_EXIT, bool>::invokeUntil([](bool is){
        return !is;
    });

    qDebug() << stop;
    if(!stop)
    {
        return 0;
    }

    return a.exec();
}

