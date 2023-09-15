import pandas as pd
import matplotlib.pyplot as plt
import os

def compare_results(*file_paths):
    # Read the first file
    data = pd.read_csv(file_paths[0])
    data = data[['Test', 'Solution', 'Time']]  # Keep only necessary columns
    label1 = os.path.basename(file_paths[0]).split('find_')[1]
    print(label1)
    data.rename(columns={"Solution": f"Solution_{label1}", "Time": f"Time_{label1}"}, inplace=True)

    # Loop through the remaining files
    for file_path in file_paths[1:]:
        df = pd.read_csv(file_path)
        df = df[['Test', 'Solution', 'Time']]  # Keep only necessary columns
        label = os.path.basename(file_path).split('find_')[1] # Extract heuristic name from the file path
        df.rename(columns={"Solution": f"Solution_{label}", "Time": f"Time_{label}"}, inplace=True)

        # Merge the new data with the existing DataFrame
        data = pd.merge(data, df, on='Test')

    # Create a new figure for the solution comparison
    plt.figure(figsize=(10, 6))
    for col in data.columns:
        if "Solution_" in col:
            plt.plot(range(len(data)), data[col], label=col)
    plt.xlabel('Test Index')
    plt.ylabel('Solution')
    plt.title('Solution Comparison')
    plt.legend()
    plt.show()

    # Create a new figure for the time comparison
    plt.figure(figsize=(10, 6))
    for col in data.columns:
        if "Time_" in col:
            plt.plot(range(len(data)), data[col], label=col)
    plt.xlabel('Test Index')
    plt.ylabel('Time')
    plt.title('Time Comparison')
    plt.legend()
    plt.show()

# Usage:
# compare_results('/Users/koselev/Desktop/Bachelorarbeit/twin-width-solver/scripts/out/2023-06-15/results/find_degree_optimized_contraction/12-32-33-results-python-exact-public-find_degree_optimized_contraction.csv',
#                 '/Users/koselev/Desktop/Bachelorarbeit/twin-width-solver/scripts/out/2023-06-20/results/find_red_edges_contraction/10-37-23-results-python-exact-public--find_red_edges_contraction.csv')

# compare_results('',
#                 '')

compare_results('/Users/koselev/Desktop/Bachelorarbeit/twin-width-solver/scripts/out/full-runs/2023-06-15/results/find_degree_optimized_contraction/13-18-12-results-python-heuristic-public-find_degree_optimized_contraction.csv',
                '/Users/koselev/Desktop/Bachelorarbeit/twin-width-solver/scripts/out/full-runs/2023-07-04/results/find_degree_merge_simulation_optimized_contraction/17-24-33-results-python-heuristic-public--find_degree_merge_simulation_optimized_contraction.csv',
                '/Users/koselev/Desktop/Bachelorarbeit/twin-width-solver/scripts/out/full-runs/2023-07-05/results/find_red_edges_contraction/08-22-56-results-python-heuristic-public--find_red_edges_contraction.csv')

# '/Users/koselev/Desktop/Bachelorarbeit/twin-width-solver/scripts/out/full-runs/2023-06-15/results/find_degree_optimized_contraction/12-32-33-results-python-exact-public-find_degree_optimized_contraction.csv',
# '/Users/koselev/Desktop/Bachelorarbeit/twin-width-solver/scripts/out/full-runs/2023-07-01/results/find_red_edges_contraction/16-50-14-results-python-exact-public--find_red_edges_contraction.csv',
# '/Users/koselev/Desktop/Bachelorarbeit/twin-width-solver/scripts/out/full-runs/2023-07-01/results/find_table_set_merge_score_optimized_contraction_sequence/17-33-00-results-python-exact-public--find_table_set_merge_score_optimized_contraction_sequence.csv',
# '/Users/koselev/Desktop/Bachelorarbeit/twin-width-solver/scripts/out/full-runs/2023-07-03/results/find_degree_merge_simulation_optimized_contraction/16-57-24-results-python-exact-public--find_degree_merge_simulation_optimized_contraction.csv'
