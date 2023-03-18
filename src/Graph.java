import java.util.*;

public class Graph {
    private final Set<Vertex> vertices;
    private final Map<Vertex, Set<Vertex>> edges;

    public Graph() {
        vertices = new HashSet<>();
        edges = new HashMap<>();
    }

    public void addVertex(Vertex v) {
        vertices.add(v);
        edges.put(v, new HashSet<>());
    }

    public void removeVertex(Vertex v) {
        vertices.remove(v);
        edges.remove(v);
        for (Vertex u : vertices) {
            edges.get(u).remove(v);
        }
    }

    public void addEdge(Vertex v, Vertex u) {
        edges.get(v).add(u);
        edges.get(u).add(v);
    }

    public void removeEdge(Vertex v, Vertex u) {
        edges.get(v).remove(u);
        edges.get(u).remove(v);
    }

    public Set<Vertex> getVertices() {
        return vertices;
    }

    public Set<Vertex> getNeighbors(Vertex v) {
        return edges.get(v);
    }

    public boolean hasEdge(Vertex v, Vertex u) {
        return edges.get(v).contains(u);
    }

    public int getSize() {
        return vertices.size();
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
