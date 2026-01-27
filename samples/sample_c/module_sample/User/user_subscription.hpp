/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef USER_SUBSCRIPTION_H
#define USER_SUBSCRIPTION_H

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
    T_DjiReturnCode User_FcSubscriptionRunSample(void);

#ifdef __cplusplus
}
#endif

#endif // TEST_FC_SUBSCRIPTION_H