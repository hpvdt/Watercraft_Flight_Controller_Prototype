// Control equation: u = -(g1H + g2H’ + g3AoA + g4)
// control AOA: current_AOA + y = corrected_AOA 
// H'' = K(AoA - H'/V) - g => compute H = noise - ((H''+g/k)-AOA)V and feed back into equation
// H'' determined by taking ((current_H -prev_H -)/dt - (prev_H - two_prev_H)/dt)/dt
// V will be defined at every point in time => V(t)

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// stores all larsen controller variables
typedef struct {
    double g1, g2, g3, g4;
    double K, V;
    double prev_H, prev_prev_H, external_Hdotdot;
    clock_t prev_time;
} LarsenController;

// initialize larsen controller
void initialize_larsen(LarsenController* larsen, double g1, double g2, double g3, double g4, double K){
    larsen->g1 = g1;
    larsen->g2 = g2;
    larsen->g3 = g3;
    larsen->g4 = g4;
    larsen->K = K;
    larsen->prev_H = 0;
    larsen->prev_prev_H = 0;
    larsen->external_Hdotdot = 0;
    larsen->prev_time = clock();
}

void Larsen_Controller(LarsenController* larsen, double H, double* AoA_output, double V) {
    clock_t curr_time = clock();
    // double dt = ((double)(curr_time - larsen->prev_time)) / CLOCKS_PER_SEC; // Convert to seconds
    // if (dt < 1e-6) dt = 1e-6; // Prevent division by zero
    double dt = 0.01; // just set a constant time frame for now
    
    // calculate after disturbance
    H += 2*larsen->external_Hdotdot*dt*dt;

    // larsen calculations
    double Hdot = (H - larsen->prev_H) / dt;
    double Hdotdot = (Hdot - (larsen->prev_H - larsen->prev_prev_H)/dt)/dt;
    double AoA = (Hdotdot + 9.81)/larsen->K + Hdot/V;
    double u = -(larsen->g1*H + larsen->g2*Hdot + larsen->g3*AoA + larsen->g4);

    // calculate output values
    *AoA_output = AoA + u;

    // Update Previous Values
    larsen->prev_prev_H = larsen->prev_H;
    larsen->prev_H = H;
    larsen->prev_time = curr_time;
}

// this function returns the current height
double get_H(double t) {
    double H = 0;
    return H;
}

// this function returns the current V 
double get_V(double t) {
    double V = 1.0;
    return V;
}

// disturbance function
double disturbance(LarsenController* larsen, double t, double prev_t, double prev_delta) {
    // revert quick change from previous time frame 
    if((t-prev_t) > (rand() % 3)){ // max 3 time frames to revert
        larsen->external_Hdotdot -= prev_delta;
        prev_t = 10000; prev_delta = 0; // reset prev stats
    }
    if(rand() % 100 < 50){ // disturbance rate: 50%
        if(rand() % 100 < 50){
            // quick change in acceleration 
            double delta = (rand() % 1000) / 100.0; // replace with gaussian later
            if(rand() % 2) delta *= -1; // negative direction
            larsen->external_Hdotdot += delta;
            return t, delta;
        }
        else{
            // step change in acceleration
            double delta = (rand() % 1000) / 100.0; // replace with gaussian later
            if(rand() % 2) delta *= -1; // negative direction
            larsen->external_Hdotdot += delta;
            return prev_t, prev_delta;
        }
    }
}

int main() {
    
    LarsenController larsen;
    initialize_larsen(&larsen, 1, 1, 1, 0, 1); // tune g1 g2 g3 g4 K

    double AoA_output, H_output; 
    double prev_t = 10000, prev_delta = 0;

    for (int t = 0; t < 2000; t++) {
        // get values from sensors; placeholders
        double H = get_H(t);
        double V = get_V(t);
        
        // random disturbance
        prev_t, prev_delta = disturbance(&larsen, t, prev_t, prev_delta); 
        
        // Larsen control
        Larsen_Controller(&larsen, H, &AoA_output, V);
        double AoA = AoA_output;
        // Print results every few time frames
        if(t%100 == 0){
            printf("Time: %.2d, Current Angle: %.2f\n", t, AoA);
        }
    }

    return 0;
}
