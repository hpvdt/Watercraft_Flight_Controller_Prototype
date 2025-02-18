#include <stdio.h>
#include <time.h>

typedef struct {
    double Kp, Ki, Kd;
    double integral_pitch, integral_height;
    double prev_pitch_err, prev_height_err;
    clock_t prev_time;
} PIDController;

void initialize_PID(PIDController* pid, double Kp, double Ki, double Kd){
    (*pid).Kp = Kp;
    (*pid).Ki = Ki;
    (*pid).Kd = Kd;
    (*pid).integral_pitch = 0;
    (*pid).integra_height = 0;
    (*pid).prev_pitch_err = 0;
    (*pid).prev_height_err = 0;
    (*pid).prev_time = clock();
}

void PID_Controller(PIDController* pid, double pitch_target, double height_target, double curr_pitch, double curr_height, double* pitch_output, double* height_output){
    clock_t curr_time = clock();
    double dt = ((double)(curr_time - (*pid).prev_time)) / CLOCKS_PER_SEC; // Convert to seconds
    if (dt < 1e-6) dt = 1e-6; // Prevent division by zero

    // Calculate Errors
    double pitch_err = pitch_target - curr_pitch;
    double height_err = height_target - curr_height;

    // PID Calculations for Pitch
    double pitch_prop = (*pid).Kp * pitch_err;
    (*pid).integral_pitch += (*pid).Ki * pitch_err * dt;
    double pitch_deriv = (*pid).Kd * (pitch_err - (*pid).prev_pitch_err) / dt;

    // PID Calculations for Height
    double height_prop = (*pid).Kp * height_err;
    (*pid).integral_height += (*pid).Ki * height_err * dt;
    double height_deriv = (*pid).Kd * (height_err - (*pid).prev_height_err) / dt;

    // Compute Outputs
    *pitch_output = pitch_prop + (*pid).integral_pitch + pitch_deriv;
    *height_output = height_prop + (*pid).integral_height + height_deriv;

    // Update Previous Values
    (*pid).prev_pitch_err = pitch_err;
    (*pid).prev_height_err = height_err;
    (*pid).prev_time = curr_time;
}

int main() {
    PIDController pid;
    initialize_PID(&pid, 1.0, 0.1, 0.01); // Random Gains

    double pitch_target = 5.0, height_target = 10.0;
    double curr_pitch = 3.0, curr_height = 8.0;
    double pitch_output, height_output;

    // Simulating a single step of PID control
    PID_controller(&pid, pitch_target, height_target, curr_pitch, curr_height, &pitch_output, &height_output);

    return 0;
}