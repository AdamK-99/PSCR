#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "calculations.h"

_plant_params plant_params = {9000.0, 0.5, 6.0};
_reg_params reg_params = {300.0, 0.5, 2.0, -180};
_lock_controller_params lock_controller_params = {2.0, 1.0, 0.5};

double plant_input, plant_output;
double river_flowrate = 4; //docelowo uzytkownik ma miec mozliwosc zmiany natezenia przeplywu rzeki
double plant_H = 5.7; //decelowo uzytkownik na poczatku ma to wprowadzic
const double H_set = 5.2;

pthread_mutex_t input_plant_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t output_plant_mutex = PTHREAD_MUTEX_INITIALIZER;

double PI(double, double*);
void alpha_controller(int, int*, int*);
int lock_controller(int, int);
int lock(int, int);
double my_noise();

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
    double e;
    pthread_mutex_lock(&output_plant_mutex);
    e = H_set - plant_output;
    pthread_mutex_unlock(&output_plant_mutex);

    //sterowanie z regulatora PI
    double PI_control;
    static double integrator_value;
    PI_control = -PI(e, &integrator_value);

    //rozdzielacz katow
    int alpha1, alpha2;
    alpha_controller(PI_control, &alpha1, &alpha2);

    //sterownik kierownic
    double e_alpha1, e_alpha2;
    static int prev_val_of_lock1, prev_val_of_lock2;
    static double prev_u_lock1, prev_u_lock2;
    e_alpha1 = alpha1 - prev_val_of_lock1;
    e_alpha2 = alpha2 - prev_val_of_lock2;

    double lock1_control, lock2_control;
    lock1_control = lock_controller(e_alpha1, prev_u_lock1);
    lock2_control = lock_controller(e_alpha2, prev_u_lock2);
    
    prev_u_lock1 = lock1_control;
    prev_u_lock2 = lock2_control;
    
    //kierownice
    double alpha_lock1, alpha_lock2;
    alpha_lock1 = lock(prev_u_lock1, prev_val_of_lock1);
    alpha_lock2 = lock(prev_u_lock2, prev_val_of_lock2);

    prev_val_of_lock1 = alpha_lock1;
    prev_val_of_lock2 = alpha_lock2;

    //dodanie szumu
    double noise = 0; //docelowo my_noise();, ale to po testach

    //wyliczenie przeplywu przez tame
    double output = -(alpha_lock1+alpha_lock2)/30 + noise;
    pthread_mutex_lock(&input_plant_mutex);
    //DODAC MUTEXA OD RIVER_FLOWRATE JAK ZROBIE ZMIANE PRZEPLYWU Z KLAWIATURY !!!!!!!!
    plant_input = river_flowrate+output;
    pthread_mutex_unlock(&input_plant_mutex);
}

double PI(double e, double *integrator)
{
     double control;
     control = e*reg_params.P + (e*reg_params.Ts_reg+(*integrator))*reg_params.I;
     if (control < reg_params.limit)
     {
        control = reg_params.limit;
     }
     else if (control > 0)
     {
        control = 0;
     }
     *integrator += e*reg_params.Ts_reg;
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

int lock(int u, int integral)
{
    double open_degree = u*reg_params.Ts_reg*2+integral;
    if(open_degree > 90)
    {
        return 90;
    } 
    return open_degree;
}

double my_noise()
{
    return (rand() % (100 + 1 - 0) + 0)/1000; //losowa liczba z przedzialu 0-0.1
}