#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <algorithm>
#include <set>
#include <limits.h>
#include <random>
#include <chrono>
#include <iomanip> 
#include <unordered_dense.h>

using namespace std;
using namespace std::chrono;

ostringstream generateRandomContractionSequence(int numVertices) { 
    ostringstream contractionSequence;

    std::random_device rd;
    std::mt19937 gen(rd());

    std::vector<int> vertices(numVertices);
    std::iota(vertices.begin(), vertices.end(), 0);

    while (vertices.size() > 1) {
        std::uniform_int_distribution<> dist(0, vertices.size() - 1);
        int idx1 = dist(gen);
        int idx2 = dist(gen);
            
        // Ensure the indices are distinct.
        while (idx2 == idx1) {
            idx2 = dist(gen);
        }

        int vertex1 = vertices[idx1];
        int vertex2 = vertices[idx2];

        contractionSequence << vertex1 + 1 << " " << vertex2 + 1 << "\n"; // Adjusting to 1-based index

        // Emulating a merge by removing one of the vertices from the list
        vertices.erase(vertices.begin() + idx2);
    }
    return contractionSequence;
}

class Graph {
private:
    ankerl::unordered_dense::set<int> vertices;
    ankerl::unordered_dense::map<int, ankerl::unordered_dense::set<int>> adjListBlack;  // For black edges
    ankerl::unordered_dense::map<int, ankerl::unordered_dense::set<int>> adjListRed;    // For red edges
    std::map<int, ankerl::unordered_dense::set<int>> redDegreeToVertices;
    int width = 0; 

public:
    Graph() {}

    void addVertex(int v){
        vertices.insert(v);
        updateVertexRedDegree(v, 0);
    }
        
    // Adds n vertices to the graph numbered from 0 to n-1
    void addVertices(int n){
        for(int i = 0; i < n; i++){
            addVertex(i);
        }
    }

    void addEdge(int v1, int v2, const string& color = "black") {
        if (color == "black") {
            adjListBlack[v1].insert(v2);
            adjListBlack[v2].insert(v1);
        } else if (color == "red") {
            updateVertexRedDegree(v1, 1);
            updateVertexRedDegree(v2, 1);
            adjListRed[v1].insert(v2);
            adjListRed[v2].insert(v1);
        }    
    }

    void removeEdge(int v1, int v2) {
        if (adjListBlack[v1].find(v2) != adjListBlack[v1].end()) {
            adjListBlack[v1].erase(v2);
            adjListBlack[v2].erase(v1);
        } else if (adjListRed[v1].find(v2) != adjListRed[v1].end()) {
            updateVertexRedDegree(v1, -1);
            updateVertexRedDegree(v2, -1);
            adjListRed[v1].erase(v2);
            adjListRed[v2].erase(v1);
        }
    }

    ankerl::unordered_dense::map<int, ankerl::unordered_dense::set<int>> getAdjListBlack() {
        return adjListBlack;
    }

    ankerl::unordered_dense::map<int, ankerl::unordered_dense::set<int>> getAdjListRed() {
        return adjListRed;
    }

    void removeVertex(int vertex) {        
        // Remove the vertex from the black adjacency list and update neighbors
        if (adjListBlack.find(vertex) != adjListBlack.end()) {
            for (int neighbor : adjListBlack[vertex]) {
                removeEdge(neighbor, vertex);
            }

            adjListBlack.erase(vertex);
        }
        
        // Remove the vertex from the red adjacency list and update neighbors
        if (adjListRed.find(vertex) != adjListRed.end()) {
            for (int neighbor : adjListRed[vertex]) {
                removeEdge(neighbor, vertex);
            }
            adjListRed.erase(vertex);
        }
        
        vertices.erase(vertex);
        redDegreeToVertices[adjListRed[vertex].size()].erase(vertex);
    }

    void updateVertexRedDegree(int vertex, int diff) {
        int oldDegree = adjListRed[vertex].size();
        
        redDegreeToVertices[oldDegree].erase(vertex);
        if (redDegreeToVertices[oldDegree].empty()) {
            redDegreeToVertices.erase(oldDegree);
        }

        redDegreeToVertices[oldDegree + diff].insert(vertex);
    }

    std::vector<int> getTopNVerticesWithHighestRedDegree(int n) {
        std::vector<int> topVertices;
        for (auto it = redDegreeToVertices.rbegin(); it != redDegreeToVertices.rend() && topVertices.size() < n; ++it) {
            for (int vertex : it->second) {
                if (topVertices.size() >= n) break;
                topVertices.push_back(vertex);
            }
        }
        return topVertices;
    }

    void mergeVertices(int source, int twin){
        auto start = high_resolution_clock::now();
        removeEdge(source, twin);
        transferRedEdges(twin, source);
        markUniqueEdgesRed(source, twin);
        addNewRedNeighbors(source, twin);
        removeVertex(twin);
        updateWidth();
        // auto stop = high_resolution_clock::now();
        // auto duration = duration_cast<milliseconds>(stop - start);
        // int seconds_part = duration.count() / 1000;
        // int milliseconds_part = duration.count() % 1000;
        // std::cout << "c (Left " << vertices.size() << ") Merged in " << seconds_part << "." 
        //         << std::setfill('0') << std::setw(9) << milliseconds_part 
        //         << " seconds" << std::endl;
    }

    void transferRedEdges(int fromVertex, int toVertex) {
        // If the twin vertex has red edges
        if(adjListRed.find(fromVertex) != adjListRed.end()) {
            for (int vertex : adjListRed[fromVertex]) {
                if (adjListRed[toVertex].find(vertex) == adjListRed[toVertex].end()) {
                    addEdge(toVertex, vertex, "red");
                }
            }
        }
    }

    void markUniqueEdgesRed(int source, int twin) {
        // Convert the unordered_dense::set to std::set for set operations
        std::set<int> source_neighbors(adjListBlack[source].begin(), adjListBlack[source].end());
        std::set<int> twin_neighbors(adjListBlack[twin].begin(), adjListBlack[twin].end());

        std::set<int> toBecomeRed;
        std::set_difference(
            source_neighbors.begin(), source_neighbors.end(),
            twin_neighbors.begin(), twin_neighbors.end(),
            std::inserter(toBecomeRed, toBecomeRed.begin())
        );

        for (int v : toBecomeRed) {
            removeEdge(source, v);
            // don't understand why is this possible since were considering only black edges
            if (adjListRed[source].find(v) == adjListRed[source].end()) {
                addEdge(source, v, "red");
            }
        }
    }

    // something wrong here
    void addNewRedNeighbors(int source, int twin) {
        // Merge red and black edges for both source and twin
        std::set<int> mergedSourceNeighbors(adjListBlack[source].begin(), adjListBlack[source].end());
        if (adjListRed.find(source) != adjListRed.end()) {
            mergedSourceNeighbors.insert(adjListRed[source].begin(), adjListRed[source].end());
        }

        std::set<int> mergedTwinNeighbors(adjListBlack[twin].begin(), adjListBlack[twin].end());
        if (adjListRed.find(twin) != adjListRed.end()) {
            mergedTwinNeighbors.insert(adjListRed[twin].begin(), adjListRed[twin].end());
        }

        // Find edges of twin that are not adjacent to source
        std::set<int> newRedEdges;
        std::set_difference(
            mergedTwinNeighbors.begin(), mergedTwinNeighbors.end(),
            mergedSourceNeighbors.begin(), mergedSourceNeighbors.end(),
            std::inserter(newRedEdges, newRedEdges.end())
        );

        // Add these edges as red edges for source
        for (int v : newRedEdges) {
            addEdge(source, v, "red");
        }
    }

    int getScore(int v1, int v2) {
        ankerl::unordered_dense::set<int> neighbors_v1_temp = adjListBlack[v1];
        if (adjListRed.find(v1) != adjListRed.end()) {
            neighbors_v1_temp.insert(adjListRed[v1].begin(), adjListRed[v1].end());
        }

        ankerl::unordered_dense::set<int> neighbors_v2_temp = adjListBlack[v2];
        if (adjListRed.find(v2) != adjListRed.end()) {
            neighbors_v2_temp.insert(adjListRed[v2].begin(), adjListRed[v2].end());
        }

        // Convert to std::set for set operations
        std::set<int> neighbors_v1(neighbors_v1_temp.begin(), neighbors_v1_temp.end());
        std::set<int> neighbors_v2(neighbors_v2_temp.begin(), neighbors_v2_temp.end());

        std::set<int> common_neighbors;
        std::set<int> all_neighbors;
        std::set<int> all_neighbors_minus_common;

        std::set_intersection(neighbors_v1.begin(), neighbors_v1.end(),
                            neighbors_v2.begin(), neighbors_v2.end(),
                            std::inserter(common_neighbors, common_neighbors.begin()));

        std::set_union(neighbors_v1.begin(), neighbors_v1.end(),
                    neighbors_v2.begin(), neighbors_v2.end(),
                    std::inserter(all_neighbors, all_neighbors.begin()));

        all_neighbors.erase(v1);
        all_neighbors.erase(v2);

        std::set_difference(all_neighbors.begin(), all_neighbors.end(),
                            common_neighbors.begin(), common_neighbors.end(),
                            std::inserter(all_neighbors_minus_common, all_neighbors_minus_common.begin()));

        return all_neighbors_minus_common.size();
    }

    ostringstream findRandomContraction(){ 
        ostringstream contractionSequence;

        std::random_device rd;
        std::mt19937 gen(rd());

        while (vertices.size() > 1) {
            
            std::uniform_int_distribution<> dist(0, vertices.size() - 1);
            int idx1 = dist(gen);
            int idx2 = dist(gen);
            
            // Ensure the indices are distinct.
            while (idx2 == idx1) {
                idx2 = dist(gen);
            }

            auto it1 = vertices.begin();
            std::advance(it1, idx1);
            
            auto it2 = vertices.begin();
            std::advance(it2, idx2);

            contractionSequence << *it1 + 1 << " " << *it2 + 1 << "\n";
            mergeVertices(*it1, *it2);
        }
        return contractionSequence;
    }

    ostringstream findRedDegreeContraction(){ 
        ostringstream contractionSequence;
        
        while (vertices.size() > 1) {
            // auto start = high_resolution_clock::now();

            vector<int> highestDegreeVertices = getTopNVerticesWithHighestRedDegree(20);
            
            int bestScore = INT_MAX;
            pair<int, int> bestPair;

            for (int i = 0; i < highestDegreeVertices.size(); i++) {
                for (int j = i+1; j < highestDegreeVertices.size(); j++) {
                    int v1 = highestDegreeVertices[i];
                    int v2 = highestDegreeVertices[j];
                    int score = getScore(v1, v2);
                    
                    if (score < bestScore) {
                        bestScore = score;
                        bestPair = {v1, v2};
                    }
                }
            }

            contractionSequence << bestPair.first + 1 << " " << bestPair.second + 1 << "\n";
            mergeVertices(bestPair.first, bestPair.second);

            // auto stop = high_resolution_clock::now();
            // auto duration = duration_cast<milliseconds>(stop - start);
            // int seconds_part = duration.count() / 1000;
            // int milliseconds_part = duration.count() % 1000;
            // std::cout << "c (Left " << vertices.size() << ") Cycle in " << seconds_part << "." 
            // << std::setfill('0') << std::setw(9) << milliseconds_part 
            // << " seconds" << std::endl;
        }
        return contractionSequence;
    }


private:
    void updateWidth() {
        for (const auto& pair : adjListRed) {
            width = max(width, static_cast<int>(pair.second.size()));
        }
    }
};

int main() {
    Graph g;
    string line;
    int numVertices, numEdges;

    auto start = high_resolution_clock::now(); 

    while (getline(cin, line)) {
        if (line[0] == 'c') {
            continue;
        }

        vector<string> tokens;
        string token;
        std::stringstream tokenStream(line);
        while (tokenStream >> token) {
            tokens.push_back(token);
        }

        if (tokens[0] == "p" && tokens[1] == "tww") {
            numVertices = stoi(tokens[2]);
            numEdges = stoi(tokens[3]);
            // cout << generateRandomContractionSequence(numVertices).str();
            // return 0;
            g.addVertices(numVertices);
        } else {
            int u = stoi(tokens[0]);
            int v = stoi(tokens[1]);
            g.addEdge(u - 1, v - 1, "black");
        }
    }

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(stop - start);
    std::cout << "c Time taken too initialize the graph: " << duration.count() << " seconds" << std::endl;

    cout << g.findRedDegreeContraction().str();

    auto final_stop = high_resolution_clock::now();
    auto final_duration = duration_cast<seconds>(final_stop - start);
    std::cout << "c In total: " << final_duration.count() << " seconds" << std::endl;

    return 0;    
}