import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

public class Solver {
    public static void main(String[] args) throws IOException {
        BufferedReader bi = new BufferedReader(new InputStreamReader(System.in));
        Graph graph = new Graph();

        String line;
        while ((line = bi.readLine()) != null) {
            if (line.startsWith("c")) {
                continue;
            }

            String[] tokens = line.trim().split("\\s+");
            if (tokens[0].equals("p") && tokens[1].equals("tww")) {
                int numVertices = Integer.parseInt(tokens[2]);
                int numEdges = Integer.parseInt(tokens[3]);
                for (int i = 1; i <= numVertices; i++) {
                    graph.addVertex(new Vertex(i));
                }
            } else {
                tokens = line.trim().split("\\s+");
                int u = Integer.parseInt(tokens[0]);
                int v = Integer.parseInt(tokens[1]);
                graph.addEdge(new Vertex(u), new Vertex(v));
            }
        }

        System.out.println("Graph:");
        System.out.println(graph);
    }
}
