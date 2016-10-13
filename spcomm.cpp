#include "spcomm.h"
#include "mainwindow.h"
#include <QMessageBox>

SPComm::SPComm(QObject *object)
{
    mainWindow = (MainWindow *)object;
    serialPort = new QSerialPort();
    flyParams = FlyParams::getFlyParams();

    baudRate = 115200;
    protName = "";
    dataBits = QSerialPort::Data8;

    isHexRec = false;
    isHexSend = false;

    connect(serialPort, SIGNAL(readyRead()), this, SLOT(readData()));
    connect(serialPort, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(portError(QSerialPort::SerialPortError)));
}

SPComm::~SPComm()
{
    flyParams = NULL;
    if(serialPort->isOpen())
        serialPort->close();

    delete serialPort;

    serialPort = NULL;
}

/* 获取所有可用串口的集合 */
QStringList SPComm::getAllPorts()
{
    comNames.clear();
    comNames << "";
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        comNames << info.portName();
    }
    return comNames;
}

/* 判断当前串口是否打开 */
bool SPComm::isOpen() const
{
    return serialPort->isOpen();
}

/* 获取当前串口的名称 */
QString SPComm::getPortName() const
{
    return protName;
}

/* 设置串口名称 */
void SPComm::setPortName(QString portName)
{
    this->protName = portName;
}

/* 获取当前设置的波特率 */
int SPComm::getBaudRate() const
{
    return baudRate;
}

/* 设置当前波特率 */
void SPComm::setBaudRate(int baudRate)
{
    this->baudRate = baudRate;
}

/* 获取设置的数据位 */
int SPComm::getDataBits() const
{
    return dataBits;
}

/* 设置数据位 */
void SPComm::setDataBits(int dataBits)
{
    this->dataBits = dataBits;
}


/* 打开串口 */
bool SPComm::openPort()
{
    if(serialPort->isOpen())
    {
        return true;
    }
    serialPort->setPortName(protName);                          // 设置串口名称
    serialPort->setBaudRate(calculateBaudRate(baudRate));       // 设置波特率
    serialPort->setParity(QSerialPort::NoParity);               // 设置奇偶校验
    serialPort->setDataBits(calculateDataBits(dataBits));       // 设置数据位
    serialPort->setStopBits(QSerialPort::OneStop);              // 设置停止位
    serialPort->setFlowControl(QSerialPort::NoFlowControl);     // 设置流量控制
    serialPort->setReadBufferSize(1024);                        // 设置读缓冲大小
    return serialPort->open(QSerialPort::ReadWrite);            // 以读写方式打开串口
}

/* 关闭串口 */
void SPComm::closePort()
{
    if(serialPort->isOpen())
    {
        serialPort->close();
    }
}

/* 重启串口，清除数据 */
bool SPComm::clearPort()
{
    if(serialPort->isOpen())
    {
        serialPort->clear();
        this->closePort();
        return this->openPort();
    }
    return false;
}

/* 向串口写数据 */
void SPComm::writeData(const QString &string)
{
    if(serialPort->isOpen() && !string.isEmpty())
    {
        if(isHexSend)
            serialPort->write(string.toLocal8Bit().toHex());
        else
            serialPort->write(string.toLocal8Bit());
    }
}

/* 发送整形数据 */
void SPComm::sendIntValue(int value)
{
    byteArr[0] = value;
    serialPort->write(byteArr);
}

/* 设置是否以十六进制接收 */
void SPComm::setHexRec(bool isHex)
{
    isHexRec = isHex;
}

/* 设置是否以十六进制发送 */
void SPComm::setHexSend(bool isHex)
{
    isHexSend = isHex;
}

/* 获取 serialPort 对象 */
QSerialPort *SPComm::getSerialPort()
{
    return serialPort;
}

/* 计算波特率，打开串口时用到 */
QSerialPort::BaudRate SPComm::calculateBaudRate(int baudRate)
{
    switch(baudRate)
    {
    case 1200:
        return QSerialPort::Baud1200;
    case 2400:
        return QSerialPort::Baud2400;
    case 4800:
        return QSerialPort::Baud4800;
    case 9600:
        return QSerialPort::Baud9600;
    case 19200:
        return QSerialPort::Baud19200;
    case 38400:
        return QSerialPort::Baud38400;
    case 57600:
        return QSerialPort::Baud57600;
    case 115200:
        return QSerialPort::Baud115200;
    default:
        return QSerialPort::UnknownBaud;
    }
}

/* 计算数据位 */
QSerialPort::DataBits SPComm::calculateDataBits(int dataBits)
{
    switch(dataBits)
    {
    case 5:
        return QSerialPort::Data5;
    case 6:
        return QSerialPort::Data6;
    case 7:
        return QSerialPort::Data7;
    case 8:
        return QSerialPort::Data8;
    default:
        return QSerialPort::UnknownDataBits;
    }
}

/* 串口接收到数据 */
void SPComm::readData()
{
    QByteArray data = serialPort->readAll();

    bool isSuccess;
    QString flyStateStr;
    qlonglong comData = QString::fromLocal8Bit(data.toHex()).toLongLong(&isSuccess, 16);
    int comBit0 = (comData & 0xff000000) >> 24;
    int comBit1 = (comData & 0x00ff0000) >> 16;
    int comBit2 = (comData & 0x0000ff00) >> 8;
    int comBit3 = comData & 0x000000ff;

    if(isSuccess
    && comBit0 == UART_HEAD_COM
    && (comBit3 == UART_FLY_COM || comBit3 == UART_POWER_COM || comBit3 == UART_ATTITUDE_COM))
    {
        int comData = comBit1*100 + comBit2;
        if(comData < 14
        && (flyParams->getFlyControl() == BT_FLY_FRONT ||
            flyParams->getFlyControl() == BT_FLY_BACK ||
            flyParams->getFlyControl() == BT_FLY_LEFT ||
            flyParams->getFlyControl() == BT_FLY_RIGHT))
            sendIntValue(BT_FLY_CENTER);
        else
            sendIntValue(flyParams->getFlyControl());

        if(flyParams->getFlyControl() == BT_FLY_UNLOCK
        || flyParams->getFlyControl() == BT_FLY_LOCK
        || flyParams->getFlyControl() == BT_ATTITUDE_ADD
        || flyParams->getFlyControl() == BT_ATTITUDE_DOWN
        || flyParams->getFlyControl() == BT_FLY_STOP
        || flyParams->getFlyControl() == BT_THROW_BOLL
        || flyParams->getFlyControl() == BT_FLY_ATTITUDE
        || flyParams->getFlyControl() == BT_FLY_POWER_MODE
        || flyParams->getFlyControl() == BT_FLY_SWAVE_MODE)
            flyParams->setFlyControl(BT_FLY_CENTER);

        if(comBit3 == UART_POWER_COM)
            flyStateStr = QString("当前油门值：%1").arg(comData);
        else if(comBit3 == UART_ATTITUDE_COM)
            flyStateStr = QString("当前高度值：%1 CM").arg(comData);
        else if(comBit3 == UART_FLY_COM && comData == FLY_LOCKED)
            flyStateStr = QString("\351\243\236\346\216\247\346\234\252\350\247\243\351\224\201");
        else if(comBit3 == UART_FLY_COM && comData == FLY_STOP)
            flyStateStr = QString("\345\201\234\346\255\242\347\212\266\346\200\201");
        else if(comBit3 == UART_FLY_COM && comData == FLY_CENTER)
            flyStateStr = QString("\346\202\254\345\201\234\347\212\266\346\200\201");
        else if(comBit3 == UART_FLY_COM && comData == FLY_FRONT)
            flyStateStr = QString("\345\220\221\345\211\215\351\243\236");
        else if(comBit3 == UART_FLY_COM && comData == FLY_BACK)
            flyStateStr = QString("\345\220\221\345\220\216\351\243\236");
        else if(comBit3 == UART_FLY_COM && comData == FLY_LEFT)
            flyStateStr = QString("\345\220\221\345\267\246\351\243\236");
        else if(comBit3 == UART_FLY_COM && comData == FLY_RIGHT)
            flyStateStr = QString("\345\220\221\345\217\263\351\243\236");
        else if(comBit3 == UART_FLY_COM && comData == FLY_LEFT_FRONT)
            flyStateStr = QString("\345\220\221\345\267\246\345\211\215\351\243\236");
        else if(comBit3 == UART_FLY_COM && comData == FLY_LEFT_BACK)
            flyStateStr = QString("\345\220\221\345\267\246\345\220\216\351\243\236");
        else if(comBit3 == UART_FLY_COM && comData == FLY_RIGHT_FRONT)
            flyStateStr = QString("\345\220\221\345\217\263\345\211\215\351\243\236");
        else if(comBit3 == UART_FLY_COM && comData == FLY_RIGHT_BACK)
            flyStateStr = QString("\345\220\221\345\217\263\345\220\216\351\243\236");
        else if(comBit3 == UART_FLY_COM && comData == FLY_PITCH_CENTER)
            flyStateStr = QString("\344\277\257\344\273\260\346\226\271\345\220\221\346\202\254\345\201\234");
        else if(comBit3 == UART_FLY_COM && comData == FLY_ROLL_CENTER)
            flyStateStr = QString("\346\250\252\346\273\232\346\226\271\345\220\221\346\202\254\345\201\234");
        else;
    }
    if(mainWindow->setFlyState(flyStateStr))
        return;

    if(isHexRec)
        mainWindow->updateRecData(data.toHex());
    else
        mainWindow->updateRecData(QString::fromLocal8Bit(data));
}

/* 串口出错 */
void SPComm::portError(QSerialPort::SerialPortError error)
{
    if(error == QSerialPort::ResourceError && serialPort->isOpen())
    {
        serialPort->close();
        mainWindow->changePortUi(true);
        mainWindow->setPortCBox(0);
        QMessageBox::information(mainWindow, "警告", "连接被强制断开");
    }
}
