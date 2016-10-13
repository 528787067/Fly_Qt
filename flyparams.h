#ifndef FLYPARAMS_H
#define FLYPARAMS_H

#include "QObject"

/* 通信协议 */
#define UART_HEAD_COM		0x08		// 串口通信数据头
#define UART_FLY_COM		0x01		// 串口通信飞行状态数据尾
#define UART_POWER_COM		0x02		// 串口通信油门数据尾
#define UART_ATTITUDE_COM	0x03		// 串口通信高度数据尾

/* 控制命令 */
#define BT_NO_CHAR			0x00		// 没有接收到字符
#define BT_FLY_UNLOCK		0x01		// 飞控解锁
#define BT_FLY_LOCK			0x02		// 飞控锁定
#define BT_FLY_STOP			0x03		// 安全停机
#define BT_ATTITUDE_ADD		0x04		// 高度加
#define BT_ATTITUDE_DOWN	0x05		// 高度减
#define BT_FLY_CENTER		0x06		// 悬停
#define BT_FLY_FRONT		0x07		// 向前飞
#define BT_FLY_BACK			0x08		// 向后飞
#define BT_FLY_LEFT			0x09		// 向左飞
#define BT_FLY_RIGHT		0x0a		// 向右飞
#define BT_THROW_BOLL		0x0b		// 投球
#define BT_FLY_ATTITUDE     0x0c        // 一键起飞
#define BT_FLY_POWER_MODE	0x0d		// 油门控制模式
#define BT_FLY_SWAVE_MODE	0x0e		// 超声波定高模式
#define BT_FLY_STOP_QUICK   0x0f        // 紧急停机
#define BT_FLY_PITCH_CENTER	0x10		// 俯仰方向悬停
#define BT_FLY_ROLL_CENTER	0x11		// 横滚方向悬停
#define BT_FLY_LEFT_FRONT	0x12		// 左前
#define BT_FLY_LEFT_BACK	0x13		// 左后
#define BT_FLY_RIGHT_FRONT	0x14		// 右前
#define BT_FLY_RIGHT_BACK	0x15		// 右后

typedef enum
{
    FLY_LOCKED,							// 飞控锁定状态
    FLY_STOP,							// 飞控停止状态
    FLY_CENTER,							// 悬停
    FLY_FRONT,							// 向前飞
    FLY_BACK,							// 向后飞
    FLY_LEFT,							// 向左飞
    FLY_RIGHT,							// 向右飞
    FLY_LEFT_FRONT,						// 向左前飞
    FLY_LEFT_BACK,						// 向左后飞
    FLY_RIGHT_FRONT,					// 向右前飞
    FLY_RIGHT_BACK,						// 向右后飞
    FLY_PITCH_CENTER,					// 俯仰方向悬停
    FLY_ROLL_CENTER						// 横滚方向悬停
} FlyState;								// 飞行器状态

class FlyParams : public QObject
{
public:
    ~FlyParams();
    static FlyParams *getFlyParams();
    int getFlyControl();
    void setFlyControl(int com);

private:
    FlyParams();
    static FlyParams *flyParams;
    int flyControl;
};

#endif // FLYPARAMS_H
