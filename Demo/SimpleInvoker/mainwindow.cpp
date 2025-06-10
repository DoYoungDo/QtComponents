#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "appinit.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


static SimpleAutoReg<BEROE_CREATE_WINDOW> windowCreate([](){
    qDebug() << "on window create";
});

static SimpleAutoReg<AFTER_CREATE_WINDOW, MainWindow*> windowCreated([](MainWindow* w){
    qDebug() << "on window created";
});

static SimpleAutoReg<BEROE_SHOW_WINDOW, MainWindow*> windowShow([](MainWindow* w){
    qDebug() << "on window show";
});

static SimpleAutoReg<AFTER_SHOW_WINDOW, MainWindow*> windowShowed([](MainWindow* w){
    qDebug() << "on window showed";
});
