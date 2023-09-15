import java.util.*;

public class Graph {
    private Set<Vertex> vertices;
    private LinkedList<Vertex> verticesList;
    private Map<Vertex, Set<Vertex>> edges;
    private Map<Vertex, Set<Vertex>> redEdges;
    private Map<Integer, Set<Vertex>> degreeToVertices;
    public PriorityQueue<Vertex> vertexScores = new PriorityQueue<>(Comparator.comparingInt(Vertex::getScore));
    private int twinWidth = 0;

    public Graph() {
        vertices = new HashSet<>();
        verticesList = new LinkedList<>();
        edges = new HashMap<>();
        redEdges = new HashMap<>();
        degreeToVertices = new HashMap<>();
    }

    public void addVertex(Vertex v) {
        vertices.add(v);
        verticesList.add(v);
        edges.put(v, new HashSet<>());
        redEdges.put(v, new HashSet<>());
    }

    public void removeVertex(Vertex v) {
        degreeToVertices.get(edges.get(v).size()).remove(v);
        vertices.remove(v);
        verticesList.remove(v);
        edges.replace(v, new HashSet<>());
        redEdges.replace(v, new HashSet<>());

        for (Vertex u : vertices) {
            int oldDegreeU = edges.get(u).size();
            edges.get(u).remove(v);
            redEdges.get(u).remove(v);
            updateDegreeToVerticesMapping(u, oldDegreeU);
        }

        vertexScores.remove(v);
    }

    public Vertex getRandomVertex() {
        if (vertices.isEmpty()) {
            return null;
        }

        Random random = new Random();
        int randomIndex = random.nextInt(vertices.size());
        Vertex[] vertexArray = vertices.toArray(new Vertex[0]);
        return vertexArray[randomIndex];
    }

    public Vertex getVertexFromInterval(Vertex v, int threshold) {
        int lowerbound = Math.max(0, edges.get(v).size() - threshold);
        int upperbound = Math.min(edges.get(v).size(), edges.get(v).size() + threshold);
        Vertex bestVertex = getRandomVertex();
        while (bestVertex.equals(v)) bestVertex = getRandomVertex();
        int bestScore = getScore(v, bestVertex);
        for (int i = lowerbound; i <= upperbound; i++){
            HashSet<Vertex> candidates = (HashSet<Vertex>) getVerticesByDegree(i);
            for (Vertex candidate : candidates){
                if (v.equals(candidate)) continue;
                int score = getScore(v, candidate);
                if (score < bestScore){
                    bestVertex = candidate;
                    bestScore = score;
                }
            }
        }
        return bestVertex;
    }

    public PriorityQueue<Vertex> getVertexScores() { return vertexScores; }

    public void addEdge(Vertex v, Vertex u) {
        edges.get(v).add(u);
        edges.get(u).add(v);
        verticesList.get(verticesList.indexOf(v)).degree++;
        verticesList.get(verticesList.indexOf(u)).degree++;
        updateDegreeToVerticesMapping(v, edges.get(v).size() - 1);
        updateDegreeToVerticesMapping(u, edges.get(u).size() - 1);
    }


    public void addRedEdge(Vertex v, Vertex u) {
        redEdges.get(v).add(u);
        redEdges.get(u).add(v);
    }

    public void removeEdge(Vertex v, Vertex u) {
        if (!edges.containsKey(v)){
            System.out.println();
        }
        int oldDegreeV = edges.get(v).size();
        int oldDegreeU = edges.get(u).size();
        edges.get(v).remove(u);
        edges.get(u).remove(v);
        redEdges.get(v).remove(u);
        redEdges.get(u).remove(v);
        v.degree--;
        u.degree--;
        updateDegreeToVerticesMapping(v, oldDegreeV);
        updateDegreeToVerticesMapping(u, oldDegreeU);
    }

    private void updateDegreeToVerticesMapping(Vertex v, int oldDegree) {
        int newDegree = edges.get(v).size();

        // Remove vertex from the old degree list
        Set<Vertex> oldDegreeVertices = degreeToVertices.get(oldDegree);
        if (oldDegreeVertices != null) {
            oldDegreeVertices.remove(v);
        }

        // Add vertex to the new degree list
        Set<Vertex> newDegreeVertices = degreeToVertices.computeIfAbsent(newDegree, k -> new HashSet<>());
        newDegreeVertices.add(v);
    }


    public Set<Vertex> getVerticesByDegree(int degree) {
        return degreeToVertices.getOrDefault(degree, new HashSet<>());
    }

    public Set<Vertex> getVertices() {
        return vertices;
    }

    public Map<Vertex, Set<Vertex>> getEdges() {
        return edges;
    }


    public Set<Vertex> getNeighbors(Vertex v) {
        return edges.get(v);
    }

    public int[][] createScoreTable(){
        int[][] scores = new int[getVertices().size()][getVertices().size()];
        Arrays.stream(scores).forEach(row -> Arrays.fill(row, Integer.MIN_VALUE));
        for (Vertex v1 : getVertices()){
            for (Vertex v2 : getVertices()){
                if (v1.equals(v2)) scores[v1.getId() - 1][v1.getId() - 1] = 0;
                if (scores[v1.getId() - 1][v2.getId() - 1] != Integer.MIN_VALUE) continue;
                scores[v1.getId() - 1][v2.getId() - 1] = getScore(v1, v2);
                scores[v2.getId() - 1][v1.getId() - 1] = getScore(v1, v2);
            }
            v1.score = Arrays.stream(scores[v1.getId() - 1]).sum();
            vertexScores.add(v1);
        }
        return scores;
    }

    public int getScore(Vertex v1, Vertex v2){
        HashSet<Vertex> allNeighbors = new HashSet<>(getEdges().get(v1));
        allNeighbors.addAll(getEdges().get(v2));
        HashSet<Vertex> commonNeighbors = new HashSet<>(getEdges().get(v1));
        commonNeighbors.retainAll(getEdges().get(v2));
        allNeighbors.removeAll(commonNeighbors);
        allNeighbors.remove(v1);
        allNeighbors.remove(v2);
        return allNeighbors.size();
    }

    public HashSet<Vertex> getCommonNeighbors(Vertex v1, Vertex v2){
        HashSet<Vertex> commonNeighbors = new HashSet<>(getNeighbors(v1));
        commonNeighbors.retainAll(getNeighbors(v2));
        return commonNeighbors;
    }

    public HashSet<Vertex> getDifferentNeighbors(Vertex v1, Vertex v2){
        HashSet<Vertex> allNeighbors = new HashSet<>(getNeighbors(v1));
        allNeighbors.addAll(getNeighbors(v2));
        allNeighbors.removeAll(getCommonNeighbors(v1, v2));
        allNeighbors.remove(v1);
        allNeighbors.remove(v2);
        return allNeighbors;
    }

    public Edge getBmsEdge(int k){
        Vertex v = getRandomVertex();
        Vertex u = getRandomVertex();
        while (u.equals(v)) u = getRandomVertex();
        int mergeScore = getScore(v, u);
        for (int i = 0; i < k; i++) {
            Vertex v1 = getRandomVertex();
            Vertex u1 = getRandomVertex();
            while (u1.equals(v1)) u1 = getRandomVertex();
            int newScore = getScore(v1, u1);
            if (newScore < mergeScore){
                mergeScore = newScore;
                v = v1;
                u = u1;
            }
        }
        return new Edge(v, u, mergeScore);
    }

    public void mergeVertices(Vertex source, Vertex twin) {
        // any edge (black or red) between x and y gets deleted
        removeEdge(source, twin);
        transferRedEdges(twin, source);

        // x retains all black edges to its neighbors that are adjacent to y
        // all edges from x to vertices that are not adjacent to y become red
        Set<Vertex> redEdges = new HashSet<>(getNeighbors(source));
        redEdges.removeAll(getNeighbors(twin));
        for (Vertex v : redEdges) {
            addRedEdge(source, v);
        }

        // x is connected with a red edge to all vertices that are connected to y but not to x
        Set<Vertex> newNeighbors = new HashSet<>(getNeighbors(twin));
        newNeighbors.removeAll(getNeighbors(source));
        newNeighbors.remove(source);
        for (Vertex neighbor : newNeighbors) {
            addEdge(source, neighbor);
            addRedEdge(source, neighbor);
        }
        removeVertex(twin);
        twinWidth = Math.max(twinWidth, getRedEdgesAmount());
    }

    public int getRedEdgesFromPotentialMerge(Vertex source, Vertex twin) {
        Graph graph = new Graph();
        for (Vertex vertex : vertices){
            graph.addVertex(vertex);
        }
        for (Vertex vertex : edges.keySet()){
            for (Vertex neighbor : edges.get(vertex)){
                graph.addEdge(vertex, neighbor);
            }
        }
        for (Vertex vertex : redEdges.keySet()){
            for (Vertex neighbor : redEdges.get(vertex)){
                graph.addRedEdge(vertex, neighbor);
            }
        }
        graph.mergeVertices(source, twin);
        return graph.getTwinWidth();
    }

    public int getRedEdgesAmount() {
        int twinWidth = 0;
        for (Vertex v : redEdges.keySet()){
            twinWidth = Math.max(twinWidth, redEdges.get(v).size());
        }
        return twinWidth;
    }

    public void transferRedEdges(Vertex from, Vertex to){
        for (Vertex vertex : redEdges.get(from)){
            redEdges.get(to).add(vertex);
            redEdges.get(vertex).add(to);
        }
    }

    public void printRedEdges(){
        for (Vertex vertex : redEdges.keySet()){
            System.out.print(vertex + ": ");
            System.out.println(redEdges.get(vertex));
        }
    }
    public void printEdges(){
        for (Vertex vertex : edges.keySet()){
            System.out.print(vertex + ": ");
            System.out.println(edges.get(vertex));
        }
    }

    public int getSize() {
        return vertices.size();
    }

    public Vertex getVertexFromId(int id){
        for (Vertex vertex : vertices){
            if (vertex.getId() == id){
                return vertex;
            }
        }
        return null;
    }

    public int getTwinWidth(){
        return twinWidth;
    }

    public int getLowerbound(){
        int lowerbound = Integer.MAX_VALUE;
        for (Vertex v : vertices){
            for (Vertex u : vertices){
                if (v.equals(u)) continue;
                int redEdges = getRedEdgesFromPotentialMerge(v, u);
                if (redEdges < lowerbound){
                    lowerbound = redEdges;
                }
            }
        }
        return lowerbound;
    }

    public ArrayList<Vertex> getTopNVerticesByDegree(int n, boolean highest) {
        verticesList.sort(Comparator.comparingInt(Vertex::getDegree));

        return highest ?
                new ArrayList<Vertex>(verticesList. subList(verticesList.size() - n, verticesList.size())) :
                new ArrayList<Vertex>(verticesList.subList(0, n));
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        for (Vertex v : vertices) {
            sb.append(v.toString()).append(": ");
            for (Vertex u : edges.get(v)) {
                sb.append(u.toString()).append(" ");
            }
            sb.append("\n");
        }
        return sb.toString();
    }
}
