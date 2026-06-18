#include <iostream>
#include <vector>
#include <queue>
#include <cmath>
#include <fstream>
#include <algorithm>
#include <string>

using namespace std;

// Configuration based on arXiv:2604.17436
const int OBSTACLE = 2;
const int ICE = 1;
const int SAFE = 0;

struct Node {
    int x, y;
    double g, h, cost;
    Node* parent;

    Node(int x, int y, double g, double h, double cost = 1.0, Node* parent = nullptr) 
        : x(x), y(y), g(g), h(h), cost(cost), parent(parent) {}

    double getF() const { return g + h; }
};

struct CompareNode {
    bool operator()(Node* const& n1, Node* const& n2) {
        return n1->getF() > n2->getF();
    }
};

double calculateH(int x, int y, int tx, int ty) {
    return sqrt(pow(x - tx, 2) + pow(y - ty, 2));
}

bool isValid(int x, int y, int r, int c) {
    return (x >= 0 && x < r && y >= 0 && y < c);
}

// Emulates SfS Micro-Slope Generation using grazing illumination constraints from the paper
vector<vector<double>> computeSfSSlopeMap(const vector<vector<int>>& baseGrid, double W) {
    int rows = baseGrid.size();
    int cols = baseGrid[0].size();
    vector<vector<double>> slopeMap(rows, vector<double>(cols, 0.0));

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (baseGrid[i][j] == OBSTACLE) {
                slopeMap[i][j] = 30.0; // Macro-hazard
            } else {
                // Simulate micro-topography variations scaled by the SfS smoothness weight W.
                // As W -> 0, fine-scale shading data reveals unconstrained micro-craters.
                double noiseFactor = sin(i * 0.5) * cos(j * 0.5);
                double sfsRefinement = (W < 1.0) ? abs(noiseFactor) * (15.0 / (W + 0.1)) : 2.0;
                
                // Keep simulated micro-slopes baseline realistic
                slopeMap[i][j] = min(25.0, max(1.0, sfsRefinement));
            }
        }
    }
    return slopeMap;
}

int main() {
    // 20x20 Base Grid Map
    vector<vector<int>> baseGrid = {
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,2,2,2,2,2,2,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,2,2,2,2,2,2,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,2,2,2,2,2,2,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,2,2,2,2,2,2,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,2,2,2,2,2,2,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,2,2,2,2,2,2,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
    };

    cout << "========================================================\n";
    cout << "  THUNDERBOLTS: SfS-DYNAMICS ROVER PATH PROCTOR SYSTEM  \n";
    cout << "  (Framework Base: arXiv:2604.17436)                   \n";
    cout << "========================================================\n\n";

    // CLI Input Gathering for SfS Variables
    double W; 
    cout << "Enter SfS Smoothness Weight W (e.g., 0.1 for high detail, 5.0 for smooth): ";
    cin >> W;

    double maxAllowableSlope;
    cout << "Enter Rover Max Traversable Slope Limit (degrees, e.g., 12.0): ";
    cin >> maxAllowableSlope;

    // Process Refined DEM Environment Map using Shape-from-Shading Model
    cout << "\n[Processing] Running SfS integration kernel over OHRC datasets...\n";
    vector<vector<double>> refinedSlopes = computeSfSSlopeMap(baseGrid, W);

    pair<int, int> start = {2, 2};
    pair<int, int> target = {17, 17};

    // A* Pathfinding Processing Engine incorporating dynamic local slopes
    int rows = baseGrid.size();
    int cols = baseGrid[0].size();
    priority_queue<Node*, vector<Node*>, CompareNode> openList;
    vector<vector<bool>> closedList(rows, vector<bool>(cols, false));

    openList.push(new Node(start.first, start.second, 0.0, calculateH(start.first, start.second, target.first, target.second)));

    int dx[] = {-1, 1, 0, 0, -1, -1, 1, 1};
    int dy[] = {0, 0, -1, 1, -1, 1, -1, 1};
    Node* finalNode = nullptr;

    while (!openList.empty()) {
        Node* current = openList.top();
        openList.pop();

        int x = current->x;
        int y = current->y;

        if (x == target.first && y == target.second) {
            finalNode = current;
            break;
        }

        if (closedList[x][y]) continue;
        closedList[x][y] = true;

        for (int i = 0; i < 8; ++i) {
            int nX = x + dx[i];
            int nY = y + dy[i];

            if (isValid(nX, nY, rows, cols) && !closedList[nX][nY]) {
                // Dynamically intercept micro-scale hazards uncovered via Shape-from-Shading
                if (refinedSlopes[nX][nY] > maxAllowableSlope) {
                    continue; // Actively proctor out unsafe micro-slopes
                }

                double stepDistance = (i >= 4) ? 1.414 : 1.0;
                // Add energy penalty scaling proportionally with local slope gradient
                double dynamicCost = stepDistance * (1.0 + (refinedSlopes[nX][nY] / 10.0));

                double newG = current->g + dynamicCost;
                double newH = calculateH(nX, nY, target.first, target.second);

                openList.push(new Node(nX, nY, newG, newH, refinedSlopes[nX][nY], current));
            }
        }
    }

    // Save outputs
    vector<pair<int, int>> path;
    if (finalNode != nullptr) {
        Node* curr = finalNode;
        while (curr != nullptr) {
            path.push_back({curr->x, curr->y});
            curr = curr->parent;
        }
        reverse(path.begin(), path.end());

        // Update the export file grids
        ofstream gridFile("lunar_grid.csv");
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                // Visualize dangerous micro-topography on the dashboard
                if (refinedSlopes[i][j] > maxAllowableSlope && baseGrid[i][j] != OBSTACLE) {
                    gridFile << 2; // Mark as terrain hazard
                } else {
                    gridFile << baseGrid[i][j];
                }
                if (j < cols - 1) gridFile << ",";
            }
            gridFile << "\n";
        }
        gridFile.close();

        ofstream pathFile("rover_path.csv");
        for (auto p : path) {
            pathFile << p.second << "," << p.first << "\n";
        }
        pathFile.close();

        cout << "\n[Success] Path safely proctored! Trajectory path vector: " << path.size() << " tracking points." << endl;
        cout << "[File IO] Fresh layers output to 'lunar_grid.csv' and 'rover_path.csv'." << endl;
    } else {
        cout << "\n[Failure] No viable route found. SfS processing indicates target isolated by terrain hazards." << endl;
    }
    return 0;
}