#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "calculations.h"

_plant_params plant_params = {900000.0, 0.5, 6.0};
_reg_params reg_params = {300.0, 0.5, 2.0, -180};
_lock_controller_params lock_controller_params = {2.0, 1.0, 0.5};
_sluice_lock_params sluice_lock_params = {5000.0, 0.02};
_sluice_lock_params sluice_tanks_lock_params = {1500.0, 0.02};

double plant_input, plant_output;
double river_flowrate = 400.0; //natezenie przeplywu rzeki
double plant_H; //poziom wody w zbiorniku
int mode; //0 - auto, 1 - close, 2 - kierownica 1, 3 - kierownica 2, 4 - obie kierownice
const double H_set = 5.2, H_set_down = 2.2;
const double H_min_for_power_plant = 2.0; //minimalny spad, przy ktorym moze pracowac elektrownia, potrzebne przy sluzie
int lock1_control, lock2_control; //sygnaly sterujace kierowncami (1 - otwieranie, 0 - brak akcji, -1 - zamykanie)
double lock1_angle, lock2_angle; //aktualne otwarcie kierownic
int lock1_set = 60, lock2_set = 60; //nastawy kierwnic przy sterowaniu recznym
double sluice_lock_opened = 0.0;
double sluice_tanks_lock_opened = 0.0;
int sluice_step = 0;
int sluice_door_opened = 0;
int sluice_signal_to_close_door = 0;
int was_tank_used = 0;
const double autiliary_tank_volume = 30*30*2.6*1000; //objetosc*1000l

pthread_mutex_t input_plant_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t output_plant_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t locks_angles = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t locks_u = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mode_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t locks_set = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sluice_lock_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sluice_step_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sluice_door_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sluice_signal_to_close_door_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sluice_tank_lock_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t auxiliary_tank_used_mutex = PTHREAD_MUTEX_INITIALIZER;

double PI(double, double*);
void alpha_controller(int, int*, int*);
int lock_controller(int, int);
int lock(int, int);
double my_noise();

void plant_step() //krok symulacji obiektu
{
    double H_new, plant_input_local, H_prev;
    pthread_mutex_lock(&input_plant_mutex);
    plant_input_local = plant_input;
    pthread_mutex_unlock(&input_plant_mutex);
    H_new = plant_input_local * (1/plant_params.Ti) * plant_params.Ts_sim + plant_H;
    if(H_new > plant_params.Hlimit)
    {
        H_new = plant_params.Hlimit;
    }
    H_prev = plant_H;
    plant_H = H_new;
    pthread_mutex_lock(&output_plant_mutex);
    pthread_mutex_lock(&sluice_door_mutex);
    if(sluice_door_opened == 0)
    {
        plant_output = H_new;
    }
    else
    {
        plant_output = H_set_down;
        plant_H = H_prev;
    }
    pthread_mutex_unlock(&output_plant_mutex);
    pthread_mutex_unlock(&sluice_door_mutex);
}
void calculate_input() //obliczenie przeplywu wody przez uklad 
{
    pthread_mutex_lock(&locks_angles);
    pthread_mutex_lock(&locks_u);
    lock1_angle = lock(lock1_control, lock1_angle);
    lock2_angle = lock(lock2_control, lock2_angle);
    pthread_mutex_unlock(&locks_angles);
    pthread_mutex_unlock(&locks_u);

    //dodanie szumu
    double noise = my_noise();
    
    double lock1_angle_local, lock2_angle_local, sluice_lock_opened_local, sluice_tanks_lock_opened_local;

    pthread_mutex_lock(&locks_angles);
    lock1_angle_local = lock1_angle;
    lock2_angle_local = lock2_angle;
    pthread_mutex_unlock(&locks_angles);

    pthread_mutex_lock(&sluice_lock_mutex);
    sluice_lock_opened_local = sluice_lock_opened;
    pthread_mutex_unlock(&sluice_lock_mutex);

    pthread_mutex_lock(&sluice_tank_lock_mutex);
    sluice_tanks_lock_opened_local = sluice_tanks_lock_opened;
    pthread_mutex_unlock(&sluice_tank_lock_mutex);

    double output = -(lock1_angle_local+lock2_angle_local)/180*600 + noise - sluice_lock_params.flowrate*sluice_lock_opened_local + sluice_tanks_lock_opened_local * sluice_tanks_lock_params.flowrate;
    
    pthread_mutex_lock(&input_plant_mutex);
    plant_input = river_flowrate+output; //przeplyw rzeki odjac przeplyw przez zapore
    pthread_mutex_unlock(&input_plant_mutex);
}

void calculate_control() //wyznaczenie sterowania
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

void alpha_controller(int alpha, int *alpha1, int *alpha2) //rozdzielacz katow
{
    int mode_local, sluice_step_local;

    pthread_mutex_lock(&mode_mutex);
    mode_local = mode;
    pthread_mutex_unlock(&mode_mutex);

    pthread_mutex_lock(&sluice_step_mutex);
    sluice_step_local = sluice_step;
    pthread_mutex_unlock(&sluice_step_mutex);
    
    if(mode_local == 0)
    {
        if(alpha < 30)
        {
            *alpha1 = -4;
            *alpha2 = -4;
        }
        else if(alpha>=30 && alpha<=60)
        {
            *alpha1 = alpha;
            *alpha2 = -4;
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
        *alpha1 = -4;
        *alpha2 = -4;
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
    else if (mode_local == 4)
    {
        pthread_mutex_lock(&locks_set);
        *alpha1 = lock1_set;
        *alpha2 = lock2_set;
        pthread_mutex_unlock(&locks_set);
    }
    else if (mode_local == 5 && sluice_step_local == 0)
    {
        double water_lvl;
        pthread_mutex_lock(&output_plant_mutex);
        water_lvl = plant_output;
        pthread_mutex_unlock(&output_plant_mutex);
        if(water_lvl - H_set_down >= H_min_for_power_plant)
        {
            *alpha1 = 94;
            *alpha2 = 94;
        }
        else
        {
            *alpha1 = -4;
            *alpha2 = -4;
        }
    }
    else //sluza; zamykanie przy spuszczaniu wody
    {
        *alpha1 = -4;
        *alpha2 = -4;
    }
    
}

int lock_controller(int e, int prev_lock) //sterownik kierownic
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

int lock(int u, int integral) //kierownica
{
    double open_degree = u*plant_params.Ts_sim*2+integral;
    if(open_degree > 90)
    {
        return 90;
    } 
    else if (open_degree < 0)
    {
        return 0;
    }
    return open_degree;
}

double my_noise()
{
    return (rand() % (100 + 1 - 0) + 0)/20; //losowa liczba z przedzialu 0-5
}

void sluice_lock()
{
    int mode_local, sluice_step_local;
    pthread_mutex_lock(&mode_mutex);
    mode_local = mode;
    pthread_mutex_unlock(&mode_mutex);

    pthread_mutex_lock(&sluice_step_mutex);
    sluice_step_local = sluice_step;
    pthread_mutex_unlock(&sluice_step_mutex);

    if (mode_local == 5 && sluice_step_local == 0)
    {
        pthread_mutex_lock(&sluice_lock_mutex);
        if(sluice_lock_opened < 1.0 - sluice_lock_params.step + 0.01) //+0.01 bo 0.98 nie dzialalo
        {
            sluice_lock_opened += sluice_lock_params.step;
        }
        pthread_mutex_unlock(&sluice_lock_mutex);
    }
    else
    {
        pthread_mutex_lock(&sluice_lock_mutex);
        if(sluice_lock_opened > sluice_lock_params.step - 0.01)
        {
            sluice_lock_opened -= sluice_lock_params.step;
        }
        pthread_mutex_unlock(&sluice_lock_mutex);
    }
}

void sluice()
{
    int mode_local, sluice_step_local;
    pthread_mutex_lock(&mode_mutex);
    mode_local = mode;
    pthread_mutex_unlock(&mode_mutex);

    pthread_mutex_lock(&sluice_step_mutex);
    sluice_step_local = sluice_step;
    pthread_mutex_unlock(&sluice_step_mutex);

    if(mode_local == 5)
    {
        int sluice_signal_to_close_door_local;
        pthread_mutex_lock(&sluice_signal_to_close_door_mutex);
        sluice_signal_to_close_door_local = sluice_signal_to_close_door;
        pthread_mutex_unlock(&sluice_signal_to_close_door_mutex);

        double water_lvl;
        pthread_mutex_lock(&output_plant_mutex);
        water_lvl = plant_output;
        pthread_mutex_unlock(&output_plant_mutex);

        if(water_lvl < H_set_down && sluice_step_local == 0)
        {
            pthread_mutex_lock(&sluice_door_mutex);
            sluice_door_opened = 1;
            pthread_mutex_unlock(&sluice_door_mutex);

            pthread_mutex_lock(&sluice_step_mutex);
            sluice_step = 1;
            pthread_mutex_unlock(&sluice_step_mutex);
        }
        else if(sluice_signal_to_close_door_local == 1)
        {
            pthread_mutex_lock(&sluice_door_mutex);
            sluice_door_opened = 0;
            pthread_mutex_unlock(&sluice_door_mutex);

            int was_tank_used_local;
            pthread_mutex_lock(&auxiliary_tank_used_mutex);
            was_tank_used_local = was_tank_used;
            pthread_mutex_unlock(&auxiliary_tank_used_mutex);

            if(was_tank_used_local == 0)
            {
                pthread_mutex_lock(&sluice_tank_lock_mutex);
                if(sluice_tanks_lock_opened < 1.0 - sluice_tanks_lock_params.step + 0.01 && was_tank_used == 0)
                {
                    sluice_tanks_lock_opened += sluice_tanks_lock_params.step;
                }
            pthread_mutex_unlock(&sluice_tank_lock_mutex);
            }
            
            if(water_lvl - H_set_down > 2.6) //napelnienie do 4,8m
            {
                pthread_mutex_lock(&mode_mutex);
                mode = 0;
                pthread_mutex_unlock(&mode_mutex);

                pthread_mutex_lock(&sluice_signal_to_close_door_mutex);
                sluice_signal_to_close_door = 0;
                pthread_mutex_unlock(&sluice_signal_to_close_door_mutex);
                pthread_mutex_lock(&auxiliary_tank_used_mutex);
                if(was_tank_used == 0)
                {
                    was_tank_used = 1;
                }
                pthread_mutex_unlock(&auxiliary_tank_used_mutex);
            }
        }
    }
    else
    {
        pthread_mutex_lock(&sluice_step_mutex);
        sluice_step = 0;
        pthread_mutex_unlock(&sluice_step_mutex);

        pthread_mutex_lock(&sluice_signal_to_close_door_mutex);
        sluice_signal_to_close_door = 0;
        pthread_mutex_unlock(&sluice_signal_to_close_door_mutex);

        pthread_mutex_lock(&sluice_door_mutex);
        sluice_door_opened = 0;
        pthread_mutex_unlock(&sluice_door_mutex);

        pthread_mutex_lock(&sluice_tank_lock_mutex);
        if(sluice_tanks_lock_opened > sluice_tanks_lock_params.step - 0.01) //zamykanie zasuwy
        {
            sluice_tanks_lock_opened -= sluice_tanks_lock_params.step;
        }
        pthread_mutex_unlock(&sluice_tank_lock_mutex);
    }
}

void auxiliaryTank()
{
    static double current_volume;
    int was_tank_used_local;
    pthread_mutex_lock(&auxiliary_tank_used_mutex);
    was_tank_used_local = was_tank_used;
    pthread_mutex_unlock(&auxiliary_tank_used_mutex);
    if(was_tank_used_local == 1)
    {
        pthread_mutex_lock(&input_plant_mutex);
        current_volume += river_flowrate;
        pthread_mutex_unlock(&input_plant_mutex);
        if(current_volume >= autiliary_tank_volume)
        {
            pthread_mutex_lock(&auxiliary_tank_used_mutex);
            was_tank_used = 0;
            pthread_mutex_unlock(&auxiliary_tank_used_mutex);
            current_volume = 0.0; //mozna wyzerowac, to juz nieistotne, a po uzyciu i tak wyniesie 0
        }
    }
}
