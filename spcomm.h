#ifndef SPCOMM_H
#define SPCOMM_H

#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QStringList>
#include <QByteArray>
#include "flyparams.h"

class MainWindow;

class SPComm : public QObject
{
    Q_OBJECT
public:
    explicit SPComm(QObject *object);
    ~SPComm();

    QStringList getAllPorts();                                  // 获取所有可用串口的的集合
    bool isOpen() const;                                        // 判断当前串口是否打开
    QString getPortName() const;                                // 获取当前串口名称
    void setPortName(QString portName);                         // 设置当前串口名称
    int getBaudRate() const;                                    // 获取当前波特率
    void setBaudRate(int baudRate);                             // 设置波特率
    int getDataBits() const;                                    // 获取设置的数据位
    void setDataBits(int dataBits);                             // 设置数据位
    virtual bool openPort();                                    // 打开串口
    virtual void closePort();                                   // 关闭串口
    virtual bool clearPort();                                   // 重启串口，清除数据

    void writeData(const QString &string);                      // 向串口写数据
    void sendIntValue(int value);                               // 发送整形数据

    void setHexRec(bool isHex);                                 // 设置是否以十六进制接收
    void setHexSend(bool isHex);                                // 设置是否以十六进制发送

    QSerialPort *getSerialPort();

private:
    FlyParams *flyParams;
    MainWindow *mainWindow;
    QSerialPort *serialPort;                                    // 代表串口对象
    QStringList comNames;                                       // 所有可用串口名称的集合
    QString protName;                                           // 代表当前串口名称
    int baudRate;                                               // 代表当前选择的波特率
    int dataBits;                                               // 代表当前选择的数据位

    QByteArray byteArr;

    bool isHexRec;                                              // 是否以十六进制接收
    bool isHexSend;                                             // 是否以十六进制发送

    QSerialPort::BaudRate calculateBaudRate(int baudRate);      // 计算波特率
    QSerialPort::DataBits calculateDataBits(int dataBits);      // 计算数据位

signals:

private slots:
    void readData();                                            // 串口接收到数据
    void portError(QSerialPort::SerialPortError error);         // 串口出错
};

#endif // SPCOMM_H
