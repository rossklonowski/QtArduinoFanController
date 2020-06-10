#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPort>
#include <QDebug>
#include <cmath>
#include <QString>
#include <QTextCodec>

QSerialPort *serialPort;
bool debug = true;

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

void MainWindow::splitSpeed(QByteArray speedFromArduino){

    speedFromArduino.replace('\r', ""); // Remove Carraige Return
    speedFromArduino.replace('\n', ""); // Remove New Line Character return

    QByteArray speedometerText = speedFromArduino; // Speedometer is an integer ONLY
    int digits = speedometerText.length();         // Get Count

    switch(digits) {
        case 1 :
            speedometerText.prepend("000");
            ui->speed0->setStyleSheet("color : gray;");
            ui->speed1->setStyleSheet("color : gray;");
            ui->speed2->setStyleSheet("color : gray;");
            ui->speed3->setStyleSheet("color : white;");
            break;
        case 2 :
            speedometerText.prepend("00");
            ui->speed0->setStyleSheet("color : gray;");
            ui->speed1->setStyleSheet("color : gray;");
            ui->speed2->setStyleSheet("color : white;");
            ui->speed3->setStyleSheet("color : white;");
            break;
        case 3 :
            speedometerText.prepend("0");
            ui->speed0->setStyleSheet("color : gray;");
            ui->speed1->setStyleSheet("color : white;");
            ui->speed2->setStyleSheet("color : white;");
            ui->speed3->setStyleSheet("color : white;");
            break;
        case 4 :
            ui->speed0->setStyleSheet("color : white;");
            ui->speed1->setStyleSheet("color : white;");
            ui->speed2->setStyleSheet("color : white;");
            ui->speed3->setStyleSheet("color : white;");
            break;
    }

    const QString speed(speedometerText);   // Convert to qstring for setText function

    ui->speed0->setText(speed[0]);  // Set left most digit of speedometer reading
    ui->speed1->setText(speed[1]);
    ui->speed2->setText(speed[2]);
    ui->speed3->setText(speed[3]);  // Set right most digit of speedometer reading

    if (debug) {
        qDebug() << "DEBUG: Qbytearray: " << speedometerText;
        qDebug() << "DEBUG: 0: " << speedometerText[0];
        qDebug() << "DEBUG: 1: " << speedometerText[1];
        qDebug() << "DEBUG: 3: " << speedometerText[2];
        qDebug() << "DEBUG: 2: " << speedometerText[3];
    }
}

QByteArray ba;

void MainWindow::serialReceived(){

        ba.append(serialPort->readAll());

        if (debug){
            qDebug() << "DEBUG: Message: " << ba;
            qDebug() << "DEBUG: Message length: " << ba.length();
        }

        if (!ba.contains("\r\n"))
        {
            return;
        }

        splitSpeed(ba);

        ba = "";
}

void MainWindow::serialSend(){

    if (debug) qDebug() << "DEBUG: Pressed";

    int percentOn = ui->percentSlider->value();
    if (debug) qDebug() << "DEBUG: SliderValue: " << ui->percentSlider->value();
    if (debug) qDebug() << "DEBUG: PercentOn: " << percentOn;

    double PWMduty = (percentOn/100.0) * 255.0;
    if (debug) qDebug() << "DEBUG: PWMduty " << PWMduty;
    PWMduty = round(PWMduty);
    if (debug) qDebug() << "DEBUG: PWMduty Rounded: " << PWMduty;

    QString PWMdutyString = QString::number(PWMduty);
    if (debug) qDebug() << "DEBUG: PWMdutyString: " << PWMdutyString;
    int lengthOfDuty = PWMdutyString.length();

    if (debug) qDebug() << "DEBUG: length of duty: " << lengthOfDuty;

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

    if (debug) qDebug() << "DEBUG: PWMdutyString: " << PWMdutyString;

    QByteArray array = PWMdutyString.toLocal8Bit();
    char* buffer = array.data();

    serialPort->write(buffer);
}

void MainWindow::updateFanSpeedText(int percentOn){

    ui->currentPercent->setText(QString::number(percentOn));

}
