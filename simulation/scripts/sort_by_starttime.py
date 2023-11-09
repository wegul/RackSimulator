import pandas as pd
import argparse


parser = argparse.ArgumentParser()
parser.add_argument('-fi', required=True)
parser.add_argument('-fo', required=True)
parser.add_argument('-n', default=0)
args = parser.parse_args()

num_of_lines = (int)(args.n)
filename = args.fi
outfile = args.fo

# Load the data from the CSV file
data = pd.read_csv(filename, header=None, index_col=0)
if num_of_lines <1 :
    num_of_lines = data.shape[0]
    
ans = data.sort_values(data.columns[-1],ascending=True)
ans.reset_index(drop=True,inplace=True)
ans.to_csv(outfile,header=None)
