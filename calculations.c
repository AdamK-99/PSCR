#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "calculations.h"

_plant_params plant_params = {9000.0, 0.5, 6.0};
_reg_params reg_params = {300.0, 0.5, 2.0, -180};
_lock_controller_params lock_controller_params = {2.0, 1.0, 0.5};

double plant_input = 4, plant_output;
double control_with_flow;
double plant_H = 5.7; //decelowo uzytkownik na poczatku ma to wprowadzic
const double H_set = 5.2;

pthread_mutex_t input_plant_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t output_plant_mutex = PTHREAD_MUTEX_INITIALIZER;

int PI(double, double*);
void alpha_controller(int, int*, int*);
int lock_controller(int, int);

void plant_step()
{
    double flowrate, H_new;
    pthread_mutex_lock(&input_plant_mutex);
    flowrate = plant_input;
    pthread_mutex_unlock(&input_plant_mutex);
    H_new = flowrate * (1/plant_params.Ti) * plant_params.Ts_sim + plant_H;
    if(H_new > plant_params.Hlimit)
    {
        H_new = plant_params.Hlimit;
    }
    plant_H = H_new;
    pthread_mutex_lock(&output_plant_mutex);
    plant_output = H_new;
    pthread_mutex_unlock(&output_plant_mutex);
}

void calculate_control()
{
    double e, PI_control;
    double alpha1, alpha2;
    double e_alpha1, e_alpha2;
    double lock1_control, lock2_control;
    static double integrator_value, prev_lock1, prev_lock2;
    static double prev_u_lock1, prev_u_lock2;
    pthread_mutex_lock(&output_plant_mutex);
    e = H_set - plant_output;
    pthread_mutex_unlock(&output_plant_mutex);
    //sterowanie z regulatora PI
    PI_control = -PI(e, &integrator_value);
    //rozdzielacz katow
    alpha_controller(PI_control, &alpha1, &alpha2);
    //sterownik kierownic
    e_alpha1 = alpha1 - prev_lock1;
    e_alpha2 = alpha2 - prev_lock2;
    lock1_control = lock_controller(e_alpha1, prev_u_lock1);
    lock2_control = lock_controller(e_alpha2, prev_u_lock2);
    prev_u_lock1 = lock1_control;
    prev_u_lock2 = lock2_control;
    //
}

int PI(double e, double *integrator)
{
     double control;
     control = e*reg_params.P + (e*reg_params.Ts_reg+(*integrator))*reg_params.I;
     if (control < reg_params.limit)
     {
        return reg_params.limit;
     }
     else if (control > 0)
     {
        return 0;
     }
     return control;
}

void alpha_controller(int alpha, int *alpha1, int *alpha2)
{
    if(alpha < 30)
    {
        *alpha1 = 0;
        *alpha2 = 0;
    }
    else if(alpha>=30 && alpha<=60)
    {
        *alpha1 = alpha;
        *alpha2 = 0;
    }
    else if(alpha > 60 && alpha <=120)
    {
        *alpha1 = 60;
        *alpha2 = alpha - *alpha1;
    }
    else
    {
        *alpha1 = alpha/2;
        *alpha2 = *alpha1;
    }
}

int lock_controller(int e, int prev_lock)
{
    int u;
    if(prev_lock == 1)
    {
        if(e > lock_controller_params.b)
        {
            u = 1;
        }
        else 
        {
            u = 0;
        }
    }
    else if(prev_lock == 0)
    {
        if(e >=lock_controller_params.h)
        {
            u = 1;
        }
        else if (e <= -lock_controller_params.p - lock_controller_params.h)
        {
            u = -1;
        }
        else
        {
            u = 0;
        }
    }
    else
    {
        if(e >= -lock_controller_params.p - lock_controller_params.b)
        {
            u = 0;
        }
        else 
        {
            u = -1;
        }
    }
    return u;
}