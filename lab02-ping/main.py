import pandas as pd
import matplotlib.pyplot as plt
import sys
import numpy as np

def plot_graphs(csv_file):
    # Read the CSV file into a DataFrame
    df = pd.read_csv(csv_file)

    # Check if required columns exist
    required_columns = ['size', 'time', 'bandwidth', 'core']
    for col in required_columns:
        if col not in df.columns:
            print(f"Error: Missing column '{col}' in the CSV file.")
            return

    # Create the first graph: size vs time colored by core
    plt.figure(figsize=(12, 6))
    
    for core in df['core'].unique():
        subset = df[df['core'] == core]
        plt.plot(subset['size'], subset['time'], label=f'Core {core}')

    plt.title('Time')
    plt.xlabel('Size (KB)')
    plt.ylabel('Time (sec)')
    plt.xscale('log', base=2)
    plt.legend(title='Core')
    plt.grid()
    plt.savefig("time.png")

    # Create the second graph: size vs bandwidth colored by core
    plt.figure(figsize=(12, 6))

    for core in df['core'].unique():
        subset = df[df['core'] == core]
        plt.plot(subset['size'], subset['bandwidth'], label=f'Core {core}')

    plt.title('Bandwidth (MB/sec)')
    plt.xlabel('Size (KB)')
    plt.ylabel('Bandwidth (MB/sec)')
    plt.xscale('log', base=2)
    plt.legend(title='Core')
    plt.grid()
    plt.savefig("bandwidth.png")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python script.py <path_to_csv>")
        sys.exit(1)

    csv_file_path = sys.argv[1]
    plot_graphs(csv_file_path)
