/*
 * Copyright (C) 2018 Markus Ippy
 *
 * Nissan Consult Communication Protocol for PowerTune
 *
 * This software comes under the GPL (GNU Public License)
 * You may freely copy,distribute etc. this as long as the source code
 * is made available for FREE.
 *
 * No warranty is made or implied. You use this program at your own risk.

  \file Nissanconsult.h
  \brief request and receive messages from Nissan Consult ECU’s
  \author Markus Ippy
 */

/*
Generic Nissan Engine ECU Register Table.
This is a list of all known registers and their function. Any particular ECU will only respond to a sub-set of
these depending on the engine type and configuration. Please report inaccuracies back to the Nissan
Technical egroup.

INITIALISATION 			0xFF 0xFF 0xEF
SELF DIAGNOSTIC 0		xD1
ERASE ERROR CODES 		0xC1
ECU INFO 				0xD0
Data Name 						Byte 		Register No (hex) 			Scaling
CAS Position (RPM) 				MSB 			0x00 					See LSB
CAS Position (RPM) 				LSB 			0x01 					Value * 12.5 (RPM)
CAS Reference (RPM) 			MSB 			0x02 					See LSB
CAS Reference (RPM) 			LSB 			0x03 					Value * 8 (RPM) ??
MAF voltage 					MSB 			0x04 					See LSB
MAF voltage 					LSB 			0x05 					Value * 5 (mV)
RH MAF voltage 					MSB 			0x06 					See LSB
RH MAF voltage 					LSB 			0x07 					Value * 5 (mV)
Coolant temp 					Single byte 	0x08 					Value-50 (deg C)
LH O2 Sensor Voltage 			Single byte 	0x09 					Value * 10 (mV)
RH O2 Sensor Voltage 			Single byte 	0x0a 					Value * 10 (mV)
Vehicle speed 					Single byte 	0x0b 					Value * 2 (kph)
Battery Voltage 				Single Byte 	0x0c 					Value * 80 (mV)
Throttle Position Sensor 		Single Byte 	0x0d 					Value * 20 (mV)
FUEL TEMP SEN 					Single byte 	0x0f 					Value-50 (deg C)
Intake Air Temp 				Single Byte 	0x11 					Value -50 (deg C)
Exhaust Gas Temp 				Single Byte 	0x12 					Value * 20 (mV)
Digital Bit Register 			Single byte 	0x13 					See digital register table
Injection Time (LH) 			MSB 			0x14 					See LSB
Injection Time (LH) 			LSB 			0x15 					Value / 100 (mS)
Ignition Timing 				Single Byte 	0x16 					110 – Value (Deg BTDC)
AAC Valve (Idle Air Valve %) 	Single Byte 	0x17 					Value / 2 (%)
A/F ALPHA-LH 					Single byte     0x1a 					Value (%)
A/F ALPHA-RH 					Single byte 	0x1b 					Value (%)
A/F ALPHA-LH (SELFLEARN) 		Single byte 	0x1c 					Value (%)
A/F ALPHA-RH (SELFLEARN) 		Single byte 	0x1d 					Value (%)
Digital Control Register 		Single byte 	0x1e 					See digital register table
Digital Control Register 		Single byte 	0x1f 					See digital register table
M/R F/C MNT 					Single byte 	0x21 					See digital register table
Injector time (RH) 				MSB 			0x22 					See LSB
Injector time (RH) 				LSB 			0x23 					Value / 100 (mS)
Waste Gate Solenoid % 			Single byte 	0x28
Turbo Boost Sensor, Voltage 	Single byte 	0x29
Engine Mount On/Off 			Single byte 	0x2a
Position Counter 				Single byte		0x2e
Purg. Vol. Control Valve, Step 	Single byte 	0x25
Tank Fuel Temperature, C 		Single byte 	0x26
FPCM DR, Voltage 				Single byte		0x27
Fuel Gauge, Voltage 			Single byte	 	0x2f
FR O2 Heater-B1 				Single byte 	0x30 					Bank 1?
FR O2 Heater-B2 				Single byte 	0x31 					Bank 2?
Ignition Switch 				Single byte 	0x32
CAL/LD Value, % 				Single byte 	0x33
B/Fuel Schedule, mS 			Single byte 	0x34
RR O2 Sensor Voltage 			Single byte 	0x35
RR O2 Sensor-B2 Voltage 		Single byte 	0x36 					Bank 2?
Absolute Throttle Position,		Single byte		0x37
Voltage
MAF gm/S 						Single byte 	0x38
Evap System Pressure, Voltage 	Single byte 	0x39
Absolute Pressure Sensor,		Dual 			0x3a, 0x4a
Voltage
FPCM F/P Voltage 				Dual 			0x52, 0x53
*/
#include "Nissanconsult.h"
#include "dashboard.h"
#include "connect.h"
#include "serialport.h"
#include <QDebug>
#include <QSerialPort>
#include <QSerialPortInfo>
#include "QObject"
#include <QFile>
#include <QTextStream>

Nissanconsult::Nissanconsult(QObject *parent)
    : QObject(parent)
    , m_dashboard(Q_NULLPTR)

{

}
Nissanconsult::Nissanconsult(DashBoard *dashboard, QObject *parent)
    : QObject(parent)
    , m_dashboard(dashboard)
{
}
QByteArray InitECU = (QByteArray::fromHex("FFFFEF"));
QByteArray Liveread;
QByteArray Livereply;
QByteArray Livereplystructure;



int Livedatarequested = 0;
int Stoprequested = 0;
int DTCrequested = 0;
int requestlenght;
int messagestructinit =0;


int CASPosRPMMSB;
int CASPosRPMLSB;
int CASRefRPM;
int MAFvoltage;
int RHMAFvoltage;
int Coolanttemp;
int LHO2SensorVolt;
int RHO2SensorVolt;
int Speed;
int BatteryV;
int ThrottlePos;
int FUELTEMPSEN;
int IntakeAirTemp;
int ExhaustGasTemp;
int DigitalBitRegister;
int InjectionTimeLH;
int IgnitionTiming;
int AACValve;
int AFALPHALH;
int AFALPHARH;
int AFALPHALHSELFLEARN;
int AFALPHARHSELFLEARN;
int Expectedlenght;


//Live Data Request commands


int ECUinitialized = 0;

void Nissanconsult::LiveReqMsg(const int &val1, const int &val2, const int &val3, const int &val4, const int &val5, const int &val6, const int &val7, const int &val8, const int &val9, const int &val10, const int &val11, const int &val12, const int &val13, const int &val14, const int &val15, const int &val16, const int &val17, const int &val18, const int &val19, const int &val20, const int &val21, const int &val22, const int &val23, const int &val24, const int &val25, const int &val26, const int &val27, const int &val28, const int &val29)

{

    // Ensure the Array is cleared first

    Liveread.clear();
    // Build the request message for live Data based on the usser selected Sennsors (Reequest from QML)

    if (val1 == 2) //RPMPosition
    {
        Liveread.append(ConsultData::LiveDataRequest);
        Liveread.append(ConsultData::CASPosRPMMSB);
        Liveread.append(ConsultData::LiveDataRequest);
        Liveread.append(ConsultData::CASPosRPMLSB);
    }

    if (val2 == 2)//RPMReference
    {
        Liveread.append(ConsultData::LiveDataRequest);
        Liveread.append(ConsultData::CASRefRPMMSB);
        Liveread.append(ConsultData::LiveDataRequest);
        Liveread.append(ConsultData::CASRefRPMLSB);
    }

    if (val3 == 2)
    {
        Liveread.append(ConsultData::LiveDataRequest);
        Liveread.append(ConsultData::MAFVoltMSB);
        Liveread.append(ConsultData::LiveDataRequest);
        Liveread.append(ConsultData::MAFVoltLSB);
    }

    if (val4 == 2)
    {
        Liveread.append(ConsultData::LiveDataRequest);
        Liveread.append(ConsultData::RHMAFVoltMSB);
        Liveread.append(ConsultData::LiveDataRequest);
        Liveread.append(ConsultData::RHMAFVoltLSB);
    }

    if (val5 == 2)
    {
        Liveread.append(ConsultData::LiveDataRequest);
        Liveread.append(ConsultData::CoolantTemp);
    }
    if (val6 == 2)
    {
        Liveread.append(ConsultData::LiveDataRequest);
        Liveread.append(ConsultData::LHO2Volt);

    }
    if (val7 == 2)
    {
        Liveread.append(ConsultData::LiveDataRequest);
        Liveread.append(ConsultData::RHO2Volt);
    }
    if (val8 == 2)
    {
        Liveread.append(ConsultData::LiveDataRequest);
        Liveread.append(ConsultData::Speed);
    }
    if (val9 == 2)
    {
        Liveread.append(ConsultData::LiveDataRequest);
        Liveread.append(ConsultData::BattVolt);
    }
    if (val10 == 2)
    {
        Liveread.append(ConsultData::LiveDataRequest);
        Liveread.append(ConsultData::TPS);
    }
    if (val11 == 2)
    {
        Liveread.append(ConsultData::LiveDataRequest);
        Liveread.append(ConsultData::FuelTemp);
    }
    if (val12 == 2)
    {
        Liveread.append(ConsultData::LiveDataRequest);
        Liveread.append(ConsultData::IAT);
    }
    if (val13 == 2)
    {
        Liveread.append(ConsultData::LiveDataRequest);
        Liveread.append(ConsultData::EGT);
    }
    if (val14 == 2)
    {
        Liveread.append(ConsultData::LiveDataRequest);
        Liveread.append(ConsultData::DigitalBitRegister);
    }
    if (val15 == 2)
    {
        Liveread.append(ConsultData::LiveDataRequest);
        Liveread.append(ConsultData::InjectTimeLHMSB);
        Liveread.append(ConsultData::LiveDataRequest);
        Liveread.append(ConsultData::InjectTimeLHLSB);
    }
    if (val16 == 2)
    {
        Liveread.append(ConsultData::LiveDataRequest);
        Liveread.append(ConsultData::IgnitionTiming);
    }
    if (val17 == 2)
    {
        Liveread.append(ConsultData::LiveDataRequest);
        Liveread.append(ConsultData::AACValve);
    }
    if (val18 == 2)
    {
        Liveread.append(ConsultData::LiveDataRequest);
        Liveread.append(ConsultData::AFALPHALH);
    }
    if (val19 == 2)
    {
        Liveread.append(ConsultData::LiveDataRequest);
        Liveread.append(ConsultData::AFALPHARH);
    }
    if (val20 == 2)
    {
        Liveread.append(ConsultData::LiveDataRequest);
        Liveread.append(ConsultData::AFALPHASELFLEARNLH);
    }
    if (val21 == 2)
    {
        Liveread.append(ConsultData::LiveDataRequest);
        Liveread.append(ConsultData::AFALPHASELFLEARNRH);
    }
    if (val22 == 2)
    {
        Liveread.append(ConsultData::LiveDataRequest);
        Liveread.append(ConsultData::DigitalControlReg1);
    }
    if (val23 == 2)
    {
        Liveread.append(ConsultData::LiveDataRequest);
        Liveread.append(ConsultData::DigitalControlReg2);
    }
    if (val24 == 2)
    {
        Liveread.append(ConsultData::LiveDataRequest);
        Liveread.append(ConsultData::MRFCMNT);
    }
    if (val25 == 2)
    {
        Liveread.append(ConsultData::LiveDataRequest);
        Liveread.append(ConsultData::InjecttimeRHMSB);
        Liveread.append(ConsultData::LiveDataRequest);
        Liveread.append(ConsultData::InjecttimeRHLSB);
    }

    if (val26 == 2)
    {
        Liveread.append(ConsultData::LiveDataRequest);
        Liveread.append(ConsultData::WasteGate);
    }
    if (val27 == 2)
    {
        Liveread.append(ConsultData::LiveDataRequest);
        Liveread.append(ConsultData::MAPVolt);
    }
    if (val28 == 2)
    {
        Liveread.append(ConsultData::LiveDataRequest);
        Liveread.append(ConsultData::EngineMount);
    }
    if (val29 == 2)
    {
        Liveread.append(ConsultData::LiveDataRequest);
        Liveread.append(ConsultData::PositionCounter);
    }

    //Terminate Message
    Liveread.append(ConsultData::TerminateMessage);
    QByteArray Livereply = Liveread;
    QByteArray Livereplystructure= Liveread;
    QByteArray sendrequest = (QByteArray::fromHex("5a"));
    QByteArray replyrequest = (QByteArray::fromHex("a5"));


    while (Livereply.contains(sendrequest))
    {
        int readrequest = Livereply.indexOf(sendrequest);
        Livereply.replace(readrequest,1,replyrequest);
        // replaces all 0x5A with 0xA5

    }
    // we determine the structure here
    while (Livereplystructure.contains(sendrequest))
    {
        int readrequest = Livereplystructure.indexOf(sendrequest);
        Livereplystructure.remove(readrequest,1);

    }

    // This will tell the decoder where each message is located in the array
    CASPosRPMMSB        = Livereplystructure.indexOf((QByteArray::fromHex("00")))+2;
    CASPosRPMLSB        = Livereplystructure.indexOf((QByteArray::fromHex("01")))+2;
    CASRefRPM           = Livereplystructure.indexOf((QByteArray::fromHex("02")))+2;
    MAFvoltage          = Livereplystructure.indexOf((QByteArray::fromHex("04")))+2;
    RHMAFvoltage        = Livereplystructure.indexOf((QByteArray::fromHex("06")))+2;
    Coolanttemp         = Livereplystructure.indexOf((QByteArray::fromHex("08")))+2;
    LHO2SensorVolt      = Livereplystructure.indexOf((QByteArray::fromHex("09")))+2;
    RHO2SensorVolt      = Livereplystructure.indexOf((QByteArray::fromHex("0a")))+2;
    Speed               = Livereplystructure.indexOf((QByteArray::fromHex("0b")))+2;
    BatteryV            = Livereplystructure.indexOf((QByteArray::fromHex("0c")))+2;
    ThrottlePos         = Livereplystructure.indexOf((QByteArray::fromHex("0d")))+2;
    FUELTEMPSEN         = Livereplystructure.indexOf((QByteArray::fromHex("0f")))+2;
    IntakeAirTemp       = Livereplystructure.indexOf((QByteArray::fromHex("11")))+2;
    ExhaustGasTemp      = Livereplystructure.indexOf((QByteArray::fromHex("12")))+2;
    DigitalBitRegister  = Livereplystructure.indexOf((QByteArray::fromHex("13")))+2;
    InjectionTimeLH     = Livereplystructure.indexOf((QByteArray::fromHex("14")))+2;
    IgnitionTiming      = Livereplystructure.indexOf((QByteArray::fromHex("16")))+2;
    AACValve            = Livereplystructure.indexOf((QByteArray::fromHex("17")))+2;
    AFALPHALH           = Livereplystructure.indexOf((QByteArray::fromHex("1a")))+2;
    AFALPHARH           = Livereplystructure.indexOf((QByteArray::fromHex("1b")))+2;
    AFALPHALHSELFLEARN  = Livereplystructure.indexOf((QByteArray::fromHex("1c")))+2;
    AFALPHARHSELFLEARN  = Livereplystructure.indexOf((QByteArray::fromHex("1d")))+2;


    requestlenght = Liveread.length() -1 ; // This tells us how long the initial reply will begfore the first start frame
}


void Nissanconsult::initSerialPort()
{
    if (m_serialconsult)
        delete m_serialconsult;
    m_serialconsult = new SerialPort(this);
    connect(this->m_serialconsult,SIGNAL(readyRead()),this,SLOT(readyToRead()));
    connect(&m_DTCtimer, &QTimer::timeout, this, &Nissanconsult::RequestDTC);


}


//function to open serial port
void Nissanconsult::openConnection(const QString &portName)
{

    ECUinitialized = 0;

    initSerialPort();
    m_serialconsult->setPortName(portName);
    m_serialconsult->setBaudRate(QSerialPort::Baud9600);
    m_serialconsult->setParity(QSerialPort::NoParity);
    m_serialconsult->setDataBits(QSerialPort::Data8);
    m_serialconsult->setStopBits(QSerialPort::OneStop);
    m_serialconsult->setFlowControl(QSerialPort::NoFlowControl);;

    if(m_serialconsult->open(QIODevice::ReadWrite) == false)
    {
        m_dashboard->setSerialStat(m_serialconsult->errorString());
        Nissanconsult::closeConnection();
    }
    else
    {
        m_dashboard->setSerialStat(QString("Connected to Serialport"));
        ECUinitialized = 0;
        Nissanconsult::InitECU();
    }


}

void Nissanconsult::closeConnection()

{

      m_serialconsult->close();
      disconnect(this->m_serialconsult,SIGNAL(readyRead()),this,SLOT(readyToRead()));
      disconnect(&m_DTCtimer, &QTimer::timeout, this, &Nissanconsult::RequestDTC);

}


void Nissanconsult::InitECU()

{
    ECUinitialized = 0;
    m_serialconsult->write(QByteArray::fromHex("FFFFEF"));
    // Testing , write all received Raw data to a Text File
        QString fileName = "NissanConsultRaw.txt";
        QFile mFile(fileName);
        if(!mFile.open(QFile::Append | QFile::Text)){
        }
        QTextStream out(&mFile);
        out << "Send Initialisation Request " << "0xFF 0xFF 0xEF"  <<endl;
        mFile.close();
    // Testing End

}
/*
void Nissanconsult::clear() const
{
    m_serialconsult->clear();
}
*/
void Nissanconsult::StopStream()

{
    m_serialconsult->write(QByteArray::fromHex("30"));
    Stoprequested = 1;
    // Testing , write all received Raw data to a Text File
        QString fileName = "NissanConsultRaw.txt";
        QFile mFile(fileName);
        if(!mFile.open(QFile::Append | QFile::Text)){
        }
        QTextStream out(&mFile);
        out << "Send StopStream Request " << "0x30"  <<endl;
        mFile.close();
    // Testing End
}

void Nissanconsult::RequestDTC()

{
    m_DTCtimer.stop();
    DTCrequested = 1;
    Nissanconsult::StopStream();
    Livedatarequested = 0;


}

void Nissanconsult::RequestLiveData()

{
    //m_DTCtimer.start(5000);
    Livedatarequested = 1;
    DTCrequested = 0;
    m_serialconsult->write(Liveread);

    // Testing , write all received Raw data to a Text File
        QString fileName = "NissanConsultRaw.txt";
        QFile mFile(fileName);
        if(!mFile.open(QFile::Append | QFile::Text)){
        }
        QTextStream out(&mFile);
        out << "Send request for Live Stream " << Liveread.toHex()  <<endl;
        mFile.close();
    // Testing End

}
void Nissanconsult::readyToRead()
{

    m_readDataConsult = m_serialconsult->readAll();

// Testing , write all received Raw data to a Text File
    QString fileName = "NissanConsultRaw.txt";
    QFile mFile(fileName);
    if(!mFile.open(QFile::Append | QFile::Text)){
    }
    QTextStream out(&mFile);
    out << m_readDataConsult.toHex()  <<endl;
    mFile.close();
// Testing End

    if (ECUinitialized == 1)
    {
        Nissanconsult::ProcessRawMessage(m_readDataConsult);
    }

    if (ECUinitialized == 0)
    {
        QByteArray init = (QByteArray::fromHex("10"));
        if (m_readDataConsult.contains(init))
        {
            ECUinitialized = 1;
            //qDebug() <<("Initstate ")<< ECUinitialized ;
            m_readDataConsult.clear();
            // Request Live Data Stream Request
            Nissanconsult::RequestLiveData();
        }


    }


}

void Nissanconsult::ProcessRawMessage(const QByteArray &buffer)
{
    m_buffer.append(buffer);

    QByteArray StartFrame = (QByteArray::fromHex("FF"));

  //  qDebug() <<("Current Message")<<m_buffer.toHex();
  //  qDebug() <<("Lenght of message ")<<m_buffer.length();
  //  qDebug() <<("Expected Lenght")<< Expectedlenght;



    if (m_buffer.contains(StartFrame))
    {

        int posstart = m_buffer.indexOf(StartFrame);
        // remove the all leftovers before the start frame
        while (posstart > 1)
        {
            posstart = m_buffer.indexOf(StartFrame);
            m_buffer.remove(0,1);
        }

        if  (posstart == 1)
        {
            Expectedlenght = m_buffer[1]+2; // +2 to include the startframe and lenght byte
        }

    }

    if (m_buffer.length() > Expectedlenght)
    {
        m_consultreply = m_buffer;
        m_consultreply.remove(Expectedlenght,m_buffer.length());
        m_buffer.remove(0,Expectedlenght);
    }

    if (m_buffer.length() == Expectedlenght)
    {
        m_consultreply = m_buffer;
    }


    if (Stoprequested ==1)
    {
        QByteArray Stopbyte = (QByteArray::fromHex("CF"));

        //qDebug() <<("Current Message")<<m_buffer.toHex();

        if (m_buffer.contains(Stopbyte))
        {
            //qDebug() <<("Found Stopbyte");
            m_buffer.clear();
            Stoprequested = 0;

            if (DTCrequested ==1)
            {
                DTCrequested = 0;
                Livedatarequested = 1;
                m_serialconsult->write(QByteArray::fromHex("D1F0"));

            }
            if (Livedatarequested ==1 )
            {

                Nissanconsult::RequestLiveData();;

            }
        }

    }


    if (m_consultreply.length() == Expectedlenght)
    {
/*
        if (DTCrequested ==1)
        {
            DTCrequested = 0;
            Nissanconsult::StopStream();

        }*/
        Nissanconsult::ProcessMessage(m_consultreply);
      //

    }
}



void Nissanconsult::ProcessMessage(QByteArray serialdataconsult)
{
    m_consultreply.clear();

        m_dashboard->setrpm(((serialdataconsult[CASPosRPMMSB]*256.0)+serialdataconsult[CASPosRPMLSB])*12.5);
        m_dashboard->setSpeed(serialdataconsult[Speed] *2);
        m_dashboard->setWatertemp(serialdataconsult[Coolanttemp] -50);
        m_dashboard->setBatteryV(serialdataconsult[BatteryV] * 0.80);
        m_dashboard->setIntaketemp(serialdataconsult[IntakeAirTemp] -50);


    serialdataconsult.clear();


    /*


       AACValve






           int Digital Control Register 		Single byte 	= serialdataconsult.indexOf((QByteArray::fromHex("251e);
           int Digital Control Register 		Single byte 	= serialdataconsult.indexOf((QByteArray::fromHex("251f);
           int M/R F/C MNT 					Single byte 	= serialdataconsult.indexOf((QByteArray::fromHex("2521);


           int WasteGate Solenoid  = serialdataconsult.indexOf((QByteArray::fromHex("2528);
           int BoostSensorVoltage 	= serialdataconsult.indexOf((QByteArray::fromHex("2529);
           int EngineMountOnOff 	= serialdataconsult.indexOf((QByteArray::fromHex("252a);
           int Position Counter 				Single byte		= serialdataconsult.indexOf((QByteArray::fromHex("252e);
           int Purg. Vol. Control Valve, Step 	Single byte 	= serialdataconsult.indexOf((QByteArray::fromHex("2525);
           int Tank Fuel Temperature, C 		Single byte 	= serialdataconsult.indexOf((QByteArray::fromHex("2526);
           int FPCM DR, Voltage 				Single byte		= serialdataconsult.indexOf((QByteArray::fromHex("2527);
           int Fuel Gauge, Voltage 			Single byte	 	= serialdataconsult.indexOf((QByteArray::fromHex("252f);
           int FR O2 Heater-B1 				Single byte 	= serialdataconsult.indexOf((QByteArray::fromHex("2530);  					Bank 1?
           int FR O2 Heater-B2 				Single byte 	= serialdataconsult.indexOf((QByteArray::fromHex("2531);  					Bank 2?
           int Ignition Switch 				Single byte 	= serialdataconsult.indexOf((QByteArray::fromHex("2532);
           int CAL/LD Value, % 				Single byte 	= serialdataconsult.indexOf((QByteArray::fromHex("2533);
           int B/Fuel Schedule, mS 			Single byte 	= serialdataconsult.indexOf((QByteArray::fromHex("2534);
           int RR O2 Sensor Voltage 			Single byte 	= serialdataconsult.indexOf((QByteArray::fromHex("2535);
           int RR O2 Sensor-B2 Voltage 		Single byte 	= serialdataconsult.indexOf((QByteArray::fromHex("2536);  					Bank 2?
           int Absolute Throttle Position,		Single byte		= serialdataconsult.indexOf((QByteArray::fromHex("2537);
           Voltage
           in MAF gm/S 						Single byte 	= serialdataconsult.indexOf((QByteArray::fromHex("2538);
           int int Evap System Pressure, Voltage 	Single byte 	= serialdataconsult.indexOf((QByteArray::fromHex("2539);
           int Absolute Pressure Sensor,		Dual 			= serialdataconsult.indexOf((QByteArray::fromHex("253a);
           int Voltage= serialdataconsult.indexOf((QByteArray::fromHex("254a);
           int FPCM F/P Voltage 				Dual 			= serialdataconsult.indexOf((QByteArray::fromHex("2552);
                                                           = serialdataconsult.indexOf((QByteArray::fromHex("2553);


       */

    //if(requesttypeconsult == 0x2E){m_decodernissanconsult->decodeDTCConsult(serialdataconsult);}
    //if(requesttypeconsult == 0x2E){Nissanconsult::StopStream();}
    //


}
