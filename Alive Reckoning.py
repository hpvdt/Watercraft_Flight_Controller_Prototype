def dead_reckoning_step(w_p, w_y, w_r, p_i, y_i, r_i, t):
    return [p_i+w_p*t, y_i+w_y*t, r_i+w_r*t]