#include <math.h>
#include "dji_fc_subscription.h"
#include "dji_logger.h"
#include "dji_platform.h"
#include "widget_interaction_test/test_widget_interaction.h"
#include "dji_error.h"
#include "dji_camera_manager.h"

static uint8_t DjiTest_CameraManagerGetCameraTypeIndex(E_DjiCameraType cameraType);
typedef struct
{
    E_DjiCameraType cameraType;
    char *cameraTypeStr;
} T_DjiTestCameraTypeStr;

/* Private values -------------------------------------------------------------*/
static const T_DjiTestCameraTypeStr s_cameraTypeStrList[] = {
    {DJI_CAMERA_TYPE_UNKNOWN, "Unknown"},
    {DJI_CAMERA_TYPE_Z30, "Zenmuse Z30"},
    {DJI_CAMERA_TYPE_XT2, "Zenmuse XT2"},
    {DJI_CAMERA_TYPE_PSDK, "Payload Camera"},
    {DJI_CAMERA_TYPE_XTS, "Zenmuse XTS"},
    {DJI_CAMERA_TYPE_H20, "Zenmuse H20"},
    {DJI_CAMERA_TYPE_H20T, "Zenmuse H20T"},
    {DJI_CAMERA_TYPE_P1, "Zenmuse P1"},
    {DJI_CAMERA_TYPE_L1, "Zenmuse L1"},
    {DJI_CAMERA_TYPE_L2, "Zenmuse L2"},
    {DJI_CAMERA_TYPE_H20N, "Zenmuse H20N"},
    {DJI_CAMERA_TYPE_M30, "M30 Camera"},
    {DJI_CAMERA_TYPE_M30T, "M30T Camera"},
    {DJI_CAMERA_TYPE_M3E, "M3E Camera"},
    {DJI_CAMERA_TYPE_M3T, "M3T Camera"},
    {DJI_CAMERA_TYPE_M3D, "M3D Camera"},
    {DJI_CAMERA_TYPE_M3TD, "M3TD Camera"},
    {DJI_CAMERA_TYPE_H30, "H30 Camera"},
    {DJI_CAMERA_TYPE_H30T, "H30T Camera"},
    {DJI_CAMERA_TYPE_M4T, "M4T Camera"},
    {DJI_CAMERA_TYPE_M4E, "M4E Camera"},
};

T_DjiReturnCode User_CameraRunSample(void)
{
    T_DjiReturnCode returnCode;
    E_DjiMountPosition mountPosition;
    T_DjiCameraManagerFirmwareVersion firmwareVersion;
    E_DjiCameraType cameraType;
    E_DjiCameraManagerISO isoDataTemp;
    int select_number;

    USER_LOG_INFO("Please select mount position (1-3): ");
    scanf("%d", &mountPosition);

    // 相机模块初始化
    returnCode = DjiCameraManager_Init();
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
    {
        USER_LOG_ERROR("Init camera manager failed, error code: 0x%08X\r\n", returnCode);
        goto exitCameraModule;
    }

    // 版本获取
    returnCode = DjiCameraManager_GetCameraType(mountPosition, &cameraType);
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
    {
        USER_LOG_ERROR("Get mounted position %d camera's type failed, error code: 0x%08X\r\n",
                       mountPosition, returnCode);
        goto exitCameraModule;
    }
    USER_LOG_INFO("Mounted position %d camera's type is %s",
                  mountPosition,
                  s_cameraTypeStrList[DjiTest_CameraManagerGetCameraTypeIndex(cameraType)].cameraTypeStr);

    returnCode = DjiCameraManager_GetFirmwareVersion(mountPosition, &firmwareVersion);
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
    {
        USER_LOG_ERROR("Get mounted position %d camera's firmware version failed, error code: 0x%08X\r\n",
                       mountPosition, returnCode);
        goto exitCameraModule;
    }
    USER_LOG_INFO("Mounted position %d camera's firmware is V%02d.%02d.%02d.%02d\r\n", mountPosition,
                  firmwareVersion.firmware_version[0], firmwareVersion.firmware_version[1],
                  firmwareVersion.firmware_version[2], firmwareVersion.firmware_version[3]);

    USER_LOG_INFO("select_number: 1 - ISO set, 2 - StartShoot");
    scanf("%d", &select_number);

    switch (select_number)
    {
    case 1:
    {
        int isoData = 0;
        USER_LOG_INFO("isoData:");
        scanf("%d", &isoData);

        // 获取ISO参数
        returnCode = DjiCameraManager_GetISO(mountPosition, &isoDataTemp);
        if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS &&
            returnCode != DJI_ERROR_CAMERA_MANAGER_MODULE_CODE_UNSUPPORTED_COMMAND)
        {
            USER_LOG_ERROR("Get mounted position %d camera's iso failed, error code: 0x%08X.",
                           mountPosition, returnCode);
            goto exitCameraModule;
        }
        // 参数判断
        if (isoDataTemp == isoData)
        {
            USER_LOG_INFO("The mounted position %d camera's iso is already what you expected.",
                          mountPosition);
            return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
        }
        // 设置ISO参数
        returnCode = DjiCameraManager_SetISO(mountPosition, isoData);
        if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS &&
            returnCode != DJI_ERROR_CAMERA_MANAGER_MODULE_CODE_UNSUPPORTED_COMMAND)
        {
            USER_LOG_ERROR("Set mounted position %d camera's iso %d failed, "
                           "error code: 0x%08X.",
                           mountPosition, isoData, returnCode);
            goto exitCameraModule;
        }
        break;
    }
    case 2:
    {
        USER_LOG_INFO("Mounted position %d camera start to shoot photo", mountPosition);
        returnCode = DjiCameraManager_StartShootPhoto(mountPosition, DJI_CAMERA_MANAGER_SHOOT_PHOTO_MODE_SINGLE);
        if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
        {
            USER_LOG_ERROR("Mounted position %d camera shoot photo failed, "
                           "error code :0x%08X",
                           mountPosition, returnCode);
            goto exitCameraModule;
        }
        break;
    }

    default:
        break;
    }
// 相机模块反初始化
exitCameraModule:
    returnCode = DjiCameraManager_DeInit();
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
    {
        USER_LOG_ERROR("Camera manager deinit failed ,error code :0x%08X", returnCode);
    }

    USER_LOG_INFO("User Camera sample end");
    DjiTest_WidgetLogAppend("User Camera sample end");
    return returnCode;
}

static uint8_t DjiTest_CameraManagerGetCameraTypeIndex(E_DjiCameraType cameraType)
{
    uint8_t i;

    for (i = 0; i < sizeof(s_cameraTypeStrList) / sizeof(s_cameraTypeStrList[0]); i++)
    {
        if (s_cameraTypeStrList[i].cameraType == cameraType)
        {
            return i;
        }
    }

    return 0;
}