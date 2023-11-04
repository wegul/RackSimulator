#this script mark all flows of given file as isMem-0.
import os
import random
import sys
import re
import argparse
import math
import pandas as pd


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-fi', required=True)
    parser.add_argument('-fo', required=True)
    
    args = parser.parse_args()
    filename = args.fi
    outfilename =args.fo
    trace = pd.read_csv(filename, header=None)
    isMem = trace.iloc[:,1]
    for i in range(0, trace.shape[0]):
        # reverse
        if isMem[i]==1:
            trace.iloc[:,1] = 0 #isMem
            trace.iloc[:,2] = -1 #memType
            trace.iloc[:,6] = -1 #reqLen
            
    trace.to_csv(outfilename, header=False, index=False)
    
    


if __name__ == '__main__':
    main()