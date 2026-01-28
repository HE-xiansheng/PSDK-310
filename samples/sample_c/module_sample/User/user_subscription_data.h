#ifndef USER_SUBSCRIPTION_DATA_H
#define USER_SUBSCRIPTION_DATA_H

#include <dji_typedef.h>
#include <stdbool.h>

#include <dji_fc_subscription.h>

// 订阅数据结构
typedef struct
{
    // 速度数据
    T_DjiFcSubscriptionVelocity velocity;
    uint64_t velocityTimestamp;
    bool hasVelocity;

    // 位置数据
    T_DjiFcSubscriptionGpsPosition gpsPosition;
    uint64_t positionTimestamp;
    bool hasPosition;

} T_UserSubscriptionData;

// 获取订阅数据的函数
T_UserSubscriptionData *User_Subscription_GetData(void);

#endif // USER_SUBSCRIPTION_DATA_H