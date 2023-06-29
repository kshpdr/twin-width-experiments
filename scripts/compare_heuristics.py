import pandas as pd
import matplotlib.pyplot as plt

def compare_results(file1, file2):
    # Load the data
    data1 = pd.read_csv(file1)
    data2 = pd.read_csv(file2)

    # Merge the data on the 'Test' column
    data = pd.merge(data1, data2, on='Test', suffixes=('_file1', '_file2'))

    # Create a new figure for the solution comparison
    plt.figure(figsize=(10, 6))
    plt.plot(data['Test'], data['Solution_file1'], label='File 1')
    plt.plot(data['Test'], data['Solution_file2'], label='File 2')
    plt.xlabel('Test')
    plt.ylabel('Solution')
    plt.title('Solution Comparison')
    plt.legend()
    plt.show()

    # Create a new figure for the time comparison
    plt.figure(figsize=(10, 6))
    plt.plot(data['Test'], data['Time_file1'], label='File 1')
    plt.plot(data['Test'], data['Time_file2'], label='File 2')
    plt.xlabel('Test')
    plt.ylabel('Time')
    plt.title('Time Comparison')
    plt.legend()
    plt.show()

# Usage:
# compare_results('/Users/koselev/Desktop/Bachelorarbeit/twin-width-solver/scripts/out/2023-06-15/results/find_degree_optimized_contraction/12-32-33-results-python-exact-public-find_degree_optimized_contraction.csv',
#                 '/Users/koselev/Desktop/Bachelorarbeit/twin-width-solver/scripts/out/2023-06-20/results/find_red_edges_contraction/10-37-23-results-python-exact-public--find_red_edges_contraction.csv')

# compare_results('',
#                 '')

compare_results('/Users/koselev/Desktop/Bachelorarbeit/twin-width-solver/scripts/out/2023-06-15/results/find_degree_optimized_contraction/13-18-12-results-python-heuristic-public-find_degree_optimized_contraction.csv',
                '/Users/koselev/Desktop/Bachelorarbeit/twin-width-solver/scripts/out/2023-06-20/results/find_red_edges_contraction/10-49-10-results-python-heuristic-public--find_red_edges_contraction.csv')