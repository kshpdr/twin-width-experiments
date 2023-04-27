import java.util.*;

public class Graph {
    private Set<Vertex> vertices;
    private Map<Vertex, Set<Vertex>> edges;
    private Map<Vertex, Set<Vertex>> redEdges;
    private int twinWidth = 0;

    public Graph() {
        vertices = new HashSet<>();
        edges = new HashMap<>();
        redEdges = new HashMap<>();
    }

    public void addVertex(Vertex v) {
        vertices.add(v);
        edges.put(v, new HashSet<>());
        redEdges.put(v, new HashSet<>());
    }

    public void removeVertex(Vertex v) {
        vertices.remove(v);
        edges.replace(v, new HashSet<>());
        redEdges.replace(v, new HashSet<>());

        for (Vertex u : vertices) {
            edges.get(u).remove(v);
            redEdges.get(u).remove(v);
        }
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

    public void addEdge(Vertex v, Vertex u) {
        edges.get(v).add(u);
        edges.get(u).add(v);
    }

    public void addRedEdge(Vertex v, Vertex u) {
        redEdges.get(v).add(u);
        redEdges.get(u).add(v);
    }

    public void removeEdge(Vertex v, Vertex u) {
        edges.get(v).remove(u);
        edges.get(u).remove(v);
        redEdges.get(v).remove(u);
        redEdges.get(u).remove(v);
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

    public void mergeVertices(Vertex source, Vertex twin) {
//        any edge (black or red) between x and y gets deleted
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
