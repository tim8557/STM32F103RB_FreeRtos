#ifndef BSP_CAMERA_
#define BSP_CAMERA_

typedef enum 
{
    camera_fifo_not_ready = 0,
    camera_fifo_prepare,
    camera_fifo_ready,
}Camera_Status_Type;

void Camera_Init(void);
void Camera_Runnable_20ms(void);
void Camera_Read_Device_ID(void);
void Camera_SetStatus(Camera_Status_Type status);
Camera_Status_Type Camera_GetStatus(void);

#endif