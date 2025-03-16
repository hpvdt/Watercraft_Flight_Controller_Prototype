#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define NOISE_STDDEV 1  // Adjust noise intensity
#define NOISE_MEAN 0

typedef struct {
    double Kp, Ki, Kd;
    double integral_pitch, integral_height;
    double prev_pitch_err, prev_height_err;
    clock_t prev_time;
} PIDController;

// Function to generate Gaussian noise using Box-Muller transform
double gaussian_noise(double mean, double stddev) {
    double u1 = (double)rand() / RAND_MAX;
    double u2 = (double)rand() / RAND_MAX;
    double z = sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2); // Box-Muller transform
    return mean + z * stddev;
}

void initialize_PID(PIDController* pid, double Kp, double Ki, double Kd){
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    pid->integral_pitch = 0;
    pid->integral_height = 0;
    pid->prev_pitch_err = 0;
    pid->prev_height_err = 0;
    pid->prev_time = clock();
}

double system_response(double current_angle, double pid_output) {
    double K = 0.5; // System gain
    double noise = gaussian_noise(0, 2.0); // Large noise strength
    return current_angle + K * pid_output + noise;
}

void PID_Controller(PIDController* pid, double pitch_target, double height_target, 
                    double curr_pitch, double curr_height, 
                    double* pitch_output, double* height_output) {
    clock_t curr_time = clock();
    double dt = ((double)(curr_time - pid->prev_time)) / CLOCKS_PER_SEC; // Convert to seconds
    if (dt < 1e-6) dt = 1e-6; // Prevent division by zero

    // Add noise to sensor readings
    double noisy_pitch = curr_pitch + gaussian_noise(NOISE_MEAN, NOISE_STDDEV);
    double noisy_height = curr_height + gaussian_noise(NOISE_MEAN, NOISE_STDDEV);

    // Calculate Errors
    double pitch_err = pitch_target - noisy_pitch;
    double height_err = height_target - noisy_height;

    // PID Calculations for Pitch
    double pitch_prop = pid->Kp * pitch_err;
    pid->integral_pitch += pid->Ki * pitch_err * dt;
    double pitch_deriv = pid->Kd * (pitch_err - pid->prev_pitch_err) / dt;

    // PID Calculations for Height
    double height_prop = pid->Kp * height_err;
    pid->integral_height += pid->Ki * height_err * dt;
    double height_deriv = pid->Kd * (height_err - pid->prev_height_err) / dt;

    // Compute Outputs
    *pitch_output = pitch_prop + pid->integral_pitch + pitch_deriv;
    *height_output = height_prop + pid->integral_height + height_deriv;

    // Update Previous Values
    pid->prev_pitch_err = pitch_err;
    pid->prev_height_err = height_err;
    pid->prev_time = curr_time;
}

int main() {
    srand(time(NULL));  // Seed random number generator

    PIDController pid;
    initialize_PID(&pid, 0.8, 0.01, 0);  // Derivative term disabled due to strange exponential growth

    double pitch_target = 5.0, height_target = 10.0;
    double curr_pitch = 3.0, curr_height = 8.0;
    double pitch_output, height_output;

    // Print initial values
    printf("Before PID Step:\n");
    printf("Pitch Target: %.2f, Current Pitch: %.2f\n", pitch_target, curr_pitch);
    printf("Height Target: %.2f, Current Height: %.2f\n", height_target, curr_height);
    printf("Previous Pitch Error: %.2f, Integral Pitch: %.2f\n", pid.prev_pitch_err, pid.integral_pitch);
    printf("Previous Height Error: %.2f, Integral Height: %.2f\n", pid.prev_height_err, pid.integral_height);

    for (int t = 0; t < 60; t++) {
        // PID Control step with noise
        PID_Controller(&pid, pitch_target, height_target, curr_pitch, curr_height, &pitch_output, &height_output);
        curr_pitch = system_response(curr_pitch, pitch_output);

        // Print noisy results
        printf("Time: %d, Control: %f, Current Angle: %f\n", t, pitch_output, curr_pitch);
    }

    // Print final values
    printf("\nAfter PID Step:\n");
    printf("New Pitch Output: %.2f\n", pitch_output);
    printf("New Height Output: %.2f\n", height_output);
    printf("Updated Previous Pitch Error: %.2f, Integral Pitch: %.2f\n", pid.prev_pitch_err, pid.integral_pitch);
    printf("Updated Previous Height Error: %.2f, Integral Height: %.2f\n", pid.prev_height_err, pid.integral_height);

    return 0;
}
