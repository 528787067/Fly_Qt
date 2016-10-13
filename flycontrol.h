#ifndef FLYCONTROL_H
#define FLYCONTROL_H

#include <QObject>
#include <math.h>
#include "imgthread.h"
#include "flyparams.h"

#define FLY_UNDATE_FRAME    10                                          // 上一次四轴位置更新频率
#define FLY_MIN_POINT       2                                           // 飞行器控制的最小飞行距离

typedef enum
{
    TARGET_BOX,
    TARGET_RETURN
} TargetType;

class FlyControl : public QObject
{
public:
    FlyControl(QObject *object);
    ~FlyControl();

    void setIntentParams(SelectType type, CircleParams circleParams);   // 设置目标参数
    void setPitchDirection(FlyDirection direction);                     // 设置俯仰前进方向
    void setRollDirection(FlyDirection direction);                      // 设置横滚前进方向

    void controlFly();                                                  // 控制四轴飞行器

private:
    ImgThread *imgThread;
    FlyParams *flyParams;

    CircleParams targetBox;                                             // 代表目标坐标
    CircleParams redBox;                                                // 代表红框坐标
    CircleParams yellowBox;                                             // 代表黄框坐标
    CircleParams blueBox;                                               // 代表蓝框坐标
    CircleParams startPoint;                                            // 代表起始坐标
    CircleParams flyPoint;                                              // 代表飞行器坐标
    CircleParams preFlyPoint;                                           // 代表飞行器上一次记录坐标
    int flyPointUpdateCount;                                            // 飞行器坐标更新计数器
    int preFlyCom;                                                      // 上一次飞行器的飞行命令
    int pitchError;                                                     // 飞行器与上一次在俯仰方向的偏差
    int rollError;                                                      // 飞行器与上一次在横滚方向的偏差

    FlyDirection pitchDirection;
    FlyDirection rollDirection;

    FlyDirection flyPitchDirection;
    FlyDirection flyRollDirection;

    int pitchDistance;
    int rollDistance;

    void undateTargetBox(TargetType type);                              // 设置目标类型
    bool isThrow;
};

#endif // FLYCONTROL_H
