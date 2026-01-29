#include <math.h>
#include "dji_fc_subscription.h"
#include "dji_logger.h"
#include "dji_platform.h"
#include "widget_interaction_test/test_widget_interaction.h"
#include "dji_error.h"
#include "dji_camera_manager.h"
#include "user_camera_data.h"
#include "user_camera.h"
#include "../utils/util_misc.h" // 添加这一行

// 函数声明（在 test_camera_manager.c 中定义）
T_DjiReturnCode DjiTest_CameraManagerSetExposureMode(E_DjiMountPosition position,
                                                     E_DjiCameraManagerExposureMode exposureMode);
static uint8_t DjiTest_CameraManagerGetCameraTypeIndex(E_DjiCameraType cameraType);
static E_DjiMountPosition s_cameraMountPosition = 0;
static void *User_CameraCmdHandlerTask(void *arg); // 新增

typedef struct
{
    E_DjiCameraType cameraType;
    char *cameraTypeStr;
} T_DjiTestCameraTypeStr;

static T_UserCameraCmd s_cameraCmd = {0};
static T_UserCameraStatus s_cameraStatus = {0};

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

T_UserCameraCmd *User_Camera_GetCmd(void)
{
    return &s_cameraCmd;
} // 获取命令

T_DjiReturnCode User_CameraShootSingle(void)
{
    T_DjiReturnCode returnCode;
    T_DjiOsalHandler *osalHandler = DjiPlatform_GetOsalHandler();
    E_DjiCameraManagerWorkMode workMode;

    USER_LOG_INFO("Setting camera work mode to SHOOT_PHOTO");

    // 1. 设置相机工作模式为拍照模式
    returnCode = DjiCameraManager_SetMode(s_cameraMountPosition,
                                          DJI_CAMERA_MANAGER_WORK_MODE_SHOOT_PHOTO);
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS &&
        returnCode != DJI_ERROR_CAMERA_MANAGER_MODULE_CODE_UNSUPPORTED_COMMAND)
    {
        USER_LOG_ERROR("Set work mode failed, error: 0x%08X", returnCode);
        return returnCode;
    }

    // 2. 等待模式切换
    osalHandler->TaskSleepMs(1000);

    // 3. 确认工作模式
    returnCode = DjiCameraManager_GetMode(s_cameraMountPosition, &workMode);
    USER_LOG_INFO("Camera work mode: %d", workMode);

    // 4. 设置拍照模式为单次拍照
    USER_LOG_INFO("Setting shoot photo mode to SINGLE");
    returnCode = DjiCameraManager_SetShootPhotoMode(s_cameraMountPosition,
                                                    DJI_CAMERA_MANAGER_SHOOT_PHOTO_MODE_SINGLE);
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
    {
        USER_LOG_ERROR("Set shoot photo mode failed, error: 0x%08X", returnCode);
        return returnCode;
    }

    // 5. 开始拍照
    USER_LOG_INFO("Starting shoot photo...");
    returnCode = DjiCameraManager_StartShootPhoto(s_cameraMountPosition,
                                                  DJI_CAMERA_MANAGER_SHOOT_PHOTO_MODE_SINGLE);
    return returnCode;
}

T_DjiReturnCode User_CameraSetISO(E_DjiCameraManagerISO iso)
{
    T_DjiReturnCode returnCode;

    returnCode = DjiCameraManager_SetISO(s_cameraMountPosition, iso);
    if (returnCode == DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
    {
        s_cameraStatus.currentISO = iso;
    }

    return returnCode;
} // 相机ISO设置函数

void User_Camera_ClearCmd(void)
{
    s_cameraCmd.cmdType = USER_CAMERA_CMD_NONE;
    s_cameraCmd.hasPendingCmd = false;
} // 清除命令

T_DjiReturnCode User_CameraInit(E_DjiMountPosition position)
{
    T_DjiReturnCode returnCode;

    returnCode = DjiCameraManager_Init();
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
    {
        USER_LOG_ERROR("Camera init failed, error: 0x%08X", returnCode);
        return returnCode;
    }

    s_cameraMountPosition = position;
    s_cameraStatus.mountPosition = position;
    s_cameraStatus.isCameraInitialized = true;

    // 获取相机类型
    returnCode = DjiCameraManager_GetCameraType(position, &s_cameraStatus.cameraType);
    if (returnCode == DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
    {
        uint8_t index = DjiTest_CameraManagerGetCameraTypeIndex(s_cameraStatus.cameraType);
        strncpy(s_cameraStatus.cameraTypeName,
                s_cameraTypeStrList[index].cameraTypeStr,
                sizeof(s_cameraStatus.cameraTypeName) - 1);
    }

    return returnCode;
} // 相机初始化函数

T_DjiReturnCode User_CameraStartCmdHandler(void)
{
    T_DjiReturnCode returnCode;
    T_DjiOsalHandler *osalHandler = DjiPlatform_GetOsalHandler();
    static T_DjiTaskHandle s_cameraCmdThread;

    if (osalHandler->TaskCreate("camera_cmd_task",
                                User_CameraCmdHandlerTask,
                                2048,
                                NULL,
                                &s_cameraCmdThread) != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
    {
        USER_LOG_ERROR("Camera command task create error.");
        return DJI_ERROR_SYSTEM_MODULE_CODE_UNKNOWN;
    }

    USER_LOG_INFO("Camera command handler started");
    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
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
    s_cameraMountPosition = mountPosition;
    returnCode = User_CameraStartCmdHandler();
    if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
    {
        USER_LOG_ERROR("Start camera command handler failed, error: 0x%08X", returnCode);
    }

    //版本获取
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

static void *User_CameraCmdHandlerTask(void *arg)
{
    T_DjiReturnCode returnCode;
    T_DjiOsalHandler *osalHandler = DjiPlatform_GetOsalHandler();

    USER_UTIL_UNUSED(arg);

    // 等待相机初始化完成
    USER_LOG_INFO("Camera command handler task started, waiting for camera ready...");
    osalHandler->TaskSleepMs(1000);

    while (1)
    {
        osalHandler->TaskSleepMs(50);

        if (!s_cameraCmd.hasPendingCmd)
        {
            continue;
        }

        USER_LOG_INFO("Processing camera command, type=%d", s_cameraCmd.cmdType);

        switch (s_cameraCmd.cmdType)
        {
        case USER_CAMERA_CMD_SHOOT_SINGLE:
            USER_LOG_INFO("Camera command: Shoot single photo");
            returnCode = User_CameraShootSingle();
            if (returnCode == DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
            {
                USER_LOG_INFO("Photo shoot SUCCESS");
            }
            else
            {
                USER_LOG_ERROR("Photo shoot FAILED, error code: 0x%08X", returnCode);
            }
            break;

        case USER_CAMERA_CMD_SHOOT_MULTI:
            USER_LOG_INFO("Camera command: Shoot %d photos with %dms interval",
                          s_cameraCmd.cmdParam.shootMulti.count,
                          s_cameraCmd.cmdParam.shootMulti.intervalMs);
            returnCode = User_CameraShootMulti(s_cameraCmd.cmdParam.shootMulti.count,
                                               s_cameraCmd.cmdParam.shootMulti.intervalMs);
            if (returnCode == DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
            {
                USER_LOG_INFO("Multi photo shoot SUCCESS");
            }
            else
            {
                USER_LOG_ERROR("Multi photo shoot FAILED, error code: 0x%08X", returnCode);
            }
            break;

        case USER_CAMERA_CMD_SET_ISO:
            USER_LOG_INFO("Camera command: Set ISO to %d",
                          s_cameraCmd.cmdParam.setISO.isoValue);
            returnCode = User_CameraSetISO((E_DjiCameraManagerISO)s_cameraCmd.cmdParam.setISO.isoValue);
            if (returnCode == DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
            {
                USER_LOG_INFO("ISO set SUCCESS");
            }
            else
            {
                USER_LOG_ERROR("ISO set FAILED, error code: 0x%08X", returnCode);
            }
            break;

        default:
            USER_LOG_WARN("Unknown camera command type: %d", s_cameraCmd.cmdType);
            break;
        }

        User_Camera_ClearCmd();
        USER_LOG_INFO("Camera command cleared");
    }
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

T_DjiReturnCode User_CameraGetISO(E_DjiCameraManagerISO *iso)
{
    return DjiCameraManager_GetISO(s_cameraMountPosition, iso);
} // 获取ISO

T_DjiReturnCode User_CameraShootMulti(uint32_t count, uint32_t intervalMs)
{
    T_DjiReturnCode returnCode;
    T_DjiOsalHandler *osalHandler = DjiPlatform_GetOsalHandler();

    s_cameraStatus.shootCount = count;
    s_cameraStatus.shootInterval = intervalMs;

    for (uint32_t i = 0; i < count; i++)
    {
        returnCode = DjiCameraManager_StartShootPhoto(s_cameraMountPosition,
                                                      DJI_CAMERA_MANAGER_SHOOT_PHOTO_MODE_SINGLE);
        if (returnCode != DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS)
        {
            USER_LOG_ERROR("Photo %d/%d failed", i + 1, count);
            return returnCode;
        }

        if (i < count - 1)
        {
            osalHandler->TaskSleepMs(intervalMs);
        }
    }

    return DJI_ERROR_SYSTEM_MODULE_CODE_SUCCESS;
} // 多次拍照

T_DjiReturnCode User_CameraDeInit(void)
{
    s_cameraStatus.isCameraInitialized = false;
    return DjiCameraManager_DeInit();
}