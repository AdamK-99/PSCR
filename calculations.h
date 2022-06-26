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

extern _plant_params plant_params;
extern _reg_params reg_params;
extern _lock_controller_params lock_controller_params;
extern double plant_input, plant_output;
extern double control_with_flow;
extern double plant_H;
extern const double H_set;
extern pthread_mutex_t input_plant_mutex;
extern pthread_mutex_t output_plant_mutex;

void plant_step();
void calculate_control();

#endif