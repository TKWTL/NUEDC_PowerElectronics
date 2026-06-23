#ifndef __ALGORITHM_CONTROL_H__
#define __ALGORITHM_CONTROL_H__

#include "services.h"
#include "math.h"

//PID控制器结构体
typedef struct
{
    float fpKp;
    float fpKi;
    float fpKd;
    float fpDt;

    float fpKiDt;       // Ki * Dt，减少实时计算
    float fpKdDivDt;    // Kd / Dt，减少实时计算

    float fpKaw;        // 回算抗饱和系数，建议 0.05 ~ 1.0

    float fpErr;
    float fpPreErr;

    float fpIntegral;   // 积分项，单位直接等于输出单位
    float fpOutput;     // 实际限幅后的输出
    float fpRawOutput;  // 未限幅输出，便于调试
} ST_PID;


void PID_Init(ST_PID *pid,
              float Kp,
              float Ki,
              float Kd,
              float Dt,
              float Kaw);

void PID_SetParam(ST_PID *pid,
                  float Kp,
                  float Ki,
                  float Kd,
                  float Dt,
                  float Kaw);

void PID_Reset(ST_PID *pid);

float PID_Calc(ST_PID *pid,
               float Des,
               float FB,
               float OutMin,
               float OutMax);

//PR控制器结构体
typedef struct
{
	float fpDes;                 //控制变量目标值
	float fpFB;	                //控制变量反馈值
	float fpLim;	            //抗饱和时外界输入的增益值，用于标幺化

	float fpE;	                //本次偏差
	float fpPreE;                //上次偏差
	float fpPre_PreE;            //上上次偏差
	
	float fpU;	                //本次PID运算结果
    float pre_fpU;               //上次的输出
	float pre_pre_fpU;           //上上次的输出
    
    float output_fpU;            //经过限幅后的输出
    int pre_is_cut;             //上一次限幅情况

	float omiga_0;               //中心频率
    float omiga_c;               //频带宽度    
	float Kp;                    //比例系数
	float Kr;                    //谐振系数
	float fpDt;                  //控制周期
	
    float InvKp;                 //比例系数的倒数
    float n0;
	float n1;
	float n2;
	float d1;
	float d2;
} ST_PR;

void PR_Control(ST_PR *pstPid); //QPR控制器，带抗饱和
void PR_Init(ST_PR *PR);        //计算PR算法离散参数
void PR_Clean(ST_PR *PR);       //重置PR控制器状态

#endif
