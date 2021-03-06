import pandas as pd
import matplotlib.pyplot as plt
import numpy as np 
input_file="LRU_acc_dist_"
val=input("Enter number of prog file on which you are running the program: ")
input_file=input_file+val+".txt"

data = pd.read_csv(input_file,sep=' ',header=None)
data = pd.DataFrame(data)

x = data[0]
y = data[1]
plt.plot(x, y,'r')

x1 = np.arange(0,np.float64(x[0]).item(), 0.0001)
y1 = [0]*len(x1)
plt.plot(x1,y1,'r')
#plt.xlim(xmin=0)

plt.show()