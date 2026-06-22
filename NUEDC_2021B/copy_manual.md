# 手册

> 复刻步骤、调试过程、测试报告及注意事项。

---

## 目录

- [复刻步骤](#复刻步骤)
- [调试过程](#调试过程)
- [测试结果](#测试结果)
- [注意事项](#注意事项)

---

## 复刻步骤

### 1. 获取源码

```bash
git clone <repo-url>
cd NUEDC_2021B
```

### 2. 打开工程

- 启动 Keil MDK v5
- 打开 `Firmware_0/MDK-ARM/NUEDC_2021B.uvprojx`
- 确保已安装 STM32G4 器件支持包（DFP）

### 3. 配置编译器

- 推荐使用 Arm Compiler 5（默认）
- 若使用 AC6，需手动替换 CubeMX HAL 中的 `port.c`（详见注意事项）
- 对 FreeRTOS 的 `task.c` 和 `port.c` 单独关闭 LTO（详见注意事项）

### 4. 编译与下载

- 按 F7 编译，确认无错误
- 连接 SWD 调试器（DAPLINK / ST-Link）到主控板
- 按 F8 下载固件

### 5. 验证基础运行

- 上电后 OLED 应显示主菜单
- UART4 输出调试信息（115200 baud）
- 按键可导航菜单

### 6. 硬件调试

详见 [调试过程](#调试过程) 章节。

---

## 调试过程

### 实时控制路径配置

由于工程进行了极致性能优化，CubeMX 自动生成的代码不包含以下关键配置，需**手动**在 `stm32g4xx_it.c` 中添加：

- 放置核心控制函数 `MainLoop()` 到 HRTIM 定时器 D 的重复中断服务函数中
- 使能对应定时器/ADC 的中断（如 HRTIM Timer D REP 中断、ADC 看门狗中断）
- 配置 HRTIM 的 ADC 触发（`LL_HRTIM_ConfigADCTrig`），见 `conv_protection.c`

### 调试步骤

1. **环境搭建**
   - 连接 SWD 调试器（自制隔离 DAPLINK）到主控板
   - 打开 Keil MDK 工程 `MDK-ARM/NUEDC_2021B.uvprojx`
   - 确认 UART4 串口输出正常（用于 printf 调试）

2. **开环验证**
   - 编译下载后，确认 HRTIM PWM 输出正常（示波器查看各半桥波形）
   - 验证 ADC 触发链：TA CMP4→ADC4、TB CMP3→ADC2、TC CMP4→ADC3、TD CMP3→ADC1
   - 检查 `LoadControlVar()` 转换后的物理量是否合理

3. **闭环调试**
   - 逐步使能 `MainLoop()` 中被注释的控制环路
   - 先使能 BUCK 电压环，确认 36 V 输出稳定
   - 再使能整流器电压外环，观察母线电压调节
   - 最后使能 PR 电流内环，观察三相电流跟踪波形

4. **保护验证**
   - 测试 HRTIM FLT1 硬件快速封锁（< 1 µs）
   - 验证 ADC 看门狗 AWD 中断响应
   - 确认 `AutoRecover_Task()` 自动恢复逻辑

5. **性能调优**
   - 调节 PI/PR 参数以获得最佳动态响应
   - 使用 DAC 输出调试波形到示波器
   - 记录效率、PF、纹波等指标

### 内存布局概览

根据 `.sct` 文件，物理内存分配如下：

| 区域 | 地址范围 | 大小 | 用途 |
|------|---------|:----:|------|
| **Flash (512 KB)** | `0x08000000` | 512 KB | 程序代码、只读数据 |
| **CCM SRAM (31 KB)** | `0x10000000` | ~31 KB | `ER_CCM_TEXT`：高频执行代码（MainLoop 等 RAMFUNC）|
| **CCM SRAM (尾)** | `0x10007E00` | 512 B | `ER_VECTORS`：中断向量表（重定向至此）|
| **SRAM1 (80 KB)** | `0x20000000` | 80 KB | `RW_SRAM1`：全局变量、堆、栈 |
| **SRAM2 (12 KB)** | `0x20014000` | 12 KB | `RW_SRAM2`：快速变量（`.ram_data`）、常量池（`.ram_ro`）|
| **SRAM2 (顶)** | `0x20018000` | 4 KB ↓ | `RW_MSP_STACK`：主堆栈（向下生长）|

---

## 测试结果

> 详细测试报告，包含实物接线图、仪表照片、示波器波形及效率曲线。

### 测试平台

<!-- TODO: 插入测试平台实物照片 -->

**平台搭建照片**：

| 图片 | 说明 |
|:---:|------|
| <!-- TODO: 插入整机测试平台照片 --> | 图 1 — 测试平台全貌：三相交流源 → AC-DC 变换电路 → 电子负载，功率分析仪监测 |
| <!-- TODO: 插入仪表接线照片 --> | 图 2 — 仪表接线：示波器探头连接、功率分析仪电压/电流接线、万用表测量点 |

**仪器清单**：

| 仪器 | 型号 | 用途 |
|------|------|------|
| 示波器 | <!-- TODO --> | 观测 PWM 波形、电压/电流纹波 |
| 电子负载 | <!-- TODO --> | 模拟直流输出负载 $I_o = 0.1\sim2.0\,\text{A}$ |
| 功率分析仪 | <!-- TODO --> | 测量输入功率 $P_{in}$、功率因数 $\text{PF}$ |
| Ainuo 15 kW 可编程三相交流源 | <!-- TODO --> | 提供三相对称交流输入，线电压 $U_i = 23\sim33\,\text{V}$ 连续可调 |
| 万用表 | <!-- TODO --> | 校准直流输出电压 $U_o$ |

<!-- TODO: 插入各仪器单独照片 -->

### 基本要求测试

| 测试项 | 条件 | 指标 | 实测值 | 结果 |
|:-----:|------|:----:|:-----:|:----:|
| 输出电压精度 | $U_i=28\,\text{V}$, $I_o=2\,\text{A}$ | $36.0 \pm 0.1\,\text{V}$ | <!-- TODO --> | <!-- TODO --> |
| 负载调整率 $S_I$ | $U_i=28\,\text{V}$, $I_o=0.1\to2.0\,\text{A}$ | $\leq 0.3\%$ | <!-- TODO --> | <!-- TODO --> |
| 电压调整率 $S_U$ | $I_o=2\,\text{A}$, $U_i=23\to33\,\text{V}$ | $\leq 0.3\%$ | <!-- TODO --> | <!-- TODO --> |
| 效率 $\eta$ | $U_i=28\,\text{V}$, $I_o=2\,\text{A}$, $U_o=36\,\text{V}$ | $\geq 85\%$ | <!-- TODO --> | <!-- TODO --> |

### 发挥部分测试

| 测试项 | 条件 | 指标 | 实测值 | 结果 |
|:-----:|------|:----:|:-----:|:----:|
| 功率因数 $\text{PF}$ | $U_i=28\,\text{V}$, $I_o=2\,\text{A}$, $U_o=36\,\text{V}$ | $\geq 0.99$ | <!-- TODO --> | <!-- TODO --> |
| 效率 $\eta$ | $U_i=28\,\text{V}$, $I_o=2\,\text{A}$, $U_o=36\,\text{V}$ | $\geq 95\%$ | <!-- TODO --> | <!-- TODO --> |
| PF 可调范围 | 数字设定 | $0.90 \sim 1.00$ | <!-- TODO --> | <!-- TODO --> |
| PF 设定误差 | 全范围 | $\leq 0.02$ | <!-- TODO --> | <!-- TODO --> |

### 关键波形

<!-- TODO: 插入示波器截图，每组波形附测试条件说明 -->

| 波形 | 说明 | 截图 |
|:---:|------|:----:|
| **三相桥臂 PWM 输出** | CH1/2/3/4：整流 A/B/C 及 BUCK 半桥栅极驱动波形，示波器显示死区时间 | <!-- TODO --> |
| **三相输入电压与电流** | PFC 模式下 $U_a$ 与 $I_a$ 同相位（PF ≈ 1.0），Math 显示瞬时有功功率 | <!-- TODO --> |
| **直流母线电压纹波** | AC 耦合观测 $U_{bus}$ 纹波，记录峰峰值 | <!-- TODO --> |
| **BUCK 输出电压纹波** | AC 耦合观测 $U_o = 36\,\text{V}$ 输出纹波，记录峰峰值 | <!-- TODO --> |
| **启动过程** | 母线预充 → 整流启动 → BUCK 缓升，观测软启动时间与超调量 | <!-- TODO --> |
| **负载突变响应** | $I_o$ 在 $0.1\,\text{A}$ 与 $2.0\,\text{A}$ 之间跳变，观测 $U_o$ 恢复时间与跌落幅度 | <!-- TODO --> |
| **DPWM-A 调制波形** | 不连续 PWM 钳位区间，观测 60° 钳位段内桥臂不开关 | <!-- TODO --> |
| **故障保护动作** | 过流后 HRTIM FLT 立即封锁 PWM，观测关断时间 | <!-- TODO --> |

### 效率曲线

<!-- TODO: 插入效率 vs 输出功率曲线图 -->

| 曲线说明 | 图 |
|:---------|:--:|
| 横轴：$P_o$ (W)，纵轴：$\eta$ (%)，不同 $U_i$ 下三条曲线 | <!-- TODO --> |
| DPWM-A 使能前后效率对比 | <!-- TODO --> |

### 功率因数曲线

<!-- TODO: 插入 PF vs 输出功率曲线图 -->

| 曲线说明 | 图 |
|:---------|:--:|
| 横轴：$P_o$ (W)，纵轴：$\text{PF}$，数字设定 $\text{PF}=0.90, 0.95, 1.00$ 三条实测曲线 | <!-- TODO --> |

---

## 快速开始

### 开发环境

1. **STM32CubeMX** — 用于重新生成 HAL 初始化代码（可选，`.ioc` 文件已配置）
2. **Keil MDK v5** — 主开发工具，打开 `MDK-ARM/NUEDC_2021B.uvprojx`
3. **串口调试工具** — 用于查看调试输出（UART4）

### 编译与下载

1. 打开 `MDK-ARM/NUEDC_2021B.uvprojx`
2. 编译（F7）→ 下载（F8）
3. 使用 SWD 调试器（自制隔离 DAPLINK 或 ST-Link）连接

### 配置选项

在 `Core/Inc/main.h` 中可调整关键参数：

```c
#define INITIAL_HRTIM_DEADTIME  200     // 死区时间 (ns 级)
#define INITIAL_HRTIM_PERIOD    32000   // HRTIM 周期值 (对应 ~30 kHz)
```

在 `APP/PowerControl/conv_controller.h` 中：
- `SWITCH_FREQ` — 开关频率（固定 30 kHz）
- `Ubus_Ref` — 母线电压参考值（自适应控制的目标值基础）
- `Idc_Ref_Max` — 电压外环输出限幅（对应硬件电流限幅）
- `PR_Out_Limit` — PR 电流环输出限幅（对应调制比上限）

---

## 注意事项

### 编码格式

所有源文件使用 **UTF-8 编码**，而 Keil MDK 编辑器默认编码为 ANSI/Locale。需在 `Edit → Configuration → Editor → Encoding` 中设置为 UTF-8，否则中文注释会乱码。

### AC6 编译器兼容性

若使用 **AC6 (Arm Compiler 6)** 而非默认的 AC5，ST 的 CubeMX HAL 库需要手动修改：
- 找到 CubeMX 库文件仓库（Repository）路径下的 `Device/STM32G4xx/Source/Templates/arm/` 文件夹
- 将 `RVDS` 目录中的 `port.c` 等文件替换为 `GCC` 目录下的同名文件
- 否则编译时报错：`error: undeclared function "__disable_irq"` 和 `__asm`

### LTO 优化与 FreeRTOS

- 工程开启了 **Link-Time Optimization (LTO)** 以减小代码体积和执行时间
- **但必须** 对 FreeRTOS 的 `task.c` 和 `port.c` 单独关闭 LTO 优化：
  - 右键文件 → `Options for File '...'` → `One ELF Section per Function` 取消勾选 + 优化等级设为 `Default`
  - 否则链接时报错：`Error: L6137E: Symbol vTaskSwitchContext was not preserved by the LTO codegen`

### DMA 中断

- 在 CubeMX 的 DMA 设置中，**取消勾选 "Force DMA channels Interrupts"**
- 关闭 ADC 所用的 DMA 通道中断：循环模式下 DMA 全自动运行，不需要 CPU 参与中断处理

### OLED 显示注意事项

- 本工程使用 **SH1106** 驱动芯片的 1.3" OLED（128×64，SPI 接口）
- **关键修改**：SH1106 的初始化序列 `u8x8_d_ssd1306_128x64_noname_init_seq[]` 需要手动修改：
  - **删除** 启动内部电荷泵的指令：`U8X8_CA(0x08d, 0x014)`
  - **添加** 关闭电荷泵的指令：`U8X8_CA(0xad, 0x8a)`
  - 原因：SH1106 不需要内部电荷泵（使用外部供电），错误的电荷泵指令会导致屏幕烧毁
- 若使用其他型号 OLED 或显示驱动，需相应调整 u8g2 初始化序列

### 中断向量表重定向

本工程将中断向量表从 Flash **拷贝到 CCM SRAM** 中运行（地址 `0x10007E00`），以提高中断响应速度。

**.sct 文件中的定义**（`MDK-ARM/stm32g474xx_flash.sct`）：

```ld
ER_VECTORS 0x10007E00 EMPTY 0x00000200 {
}
```

地址 `0x10007E00` 位于 CCM SRAM（31 KB）的末尾，紧接在 `ER_CCM_TEXT`（代码段，`0x10000000` 起始，大小 `0x7C00`）之后。

**运行时拷贝与重定向**（`Core/Src/main.c` 中 `RelocateVectorTable()`）：

```c
#define VECT_TAB_START  0x10007E00

void RelocateVectorTable(void) {
    __disable_irq();
    for (int i = 0; i < 128; i++) {
        ((uint32_t *)VECT_TAB_START)[i] = ((uint32_t *)0x08000000)[i];
    }
    __DSB();
    SCB->VTOR = VECT_TAB_START;
    __DSB(); __ISB();
    __enable_irq();
}
```

**注意**：如果在其他 MCU（如 STM32G474RET6，Flash 起始不同）上使用，需确认向量表拷贝源地址是否正确。

### ADC 采样校正

`APP/PowerControl/conv_adc.c` 中 `PowerControlVar_t` 结构体的 `slope`（斜率/增益）和 `zero`（零点/偏置）参数是针对当前硬件实测标定的。更换硬件平台后必须重新标定：

```c
IV_INV_t = {ADC1_Buffer, 0, 7.7004f, -12.179f, 0};
```

标定方法：接入已知电压/电流，测量 ADC 原始码值，线性拟合得到 slope 和 zero。

### HRTIM 占空比缺陷

HRTIM 在计数值接近 **0** 或接近 **PER**（周期值）时，PWM 占空比输出会出现错误。应避免在 `CMP = 0` 或 `CMP = PER` 附近更新比较值。

### MCU 降压运行

若需极致降低功耗，可将 STM32G474 VDD 降至 1.8 V，但需添加电平转换电路，且调试器需支持可变电平。

### 第二块母板硬件

第二版本母板打样与调试中，需解决 PCB 布局和信号完整性问题。

### 隔离下载器稳定性

自制隔离 DAPLINK 在高压调试场景下的通信稳定性需验证与改进。
