#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "calculations.h"

_plant_params plant_params = {900000.0, 0.5, 6.0};
_reg_params reg_params = {300.0, 0.5, 2.0, -180};
_lock_controller_params lock_controller_params = {2.0, 1.0, 0.5};

double plant_input, plant_output;
double river_flowrate = 400;
double plant_H;
int mode; //0 - auto, 1 - close, 2 - kierownica 1, 3 - kierownica 2, 4 - obie kierownice
const double H_set = 5.2;
int lock1_control, lock2_control;
double lock1_angle, lock2_angle;
int lock1_set = 60, lock2_set = 60;

pthread_mutex_t input_plant_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t output_plant_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t locks_angles = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t locks_u = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mode_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t locks_set = PTHREAD_MUTEX_INITIALIZER;

double PI(double, double*);
void alpha_controller(int, int*, int*);
int lock_controller(int, int);
int lock(int, int);
double my_noise();

void plant_step()
{
    double H_new, plant_input_local;
    pthread_mutex_lock(&input_plant_mutex);
    plant_input_local = plant_input;
    pthread_mutex_unlock(&input_plant_mutex);
    H_new = plant_input_local * (1/plant_params.Ti) * plant_params.Ts_sim + plant_H;
    if(H_new > plant_params.Hlimit)
    {
        H_new = plant_params.Hlimit;
    }
    plant_H = H_new;
    pthread_mutex_lock(&output_plant_mutex);
    plant_output = H_new;
    pthread_mutex_unlock(&output_plant_mutex);
}
void calculate_input()
{
    pthread_mutex_lock(&locks_angles);
    pthread_mutex_lock(&locks_u);
    lock1_angle = lock(lock1_control, lock1_angle);
    lock2_angle = lock(lock2_control, lock2_angle);
    pthread_mutex_unlock(&locks_angles);
    pthread_mutex_unlock(&locks_u);

    //dodanie szumu
    double noise = my_noise();
    
    double lock1_angle_local, lock2_angle_local;
    pthread_mutex_lock(&locks_angles);
    lock1_angle_local = lock1_angle;
    lock2_angle_local = lock2_angle;
    pthread_mutex_unlock(&locks_angles);
    double output = -(lock1_angle_local+lock2_angle_local)/180*600+ noise;
    pthread_mutex_lock(&input_plant_mutex);
    plant_input = river_flowrate+output;
    pthread_mutex_unlock(&input_plant_mutex);
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
    pthread_mutex_lock(&mode_mutex);
    if(mode == 0)
    {
        pthread_mutex_unlock(&mode_mutex);
        PI_control = -PI(e, &integrator_value);
    }
    pthread_mutex_unlock(&mode_mutex);

    //rozdzielacz katow
    int alpha1, alpha2;
    alpha_controller(PI_control, &alpha1, &alpha2);

    //sterownik kierownic
    double e_alpha1, e_alpha2;

    pthread_mutex_lock(&locks_angles);
    e_alpha1 = alpha1 - lock1_angle;
    e_alpha2 = alpha2 - lock2_angle;
    pthread_mutex_unlock(&locks_angles);

    pthread_mutex_lock(&locks_u);
    lock1_control = lock_controller(e_alpha1, lock1_control);
    lock2_control = lock_controller(e_alpha2, lock2_control);
    pthread_mutex_unlock(&locks_u);
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
    int mode_local;

    pthread_mutex_lock(&mode_mutex);
    mode_local = mode;
    pthread_mutex_unlock(&mode_mutex);
    if(mode_local == 0)
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
    else if (mode_local == 1)
    {
        *alpha1 = 0;
        *alpha2 = 0;
    }
    else if (mode_local == 2)
    {
        pthread_mutex_lock(&locks_set);
        *alpha1 = lock1_set;
        pthread_mutex_unlock(&locks_set);
        pthread_mutex_lock(&locks_angles);
        *alpha2 = lock2_angle;
        pthread_mutex_unlock(&locks_angles);

    }
    else if (mode_local == 3)
    {
        pthread_mutex_lock(&locks_angles);
        *alpha1 = lock1_angle;
        pthread_mutex_unlock(&locks_angles);
        pthread_mutex_lock(&locks_set);
        *alpha2 = lock2_set;
        pthread_mutex_unlock(&locks_set);
    }
    else
    {
        pthread_mutex_lock(&locks_set);
        *alpha1 = lock1_set;
        *alpha2 = lock2_set;
        pthread_mutex_unlock(&locks_set);
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
    double open_degree = u*plant_params.Ts_sim*2+integral;
    if(open_degree > 90)
    {
        return 90;
    } 
    return open_degree;
}

double my_noise()
{
    return (rand() % (100 + 1 - 0) + 0)/20; //losowa liczba z przedzialu 0-5
}