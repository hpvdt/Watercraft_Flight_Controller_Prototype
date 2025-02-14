# Sample PID Control
import numpy as np
import matplotlib.pyplot as plt
global vals
vals = []

def PID(start, set, t_step, t_end, k_p, k_i, k_d):
    global vals
    vals = [] #clears vals
    vals.append(start) #resets
    current = start
    integral = 0
    last = start
    for i in range(int(t_end/t_step)):
        p = k_p*t_step*(set-current)
        integral += (set-current)*t_step
        i = k_i*t_step*integral
        d = -k_d*(current-last)
        last = current
        current += p + i + d
        vals.append(current)


if __name__ == "__main__":
    t_end = 10
    t_step = 0.01
    PID(0, 10, t_step, t_end, 10, 0.1, 0.01)
    np_vals = np.array(vals)
    t = np.linspace(0, 10, int(t_end/t_step)+1)
    plt.plot(t, np_vals)
    plt.show()



