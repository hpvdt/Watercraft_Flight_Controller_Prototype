// Control equation: y = -(g1H + g2H’ + g3AoA + g4)
// control AOA: current_AOA + y = corrected_AOA 
// H'' = K(AoA - H/V) - g => compute H = noise - ((H''+g/k)-AOA)V and feed back into equation
// H'' determined by taking ((current_H -prev_H -)/dt - (prev_H - two_prev_H)/dt)/dt
// V will be defined at every point in time => V(t)
// functions: Gaussian noise, dirac impulse, ramp function, step function
// after solving the equation, H = V*(2*H_prev + H_prev_prev + dt^2(K*AOA-g))/(V+dt^2*K)


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

typedef struct {
    double g1, g2, g3, g4;
    double K, V;
    double prev_H, prev_prev_H;
    clock_t prev_time;
} LarsenController;


void initialize_larsen(LarsenController* larsen, double g1, double g2, double g3, double g4, double K){
    larsen->g1 = g1;
    larsen->g2 = g2;
    larsen->g3 = g3;
    larsen->g4 = g4;
    larsen->K = K;
    larsen->prev_H = 0;
    larsen->prev_prev_H = 0;
    larsen->prev_time = clock();
}

void Larsen_Controller(LarsenController* larsen, double pitch_target, double height_target, double AoA, double* AoA_output, double* H_output, double V) {
    clock_t curr_time = clock();
    double dt = ((double)(curr_time - larsen->prev_time)) / CLOCKS_PER_SEC; // Convert to seconds
    if (dt < 1e-6) dt = 1e-6; // Prevent division by zero

    // larsen calculations
    double H = V*(2*larsen->prev_H + larsen->prev_prev_H + dt*dt*(larsen->K*AoA-9.81))/(V+dt*dt*larsen->K);
    double y = -(larsen->g1*H + larsen->g2*(H-larsen->prev_H)/dt + larsen->g3*AoA + larsen->g4);

    // calculate output values
    *AoA_output = AoA + y;
    *H_output = V*(2*larsen->prev_H + larsen->prev_prev_H + dt*dt*(larsen->K*(AoA+y)-9.81))/(V+dt*dt*larsen->K);

    // Update Previous Values
    larsen->prev_prev_H = larsen->prev_H;
    larsen->prev_H = H;
    larsen->prev_time = curr_time;
}

int main() {


    LarsenController larsen;
    initialize_larsen(&larsen, 1, 0, 0, 0, 0); 

    double pitch_target = 5.0, height_target = 10.0;
    double AoA = 3.0;
    double V = 1.0;
    double AoA_output, H_output;

    // Print initial values
    printf("Before Larsen Step:\n");
    printf("Pitch Target: %.2f, Current Pitch: %.2f\n", pitch_target, AoA);

    for (int t = 0; t < 20; t++) {
        // PID Control step with noise
        Larsen_Controller(&larsen, pitch_target, height_target, AoA, &AoA_output, &H_output, V);
        AoA = AoA_output;
        // Print results
        printf("Time: %.2d, Current Angle: %.2f\n", t, AoA);
    }

    return 0;
}
