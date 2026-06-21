/*
 * This file is part of the MiaoUI Library.
 *
 * Copyright (c) 2025, JFeng-Z, <2834294740@qq.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Created on: 2025-02-08
 */
#include "services.h"

/*
 *按键名与实际位置对应示意图：
 *                 B
 *         屏幕 A     C
 *  E    F         D
 */

UI_ACTION key_mapping_table[][6] = {
   //KeyIndex_A     KeyIndex_B      KeyIndex_C      KeyIndex_D      KeyIndex_E      KeyIndex_F
    {UI_ACTION_UP,UI_ACTION_PLUS,UI_ACTION_DOWN ,UI_ACTION_MINUS, UI_ACTION_BACK,UI_ACTION_ENTER},//横屏布局一
    {UI_ACTION_DOWN,UI_ACTION_MINUS,UI_ACTION_UP ,UI_ACTION_PLUS, UI_ACTION_BACK,UI_ACTION_ENTER},//横屏布局一（反转）
    {UI_ACTION_MINUS,UI_ACTION_UP, UI_ACTION_PLUS,  UI_ACTION_DOWN , UI_ACTION_BACK, UI_ACTION_ENTER},//竖屏布局一
    {UI_ACTION_PLUS,UI_ACTION_DOWN, UI_ACTION_MINUS,  UI_ACTION_UP , UI_ACTION_BACK, UI_ACTION_ENTER},//竖屏布局一（反转）
};
uint8_t key_layout = 0;//按键布局指示，默认横屏布局一

extern ui_t ui;

UI_ACTION indevScan(void)
{
    UI_ACTION Action;
    uint16_t btn;
    osStatus_t status;
    /* 放入你的按键扫描代码 */
    if(ui.nowItem->page.location->type == UI_PAGE_TEXT) key_layout = 2;
    else key_layout = 0;
    if(ui.rotation) key_layout++;
    
    status = osMessageQueueGet(Button_QueueHandle, &btn, 0, 0);
    if(status == osOK){
        /* 唤醒不稳定期：丢弃该按键，防止唤醒按键触发立即重新休眠 */
        if (pm_api_is_unstable_wake()) {
            return UI_ACTION_NONE;
        }
        Action = key_mapping_table[key_layout][btn >> 8];
        if(ClickBeep) Beep_Send(BeepID_Short);
    }
    else{
        Action = UI_ACTION_NONE;
    }
    btn = 0;
    return Action;
}