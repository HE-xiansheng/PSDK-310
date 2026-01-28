/* user_camera_data.h */
#ifndef USER_CAMERA_DATA_H
#define USER_CAMERA_DATA_H

#include "dji_typedef.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // 相机控制命令类型
    typedef enum
    {
        USER_CAMERA_CMD_NONE = 0,     // 无命令
        USER_CAMERA_CMD_SHOOT_SINGLE, // 单次拍照
        USER_CAMERA_CMD_SHOOT_MULTI,  // 多次拍照
        USER_CAMERA_CMD_SET_ISO,      // 设置ISO
    } E_UserCameraCmdType;

    // 相机控制命令结构体
    typedef struct
    {
        E_UserCameraCmdType cmdType;
        union
        {
            struct
            {
                uint32_t count;
                uint32_t intervalMs;
            } shootMulti;
            struct
            {
                int isoValue;
            } setISO;
        } cmdParam;
        bool hasPendingCmd; // 是否有待处理的命令
    } T_UserCameraCmd;

    // 获取相机命令结构体的指针
    T_UserCameraCmd *User_Camera_GetCmd(void);

    // 清除命令标志
    void User_Camera_ClearCmd(void);

#ifdef __cplusplus
}
#endif

#endif // USER_CAMERA_DATA_H
