
/*
 * Copyright (C) 2016 Bastian Gschrey & Markus Ippy
 *
 * Digital Gauges for Apexi Power FC for RX7 on Raspberry Pi
 *
 *
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 *
 * No warranty is made or implied. You use this program at your own risk.
 */

/*!
  \file serial.cpp
  \brief Raspexi Viewer Power FC related functions
  \author Suriyan Laohaprapanon & Jacob Donley & Bastian Gschrey
 */


#include "serial.h"
#include <QDebug>

Serial::Serial(QObject *parent) : QObject(parent)
{
serialport = new QSerialPort(this);
//connect(serial,SIGNAL(advRequested()),this,SLOT(advRequested()));
connect(this->serialport,SIGNAL(readyRead()),this,SLOT(readyToRead()));


}

// FD3S map
static QString map[] = {"RPM", "Intakepress", "PressureV",
                       "ThrottleV", "Primaryinp", "Fuelc", "Leadingign", "Trailingign",
                       "Fueltemp", "Moilp", "Boosttp", "Boostwg", "Watertemp", "Intaketemp", "Knock", "BatteryV",
                       "Speed", "Iscvduty", "O2volt", "na1", "Secinjpulse",
                       "na2",
                       "AUX1", "AUX2", "AUX3", "AUX4", "AUX5", "AUX6", "AUX7", "AUX8",
                       "Analog1", "Analog2", "Analog3", "Analog4",
                       "Power", "Accel", "GForce", "ForceN", "Gear", "PrimaryInjD", "AccelTimer",
                       "Rec" };

//static double rtv[MAP_ELEMENTS];

QByteArray Serial::read() const
{
    return serialport->readAll();
}

//function to open serial port
    void Serial::openConnection(SerialSetting::Settings p)
{
qDebug() << "Enter openConnection function";
serialport->setBaudRate(QSerialPort::Baud57600);
serialport->setPortName(p.portName);
serialport->setParity(serialport->EvenParity);

serialport->setDataBits(QSerialPort::Data8);
serialport->setStopBits(QSerialPort::OneStop);
serialport->setFlowControl(QSerialPort::NoFlowControl);
qDebug() << "Try to open SerialPort:";
if(serialport->open(QIODevice::ReadWrite) == false)
{
qDebug() << serialport->errorString();
}
qDebug() << "Portname: " << p.portName;

Serial::getAdvData();
}

void Serial::closeConnection()
{
serialport->close();
}

void Serial::getAdvData()
{
    qDebug() << "Enter getAdvData function";
    serialport->write(QByteArray::fromHex("F0020D"));
    emit advRequested();
}

void Serial::readyToRead()
{
    qDebug() <<"Signal readyRead fired be QSerialPort.";
    emit readyRead();
}
