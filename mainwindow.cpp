#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    flyParams = FlyParams::getFlyParams();
    spComm = new SPComm(this);
    imgThread = new ImgThread(this);
    timer = new QTimer(this);

    this->initView();

    connect(timer, SIGNAL(timeout()), this, SLOT(timerSlot()));
    timer->start(TIMER_DELAY);
}

MainWindow::~MainWindow()
{
    flyParams = NULL;
    if(timer->isActive())
        timer->stop();
    if(imgThread->isRunning())
        if(!imgThread->closeCamera())
            imgThread->quit();
    if(spComm->isOpen())
        spComm->closePort();
    delete timer;
    delete imgThread;
    delete spComm;
    delete ui;
}

void MainWindow::updateRecData(const QString &string)
{
    ui->dataRecLabel->moveCursor(QTextCursor::End);
    ui->dataRecLabel->insertPlainText(string);
}

void MainWindow::setSelectPosition(SelectType type, int x, int y)
{
    switch(type)
    {
    case RED_BOX:
        ui->redBoxXLabel->setText(QString("X:%1").arg(x));
        ui->redBoxYLabel->setText(QString("Y:%1").arg(y));
        break;
    case YELLOW_BOX:
        ui->yellowBoxXLabel->setText(QString("X:%1").arg(x));
        ui->yellowBoxYLabel->setText(QString("Y:%1").arg(y));
        break;
    case BLUE_BOX:
        ui->blueBoxXLabel->setText(QString("X:%1").arg(x));
        ui->blueBoxYLabel->setText(QString("Y:%1").arg(y));
        break;
    case START_POINT:
        ui->startPointXLabel->setText(QString("X:%1").arg(x));
        ui->startPointYLabel->setText(QString("Y:%1").arg(y));
        break;
    case FLY_POINT:
        ui->flyPointXLabel->setText(QString("X:%1").arg(x));
        ui->flyPointYLabel->setText(QString("Y:%1").arg(y));
        break;
    default:
        return;
    }
}

void MainWindow::setOpenCVImg(QPixmap &img)
{
    ui->opencvImg->setPixmap(img.scaled(ui->opencvImg->geometry().size(), Qt::KeepAspectRatio));
}

void MainWindow::setHistogram(QPixmap &img)
{
    ui->histogramLabel->setPixmap(img.scaled(ui->opencvImg->geometry().size(), Qt::KeepAspectRatio));
}

bool MainWindow::setFlyState(QString &str)
{
    if(!ui->showFlyStateCB->isChecked())
        return false;
    if(!str.isEmpty())
        ui->flyStateLabel->setText(str);
    return true;
}

void MainWindow::initView()
{
    this->setWindowTitle("\345\271\277\344\270\234\346\265\267\346\264\213\345\244\247\345\255\246      "
                         "\344\270\211\344\275\223\351\230\237\345\233\233\350\275\264\345\234\260\351\235\242\347\253\231      "
                         "\351\230\237\345\221\230\357\274\232\346\235\216\351\223\255\345\243\253\343\200\201\351\231\210\346\227\255\347\204\266\343\200\201\345\247\232\345\217\213\344\270\232");
    ui->portCB->insertItems(0, spComm->getAllPorts());
    ui->baudCB->addItem("1200");
    ui->baudCB->addItem("2400");
    ui->baudCB->addItem("4800");
    ui->baudCB->addItem("9600");
    ui->baudCB->addItem("19200");
    ui->baudCB->addItem("38400");
    ui->baudCB->addItem("57600");
    ui->baudCB->addItem("115200");
    ui->baudCB->setCurrentIndex(7);
    ui->bitCB->addItem("5");
    ui->bitCB->addItem("6");
    ui->bitCB->addItem("7");
    ui->bitCB->addItem("8");
    ui->bitCB->setCurrentIndex(3);

    ui->cameraCB->insertItems(0, imgThread->getAllCameras());

    ui->opencvImg->setAlignment(Qt::AlignCenter);

    ui->pitchCB->addItem("\342\206\221");
    ui->pitchCB->addItem("\342\206\223");
    ui->rollCB->addItem("\342\206\220");
    ui->rollCB->addItem("\342\206\222");

    connect(ui->portCB, SIGNAL(currentIndexChanged(int)), this, SLOT(portCBoxSlot()));
    connect(ui->baudCB, SIGNAL(currentIndexChanged(int)), this, SLOT(baudCBoxSlot()));
    connect(ui->bitCB, SIGNAL(currentIndexChanged(int)), this, SLOT(dataBitsCBoxSlot()));
    connect(ui->switchBn, SIGNAL(clicked(bool)), this, SLOT(switchBnSlot()));

    connect(ui->redBoxPickBtn, SIGNAL(clicked(bool)), this, SLOT(redBoxPickBnSlot()));
    connect(ui->redBoxClearBtn, SIGNAL(clicked(bool)), this, SLOT(redBoxClearBnSlot()));
    connect(ui->yellowBoxPickBtn, SIGNAL(clicked(bool)), this, SLOT(yellowBoxPickBnSlot()));
    connect(ui->yellowBoxClearBtn, SIGNAL(clicked(bool)), this, SLOT(yellowBoxClearBnSlot()));
    connect(ui->blueBoxPickBtn, SIGNAL(clicked(bool)), this, SLOT(blueBoxPickBnSlot()));
    connect(ui->blueBoxClearBtn, SIGNAL(clicked(bool)), this, SLOT(blueBoxClearBnSlot()));
    connect(ui->startPointPickBtn, SIGNAL(clicked(bool)), this, SLOT(startPointPickBnSlot()));
    connect(ui->startPointClearBtn, SIGNAL(clicked(bool)), this, SLOT(startPointClearBnSlot()));
    connect(ui->flyPointPickBtn, SIGNAL(clicked(bool)), this, SLOT(flyPointPickBnSlot()));
    connect(ui->flyPointClearBtn, SIGNAL(clicked(bool)), this, SLOT(flyPointClearBnSlot()));
    connect(ui->distinguishCB, SIGNAL(clicked(bool)), this, SLOT(distinguishCBSlot()));
    connect(ui->switchXY, SIGNAL(clicked(bool)), this, SLOT(switchXYCheckBoxSlot()));
    connect(ui->pitchCB, SIGNAL(currentTextChanged(QString)), this, SLOT(pitchCBSlot()));
    connect(ui->rollCB, SIGNAL(currentTextChanged(QString)), this, SLOT(rollCBSlot()));

    connect(ui->hueMinSlider, SIGNAL(valueChanged(int)), this, SLOT(hueMinSliderSlot(int)));
    connect(ui->hueMaxSlider, SIGNAL(valueChanged(int)), this, SLOT(hueMaxSliderSlot(int)));
    connect(ui->satMinSlider, SIGNAL(valueChanged(int)), this, SLOT(satMinSliderSlot(int)));
    connect(ui->satMaxSlider, SIGNAL(valueChanged(int)), this, SLOT(satMaxSliderSlot(int)));
    connect(ui->valMinSlider, SIGNAL(valueChanged(int)), this, SLOT(valMinSliderSlot(int)));
    connect(ui->valMaxSlider, SIGNAL(valueChanged(int)), this, SLOT(valMaxSliderSlot(int)));

    connect(ui->cameraCB, SIGNAL(currentIndexChanged(int)), this, SLOT(setCameraIndex(int)));
    connect(ui->imgSwitchBn, SIGNAL(clicked(bool)), this, SLOT(imgSwitchBnSlot()));

    connect(ui->clearRecBn, SIGNAL(clicked(bool)), this, SLOT(clearRecBnSlot()));
    connect(ui->dataSendBn, SIGNAL(clicked(bool)), this, SLOT(dataSendBnSlot()));
    connect(ui->clearSendBn, SIGNAL(clicked(bool)), this, SLOT(clearSendBnSlot()));
    connect(ui->hexRecCheckBox, SIGNAL(clicked(bool)), this, SLOT(hexRecCheckBoxSlot()));
    connect(ui->hexSendCheckBox, SIGNAL(clicked(bool)), this, SLOT(hexSendCheckBoxSlot()));


    connect(ui->flyFrontBn, SIGNAL(pressed()), this, SLOT(flyFrontBnDown()));
    connect(ui->flyFrontBn, SIGNAL(released()), this, SLOT(flyCenter()));
    connect(ui->flyBackBn, SIGNAL(pressed()), this, SLOT(flyBackBnDown()));
    connect(ui->flyBackBn, SIGNAL(released()), this, SLOT(flyCenter()));
    connect(ui->flyLeftBn, SIGNAL(pressed()), this, SLOT(flyLeftBnDown()));
    connect(ui->flyLeftBn, SIGNAL(released()), this, SLOT(flyCenter()));
    connect(ui->flyRightBn, SIGNAL(pressed()), this, SLOT(flyRightBnDown()));
    connect(ui->flyRightBn, SIGNAL(released()), this, SLOT(flyCenter()));
    connect(ui->flyAttitudeAdd, SIGNAL(clicked(bool)), this, SLOT(flyAttitudeAddBnDown()));
    connect(ui->flyAttitudeDown, SIGNAL(clicked(bool)), this, SLOT(flyAttitudeDownBnDown()));
    connect(ui->flyUnlockBn, SIGNAL(clicked(bool)), this, SLOT(flyUnlockBnDown()));
    connect(ui->flyLockBn, SIGNAL(clicked(bool)), this, SLOT(flyLockBnDown()));
    connect(ui->flyStopBn, SIGNAL(clicked(bool)), this, SLOT(flyStopBnDown()));
    connect(ui->flyAttitudeBn, SIGNAL(clicked(bool)), this, SLOT(oneKeyFly()));
    connect(ui->flyPowerModeBn, SIGNAL(clicked(bool)), this, SLOT(flyPowerModeBnClick()));
    connect(ui->flyAttitudeModeBn, SIGNAL(clicked(bool)), this, SLOT(flyAttitudeModeBnClick()));
    connect(ui->flyStopQuickBn, SIGNAL(clicked(bool)), this, SLOT(flyStopQuickBnClick()));

    connect(ui->showFlyStateCB, SIGNAL(clicked(bool)), this, SLOT(showFlyStateCBSlot()));
}

/* 串口改变切换UI */
void MainWindow::changePortUi(bool isOpen)
{
    ui->portCB->setEnabled(isOpen);
    ui->baudCB->setEnabled(isOpen);
    ui->bitCB->setEnabled(isOpen);
    if(isOpen)
        ui->switchBn->setText("打开串口");
    else
        ui->switchBn->setText("关闭串口");
}

/* 摄像头改变切换UI */
void MainWindow::changeCameraUi(bool isOpen)
{
    ui->cameraCB->setEnabled(isOpen);
    if(isOpen)
    {
        this->flyPointClearBnSlot();
        this->redBoxClearBnSlot();
        this->yellowBoxClearBnSlot();
        this->blueBoxClearBnSlot();
        this->startPointClearBnSlot();
        ui->opencvImg->clear();
        ui->histogramLabel->clear();
        ui->imgSwitchBn->setText("打开图像");
    }
    else
        ui->imgSwitchBn->setText("关闭图像");
}

/* 设置串口 */
void MainWindow::setPortCBox(int index)
{
    ui->portCB->setCurrentIndex(index);
}

/* 串口选择框触发槽 */
void MainWindow::portCBoxSlot()
{
    spComm->setPortName(ui->portCB->currentText());
}

/* 波特率选择框触发槽 */
void MainWindow::baudCBoxSlot()
{
    spComm->setBaudRate(ui->baudCB->currentText().toInt());
}

/* 数据位选择框触发槽 */
void MainWindow::dataBitsCBoxSlot()
{
    spComm->setDataBits(ui->bitCB->currentText().toInt());
}

/* 串口开关按钮触发槽 */
void MainWindow::switchBnSlot()
{
    if(spComm->isOpen())
    {
        spComm->closePort();
        this->changePortUi(true);
    }
    else
    {
        if(ui->portCB->currentText().isEmpty())
        {
            QMessageBox::information(this, "警告", "请选择一个合适的串口");
        }
        else
        {
            if(spComm->openPort())
               this->changePortUi(false);
        }
    }
}

/* 红色框选择按钮触发槽 */
void MainWindow::redBoxPickBnSlot()
{
    imgThread->setSelectType(RED_BOX);
    if(!ui->distinguishCB->isChecked())
    {
        ui->yellowBoxPickBtn->setFont(QFont("SimSun", 9));
        ui->blueBoxPickBtn->setFont(QFont("SimSun", 9));
        ui->startPointPickBtn->setFont(QFont("SimSun", 9));
        ui->flyPointPickBtn->setFont(QFont("SimSun", 9));
        ui->redBoxPickBtn->setFont(QFont("SimSun", 9, QFont::Black));
    }
    else
        imgThread->setFindType(DISTINGUISH_ING);
}

/* 红色框清除按钮触发槽 */
void MainWindow::redBoxClearBnSlot()
{
    ui->redBoxXLabel->setText("X:");
    ui->redBoxYLabel->setText("Y:");
    imgThread->clearSelectType(RED_BOX);

    if(ui->distinguishCB->isChecked())
    {
        ui->hueMinLabel->setText(QString("MIN:%1").arg(RED_BOX_H_MIN));
        ui->hueMaxLabel->setText(QString("MAX:%1").arg(RED_BOX_H_MAX));
        ui->hueMinSlider->setValue(RED_BOX_H_MIN);
        ui->hueMaxSlider->setValue(RED_BOX_H_MAX);
        imgThread->setImgParams(HUI_MIN, RED_BOX_H_MIN);
        imgThread->setImgParams(HUI_MAX, RED_BOX_H_MAX);

        ui->satMinLabel->setText(QString("MIN:%1").arg(RED_BOX_S_MIN));
        ui->satMaxLabel->setText(QString("MAX:%1").arg(RED_BOX_S_MAX));
        ui->satMinSlider->setValue(RED_BOX_S_MIN);
        ui->satMaxSlider->setValue(RED_BOX_S_MAX);
        imgThread->setImgParams(SAT_MIN, RED_BOX_S_MIN);
        imgThread->setImgParams(SAT_MAX, RED_BOX_S_MAX);

        ui->valMinLabel->setText(QString("MIN:%1").arg(RED_BOX_V_MIN));
        ui->valMaxLabel->setText(QString("MAX:%1").arg(RED_BOX_V_MAX));
        ui->valMinSlider->setValue(RED_BOX_V_MIN);
        ui->valMaxSlider->setValue(RED_BOX_V_MAX);
        imgThread->setImgParams(VAL_MIN, RED_BOX_V_MIN);
        imgThread->setImgParams(VAL_MAX, RED_BOX_V_MAX);
    }
}

/* 黄色框选择按钮触发槽 */
void MainWindow::yellowBoxPickBnSlot()
{
    imgThread->setSelectType(YELLOW_BOX);
    if(!ui->distinguishCB->isChecked())
    {
        ui->redBoxPickBtn->setFont(QFont("SimSun", 9));
        ui->blueBoxPickBtn->setFont(QFont("SimSun", 9));
        ui->startPointPickBtn->setFont(QFont("SimSun", 9));
        ui->flyPointPickBtn->setFont(QFont("SimSun", 9));
        ui->yellowBoxPickBtn->setFont(QFont("SimSun", 9, QFont::Black));
    }
    else
        imgThread->setFindType(DISTINGUISH_ING);
}

/* 黄色框清除按钮触发槽 */
void MainWindow::yellowBoxClearBnSlot()
{
    ui->yellowBoxXLabel->setText("X:");
    ui->yellowBoxYLabel->setText("Y:");
    imgThread->clearSelectType(YELLOW_BOX);

    if(ui->distinguishCB->isChecked())
    {
        ui->hueMinLabel->setText(QString("MIN:%1").arg(YELLOW_BOX_H_MIN));
        ui->hueMaxLabel->setText(QString("MAX:%1").arg(YELLOW_BOX_H_MAX));
        ui->hueMinSlider->setValue(YELLOW_BOX_H_MIN);
        ui->hueMaxSlider->setValue(YELLOW_BOX_H_MAX);
        imgThread->setImgParams(HUI_MIN, YELLOW_BOX_H_MIN);
        imgThread->setImgParams(HUI_MAX, YELLOW_BOX_H_MAX);

        ui->satMinLabel->setText(QString("MIN:%1").arg(YELLOW_BOX_S_MIN));
        ui->satMaxLabel->setText(QString("MAX:%1").arg(YELLOW_BOX_S_MAX));
        ui->satMinSlider->setValue(YELLOW_BOX_S_MIN);
        ui->satMaxSlider->setValue(YELLOW_BOX_S_MAX);
        imgThread->setImgParams(SAT_MIN, YELLOW_BOX_S_MIN);
        imgThread->setImgParams(SAT_MAX, YELLOW_BOX_S_MAX);

        ui->valMinLabel->setText(QString("MIN:%1").arg(YELLOW_BOX_V_MIN));
        ui->valMaxLabel->setText(QString("MAX:%1").arg(YELLOW_BOX_V_MAX));
        ui->valMinSlider->setValue(YELLOW_BOX_V_MIN);
        ui->valMaxSlider->setValue(YELLOW_BOX_V_MAX);
        imgThread->setImgParams(VAL_MIN, YELLOW_BOX_V_MIN);
        imgThread->setImgParams(VAL_MAX, YELLOW_BOX_V_MAX);
    }
}

/* 蓝色框选择按钮触发槽 */
void MainWindow::blueBoxPickBnSlot()
{
    imgThread->setSelectType(BLUE_BOX);
    if(!ui->distinguishCB->isChecked())
    {
        ui->redBoxPickBtn->setFont(QFont("SimSun", 9));
        ui->yellowBoxPickBtn->setFont(QFont("SimSun", 9));
        ui->startPointPickBtn->setFont(QFont("SimSun", 9));
        ui->flyPointPickBtn->setFont(QFont("SimSun", 9));
        ui->blueBoxPickBtn->setFont(QFont("SimSun", 9, QFont::Black));
    }
    else
        imgThread->setFindType(DISTINGUISH_ING);
}

/* 蓝色框清除按钮触发槽 */
void MainWindow::blueBoxClearBnSlot()
{
    ui->blueBoxXLabel->setText("X:");
    ui->blueBoxYLabel->setText("Y:");
    imgThread->clearSelectType(BLUE_BOX);

    if(ui->distinguishCB->isChecked())
    {
        ui->hueMinLabel->setText(QString("MIN:%1").arg(BLUE_BOX_H_MIN));
        ui->hueMaxLabel->setText(QString("MAX:%1").arg(BLUE_BOX_H_MAX));
        ui->hueMinSlider->setValue(BLUE_BOX_H_MIN);
        ui->hueMaxSlider->setValue(BLUE_BOX_H_MAX);
        imgThread->setImgParams(HUI_MIN, BLUE_BOX_H_MIN);
        imgThread->setImgParams(HUI_MAX, BLUE_BOX_H_MAX);

        ui->satMinLabel->setText(QString("MIN:%1").arg(BLUE_BOX_S_MIN));
        ui->satMaxLabel->setText(QString("MAX:%1").arg(BLUE_BOX_S_MAX));
        ui->satMinSlider->setValue(BLUE_BOX_S_MIN);
        ui->satMaxSlider->setValue(BLUE_BOX_S_MAX);
        imgThread->setImgParams(SAT_MIN, BLUE_BOX_S_MIN);
        imgThread->setImgParams(SAT_MAX, BLUE_BOX_S_MAX);

        ui->valMinLabel->setText(QString("MIN:%1").arg(BLUE_BOX_V_MIN));
        ui->valMaxLabel->setText(QString("MAX:%1").arg(BLUE_BOX_V_MAX));
        ui->valMinSlider->setValue(BLUE_BOX_V_MIN);
        ui->valMaxSlider->setValue(BLUE_BOX_V_MAX);
        imgThread->setImgParams(VAL_MIN, BLUE_BOX_V_MIN);
        imgThread->setImgParams(VAL_MAX, BLUE_BOX_V_MAX);
    }
}

/* 开始位置选择按钮触发槽 */
void MainWindow::startPointPickBnSlot()
{
    imgThread->setSelectType(START_POINT);
    if(!ui->distinguishCB->isChecked())
    {
        ui->redBoxPickBtn->setFont(QFont("SimSun", 9));
        ui->yellowBoxPickBtn->setFont(QFont("SimSun", 9));
        ui->blueBoxPickBtn->setFont(QFont("SimSun", 9));
        ui->flyPointPickBtn->setFont(QFont("SimSun", 9));
        ui->startPointPickBtn->setFont(QFont("SimSun", 9, QFont::Black));
    }
    else
        imgThread->setFindType(DISTINGUISH_ING);
}

/* 开始位置清除触发槽 */
void MainWindow::startPointClearBnSlot()
{
    ui->startPointXLabel->setText("X:");
    ui->startPointYLabel->setText("Y:");
    imgThread->clearSelectType(START_POINT);

    if(ui->distinguishCB->isChecked())
    {
        ui->hueMinLabel->setText(QString("MIN:%1").arg(START_POINT_H_MIN));
        ui->hueMaxLabel->setText(QString("MAX:%1").arg(START_POINT_H_MAX));
        ui->hueMinSlider->setValue(START_POINT_H_MIN);
        ui->hueMaxSlider->setValue(START_POINT_H_MAX);
        imgThread->setImgParams(HUI_MIN, START_POINT_H_MIN);
        imgThread->setImgParams(HUI_MAX, START_POINT_H_MAX);

        ui->satMinLabel->setText(QString("MIN:%1").arg(START_POINT_S_MIN));
        ui->satMaxLabel->setText(QString("MAX:%1").arg(START_POINT_S_MAX));
        ui->satMinSlider->setValue(START_POINT_S_MIN);
        ui->satMaxSlider->setValue(START_POINT_S_MAX);
        imgThread->setImgParams(SAT_MIN, START_POINT_S_MIN);
        imgThread->setImgParams(SAT_MAX, START_POINT_S_MAX);

        ui->valMinLabel->setText(QString("MIN:%1").arg(START_POINT_V_MIN));
        ui->valMaxLabel->setText(QString("MAX:%1").arg(START_POINT_V_MAX));
        ui->valMinSlider->setValue(START_POINT_V_MIN);
        ui->valMaxSlider->setValue(START_POINT_V_MAX);
        imgThread->setImgParams(VAL_MIN, START_POINT_V_MIN);
        imgThread->setImgParams(VAL_MAX, START_POINT_V_MAX);
    }
}


/* 四轴位置选择按钮触发槽 */
void MainWindow::flyPointPickBnSlot()
{
    imgThread->setSelectType(FLY_POINT);
    if(!ui->distinguishCB->isChecked())
    {
        ui->redBoxPickBtn->setFont(QFont("SimSun", 9));
        ui->yellowBoxPickBtn->setFont(QFont("SimSun", 9));
        ui->blueBoxPickBtn->setFont(QFont("SimSun", 9));
        ui->startPointPickBtn->setFont(QFont("SimSun", 9));
        ui->flyPointPickBtn->setFont(QFont("SimSun", 9, QFont::Black));
    }
    else
        imgThread->setFindType(DISTINGUISH_ING);
}

/* 四轴位置清除按钮触发槽 */
void MainWindow::flyPointClearBnSlot()
{
    ui->flyPointXLabel->setText("X:");
    ui->flyPointYLabel->setText("Y:");
    imgThread->clearSelectType(FLY_POINT);

    if(ui->distinguishCB->isChecked())
    {
        ui->hueMinLabel->setText(QString("MIN:%1").arg(FLY_POINT_H_MIN));
        ui->hueMaxLabel->setText(QString("MAX:%1").arg(FLY_POINT_H_MAX));
        ui->hueMinSlider->setValue(FLY_POINT_H_MIN);
        ui->hueMaxSlider->setValue(FLY_POINT_H_MAX);
        imgThread->setImgParams(HUI_MIN, FLY_POINT_H_MIN);
        imgThread->setImgParams(HUI_MAX, FLY_POINT_H_MAX);

        ui->satMinLabel->setText(QString("MIN:%1").arg(FLY_POINT_S_MIN));
        ui->satMaxLabel->setText(QString("MAX:%1").arg(FLY_POINT_S_MAX));
        ui->satMinSlider->setValue(FLY_POINT_S_MIN);
        ui->satMaxSlider->setValue(FLY_POINT_S_MAX);
        imgThread->setImgParams(SAT_MIN, FLY_POINT_S_MIN);
        imgThread->setImgParams(SAT_MAX, FLY_POINT_S_MAX);

        ui->valMinLabel->setText(QString("MIN:%1").arg(FLY_POINT_V_MIN));
        ui->valMaxLabel->setText(QString("MAX:%1").arg(FLY_POINT_V_MAX));
        ui->valMinSlider->setValue(FLY_POINT_V_MIN);
        ui->valMaxSlider->setValue(FLY_POINT_V_MAX);
        imgThread->setImgParams(VAL_MIN, FLY_POINT_V_MIN);
        imgThread->setImgParams(VAL_MAX, FLY_POINT_V_MAX);
    }
    else
    {
        ui->opencvImg->clear();
        ui->histogramLabel->clear();
    }
}

/* 自识别复选框触发槽 */
void MainWindow::distinguishCBSlot()
{
    ui->redBoxPickBtn->setFont(QFont("SimSun", 9));
    ui->yellowBoxPickBtn->setFont(QFont("SimSun", 9));
    ui->blueBoxPickBtn->setFont(QFont("SimSun", 9));
    ui->startPointPickBtn->setFont(QFont("SimSun", 9));
    ui->flyPointPickBtn->setFont(QFont("SimSun", 9));

    if(ui->distinguishCB->isChecked())
    {
        ui->redBoxPickBtn->setText("识别");
        ui->yellowBoxPickBtn->setText("识别");
        ui->blueBoxPickBtn->setText("识别");
        ui->startPointPickBtn->setText("识别");
        ui->flyPointPickBtn->setText("识别");
        imgThread->setFindType(DISTINGUISH);

        if(imgThread->getSelectType() == RED_BOX)
            this->redBoxPickBnSlot();
        else if(imgThread->getSelectType() == YELLOW_BOX)
            this->yellowBoxPickBnSlot();
        else if(imgThread->getSelectType() == BLUE_BOX)
            this->blueBoxPickBnSlot();
        else if(imgThread->getSelectType() == START_POINT)
            this->startPointPickBnSlot();
        else if(imgThread->getSelectType() == FLY_POINT)
            this->flyPointPickBnSlot();
        else;
    }
    else
    {
        switch(imgThread->getSelectType())
        {
        case RED_BOX:
            ui->redBoxPickBtn->setFont(QFont("SimSun", 9, QFont::Black));
            break;
        case YELLOW_BOX:
            ui->yellowBoxPickBtn->setFont(QFont("SimSun", 9, QFont::Black));
            break;
        case BLUE_BOX:
            ui->blueBoxPickBtn->setFont(QFont("SimSun", 9, QFont::Black));
            break;
        case START_POINT:
            ui->startPointPickBtn->setFont(QFont("SimSun", 9, QFont::Black));
            break;
        case FLY_POINT:
            ui->flyPointPickBtn->setFont(QFont("SimSun", 9, QFont::Black));
            break;
        default:
            break;
        }
        ui->redBoxPickBtn->setText("选取");
        ui->yellowBoxPickBtn->setText("选取");
        ui->blueBoxPickBtn->setText("选取");
        ui->startPointPickBtn->setText("选取");
        ui->flyPointPickBtn->setText("选取");
        imgThread->setFindType(SELECT);

        ui->hueMinLabel->setText(QString("MIN:%1").arg(0));
        ui->hueMaxLabel->setText(QString("MAX:%1").arg(180));
        ui->hueMinSlider->setValue(0);
        ui->hueMaxSlider->setValue(180);
        imgThread->setImgParams(HUI_MIN, 0);
        imgThread->setImgParams(HUI_MAX, 180);

        ui->satMinLabel->setText(QString("MIN:%1").arg(0));
        ui->satMaxLabel->setText(QString("MAX:%1").arg(256));
        ui->satMinSlider->setValue(0);
        ui->satMaxSlider->setValue(256);
        imgThread->setImgParams(SAT_MIN, 0);
        imgThread->setImgParams(SAT_MAX, 256);

        ui->valMinLabel->setText(QString("MIN:%1").arg(0));
        ui->valMaxLabel->setText(QString("MAX:%1").arg(256));
        ui->valMinSlider->setValue(0);
        ui->valMaxSlider->setValue(256);
        imgThread->setImgParams(VAL_MIN, 0);
        imgThread->setImgParams(VAL_MAX, 256);
    }

    this->redBoxClearBnSlot();
    this->yellowBoxClearBnSlot();
    this->startPointClearBnSlot();
    this->flyPointClearBnSlot();
}

/* 互换 XY 复选框触发槽 */
void MainWindow::switchXYCheckBoxSlot()
{
    int pitchIndex = ui->pitchCB->currentIndex();
    int rollIndex = ui->rollCB->currentIndex();
    QString pitchText0 = ui->pitchCB->itemText(0);
    QString pitchText1 = ui->pitchCB->itemText(1);
    ui->pitchCB->setItemText(0, ui->rollCB->itemText(0));
    ui->pitchCB->setItemText(1, ui->rollCB->itemText(1));
    ui->rollCB->setItemText(0, pitchText0);
    ui->rollCB->setItemText(1, pitchText1);
    ui->pitchCB->setCurrentIndex(rollIndex);
    ui->rollCB->setCurrentIndex(pitchIndex);
}

/* 俯仰方向选择触发槽 */
void MainWindow::pitchCBSlot()
{
    if(ui->pitchCB->currentIndex() == 0 && !ui->switchXY->isChecked())
        imgThread->setPitchDirection(UP);
    else if(ui->pitchCB->currentIndex() == 1 && !ui->switchXY->isChecked())
        imgThread->setPitchDirection(DOWN);
    else if(ui->pitchCB->currentIndex() == 0 && ui->switchXY->isChecked())
        imgThread->setPitchDirection(LEFT);
    else if(ui->pitchCB->currentIndex() == 1 && ui->switchXY->isChecked())
        imgThread->setPitchDirection(RIGHT);
    else;
}

/* 横滚方向选择触发槽 */
void MainWindow::rollCBSlot()
{
    if(ui->rollCB->currentIndex() == 0 && !ui->switchXY->isChecked())
        imgThread->setRollDirection(LEFT);
    else if(ui->rollCB->currentIndex() == 1 && !ui->switchXY->isChecked())
        imgThread->setRollDirection(RIGHT);
    else if(ui->rollCB->currentIndex() == 0 && ui->switchXY->isChecked())
        imgThread->setRollDirection(UP);
    else if(ui->rollCB->currentIndex() == 1 && ui->switchXY->isChecked())
        imgThread->setRollDirection(DOWN);
    else;
}

/* 色度最小值拖动条触发槽 */
void MainWindow::hueMinSliderSlot(int value)
{
    ui->hueMinLabel->setText(QString("MIN:%1").arg(value));
    if(ui->hueMaxSlider->value() < value)
        ui->hueMaxSlider->setValue(value);
    imgThread->setImgParams(HUI_MIN, value);
    if(ui->distinguishCB->isChecked())
        imgThread->setFindType(DISTINGUISH_ING);
}

/* 色度最大值拖动条触发槽 */
void MainWindow::hueMaxSliderSlot(int value)
{
    ui->hueMaxLabel->setText(QString("MAX:%1").arg(value));
    if(ui->hueMinSlider->value() > value)
        ui->hueMinSlider->setValue(value);
    imgThread->setImgParams(HUI_MAX, value);
    if(ui->distinguishCB->isChecked())
        imgThread->setFindType(DISTINGUISH_ING);
}


/* 饱和度最小值拖动条触发槽 */
void MainWindow::satMinSliderSlot(int value)
{
    ui->satMinLabel->setText(QString("MIN:%1").arg(value));
    if(ui->satMaxSlider->value() < value)
        ui->satMaxSlider->setValue(value);
    imgThread->setImgParams(SAT_MIN, value);
    if(ui->distinguishCB->isChecked())
        imgThread->setFindType(DISTINGUISH_ING);
}

/* 饱和度最大值拖动条触发槽 */
void MainWindow::satMaxSliderSlot(int value)
{
    ui->satMaxLabel->setText(QString("MAX:%1").arg(value));
    if(ui->satMinSlider->value() > value)
        ui->satMinSlider->setValue(value);
    imgThread->setImgParams(SAT_MAX, value);
    if(ui->distinguishCB->isChecked())
        imgThread->setFindType(DISTINGUISH_ING);
}

/* 明度最小值拖动条触发槽 */
void MainWindow::valMinSliderSlot(int value)
{
    ui->valMinLabel->setText(QString("MIN:%1").arg(value));
    if(ui->valMaxSlider->value() < value)
        ui->valMaxSlider->setValue(value);
    imgThread->setImgParams(VAL_MIN, value);
    if(ui->distinguishCB->isChecked())
        imgThread->setFindType(DISTINGUISH_ING);
}

/* 明度最大值拖动条触发槽 */
void MainWindow::valMaxSliderSlot(int value)
{
    ui->valMaxLabel->setText(QString("MAX:%1").arg(value));
    if(ui->valMinSlider->value() > value)
        ui->valMinSlider->setValue(value);
    imgThread->setImgParams(VAL_MAX, value);
    if(ui->distinguishCB->isChecked())
        imgThread->setFindType(DISTINGUISH_ING);
}

/* 设置摄像头索引 */
void MainWindow::setCameraIndex(int index)
{
    imgThread->setCameraIndex(index-1);
}

/* 摄像头启动按钮触发槽 */
void MainWindow::imgSwitchBnSlot()
{
    if(!imgThread->isCameraOpen())
    {
        if(ui->cameraCB->currentIndex() == 0)
            QMessageBox::information(this, "警告", "请选择适当的摄像头");
        else if(!imgThread->openCamera())
            QMessageBox::information(this, "警告", "当前摄像头打开失败，请切换摄像头后重试");
        else
            changeCameraUi(false);
    }
    else
    {
        if(!imgThread->closeCamera())
            QMessageBox::information(this, "警告", "像头关闭失败");
        else
        {
            changeCameraUi(true);
        }
    }
}

/* 清除接收区按钮触发槽 */
void MainWindow::clearRecBnSlot()
{
    ui->dataRecLabel->clear();
}

/* 数据发送按钮触发槽 */
void MainWindow::dataSendBnSlot()
{
    spComm->writeData(ui->dataSendLabel->toPlainText());
}

/* 清除发送区按钮触发槽 */
void MainWindow::clearSendBnSlot()
{
    ui->dataSendLabel->clear();
}

/* 十六进制接收复选框槽 */
void MainWindow::hexRecCheckBoxSlot()
{
    spComm->setHexRec(ui->hexRecCheckBox->isChecked());
}

/* 十六进制发送复选框槽 */
void MainWindow::hexSendCheckBoxSlot()
{
    spComm->setHexSend(ui->hexSendCheckBox->isChecked());
}

/* 向前飞 */
void MainWindow::flyFrontBnDown()
{
    flyParams->setFlyControl(BT_FLY_FRONT);
}

/* 向后飞 */
void MainWindow::flyBackBnDown()
{
    flyParams->setFlyControl(BT_FLY_BACK);
}

/* 向左飞 */
void MainWindow::flyLeftBnDown()
{
    flyParams->setFlyControl(BT_FLY_LEFT);
}

/* 向右飞 */
void MainWindow::flyRightBnDown()
{
    flyParams->setFlyControl(BT_FLY_RIGHT);
}

/* 悬停 */
void MainWindow::flyCenter()
{
    flyParams->setFlyControl(BT_FLY_CENTER);
}

/* 解锁 */
void MainWindow::flyUnlockBnDown()
{
    flyParams->setFlyControl(BT_FLY_UNLOCK);
}

/* 上锁 */
void MainWindow::flyLockBnDown()
{
    flyParams->setFlyControl(BT_FLY_LOCK);
}

/* 高度加 */
void MainWindow::flyAttitudeAddBnDown()
{
    flyParams->setFlyControl(BT_ATTITUDE_ADD);
}

/* 高度减 */
void MainWindow::flyAttitudeDownBnDown()
{
    flyParams->setFlyControl(BT_ATTITUDE_DOWN);
}

/* 停机 */
void MainWindow::flyStopBnDown()
{
    flyParams->setFlyControl(BT_FLY_STOP);
}

/* 一键起飞 */
void MainWindow::oneKeyFly()
{
    flyParams->setFlyControl(BT_FLY_ATTITUDE);
}

/* 油门控制模式切换 */
void MainWindow::flyPowerModeBnClick()
{
    flyParams->setFlyControl(BT_FLY_POWER_MODE);
}

/* 超声波定高模式切换 */
void MainWindow::flyAttitudeModeBnClick()
{
    flyParams->setFlyControl(BT_FLY_SWAVE_MODE);
}

/* 紧急停机 */
void MainWindow::flyStopQuickBnClick()
{
    flyParams->setFlyControl(BT_FLY_STOP_QUICK);
}

/* 显示飞控状态复选框触发槽 */
void MainWindow::showFlyStateCBSlot()
{
    if(ui->showFlyStateCB->isChecked())
        if(spComm->isOpen())
            ui->flyStateLabel->setText("\351\243\236\346\216\247\345\267\262\350\277\236\346\216\245");
        else
            ui->flyStateLabel->setText("\351\243\236\346\216\247\346\234\252\350\277\236\346\216\245");
    else
        ui->flyStateLabel->clear();
}


/* 定时器定时触发槽 */
void MainWindow::timerSlot()
{
    QStringList portNames = spComm->getAllPorts();
    while(ui->portCB->count() > portNames.size())
        ui->portCB->removeItem(portNames.size());
    while(ui->portCB->count() < portNames.size())
        ui->portCB->addItem(NULL);
    for(int i = 1; i < portNames.size(); i++)
        ui->portCB->setItemText(i, portNames[i]);

    QStringList cameraNames = imgThread->getAllCameras();
    while(ui->cameraCB->count() > cameraNames.size())
        ui->cameraCB->removeItem(cameraNames.size());
    while(ui->cameraCB->count() < cameraNames.size())
        ui->cameraCB->addItem(NULL);
    for(int i = 1; i < cameraNames.size(); i++)
        ui->cameraCB->setItemText(i, cameraNames[i]);

    if(!imgThread->isCameraOpen() && !ui->cameraCB->isEnabled())
    {
        QMessageBox::information(this, "警告", "摄像头已关闭");
        imgThread->closeCamera();
        changeCameraUi(true);
    }

    if(ui->showFlyStateCB->isChecked())
        if(spComm->isOpen())
            ui->flyStateLabel->setText("\351\243\236\346\216\247\345\267\262\350\277\236\346\216\245");
        else
            ui->flyStateLabel->setText("\351\243\236\346\216\247\346\234\252\350\277\236\346\216\245");
}
