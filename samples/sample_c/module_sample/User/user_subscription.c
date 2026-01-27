#include <utils/util_misc.h>
#include <math.h>
#include "dji_fc_subscription.h"
#include "dji_logger.h"
#include "dji_platform.h"
#include "widget_interaction_test/test_widget_interaction.h"
#include "dji_error.h"

#define FC_SUBSCRIPTION_TASK_FREQ (1)

static bool s_userFcSubscriptionDataShow = false;

#ifdef __cplusplus
extern "C"
{
#endif
        T_DjiReturnCode User_FcSubscriptionRunSample(void)
        {
                T_DjiReturnCode djiStat;
                T_DjiOsalHandler *osalHandler = DjiPlatform_GetOsalHandler();
                s_userFcSubscriptionDataShow = true;
                T_DjiFcSubscriptionVelocity velocity = {0};
                T_DjiFcSubscriptionGpsPosition gpsPosition = {0};
                T_DjiDataTimestamp timestamp = {0};

                // 初始化
                djiStat = DjiFcSubscription_Init();
                if (djiStat != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
                {
                        USER_LOG_ERROR("init data subscription module error.");
                        return DJI_ERROR_SYSTEM_MODULE_CODE_UNKNOWN;
                }

                // 订阅数据
                djiStat = DjiFcSubscription_SubscribeTopic(DJI_FC_SUBSCRIPTION_TOPIC_VELOCITY, DJI_DATA_SUBSCRIPTION_TOPIC_1_HZ,
                                                           NULL);
                if (djiStat != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
                {
                        USER_LOG_ERROR("Subscribe topic velocity error.");
                        return DJI_ERROR_SYSTEM_MODULE_CODE_UNKNOWN;
                } // 速度

                djiStat = DjiFcSubscription_SubscribeTopic(DJI_FC_SUBSCRIPTION_TOPIC_GPS_POSITION, DJI_DATA_SUBSCRIPTION_TOPIC_1_HZ,
                                                           NULL);
                if (djiStat != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
                {
                        USER_LOG_ERROR("Subscribe topic gps position error.");
                        return DJI_ERROR_SYSTEM_MODULE_CODE_UNKNOWN;
                } // 位置

                // 显示输出
                // for (int i = 0; i < 10; i++)
                // {
                while(true)
                {
                        osalHandler->TaskSleepMs(1000 / FC_SUBSCRIPTION_TASK_FREQ);
                        djiStat = DjiFcSubscription_GetLatestValueOfTopic(DJI_FC_SUBSCRIPTION_TOPIC_VELOCITY,
                                                                          (uint8_t *)&velocity,
                                                                          sizeof(T_DjiFcSubscriptionVelocity),
                                                                          &timestamp);
                        if (djiStat != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
                        {
                                USER_LOG_ERROR("get value of topic velocity error.");
                        }
                        else
                        {
                                USER_LOG_INFO("velocity: x = %f y = %f z = %f healthFlag = %d, timestamp ms = %d us = %d.", velocity.data.x,
                                              velocity.data.y,
                                              velocity.data.z, velocity.health, timestamp.millisecond, timestamp.microsecond);
                        }

                        djiStat = DjiFcSubscription_GetLatestValueOfTopic(DJI_FC_SUBSCRIPTION_TOPIC_GPS_POSITION,
                                                                          (uint8_t *)&gpsPosition,
                                                                          sizeof(T_DjiFcSubscriptionGpsPosition),
                                                                          &timestamp);
                        if (djiStat != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
                        {
                                USER_LOG_ERROR("get value of topic gps position error.");
                        }
                        else
                        {
                                USER_LOG_INFO("gps position: x = %d y = %d z = %d.", gpsPosition.x, gpsPosition.y, gpsPosition.z);
                        }
                // }
                }

                // 取消订阅
                djiStat = DjiFcSubscription_UnSubscribeTopic(DJI_FC_SUBSCRIPTION_TOPIC_VELOCITY);
                if (djiStat != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
                {
                        USER_LOG_ERROR("UnSubscribe topic quaternion error.");
                        return DJI_ERROR_SYSTEM_MODULE_CODE_UNKNOWN;
                }

                djiStat = DjiFcSubscription_UnSubscribeTopic(DJI_FC_SUBSCRIPTION_TOPIC_GPS_POSITION);
                if (djiStat != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
                {
                        USER_LOG_ERROR("UnSubscribe topic quaternion error.");
                        return DJI_ERROR_SYSTEM_MODULE_CODE_UNKNOWN;
                }

                // 反初始化订阅模块
                djiStat = DjiFcSubscription_DeInit();
                if (djiStat != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
                {
                        USER_LOG_ERROR("Deinit fc subscription error.");
                        return DJI_ERROR_SYSTEM_MODULE_CODE_UNKNOWN;
                }

                s_userFcSubscriptionDataShow = false;
                USER_LOG_INFO("User subscription sample end");

                return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
        }

#ifdef __cplusplus
}
#endif
// T_DjiReturnCode djiStat;
// T_DjiOsalHandler *osalHandler = DjiPlatform_GetOsalHandler();
// T_DjiFcSubscriptionVelocity velocity = {0};
// T_DjiDataTimestamp timestamp = {0};
// T_DjiFcSubscriptionGpsPosition gpsPosition = {0};
// T_DjiFcSubscriptionSingleBatteryInfo singleBatteryInfo = {0};

// T_DjiReturnCode User_FcSubscriptionRunSample(void)
// {
// }

// T_DjiReturnCode Subscription_Init()
// {
//         T_DjiReturnCode djiStat;
//         T_DjiOsalHandler *osalHandler = NULL;

//         // 获取抽象层句柄
//         osalHandler = DjiPlatform_GetOsalHandler();
//         if (osalHandler == NULL)
//         {
//                 USER_LOG_ERROR("Get OSAL handler failed!");
//                 return DJI_ERROR_SYSTEM_MODULE_CODE_UNKNOWN;
//         }

//         // 初始化订阅模块
//         djiStat = DjiFcSubscription_Init();

//         if (djiStat != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
//         {
//                 USER_LOG_ERROR("init data subscription module error.");
//                 return DJI_ERROR_SYSTEM_MODULE_CODE_UNKNOWN;
//         }

//         // 订阅速度
//         djiStat = DjiFcSubscription_SubscribeTopic(
//             DJI_FC_SUBSCRIPTION_TOPIC_VELOCITY,
//             DJI_DATA_SUBSCRIPTION_TOPIC_1_HZ,
//             NULL);

//         if (djiStat != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
//         {
//                 USER_LOG_ERROR("Subscribe topic velocity error.");
//                 return DJI_ERROR_SYSTEM_MODULE_CODE_UNKNOWN;
//         }
//         else
//         {
//                 USER_LOG_DEBUG("Subscribe topic velocity success.");
//         }

//         // 订阅GPS
//         djiStat = DjiFcSubscription_SubscribeTopic(DJI_FC_SUBSCRIPTION_TOPIC_GPS_POSITION, DJI_DATA_SUBSCRIPTION_TOPIC_1_HZ,
//                                                    NULL);
//         if (djiStat != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
//         {
//                 USER_LOG_ERROR("Subscribe topic gps position error.");
//                 return DJI_ERROR_SYSTEM_MODULE_CODE_UNKNOWN;
//         }
//         else
//         {
//                 USER_LOG_DEBUG("Subscribe topic gps position success.");
//         }

//         djiStat = DjiFcSubscription_SubscribeTopic(DJI_FC_SUBSCRIPTION_TOPIC_GPS_DETAILS, DJI_DATA_SUBSCRIPTION_TOPIC_1_HZ,
//                                                    NULL);
//         if (djiStat != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
//         {
//                 USER_LOG_ERROR("Subscribe topic gps details error.");
//                 return DJI_ERROR_SYSTEM_MODULE_CODE_UNKNOWN;
//         }
//         else
//         {
//                 USER_LOG_DEBUG("Subscribe topic gps details success.");
//         }
// }