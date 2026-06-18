import matplotlib.pyplot as plt # type: ignore
import numpy as np # type: ignore
import csv
from matplotlib.colors import ListedColormap # type: ignore
import os

# --- CONFIGURATION ---
GRID_FILE = 'lunar_grid.csv'
PATH_FILE = 'rover_path.csv'

def generate_dummy_data_if_missing():
    """Generates dummy files if the C++ files aren't ready yet, just for testing."""
    if not os.path.exists(GRID_FILE):
        print("Generating mock lunar_grid.csv...")
        grid = np.zeros((50, 50)) # 50x50 Safe terrain
        grid[20:30, 20:30] = 2    # Add a massive steep crater obstacle
        grid[40:45, 40:45] = 1    # Add a patch of subsurface ice
        np.savetxt(GRID_FILE, grid, delimiter=',', fmt='%d')

    if not os.path.exists(PATH_FILE):
        print("Generating mock rover_path.csv...")
        # A mock path navigating around the crater to the ice
        mock_path = [[5,5], [15,15], [15, 35], [30, 40], [42, 42]]
        with open(PATH_FILE, 'w', newline='') as f:
            writer = csv.writer(f)
            writer.writerows(mock_path)

def visualize_traverse():
    generate_dummy_data_if_missing()

    # 1. Load the environment grid
    grid = np.loadtxt(GRID_FILE, delimiter=',')

    # 2. Load the calculated A* path
    path_x, path_y = [], []
    with open(PATH_FILE, 'r') as f:
        reader = csv.reader(f)
        for row in reader:
            if row: # skip empty lines
                path_x.append(float(row[0]))
                path_y.append(float(row[1]))

    # 3. Create the Visualization Map
    fig, ax = plt.subplots(figsize=(10, 8))

    # Custom color map: 0: Grey (Safe), 1: Cyan (Ice), 2: Dark Blue (Obstacle)
    cmap = ListedColormap(['#b0b0b0', '#00e5ff', '#1c2833'])
    
    # Render the map
    cax = ax.imshow(grid, cmap=cmap, origin='upper')

    # 4. Plot the rover path vector on top
    ax.plot(path_x, path_y, color='#ff003c', linewidth=3, marker='o', markersize=4, label='Rover Path')

    # 5. Mark Start (Lander) and End (Ice)
    ax.plot(path_x[0], path_y[0], '^', color='#00ff00', markersize=12, label='Safe Landing Site')
    ax.plot(path_x[-1], path_y[-1], '*', color='#ffff00', markersize=14, label='Ice Target Reached')

    # Formatting for the presentation
    ax.set_title("Thunderbolts: Subsurface Ice Traverse Model", fontsize=16, fontweight='bold')
    ax.set_xlabel("X Coordinate", fontsize=12)
    ax.set_ylabel("Y Coordinate", fontsize=12)
    ax.legend(loc="upper right")
    ax.grid(color='white', linestyle='-', linewidth=0.5, alpha=0.2)

    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    visualize_traverse()