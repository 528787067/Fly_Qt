#include "flyparams.h"

FlyParams *FlyParams::flyParams = new FlyParams();
FlyParams *FlyParams::getFlyParams()
{
    if(flyParams == NULL)
        flyParams = new FlyParams();
    return flyParams;
}

int FlyParams::getFlyControl()
{
    return flyControl;
}

void FlyParams::setFlyControl(int com)
{
    flyControl = com;
}


FlyParams::FlyParams()
{
    flyControl = BT_FLY_CENTER;
}

FlyParams::~FlyParams()
{
    delete flyParams;
    flyParams = NULL;
}
