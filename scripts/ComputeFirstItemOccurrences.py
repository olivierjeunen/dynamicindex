import argparse
import pandas as pd

if __name__ == '__main__':
    # Parse commandline arguments
    parser = argparse.ArgumentParser()
    parser.add_argument('data', type = str, help = 'The file contanining user-item interactions')
    args = parser.parse_args()

    # Read in datafile
    df = pd.read_csv(args.data)

    # Group per item
    i2f = df.groupby('item')['timestamp'].first().reset_index()

    # Write out
    i2f.to_csv('item_to_origin.csv',index=False)
