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
#include <queue>

using namespace std;
using namespace std::chrono;

struct PairHash {
    template <class T1, class T2>
    std::size_t operator() (const std::pair<T1, T2>& p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);
        return h1 ^ h2;
    }
};

class Graph {
private:
    vector<int> vertices;
    vector<vector<int>> adjListBlack;  // For black edges
    vector<vector<int>> adjListRed;    // For red edges
    vector<vector<int>> redDegreeToVertices; // vertex id saved
    int width = 0;

public:
    Graph() {}

    Graph(const Graph &g) {
        this->vertices = g.vertices;
        this->adjListBlack = g.adjListBlack;
        this->adjListRed = g.adjListRed;
        this->redDegreeToVertices = g.redDegreeToVertices;
        this->width = g.width;
    }

    void addVertex(int v){
        vertices.push_back(v);
        updateVertexRedDegree(v, 0);
    }
        
    // Adds n vertices to the graph numbered from 0 to n-1
    void addVertices(int n){
        vertices.resize(n);
        adjListBlack.resize(n);
        adjListRed.resize(n);

        std::iota(vertices.begin(), vertices.end(), 0); // populate vertices with 0...n-1
        redDegreeToVertices.insert(redDegreeToVertices.begin(), vertices);
    }

    void addVertices(int n, vector<int> ids){
        adjListBlack.resize(n);
        adjListRed.resize(n);

        vertices = ids;
        redDegreeToVertices.insert(redDegreeToVertices.begin(), ids);
    }


    void addEdge(int v1, int v2, const string& color = "black") {
        if (color == "black") {
            adjListBlack[v1].push_back(v2);
            adjListBlack[v2].push_back(v1);
        } else if (color == "red") {
            updateVertexRedDegree(v1, 1);
            updateVertexRedDegree(v2, 1);
            adjListRed[v1].push_back(v2);
            adjListRed[v2].push_back(v1);
        }
    }

    void removeEdge(int v1, int v2) {
        if (std::find(adjListBlack[v1].begin(), adjListBlack[v1].end(), v2) != adjListBlack[v1].end()) {
            // order matters since updateVertexDegree uses adjListBlack's state
            adjListBlack[v1].erase(std::remove(adjListBlack[v1].begin(), adjListBlack[v1].end(), v2), adjListBlack[v1].end());
            adjListBlack[v2].erase(std::remove(adjListBlack[v2].begin(), adjListBlack[v2].end(), v1), adjListBlack[v2].end());
        } else if (std::find(adjListRed[v1].begin(), adjListRed[v1].end(), v2) != adjListRed[v1].end()) {
            updateVertexRedDegree(v1, -1);
            updateVertexRedDegree(v2, -1);
            adjListRed[v1].erase(std::remove(adjListRed[v1].begin(), adjListRed[v1].end(), v2), adjListRed[v1].end());
            adjListRed[v2].erase(std::remove(adjListRed[v2].begin(), adjListRed[v2].end(), v1), adjListRed[v2].end());
        }
    }

    void removeVertex(int vertex) {        
        // Remove the vertex from the black adjacency list and update neighbors
        if (!adjListBlack[vertex].empty()) {
            vector<int> neighbors = adjListBlack[vertex];
            for (int neighbor : neighbors) {
                removeEdge(neighbor, vertex);
            }
        }
        
        // Remove the vertex from the red adjacency list and update neighbors
        if (!adjListRed[vertex].empty()) {
            vector<int> neighbors = adjListRed[vertex];
            for (int neighbor : neighbors) {
                removeEdge(neighbor, vertex);
            }
        }
        
        vertices.erase(std::find(vertices.begin(), vertices.end(), vertex));
        redDegreeToVertices[adjListRed[vertex].size()].erase(std::remove(redDegreeToVertices[adjListRed[vertex].size()].begin(), redDegreeToVertices[adjListRed[vertex].size()].end(), vertex));
    }

    int getWidth() const {
        return width;
    }

    vector<int> getVertices() {
        return vertices;
    }

    std::vector<Graph> findConnectedComponents() {
        vector<int> visited;
        std::vector<Graph> componentGraphs;
        int counter = 1;

        for (int vertex : vertices) {
            auto start = high_resolution_clock::now();
            if (std::find(visited.begin(), visited.end(), vertex) == visited.end()) {
                std::vector<int> component;
                dfs(vertex, visited, component);
                Graph subGraph;
                subGraph.addVertices(vertices.size(), component);
                for (int v : component) {
                    for (int neighbor : adjListBlack[v]) {
                        if (v < neighbor) { 
                            subGraph.addEdge(v, neighbor, "black");
                        }
                    }
                }

                componentGraphs.push_back(subGraph);
            }
            // auto stop = high_resolution_clock::now();
            // auto duration = duration_cast<milliseconds>(stop - start);
            // int seconds_part = duration.count() / 1000;
            // int milliseconds_part = duration.count() % 1000;
            // std::cout << "c Cycle " << counter <<  " in " << seconds_part << "." 
            // << std::setfill('0') << std::setw(9) << milliseconds_part 
            // << " seconds" << std::endl;
            // counter++;
        }

        return componentGraphs;
    }

    void dfs(int v, vector<int>& visited, std::vector<int>& component) {
        visited.push_back(v);
        component.push_back(v);
        
        // For black edges
        for (int neighbor : adjListBlack[v]) {
            if (std::find(visited.begin(), visited.end(), neighbor) == visited.end()) {
                dfs(neighbor, visited, component);
            }
        }

        // For red edges
        for (int neighbor : adjListRed[v]) {
            if (std::find(visited.begin(), visited.end(), neighbor) == visited.end()) {
                dfs(neighbor, visited, component);
            }
        }
    }


    void updateVertexRedDegree(int vertex, int diff) {
        int oldDegree = adjListRed[vertex].size();
        int newDegree = oldDegree + diff;
        redDegreeToVertices[oldDegree].erase(std::remove(redDegreeToVertices[oldDegree].begin(), redDegreeToVertices[oldDegree].end(), vertex), redDegreeToVertices[oldDegree].end());
        
        if (redDegreeToVertices.size() <= newDegree) redDegreeToVertices.resize(newDegree + 1);
        redDegreeToVertices[oldDegree + diff].push_back(vertex);
    }

    std::vector<int> getTopNVerticesWithLowestRedDegree(int n) {
        std::vector<int> topVertices;
        
        for (const auto& degreeVector : redDegreeToVertices) {
            for (int vertex : degreeVector) {
                if (topVertices.size() >= n) break;
                topVertices.push_back(vertex);
            }
            if (topVertices.size() >= n) break;
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
    }

    void addNewRedNeighbors(int source, int twin) {
        // Merge red and black edges for both source and twin
        vector<int> mergedSourceNeighbors(adjListBlack[source].begin(), adjListBlack[source].end());
        // if (!adjListRed[source].empty()) {
        //     mergedSourceNeighbors.insert(mergedSourceNeighbors.end(), adjListRed[source].begin(), adjListRed[source].end());
        // }

        std::vector<int> mergedTwinNeighbors(adjListBlack[twin].begin(), adjListBlack[twin].end());
        // if (!adjListRed[twin].empty()) {
        //     mergedTwinNeighbors.insert(mergedTwinNeighbors.end(), adjListRed[twin].begin(), adjListRed[twin].end());
        // }

        // Find edges of twin that are not adjacent to source
        std::vector<int> newRedEdges;
        std::set_difference(
            mergedTwinNeighbors.begin(), mergedTwinNeighbors.end(),
            mergedSourceNeighbors.begin(), mergedSourceNeighbors.end(),
            std::inserter(newRedEdges, newRedEdges.end())
        );

        // Add these edges as red edges for source
        for (int v : newRedEdges) {
            if (std::find(adjListRed[source].begin(), adjListRed[source].end(), v) == adjListRed[source].end()) {
                addEdge(source, v, "red");
            }
        }
    }


    void transferRedEdges(int fromVertex, int toVertex) {
        // If the twin vertex has red edges
        if(!adjListRed[fromVertex].empty()) {
            for (int vertex : adjListRed[fromVertex]) {
                if (std::find(adjListRed[toVertex].begin(), adjListRed[toVertex].end(), vertex) == adjListRed[toVertex].end()) {
                    addEdge(toVertex, vertex, "red");
                }
            }
        }
    }

    void deleteTransferedEdges(int vertex, vector<int> neighbors) {
        if(!neighbors.empty()) {
            for (int neighbor : neighbors) {
                removeEdge(vertex, neighbor);
            }
        }
    }

    void markUniqueEdgesRed(int source, int twin) {
        // Convert the unordered_dense::set to std::set for set operations
        vector<int> source_neighbors(adjListBlack[source].begin(), adjListBlack[source].end());
        vector<int> twin_neighbors(adjListBlack[twin].begin(), adjListBlack[twin].end());

        vector<int> toBecomeRed;
        std::set_difference(
            source_neighbors.begin(), source_neighbors.end(),
            twin_neighbors.begin(), twin_neighbors.end(),
            std::inserter(toBecomeRed, toBecomeRed.begin())
        );

        for (int v : toBecomeRed) {
            removeEdge(source, v);
            // don't understand why is this possible since were considering only black edges
            if (std::find(adjListRed[source].begin(), adjListRed[source].end(), v) == adjListRed[source].end()) {
                addEdge(source, v, "red");
            }
        }
    }

    int getScore(int v1, int v2) {
        if (v1 == v2){
            cout << "hui";
        }
        vector<int> neighbors_v1 = adjListBlack[v1];
        if (!adjListRed[v1].empty()) {
            neighbors_v1.insert(neighbors_v1.end(), adjListRed[v1].begin(), adjListRed[v1].end());
        }

        vector<int> neighbors_v2 = adjListBlack[v2];
        if (!adjListRed[v2].empty()) {
            neighbors_v2.insert(neighbors_v2.end(), adjListRed[v2].begin(), adjListRed[v2].end());
        }

        vector<int> symmetric_difference;
        std::set_symmetric_difference(neighbors_v1.begin(), neighbors_v1.end(),
                                    neighbors_v2.begin(), neighbors_v2.end(),
                                    std::inserter(symmetric_difference, symmetric_difference.begin()));
        auto it1 = std::find(symmetric_difference.begin(), symmetric_difference.end(), v1);
        if (it1 != symmetric_difference.end()) {
            symmetric_difference.erase(it1);
        }

        auto it2 = std::find(symmetric_difference.begin(), symmetric_difference.end(), v2);
        if (it2 != symmetric_difference.end()) {
            symmetric_difference.erase(it2);
        }

        return symmetric_difference.size();
    }


    ostringstream findRedDegreeContraction(){ 
        ostringstream contractionSequence;
        ankerl::unordered_dense::map<pair<int, int>, int, PairHash> scores;
        auto heuristic_start_time = high_resolution_clock::now();
        
        int iterationCounter = 0;
        while (vertices.size() > 1) {
            auto start = high_resolution_clock::now();

            vector<int> lowestDegreeVertices = getTopNVerticesWithLowestRedDegree(20);
            
            int bestScore = INT_MAX;
            pair<int, int> bestPair;

            for (int i = 0; i < lowestDegreeVertices.size(); i++) {
                for (int j = i+1; j < lowestDegreeVertices.size(); j++) {
                    int v1 = lowestDegreeVertices[i];
                    int v2 = lowestDegreeVertices[j];
                    // if (!getTwoNeighborhood(v1).contains(v2)) continue;
                    if (v2 > v1) {
                        std::swap(v1, v2);
                    }

                    auto it = scores.find({v1, v2});
                    int score;
                    if (it != scores.end()) {
                        score = it->second;
                    }
                    else {
                        score = getScore(v1, v2);
                        scores[{v1, v2}] = score;
                    }
                    
                    if (score < bestScore) {
                        bestScore = score;
                        bestPair = {v1, v2};
                    }
                }
            }

            contractionSequence << bestPair.first + 1 << " " << bestPair.second + 1 << "\n";
            mergeVertices(bestPair.first, bestPair.second);

            auto stop = high_resolution_clock::now();
            auto duration = duration_cast<milliseconds>(stop - start);
            int seconds_part = duration.count() / 1000;
            int milliseconds_part = duration.count() % 1000;
            std::cout << "c (Merged ( " << bestPair.first << "," << bestPair.second << "), left " << vertices.size() << ", tww: " << getWidth() << ") Cycle in " << seconds_part << "." 
            << std::setfill('0') << std::setw(9) << milliseconds_part 
            << " seconds" << std::endl;
        }
        return contractionSequence;
    }

private:
    void updateWidth() {
        for (const auto& innerVector : adjListRed) {
            width = max(width, static_cast<int>(innerVector.size()));
        }
    }

    int getUpdatedWidth() {
        int updatedWidth = 0;
        for (const auto& innerVector : adjListRed) {
            updatedWidth = max(updatedWidth, static_cast<int>(innerVector.size()));
        }
        return updatedWidth;
    }
};

string getLastLine(ostringstream& oss) {
    const string& s = oss.str();
    auto lastNewlinePos = s.rfind('\n', s.length() - 2); // Start search before the very last character, which is likely a newline.
    if (lastNewlinePos != string::npos) {
        return s.substr(lastNewlinePos + 1); // Returns string after the last newline.
    } else {
        return s; // Return the entire string if there's no newline (i.e., it's a one-line string).
    }
}

int main() {
    Graph g;
    string line;
    int numVertices, numEdges;
    set<pair<int, int>> readEdges;
    bool constructComplement = false;

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

        if (tokens[0] == "p") {
            numVertices = stoi(tokens[2]);
            numEdges = stoi(tokens[3]);
            g.addVertices(numVertices);

            double density = (2.0 * numEdges) / (numVertices * (numVertices - 1));
            if (density > 0.5) {
                constructComplement = true;
            }
        } else if (constructComplement) {
            int u = stoi(tokens[0]);
            int v = stoi(tokens[1]);
            readEdges.insert({min(u-1, v-1), max(u-1, v-1)});
        } else {
            int u = stoi(tokens[0]);
            int v = stoi(tokens[1]);
            g.addEdge(u - 1, v - 1, "black");
        }
    }

    if (constructComplement) {
        for (int i = 0; i < numVertices; i++) {
            for (int j = i + 1; j < numVertices; j++) {
                if (readEdges.find({i, j}) == readEdges.end()) {
                    g.addEdge(i, j, "black");
                }
            }
        }
    }

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(stop - start);
    std::cout << "c Time taken too initialize the graph: " << duration.count() << " seconds" << std::endl;

    std::vector<Graph> components = g.findConnectedComponents();
    std::vector<int> remainingVertices;
    for (Graph& c : components) {
        std::vector<int> partition1;
        std::vector<int> partition2;
        ostringstream componentContraction;

        // ostringstream twins = c.findTwins();
        // cout << twins.str();
        // cout << c.applyOneDegreeRule().str();

        // if (c.isBipartiteBoost(partition1, partition2)) {
        //     componentContraction = c.findRedDegreeContractionPartitioned(partition1, partition2);
        //     cout << "c ITS BIPARTITE" << endl;
        // }
        // else componentContraction = c.findRedDegreeContraction();
        // cout << componentContraction.str();

        cout << c.findRedDegreeContraction().str();
        if (c.getVertices().size() == 1){
            int remainingVertex = *c.getVertices().begin() + 1;
            remainingVertices.push_back(remainingVertex);
        }
        else {
            // Extract here the last remaining vertex from the findRedDegreeContraction's output and push it back to remaining vertices
            string lastLine = getLastLine(componentContraction);
            stringstream lastPair(lastLine);
            int remainingVertex;
            lastPair >> remainingVertex;
            remainingVertices.push_back(remainingVertex);
        }
    }

    int primaryVertex = remainingVertices[0];
    for (size_t i = 1; i < remainingVertices.size(); ++i) {
        cout << primaryVertex << " " << remainingVertices[i] << endl;
    }

    // cout << g.findRedDegreeContraction().str();

    auto final_stop = high_resolution_clock::now();
    auto final_duration = duration_cast<seconds>(final_stop - start);
    std::cout << "c In total: " << final_duration.count() << " seconds" << std::endl;

    return 0;    
}