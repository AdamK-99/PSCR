#ifndef CALCULATIONS_H_
#define CALCULATIONS_H_

struct _plant_params
{
    double Ti;
    double Ts_sim;
    double Hlimit;
};
typedef struct _plant_params _plant_params;

struct _reg_params
{
    double P;
    double I;
    double Ts_reg;
    int limit;
};
typedef struct _reg_params _reg_params;


struct _lock_controller_params
{
    double h;
    double b;
    double p;
};
typedef struct _lock_controller_params _lock_controller_params;

struct _sluice_lock_params
{
    double flowrate;
    double step;
};
typedef struct _sluice_lock_params _sluice_lock_params;

extern _plant_params plant_params;
extern _reg_params reg_params;
extern _lock_controller_params lock_controller_params;
extern double plant_input, plant_output;
extern double river_flowrate;
extern double plant_H;
extern const double H_set, H_set_down;
extern int lock1_control, lock2_control;
extern double lock1_angle, lock2_angle;
extern int mode;
extern int lock1_set, lock2_set;
extern int sluice_door_opened;
extern int sluice_signal_to_close_door;
extern pthread_mutex_t input_plant_mutex;
extern pthread_mutex_t output_plant_mutex;
extern pthread_mutex_t locks_angles; 
extern pthread_mutex_t locks_u;
extern pthread_mutex_t mode_mutex;
extern pthread_mutex_t locks_set;
extern pthread_mutex_t sluice_lock_mutex;
extern pthread_mutex_t sluice_step_mutex;
extern pthread_mutex_t sluice_door_mutex;
extern pthread_mutex_t sluice_signal_to_close_door_mutex;

void plant_step();
void calculate_control();
void calculate_input();
void sluice_lock();
void sluice();

#endif