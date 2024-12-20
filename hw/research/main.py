import pandas as pd
import matplotlib.pyplot as plt
def draw_table(df):
    sizes= df['size'].unique()
    sizes = sorted(list(sizes))
    for sz in sizes:
        if sz % 100 != 0:
            continue
        subset = df[df['size'] == sz]
        time_at_1proc = subset[subset['procs'] == 1]['time'].values[0]

        procs = subset['procs'].unique()
        print(f"{sz} & ", end = "")
        procs = sorted(list(procs))
        #procs = [1, 2, 4, 6, 8, 10]
        print("T & ", end = "")
        for i, p in enumerate(procs):
            time = subset[subset['procs'] == p]['time'].values[0]
            if i != len(procs) - 1:
                print(f"{time:.3f} & ", end = "")
            else:
                print(f"{time:.3f} \\\\")
        print("& A & ", end = "")
        for i, p in enumerate(procs):
            time = subset[subset['procs'] == p]['time'].values[0]
            acc = time_at_1proc / time
            if i != len(procs) - 1:
                print(f"{acc:.3f} & ", end = "")
            else:
                print(f"{acc:.3f} \\\\")
        print("& E & ", end = "")
        for i, p in enumerate(procs):
            time = subset[subset['procs'] == p]['time'].values[0]
            acc = time_at_1proc / time
            eff = acc / p
            if i != len(procs) - 1:
                print(f"{eff:.3f} & ", end = "")
            else:
                print(f"{eff:.3f} \\\\")
        print("\hline")
        
            
# Чтение данных из CSV файла
df = pd.read_csv('full_mpi.csv')
draw_table(df)
def draw_graphs():
    # Уникальные размеры
    sizes = df['size'].unique()
    ch = [100, 200, 500, 1000, 1500, 2000, 3000, 5000]
    # 1. График зависимости time от количества procs
    plt.figure(figsize=(12, 6))
    for size in sizes:
        if size not in ch:
            continue
        subset = df[df['size'] == size]
        plt.plot(subset['procs'], subset['time'], marker='o', label=f'Size {size}')

    plt.title('Зависимость времени от количества процессов')
    plt.xlabel('Количество процессов (procs)')
    plt.ylabel('Время (time)')
    plt.legend()
    plt.grid()
    plt.savefig("mpi_time.png")

    # 2. График зависимости ускорения от количества procs
    plt.figure(figsize=(12, 6))
    for size in sizes:
        if size not in ch:
            continue
        subset = df[df['size'] == size]
        time_at_1proc = subset[subset['procs'] == 1]['time'].values[0]
        acceleration = time_at_1proc / subset['time']
        plt.plot(subset['procs'], acceleration, marker='o', label=f'Size {size}')

    plt.title('Зависимость ускорения от количества процессов')
    plt.xlabel('Количество процессов (procs)')
    plt.ylabel('Ускорение')
    plt.legend()
    plt.grid()
    plt.savefig("mpi_acc.png")

    # 3. График зависимости эффективности от количества procs
    plt.figure(figsize=(12, 6))
    for size in sizes:
        if size not in ch:
            continue
        subset = df[df['size'] == size]
        time_at_1proc = subset[subset['procs'] == 1]['time'].values[0]
        acceleration = time_at_1proc / subset['time']
        efficiency = acceleration / subset['procs']
        plt.plot(subset['procs'], efficiency, marker='o', label=f'Size {size}')

    plt.title('Зависимость эффективности от количества процессов')
    plt.xlabel('Количество процессов (procs)')
    plt.ylabel('Эффективность')
    plt.legend()
    plt.grid()
    plt.savefig("mpi_eff.png")
draw_graphs()
