#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPort>
#include <QDebug>
#include <cmath>
#include <QString>

QSerialPort *serialPort;

MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    serialPort = new QSerialPort(this);
    serialPort->setPortName("COM4");
    serialPort->setBaudRate(QSerialPort::Baud9600);
    serialPort->setDataBits(QSerialPort::Data8);
    serialPort->setParity(QSerialPort::NoParity);
    serialPort->setStopBits(QSerialPort::OneStop);
    serialPort->setFlowControl(QSerialPort::SoftwareControl);
    serialPort->open(QIODevice::ReadWrite);

    connect(serialPort, SIGNAL(readyRead()), this, SLOT(serialReceived()));                         // sender, value changed, receiver, new value
    connect(ui->fanSpeedSendButton, SIGNAL (released()), this, SLOT (serialSend()));                //connect Send Button to serialSend function

    connect(ui->percentSlider, SIGNAL (valueChanged(int)), this, SLOT (updateFanSpeedText(int)));   //Connect slider to text edit function
    ui->currentPercent->setText(QString::number(ui->percentSlider->value()));                       //Initially set slider text to slider Position
}

MainWindow::~MainWindow() //Destructor function
{
    delete ui;
    serialPort->close();
}

QByteArray ba;

void MainWindow::serialReceived(){

        ba.append(serialPort->readAll());

        qDebug() << "Message: " << ba;
        qDebug() << "Characters: " << ba.length();

        if(!ba.contains("\r\n"))
        {
            return;
        }

        ui->label->setText(ba);
        ba = "";
}

void MainWindow::serialSend(){

    qDebug() << "Pressed";

    int percentOn = ui->percentSlider->value();
    qDebug() << "DEBUG: SliderValue: " << ui->percentSlider->value();
    qDebug() << "DEBUG: PercentOn: " << percentOn;

    double PWMduty = (percentOn/100.0) * 255.0;
    qDebug() << "DEBUG: PWMduty " << PWMduty;
    PWMduty = round(PWMduty);
    qDebug() << "DEBUG: PWMduty Rounded: " << PWMduty;

    QString PWMdutyString = QString::number(PWMduty);
    qDebug() << "DEBUG: PWMdutyString: " << PWMdutyString;
    int lengthOfDuty = PWMdutyString.length();

    qDebug() << "DEBUG: length of duty: " << lengthOfDuty;

    switch(lengthOfDuty) {
       case 1 :
            //prepend 2 0s
            PWMdutyString.insert(0, "00");
            break;
       case 2 :
            //prepend 1 0s
            PWMdutyString.insert(0, "0");
       case 3 :
            break;
    }

    qDebug() << "DEBUG: PWMdutyString: " << PWMdutyString;

    QByteArray array = PWMdutyString.toLocal8Bit();
    char* buffer = array.data();

    serialPort->write(buffer);
}

void MainWindow::updateFanSpeedText(int percentOn){

    ui->currentPercent->setText(QString::number(percentOn));

}
