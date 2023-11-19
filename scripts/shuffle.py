# Shuffle and sample the input file and give each row a new flow id

import pandas as pd
import argparse
from sklearn.utils import shuffle


parser = argparse.ArgumentParser()
parser.add_argument('-fi', required=True)
parser.add_argument('-fo', required=True)
parser.add_argument('-n', default=0)
args = parser.parse_args()

num_of_lines = (int)(args.n)
filename = args.fi

# Load the data from the CSV file
data = pd.read_csv(filename, header=None, index_col=0,dtype=str)
if num_of_lines <1 :
    num_of_lines = data.shape[0]

# Shuffle the rows of the DataFrame
shuffled_data = shuffle(data, random_state=1)

# Reset the index (optional, if you want to keep the original index)
shuffled_data.reset_index(drop=True, inplace=True)

# Write the shuffled data back to a CSV file
# shuffled_data.to_csv("./proced_workloads/"+ "memcached" + "_shuffled.csv", header=False, index=True)
shuffled_data[0:num_of_lines].to_csv(args.fo, header=False, index=True)
