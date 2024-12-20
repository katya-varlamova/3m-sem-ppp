import pandas as pd
import matplotlib.pyplot as plt
def draw_table(df):
    subset_0 = df[df['size'] == 3000]
    threads = df['threads'].unique()
    threads = sorted(list(threads))
    for th in threads:
        subset = subset_0[subset_0['threads'] == th]
        ss = subset_0[subset_0['procs'] == 1]
        time_at_1proc = ss[ss['threads'] == 1]['time'].values[0]

        procs = subset['procs'].unique()
        print(f"{th} & ", end = "")
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
            eff = acc / p / th
            if i != len(procs) - 1:
                print(f"{eff:.3f} & ", end = "")
            else:
                print(f"{eff:.3f} \\\\")
        print("\hline")
                
  
            
# Чтение данных из CSV файла
df = pd.read_csv('full_openmp.csv')
draw_table(df)
def draw_graphs(df):
    # Уникальные размеры
    df = df[df['size'] == 3000]
    ss = df[df['procs'] == 1]
    time_at_1proc = ss[ss['threads'] == 1]['time'].values[0]
    
    threads = df['threads'].unique()

    # 1. График зависимости time от количества procs
    plt.figure(figsize=(12, 6))
    for th in threads:
        subset = df[df['threads'] == th]
        plt.plot(subset['procs'], subset['time'], marker='o', label=f'Количество потоков {th}')

    plt.title('Зависимость времени от количества процессоров')
    plt.xlabel('Количество процессоров')
    plt.ylabel('Время')
    plt.legend()
    plt.grid()
    plt.savefig("openmp_time.png")

    # 2. График зависимости ускорения от количества procs
    plt.figure(figsize=(12, 6))
    for th in threads:
        subset = df[df['threads'] == th]
        acceleration = time_at_1proc / subset['time']
        plt.plot(subset['procs'], acceleration, marker='o', label=f'Количество потоков {th}')

    plt.xlabel('Количество процессоров')
    plt.title('Зависимость ускорения от количества процессоров')
    plt.ylabel('Ускорение')
    plt.legend()
    plt.grid()
    plt.savefig("openmp_acc.png")

    # 3. График зависимости эффективности от количества procs
    plt.figure(figsize=(12, 6))

    for th in threads:
        subset = df[df['threads'] == th]
        acceleration = time_at_1proc / subset['time']
        efficiency = acceleration / subset['procs'] / th
        plt.plot(subset['procs'], efficiency, marker='o', label=f'Количество потоков {th}')

    plt.xlabel('Количество процессоров')
    plt.title('Зависимость эффективности от количества процессоров')
    plt.ylabel('Эффективность')
    plt.legend()
    plt.grid()
    plt.savefig("openmp_eff.png")
draw_graphs(df)
