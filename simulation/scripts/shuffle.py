import pandas as pd
import argparse
from sklearn.utils import shuffle


parser = argparse.ArgumentParser()
parser.add_argument('-f', required=True)
args = parser.parse_args()

filename = args.f

# Load the data from the CSV file
data = pd.read_csv(filename, header=None, index_col=0)
# Shuffle the rows of the DataFrame
shuffled_data = shuffle(data, random_state=1)

# Reset the index (optional, if you want to keep the original index)
shuffled_data.reset_index(drop=True, inplace=True)

# Write the shuffled data back to a CSV file
shuffled_data.to_csv("shuffled.csv", header=False, index=True)
