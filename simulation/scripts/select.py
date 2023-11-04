import pandas as pd
import numpy as np
import argparse


parser = argparse.ArgumentParser()
parser.add_argument('-fm', required=True)
parser.add_argument('-fn', required=True)
parser.add_argument('-x', required=True)
parser.add_argument('-r', required=True) # net/mem ratio
parser.add_argument('-fo', required=True)
args = parser.parse_args()
x = (int)(args.x) 
ratio = (float)(args.r)
cnt=20


print("file: {0}, cnt= {1}".format(args.fo,cnt))

# Load the data
mem_data = pd.read_csv(args.fm)
net_data = pd.read_csv(args.fn)

# Set the seed for reproducibility
seed1 = np.random.randint(0,1000)

# Randomly select x rows from mem.csv
mem_sample = mem_data.sample(n=x,random_state=seed1)

# Calculate the sum for the selected rows
mem_sum = mem_sample.iloc[:, 5].sum()
reqSize_sum = 0
for reqSize in mem_sample.iloc[:, 6]:
    if reqSize > 0:
        reqSize_sum += reqSize
mem_sum += reqSize_sum

# Initialize variables
min_diff = float('inf')
min_idx = -1
best_net_sample = None

while cnt>0:
    seed2 = np.random.randint(0,1000)
    cnt-=1
    print(cnt)
    # Iterate to find the best sample from net.csv
    for y in range(1, len(net_data) + 1):  # replace range as needed
        # Get all possible combinations of y rows from net.csv
        net_combinations = net_data.sample(n=y,random_state=seed2)
        net_sum = net_combinations.iloc[:, 5].sum()
        diff = abs(net_sum - mem_sum*ratio)
        if diff < min_diff:
            min_diff = diff
            min_idx = y
            best_net_sample = net_combinations
        if min_diff < 1000:
            break  # exit early if an exact match is found


ans = np.concatenate((mem_sample, best_net_sample), axis=0)
print("min diff= ",min_diff,end=", ")
print(min_idx)
df = pd.DataFrame(ans)
df.to_csv(args.fo, header=False, index=False)
# best_net_sample now holds the rows from net.csv that best match the sum from mem.csv
