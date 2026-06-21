#ifndef __GAME_DINOSAUR_H__
#define __GAME_DINOSAUR_H__

#ifdef __cplusplus
extern "C" {
#endif
    
#include "services.h"

#define DINO_DEFAULT_POS_X  6
#define DINO_DEFAULT_HEIGHT 46
#define DINO_JUMP_SPEED_INITIAL -6
#define DINO_JUMP_GRAVITY   0.5
//跳跃高度H = (DINO_JUMP_SPEED_INITIAL)^2/(2* DINO_JUMP_GRAVITY)，控制在36附近

    
// 云对象的数据结构
typedef struct {
    uint8_t available;
    uint32_t start_pos;//available == 1时，标记云朵的初始位置对应的坐标值；== 0时，用于云朵生成的倒计时
    uint8_t y_pos;
    uint8_t speed_div;
}Dino_Cloud_t;
// 云的宏定义
#define CLOUD_RESPAWN_MAX   512
#define CLOUD_RESPAWN_MIN   32
#define CLOUD_YPOS_MAX  40
#define CLOUD_YPOS_MIN  4
#define CLOUD_SPEED_DIVID_MAX   4   //云朵速度相当于背景速度的几分之一（最慢）
#define CLOUD_SPEED_DIVID_MIN   2   //云朵速度相当于背景速度的几分之一（最快）
#define CLOUD_POOL_DEPTH    4


typedef struct {
    const uint8_t *pdata;
    uint8_t width;
    uint8_t height;
}Dino_Cactus_Image_t;//仙人掌图像结构体
typedef struct {
    uint8_t available;
    uint32_t start_pos;
    uint8_t type;
}Dino_Cactus_t;//仙人掌对象结构体
// 仙人掌的宏定义
#define CACTUS_RESPAWN_MAX  256
#define CACTUS_RESPAWN_MIN  64
#define CACTUS_SPACE        96
#define CACTUS_POOL_DEPTH   3

//提供给UI调用的谷歌小恐龙实现函数
void Game_DinoSaur(ui_t *ui);
    
#ifdef __cplusplus
}
#endif

#endif
 