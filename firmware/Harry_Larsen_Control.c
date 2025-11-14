// Control equation: y = -(g1H + g2H’ + g3AoA + g4)
// control AOA: current_AOA + y = corrected_AOA 
// H’’ = K(AoA - H/V) - g => compute H = noise - ((H''+g/k)-AOA)V and feed back into equation
// H'' determined by taking ((current_H -prev_H -)/dt - (prev_H - two_prev_H)/dt)/dt
// V will be defined at every point in time => V(t)
// functions: Gaussian noise, dirac impulse, ramp function, step function
