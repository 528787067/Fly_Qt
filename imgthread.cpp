#include "imgthread.h"
#include "mainwindow.h"
#include "flycontrol.h"

static ImgThread *imgThread;
ImgThread::ImgThread(QObject *object)
{
    this->mainWindow = (MainWindow *)object;
    this->flyControl = new FlyControl(this);
    imgThread = this;

    runFlag = false;

    findType = SELECT;
    selectType = FLY_POINT;
    redBox.radius = 0;
    yellowBox.radius = 0;
    blueBox.radius = 0;
    startPoint.radius = 0;
    flyPoint.radius = 0;

    hueParams.minValue = 0;
    hueParams.maxValue = 180;
    satParams.minValue = 0;
    satParams.maxValue = 256;
    valParams.minValue = 0;
    valParams.maxValue = 256;

    selectObject = false;
    trackObject = false;
    hsize = 16;
    hranges[0] = 0;
    hranges[1] = 180;
    phranges = hranges;
    histimg = Mat::zeros(200, 320, CV_8UC3);
}

ImgThread::~ImgThread()
{
    if(this->isRunning())
    {
        runFlag = false;
        //this->quit();
        //while(this->isRunning());
    }
    imgThread = NULL;
    this->mainWindow = NULL;        // 将指针悬空

    delete flyControl;
}

/* 获取所有摄像头名称 */
QStringList ImgThread::getAllCameras()
{
    cameraNames.clear();
    cameraNames << "";
    foreach (const QCameraInfo &cameraInfo, QCameraInfo::availableCameras())
    {
        cameraNames << cameraInfo.description();
    }
    return cameraNames;
}

/* 启动摄像头获取图像 */
bool ImgThread::openCamera()
{
    capture.open(cameraIndex);
    if(capture.isOpened())
    {
        runFlag = true;
        this->start();
        return true;
    }
    else
        return false;
}

/* 关闭摄像头 */
bool ImgThread::closeCamera()
{
    if(runFlag)
        runFlag = false;
    return true;
}

/* 设置摄像头索引 */
void ImgThread::setCameraIndex(int index)
{
    this->cameraIndex = index;
}

/* 设置参数值 */
void ImgThread::setImgParams(ImgParamsType type, int value)
{
    switch(type)
    {
    case HUI_MIN:
        hueParams.minValue = value;
        break;
    case HUI_MAX:
        hueParams.maxValue = value;
        break;
    case SAT_MIN:
        satParams.minValue = value;
        break;
    case SAT_MAX:
        satParams.maxValue = value;
        break;
    case VAL_MIN:
        valParams.minValue = value;
        break;
    case VAL_MAX:
        valParams.maxValue = value;
        break;
    default:
        return;
    }
}

/* 设置识别类型 */
void ImgThread::setFindType(FindType findType)
{
    this->findType = findType;
}

FindType ImgThread::getFindType()
{
    return this->findType;
}

/* 设置当前要选择的类型 */
void ImgThread::setSelectType(SelectType type)
{
    switch(type)
    {
    case RED_BOX:
        selectType = RED_BOX;
        break;
    case YELLOW_BOX:
        selectType = YELLOW_BOX;
        break;
    case BLUE_BOX:
        selectType = BLUE_BOX;
        break;
    case START_POINT:
        selectType = START_POINT;
        break;
    case FLY_POINT:
        selectType = FLY_POINT;
        break;
    default:
        return;
    }
}

/* 返回选择类型 */
SelectType ImgThread::getSelectType()
{
    return this->selectType;
}

/* 清除选择的识别区域 */
void ImgThread::clearSelectType(SelectType type)
{
    switch(type)
    {
    case RED_BOX:
        redBox.radius = 0;
        flyControl->setIntentParams(RED_BOX, redBox);
        break;
    case YELLOW_BOX:
        yellowBox.radius = 0;
        flyControl->setIntentParams(YELLOW_BOX, yellowBox);
        break;
    case BLUE_BOX:
        blueBox.radius = 0;
        flyControl->setIntentParams(BLUE_BOX, blueBox);
        break;
    case START_POINT:
        startPoint.radius = 0;
        flyControl->setIntentParams(START_POINT, startPoint);
        break;
    case FLY_POINT:
        flyPoint.radius = 0;
        flyControl->setIntentParams(FLY_POINT, flyPoint);
        break;
    default:
        return;
    }
}

/* 返回摄像头是否打开 */
bool ImgThread::isCameraOpen()
{
    return capture.isOpened();
}

/* 设置俯仰前进方向 */
void ImgThread::setPitchDirection(FlyDirection direction)
{
    this->flyControl->setPitchDirection(direction);
}

/* 设置横滚前进方向 */
void ImgThread::setRollDirection(FlyDirection direction)
{
    this->flyControl->setRollDirection(direction);
}


/* 鼠标回调函数 */
static void _onMouse(int event, int x, int y, int, void *)
{
    if(imgThread->getFindType() == DISTINGUISH)
        return;
    else
        imgThread->onMouse(event, x, y);
}

void ImgThread::run()
{
    namedWindow(IMG_WINDOW_NAME);
    setMouseCallback(IMG_WINDOW_NAME, _onMouse, 0);
    while(runFlag && capture.isOpened())
    {
        if(!cvGetWindowHandle(IMG_WINDOW_NAME))
        {
            mainWindow->changeCameraUi(true);
            break;
        }
        capture >> frame;
        if(frame.empty())
            continue;

        if(this->findType == DISTINGUISH_ING)
        {
            this->findType = DISTINGUISH;
            trackObject = true;
            selection.x = 0;
            selection.y = 0;
            selection.width = frame.cols - 1;
            selection.height = frame.rows - 1;
        }
        if(this->findType != SELECT)
        {
            this->dealImg(frame);

            if(flyPoint.radius > 0 && this->selectType != FLY_POINT)
                circle(frame,
                       Point(flyPoint.centerX, flyPoint.centerY),
                       flyPoint.radius,
                       Scalar(255, 255, 0),
                       2,
                       CV_AA);
        }
        else if(flyPoint.radius > 0)
        {
            if(selection.area() > 0 && (!selectObject || (selectObject && selectType != FLY_POINT)))
                this->dealImg(frame);
            else
                circle(frame,
                       Point(flyPoint.centerX, flyPoint.centerY),
                       flyPoint.radius,
                       Scalar(255, 255, 0),
                       2,
                       CV_AA);
        }
        else;

        if(redBox.radius > 0 && ((findType == SELECT) || (findType != SELECT && selectType != RED_BOX)))
            circle(frame,
                   Point(redBox.centerX, redBox.centerY),
                   redBox.radius,
                   Scalar(0, 0, 255),
                   2,
                   CV_AA);
        if(yellowBox.radius > 0 && ((findType == SELECT) || (findType != SELECT && selectType != YELLOW_BOX)))
            circle(frame,
                   Point(yellowBox.centerX, yellowBox.centerY),
                   yellowBox.radius,
                   Scalar(0, 255, 255),
                   2,
                   CV_AA);
        if(blueBox.radius > 0 && ((findType == SELECT) || (findType != SELECT && selectType != BLUE_BOX)))
            circle(frame,
                   Point(blueBox.centerX, blueBox.centerY),
                   blueBox.radius,
                   Scalar(255, 0, 0),
                   2,
                   CV_AA);
        if(startPoint.radius > 0 && ((findType == SELECT) || (findType != SELECT && selectType != START_POINT)))
            circle(frame,
                   Point(startPoint.centerX, startPoint.centerY),
                   startPoint.radius,
                   Scalar(255, 255, 255),
                   2,
                   CV_AA);
        imshow(IMG_WINDOW_NAME, frame);
        waitKey(1);
    }
    destroyWindow(IMG_WINDOW_NAME);
    if(capture.isOpened())
        capture.release();
}

/* 设置选区 */
void ImgThread::setSelectValue(SelectType type, int x, int y, int radius)
{
    switch(type)
    {
    case RED_BOX:
        redBox.centerX = x;
        redBox.centerY = y;
        redBox.radius = radius;
        break;
    case YELLOW_BOX:
        yellowBox.centerX = x;
        yellowBox.centerY = y;
        yellowBox.radius = radius;
        break;
    case BLUE_BOX:
        blueBox.centerX = x;
        blueBox.centerY = y;
        blueBox.radius = radius;
        break;
    case START_POINT:
        startPoint.centerX = x;
        startPoint.centerY = y;
        startPoint.radius = radius;
        break;
    case FLY_POINT:
        flyPoint.centerX = x;
        flyPoint.centerY = y;
        flyPoint.radius = radius;
        break;
    default:
        return;
    }
}

/* 图像处理 */
void ImgThread::dealImg(Mat &image)
{
    // 将图像转换为 HSV 模式
    cvtColor(image, hsv, COLOR_BGR2HSV);
    // 设置图像 HSV 范围
    inRange(hsv,
            Scalar(hueParams.minValue, satParams.minValue, valParams.minValue),
            Scalar(hueParams.maxValue, satParams.maxValue, valParams.maxValue),
            mask);

    int ch[] = {0, 0};
    hue.create(hsv.size(), hsv.depth());
    // 将hsv第一个通道(也就是色调)的数复制到hue中，0索引数组
    mixChannels(&hsv, 1, &hue, 1, ch, 1);

    if(trackObject)
    {
        trackObject = false;
        Mat roi(hue, selection);
        Mat maskroi(mask, selection);
        //将roi的0通道计算直方图并通过mask放入hist中，hsize为每一维直方图的大小
        calcHist(&roi, 1, 0, maskroi, hist, 1, &hsize, &phranges);
        // 将hist矩阵进行数组范围归一化，都归一化到0~255
        normalize(hist, hist, 0, 255, CV_MINMAX);
        trackWindow = selection;
        histimg = Scalar::all(0);
        // bin 的宽度
        int binW = histimg.cols / hsize;
        // 定义一个缓冲单bin矩阵
        Mat buf(1, hsize, CV_8UC3);
        for( int i = 0; i < hsize; i++ )
            buf.at<Vec3b>(i) = Vec3b(saturate_cast<uchar>(i*180./hsize), 255, 255);
        // 将hsv又转换成bgr
        cvtColor(buf, buf, CV_HSV2BGR);
        for( int i = 0; i < hsize; i++ )
        {
            int val = saturate_cast<int>(hist.at<float>(i)*histimg.rows/255);
            // 在一幅输入图像上画一个简单抽的矩形，指定左上角和右下角，并定义颜色，大小，线型等
            rectangle(histimg, Point(i*binW, histimg.rows),
                      Point((i+1)*binW, histimg.rows - val),
                      Scalar(buf.at<Vec3b>(i)), -1, 8);
        }
        mainWindow->setHistogram(QPixmap::fromImage(cvMat2QImage(histimg)));
    }

    // 计算直方图的反向投影，计算hue图像0通道直方图hist的反向投影，并让入backproj中
    calcBackProject(&hue, 1, 0, hist, backproj, &phranges);
    backproj &= mask;
    RotatedRect trackBox = CamShift(backproj,
                                    trackWindow,
                                    TermCriteria( CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1 ));
    if(trackWindow.area() <= 1)
    {
        int cols = backproj.cols, rows = backproj.rows, r = (MIN(cols, rows) + 5)/6;
        trackWindow = Rect(trackWindow.x - r, trackWindow.y - r,
                          trackWindow.x + r, trackWindow.y + r)
                    & Rect(0, 0, cols, rows);
    }

    // 界面显示二值化图像
    mainWindow->setOpenCVImg(QPixmap::fromImage(cvMat2QImage(backproj)));

    if(this->findType == SELECT)
    {
        int trackBoxWidth = trackBox.size.width;
        int trackBoxHeight = trackBox.size.height;
        if(trackBoxWidth < 0)
            trackBoxWidth = 0;
        if(trackBoxHeight < 0)
            trackBoxHeight = 0;

        // 设置四轴飞行器的坐标
        flyPoint.centerX = trackBox.center.x;
        flyPoint.centerY = trackBox.center.y;
        flyPoint.radius = trackBoxWidth < trackBoxHeight
                        ? trackBoxWidth
                        : trackBoxHeight;

        if(flyPoint.radius < 1 && flyPoint.radius > 0)
            flyPoint.radius = 1;

        // 在原图上画圈出目标
        //ellipse(image, trackBox, Scalar(255, 255, 0), 2, CV_AA);
        ellipse(image,
                RotatedRect(Point2f(flyPoint.centerX, flyPoint.centerY),
                            Size2f(trackBoxWidth, trackBoxHeight),
                            trackBox.angle),
                Scalar(255, 255, 0),
                2,
                CV_AA);

        // 设置主界面坐标
        mainWindow->setSelectPosition(FLY_POINT, flyPoint.centerX, flyPoint.centerY);
        flyControl->setIntentParams(FLY_POINT, flyPoint);
    }
    else
    {
        if(this->selectType == RED_BOX)
        {
            ellipse(image, trackBox, Scalar(0, 0, 255), 2, CV_AA);
            redBox.centerX = trackBox.center.x;
            redBox.centerY = trackBox.center.y;
            Rect boundRect = trackBox.boundingRect();
            redBox.radius = boundRect.width < boundRect.height
                            ? boundRect.width
                            : boundRect.height;
            if(redBox.radius < 1)
                redBox.radius = 1;
            mainWindow->setSelectPosition(RED_BOX, redBox.centerX, redBox.centerY);
            flyControl->setIntentParams(RED_BOX, redBox);
        }
        else if(this->selectType == YELLOW_BOX)
        {
            ellipse(image, trackBox, Scalar(0, 255, 255), 2, CV_AA);
            yellowBox.centerX = trackBox.center.x;
            yellowBox.centerY = trackBox.center.y;
            Rect boundRect = trackBox.boundingRect();
            yellowBox.radius = boundRect.width < boundRect.height
                            ? boundRect.width
                            : boundRect.height;
            if(yellowBox.radius < 1)
                yellowBox.radius = 1;
            mainWindow->setSelectPosition(YELLOW_BOX, yellowBox.centerX, yellowBox.centerY);
            flyControl->setIntentParams(YELLOW_BOX, yellowBox);
        }
        else if(this->selectType == BLUE_BOX)
        {
            ellipse(image, trackBox, Scalar(255, 0, 0), 2, CV_AA);
            blueBox.centerX = trackBox.center.x;
            blueBox.centerY = trackBox.center.y;
            Rect boundRect = trackBox.boundingRect();
            blueBox.radius = boundRect.width < boundRect.height
                            ? boundRect.width
                            : boundRect.height;
            if(blueBox.radius < 1)
                blueBox.radius = 1;
            mainWindow->setSelectPosition(BLUE_BOX, blueBox.centerX, blueBox.centerY);
            flyControl->setIntentParams(BLUE_BOX, blueBox);
        }
        else if(this->selectType == START_POINT)
        {
            ellipse(image, trackBox, Scalar(255, 255, 255), 2, CV_AA);
            startPoint.centerX = trackBox.center.x;
            startPoint.centerY = trackBox.center.y;
            Rect boundRect = trackBox.boundingRect();
            startPoint.radius = boundRect.width < boundRect.height
                            ? boundRect.width
                            : boundRect.height;
            if(startPoint.radius < 1)
                startPoint.radius = 1;
            mainWindow->setSelectPosition(START_POINT, startPoint.centerX, startPoint.centerY);
            flyControl->setIntentParams(START_POINT, startPoint);
        }
        else if(this->selectType == FLY_POINT)
        {
            ellipse(image, trackBox, Scalar(255, 255, 0), 2, CV_AA);
            flyPoint.centerX = trackBox.center.x;
            flyPoint.centerY = trackBox.center.y;
            Rect boundRect = trackBox.boundingRect();
            flyPoint.radius = boundRect.width < boundRect.height
                            ? boundRect.width
                            : boundRect.height;
            if(flyPoint.radius < 1)
                flyPoint.radius = 1;
            mainWindow->setSelectPosition(FLY_POINT, flyPoint.centerX, flyPoint.centerY);
            flyControl->setIntentParams(FLY_POINT, flyPoint);
        }
        else;
    }

    // 控制四轴飞行器
    if(flyPoint.radius > 0)
        flyControl->controlFly();
}

/* 鼠标处理函数 */
void ImgThread::onMouse(int event, int x, int y)
{
    if(selectObject)
    {
        if(selectType == RED_BOX)
        {
            redBox.radius = (int)sqrt((x - redBox.centerX)*(x - redBox.centerX)
                                     +(y - redBox.centerY)*(y - redBox.centerY));
            if(redBox.radius < 1)
                redBox.radius = 1;
        }
        else if(selectType == YELLOW_BOX)
        {
            yellowBox.radius = (int)sqrt((x - yellowBox.centerX)*(x - yellowBox.centerX)
                                        +(y - yellowBox.centerY)*(y - yellowBox.centerY));
            if(yellowBox.radius < 1)
                yellowBox.radius = 1;
        }
        else if(selectType == BLUE_BOX)
        {
            blueBox.radius = (int)sqrt((x - blueBox.centerX)*(x - blueBox.centerX)
                                      +(y - blueBox.centerY)*(y - blueBox.centerY));
            if(blueBox.radius < 1)
                blueBox.radius = 1;
        }
        else if(selectType == START_POINT)
        {
            startPoint.radius = (int)sqrt((x - startPoint.centerX)*(x - startPoint.centerX)
                                         +(y - startPoint.centerY)*(y - startPoint.centerY));
            if(startPoint.radius < 1)
                startPoint.radius = 1;
        }
        else if(selectType == FLY_POINT)
        {
            flyPoint.radius = (int)sqrt((x - flyPoint.centerX)*(x - flyPoint.centerX)
                                       +(y - flyPoint.centerY)*(y - flyPoint.centerY));
            if(flyPoint.radius < 1)
                flyPoint.radius = 1;
        }
        else;
    }
    switch(event)
    {
    case EVENT_LBUTTONDOWN:
        selectObject = true;
        if(selectType == RED_BOX)
        {
            this->setSelectValue(RED_BOX, x, y, 1);
            mainWindow->setSelectPosition(RED_BOX, x, y);
        }
        else if(selectType == YELLOW_BOX)
        {
            this->setSelectValue(YELLOW_BOX, x, y, 1);
            mainWindow->setSelectPosition(YELLOW_BOX, x, y);
        }
        else if(selectType == BLUE_BOX)
        {
            this->setSelectValue(BLUE_BOX, x, y, 1);
            mainWindow->setSelectPosition(BLUE_BOX, x, y);
        }
        else if(selectType == START_POINT)
        {
            this->setSelectValue(START_POINT, x, y, 1);
            mainWindow->setSelectPosition(START_POINT, x, y);
        }
        else if(selectType == FLY_POINT)
        {
            this->setSelectValue(FLY_POINT, x, y, 1);
            mainWindow->setSelectPosition(FLY_POINT, x, y);
        }
        else
            break;
        break;
    case EVENT_LBUTTONUP:
        if(selectObject)
        {
            selectObject = false;
            if(selectType == RED_BOX)
                flyControl->setIntentParams(RED_BOX, redBox);
            else if(selectType == YELLOW_BOX)
                flyControl->setIntentParams(YELLOW_BOX, yellowBox);
            else if(selectType == BLUE_BOX)
                flyControl->setIntentParams(BLUE_BOX, blueBox);
            else if(selectType == START_POINT)
                flyControl->setIntentParams(START_POINT, startPoint);
            else if(selectType == FLY_POINT)
            {
                flyControl->setIntentParams(FLY_POINT, flyPoint);

                trackObject = true;
                selection.x = flyPoint.centerX - flyPoint.radius*0.707;
                selection.y = flyPoint.centerY - flyPoint.radius*0.707;
                selection.width = flyPoint.radius*1.414;
                selection.height = selection.width;

                if(selection.x < 0)
                    selection.x = 0;
                if(selection.x > frame.cols-1)
                    selection.x = frame.cols - 1;
                if(selection.y < 0)
                    selection.y = 0;
                if(selection.y > frame.rows-1)
                    selection.y = frame.rows - 1;
                if(selection.x + selection.width > frame.cols - 1)
                    selection.width = frame.cols - 1 - selection.x;
                if(selection.y + selection.height > frame.rows - 1)
                    selection.height = frame.rows - 1 - selection.y;
            }
            else;
        }
        break;
    }
}


/* QImage 转 Mat */
Mat ImgThread::qImage2cvMat(const QImage &image)
{
    Mat mat;

    switch(image.format())
    {
    case QImage::Format_ARGB32:
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32_Premultiplied:
        mat = Mat(image.height(), image.width(), CV_8UC4, (void*)image.constBits(), image.bytesPerLine());
        break;
    case QImage::Format_RGB888:
        mat = Mat(image.height(), image.width(), CV_8UC3, (void*)image.constBits(), image.bytesPerLine());
        cv::cvtColor(mat, mat, CV_BGR2RGB);
        break;
    case QImage::Format_Indexed8:
        mat = Mat(image.height(), image.width(), CV_8UC1, (void*)image.constBits(), image.bytesPerLine());
        break;
    }
    return mat;
}

/* Mat 转 QImage */
QImage ImgThread::cvMat2QImage(const Mat& mat)
{
    if(mat.type() == CV_8UC1)
    {
        QImage image(mat.cols, mat.rows, QImage::Format_Indexed8);
        image.setColorCount(256);
        for(int i = 0; i < 256; i++)
        {
            image.setColor(i, qRgb(i, i, i));
        }
        uchar *pSrc = mat.data;
        for(int row = 0; row < mat.rows; row ++)
        {
            uchar *pDest = image.scanLine(row);
            memcpy(pDest, pSrc, mat.cols);
            pSrc += mat.step;
        }
        return image;
    }
    else if(mat.type() == CV_8UC3)
    {
        const uchar *pSrc = (const uchar*)mat.data;
        QImage image(pSrc, mat.cols, mat.rows, (int)mat.step, QImage::Format_RGB888);
        return image.rgbSwapped();
    }
    else if(mat.type() == CV_8UC4)
    {
        const uchar *pSrc = (const uchar*)mat.data;
        QImage image(pSrc, mat.cols, mat.rows, (int)mat.step, QImage::Format_RGB32);
        return image.copy();
    }
    else
    {
        return QImage();
    }
}

