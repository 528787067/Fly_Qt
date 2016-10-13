#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QStringList>
#include <QMessageBox>
#include <QFont>
#include "spcomm.h"
#include "imgthread.h"
#include "flyparams.h"

#define TIMER_DELAY         1000

/* 红框 HSV 参数 */
#define RED_BOX_H_MIN       0
#define RED_BOX_H_MAX       10
#define RED_BOX_S_MIN       0
#define RED_BOX_S_MAX       256
#define RED_BOX_V_MIN       0
#define RED_BOX_V_MAX       256

/* 黄框 HSV 参数 */
#define YELLOW_BOX_H_MIN    25
#define YELLOW_BOX_H_MAX    35
#define YELLOW_BOX_S_MIN    0
#define YELLOW_BOX_S_MAX    256
#define YELLOW_BOX_V_MIN    0
#define YELLOW_BOX_V_MAX    256

/* 蓝框 HSV 参数 */
#define BLUE_BOX_H_MIN      115
#define BLUE_BOX_H_MAX      125
#define BLUE_BOX_S_MIN      0
#define BLUE_BOX_S_MAX      256
#define BLUE_BOX_V_MIN      0
#define BLUE_BOX_V_MAX      256

/* 起始点（青） HSV 参数 */
#define START_POINT_H_MIN   85
#define START_POINT_H_MAX   95
#define START_POINT_S_MIN   0
#define START_POINT_S_MAX   256
#define START_POINT_V_MIN   0
#define START_POINT_V_MAX   256

/* 飞行器（绿） HSV 参数 */
#define FLY_POINT_H_MIN     55
#define FLY_POINT_H_MAX     65
#define FLY_POINT_S_MIN     0
#define FLY_POINT_S_MAX     256
#define FLY_POINT_V_MIN     0
#define FLY_POINT_V_MAX     256

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void changePortUi(bool isOpen);
    void changeCameraUi(bool isOpen);
    void setPortCBox(int index);
    void updateRecData(const QString &string);
    void setSelectPosition(SelectType type, int x, int y);
    void setOpenCVImg(QPixmap &img);
    void setHistogram(QPixmap &img);
    bool setFlyState(QString &str);

private:
    Ui::MainWindow *ui;
    QTimer *timer;
    SPComm *spComm;
    ImgThread *imgThread;
    FlyParams *flyParams;

    void initView();                        // 初始化控件

private slots:
    void portCBoxSlot();                    // 串口选择槽
    void baudCBoxSlot();                    // 波特率选择槽
    void dataBitsCBoxSlot();                // 数据位选择槽
    void switchBnSlot();                    // 串口打开按钮槽

    void redBoxPickBnSlot();                // 红色框选择按钮槽
    void redBoxClearBnSlot();               // 红色框清除按钮槽
    void yellowBoxPickBnSlot();             // 黄色框选择按钮槽
    void yellowBoxClearBnSlot();            // 黄色框清除按钮槽
    void blueBoxPickBnSlot();               // 蓝色框选择按钮槽
    void blueBoxClearBnSlot();              // 蓝色框清除按钮槽
    void startPointPickBnSlot();            // 开始位置选择按钮槽
    void startPointClearBnSlot();           // 开始位置清除按钮槽
    void flyPointPickBnSlot();              // 四轴位置选择按钮槽
    void flyPointClearBnSlot();             // 四轴位置清除按钮槽

    void distinguishCBSlot();               // 自识别复选框触发槽
    void switchXYCheckBoxSlot();            // 互换 XY 复选框触发槽
    void pitchCBSlot();                     // 俯仰方向选择触发槽
    void rollCBSlot();                      // 横滚方向选择触发槽

    void hueMinSliderSlot(int value);       // 色度最小值拖动条槽
    void hueMaxSliderSlot(int value);       // 色度最大值拖动条槽
    void satMinSliderSlot(int value);       // 饱和度最小值拖动条槽
    void satMaxSliderSlot(int value);       // 饱和度最大值拖动条槽
    void valMinSliderSlot(int value);       // 明度最小值拖动条槽
    void valMaxSliderSlot(int value);       // 明度最大值拖动条槽

    void setCameraIndex(int index);         // 设置摄像头索引
    void imgSwitchBnSlot();                 // 摄像头开关按钮槽

    void clearRecBnSlot();                  // 清除接收区按钮槽
    void dataSendBnSlot();                  // 数据发送按钮槽
    void clearSendBnSlot();                 // 清除发送区按钮槽
    void hexRecCheckBoxSlot();              // 十六进制接收复选框槽
    void hexSendCheckBoxSlot();             // 十六进制发送复选框槽

    void flyFrontBnDown();                  // 前飞
    void flyBackBnDown();                   // 后飞
    void flyLeftBnDown();                   // 左飞
    void flyRightBnDown();                  // 右飞
    void flyCenter();                       // 悬停
    void flyUnlockBnDown();                 // 解锁
    void flyLockBnDown();                   // 上锁
    void flyAttitudeAddBnDown();            // 高度加
    void flyAttitudeDownBnDown();           // 高度减
    void flyStopBnDown();                   // 停机
    void oneKeyFly();                       // 一键起飞
    void flyPowerModeBnClick();             // 油门控制模式切换
    void flyAttitudeModeBnClick();          // 定高模式切换
    void flyStopQuickBnClick();             // 紧急停机

    void showFlyStateCBSlot();              // 显示飞控状态复选框触发槽

    void timerSlot();                       // 定时器定时触发槽
};

#endif // MAINWINDOW_H
