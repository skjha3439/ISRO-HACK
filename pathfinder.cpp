#include <iostream>
#include <vector>
#include <queue>
#include <cmath>
#include <fstream>
#include <algorithm>
#include <string>

using namespace std;

// Configuration based on arXiv:2604.17436 & Team Innovation Parameters
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
// Structure to track landing site candidates
struct LandingSite {
    int x, y;
    double suitabilityScore;
};

double calculateH(int x, int y, int tx, int ty) {
    return sqrt(pow(x - tx, 2) + pow(y - ty, 2));
}

bool isValid(int x, int y, int r, int c) {
    return (x >= 0 && x < r && y >= 0 && y < c);
}

// Emulates SfS Micro-Slope Generation with targeted micro-scale topography hurdles injected
vector<vector<double>> computeSfSSlopeMap(const vector<vector<int>>& baseGrid, double W) {
    int rows = baseGrid.size();
    int cols = baseGrid[0].size();
    vector<vector<double>> slopeMap(rows, vector<double>(cols, 0.0));

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (baseGrid[i][j] == OBSTACLE) {
                slopeMap[i][j] = 30.0; // Macro-hazard crater wall
            } else {
                // Baseline smooth slope model
                double noiseFactor = sin(i * 0.5) * cos(j * 0.5);
                double sfsRefinement = (W < 1.0) ? abs(noiseFactor) * (15.0 / (W + 0.1)) : 1.5;
                slopeMap[i][j] = min(25.0, max(1.0, sfsRefinement));
            }
        }
    }

    // --- INTEGRATION OF REALISTIC HAZARD STRATEGIES ---
    // Inject a micro-ridge hurdle patch right in the path corridor (Slopes ~ 4.5° - 6.0°)
    // This forces SAFEST mode to detour while EFFICIENT/SHORTEST cut through it.
    for (int i = 4; i <= 7; ++i) {
        for (int j = 4; j <= 7; ++j) {
            if (baseGrid[i][j] != OBSTACLE) {
                slopeMap[i][j] = 5.5; 
            }
        }
    }

    // Inject a steep localized micro-crater lip field (Slopes ~ 7.0° - 7.8°) near the approach lane
    // This forces SAFEST and EFFICIENT to steer clear, leaving only SHORTEST to brave the straight route.
    for (int j = 11; j <= 15; ++j) {
        if (baseGrid[14][j] != OBSTACLE) {
            slopeMap[14][j] = 7.5;
        }
    }

    return slopeMap;
}

// Tailored A* Pathfinder core that alters weights and limits dynamically per objective criteria
vector<pair<int, int>> calculateObjectivePath(const vector<vector<int>>& grid, const vector<vector<double>>& slopes, 
                                             pair<int, int> start, pair<int, int> target, string mode) {
    int rows = grid.size();
    int cols = grid[0].size();
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

            if (isValid(nX, nY, rows, cols) && grid[nX][nY] != OBSTACLE && !closedList[nX][nY]) {
                double slope = slopes[nX][nY];
                double stepDistance = (i >= 4) ? 1.414 : 1.0;
                double slopePenalty = 0.0;

                // Code Integration of Arihant's Risk-Factor Hierarchy
                if (mode == "SAFEST") {
                    // Green Path Criteria: Prefers 1-2 degree slopes. Actively avoids higher angles.
                    if (slope > 2.0) {
                        slopePenalty = slope * 25.0; // Extreme path-cost scaling penalty
                    } else {
                        slopePenalty = slope * 1.5;
                    }
                } 
                else if (mode == "EFFICIENT") {
                    // Orange Path Criteria: Balances time & energy. Tolerates 2-5 degree micro-slopes.
                    if (slope > 5.0) {
                        slopePenalty = slope * 35.0; // Block paths above 5 degrees
                    } else {
                        slopePenalty = slope * 4.0;   // Moderate energy traction cost factor
                    }
                } 
                else if (mode == "SHORTEST") {
                    // Red Path Criteria: Prioritizes minimal distance. Tolerates up to critical 5-8 degree slopes.
                    if (slope > 8.0) {
                        continue; // Strict hard-abort cap if rover risk profile faces tipping hazard
                    }
                    slopePenalty = 0.0; // Distance is prioritized over baseline incline drag
                }

                double newG = current->g + stepDistance + slopePenalty;
                double newH = calculateH(nX, nY, target.first, target.second);

                openList.push(new Node(nX, nY, newG, newH, slope, current));
            }
        }
    }

    // Extract path coordinates
    vector<pair<int, int>> path;
    Node* curr = finalNode;
    while (curr != nullptr) {
        path.push_back({curr->x, curr->y});
        curr = curr->parent;
    }
    reverse(path.begin(), path.end());
    return path;
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
    cout << "  THUNDERBOLTS: MULTI-CRITERIA LANDING SITE & TRAJECTORY \n";
    cout << "  (SfS Engine Kernel Base: arXiv:2604.17436)             \n";
    cout << "========================================================\n\n";

    double W = 2.0; 
    vector<vector<double>> refinedSlopes = computeSfSSlopeMap(baseGrid, W);
    pair<int, int> targetIce = {17, 17}; // Centroid of the ice deposit

    // --- PHASE 2: CRITERIA SCANNING KERNEL ---
    cout << "[Scanning] Analyzing terrain metrics for optimal landing constraints..." << endl;
    LandingSite bestSite = {0, 0, -99999.0};

    for (int i = 0; i < baseGrid.size(); ++i) {
        for (int j = 0; j < baseGrid[0].size(); ++j) {
            // Hard Constraint: Lander cannot touch down on macro-obstacles or steep slopes (> 5 degrees)
            if (baseGrid[i][j] == OBSTACLE || baseGrid[i][j] == ICE || refinedSlopes[i][j] > 5.0) {
                continue; 
            }

            // Calculate criteria metrics
            double slopeSafety = 30.0 - refinedSlopes[i][j]; // Lower slope = higher safety score
            double distanceToIce = calculateH(i, j, targetIce.first, targetIce.second);
            double proximityScore = 40.0 - distanceToIce;    // Closer to ice target = better resource proximity

            // Multi-Criteria Weight Evaluation
            double totalScore = (0.6 * slopeSafety) + (0.4 * proximityScore);

            if (totalScore > bestSite.suitabilityScore) {
                bestSite.x = i;
                bestSite.y = j;
                bestSite.suitabilityScore = totalScore;
            }
        }
    }

    pair<int, int> computedStart = {bestSite.x, bestSite.y};
    cout << "\n[Target Found] Optimized Landing Site Evaluated at Coordinates: (" 
         << computedStart.second << ", " << computedStart.first << ")" << endl;
    cout << "  -> Suitability Evaluation Index Score: " << bestSite.suitabilityScore << "\n\n";

    // --- PHASE 1: EXECUTE MULTI-PATH TRAJECTORY FROM THE NEW SITE ---
    cout << "[Optimizing] Launching multi-path proctor from new landing coordinates..." << endl;
    vector<pair<int, int>> safestPath = calculateObjectivePath(baseGrid, refinedSlopes, computedStart, targetIce, "SAFEST");
    vector<pair<int, int>> efficientPath = calculateObjectivePath(baseGrid, refinedSlopes, computedStart, targetIce, "EFFICIENT");
    vector<pair<int, int>> shortestPath = calculateObjectivePath(baseGrid, refinedSlopes, computedStart, targetIce, "SHORTEST");

    // File Output Processing (Overwrites the old start marker coordinate dynamically in CSVs)
    ofstream gridFile("lunar_grid.csv");
    for (int i = 0; i < baseGrid.size(); ++i) {
        for (int j = 0; j < baseGrid[0].size(); ++j) {
            gridFile << baseGrid[i][j];
            if (j < baseGrid[0].size() - 1) gridFile << ",";
        }
        gridFile << "\n";
    }
    gridFile.close();

    ofstream fSafest("path_safest.csv"); for (auto p : safestPath) fSafest << p.second << "," << p.first << "\n"; fSafest.close();
    ofstream fEfficient("path_efficient.csv"); for (auto p : efficientPath) fEfficient << p.second << "," << p.first << "\n"; fEfficient.close();
    ofstream fShortest("path_shortest.csv"); for (auto p : shortestPath) fShortest << p.second << "," << p.first << "\n"; fShortest.close();

    return 0;
}