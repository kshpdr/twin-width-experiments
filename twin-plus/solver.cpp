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

const auto TIME_LIMIT = std::chrono::seconds(1000);

struct PairHash {
    size_t operator()(const pair<int, int>& p) const {
        return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 1);
    }
};

ostringstream generateRandomContractionSequence(const ankerl::unordered_dense::set<int>& verticesSet) {
    ostringstream contractionSequence;

    std::random_device rd;
    std::mt19937 gen(rd());

    std::vector<int> vertices(verticesSet.begin(), verticesSet.end());

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
    // std::map<int, ankerl::unordered_dense::set<int>> degreeToVertices;
    bool useRedDegreeMap = false;
    // bool useDegreeMap = false;
    int width = 0; 

public:
    Graph() {}

    Graph(const Graph &g) {
        this->vertices = g.vertices;
        this->adjListBlack = g.adjListBlack;
        this->adjListRed = g.adjListRed;
        this->redDegreeToVertices = g.redDegreeToVertices;
        // this->degreeToVertices = g.degreeToVertices;
        // this->useDegreeMap = g.useDegreeMap;
        this->useRedDegreeMap = g.useRedDegreeMap;
        this->width = g.width;
    }

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
        // updateVertexDegree(v1, 1);
        // updateVertexDegree(v2, 1);
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
            // order matters since updateVertexDegree uses adjListBlack's state
            // updateVertexDegree(v1, -1);
            // updateVertexDegree(v2, -1);
            adjListBlack[v1].erase(v2);
            adjListBlack[v2].erase(v1);
        } else if (adjListRed[v1].find(v2) != adjListRed[v1].end()) {
            // updateVertexDegree(v1, -1);
            // updateVertexDegree(v2, -1);
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

    ankerl::unordered_dense::set<int> getTwoNeighborhood(int vertex) {
        ankerl::unordered_dense::set<int> firstNeighbors;
        ankerl::unordered_dense::set<int> secondNeighbors;
        if (adjListRed.find(vertex) != adjListRed.end()) {
            firstNeighbors.insert(adjListRed[vertex].begin(), adjListRed[vertex].end());
        }
        if (adjListBlack.find(vertex) != adjListBlack.end()) {
            firstNeighbors.insert(adjListBlack[vertex].begin(), adjListBlack[vertex].end());
        }

        for (int directNeighbor : firstNeighbors) {
            if (adjListRed.find(directNeighbor) != adjListRed.end()) {
                secondNeighbors.insert(adjListRed[directNeighbor].begin(), adjListRed[directNeighbor].end());
            }
            if (adjListBlack.find(directNeighbor) != adjListBlack.end()) {
                secondNeighbors.insert(adjListBlack[directNeighbor].begin(), adjListBlack[directNeighbor].end());
            }
        }
        secondNeighbors.insert(firstNeighbors.begin(), firstNeighbors.end());
        secondNeighbors.erase(vertex);

        return secondNeighbors;
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
        // degreeToVertices[adjListBlack[vertex].size() + adjListRed[vertex].size()].erase(vertex);
    }

    pair<set<int>, set<int>> removeVertexAndReturnNeighbors(int vertex) {        
        // Remove the vertex from the black adjacency list and update neighbors
        set<int> blackNeighbors;
        set<int> redNeighbors;
        if (adjListBlack.find(vertex) != adjListBlack.end()) {
            for (int neighbor : adjListBlack[vertex]) {
                removeEdge(neighbor, vertex);
                blackNeighbors.insert(neighbor);
            }

            adjListBlack.erase(vertex);
        }
        
        // Remove the vertex from the red adjacency list and update neighbors
        if (adjListRed.find(vertex) != adjListRed.end()) {
            for (int neighbor : adjListRed[vertex]) {
                removeEdge(neighbor, vertex);
                redNeighbors.insert(neighbor);
            }
            adjListRed.erase(vertex);
        }
        
        vertices.erase(vertex);
        redDegreeToVertices[adjListRed[vertex].size()].erase(vertex);
        // degreeToVertices[adjListBlack[vertex].size() + adjListRed[vertex].size()].erase(vertex);
        pair<set<int>, set<int>> neighbors = make_pair(blackNeighbors, redNeighbors);
        return neighbors;
    }

    void addBackVertex(int vertex, pair<set<int>, set<int>> neighbors){
        addVertex(vertex);
        for (int neighbor : neighbors.first) {
            addEdge(vertex, neighbor);
        }

        for (int neighbor : neighbors.second) {
            addEdge(vertex, neighbor, "red");
        }
    }

    int getWidth() const {
        return width;
    }

    void updateVertexRedDegree(int vertex, int diff) {
        if (!useRedDegreeMap) return;
        int oldDegree = adjListRed[vertex].size();
        
        redDegreeToVertices[oldDegree].erase(vertex);
        if (redDegreeToVertices[oldDegree].empty()) {
            redDegreeToVertices.erase(oldDegree);
        }

        redDegreeToVertices[oldDegree + diff].insert(vertex);
    }

    // void updateVertexDegree(int vertex, int diff) {
    //     if (!useDegreeMap) return;
    //     int oldDegree = adjListRed[vertex].size() + adjListBlack[vertex].size();
        
    //     degreeToVertices[oldDegree].erase(vertex);
    //     if (degreeToVertices[oldDegree].empty()) {
    //         degreeToVertices.erase(oldDegree);
    //     }

    //     degreeToVertices[oldDegree + diff].insert(vertex);
    // }

    std::vector<int> getTopNVerticesWithLowestRedDegree(int n) {
        std::vector<int> topVertices;
        for (auto it = redDegreeToVertices.begin(); it != redDegreeToVertices.end() && topVertices.size() < n; ++it) {
            for (int vertex : it->second) {
                if (topVertices.size() >= n) break;
                topVertices.push_back(vertex);
            }
        }
        return topVertices;
    }

    // std::vector<int> getTopNVerticesWithLowestDegree(int n) {
    //     std::vector<int> topVertices;
    //     for (auto it = degreeToVertices.begin(); it != degreeToVertices.end() && topVertices.size() < n; ++it) {
    //         for (int vertex : it->second) {
    //             if (topVertices.size() >= n) break;
    //             topVertices.push_back(vertex);
    //         }
    //     }
    //     return topVertices;
    // }

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

    ankerl::unordered_dense::set<int> transferRedEdgesAndReturnNeighbors(int fromVertex, int toVertex) {
        ankerl::unordered_dense::set<int> neighbors;
        // If the twin vertex has red edges
        if(adjListRed.find(fromVertex) != adjListRed.end()) {
            for (int vertex : adjListRed[fromVertex]) {
                if (adjListRed[toVertex].find(vertex) == adjListRed[toVertex].end()) {
                    addEdge(toVertex, vertex, "red");
                    neighbors.insert(vertex);
                }
            }
        }
        return neighbors;
    }

    void deleteTransferedEdges(int vertex, ankerl::unordered_dense::set<int> neighbors) {
        if(!neighbors.empty()) {
            for (int neighbor : neighbors) {
                removeEdge(vertex, neighbor);
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

    std::set<int> markUniqueEdgesRedAndReturnNeighbors(int source, int twin) {
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
        return toBecomeRed;
    }

    void unmarkUniqueEdgesRed(int vertex, set<int> neighbors){
        for (int neighbor : neighbors) {
            removeEdge(vertex, neighbor);
            addEdge(vertex, neighbor);
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

    set<int> addNewRedNeighborsAndReturnThem(int source, int twin) {
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
        return newRedEdges;
    }

    void deleteNewNeighbors(int source, set<int> neighbors) {
        for (int neighbor : neighbors) {
            removeEdge(source, neighbor);
        }
    }


    int getRealScore(int source, int twin) {
        // Graph graphCopy(*this);   // Assuming you've implemented the copy constructor for Graph class

        // // Merge vertices on the copied graph
        // graphCopy.mergeVertices(source, twin);

        // // Return the updated width of the copied graph
        // return graphCopy.getWidth();

        bool blackEdgeExists = adjListBlack[source].contains(twin);
        bool redEdgeExists = adjListRed[source].contains(twin);

        removeEdge(source, twin);
        ankerl::unordered_dense::set<int> transferedNeighbors = transferRedEdgesAndReturnNeighbors(twin, source);
        set<int> uniqueNeighbors = markUniqueEdgesRedAndReturnNeighbors(source, twin);
        set<int> newNeighbors = addNewRedNeighborsAndReturnThem(source, twin);
        pair<set<int>, set<int>> twinNeighbors = removeVertexAndReturnNeighbors(twin);
        int score = getUpdatedWidth();

        if (blackEdgeExists) addEdge(source, twin);
        else if (redEdgeExists) addEdge(source, twin, "red");
    
        deleteTransferedEdges(source, transferedNeighbors);
        unmarkUniqueEdgesRed(source, uniqueNeighbors);
        deleteNewNeighbors(source, newNeighbors);
        addBackVertex(twin, twinNeighbors);

        return score;
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
        std::set<int> symmetric_difference;

        std::set_symmetric_difference(neighbors_v1.begin(), neighbors_v1.end(),
                                    neighbors_v2.begin(), neighbors_v2.end(),
                                    std::inserter(symmetric_difference, symmetric_difference.begin()));

        symmetric_difference.erase(v1);
        symmetric_difference.erase(v2);

        return symmetric_difference.size();
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
        ankerl::unordered_dense::map<pair<int, int>, int, PairHash> scores;
        auto heuristic_start_time = high_resolution_clock::now();
        
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

            auto elapsed_time = high_resolution_clock::now() - heuristic_start_time;
            if (elapsed_time > TIME_LIMIT) {
                contractionSequence << generateRandomContractionSequence(vertices).str();
                break;
            }

            auto stop = high_resolution_clock::now();
            auto duration = duration_cast<milliseconds>(stop - start);
            int seconds_part = duration.count() / 1000;
            int milliseconds_part = duration.count() % 1000;
            std::cout << "c (Left " << vertices.size() << ") Cycle in " << seconds_part << "." 
            << std::setfill('0') << std::setw(9) << milliseconds_part 
            << " seconds" << std::endl;
        }
        return contractionSequence;
    }

    // ostringstream findDegreeContraction(){ 
    //     ostringstream contractionSequence;
    //     ankerl::unordered_dense::map<pair<int, int>, int, PairHash> scores;
    //     auto heuristic_start_time = high_resolution_clock::now();
        
    //     while (vertices.size() > 1) {
    //         auto start = high_resolution_clock::now();

    //         vector<int> lowestDegreeVertices = getTopNVerticesWithLowestDegree(20);
            
    //         int bestScore = INT_MAX;
    //         pair<int, int> bestPair;

    //         for (int i = 0; i < lowestDegreeVertices.size(); i++) {
    //             for (int j = i+1; j < lowestDegreeVertices.size(); j++) {
    //                 int v1 = lowestDegreeVertices[i];
    //                 int v2 = lowestDegreeVertices[j];
    //                 // if (!getTwoNeighborhood(v1).contains(v2)) continue;
    //                 if (v2 > v1) {
    //                     std::swap(v1, v2);
    //                 }

    //                 auto it = scores.find({v1, v2});
    //                 int score;
    //                 if (it != scores.end()) {
    //                     score = it->second;
    //                 }
    //                 else {
    //                     score = getScore(v1, v2);
    //                     scores[{v1, v2}] = score;
    //                 }
                    
    //                 if (score < bestScore) {
    //                     bestScore = score;
    //                     bestPair = {v1, v2};
    //                 }
    //             }
    //         }

    //         contractionSequence << bestPair.first + 1 << " " << bestPair.second + 1 << "\n";
    //         mergeVertices(bestPair.first, bestPair.second);

    //         auto elapsed_time = high_resolution_clock::now() - heuristic_start_time;
    //         if (elapsed_time > TIME_LIMIT) {
    //             contractionSequence << generateRandomContractionSequence(vertices).str();
    //             break;
    //         }

    //         auto stop = high_resolution_clock::now();
    //         auto duration = duration_cast<milliseconds>(stop - start);
    //         int seconds_part = duration.count() / 1000;
    //         int milliseconds_part = duration.count() % 1000;
    //         std::cout << "c (Left " << vertices.size() << ") Cycle in " << seconds_part << "." 
    //         << std::setfill('0') << std::setw(9) << milliseconds_part 
    //         << " seconds" << std::endl;
    //     }
    //     return contractionSequence;
    // }


    // really bad, h002 921
    ostringstream findRedDegreeContractionN2(){ 
        ostringstream contractionSequence;
        ankerl::unordered_dense::map<pair<int, int>, int, PairHash> scores;
        auto heuristic_start_time = high_resolution_clock::now();
        
        while (vertices.size() > 1) {
            auto start = high_resolution_clock::now();

            vector<int> lowestDegreeVertices = getTopNVerticesWithLowestRedDegree(20);
            
            int bestScore = INT_MAX;
            pair<int, int> bestPair;

            for (int i = 0; i < lowestDegreeVertices.size(); i++) {
                int v1 = lowestDegreeVertices[i];
                ankerl::unordered_dense::set<int> twoNeighborhood = getTwoNeighborhood(lowestDegreeVertices[i]);
                ankerl::unordered_dense::set<int>::iterator it = twoNeighborhood.begin();
                for(int i = 0; i < 10 && it != twoNeighborhood.end(); ++i, ++it) {
                    int v2 = *it;
                    if (v2 > v1) {
                        std::swap(v1, v2);
                    }

                    auto it2 = scores.find({v1, v2});
                    int score;
                    if (it2 != scores.end()) {
                        score = it2->second;
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

            auto elapsed_time = high_resolution_clock::now() - heuristic_start_time;
            if (elapsed_time > TIME_LIMIT) {
                contractionSequence << generateRandomContractionSequence(vertices).str();
                break;
            }

            auto stop = high_resolution_clock::now();
            auto duration = duration_cast<milliseconds>(stop - start);
            int seconds_part = duration.count() / 1000;
            int milliseconds_part = duration.count() % 1000;
            std::cout << "c (Left " << vertices.size() << ") Cycle in " << seconds_part << "." 
            << std::setfill('0') << std::setw(9) << milliseconds_part 
            << " seconds" << std::endl;
        }
        return contractionSequence;
    }

    ostringstream findRedDegreeContractionMultipleVertices(){ 
        ostringstream contractionSequence;
        ankerl::unordered_dense::map<pair<int, int>, int, PairHash> scores;
        auto heuristic_start_time = high_resolution_clock::now();
        
        while (vertices.size() > 1) {
            auto start = high_resolution_clock::now();

            vector<int> lowestDegreeVertices = getTopNVerticesWithLowestRedDegree(20);
            
            // Container to store top 5 pairs of the iteration
            vector<pair<int, pair<int, int>>> topPairs; 

            for (int i = 0; i < lowestDegreeVertices.size(); i++) {
                for (int j = i+1; j < lowestDegreeVertices.size(); j++) {
                    int v1 = lowestDegreeVertices[i];
                    int v2 = lowestDegreeVertices[j];
                    if (v2 > v1) {
                        std::swap(v1, v2);
                    }

                    // Check if either vertex is already in topPairs. If so, skip.
                    bool alreadyExists = false;
                    for (const auto& topPair : topPairs) {
                        if (topPair.second.first == v1 || topPair.second.first == v2 || topPair.second.second == v1 || topPair.second.second == v2) {
                            alreadyExists = true;
                            break;
                        }
                    }
                    if (alreadyExists) continue;

                    auto it = scores.find({v1, v2});
                    int score;
                    if (it != scores.end()) {
                        score = it->second;
                    }
                    else {
                        score = getScore(v1, v2);
                        scores[{v1, v2}] = score;
                    }

                    if (topPairs.size() < 5) {
                        topPairs.push_back({score, {v1, v2}});
                    }
                    else {
                        // Find the pair with the worst score and replace if current score is better
                        auto worstPairIt = std::max_element(topPairs.begin(), topPairs.end(), [](const auto& a, const auto& b) {
                            return a.first < b.first;
                        });
                        if (score < worstPairIt->first) {
                            *worstPairIt = {score, {v1, v2}};
                        }
                    }
                }
            }

            // Contract the top pairs and remove from scores
            for (const auto& topPair : topPairs) {
                contractionSequence << topPair.second.first + 1 << " " << topPair.second.second + 1 << "\n";
                mergeVertices(topPair.second.first, topPair.second.second);
                scores.erase(topPair.second);
            }

            auto elapsed_time = high_resolution_clock::now() - heuristic_start_time;
            if (elapsed_time > TIME_LIMIT) {
                contractionSequence << generateRandomContractionSequence(vertices).str();
                break;
            }

            auto stop = high_resolution_clock::now();
            auto duration = duration_cast<milliseconds>(stop - start);
            int seconds_part = duration.count() / 1000;
            int milliseconds_part = duration.count() % 1000;
            std::cout << "c (Left " << vertices.size() << ") Cycle in " << seconds_part << "." 
            << std::setfill('0') << std::setw(9) << milliseconds_part 
            << " seconds" << std::endl;
        }
        return contractionSequence;
    }


private:
    void updateWidth() {
        for (const auto& pair : adjListRed) {
            width = max(width, static_cast<int>(pair.second.size()));
        }
    }

    int getUpdatedWidth() {
        int updatedWidth;
        for (const auto& pair : adjListRed) {
            updatedWidth = max(updatedWidth, static_cast<int>(pair.second.size()));
        }
        return updatedWidth;
    }
};

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

        if (tokens[0] == "p" && tokens[1] == "tww") {
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

    cout << g.findRedDegreeContraction().str();

    auto final_stop = high_resolution_clock::now();
    auto final_duration = duration_cast<seconds>(final_stop - start);
    std::cout << "c In total: " << final_duration.count() << " seconds" << std::endl;

    return 0;    
}