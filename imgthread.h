#ifndef IMGTHREAD_H
#define IMGTHREAD_H

#include <QThread>
#include <QCameraInfo>
#include <QStringList>
#include <QList>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "math.h"

#define IMG_WINDOW_NAME "Camera"

class MainWindow;
class FlyControl;

using namespace cv;

typedef struct
{
    int minValue;
    int maxValue;
} ParamsValue;

typedef struct
{
    int centerX;
    int centerY;
    int radius;
} CircleParams;

typedef enum
{
    RED_BOX,
    YELLOW_BOX,
    BLUE_BOX,
    START_POINT,
    FLY_POINT
} SelectType;

typedef enum
{
    HUI_MIN,
    HUI_MAX,
    SAT_MIN,
    SAT_MAX,
    VAL_MIN,
    VAL_MAX
} ImgParamsType;

typedef enum
{
    UP,
    DOWN,
    LEFT,
    RIGHT
} FlyDirection;

typedef enum
{
    SELECT,
    DISTINGUISH,
    DISTINGUISH_ING
} FindType;

class ImgThread : public QThread
{
public:
    ImgThread(QObject *object);
    ~ImgThread();

    QStringList getAllCameras();                                // 获取所有摄像头
    bool openCamera();                                          // 启动摄像头获取图像
    bool closeCamera();                                         // 关闭摄像头

    void setCameraIndex(int index);                             // 设置摄像头索引
    bool isCameraOpen();                                        // 返回摄像头是否打开
    void setImgParams(ImgParamsType type, int value);           // 设置图像参数值

    void setFindType(FindType findType);                        // 设置识别类型
    FindType getFindType();                                     // 返回识别类型
    void setSelectType(SelectType type);                        // 设置当前要选择的类型
    SelectType getSelectType();                                 // 返回选择类型
    void clearSelectType(SelectType type);                      // 清除选择的识别区域

    void setPitchDirection(FlyDirection direction);             // 设置俯仰前进方向
    void setRollDirection(FlyDirection direction);              // 设置横滚前进方向

protected:
    void run();

private:
    MainWindow *mainWindow;
    FlyControl *flyControl;

    Mat frame;
    VideoCapture capture;

    QStringList cameraNames;
    int cameraIndex;
    bool runFlag;

    FindType findType;
    SelectType selectType;

    CircleParams redBox;
    CircleParams yellowBox;
    CircleParams blueBox;
    CircleParams startPoint;
    CircleParams flyPoint;

    ParamsValue hueParams;
    ParamsValue satParams;
    ParamsValue valParams;

    bool selectObject;
    Rect selection;
    bool trackObject;

    int hsize;
    float hranges[2];
    const float *phranges;
    Rect trackWindow;
    Mat hsv;
    Mat hue;
    Mat mask;
    Mat hist;
    Mat histimg;
    Mat backproj;

    void setSelectValue(SelectType type, int x, int y, int radius); // 设置选区
    void dealImg(Mat &image);                                       // 图像处理函数
    Mat qImage2cvMat(const QImage &image);                          // QImage 转 Mat
    QImage cvMat2QImage(const Mat& mat);                            // Mat 转 QImage

public:
    void onMouse(int event, int x, int y);                          // 鼠标处理函数
};

#endif // IMGTHREAD_H
