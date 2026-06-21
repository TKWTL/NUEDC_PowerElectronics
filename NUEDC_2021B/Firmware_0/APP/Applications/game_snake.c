#include "game_snake.h"
#include <stdlib.h>//含有随机数函数库
#include <string.h>

/*
 *使用二维数组实现的贪吃蛇游戏
 *场地13格高，31格宽，分为有边和无边两种
 *蛇的速度和蛇的速度递增可选以适应不同难度
 *由 Grok3.0 辅助创作  TKWTL 2025/05/04
 */


typedef enum {
    PLAYING,    // 正常游戏中
    PAUSED,     // 暂停菜单
    FAILED,     // 游戏失败
    WON         // 游戏胜利
} GameState_t;  // 游戏状态枚举

// 游戏参数
#define ROWS 13
#define COLS 31
#define TILE_SIZE 4
#define FIELD_TOP 10


typedef enum {
    DIR_NONE,
    DIR_UP,
    DIR_RIGHT,
    DIR_DOWN,
    DIR_LEFT
} Direction_t;  // 方向定义

// 游戏状态变量
static uint8_t field[ROWS][COLS];//地图
static uint8_t head_row, head_col;//头节点坐标
static uint8_t tail_row, tail_col;//尾节点坐标
static uint8_t direction;//当前蛇头方向，为0时静止

static uint16_t score;
static uint16_t high_score = 0;

static bool has_borders = true;//是否有边界

static uint16_t base_speed_ms = 400;
static uint16_t speed_increment_ms_per_fruit = 10;
static uint16_t min_speed_ms = 160;

static uint8_t menu_selection = 0;
static GameState_t current_state = PLAYING; // 当前游戏状态
static TickType_t last_move_tick;

//四个方向在行与列方向上的投影值
static const int8_t dr[5] = {0, -1, 0, 1, 0}; // 0, up, right, down, left
static const int8_t dc[5] = {0, 0, 1, 0, -1};

// 生成水果
static void generate_fruit(void) {
    // 尝试100次生成水果，确保中心及邻居格子为空
    int tries = 100;
    while (tries--) {
        uint8_t r = (rand() % 11) + 1; // 1 to 11
        uint8_t c = (rand() % 29) + 1; // 1 to 29
        if (field[r][c] == 0) {
            field[r][c] = 255; // 仅中心格子设为255
            return;
        }
    }
}

// 初始化游戏
static void init_game(void) {
    uint8_t dir;
    // 重置得分和场地
    score = 0;
    memset(field, DIR_NONE, sizeof(field));
    
    // 随机选择方向
    dir = (rand() % 4) + 1; // 1到4
    
    // 设置蛇头位置
    head_row = 2 + rand()% (ROWS- 4);
    head_col = 2 + rand()% (COLS- 4);
    field[head_row][head_col] = dir;

    // 设置蛇身与蛇尾位置
    switch (dir) {
        case DIR_UP:
            field[head_row + 1][head_col] = dir;
            field[head_row + 2][head_col] = dir;
            tail_row = head_row + 2;
            tail_col = head_col;
            break;
        case DIR_DOWN:
            field[head_row - 1][head_col] = dir;
            field[head_row - 2][head_col] = dir;
            tail_row = head_row - 2;
            tail_col = head_col;
            break;
        case DIR_LEFT:
            field[head_row][head_col + 1] = dir;
            field[head_row][head_col + 2] = dir;
            tail_row = head_row;
            tail_col = head_col + 2;
            break;
        case DIR_RIGHT:
            field[head_row][head_col - 1] = dir;
            field[head_row][head_col - 2] = dir;
            tail_row = head_row;
            tail_col = head_col - 2;
            break;
    }
    
    // 设置初始方向为0，等待用户输入
    direction = DIR_NONE;
    
    // 生成水果
    generate_fruit();
    // 初始化状态
    current_state = PLAYING;
    //menu_selection = 0;
}

// 移动蛇
static void move_snake(void) {
    int8_t new_row, new_col;
    // 若方向为0，蛇保持静止
    if (direction == 0) return;
    
    field[head_row][head_col] = direction;// 设置喉部
    
    // 计算新头部位置
    new_row = head_row + dr[direction];
    new_col = head_col + dc[direction];
    
    // 无边模式环绕或撞墙检测
    if (has_borders) {
        if (new_row < 0 || new_row >= ROWS || new_col < 0 || new_col >= COLS) {//撞墙
            current_state = FAILED;
            return;
        }
    }
    else {
        if (new_row < 0) new_row = ROWS - 1;
        else if (new_row >= ROWS) new_row = 0;
        
        if (new_col < 0) new_col = COLS - 1;
        else if (new_col >= COLS) new_col = 0;
    }
    
    if (field[new_row][new_col] >= 1 && field[new_row][new_col] <= 4) {//撞自己
        current_state = FAILED;
        return;
    }
    
    // 设置新头部
    head_row = new_row;
    head_col = new_col;
    if (field[head_row][head_col] == 255) {//吃到了果子
        field[head_row][head_col] = direction;// 设置新头部
        score++;
        if (score >= 400) {
            current_state = WON;
            return;
        }
        if (score > high_score) high_score = score;
        
        generate_fruit();// 生成新水果，蛇头已覆盖旧水果位置
    }
    else {//没吃到
        field[head_row][head_col] = direction;// 设置新头部
        
        // 更新尾部位置
        new_row = tail_row;
        new_col = tail_col;
        switch (field[tail_row][tail_col]) {
            case DIR_UP:
                new_row = tail_row- 1;
                if (!has_borders && new_row < 0) new_row = ROWS - 1;
                break;
            case DIR_RIGHT:
                new_col = tail_col+ 1;
                if (!has_borders && new_col >= COLS) new_col = 0;
                break;
            case DIR_DOWN:
                new_row = tail_row+ 1;
                if (!has_borders && new_row >= ROWS) new_row = 0;
                break;
            case DIR_LEFT:
                new_col = tail_col- 1;
                if (!has_borders && new_col < 0) new_col = COLS - 1;
                break;
            default:
                current_state = FAILED;// 调试：方向无效，可能需检查初始化或状态
        }
        field[tail_row][tail_col] = 0;// 移除旧尾部
        tail_row = new_row;
        tail_col = new_col;
        
        //验证新尾部位置是否为蛇身
        if (field[tail_row][tail_col] < DIR_UP || field[tail_row][tail_col] > DIR_LEFT) 
            current_state = PAUSED;// 调试：新尾部位置无效，可能需检查蛇身路径
    }
}

static const char *menu_options[] = {"Exit", "Border:", "Speed:", "SpeedInc:"};// 菜单选项（不含首项）
static const char *speed_options[] = {"Slow", "Medium", "Fast"};
static const char *inc_options[] = {"None", "Low", "High"};

//提供给UI调用的贪吃蛇实现函数
void Game_Snake(ui_t *ui){
    char buf[16];                                                               //显示用缓冲区数组
    uint8_t color;                                                              //设置颜色
    uint8_t i, j;                                                               //遍历用变量
    uint8_t x, y;                                                               //定位用变量
    uint16_t current_speed;
    
    Disp_SetFont(font_menu_main_h12w6);//设置字体
    srand(sTime.Seconds + sTime.Date + sTime.Month + sTime.Year);//初始化种子
    init_game();//初始化地图
    
    while(1){
        //两种颜色下的清屏
        Disp_SetDrawColor(&ui->bgColor);
        Disp_DrawBox(0, 0, UI_HOR_RES, UI_VER_RES);
        color = ui->bgColor^ 0x01;
        Disp_SetDrawColor(&color);
        
        // 显示得分
        sprintf(buf, "High: %d", high_score);
        Disp_DrawStr(1, 8, buf);
        sprintf(buf, "Score: %d", score);
        Disp_DrawStr(UI_HOR_RES/ 2+ 2, 8, buf);
        
        if (current_state != PLAYING) {
            // 显示状态
            switch (current_state) {
                case PAUSED:
                    Disp_DrawStr(1, 8, " Paused  ");
                    break;
                case FAILED:
                    Disp_DrawStr(1, 8, "Game Over!");
                    break;
                case WON:
                    Disp_DrawStr(1, 8, "Victory!!!");
                    break;
                default:
                    break;
            }
            
            // 处理菜单输入
            switch(indevScan()) {
                case UI_ACTION_MINUS:
                    if(menu_selection < 4) menu_selection++;
                    else menu_selection = 0;
                    break;
                case UI_ACTION_PLUS:
                    if(menu_selection > 0) menu_selection--;
                    else menu_selection = 4;
                    break;
                case UI_ACTION_BACK:
                    if(current_state == PAUSED) current_state = PLAYING;
                    else{
                        ui->action = UI_ACTION_ENTER;
                        return;
                    }
                    break;
                case UI_ACTION_ENTER:
                    switch(menu_selection) {
                        case 0: // Resume/Restart
                            if(current_state == PAUSED) current_state = PLAYING;
                            else  init_game();
                            break;
                        case 1: // Exit
                            ui->action = UI_ACTION_ENTER;
                            return;
                        case 2: // Border
                            has_borders = !has_borders;
                            break;
                        case 3: // Speed
                            base_speed_ms = (base_speed_ms == 600) ? 400 : (base_speed_ms == 400) ? 200 : 600;
                            break;
                        case 4: // Speed Increment
                            speed_increment_ms_per_fruit = (speed_increment_ms_per_fruit == 0) ? 10 : (speed_increment_ms_per_fruit == 10) ? 20 : 0;
                            break;
                    }
                    break;
                case UI_ACTION_NONE:
                    break;
                default:
                    break;
            }
            
            // 绘制菜单
            for (i = 0; i < 5; i++) {
                y = 20 + i * 10;
                if (i == 0) {
                    strcpy(buf, (current_state == PAUSED) ? "Resume" : "Restart");
                } else if (i == 1) {
                    strcpy(buf, menu_options[0]);
                } else if (i == 2) {
                    sprintf(buf, "%s %s", menu_options[1], has_borders ? "On" : "Off");
                } else if (i == 3) {
                    uint8_t speed_idx = (base_speed_ms == 600) ? 0 : (base_speed_ms == 400) ? 1 : 2;
                    sprintf(buf, "%s %s", menu_options[2], speed_options[speed_idx]);
                } else {
                    uint8_t inc_idx = (speed_increment_ms_per_fruit == 0) ? 0 : (speed_increment_ms_per_fruit == 10) ? 1 : 2;
                    sprintf(buf, "%s %s", menu_options[3], inc_options[inc_idx]);
                }
                Disp_DrawStr(2, y, buf);
                if (i == menu_selection){
                    if(i == 0) 
                        ui->cursor.nowWide  = (int)UI_Animation(strlen((current_state == PAUSED) ? "Resume" : "Restart")* 6 + 3, (float)ui->cursor.nowWide, &ui->animation.optionbarPos_ani);
                    else
                        ui->cursor.nowWide  = (int)UI_Animation(strlen(menu_options[i- 1])* 6 + 3, (float)ui->cursor.nowWide, &ui->animation.optionbarPos_ani);
                }
            }
            // 绘制光标
            ui->cursor.targrtRow = (int)UI_Animation(10+ menu_selection* 10, (float)ui->cursor.targrtRow, &ui->animation.cursor_ani);
            color = 0x02;
            Disp_SetDrawColor(&color);
            Disp_DrawRBox(0, ui->cursor.targrtRow + 1, ui->cursor.nowWide, 10, 0);
            color = ui->bgColor^ 0x01;
            Disp_SetDrawColor(&color);
        }
        else {//游戏界面
            // 处理游戏输入
            switch (indevScan()) {
                case UI_ACTION_PLUS:
                    if (field[head_row][head_col] != DIR_DOWN) direction = DIR_UP;//避免回头撞死自己
                    break;
                case UI_ACTION_MINUS:
                    if (field[head_row][head_col] != DIR_UP) direction = DIR_DOWN;
                    break;
                case UI_ACTION_UP:
                    if (field[head_row][head_col] != DIR_RIGHT) direction = DIR_LEFT;
                    break;
                case UI_ACTION_DOWN:
                    if (field[head_row][head_col] != DIR_LEFT) direction = DIR_RIGHT;
                    break;
                case UI_ACTION_BACK:
                    ui->action = UI_ACTION_ENTER;
                    return;
                case UI_ACTION_ENTER:
                    current_state = PAUSED;
                    break;
                case UI_ACTION_NONE:
                    break;
            }
            
            // 移动蛇（速度逻辑处理）
            TickType_t current_tick = xTaskGetTickCount();
            
            if (score * speed_increment_ms_per_fruit > base_speed_ms - min_speed_ms) current_speed = min_speed_ms;
            else current_speed = base_speed_ms - score * speed_increment_ms_per_fruit;
            
            if (direction != DIR_NONE && (current_tick - last_move_tick >= pdMS_TO_TICKS(current_speed))) {
                move_snake();
                last_move_tick = current_tick;
            }
            
            // 绘制边框
            if (has_borders) Disp_DrawFrame(0, FIELD_TOP- 1, COLS * TILE_SIZE+ 3, ROWS * TILE_SIZE+ 3);
            else{
                Disp_DrawPixel(0, FIELD_TOP- 1);
                Disp_DrawPixel(0, FIELD_TOP- 1+ ROWS * TILE_SIZE+ 2);
                Disp_DrawPixel(COLS * TILE_SIZE+ 3, FIELD_TOP- 1);
                Disp_DrawPixel(COLS * TILE_SIZE+ 3, FIELD_TOP- 1+ ROWS * TILE_SIZE+ 2);
            }
            
            for (i = 0; i < ROWS; i++) {
                for (j = 0; j < COLS; j++) {
                    //遍历每块地图
                    x = j * TILE_SIZE + 2;
                    y = FIELD_TOP+ 2+ i * TILE_SIZE;
                    if (field[i][j] >= 1 && field[i][j] <= 4) {//画蛇身
                        Disp_DrawBox(x, y, 3, 3);
                        switch (field[i][j]) {//添加方向标识
                            case DIR_RIGHT: Disp_DrawPixel(x + 3, y + 1); break;
                            case DIR_LEFT:  Disp_DrawPixel(x - 1, y + 1); break;
                            case DIR_UP:    Disp_DrawPixel(x + 1, y - 1); break;
                            case DIR_DOWN:  Disp_DrawPixel(x + 1, y + 3); break;
                        }
                    }
                    if (field[i][j] == 255) {//画水果
                        Disp_DrawPixel(x + 1, y);
                        Disp_DrawPixel(x, y + 1);
                        Disp_DrawPixel(x + 2, y + 1);
                        Disp_DrawPixel(x + 1, y + 2);
                    }
                }
            }
        }
        
        Disp_SendBuffer();
        osDelay(pdMS_TO_TICKS(10));
    }
}
