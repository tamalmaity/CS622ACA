import pandas as pd
import matplotlib.pyplot as plt
input_file="acc_dist_"
val=input("Enter number of prog file on which you are running the program: ")
input_file=input_file+val+".txt"
data = pd.read_csv(input_file,sep=' ',header=None)
data = pd.DataFrame(data)

x = data[0]
y = data[1]
plt.plot(x, y,'r')
plt.show()
