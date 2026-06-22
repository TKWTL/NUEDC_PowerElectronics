# 三相 AC-DC 变换电路 — 嵌入式固件

> **2021 年全国大学生电子设计竞赛（TI 杯）本科组 B 题**
>
> 基于 STM32G474VCT6 的三相 AC-DC 变换电路数字控制系统

---

## 目录

- [项目概述](#项目概述)
- [题目简介](#题目简介)
- [做题流程](#做题流程)
- [系统架构](#系统架构)
- [固件架构](#固件架构)
- [硬件平台](#硬件平台)
- [硬件设计](#硬件设计)
- [项目结构](#项目结构)
- [快速开始](#快速开始)
- [关键模块说明](#关键模块说明)
- [测试结果](#测试结果)
- [当前状态](#当前状态)
- [许可证](#许可证)

---

## 项目概述

本项目实现了 **2021 年全国大学生电子设计竞赛 B 题——三相 AC-DC 变换电路** 的完整嵌入式控制系统。

系统以 **STM32G474VCT6** 为主控芯片，采用 **三相两电平 PWM 电压型整流器（VSR）+ 一路 BUCK 降压** 拓扑，共 4 个半桥并联，配合高分辨率定时器（HRTIM）、多通道 ADC 同步采样等片上资源，实现：

- **直流输出稳压**：$U_o = 36\,\text{V} \pm 0.1\,\text{V}$
- **负载调整率**：$S_I \leq 0.3\%$（$I_o = 0.1\,\text{A} \sim 2.0\,\text{A}$）
- **电压调整率**：$S_U \leq 0.3\%$（$U_i = 23\,\text{V} \sim 33\,\text{V}$）
- **高效率**：$\eta \geq 85\%$（基本）/ $\eta \geq 95\%$（发挥）
- **高功率因数**：$\text{PF} \geq 0.99$，且可数字设定（$0.90 \sim 1.00$）

详细题目要求见 [`Docs/B_三相AC-DC变换电路.pdf`](../Docs/B_三相AC-DC变换电路.pdf)。

---

## 题目简介

2021 年 TI 杯全国大学生电子设计竞赛 **B 题——三相 AC-DC 变换电路**，要求设计并制作一个将三相交流电转换为稳定直流输出的变换电路。

> **题目原文**：[`Docs/B_三相AC-DC变换电路.pdf`](../Docs/B_三相AC-DC变换电路.pdf)

<!-- ====== 题目图片占位符 ====== -->

| 图片 | 说明 |
|:---:|------|
| <!-- TODO: 插入题目系统框图 --> | **图 1‑1 题目系统框图** — 三相交流输入 → 三相变压器 → 三相 AC-DC 变换电路 → 直流输出 $(U_o=36\,\text{V}, I_o=2\,\text{A})$ |
| <!-- TODO: 插入题目基本要求表格截图 --> | **图 1‑2 基本要求** — 输出电压精度、负载调整率、电压调整率、效率指标 |
| <!-- TODO: 插入题目发挥部分表格截图 --> | **图 1‑3 发挥部分** — 功率因数、效率、可调 PF、创新加分 |

### 基本要求

| 项目 | 条件 | 指标 |
|:---:|------|------|
| 输出电压 | $U_i = 28\,\text{V}$, $I_o = 2\,\text{A}$ | $U_o = 36\,\text{V} \pm 0.1\,\text{V}$ |
| 负载调整率 | $U_i = 28\,\text{V}$, $I_o$ 在 $0.1 \sim 2.0\,\text{A}$ 变化 | $S_I \leq 0.3\%$ |
| 电压调整率 | $I_o = 2\,\text{A}$, $U_i$ 在 $23 \sim 33\,\text{V}$ 变化 | $S_U \leq 0.3\%$ |
| 效率 | $U_i = 28\,\text{V}$, $I_o = 2\,\text{A}$, $U_o = 36\,\text{V}$ | $\eta \geq 85\%$ |

### 发挥部分

| 项目 | 条件 | 指标 |
|:---:|------|------|
| 功率因数 | $U_i = 28\,\text{V}$, $I_o = 2\,\text{A}$, $U_o = 36\,\text{V}$ | $\text{PF} \geq 0.99$ |
| 效率 | $U_i = 28\,\text{V}$, $I_o = 2\,\text{A}$, $U_o = 36\,\text{V}$ | $\eta \geq 95\%$ |
| 功率因数可调 | 数字设定 | $0.90 \sim 1.00$，误差 $\leq 0.02$ |
| 创新 | 自主设计加分项 | — |

---

## 做题流程

本文档记录了解题的核心思路与工程流程，供后续调试与复现参考。

```mermaid
flowchart LR
    A[① 题目要求分析] --> B[② 拓扑选型]
    B --> C[③ 根据拓扑确定<br>控制架构与控制量]
    C --> D[④ 采样点与 ADC 分配<br>↓ 见 Firmware_0/README.md]
    D --> E[⑤ 控制策略设计<br>↓ 见 Firmware_0/README.md]
    E --> F[⑥ 软件架构设计<br>↓ 见 Firmware_0/README.md]
    F --> G[⑦ 硬件电路设计]
    G --> H[⑧ 调试与验证]
    H --> I{指标达标?}
    I -- 否 --> E
    I -- 是 --> J[⑨ 文档归档]
```

### ① 题目要求分析

- 稳定输出 **36 V**，精度 $\pm 0.1\,\text{V}$（0.28%）
- 输入线电压 **23~33 V**，输出电流 **0.1~2.0 A** 宽范围
- 效率 **≥ 85%**（基本）/ **≥ 95%**（发挥）
- 功率因数 **≥ 0.99**，且可数字设定 $0.90 \sim 1.00$
- 须计入辅助电源功耗
- 过流/过压/短路保护

### ② 拓扑选型

| 拓扑方案 | 优点 | 缺点 | 选择 |
|---------|------|------|:---:|
| 三相维也纳整流 (Vienna) | 3 管、无桥臂直通 | ① 整流二极管导通压降在 $<50\,\text{V}$ 低压输出时损耗占比极大，严重拉低效率 ② 母线电压需高于输入线电压峰值（$>\sqrt{2}\cdot 33\,\text{V} \approx 46.7\,\text{V}$），过高母线电压增加 BUCK 级开关损耗 | |
| 三相两电平 PWM 整流器 (VSR) | 结构简洁、控制成熟、每相独立、低电压效率高 | 有直通风险和 EMI 问题较大 | ⭐ |

**本方案**：采用 **三相两电平 PWM 电压型整流器（VSR）+ 一路 BUCK 降压** 拓扑。三相整流器负责 AC-DC 变换与 PFC，BUCK 级将不稳定的母线电压稳压至 36 V 输出。

**为什么不是两级（整流+逆变）而是整流+BUCK？**

- B 题只需单向 AC→DC 变换，无需逆变回馈
- BUCK 降压级比逆变级更适合宽范围输出稳压

### ③ 根据拓扑确定控制架构与控制量

选定 VSR + BUCK 拓扑后，推导控制架构：

| 步骤 | 内容 |
|------|------|
| **确定被控对象** | 前级 VSR → 直流母线电压 $U_{bus}$；后级 BUCK → 输出电压 $U_o = 36\,\text{V}$ |
| **确定控制量** | 三相输入电流 $I_a, I_b, I_c$（内环）、母线电压 $U_{bus}$（外环）、BUCK 电感电流 $I_L$（内环） |
| **确定执行器** | 4 个半桥：HRTIM A/B/C → VSR 三相开关，HRTIM D → BUCK 开关 |
| **确定反馈量** | 线电压 $U_{ab}, U_{bc}$（构造参考电流波形）、相电流 $I_a, I_b, I_c$、母线电压 $U_{bus}$、BUCK 输出 $U_o$, $I_L$ |
| **确定参考量来源** | $U_{bus\_ref}$ 由软件设定（自适应或固定），$I_{ref}$ 由电压外环输出与输入电压归一化波形相乘合成 |
| **确定调制方式** | SVPWM（标准 7 段式），零序分量 min-max 注入；待实现 DPWM-A |

### ④ 采样点确定与 ADC 分配

> ADC 通道映射、HRTIM 触发链、有效值计算与 PF 移相 → [`Firmware_0/README.md`](Firmware_0/README.md#④-采样点确定与-adc-分配)

### ⑤ 控制策略设计

> 电压外环 PI、参考电流合成、双 PR 电流内环、BUCK 控制、保护系统 → [`Firmware_0/README.md`](Firmware_0/README.md#⑤-控制策略设计)

### ⑥ 软件架构设计

> 三优先级架构、HRTIM 中断链、FreeRTOS 任务、保护分层 → [`Firmware_0/README.md`](Firmware_0/README.md#⑥-软件架构设计)

### ⑦ 硬件电路设计

见 [硬件设计](#硬件设计) 章节。

### ⑧ 调试与验证

见 [测试结果](#测试结果) 章节。

---

## 系统架构

```
三相交流输入 (Ainuo 15 kW 可编程三相交流源, Ui = 23~33 V)
        │
        ▼
┌───────────────────────────────────────────────────────┐
│                三相 AC-DC 变换电路                      │
│                                                        │
│  ┌──────────────────────────┐  ┌──────────────────┐   │
│  │  三相两电平 PWM 整流器     │  │  BUCK 降压级      │   │
│  │  (3 个半桥并联, HRTIM A-C) │→│  (1 个半桥,       │   │
│  │   三相交流 → 直流母线      │  │   HRTIM D)        │   │
│  │   + PFC + 升压            │  │   母线电压 → 36V  │   │
│  └──────────────────────────┘  └──────────────────┘   │
│                                                        │
│  ┌─ 采样: Ia, Ib, Ic ─────────────────────────────┐   │
│  │  Uab, Ubc, Uz, Ubus, Ibuck, Ubuck              │   │
│  └────────────────────────────────────────────────┘   │
│                                                        │
│  ┌─ HRTIM1 资源分配 ───────────────────────────────┐  │
│  │  Timer A: 整流 A 相 (TA2=AH, TA1=AL)            │   │
│  │  Timer B: 整流 B 相 (TB2=BH, TB1=BL)            │   │
│  │  Timer C: 整流 C 相 (TC2=CH, TC1=CL)            │   │
│  │  Timer D: BUCK 半桥 (TD2=NH, TD1=NL)            │   │
│  │  PWM EN → GPIO 3V4                              │   │
│  │  OC 保护 → HRTIM FLT1                           │   │
│  │  ADC 触发: TA CMP4→ADC4 / TB CMP3→ADC2         │   │
│  │            TC CMP4→ADC3 / TD CMP3→ADC1→EOS→Main │   │
│  └────────────────────────────────────────────────┘   │
└──────────────────────────┬────────────────────────────┘
                           │
                           ▼
                  直流输出 Uo = 36 V
                    Io = 0.1~2.0 A
```

系统采用 **三相两电平 PWM 电压型整流器（VSR）+ 一路 BUCK 降压** 的级联结构：

- **前级 — 三相两电平 PWM 整流器**：3 个半桥（HRTIM A/B/C），将三相交流电整流为直流母线电压，同时实现 PFC（功率因数校正）。控制策略为电压外环 PI 输出直流电流幅值 `Idc_ref`，与采样输入电压经有效值归一化后的单位波形相乘，构造正弦参考电流，再送入双 PR 电流内环（仅控制 A/B 两相，C 相由 $-A-B$ 导出）。无需 $dq$ 旋转坐标系和锁相环（PLL）。
- **后级 — BUCK 降压变换器**：1 个半桥（HRTIM D），将不稳定的直流母线电压降压至 36 V 稳定输出。控制策略为 PI 电压环 + P 电流环。
- **共 4 个半桥**，全部由 **HRTIM1** 驱动。PWM 引脚分配：TA2=AH / TA1=AL（整流 A），TB2=BH / TB1=BL（整流 B），TC2=CH / TC1=CL（整流 C），TD2=NH / TD1=NL（BUCK）。PW EN 由 GPIO 3V4 控制，过流保护接入 HRTIM FLT1。
- **ADC 触发**：TA CMP4→ADC4（IA/VAB），TB CMP3→ADC2（IC/VZS/VHFREF），TC CMP4→ADC3（IB/VBC/VBUS），TD CMP3→ADC1（IOUT/VOUT），ADC4 EOS 中断调用 `MainLoop()`。
- **功率因数调节**：通过数组延迟输入电压波形实现移相，延迟后的单位化电压与 `Idc_ref` 相乘即得到所需 PF 的正弦参考电流。

## 固件架构

> 软件分层、FreeRTOS 任务、实时控制路径、任务间通信详见 [`Firmware_0/README.md#固件架构`](Firmware_0/README.md#固件架构)。

---

## 硬件平台

| 项目 | 规格 |
|------|------|
| **主控 MCU** | STM32G474VCT6 (Cortex-M4F @ 120 MHz，可超频至 240 MHz) |
| **PWM 定时器** | HRTIM1 (6 个定时器单元, 高分辨率 ~217 ps)，使用 A/B/C/D 四单元；TA2=AH/TA1=AL, TB2=BH/TB1=BL, TC2=CH/TC1=CL, TD2=NH/TD1=NL；PWM EN → GPIO 3V4, OC → FLT1 |
| **ADC** | 4 个独立 ADC (12-bit, 硬件触发同步采样, DMA 循环传输)；触发链: TA CMP4→ADC4, TB CMP3→ADC2, TC CMP4→ADC3, TD CMP3→ADC1→EOS→MainLoop |
| **硬件加速** | CCM SRAM (关键代码如 MainLoop 放入 CCM 以提高实时性) |
| **显示** | 1.3" OLED 128×64 (SH1106, SPI 4线) |
| **比较器** | 4路LMV393窗口比较器 (过流硬件保护) |
| **通信** | FDCAN (1.25 Mbps)、USART/UART (带 DMA) |
| **调试** | 自制隔离 DAPLINK（SWD + UART4）|
| **开发工具** | Keil MDK (µVision), STM32CubeMX, LCEDA-Pro |

## 硬件设计

> 系统硬件由 **3 块 PCB + 1 个调试器** 组成，详见 [`Hardware/README.md`](Hardware/README.md)。

---

## 项目结构

```
NUEDC_2021B/
├── Docs/                                ← 题目文档
│   └── B_三相AC-DC变换电路.pdf
├── Firmware_0/                          ← 固件主目录
│   ├── APP/
│   │   ├── adc_handle.c                 ← ADC 采集处理
│   │   ├── beep.c / beep.h              ← 蜂鸣器任务
│   │   ├── button_handle.c              ← 按键扫描任务
│   │   ├── ui_task.c                    ← UI 任务入口
│   │   ├── Applications/                ← 应用层
│   │   │   ├── application.h            ← 应用统一头文件（Hub）
│   │   │   ├── burn_in_tester.c/h       ← 烧屏测试
│   │   │   ├── game_dinosaur.c/h        ← 谷歌小恐龙游戏
│   │   │   ├── game_snake.c/h           ← 贪吃蛇游戏
│   │   │   └── i2c_scanner.c/h          ← I2C 设备扫描
│   │   ├── framework/                   ← 框架服务层
│   │   │   ├── app_event.h              ← 应用事件定义
│   │   │   ├── pm_api.h                 ← 电源管理 API
│   │   │   ├── pm_controller.c/h        ← 电源管理控制器
│   │   │   ├── pm_device.c/h            ← 外设电源管理钩子
│   │   │   ├── pm_device_builtin.c      ← 内置设备电源管理
│   │   │   ├── pm_policy.c/h            ← 电源策略决策
│   │   │   ├── pm_sleep_timer.c/h       ← 休眠定时器
│   │   │   ├── pm_ui_register.c/h       ← UI 电源注册
│   │   │   ├── powerdown_task.c         ← 电源管理主任务
│   │   │   ├── services.h               ← 服务声明
│   │   │   ├── system_operation.c/h     ← 系统操作（复位、蜂鸣）
│   │   │   ├── system_statistic.c/h     ← 系统统计（温度、电压）
│   │   │   └── time.c/h                 ← RTC 时间服务
│   │   ├── MiaoUI/                      ← UI 框架 (v1.2)
│   │   │   ├── core/                    ← UI 核心引擎
│   │   │   ├── display/                 ← 显示驱动 (SPI OLED)
│   │   │   ├── fonts/                   ← 字体
│   │   │   ├── images/                  ← 图标资源
│   │   │   ├── indev/                   ← 输入驱动（按键）
│   │   │   ├── notifications/           ← 通知弹窗
│   │   │   ├── widget/                  ← 控件
│   │   │   ├── ui_conf.c/h              ← 菜单树配置
│   │   │   └── version.h                ← 版本信息
│   │   └── PowerControl/                ← 电源控制核心
│   │       ├── algorithm_control.c/h    ← PID / 准PR 控制器算法
│   │       ├── algorithm_filtering.c/h  ← 数字滤波算法
│   │       ├── conv_adc.c/h             ← ADC 采样与标定
│   │       ├── conv_controller.c/h      ← 控制状态机 + MainLoop
│   │       ├── conv_loop.c/h            ← 双 PR 电流环(A/B相) / BUCK 控制
│   │       ├── conv_protection.c/h      ← 过流过压保护 + 自动恢复
│   │       ├── conv_pwm.c/h             ← HRTIM PWM 驱动
│   │       ├── dashboard.c/h            ← 仪表盘界面
│   │       └── README.md                ← PowerControl 设计文档
│   ├── BSP/
│   │   ├── Buttons/                     ← 按键驱动
│   │   ├── OLED/                        ← OLED 底层驱动
│   │   └── u8g2/                        ← u8g2 图形库移植
│   ├── Core/
│   │   ├── Inc/                         ← HAL 外设头文件
│   │   └── Src/
│   │       ├── main.c                   ← 系统初始化入口
│   │       ├── app_freertos.c           ← FreeRTOS 任务/队列定义
│   │       └── stm32g4xx_it.c           ← 中断服务（HRTIM→MainLoop）
│   ├── Drivers/                         ← STM32G4 HAL + CMSIS
│   ├── MDK-ARM/                         ← Keil MDK 工程文件
│   ├── Middlewares/                      ← 第三方库
│   ├── NUEDC_2021B.ioc                  ← CubeMX 配置文件
│   ├── readme.md                        ← 本文件
│   └── LICENSE                          ← MIT 许可证
├── Hardware/                            ← 硬件设计（3 个 LCEDA-Pro 工程）
│   ├── LCEDA_MainBoard/                 ← 主控板（MCU、OLED、通信、IO 保护）
│   ├── LCEDA_MotherBoard/               ← 母板（功率插座、辅助电源、电压采样）
│   ├── LCEDA_PowerModule/               ← 驱动-功率一体模块（半桥+电感+电流传感器）
│   └── docs/                            ← 硬件设计文档、BOM、引脚定义
├── Images/                              ← 图片资源（待补充）
├── Model/                               ← 仿真模型（待补充）
└── Scripts/                             ← 辅助脚本（待补充）
```



## 关键模块说明

### 电源控制链

| 模块 | 文件 | 功能 |
|------|------|------|
| **ADC 采样** | `conv_adc.c/h` | HRTIM 触发同步采样，9 路信号 DMA 传输，物理量换算 |
| **控制算法** | `algorithm_control.c/h` | 位置式 PID、准比例谐振 (PR) 控制器（IIR 实现） |
| **环路算法** | `conv_loop.c/h` | 三相整流双 PR 电流环（A/B 相 120°）+ BUCK PI/P 控制 + 参考电流合成 |
| **PWM 驱动** | `conv_pwm.c/h` | HRTIM 四单元 PWM 配置，SVPWM/DPWM 占空比设置 |
| **保护系统** | `conv_protection.c/h` | 软件过流检测、HRTIM 硬件故障封锁、ADC 看门狗、自动恢复 |
| **控制状态机** | `conv_controller.c/h` | MainLoop (开关频率实时中断)、CONV_STOP/RUN/FAULT 状态切换 |

### 控制信号链

```
                        ┌───────────┐
  Ubus_ref ──→ [+]/[−] ─→│ PI 电压外环 │ ─→ Idc_ref (直流幅值)
                ↑        └───────────┘
              Ubus_fb
                        │ Idc_ref
                        ▼
  ┌──────────────────────────────────────────────┐
  │          参考电流波形合成                       │
  │                                              │
  │  ① 由 Uab, Ubc 重构 Ua, Ub, Uc              │
  │  ② RMS 归一化: Ux_norm = Ux / Urms          │
  │  ③ PF=1:   Iref_x = Idc_ref × Ux_norm       │
  │  ④ PF≠1:   数组延迟移相后再相乘               │
  │       延迟点数 = (φ/360°) × (fsw/fgrid)     │
  │                                              │
  │  结果: Iref_a, Iref_b (正弦), Iref_c 导出    │
  └──────────────────────┬───────────────────────┘
                         │ Iref_a, Iref_b
                         ▼
  ┌──────────────────────────────────────────────┐
  │         双 PR 电流内环 (A/B 相, 120°)         │
  │                                              │
  │  Ia_err = Iref_a - Ia_fb                     │
  │  Ib_err = Iref_b - Ib_fb                     │
  │       │            │                         │
  │    [PR_a(s)]    [PR_b(s)]                    │
  │       │            │                         │
  │     Va_ref      Vb_ref                       │
  │       │            │                         │
  │       └────┬───────┘                         │
  │            ▼                                 │
  │     Vc_ref = -Va_ref - Vb_ref                │
  │     + 零序注入 (min-max)                     │
  │     → SVPWM / DPWM-A                        │
  │     → SetDuty_Rec() → HRTIM A/B/C           │
  └──────────────────────────────────────────────┘

  ┌──────────────────────────────────────────────┐
  │  BUCK 控制:                                   │
  │  Uo_ref ─→ [+/−] ─→ PI → Ibuck_ref           │
  │                         [+/−] ─→ P → Duty    │
  │                   Ibuck_fb ↑                  │
  │  → SetDuty_Buck() → HRTIM D                  │
  └──────────────────────────────────────────────┘
```


---

## 复刻与测试

> 复刻步骤、调试过程及测试报告详见 [`copy_manual.md`](copy_manual.md)。

---

## 测试结果

### 基本要求

| 项目 | 条件 | 指标 | 实测值 | 结论 |
|:---:|------|:----:|:-----:|:----:|
| 输出电压 | $U_i=28\,\text{V}$, $I_o=2\,\text{A}$ | $U_o = 36\,\text{V} \pm 0.1\,\text{V}$ | <!-- TODO --> | <!-- TODO --> |
| 负载调整率 $S_I$ | $U_i=28\,\text{V}$, $I_o=0.1\to2.0\,\text{A}$ | $\leq 0.3\%$ | <!-- TODO --> | <!-- TODO --> |
| 电压调整率 $S_U$ | $I_o=2\,\text{A}$, $U_i=23\to33\,\text{V}$ | $\leq 0.3\%$ | <!-- TODO --> | <!-- TODO --> |
| 效率 $\eta$ | $U_i=28\,\text{V}$, $I_o=2\,\text{A}$, $U_o=36\,\text{V}$ | $\geq 85\%$ | <!-- TODO --> | <!-- TODO --> |

### 发挥部分

| 项目 | 条件 | 指标 | 实测值 | 结论 |
|:---:|------|:----:|:-----:|:----:|
| 功率因数 $\text{PF}$ | $U_i=28\,\text{V}$, $I_o=2\,\text{A}$, $U_o=36\,\text{V}$ | $\geq 0.99$ | <!-- TODO --> | <!-- TODO --> |
| 效率 $\eta$ | $U_i=28\,\text{V}$, $I_o=2\,\text{A}$, $U_o=36\,\text{V}$ | $\geq 95\%$ | <!-- TODO --> | <!-- TODO --> |
| PF 可调范围 | 数字设定 | $0.90 \sim 1.00$ | <!-- TODO --> | <!-- TODO --> |
| PF 设定误差 | 全范围 | $\leq 0.02$ | <!-- TODO --> | <!-- TODO --> |

> 详细测试平台、仪表接线、波形截图及效率曲线见 [`copy_manual.md`](copy_manual.md#测试结果)。

### 结论

<!-- TODO: 待实测后填写最终结论 -->

---

## 当前状态

> 项目进度、待办清单与已知问题详见 [`todolist.md`](todolist.md)。

---

## 许可证

本项目基于 [MIT 许可证](LICENSE) 开源。

版权所有 © 2026 TKWTL

---

## 参考

- [2021 NUEDC B 题官方文档](Docs/B_三相AC-DC变换电路.pdf)
- [STM32G474 参考手册 (RM0440)](https://www.st.com/resource/en/reference_manual/dm00451132.pdf)
- [u8g2 图形库](https://github.com/olikraus/u8g2)
- [MiaoUI 框架](APP/MiaoUI/README.md)


