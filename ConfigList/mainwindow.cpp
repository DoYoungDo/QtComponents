#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "form.h"
#include "notifiercontainer.h"
#include "configurationdiaplayer.h"
#include <QBoxLayout>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    SchemaProperty cfgp1 = {
        "config.id.1",
        "this is config first",
        STRING,
        "false"
    };
    SchemaProperty cfgp2 = {
        "config.id.2",
        "this is config second",
        BOOLEAN,
        false
    };
    SchemaProperty cfgp3 = {
        "config.id.3",
        "this is config third",
        NUMBER,
        false
    };
    SchemaConfiguration cfg1 = {
        "常用设置",
        {
            {"config.id.1",cfgp1},
            {"config.id.2",cfgp2},
            {"config.id.3",cfgp3}
        }
    };

    SchemaConfiguration cfg2 = {
        "编辑器设置",
        {
            {"config.id.1",cfgp1},
            {"config.id.2",cfgp2},
            {"config.id.3",cfgp3}
        },
        {
            {
                "光标",
                {
                    {"config.id.1",cfgp1},
                    {"config.id.2",cfgp2},
                    {"config.id.3",cfgp3}
                },
                {
                    {
                        "子光标",
                        {
                            {"config.id.1",cfgp1},
                            {"config.id.2",cfgp2},
                            {"config.id.3",cfgp3}
                        }
                    }
                }
            }
        }
    };

    QList<SchemaConfiguration> cfgs = QList<SchemaConfiguration>() << cfg1 << cfg2;

    ConfigurationDiaplayer* cd = new ConfigurationDiaplayer(cfgs);
    ui->centralwidget->layout()->addWidget(cd);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    Form* form = new Form(this);
    this->installEventFilter(form);
    form->show();
}

