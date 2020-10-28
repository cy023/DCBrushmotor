/**
 * @file   pid.h
 * @author cy023
 * @date   2020.10.28
 * @brief  PID control 功能實現
 */

#ifndef __PID_H__
#define __PID_H__

typedef struct {
    float p_err;
    float d_err;
    float i_err;
} PIDerror_t;

typedef struct {
    float Kp;
    float Kd;
    float Ki;
} PIDgain_t;

typedef struct {
    float p_lerr;
    float d_lerr;
    float i_lerr;
} PIDlesterror_t;

typedef struct {
    PIDerror_t PID_error;
    PIDgain_t PID_gain;
    PIDlesterror_t PID_lesterror;
    float Cmd;
    volatile float FB;
    float output;
} PID_t;

void PID_controller(PID_t* PID_str);
void PD_controller(PID_t* PID_str);
void PI_controller(PID_t* PID_str);

#endif