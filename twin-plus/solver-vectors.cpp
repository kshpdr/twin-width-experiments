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
#include <unordered_set>
#include <cmath>
#include "BoostGraph.hpp"

using namespace std;
using namespace std::chrono;

const int SCORE_RESET_THRESHOLD = 10000000;
const int TIME_LIMIT = 20;  
int cnt = 0;
bool connectedComponents = true;

struct PairHash {
    template <class T1, class T2>
    std::size_t operator() (const std::pair<T1, T2>& p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);
        return h1 ^ h2;
    }
};

struct ContractionStep {
    int iteration;
    pair<int, int> vertexPair;
    int score;
    int width;
};

struct ComponentSolution {
    ostringstream stringSequence;
    vector<ContractionStep> contractionSteps;
    int width;

    // Copy constructor
    ComponentSolution(const ComponentSolution& other) {
        width = other.width;

        // Copy stringSequence
        stringSequence.str(other.stringSequence.str());

        // Copy contractionSteps
        contractionSteps = other.contractionSteps;
    }

    // Default constructor
    ComponentSolution() = default;

    // Copy assignment operator
    ComponentSolution& operator=(const ComponentSolution& other) {
        if (this == &other) return *this; // Handle self assignment

        width = other.width;

        // Copy stringSequence
        stringSequence.str("");
        stringSequence << other.stringSequence.str();

        // Copy contractionSteps
        contractionSteps = other.contractionSteps;

        return *this;
    }
};


class Graph {
private:
    vector<int> vertices;
    vector<int> ids; // mapping id -> index, used for connected components
    vector<vector<int>> adjListBlack;  // For black edges
    vector<vector<int>> adjListRed;    // For red edges
    vector<vector<int>> redDegreeToVertices; // vertex id saved
    vector<vector<int>> degreeToVertices;
    int width = 0;
    std::mt19937 gen;
    bool useFixedSeed = true;

public:
    Graph() {
        if(useFixedSeed) {
            gen.seed(12345);
        } else {
            std::random_device rd;
            gen.seed(rd());
        }
    }

    Graph(const Graph &g) : gen(12345) {
        this->vertices = g.vertices;
        this->ids = g.ids;
        this->adjListBlack = g.adjListBlack;
        this->adjListRed = g.adjListRed;
        this->redDegreeToVertices = g.redDegreeToVertices;
        this->degreeToVertices = g.degreeToVertices;
        this->width = g.width;

        if(useFixedSeed) {
            gen.seed(12345);
        } else {
            std::random_device rd;
            gen.seed(rd());
        }
    }

    void updateDegrees(int v){
        updateVertexRedDegree(v, 0);
        updateVertexDegree(v, 0);
    }

    int getVertexId(int v){
        return ids[v];
    }
        
    // Adds n vertices to the graph numbered from 0 to n-1
    void addVertices(int n){
        vertices.resize(n);
        adjListBlack.resize(n);
        adjListRed.resize(n);

        std::iota(vertices.begin(), vertices.end(), 0); // populate vertices with 0...n-1
        redDegreeToVertices.insert(redDegreeToVertices.begin(), vertices);
        // degreeToVertices.insert(degreeToVertices.begin(), vertices);
    }

    void addVertices(int n, vector<int> ids){
        adjListBlack.resize(n);
        adjListRed.resize(n);
        vertices.resize(n);

        this->ids = ids;
        std::iota(vertices.begin(), vertices.end(), 0); // populate vertices with 0...n-1
        redDegreeToVertices.insert(redDegreeToVertices.begin(), vertices);
        // degreeToVertices.insert(degreeToVertices.begin(), vertices);
    }

    void setIds(vector<int> values) {
        ids = values;
    }

    void addEdgeBegin(int v1, int v2) {
        if (v1 < v2) {
            adjListBlack[v2].push_back(v1);
            adjListBlack[v1].push_back(v2);
        }
    }

    void updateBlackDegrees() {
        for (int i = 0; i < adjListBlack.size(); ++i) {
            if (degreeToVertices.size() <= adjListBlack[i].size()) degreeToVertices.resize(adjListBlack[i].size() + 1);
            degreeToVertices[adjListBlack[i].size()].push_back(i);
        }
    }

    void addEdge(int v1, int v2, const string& color = "black") {
        if (color == "black" && std::find(adjListBlack[v1].begin(), adjListBlack[v1].end(), v2) == adjListBlack[v1].end()) {
            updateVertexDegree(v1, 1);
            updateVertexDegree(v2, 1);
            adjListBlack[v1].push_back(v2);
            adjListBlack[v2].push_back(v1);
        } else if (color == "red" && std::find(adjListRed[v1].begin(), adjListRed[v1].end(), v2) == adjListRed[v1].end()) {
            updateVertexDegree(v1, 1);
            updateVertexDegree(v2, 1);
            updateVertexRedDegree(v1, 1);
            updateVertexRedDegree(v2, 1);
            adjListRed[v1].push_back(v2);
            adjListRed[v2].push_back(v1);
        }
    }

    void removeEdge(int v1, int v2) {
        if (std::find(adjListBlack[v1].begin(), adjListBlack[v1].end(), v2) != adjListBlack[v1].end()) {
            // order matters since updateVertexDegree uses adjListBlack's state
            updateVertexDegree(v1, -1);
            updateVertexDegree(v2, -1);
            adjListBlack[v1].erase(std::remove(adjListBlack[v1].begin(), adjListBlack[v1].end(), v2), adjListBlack[v1].end());
            adjListBlack[v2].erase(std::remove(adjListBlack[v2].begin(), adjListBlack[v2].end(), v1), adjListBlack[v2].end());
        } else if (std::find(adjListRed[v1].begin(), adjListRed[v1].end(), v2) != adjListRed[v1].end()) {
            updateVertexDegree(v1, -1);
            updateVertexDegree(v2, -1);
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
        degreeToVertices[adjListBlack[vertex].size() + adjListRed[vertex].size()].erase(std::remove(degreeToVertices[adjListRed[vertex].size() + adjListBlack[vertex].size()].begin(), degreeToVertices[adjListRed[vertex].size() + adjListBlack[vertex].size()].end(), vertex));
    }

    int getWidth() const {
        return width;
    }

    vector<int> getVertices() {
        return vertices;
    }

    vector<int> getIds() {
        return this->ids;
    }

    std::vector<Graph> findConnectedComponentsBoost() {
        BoostGraph boostGraph(vertices.size());
        for (int i = 0; i < adjListBlack.size(); ++i) {
            for (int j = 0; j < adjListBlack[i].size(); ++j) {
                boostGraph.addEdge(i, adjListBlack[i][j]);
            }
        }

        vector<set<pair<int, int>>> components;
        vector<vector<int>> vertices;
        std::tie(components, vertices) = boostGraph.getConnectedComponentAndVertices();  // Call getConnectedComponents and unpack the result
        std::vector<Graph> result;
        if (components.size() == 1) {
            result.push_back(*this);
            return result;
        }

        for (int i = 0; i < components.size(); ++i) {
            Graph g;
            g.addVertices(vertices[i].size(), vertices[i]);
            constructFromEdges(g, components[i]);
            g.updateBlackDegrees();
            result.push_back(g);
        }
        for (const auto& edges : components) {
        }
        return result;
    }

    static void constructFromEdges(Graph& g, const std::set<std::pair<int, int>>& edges) {
        for (const auto& edge : edges) {
            int u = edge.first, v = edge.second;
            vector<int> ids = g.getIds();
            int vId = std::distance(ids.begin(), std::find(ids.begin(), ids.end(), v)); 
            int uId = std::distance(ids.begin(), std::find(ids.begin(), ids.end(), u)); 
            g.addEdgeBegin(vId, uId);
        }
    }

    float getDegreeDeviation() {
        int totalVertices = vertices.size();
        int totalDegree = 0;
        for(int i = 0; i < degreeToVertices.size(); ++i) {
            totalDegree += i * degreeToVertices[i].size();
        }
        float meanDegree = static_cast<float>(totalDegree) / totalVertices;

        float sumAbsoluteDeviations = 0.0;
        for(int i = 0; i < degreeToVertices.size(); ++i) {
            sumAbsoluteDeviations += abs(i - meanDegree) * degreeToVertices[i].size();
        }
        
        float averageDegreeDeviation = sumAbsoluteDeviations / totalVertices;
        return averageDegreeDeviation;
    }

    std::vector<Graph> findConnectedComponents() {
        ankerl::unordered_dense::set<int> visited;
        std::vector<Graph> componentGraphs;

        int cnt = 0;
        for (int vertex : vertices) {
            if (visited.find(vertex) == visited.end()) {
                std::vector<int> component;
                dfs(vertex, visited, component);
                Graph subGraph;
                // sort(component.begin(), component.end()); // for debug, else comment
                subGraph.addVertices(component.size(), component);
                for (size_t i = 0; i < component.size(); ++i) {
                    // subGraph.updateDegrees(i);
                    for (int neighbor : adjListBlack[component[i]]) {
                        if (component[i] < neighbor) {
                            int vId = std::distance(component.begin(), std::find(component.begin(), component.end(), component[i])); 
                            int neighborId = std::distance(component.begin(), std::find(component.begin(), component.end(), neighbor)); 
                            subGraph.addEdgeBegin(vId, neighborId);
                        }
                    }
                }

                componentGraphs.push_back(subGraph);
            }
            // cout << cnt << endl;
            cnt++;
        }

        return componentGraphs;
    }

    void dfs(int v, ankerl::unordered_dense::set<int>& visited, std::vector<int>& component) {
        // std::cout << "Vertex " << v << endl;
        // std::cout << "Count: " << cnt << endl;
        cnt++; 
        visited.insert(v);
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

    void updateVertexDegree(int vertex, int diff) {
        int oldDegree = adjListRed[vertex].size() + adjListBlack[vertex].size();
        int newDegree = oldDegree + diff;
        degreeToVertices[oldDegree].erase(std::remove(degreeToVertices[oldDegree].begin(), degreeToVertices[oldDegree].end(), vertex), degreeToVertices[oldDegree].end());
        
        if (degreeToVertices.size() <= newDegree) degreeToVertices.resize(newDegree + 1);
        degreeToVertices[oldDegree + diff].push_back(vertex);
    }

    int getWorstVertex() {
        for (auto rit = redDegreeToVertices.rbegin(); rit != redDegreeToVertices.rend(); ++rit) {
            const auto& degreeVector = *rit;
            if (degreeVector.empty()) continue;
            for (int vertex : degreeVector) {
                return vertex;
            }
        }
        return -1;
    }

    bool contrainsWorstVertex(int v1, int v2, int worstVertex) {
        vector<int> black_neighbors = adjListBlack[worstVertex];
        vector<int> red_neighbors = adjListRed[worstVertex];
        if (std::find(black_neighbors.begin(), black_neighbors.end(), v1) != black_neighbors.end()) return true;
        if (std::find(red_neighbors.begin(), red_neighbors.end(), v1) != red_neighbors.end()) return true;
        if (std::find(black_neighbors.begin(), black_neighbors.end(), v2) != black_neighbors.end()) return true;
        if (std::find(red_neighbors.begin(), red_neighbors.end(), v2) != red_neighbors.end()) return true;
        return false;
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

    std::vector<int> getTopNVerticesWithLowestDegree(int n) {
        std::vector<int> topVertices;
        
        for (const auto& degreeVector : degreeToVertices) {
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
        sort(mergedSourceNeighbors.begin(), mergedSourceNeighbors.end());
        sort(mergedTwinNeighbors.begin(), mergedTwinNeighbors.end());


        // Find edges of twin that are not adjacent to source
        std::vector<int> newRedEdges;
        std::set_difference(
            mergedTwinNeighbors.begin(), mergedTwinNeighbors.end(),
            mergedSourceNeighbors.begin(), mergedSourceNeighbors.end(),
            std::inserter(newRedEdges, newRedEdges.end())
        );

        // Add these edges as red edges for source
        for (int v : newRedEdges) {
            addEdge(source, v, "red");
            // if (std::find(adjListRed[source].begin(), adjListRed[source].end(), v) == adjListRed[source].end()) {
            //     addEdge(source, v, "red");
            // }
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

        sort(source_neighbors.begin(), source_neighbors.end());
        sort(twin_neighbors.begin(), twin_neighbors.end());

        vector<int> toBecomeRed;
        std::set_difference(
            source_neighbors.begin(), source_neighbors.end(),
            twin_neighbors.begin(), twin_neighbors.end(),
            std::inserter(toBecomeRed, toBecomeRed.begin())
        );

        for (int v : toBecomeRed) {
            removeEdge(source, v);
            addEdge(source, v, "red");
            // don't understand why is this possible since were considering only black edges
            // if (std::find(adjListRed[source].begin(), adjListRed[source].end(), v) == adjListRed[source].end()) {
            //     addEdge(source, v, "red");
            // }
        }
    }

    int getRealScore(int source, int twin) {
        Graph graphCopy(*this);   // Assuming you've implemented the copy constructor for Graph class

        // Merge vertices on the copied graph
        graphCopy.mergeVertices(source, twin);

        // Return the updated width of the copied graph
        return graphCopy.getWidth();
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

        sort(neighbors_v1.begin(), neighbors_v1.end());
        sort(neighbors_v2.begin(), neighbors_v2.end());

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

    int getScoreBlack(int v1, int v2) {
        vector<int> neighbors_v1 = adjListBlack[v1];
        vector<int> neighbors_v2 = adjListBlack[v2];
        sort(neighbors_v1.begin(), neighbors_v1.end());
        sort(neighbors_v2.begin(), neighbors_v2.end());

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

    float getGScore(int v1, int v2) {
        vector<int> neighbors_v1 = adjListBlack[v1];
        if (!adjListRed[v1].empty()) {
            neighbors_v1.insert(neighbors_v1.end(), adjListRed[v1].begin(), adjListRed[v1].end());
        }

        vector<int> neighbors_v2 = adjListBlack[v2];
        if (!adjListRed[v2].empty()) {
            neighbors_v2.insert(neighbors_v2.end(), adjListRed[v2].begin(), adjListRed[v2].end());
        }
        sort(neighbors_v1.begin(), neighbors_v1.end());
        sort(neighbors_v2.begin(), neighbors_v2.end());
        
        vector<int> common_neighbors;
        set_intersection(neighbors_v1.begin(), neighbors_v1.end(), neighbors_v2.begin(), neighbors_v2.end(), back_inserter(common_neighbors));
        float common_neighbors_count = common_neighbors.size();

        // Degree Difference
        float degree_diff = abs((float)neighbors_v1.size() - (float)neighbors_v2.size());

        // Red Edges Count
        float red_edges_count = adjListRed[v1].size() + adjListRed[v2].size();

        // This is a simplistic formula and may need to be refined based on your specific needs and understanding of the graph structure.
        float score = -common_neighbors_count + degree_diff + red_edges_count;

        return score;
    }

    float getGScoreBlack(int v1, int v2) {
        vector<int> neighbors_v1 = adjListBlack[v1];
        // if (!adjListRed[v1].empty()) {
        //     neighbors_v1.insert(neighbors_v1.end(), adjListRed[v1].begin(), adjListRed[v1].end());
        // }

        vector<int> neighbors_v2 = adjListBlack[v2];
        // if (!adjListRed[v2].empty()) {
        //     neighbors_v2.insert(neighbors_v2.end(), adjListRed[v2].begin(), adjListRed[v2].end());
        // }
        sort(neighbors_v1.begin(), neighbors_v1.end());
        sort(neighbors_v2.begin(), neighbors_v2.end());
        
        vector<int> common_neighbors;
        set_intersection(neighbors_v1.begin(), neighbors_v1.end(), neighbors_v2.begin(), neighbors_v2.end(), back_inserter(common_neighbors));
        float common_neighbors_count = common_neighbors.size();

        // Degree Difference
        float degree_diff = abs((float)neighbors_v1.size() - (float)neighbors_v2.size());

        // Red Edges Count
        float red_edges_count = adjListRed[v1].size() + adjListRed[v2].size();

        // This is a simplistic formula and may need to be refined based on your specific needs and understanding of the graph structure.
        float score = -common_neighbors_count + degree_diff + red_edges_count;

        return score;
    }

    float getNScore(int v1, int v2) {
        vector<int> neighbors_v1 = adjListBlack[v1];
        vector<int> neighbors_v2 = adjListBlack[v2];

        sort(neighbors_v1.begin(), neighbors_v1.end());
        sort(neighbors_v2.begin(), neighbors_v2.end());

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

        std::vector<int> allNeighbors;
        if (!adjListRed[v1].empty()) {
            neighbors_v1.insert(neighbors_v1.end(), adjListRed[v1].begin(), adjListRed[v1].end());
        }
        if (!adjListRed[v2].empty()) {
            neighbors_v2.insert(neighbors_v2.end(), adjListRed[v2].begin(), adjListRed[v2].end());
        }
        std::set_union(neighbors_v1.begin(), neighbors_v1.end(), neighbors_v2.begin(), neighbors_v2.end(), std::back_inserter(allNeighbors));

        return symmetric_difference.size() + allNeighbors.size();
    }

    float getNeighborsScore(int v1, int v2) {
        int black_score = getScoreBlack(v1, v2);
        vector<int> neighbors_v1 = adjListBlack[v1];
        vector<int> neighbors_v2 = adjListBlack[v2];
        if (!adjListRed[v1].empty()) {
            neighbors_v1.insert(neighbors_v1.end(), adjListRed[v1].begin(), adjListRed[v1].end());
        }
        if (!adjListRed[v2].empty()) {
            neighbors_v2.insert(neighbors_v2.end(), adjListRed[v2].begin(), adjListRed[v2].end());
        }

        sort(neighbors_v1.begin(), neighbors_v1.end());
        sort(neighbors_v2.begin(), neighbors_v2.end());

        std::vector<int> allNeighbors;
        std::set_union(neighbors_v1.begin(), neighbors_v1.end(), neighbors_v2.begin(), neighbors_v2.end(), std::back_inserter(allNeighbors));

        return black_score +  black_score / allNeighbors.size();
    }

    int getScore1(int v1, int v2) {
        vector<int> neighbors_v1 = adjListBlack[v1];
        vector<int> neighbors_v2 = adjListBlack[v2];
        if (!adjListRed[v2].empty()) {
            neighbors_v2.insert(neighbors_v2.end(), adjListRed[v2].begin(), adjListRed[v2].end());
        }
        sort(neighbors_v1.begin(), neighbors_v1.end());
        sort(neighbors_v2.begin(), neighbors_v2.end());

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

    float getG2Score(int v1, int v2) {
        vector<int> neighbors_v1 = adjListBlack[v1];
        if (!adjListRed[v1].empty()) {
            neighbors_v1.insert(neighbors_v1.end(), adjListRed[v1].begin(), adjListRed[v1].end());
        }

        vector<int> neighbors_v2 = adjListBlack[v2];
        if (!adjListRed[v2].empty()) {
            neighbors_v2.insert(neighbors_v2.end(), adjListRed[v2].begin(), adjListRed[v2].end());
        }
        sort(neighbors_v1.begin(), neighbors_v1.end());
        sort(neighbors_v2.begin(), neighbors_v2.end());

        std::vector<int> allNeighbors;
        std::set_union(neighbors_v1.begin(), neighbors_v1.end(), neighbors_v2.begin(), neighbors_v2.end(), std::back_inserter(allNeighbors));
        float union_size = allNeighbors.size();

        // Red-Black Edge Ratio
        float red_edges_count = adjListRed[v1].size() + adjListRed[v2].size();
        float black_edges_count = adjListBlack[v1].size() + adjListBlack[v2].size();
        float red_black_ratio = (red_edges_count + 1) / (black_edges_count + 1);  // +1 to avoid division by zero

        // Potential New Red Edges
        // Assume that every neighbor of v1 and v2 will be connected by a red edge after merging
        float potential_new_red_edges = union_size - (red_edges_count + black_edges_count);

        // Score Formula: a linear combination of the above metrics
        float score = union_size - red_black_ratio + potential_new_red_edges;

        return score;
    }

    int getRandomDistance() {
        std::uniform_int_distribution<> distrib(1, 2);
        return distrib(gen);
    }

    int getRandomNeighbor(int vertex) {
        vector<int> allNeighbors;
        allNeighbors.insert(allNeighbors.end(), adjListBlack[vertex].begin(), adjListBlack[vertex].end());
        allNeighbors.insert(allNeighbors.end(), adjListRed[vertex].begin(), adjListRed[vertex].end());

        std::shuffle(allNeighbors.begin(), allNeighbors.end(), gen);
        return allNeighbors[0];
    }

    set<int> getRandomWalkVertices(int vertex, int numberVertices) {
        set<int> randomWalkVertices;
        for (int i = 0; i < numberVertices; ++i) {
            int distance = getRandomDistance();            
            int randomVertex = getRandomNeighbor(vertex);
            if (distance == 2 && adjListBlack[randomVertex].size() + adjListRed[randomVertex].size() != 0) randomVertex = getRandomNeighbor(randomVertex);
            randomWalkVertices.insert(randomVertex);
        }
        randomWalkVertices.erase(vertex);
        return randomWalkVertices;
    }


    ostringstream findTwins(bool trueTwins) {
        ostringstream contractionSequence;        
        
        vector<vector<int>> partitions; 
        vector<vector<int>> updated_partitions; 
        partitions.push_back(vertices);

        for (int v : vertices) {
            vector<int> neighbors = adjListBlack[v];
            if (trueTwins) neighbors.push_back(v);
            sort(neighbors.begin(), neighbors.end());
            for (vector partition : partitions) {
                vector<int> difference;
                std::set_difference(partition.begin(), partition.end(), neighbors.begin(), neighbors.end(), 
                        std::inserter(difference, difference.end()));

                vector<int> intersection;
                std::set_intersection(partition.begin(), partition.end(), neighbors.begin(), neighbors.end(), 
                        std::inserter(intersection, intersection.end()));

                if (!difference.empty()) updated_partitions.push_back(difference);
                if (!intersection.empty()) updated_partitions.push_back(intersection);
            }
            partitions = updated_partitions;
            updated_partitions.clear();
        }
        for (const vector<int>& partition : partitions) {
            if (partition.size() > 1) {
                auto it = partition.begin();
                int first = *it;
                ++it;
                while(it != partition.end()) {
                    int next = *it;
                    contractionSequence << first + 1 << " " << next + 1 << "\n";
                    mergeVertices(first, next); 
                    cout << "c Found twins";
                    ++it;
                }
            }
        }
        return contractionSequence;
    }


    ostringstream findRedDegreeContractionRandomWalk(){ 
        ostringstream contractionSequence;
        vector<vector<pair<int, int>>> scores(vertices.size());
        auto heuristic_start_time = high_resolution_clock::now();
        
        int iterationCounter = 0;
        while (vertices.size() > 1) {
            auto start = high_resolution_clock::now();

            vector<int> lowestDegreeVertices = getTopNVerticesWithLowestRedDegree(2);
            // vector<int> lowestDegreeVertices = getTopNVerticesWithLowestDegree(static_cast<int>(ceil(log(vertices.size())))+1);

            int bestScore = INT_MAX;
            pair<int, int> bestPair;

            for (int i = 0; i < lowestDegreeVertices.size(); i++) {
                int v1 = lowestDegreeVertices[i];
                set<int> randomWalkVertices = getRandomWalkVertices(v1, 105);  
              
                for (int v2 : randomWalkVertices) {
                    if (v2 > v1) {
                        std::swap(v1, v2);
                    }

                    auto it = find_if(scores[v1].begin(), scores[v1].end(),
                                    [v2](const pair<int, int>& p){ return p.first == v2; });
                    int score;
                    if (it != scores[v1].end()) {
                        score = it->second;
                    }
                    else {
                        score = getScore(v1, v2);
                        scores[v1].push_back({v2, score});
                    }
                    
                    if (score < bestScore) {
                        bestScore = score;
                        bestPair = {v1, v2};
                    }
                }
            }

            contractionSequence << getVertexId(bestPair.first) + 1 << " " << getVertexId(bestPair.second) + 1 << "\n";

            // if (!areInTwoNeighborhood(bestPair.first, bestPair.second)) cout << "c Not neighbors, score: " << bestScore << endl;
            // else cout << "c Neighbors, score: " << bestScore << endl;

            mergeVertices(bestPair.first, bestPair.second);

            auto stop = high_resolution_clock::now();
            auto duration = duration_cast<milliseconds>(stop - start);
            int seconds_part = duration.count() / 1000;
            int milliseconds_part = duration.count() % 1000;
            std::cout << "c (Left " << vertices.size() << ", tww: " << getWidth() << ") Cycle in " << seconds_part << "." 
            << std::setfill('0') << std::setw(9) << milliseconds_part 
            << " seconds" << std::endl;

            for(int i = 0; i < scores.size(); ++i) {
                scores[i].clear();
            }
        }
        return contractionSequence;
    }

    ComponentSolution findRedDegreeContractionRandomWalkExhaustively(const ComponentSolution& prevSolution = ComponentSolution()){ 
        // ostringstream contractionSequence;
        ComponentSolution solution;
        vector<vector<pair<int, int>>> scores(vertices.size());
        auto heuristic_start_time = high_resolution_clock::now();
        
        int iterationCounter = 0;
        while (vertices.size() > 1) {
            auto start = high_resolution_clock::now();

            vector<int> lowestDegreeVertices = getTopNVerticesWithLowestRedDegree(20);
            // vector<int> lowestDegreeVertices = getTopNVerticesWithLowestDegree(static_cast<int>(ceil(log(vertices.size())))+1);

            int bestScore = INT_MAX;
            pair<int, int> bestPair;

            for (int i = 0; i < lowestDegreeVertices.size(); i++) {
                int v1 = lowestDegreeVertices[i];
                set<int> randomWalkVertices = getRandomWalkVertices(v1, 10);  
              
                for (int v2 : randomWalkVertices) {
                    if (v2 > v1) {
                        std::swap(v1, v2);
                    }

                    auto it = find_if(scores[v1].begin(), scores[v1].end(),
                                    [v2](const pair<int, int>& p){ return p.first == v2; });
                    int score;
                    if (it != scores[v1].end()) {
                        score = it->second;
                    }
                    else {
                        score = getScore(v1, v2);
                        scores[v1].push_back({v2, score});
                    }
                    
                    if (score < bestScore) {
                        bestScore = score;
                        bestPair = {v1, v2};
                    }
                }
            }

            solution.stringSequence << getVertexId(bestPair.first) + 1 << " " << getVertexId(bestPair.second) + 1 << "\n";

            mergeVertices(bestPair.first, bestPair.second);
            solution.width = getWidth();

            ContractionStep step;
            step.iteration = iterationCounter;
            step.vertexPair = bestPair;
            step.score = bestScore;
            step.width = getWidth();
            solution.contractionSteps.push_back(step);

            if (prevSolution.width != 0 && step.width > prevSolution.width) {
                cout << "c Discarded" << endl;
                return solution;
            }

            auto stop = high_resolution_clock::now();
            auto duration = duration_cast<milliseconds>(stop - start);
            int seconds_part = duration.count() / 1000;
            int milliseconds_part = duration.count() % 1000;
            std::cout << "c (Left " << vertices.size() << ", tww: " << getWidth() << ") Cycle in " << seconds_part << "." 
            << std::setfill('0') << std::setw(9) << milliseconds_part 
            << " seconds" << std::endl;

            // cout << getWidth() << endl;
            iterationCounter++;

            for(int i = 0; i < scores.size(); ++i) {
                scores[i].clear();
            }
        }
        // cout << "stop" << endl;
        return solution;
    }

    ComponentSolution findDegreeContractionExhaustively(const ComponentSolution& prevSolution = ComponentSolution()){ 
        ComponentSolution solution;
        ostringstream contractionSequence;
        vector<vector<pair<int, int>>> scores(vertices.size());
        auto heuristic_start_time = high_resolution_clock::now();
        
        int iterationCounter = 0;
        while (vertices.size() > 1) {
            auto start = high_resolution_clock::now();

            vector<int> lowestDegreeVertices = getTopNVerticesWithLowestDegree(20);
            // vector<int> lowestDegreeVertices;
            // if (vertices.size() < 50) lowestDegreeVertices = getTopNVerticesWithLowestDegree(20);
            // else lowestDegreeVertices = getTopNVerticesWithLowestDegree(static_cast<int>(3 * ceil(log(vertices.size()))));            
            
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

                    auto it = find_if(scores[v1].begin(), scores[v1].end(),
                                    [v2](const pair<int, int>& p){ return p.first == v2; });
                    int score;
                    if (it != scores[v1].end()) {
                        score = it->second;
                    }
                    else {
                        score = getScore(v1, v2);
                        scores[v1].push_back({v2, score});
                    }
                    
                    if (score < bestScore) {
                        bestScore = score;
                        bestPair = {v1, v2};
                    }
                }
            }

            solution.stringSequence << getVertexId(bestPair.first) + 1 << " " << getVertexId(bestPair.second) + 1 << "\n";

            mergeVertices(bestPair.first, bestPair.second);
            solution.width = getWidth();

            ContractionStep step;
            step.iteration = iterationCounter;
            step.vertexPair = bestPair;
            step.score = bestScore;
            step.width = getWidth();
            solution.contractionSteps.push_back(step);

            if (prevSolution.width != 0 && step.width > prevSolution.width) {
                cout << "c Discarded" << endl;
                return solution;
            }

            auto stop = high_resolution_clock::now();
            auto duration = duration_cast<milliseconds>(stop - start);
            int seconds_part = duration.count() / 1000;
            int milliseconds_part = duration.count() % 1000;
            std::cout << "c (Merged ( " << getVertexId(bestPair.first) << "," << getVertexId(bestPair.second) << "), left " << vertices.size() << ", tww: " << getWidth() << ") Cycle in " << seconds_part << "." 
            << std::setfill('0') << std::setw(9) << milliseconds_part 
            << " seconds" << std::endl;

            for(int i = 0; i < scores.size(); ++i) {
                scores[i].clear();
            }
            iterationCounter++;
        }
        return solution;
    }

    ostringstream findDegreeContraction(){ 
        ostringstream contractionSequence;
        vector<vector<pair<int, int>>> scores(vertices.size());
        auto heuristic_start_time = high_resolution_clock::now();
        
        int iterationCounter = 0;
        while (vertices.size() > 1) {
            auto start = high_resolution_clock::now();

            // vector<int> lowestDegreeVertices = getTopNVerticesWithLowestDegree(20);
            vector<int> lowestDegreeVertices = getTopNVerticesWithLowestDegree(50);
            // vector<int> lowestDegreeVertices;
            // if (vertices.size() < 50) lowestDegreeVertices = getTopNVerticesWithLowestDegree(20);
            // else lowestDegreeVertices = getTopNVerticesWithLowestDegree(static_cast<int>(3 * ceil(log(vertices.size()))));            
            
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

                    auto it = find_if(scores[v1].begin(), scores[v1].end(),
                                    [v2](const pair<int, int>& p){ return p.first == v2; });
                    int score;
                    if (it != scores[v1].end()) {
                        score = it->second;
                    }
                    else {
                        score = getScore(v1, v2);
                        scores[v1].push_back({v2, score});
                    }
                    
                    if (score < bestScore) {
                        bestScore = score;
                        bestPair = {v1, v2};
                    }
                }
            }

            contractionSequence << getVertexId(bestPair.first) + 1 << " " << getVertexId(bestPair.second) + 1 << "\n";
            mergeVertices(bestPair.first, bestPair.second);


            // if (++iterationCounter >= SCORE_RESET_THRESHOLD) {
            //     for(int i = 0; i < scores.size(); ++i) {
            //         scores[i].clear();
            //     }
            //     iterationCounter = 0;
            // }

            auto stop = high_resolution_clock::now();
            auto duration = duration_cast<milliseconds>(stop - start);
            int seconds_part = duration.count() / 1000;
            int milliseconds_part = duration.count() % 1000;
            std::cout << "c (Merged ( " << getVertexId(bestPair.first) << "," << getVertexId(bestPair.second) << "), left " << vertices.size() << ", tww: " << getWidth() << ") Cycle in " << seconds_part << "." 
            << std::setfill('0') << std::setw(9) << milliseconds_part 
            << " seconds" << std::endl;

            for(int i = 0; i < scores.size(); ++i) {
                scores[i].clear();
            }
        }
        return contractionSequence;
    }


    ostringstream findDegreeContractionRandomWalk(){ 
        ostringstream contractionSequence;
        vector<vector<pair<int, int>>> scores(vertices.size());
        auto heuristic_start_time = high_resolution_clock::now();
        
        int iterationCounter = 0;
        while (vertices.size() > 1) {
            auto start = high_resolution_clock::now();

            vector<int> lowestDegreeVertices = getTopNVerticesWithLowestDegree(20);

            int bestScore = INT_MAX;
            pair<int, int> bestPair;

            for (int i = 0; i < lowestDegreeVertices.size(); i++) {
                int v1 = lowestDegreeVertices[i];
                set<int> randomWalkVertices = getRandomWalkVertices(v1, 10);  
              
                for (int v2 : randomWalkVertices) {
                    if (v2 > v1) {
                        std::swap(v1, v2);
                    }

                    auto it = find_if(scores[v1].begin(), scores[v1].end(),
                                    [v2](const pair<int, int>& p){ return p.first == v2; });
                    int score;
                    if (it != scores[v1].end()) {
                        score = it->second;
                    }
                    else {
                        score = getScore(v1, v2);
                        scores[v1].push_back({v2, score});
                    }
                    
                    if (score < bestScore) {
                        bestScore = score;
                        bestPair = {v1, v2};
                    }
                }
            }

            contractionSequence << getVertexId(bestPair.first) + 1 << " " << getVertexId(bestPair.second) + 1 << "\n";

            // if (!areInTwoNeighborhood(bestPair.first, bestPair.second)) cout << "c Not neighbors, score: " << bestScore << endl;
            // else cout << "c Neighbors, score: " << bestScore << endl;

            mergeVertices(bestPair.first, bestPair.second);

            auto stop = high_resolution_clock::now();
            auto duration = duration_cast<milliseconds>(stop - start);
            int seconds_part = duration.count() / 1000;
            int milliseconds_part = duration.count() % 1000;
            std::cout << "c (Left " << vertices.size() << ", tww: " << getWidth() << ") Cycle in " << seconds_part << "." 
            << std::setfill('0') << std::setw(9) << milliseconds_part 
            << " seconds" << std::endl;
            // for(int i = 0; i < scores.size(); ++i) {
            //     scores[i].clear();
            // }
        }
        return contractionSequence;
    }

    ostringstream findBestVertexContraction(){ 
        ostringstream contractionSequence;
        ankerl::unordered_dense::map<pair<int, int>, int, PairHash> scores;
        auto heuristic_start_time = high_resolution_clock::now();
        
        int iterationCounter = 0;
        while (vertices.size() > 1) {
            auto start = high_resolution_clock::now();

            vector<int> lowestDegreeVertices = getTopNVerticesWithLowestRedDegree(1);
            int v = lowestDegreeVertices[0];

            int bestScore = INT_MAX;
            pair<int, int> bestPair;

            for (int n1 : adjListBlack[v]) {
                int score;
                for (int n2 : adjListBlack[v]) {
                    if (v == n2) continue;
                    score = getScore(v, n2);
                    if (score < bestScore) {
                        bestScore = score;
                        bestPair = {v, n2};
                    }
                }
                for (int n2 : adjListRed[v]) {
                    if (v == n2) continue;
                    score = getScore(v, n2);
                    if (score < bestScore) {
                        bestScore = score;
                        bestPair = {v, n2};
                    }
                }
            }

            for (int n1 : adjListRed[v]) {
                int score;
                for (int n2 : adjListBlack[v]) {
                    if (v == n2) continue;
                    score = getScore(v, n2);
                    if (score < bestScore) {
                        bestScore = score;
                        bestPair = {v, n2};
                    }
                }
                for (int n2 : adjListRed[v]) {
                    if (v == n2) continue;
                    score = getScore(v, n2);
                    if (score < bestScore) {
                        bestScore = score;
                        bestPair = {v, n2};
                    }
                }
            }

            contractionSequence << getVertexId(bestPair.first) + 1 << " " << getVertexId(bestPair.second) + 1 << "\n";
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

    ostringstream findRedDegreeContraction(){ 
        ostringstream contractionSequence;
        ankerl::unordered_dense::map<pair<int, int>, int, PairHash> scores;
        auto heuristic_start_time = high_resolution_clock::now();
        
        int iterationCounter = 0;
        while (vertices.size() > 1) {
            auto start = high_resolution_clock::now();

            vector<int> lowestDegreeVertices;
            if (iterationCounter == 0) {
                    std::random_device rd;
                    std::mt19937 g(rd());
                    vector<int> v_copy(vertices);
                    std::shuffle(v_copy.begin(), v_copy.end(), g);
                    int num_elements = std::min((int)v_copy.size(), 20);  // Take 20 elements or the entire vector if it has fewer than 20 elements
                    lowestDegreeVertices.assign(v_copy.begin(), v_copy.begin() + num_elements);
            }
            else lowestDegreeVertices = getTopNVerticesWithLowestRedDegree(20);
            // if (vertices.size() < 50) lowestDegreeVertices = getTopNVerticesWithLowestRedDegree(20);
            // else lowestDegreeVertices = getTopNVerticesWithLowestRedDegree(static_cast<int>(ceil(vertices.size() / 10.0)));            
            
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

            contractionSequence << getVertexId(bestPair.first) + 1 << " " << getVertexId(bestPair.second) + 1 << "\n";
            mergeVertices(bestPair.first, bestPair.second);

            auto stop = high_resolution_clock::now();
            auto duration = duration_cast<milliseconds>(stop - start);
            int seconds_part = duration.count() / 1000;
            int milliseconds_part = duration.count() % 1000;
            std::cout << "c (Merged ( " << bestPair.first << "," << bestPair.second << "), left " << vertices.size() << ", tww: " << getWidth() << ") Cycle in " << seconds_part << "." 
            << std::setfill('0') << std::setw(9) << milliseconds_part 
            << " seconds" << std::endl;
            iterationCounter++;
            scores.clear();
        }
        return contractionSequence;
    }

    ostringstream findRedDegreeContractionWorstVertex(){ 
        ostringstream contractionSequence;
        ankerl::unordered_dense::map<pair<int, int>, int, PairHash> scores;
        auto heuristic_start_time = high_resolution_clock::now();
        
        int iterationCounter = 0;
        while (vertices.size() > 1) {
            auto start = high_resolution_clock::now();

            vector<int> lowestDegreeVertices = getTopNVerticesWithLowestRedDegree(20);
            int worstVertex = getWorstVertex();
            
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
                        if (contrainsWorstVertex(v1,v2,worstVertex)) score += 5;
                        scores[{v1, v2}] = score;
                    }
                    
                    if (score < bestScore) {
                        bestScore = score;
                        bestPair = {v1, v2};
                    }
                }
            }

            contractionSequence << getVertexId(bestPair.first) + 1 << " " << getVertexId(bestPair.second) + 1 << "\n";
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

    ostringstream findRedDegreeContractionWhileLoop(){ 
        ostringstream contractionSequence;
        unordered_map<pair<int, int>, int, PairHash> scores;
        auto heuristic_start_time = high_resolution_clock::now();
        
        unordered_set<int> mergedVertices;  // Moved outside the while loop

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
                    if (v2 > v1) {
                        swap(v1, v2);
                    }

                    int score = getScore(v1, v2);
                    scores[{v1, v2}] = score;

                    if (score < bestScore) {
                        bestScore = score;
                        bestPair = {v1, v2};
                    }
                }
            }

            contractionSequence << getVertexId(bestPair.first) + 1 << " " << getVertexId(bestPair.second) + 1 << "\n";
            if (getVertexId(bestPair.second) == 85) {
                cout << "c checl";
            }
            mergeVertices(bestPair.first, bestPair.second);
            mergedVertices.insert(bestPair.second);

            for (const auto& [pair, score] : scores) {
                auto [v1, v2] = pair;

                // Check if either vertex has already been merged
                if (mergedVertices.count(v1) || mergedVertices.count(v2)) {
                    continue;
                }

                // // Check for independence
                if (!checkIndependence(bestPair, pair)) {
                    continue;
                }

                cout << "c wow (Merged (" << getVertexId(v1) << "," << getVertexId(v2) << ")" << endl;
                contractionSequence << getVertexId(v1) + 1 << " " << getVertexId(v2) + 1 << "\n";
                mergeVertices(v1, v2);
                mergedVertices.insert(v2);
            }

            scores.clear();  // Clear scores for the next iteration

            auto stop = high_resolution_clock::now();
            auto duration = duration_cast<milliseconds>(stop - start);
            int seconds_part = duration.count() / 1000;
            int milliseconds_part = duration.count() % 1000;
            cout << "c (Merged ( " << getVertexId(bestPair.first) << "," << getVertexId(bestPair.second) << "), left " << vertices.size() << ", tww: " << getWidth() << ") Cycle in " << seconds_part << "." 
                 << setfill('0') << setw(9) << milliseconds_part 
                 << " seconds" << endl;
        }

        return contractionSequence;
    }

    bool checkIndependence(std::pair<int, int> pair1, std::pair<int, int> pair2) {
        // Get the second neighborhoods of the vertices in pair1 and pair2
        std::vector<int> N2_v1 = getNeighbors(pair1.first);
        std::vector<int> N2_v2 = getNeighbors(pair2.first);
        std::vector<int> N2_u2 = getNeighbors(pair2.second);

        std::sort(N2_v1.begin(), N2_v1.end());
        std::sort(N2_v2.begin(), N2_v2.end());
        std::sort(N2_u2.begin(), N2_u2.end());

        // Combine the neighborhoods of each vertex pair
        std::vector<int> union2;
        std::set_union(N2_v2.begin(), N2_v2.end(), N2_u2.begin(), N2_u2.end(), std::back_inserter(union2));

        std::sort(union2.begin(), union2.end());
        // Check if the unions have any common elements
        std::vector<int> intersection;
        std::set_intersection(N2_v1.begin(), N2_v1.end(), union2.begin(), union2.end(), std::back_inserter(intersection));

        return intersection.empty();  // Return true if there are no common elements, false otherwise
    }

    std::vector<int> getNeighbors(int vertex) {
        vector<int> neighbors = adjListBlack[vertex];
        neighbors.insert(neighbors.begin(), adjListRed[vertex].begin(), adjListRed[vertex].end());
        return neighbors;
    }

    std::vector<int> getSecondNeighborhood(int vertexIndex) {
        std::vector<bool> visited(vertices.size(), false);  // To keep track of visited vertices
        std::queue<std::pair<int, int>> bfsQueue;  // Pair of vertex index and depth
        
        bfsQueue.push({vertexIndex, 0});
        visited[vertexIndex] = true;
        
        std::vector<int> neighborhood;
        
        while (!bfsQueue.empty()) {
            auto [currentVertex, depth] = bfsQueue.front();
            bfsQueue.pop();
            
            // Explore the neighbors of the current vertex if the depth is less than 2
            if (depth < 2) {
                for (int neighbor : adjListBlack[currentVertex]) {  // Assuming black edges represent normal adjacency
                    if (!visited[neighbor]) {
                        visited[neighbor] = true;
                        neighborhood.push_back(neighbor);
                        bfsQueue.push({neighbor, depth + 1});
                    }
                }
                
                for (int neighbor : adjListRed[currentVertex]) {  // Assuming red edges represent normal adjacency
                    if (!visited[neighbor]) {
                        visited[neighbor] = true;
                        neighborhood.push_back(neighbor);
                        bfsQueue.push({neighbor, depth + 1});
                    }
                }
            }
        }

        // Sort the neighborhood vector to enable efficient set operations later
        std::sort(neighborhood.begin(), neighborhood.end());

        return neighborhood;
    }

private:
    // void updateWidth() {
    //     for (const auto& innerVector : adjListRed) {
    //         width = max(width, static_cast<int>(innerVector.size()));
    //     }
    // }

    void updateWidth() {
        for (int i = redDegreeToVertices.size() - 1; i >= 0; i--) {
            if (!redDegreeToVertices[i].empty()) {
                width = max(width ,i);
                break;
            }
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
    double density;
    int maxTww = 0;
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

            density = (2.0 * numEdges) / (numVertices * (numVertices - 1));
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
            g.addEdgeBegin(u - 1, v - 1);
        }
    }

    if (constructComplement) {
        for (int i = 0; i < numVertices; i++) {
            for (int j = i + 1; j < numVertices; j++) {
                if (readEdges.find({i, j}) == readEdges.end()) {
                    g.addEdgeBegin(i, j);
                }
            }
        }
    }
    g.updateBlackDegrees();
    g.setIds(g.getVertices());

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(stop - start);
    std::cout << "c Time taken too initialize the graph: " << duration.count() << " seconds" << std::endl;

    // for (int i = 0; i <= 3598623; ++i){
    //     cout << i << endl;
    //     g.getTopNVerticesWithLowestDegree(20);
    // }

    // g.setIds(g.getVertices());
    // cout << g.findDegreeContractionRandomWalk().str();

    start = high_resolution_clock::now(); 
    
    vector<Graph> components;
    if (connectedComponents) {
        components = g.findConnectedComponentsBoost();
    }
    else {
        components.push_back(g);
    }
    
    stop = high_resolution_clock::now();
    duration = duration_cast<seconds>(stop - start);
    cout << "c Time taken for connected components: " << duration.count() << " seconds" << std::endl;

    std::vector<int> remainingVertices;
    for (Graph& c : components) {
        ostringstream componentContraction;
        // vector<int> partition1;
        // vector<int> partition2;

        // ostringstream twins = c.findTwins(false);
        // cout << twins.str();
        // cout << c.applyOneDegreeRule().str();

        // if (c.isBipartiteBoost(partition1, partition2)) {
        //     componentContraction = c.findRedDegreeContractionPartitioned(partition1, partition2);
        //     cout << "c ITS BIPARTITE" << endl;
        // }
        // else componentContraction = c.findRedDegreeContraction();
        // cout << componentContraction.str();
        // if (density < 0.01) {
        //     cout << c.findRedDegreeContractionRandomWalk().str();
        // }
        // else {
        //     cout << c.findRedDegreeContraction().str();
        // }

        float degreeDeviation = c.getDegreeDeviation();
        cout << "c Deviation: " << degreeDeviation << endl;

        // if (degreeDeviation <= 25.0) cout << c.findRedDegreeContractionRandomWalk().str();
        // else cout << c.findDegreeContraction().str();

        cout << c.findRedDegreeContractionRandomWalk().str();

        maxTww = max(maxTww, c.getWidth());

        if (c.getVertices().size() == 1){
            int remainingVertex = c.getVertexId(*c.getVertices().begin()) + 1;
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
    cout << "c twin-width: " << maxTww << endl;
    return 0;    
}


// int main() {
//     Graph g;
//     string line;
//     int numVertices, numEdges;
//     set<pair<int, int>> readEdges;
//     double density;
//     int maxTww = 0;
//     bool constructComplement = false;
//     auto global_start = high_resolution_clock::now();

//     auto start = high_resolution_clock::now(); 

//     while (getline(cin, line)) {
//         if (line[0] == 'c') {
//             continue;
//         }

//         vector<string> tokens;
//         string token;
//         std::stringstream tokenStream(line);
//         while (tokenStream >> token) {
//             tokens.push_back(token);
//         }

//         if (tokens[0] == "p") {
//             numVertices = stoi(tokens[2]);
//             numEdges = stoi(tokens[3]);
//             g.addVertices(numVertices);

//             density = (2.0 * numEdges) / (numVertices * (numVertices - 1));
//             if (density > 0.5) {
//                 constructComplement = true;
//             }
//         } else if (constructComplement) {
//             int u = stoi(tokens[0]);
//             int v = stoi(tokens[1]);
//             readEdges.insert({min(u-1, v-1), max(u-1, v-1)});
//         } else {
//             int u = stoi(tokens[0]);
//             int v = stoi(tokens[1]);
//             g.addEdgeBegin(u - 1, v - 1);
//         }
//     }

//     if (constructComplement) {
//         for (int i = 0; i < numVertices; i++) {
//             for (int j = i + 1; j < numVertices; j++) {
//                 if (readEdges.find({i, j}) == readEdges.end()) {
//                     g.addEdgeBegin(i, j);
//                 }
//             }
//         }
//     }
//     g.updateBlackDegrees();
//     g.setIds(g.getVertices());

//     auto stop = high_resolution_clock::now();
//     auto duration = duration_cast<seconds>(stop - start);
//     std::cout << "c Time taken too initialize the graph: " << duration.count() << " seconds" << std::endl;

//     start = high_resolution_clock::now(); 
    
//     vector<Graph> components;
//     if (connectedComponents) {
//         components = g.findConnectedComponentsBoost();
//     }
//     else {
//         components.push_back(g);
//     }
    
//     stop = high_resolution_clock::now();
//     duration = duration_cast<seconds>(stop - start);
//     cout << "c Time taken for connected components: " << duration.count() << " seconds" << std::endl;

//     vector<ComponentSolution> componentSolutions(components.size());
//     vector<int> remainingVertices((components.size()));

//     for (size_t i = 0; i < components.size(); ++i) {
//         Graph componentCopy(components[i]);
//         componentSolutions[i] = componentCopy.findRedDegreeContractionRandomWalkExhaustively();
//         int remainingVertex = componentCopy.getVertexId(*componentCopy.getVertices().begin()) + 1;
//         remainingVertices[i] = remainingVertex;
//         maxTww = max(maxTww, componentSolutions[i].width);
//     }

//     while (true) { 
//         auto curr_time = high_resolution_clock::now();
//         auto elapsed_time = duration_cast<seconds>(curr_time - global_start);
//         if (elapsed_time.count() >= TIME_LIMIT) {
//             break; 
//         }

//         size_t worstIdx = 0;
//         int worstWidth = componentSolutions[0].width;
//         for (size_t i = 1; i < componentSolutions.size(); ++i) {
//             if (componentSolutions[i].width > worstWidth) {
//                 worstWidth = componentSolutions[i].width;
//                 worstIdx = i;
//             }
//         }

//         Graph worstComponentCopy(components[worstIdx]); 
//         ComponentSolution tempSolution = worstComponentCopy.findRedDegreeContractionRandomWalkExhaustively(componentSolutions[worstIdx]);
        
//         if (worstComponentCopy.getVertices().size() != 1) continue;
        
//         if (tempSolution.width < componentSolutions[worstIdx].width) {
//             componentSolutions[worstIdx] = tempSolution;
//             int remainingVertex = worstComponentCopy.getVertexId(*worstComponentCopy.getVertices().begin()) + 1;
//             remainingVertices[worstIdx] = remainingVertex;
//             maxTww = componentSolutions[worstIdx].width;
//         }
//     }

//     for (const auto& solution : componentSolutions) {
//         cout << solution.stringSequence.str();
//     }

//     int primaryVertex = remainingVertices[0];
//     for (size_t i = 1; i < remainingVertices.size(); ++i) {
//         cout << primaryVertex << " " << remainingVertices[i] << endl;
//     }

//     auto final_stop = high_resolution_clock::now();
//     auto final_duration = duration_cast<seconds>(final_stop - start);
//     std::cout << "c In total: " << final_duration.count() << " seconds" << std::endl;
//     cout << "c twin-width: " << maxTww << endl;
//     return 0;    
// }