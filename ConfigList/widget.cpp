#include "widget.h"
#include "./ui_widget.h"

#include <QColorDialog>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    ui->splitter->setStretchFactor(0,1);
    ui->splitter->setStretchFactor(1,1);
    ui->splitter->setStretchFactor(2,2);

    SchemaProperty cfgp1 = {
        "config.id.1",
        "this is string type config",
        STRING,
        "false"
    };
    SchemaProperty cfgp2 = {
        "config.id.2",
        "this is boolean type config",
        BOOLEAN,
        false
    };
    SchemaProperty cfgp3 = {
        "config.id.3",
        "this is number type config",
        NUMBER,
        123
    };
    SchemaConfiguration cfg1 = {
        "Title_level_1_1",
        {
            {"config.id.1",cfgp1},
            {"config.id.2",cfgp2},
            {"config.id.3",cfgp3}
        }
    };

    SchemaConfiguration cfg2 = {
        "Title_level_1_2",
        {
            {"config.id.1",cfgp1},
            {"config.id.2",cfgp2},
            {"config.id.3",cfgp3}
        },
        {
            {
                "Title_level_2",
                {
                    {"config.id.1",cfgp1},
                    {"config.id.2",cfgp2},
                    {"config.id.3",cfgp3}
                },
                {
                    {
                        "Title_level_3",
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

    mDefaultConfigs << cfg1 << cfg2;

    ui->right->reloadConfigurations(mDefaultConfigs);

    qssHighlighter = new QssHighlighter(this);
    jsonHighlighter = new JsonHighlighter(this);
    cppHighlighter = new CppHighlighter(this);
    cppHighlighter1 = new CppHighlighter(this);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_pushButton_clicked()
{
    QString text = ui->plainTextEdit->toPlainText();
    qApp->setStyleSheet(text);

    // ui->right->adjustSize();
}


void Widget::on_pushButton_2_clicked()
{
    QString text = ui->plainTextEdit_2->toPlainText();

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(text.toLocal8Bit() ,&err);
    if(err.error == QJsonParseError::NoError)
    {
        if(doc.isObject())
        {
            QJsonObject obj = doc.object();
            if(ConfigurationDiaplayer::isValidConfigurationFromJson(obj))
            {
                ui->right->reloadConfigurations(QList<SchemaConfiguration>() << ConfigurationDiaplayer::configurationFromJson(obj));
            }
        }
        else if(doc.isArray())
        {
            QJsonArray arr = doc.array();
            QList<SchemaConfiguration> configs;
            for(auto ele : arr)
            {
                if(ConfigurationDiaplayer::isValidConfigurationFromJson(ele.toObject()))
                {
                    configs << ConfigurationDiaplayer::configurationFromJson(ele.toObject());
                }
            }
            ui->right->reloadConfigurations(configs);
        }
    }
}


void Widget::on_pushButton_3_clicked()
{
    QFile file(":/res/config_example.json");
    if(file.open(QIODevice::ReadOnly))
    {
        QString content = QString(file.readAll());
        ui->plainTextEdit_2->clear();
        ui->plainTextEdit_2->insertPlainText(content);

        jsonHighlighter->setDocument(ui->plainTextEdit_2->document());
        cppHighlighter1->setDocument(nullptr);
        file.close();
    }
}


void Widget::on_pushButton_4_clicked()
{
    QFile file(":/res/style_example.qss");
    if(file.open(QIODevice::ReadOnly))
    {
        QString content = QString(file.readAll());
        ui->plainTextEdit->clear();
        ui->plainTextEdit->insertPlainText(content);

        qssHighlighter->setDocument(ui->plainTextEdit->document());
        cppHighlighter->setDocument(nullptr);
        file.close();
    }
}


void Widget::on_pushButton_5_clicked()
{
    ui->plainTextEdit->clear();
    on_pushButton_clicked();

    ui->plainTextEdit_2->clear();
    ui->right->reloadConfigurations(mDefaultConfigs);

    ui->spinBox->setValue(500);
    ui->checkBox->setChecked(true);
    ui->right->setSearchFilterButtonHide(true);
    ui->label_10->setText(QString("<font style=\"background-color:%1\">%1</font>").arg("#c9c6c8"));
    ui->label_10->setProperty("name", "#c9c6c8");
    ui->label_11->setText(QString("<font style=\"color:%1\">%1</font>").arg("#f9f8f7"));
    ui->label_11->setProperty("name", "#f9f8f7");
    ui->lineEdit_3->setText("无效的输入");
    ui->label_12->setText(QString("<font style=\"background-color:%1\">%1</font>").arg("transparent"));
    ui->label_12->setProperty("name", "transparent");
    ui->label_13->setText(QString("<font style=\"color:%1\">%1</font>").arg("red"));
    ui->label_13->setProperty("name", "red");

    on_pushButton_7_clicked();
}

void Widget::on_right_propertyValueChanged(const SchemaProperty &v , const QString &o, const QString &n)
{
    qDebug() << o << n;
}


void Widget::on_pushButton_6_clicked()
{
    QString text = ui->plainTextEdit_2->toPlainText();
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8(), &err);
    if(err.error != QJsonParseError::NoError)
    {
        return;
    }

    QString newText = doc.toJson(QJsonDocument::Indented);
    ui->plainTextEdit_2->setPlainText(newText);
}


void Widget::on_pushButton_8_clicked()
{
    ui->right->setSearchAutoResponseInterval(ui->spinBox->value());
}


void Widget::on_checkBox_clicked(bool checked)
{
    ui->right->setSearchFilterButtonHide(checked);
}


void Widget::on_pushButton_9_clicked()
{
    QColorDialog dialog;
    dialog.exec();

    QColor color = dialog.currentColor();
    ui->label_10->setText(QString("<font style=\"background-color:%1\">%1</font>").arg(color.name()));
    ui->label_10->setProperty("name", color.name());
}


void Widget::on_pushButton_10_clicked()
{
    ui->right->setSearchHighlightBackgroundColor(ui->label_10->property("name").toString());
}


void Widget::on_pushButton_11_clicked()
{
    QColorDialog dialog;
    dialog.exec();

    QColor color = dialog.currentColor();
    ui->label_11->setText(QString("<font style=\"color:%1\">%1</font>").arg(color.name()));
    ui->label_11->setProperty("name", color.name());
}


void Widget::on_pushButton_12_clicked()
{
    ui->right->setSearchHighlightForegroundColor(ui->label_11->property("name").toString());
}


void Widget::on_pushButton_13_clicked()
{
    ui->right->setInputWarningText(ui->lineEdit_3->text());
}


void Widget::on_pushButton_14_clicked()
{
    QColorDialog dialog;
    dialog.exec();

    QColor color = dialog.currentColor();
    ui->label_12->setText(QString("<font style=\"background-color:%1\">%1</font>").arg(color.name()));
    ui->label_12->setProperty("name", color.name());
}


void Widget::on_pushButton_15_clicked()
{
    ui->right->setInputWarningTextBackgroundColor(ui->label_12->property("name").toString());
}


void Widget::on_pushButton_16_clicked()
{
    QColorDialog dialog;
    dialog.exec();

    QColor color = dialog.currentColor();
    ui->label_13->setText(QString("<font style=\"color:%1\">%1</font>").arg(color.name()));
    ui->label_13->setProperty("name", color.name());
}


void Widget::on_pushButton_17_clicked()
{
    ui->right->setInputWarningTextForegroundColor(ui->label_13->property("name").toString());
}


void Widget::on_pushButton_7_clicked()
{
    on_pushButton_8_clicked();
    on_pushButton_10_clicked();
    on_pushButton_12_clicked();
    on_pushButton_13_clicked();
    on_pushButton_15_clicked();
    on_pushButton_17_clicked();
}


void Widget::on_pushButton_18_clicked()
{
    static int count = 0;
    if(count++ < 5)
    {
        return;
    }
    count = 0;

    QFile file(":/configurationdiaplayer.h");
    if(file.open(QIODevice::ReadOnly))
    {
        QString content = QString(file.readAll());
        file.close();

        jsonHighlighter->setDocument(nullptr);
        cppHighlighter->setDocument(ui->plainTextEdit->document());
        ui->plainTextEdit->setPlainText(content);
    }

    QFile file1(":/configurationdiaplayer.cpp");
    if(file1.open(QIODevice::ReadOnly))
    {
        QString content = QString(file1.readAll());
        file1.close();

        qssHighlighter->setDocument(nullptr);
        cppHighlighter1->setDocument(ui->plainTextEdit_2->document());
        ui->plainTextEdit_2->setPlainText(content);
    }
}

