
#include <stdio.h>
#include <math.h>

uint8_t res1;
uint8_t res2;
uint8_t read_status;




void dead_reckoning(double ***res, double t, double *x_0, double *y_0, double *z_0, double w_p, double w_r, double w_y, double a_x, double a_y, double a_z, double r_z, double v_z) {
    double *r[4];
    double x[3];
    double y[3];
    double z[3];

    double p_c = cos(t*w_p);
    double p_s = sin(t*w_p);
    double r_c = cos(t*w_r);
    double r_s = sin(t*w_r);
    double y_c = cos(t*w_y);
    double y_s = sin(t*w_y);

    x[0] = x_0[0]*(p_c*y_c+p_s*r_s*y_s) + x_0[1]*(p_c*y_s-p_s*r_s*y_c) + x_0[2]*(p_s*y_c);
    x[1] = x_0[0]*(-r_c*y_s) + x_0[1]*(r_c*y_c) + x_0[2]*(r_s);
    x[2] = x_0[0]*(p_c*r_s*y_s-p_s*y_c) + x_0[1]*(-p_c*r_s*y_c-p_s*y_s) + x_0[2]*(p_c*r_c);

    y[0] = y_0[0]*(p_c*y_c+p_s*r_s*y_s) + y_0[1]*(p_c*y_s-p_s*r_s*y_c) + y_0[2]*(p_s*y_c);
    y[1] = y_0[0]*(-r_c*y_s) + y_0[1]*(r_c*y_c) + y_0[2]*(r_s);
    y[2] = y_0[0]*(p_c*r_s*y_s-p_s*y_c) + y_0[1]*(-p_c*r_s*y_c-p_s*y_s) + y_0[2]*(p_c*r_c);    

    z[0] = z_0[0]*(p_c*y_c+p_s*r_s*y_s) + z_0[1]*(p_c*y_s-p_s*r_s*y_c) + z_0[2]*(p_s*y_c);
    z[1] = z_0[0]*(-r_c*y_s) + z_0[1]*(r_c*y_c) + z_0[2]*(r_s);
    z[2] = z_0[0]*(p_c*r_s*y_s-p_s*y_c) + z_0[1]*(-p_c*r_s*y_c-p_s*y_s) + z_0[2]*(p_c*r_c);

    double z_g[3];
    z_g[0] = z[0]*a_x + z[1]*a_y + z[2]*a_z;
    z_g[1] = z_g[0]*t+v_z;
    z_g[2] = r_z + z_g[1]*t;

    r[0] = x;
    r[1] = y;
    r[2] = z;
    r[3] = z_g;

    *res = r;
}

void initialize(int check, double ****p_res) {
    if(check == 0) {
        double ***res = (double ***)malloc(sizeof(double **));
        double z_g[3];
        z_g[1] = 0;
        z_g[2] = 0;
        double x[] = {1, 0, 0};
        double y[] = {0, 1, 0};
        double z[] = {0, 0, 1};
        *res[0] = x;
        *res[1] = y;
        *res[2] = z;
        *res[3] = z_g;
        *p_res = res;
    } 
}

int main() {
}