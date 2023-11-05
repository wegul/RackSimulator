# Separate mem flows and net flows
import pandas as pd
import numpy as np
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('-f', required=True)
args = parser.parse_args()

filename = args.f
prefix = filename.split('_')[0]


data = pd.read_csv(filename, header=None, index_col=None,dtype=str)
data_net = []
data_mem = []

for i in range(0,data.shape[0]):
    if (int)(data.iloc[i,1]) ==0: #net
        data_net.append(data.iloc[i].values)
    else:
        data_mem.append(data.iloc[i].values)

netcsv=pd.DataFrame(data_net)
memcsv=pd.DataFrame(data_mem)
netcsv.to_csv(prefix+'_net.csv',header=None, index=None)
memcsv.to_csv(prefix+'_mem.csv',header=None, index=None)