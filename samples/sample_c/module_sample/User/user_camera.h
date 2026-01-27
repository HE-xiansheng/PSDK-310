/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef USER_CAMERA_H
#define USER_CAMERA_H

/* Includes ------------------------------------------------------------------*/
#include "dji_typedef.h"
#include "dji_fc_subscription.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /* Exported constants --------------------------------------------------------*/

    /* Exported types ------------------------------------------------------------*/

    /* Exported functions --------------------------------------------------------*/
    T_DjiReturnCode User_CameraRunSample(void);

#ifdef __cplusplus
}
#endif

#endif // TEST_FC_SUBSCRIPTION_H