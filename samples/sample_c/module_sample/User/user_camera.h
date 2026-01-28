/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef USER_CAMERA_H
#define USER_CAMERA_H

/* Includes ------------------------------------------------------------------*/
#include "dji_typedef.h"
#include "dji_fc_subscription.h"
#include "dji_camera_manager.h"
#ifdef __cplusplus
extern "C"
{
#endif

    /* Exported constants --------------------------------------------------------*/
    typedef struct
    {
        E_DjiMountPosition mountPosition;
        E_DjiCameraManagerISO currentISO;
        E_DjiCameraType cameraType;
        uint32_t shootCount;
        uint32_t shootInterval;
        bool isCameraInitialized;
        bool isShooting;
        uint32_t photosTaken;
        char cameraTypeName[32];
    } T_UserCameraStatus;
    /* Exported types ------------------------------------------------------------*/
    // 公共接口函数
    T_DjiReturnCode User_CameraInit(E_DjiMountPosition position);
    T_DjiReturnCode User_CameraGetStatus(T_UserCameraStatus *status);
    T_DjiReturnCode User_CameraSetISO(E_DjiCameraManagerISO iso);
    T_DjiReturnCode User_CameraGetISO(E_DjiCameraManagerISO *iso);
    T_DjiReturnCode User_CameraShootSingle(void);
    T_DjiReturnCode User_CameraShootMulti(uint32_t count, uint32_t intervalMs);
    T_DjiReturnCode User_CameraDeInit(void);
    /* Exported functions --------------------------------------------------------*/
    T_DjiReturnCode User_CameraRunSample(void);
    T_DjiReturnCode User_CameraStartCmdHandler(void);

#ifdef __cplusplus
}
#endif

#endif // TEST_FC_SUBSCRIPTION_H