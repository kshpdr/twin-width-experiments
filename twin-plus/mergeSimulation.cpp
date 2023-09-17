#include "solver.cpp"

pair<set<int>, set<int>> Graph::removeVertexAndReturnNeighbors(int vertex) {        
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
    pair<set<int>, set<int>> neighbors = make_pair(blackNeighbors, redNeighbors);
    return neighbors;
}

void Graph::addBackVertex(int vertex, pair<set<int>, set<int>> neighbors){
    addVertex(vertex);
    for (int neighbor : neighbors.first) {
        addEdge(vertex, neighbor);
    }

    for (int neighbor : neighbors.second) {
        addEdge(vertex, neighbor, "red");
    }
}

ankerl::unordered_dense::set<int> Graph::transferRedEdgesAndReturnNeighbors(int fromVertex, int toVertex) {
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

void Graph::deleteTransferedEdges(int vertex, ankerl::unordered_dense::set<int> neighbors) {
    if(!neighbors.empty()) {
        for (int neighbor : neighbors) {
            removeEdge(vertex, neighbor);
        }
    }
}

std::set<int> Graph::markUniqueEdgesRedAndReturnNeighbors(int source, int twin) {
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

void Graph::unmarkUniqueEdgesRed(int vertex, set<int> neighbors){
    for (int neighbor : neighbors) {
        removeEdge(vertex, neighbor);
        addEdge(vertex, neighbor);
    }
}


set<int> Graph::addNewRedNeighborsAndReturnThem(int source, int twin) {
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

void Graph::deleteNewNeighbors(int source, set<int> neighbors) {
    for (int neighbor : neighbors) {
        removeEdge(source, neighbor);
    }
}

int Graph::getRealScore(int source, int twin) {
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
