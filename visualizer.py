import matplotlib.pyplot as plt
import numpy as np
import csv
from matplotlib.colors import ListedColormap
import os

# --- MULTI-PATH CONFIGURATION ---
GRID_FILE = 'lunar_grid.csv'
SAFEST_PATH = 'path_safest.csv'
EFFICIENT_PATH = 'path_efficient.csv'
SHORTEST_PATH = 'path_shortest.csv'

def load_path_file(filename):
    """Safely loads a coordinate trajectory path if it exists."""
    path_x, path_y = [], []
    if os.path.exists(filename):
        with open(filename, 'r') as f:
            reader = csv.reader(f)
            for row in reader:
                if row:
                    path_x.append(float(row[0]))
                    path_y.append(float(row[1]))
    return path_x, path_y

# 1. Load the core map layers
if not os.path.exists(GRID_FILE):
    print(f"Error: {GRID_FILE} not found. Run the C++ engine first!")
    exit()

grid = np.loadtxt(GRID_FILE, delimiter=',')

# 2. Setup the visual plot window
fig, ax = plt.subplots(figsize=(10, 8))
plt.title("Thunderbolts: Multi-Objective Mission Dashboard", fontsize=14, fontweight='bold', pad=15)

# Custom color palette for terrain features
cmap = ListedColormap(['#ababab', '#00f6ff', '#1c2430']) 
ax.imshow(grid, cmap=cmap, origin='upper')

# 3. Load and plot all 3 distinct optimization paths
sx, sy = load_path_file(SAFEST_PATH)
ex, ey = load_path_file(EFFICIENT_PATH)
hx, hy = load_path_file(SHORTEST_PATH)

# Plot the lines with Arihant's specified color hierarchy
if sx:
    ax.plot(sx, sy, color='#2ecc71', linewidth=4, linestyle='-', marker='o', markersize=4, label='🟢 Safest Path (1-2° Slopes)')
if ex:
    ax.plot(ex, ey, color='#e67e22', linewidth=3, linestyle='--', marker='s', markersize=3, label='🟠 Efficient Path (2-5° Slopes)')
if hx:
    ax.plot(hx, hy, color='#e74c3c', linewidth=2, linestyle=':', marker='x', markersize=4, label='🔴 Shortest Path (5-8° Slopes)')

# 4. Mark positions for Lander and Subsurface Target Assets
# Start node (2,2) -> Column 2, Row 2
ax.plot(2, 2, marker='^', color='#00ff00', markersize=14, linestyle='None', label='Safe Landing Site')
# End node target center inside the ice patch
ax.plot(17, 17, marker='*', color='#ffff00', markersize=15, linestyle='None', label='Subsurface Ice Reached')

# Layout refinement adjustments
ax.set_xlabel("X Coordinate (Kilometers / Reference Matrix)", fontsize=11, labelpad=8)
ax.set_ylabel("Y Coordinate (Kilometers / Reference Matrix)", fontsize=11, labelpad=8)
ax.grid(True, which='both', color='white', linestyle='-', linewidth=0.3, alpha=0.5)
ax.legend(loc='upper right', framealpha=0.9, facecolor='#ffffff', edgecolor='#333333', fontsize=10)

plt.tight_layout()
plt.show()