/**
 * @file   pid.c
 * @author cy023
 * @date   2020.10.28
 * @brief  PID control 功能實現
 */

#include "./PID.h"

void PID_controller (PID_t* PID_str)
{
    PID_str->PID_error.p_err = PID_str->Cmd - PID_str->FB;
    PID_str->PID_error.i_err += PID_str->PID_error.p_err;
    PID_str->PID_error.d_err = PID_str->PID_error.p_err - PID_str->PID_lesterror.p_lerr;
    PID_str->output = PID_str->PID_error.p_err * PID_str->PID_gain.Kp + \
                      PID_str->PID_error.i_err * PID_str->PID_gain.Ki + \
                      PID_str->PID_error.d_err * PID_str->PID_gain.Kd;
    PID_str->PID_lesterror.p_lerr = PID_str->PID_error.p_err;
}

void PD_controller (PID_t* PID_str)
{
    PID_str->PID_error.p_err = PID_str->Cmd - PID_str->FB;
    PID_str->PID_error.d_err = PID_str->PID_error.p_err - PID_str->PID_lesterror.p_lerr;
    PID_str->output = PID_str->PID_error.p_err * PID_str->PID_gain.Kp + \
                      PID_str->PID_error.d_err * PID_str->PID_gain.Kd;
    PID_str->PID_lesterror.p_lerr = PID_str->PID_error.p_err;
}

void PI_controller (PID_t* PID_str)
{
    PID_str->PID_error.p_err = PID_str->Cmd - PID_str->FB;
    PID_str->PID_error.i_err += PID_str->PID_error.p_err;
    PID_str->output = PID_str->PID_error.p_err * PID_str->PID_gain.Kp + \
                      PID_str->PID_error.i_err * PID_str->PID_gain.Ki;
}