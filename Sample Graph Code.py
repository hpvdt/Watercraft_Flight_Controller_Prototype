# Sample Distance Graph

import matplotlib.pyplot as plt
import numpy as np
import math
import scipy as scp

def initialize():
    global dist_arr
    filename = input("Enter file name: ")
    f = open(filename+".txt", "r")
    dist_arr = []
    l_list = []
    col_index_list = []
    newl_index_list = []
    for L in f:
        l_list.append(L + "\n")
        col_index_list.append(L.find(":"))
        newl_index_list.append(L.find("m"))
    for i in range(len(l_list)):
        if(l_list[i][col_index_list[i]+1:newl_index_list[i]] !=' '):
            dist_arr.append(int(l_list[i][col_index_list[i]+1:newl_index_list[i]]))

if __name__ == "__main__":
    initialize()
    np_dist = np.array(dist_arr)
    plt.plot(np_dist, color = "black")
    plt.xlabel("Measurement Number")
    plt.ylabel("Distance (mm)")
    plt.title("Distance Graph")
    plt.show()