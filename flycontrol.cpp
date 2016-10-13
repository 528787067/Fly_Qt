#include "flycontrol.h"

FlyControl::FlyControl(QObject *object)
{
    this->imgThread = (ImgThread *)object;
    flyParams = FlyParams::getFlyParams();

    targetBox.radius = 0;
    redBox.radius = 0;
    yellowBox.radius = 0;
    blueBox.radius = 0;
    startPoint.radius = 0;
    flyPoint.radius = 0;
    preFlyPoint.radius = 0;
    flyPointUpdateCount = 0;
    preFlyCom = BT_FLY_CENTER;
    pitchError = 0;
    rollError = 0;

    pitchDirection = UP;
    rollDirection = LEFT;

    isThrow = false;
}

FlyControl::~FlyControl()
{
    this->imgThread = NULL;
    flyParams = NULL;
}

/* 设置目标参数 */
void FlyControl::setIntentParams(SelectType type, CircleParams circleParams)
{
    switch(type)
    {
    case RED_BOX:
        redBox.centerX = circleParams.centerX;
        redBox.centerY = circleParams.centerY;
        redBox.radius = circleParams.radius;
        break;
    case YELLOW_BOX:
        yellowBox.centerX = circleParams.centerX;
        yellowBox.centerY = circleParams.centerY;
        yellowBox.radius = circleParams.radius;
        break;
    case BLUE_BOX:
        blueBox.centerX = circleParams.centerX;
        blueBox.centerY = circleParams.centerY;
        blueBox.radius = circleParams.radius;
        break;
    case START_POINT:
        startPoint.centerX = circleParams.centerX;
        startPoint.centerY = circleParams.centerY;
        startPoint.radius = circleParams.radius;
        break;
    case FLY_POINT:
        flyPoint.centerX = circleParams.centerX;
        flyPoint.centerY = circleParams.centerY;
        flyPoint.radius = circleParams.radius;
        if(preFlyPoint.radius < 1 || flyPoint.radius < 1)
        {
            preFlyPoint.centerX = flyPoint.centerX;
            preFlyPoint.centerY = flyPoint.centerY;
            preFlyPoint.radius = flyPoint.radius;
        }
        break;
    default:
        return;
    }
    if(type != FLY_POINT)
        undateTargetBox(TARGET_BOX);
}

/* 设置俯仰前进方向 */
void FlyControl::setPitchDirection(FlyDirection direction)
{
    this->pitchDirection = direction;
}

/* 设置横滚前进方向 */
void FlyControl::setRollDirection(FlyDirection direction)
{
    this->rollDirection = direction;
}

/* 控制四轴飞行器 */
void FlyControl::controlFly()
{
    /* 还没有选中目标框则直接返回 */
    if(targetBox.radius < 1 || startPoint.radius < 1)
    {
        flyParams->setFlyControl(BT_FLY_CENTER);
        preFlyCom = BT_FLY_CENTER;
        return;
    }

    /* 判断是否需要更新飞行器位置并控制飞行 */
    if(flyPointUpdateCount == 0)
        flyPointUpdateCount++;
    else
    {
        if(++flyPointUpdateCount > FLY_UNDATE_FRAME)
            flyPointUpdateCount = 0;
        flyParams->setFlyControl(preFlyCom);
        return;
    }

    /* 计算俯仰方向和横滚方向到目标的距离 */
    switch(this->pitchDirection)
    {
    case UP:
        pitchDistance = flyPoint.centerY - targetBox.centerY;
        if(this->rollDirection == LEFT)
            rollDistance = flyPoint.centerX - targetBox.centerX;
        else //if(this->rollDirection == RIGHT)
            rollDistance = targetBox.centerX - flyPoint.centerX;
        pitchError = abs(pitchDistance) - abs(targetBox.centerY - preFlyPoint.centerY);
        rollError = abs(rollDistance) - abs(targetBox.centerX - preFlyPoint.centerX);
        break;
    case DOWN:
        pitchDistance = targetBox.centerY - flyPoint.centerY;
        if(this->rollDirection == LEFT)
            rollDistance = flyPoint.centerX - targetBox.centerX;
        else //if(this->rollDirection == RIGHT)
            rollDistance = targetBox.centerX - flyPoint.centerX;
        pitchError = abs(pitchDistance) - abs(targetBox.centerY - preFlyPoint.centerY);
        rollError = abs(rollDistance) - abs(targetBox.centerX - preFlyPoint.centerX);
        break;
    case LEFT:
        pitchDistance = flyPoint.centerX - targetBox.centerX;
        if(this->rollDirection == UP)
            rollDistance = flyPoint.centerY - targetBox.centerY;
        else //if(this->rollDirection == DOWN)
            rollDistance = targetBox.centerY - flyPoint.centerY;
        pitchError = abs(pitchDistance) - abs(targetBox.centerX - preFlyPoint.centerX);
        rollError = abs(rollDistance) - abs(targetBox.centerY - preFlyPoint.centerY);
        break;
    case RIGHT:
        pitchDistance = targetBox.centerX - flyPoint.centerX;
        if(this->rollDirection == UP)
            rollDistance = flyPoint.centerY - targetBox.centerY;
        else //if(this->rollDirection == DOWN)
            rollDistance = targetBox.centerY - flyPoint.centerY;
        pitchError = abs(pitchDistance) - abs(targetBox.centerX - preFlyPoint.centerX);
        rollError = abs(rollDistance) - abs(targetBox.centerY - preFlyPoint.centerY);
        break;
    default:
        return;
    }

    /* 更新飞行器坐标 */
    preFlyPoint.centerX = flyPoint.centerX;
    preFlyPoint.centerY = flyPoint.centerY;
    preFlyPoint.radius = flyPoint.radius;

#if 0    /* 过线投球不返回 */
    if(isThrow)
    {
        flyParams->setFlyControl(BT_FLY_STOP);
        isThrow = false;
        undateTargetBox(TARGET_BOX);
        return;
    }

    if(pitchDistance <= targetBox.radius
    && targetBox.centerX != startPoint.centerX
    && targetBox.centerY != startPoint.centerY
    && targetBox.radius != startPoint.radius
    && !isThrow)
    {
        flyParams->setFlyControl(BT_THROW_BOLL);
        isThrow = true;
        undateTargetBox(TARGET_RETURN);
        return;
    }
#else   /* 进框投球并返回 */
    /* 飞行器进入目标坐标范围 */
    if(abs(pitchDistance) <= targetBox.radius && abs(rollDistance) <= targetBox.radius)
    {
        /* 目标坐标为起始坐标 */
        if(targetBox.centerX == startPoint.centerX
        && targetBox.centerY == startPoint.centerY
        && targetBox.radius == startPoint.radius)
        {            
            flyParams->setFlyControl(BT_FLY_STOP);      // 降落
            undateTargetBox(TARGET_BOX);                // 将目标坐标设置为投球框
            isThrow = false;
        }
        /* 目标坐标为投球框 */
        else
        {
            flyParams->setFlyControl(BT_THROW_BOLL);    // 投球
            undateTargetBox(TARGET_RETURN);             // 将目标坐标设置为起始坐标            
            isThrow = true;
        }
        preFlyCom = BT_FLY_CENTER;
        return;
    }
#endif

    /* 左飞 */
    if((this->pitchDirection == UP && this->rollDirection == LEFT && rollDistance > targetBox.radius)
     ||(this->pitchDirection == UP && this->rollDirection == RIGHT && rollDistance < -targetBox.radius)
     ||(this->pitchDirection == DOWN && this->rollDirection == LEFT && rollDistance < -targetBox.radius)
     ||(this->pitchDirection == DOWN && this->rollDirection == RIGHT && rollDistance > targetBox.radius)
     ||(this->pitchDirection == LEFT && this->rollDirection == UP && rollDistance < -targetBox.radius)
     ||(this->pitchDirection == LEFT && this->rollDirection == DOWN && rollDistance > targetBox.radius)
     ||(this->pitchDirection == RIGHT && this->rollDirection == UP && rollDistance > targetBox.radius)
     ||(this->pitchDirection == RIGHT && this->rollDirection == DOWN && rollDistance < -targetBox.radius))
    {
        /* 前飞 */
        if(pitchDistance > targetBox.radius)
        {
            // 向左前方向飘移则发悬停指令
            if(pitchError <= -FLY_MIN_POINT && rollError <= -FLY_MIN_POINT)
            {
                flyParams->setFlyControl(BT_FLY_CENTER);
                preFlyCom = BT_FLY_CENTER;
            }
            // 向前方飘移则发向左飞指令
            else if(pitchError <= -FLY_MIN_POINT && rollError > FLY_MIN_POINT)
            {
                flyParams->setFlyControl(BT_FLY_LEFT);
                preFlyCom = BT_FLY_LEFT;
            }
            // 向左边飘移则发向前指令
            else if(pitchError > FLY_MIN_POINT && rollError <= -FLY_MIN_POINT)
            {
                flyParams->setFlyControl(BT_FLY_FRONT);
                preFlyCom = BT_FLY_FRONT;
            }
            // 两个方向都没有向目标飘移则发左前指令
            else //if(pitchError > FLY_MIN_POINT && rollError > FLY_MIN_POINT)
            {
                flyParams->setFlyControl(BT_FLY_LEFT_FRONT);
                preFlyCom = BT_FLY_LEFT_FRONT;
            }
        }
        /* 后飞 */
        else if(pitchDistance < -targetBox.radius)
        {
            // 向左后方向飘移则发悬停指令
            if(pitchError <= -FLY_MIN_POINT && rollError <= -FLY_MIN_POINT)
            {
                flyParams->setFlyControl(BT_FLY_CENTER);
                preFlyCom = BT_FLY_CENTER;
            }
            // 向后方飘移则发向左指令
            else if(pitchError <= -FLY_MIN_POINT && rollError > FLY_MIN_POINT)
            {
                flyParams->setFlyControl(BT_FLY_LEFT);
                preFlyCom = BT_FLY_LEFT;
            }
            // 向左边飘移则发向后指令
            else if(pitchError > FLY_MIN_POINT && rollError <= -FLY_MIN_POINT)
            {
                flyParams->setFlyControl(BT_FLY_BACK);
                preFlyCom = BT_FLY_BACK;
            }
            // 两个方向都没有向目标飘移则发左前指令
            else //if(pitchError > FLY_MIN_POINT && rollError > FLY_MIN_POINT)
            {
                flyParams->setFlyControl(BT_FLY_LEFT_BACK);
                preFlyCom = BT_FLY_LEFT_BACK;
            }
        }
        /* 俯仰方向悬停 */
        else
        {
            // 向左飘则发悬停指令
            if(rollError <= -FLY_MIN_POINT)
            {
                flyParams->setFlyControl(BT_FLY_CENTER);
                preFlyCom = BT_FLY_CENTER;
            }
            // 横滚方向没有向目标飘则发向左指令
            else
            {
                flyParams->setFlyControl(BT_FLY_LEFT);
                preFlyCom = BT_FLY_LEFT;
            }
        }
    }
    /* 右飞 */
    else if((this->pitchDirection == UP && this->rollDirection == LEFT && rollDistance < -targetBox.radius)
          ||(this->pitchDirection == UP && this->rollDirection == RIGHT && rollDistance > targetBox.radius)
          ||(this->pitchDirection == DOWN && this->rollDirection == LEFT && rollDistance > targetBox.radius)
          ||(this->pitchDirection == DOWN && this->rollDirection == RIGHT && rollDistance < -targetBox.radius)
          ||(this->pitchDirection == LEFT && this->rollDirection == UP && rollDistance > targetBox.radius)
          ||(this->pitchDirection == LEFT && this->rollDirection == DOWN && rollDistance < -targetBox.radius)
          ||(this->pitchDirection == RIGHT && this->rollDirection == UP && rollDistance < targetBox.radius)
          ||(this->pitchDirection == RIGHT && this->rollDirection == DOWN && rollDistance > targetBox.radius))
    {
        /* 前飞 */
        if(pitchDistance > targetBox.radius)
        {
            // 向右前方向飘移则发悬停指令
            if(pitchError <= -FLY_MIN_POINT && rollError <= -FLY_MIN_POINT)
            {
                flyParams->setFlyControl(BT_FLY_CENTER);
                preFlyCom = BT_FLY_CENTER;
            }
            // 向前方飘移则发向右飞指令
            else if(pitchError <= -FLY_MIN_POINT && rollError > FLY_MIN_POINT)
            {
                flyParams->setFlyControl(BT_FLY_RIGHT);
                preFlyCom = BT_FLY_RIGHT;
            }
            // 向右边飘移则发向前指令
            else if(pitchError > FLY_MIN_POINT && rollError <= -FLY_MIN_POINT)
            {
                flyParams->setFlyControl(BT_FLY_FRONT);
                preFlyCom = BT_FLY_FRONT;
            }
            // 两个方向都没有向目标飘移则发右前指令
            else //if(pitchError > FLY_MIN_POINT && rollError > FLY_MIN_POINT)
            {
                flyParams->setFlyControl(BT_FLY_RIGHT_FRONT);
                preFlyCom = BT_FLY_RIGHT_FRONT;
            }
        }
        /* 后飞 */
        else if(pitchDistance < -targetBox.radius)
        {
            // 向右后方向飘移则发悬停指令
            if(pitchError <= -FLY_MIN_POINT && rollError <= -FLY_MIN_POINT)
            {
                flyParams->setFlyControl(BT_FLY_CENTER);
                preFlyCom = BT_FLY_CENTER;
            }
            // 向后方飘移则发向右飞指令
            else if(pitchError <= -FLY_MIN_POINT && rollError > FLY_MIN_POINT)
            {
                flyParams->setFlyControl(BT_FLY_RIGHT);
                preFlyCom = BT_FLY_RIGHT;
            }
            // 向右边飘移则发向后指令
            else if(pitchError > FLY_MIN_POINT && rollError <= -FLY_MIN_POINT)
            {
                flyParams->setFlyControl(BT_FLY_BACK);
                preFlyCom = BT_FLY_BACK;
            }
            // 两个方向都没有向目标飘移则发右后指令
            else //if(pitchError > FLY_MIN_POINT && rollError > FLY_MIN_POINT)
            {
                flyParams->setFlyControl(BT_FLY_RIGHT_BACK);
                preFlyCom = BT_FLY_RIGHT_BACK;
            }
        }
        /* 俯仰方向悬停 */
        else
        {
            // 向右飘则发悬停指令
            if(rollError <= -FLY_MIN_POINT)
            {
                flyParams->setFlyControl(BT_FLY_CENTER);
                preFlyCom = BT_FLY_CENTER;
            }
            // 横滚方向没有向目标飘则发向右指令
            else
            {
                flyParams->setFlyControl(BT_FLY_RIGHT);
                preFlyCom = BT_FLY_RIGHT;
            }
        }
    }
    /* 横滚方向悬停 */
    else
    {
        /* 前飞 */
        if(pitchDistance > targetBox.radius)
        {
            // 向前飘则发悬停指令
            if(pitchError <= -FLY_MIN_POINT)
            {
                flyParams->setFlyControl(BT_FLY_CENTER);
                preFlyCom = BT_FLY_CENTER;
            }
            // 俯仰方向没有向前飘则发向前指令
            else
            {
                flyParams->setFlyControl(BT_FLY_FRONT);
                preFlyCom = BT_FLY_FRONT;
            }
        }
        /* 后飞 */
        else if(pitchDistance < -targetBox.radius)
        {
            // 向后飘则发悬停指令
            if(pitchError <= -FLY_MIN_POINT)
            {
                flyParams->setFlyControl(BT_FLY_CENTER);
                preFlyCom = BT_FLY_CENTER;
            }
            // 俯仰方向没有向前飘则发向后指令
            else
            {
                flyParams->setFlyControl(BT_FLY_BACK);
                preFlyCom = BT_FLY_BACK;
            }
        }
        /* 俯仰方向悬停 */
        else
        {
            flyParams->setFlyControl(BT_FLY_CENTER);
            preFlyCom = BT_FLY_CENTER;
        }
    }
}

/* 设置目标类型 */
void FlyControl::undateTargetBox(TargetType type)
{
    if(type == TARGET_BOX)
        if(yellowBox.radius > 0)
        {
            targetBox.centerX = yellowBox.centerX;
            targetBox.centerY = yellowBox.centerY;
            targetBox.radius = yellowBox.radius;
        }
        else if(redBox.radius > 0)
        {
            targetBox.centerX = redBox.centerX;
            targetBox.centerY = redBox.centerY;
            targetBox.radius = redBox.radius;
        }
        else if(blueBox.radius > 0)
        {
            targetBox.centerX = blueBox.centerX;
            targetBox.centerY = blueBox.centerY;
            targetBox.radius = blueBox.radius;
        }
        else
            targetBox.radius = 0;
    else if(type == TARGET_RETURN)
    {
        targetBox.centerX = startPoint.centerX;
        targetBox.centerY = startPoint.centerY;
        targetBox.radius = startPoint.radius;
    }
    else;
}
