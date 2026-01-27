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

T_DjiReturnCode DjiTest_CameraManagerSetExposureMode(E_DjiMountPosition position,
                                                     E_DjiCameraManagerExposureMode exposureMode)
{
    T_DjiReturnCode returnCode;
    E_DjiCameraManagerExposureMode exposureModeTemp;

    returnCode = DjiCameraManager_GetExposureMode(position, &exposureModeTemp);
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS &&
        returnCode != DJI_ERROR_CAMERA_MANAGER_MODULE_CODE_UNSUPPORTED_COMMAND)
    {
        USER_LOG_ERROR("Get mounted position %d exposure mode failed, error code: 0x%08X",
                       position, returnCode);
        return returnCode;
    }

    if (exposureModeTemp == exposureMode)
    {
        USER_LOG_INFO("The mounted position %d camera's exposure mode is already what you expected.",
                      position);
        return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
    }

    returnCode = DjiCameraManager_SetExposureMode(position, exposureMode);
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS &&
        returnCode != DJI_ERROR_CAMERA_MANAGER_MODULE_CODE_UNSUPPORTED_COMMAND)
    {
        USER_LOG_ERROR("Set mounted position %d camera's exposure mode %d failed, current exposure is %d,"
                       " error code: 0x%08X",
                       position, exposureMode, exposureModeTemp, returnCode);
    }

    return returnCode;
}

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

        E_DjiCameraManagerISO isoData;
        USER_LOG_INFO("Available ISO values:");
        USER_LOG_INFO("0: AUTO(0), 1: ISO_100(3), 2: ISO_200(4), 3: ISO_400(5)");
        USER_LOG_INFO("4: ISO_800(6), 5: ISO_1600(7), 6: ISO_3200(8), 7: ISO_6400(9)");
        USER_LOG_INFO("8: ISO_12800(10), 9: ISO_25600(11)");
        int isoInput;
        scanf("%d", &isoInput);

        switch (isoInput)
        {
        case 0:
            isoData = DJI_CAMERA_MANAGER_ISO_AUTO;
            break;
        case 1:
            isoData = DJI_CAMERA_MANAGER_ISO_100;
            break;
        case 2:
            isoData = DJI_CAMERA_MANAGER_ISO_200;
            break;
        case 3:
            isoData = DJI_CAMERA_MANAGER_ISO_400;
            break;
        case 4:
            isoData = DJI_CAMERA_MANAGER_ISO_800;
            break;
        case 5:
            isoData = DJI_CAMERA_MANAGER_ISO_1600;
            break;
        case 6:
            isoData = DJI_CAMERA_MANAGER_ISO_3200;
            break;
        case 7:
            isoData = DJI_CAMERA_MANAGER_ISO_6400;
            break;
        case 8:
            isoData = DJI_CAMERA_MANAGER_ISO_12800;
            break;
        case 9:
            isoData = DJI_CAMERA_MANAGER_ISO_25600;
            break;
        default:
            USER_LOG_ERROR("Invalid ISO number");
            goto exitCameraModule;
        }

        // 设置曝光模式
        USER_LOG_INFO("Setting exposure mode to manual...");
        returnCode = DjiTest_CameraManagerSetExposureMode(mountPosition, DJI_CAMERA_MANAGER_EXPOSURE_MODE_EXPOSURE_MANUAL);
        if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
        {
            USER_LOG_ERROR("Set exposure mode failed, error code: 0x%08X", returnCode);
            goto exitCameraModule;
        }

        // 获取ISO参数
        USER_LOG_INFO("Getting current ISO value...");
        returnCode = DjiCameraManager_GetISO(mountPosition, &isoDataTemp);
        if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS &&
            returnCode != DJI_ERROR_CAMERA_MANAGER_MODULE_CODE_UNSUPPORTED_COMMAND)
        {
            USER_LOG_ERROR("Get mounted position %d camera's iso failed, error code: 0x%08X.",
                           mountPosition, returnCode);
            goto exitCameraModule;
        }
        USER_LOG_INFO("Current ISO: %d, Target ISO: %d", isoDataTemp, isoData);

        // 参数判断
        if (isoDataTemp == isoData)
        {
            USER_LOG_INFO("The mounted position %d camera's iso is already what you expected.",
                          mountPosition);
        }
        else
        {
            // 设置ISO参数
            USER_LOG_INFO("Setting ISO to %d...", isoData);
            returnCode = DjiCameraManager_SetISO(mountPosition, isoData);
            if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS &&
                returnCode != DJI_ERROR_CAMERA_MANAGER_MODULE_CODE_UNSUPPORTED_COMMAND)
            {
                USER_LOG_ERROR("Set mounted position %d camera's iso %d failed, "
                               "error code: 0x%08X.",
                               mountPosition, isoData, returnCode);
                goto exitCameraModule;
            }
            USER_LOG_INFO("ISO set successfully to %d", isoData);
        }
        break;
    }
    case 2:
    {
        int shootCount = 0;
        int shootMode = 0;
        int shootInterval = 0; // 拍照间隔（毫秒）
        int i;

        USER_LOG_INFO("Please select shoot mode:");
        USER_LOG_INFO("1: Single shoot (single photo)");
        USER_LOG_INFO("2: Multi shoot (multiple photos with interval)");
        USER_LOG_INFO("3: Continuous shoot (continuous mode)");
        scanf("%d", &shootMode);

        if (shootMode < 1 || shootMode > 3)
        {
            USER_LOG_ERROR("Invalid shoot mode");
            goto exitCameraModule;
        }
        if (shootMode == 1)
        {
            // 单次拍照
            USER_LOG_INFO("Mounted position %d camera start to shoot photo", mountPosition);
            returnCode = DjiCameraManager_StartShootPhoto(mountPosition, DJI_CAMERA_MANAGER_SHOOT_PHOTO_MODE_SINGLE);
            if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
            {
                USER_LOG_ERROR("Mounted position %d camera shoot photo failed, "
                               "error code :0x%08X",
                               mountPosition, returnCode);
                goto exitCameraModule;
            }
            USER_LOG_INFO("Single photo taken successfully");
        }
        else if (shootMode == 2)
        {
            // 多次拍照（带间隔）
            USER_LOG_INFO("Please enter shoot count (1-100):");
            scanf("%d", &shootCount);
            if (shootCount < 1 || shootCount > 100)
            {
                USER_LOG_ERROR("Invalid shoot count, please use 1-100");
                goto exitCameraModule;
            }

            USER_LOG_INFO("Please enter shoot interval in milliseconds (100-5000):");
            scanf("%d", &shootInterval);
            if (shootInterval < 100 || shootInterval > 5000)
            {
                USER_LOG_ERROR("Invalid shoot interval, please use 100-5000ms");
                goto exitCameraModule;
            }

            USER_LOG_INFO("Starting to shoot %d photos with %dms interval...", shootCount, shootInterval);

            T_DjiOsalHandler *osalHandler = DjiPlatform_GetOsalHandler();

            for (i = 0; i < shootCount; i++)
            {
                USER_LOG_INFO("Shooting photo %d/%d...", i + 1, shootCount);
                returnCode = DjiCameraManager_StartShootPhoto(mountPosition, DJI_CAMERA_MANAGER_SHOOT_PHOTO_MODE_SINGLE);
                if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
                {
                    USER_LOG_ERROR("Photo %d/%d failed, error code: 0x%08X", i + 1, shootCount, returnCode);
                    break;
                }
                USER_LOG_INFO("Photo %d/%d taken successfully", i + 1, shootCount);

                // 间隔等待
                if (i < shootCount - 1)
                {
                    osalHandler->TaskSleepMs(shootInterval);
                }
            }

            if (i == shootCount)
            {
                USER_LOG_INFO("All %d photos taken successfully", shootCount);
            }
        }
        else if (shootMode == 3)
        {
            // 连拍模式
            USER_LOG_INFO("Please enter shoot count for continuous mode (1-100):");
            scanf("%d", &shootCount);
            if (shootCount < 1 || shootCount > 100)
            {
                USER_LOG_ERROR("Invalid shoot count, please use 1-100");
                goto exitCameraModule;
            }

            USER_LOG_INFO("Starting burst shoot mode for %d photos...", shootCount);
            returnCode = DjiCameraManager_StartShootPhoto(mountPosition, DJI_CAMERA_MANAGER_SHOOT_PHOTO_MODE_BURST);
            if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
            {
                USER_LOG_ERROR("Burst shoot failed, error code: 0x%08X", returnCode);
                goto exitCameraModule;
            }
            USER_LOG_INFO("Burst shoot started successfully");
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
