import pandas as pd
import matplotlib.pyplot as plt
import sys
import numpy as np

def plot_graphs(csv_file):
    df = pd.read_csv(csv_file)

    required_columns = ['size', 'time', 'core']
    for col in required_columns:
        if col not in df.columns:
            print(f"Error: Missing column '{col}' in the CSV file.")
            return

    plt.figure(figsize=(12, 6))
    
    for core in df['core'].unique():
        subset = df[df['core'] == core]
        plt.plot(subset['size'], subset['time'])

    plt.title('Обычная реализация')
    plt.xlabel('Размер матрицы')
    plt.ylabel('Время (сек.)')
    plt.grid()
    plt.savefig("regular.png")


if __name__ == "__main__":
    csv_file_path = "single.csv"
    plot_graphs(csv_file_path)
