import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.Random;
import java.util.Set;

public class Solver {
//    public static LinkedList<String> findContractionSequence(Graph graph){
//        LinkedList<Edge> mergedEdges = new LinkedList<>();
//        while (graph.getSize() != 0){
////            Vertex source = graph.getVertices().iterator().next();
//            Vertex source = getRandomSetElement(graph.getVertices());
//            Vertex twin = null;
//
//            int maxNeighborDifference = -1;
//            for (Vertex vertex : graph.getVertices()){
//                if (source.equals(vertex)) continue;
//                if (graph.getCommonNeighbors(source, vertex).size() > maxNeighborDifference) {
//                    twin = vertex;
//                    maxNeighborDifference = graph.getCommonNeighbors(source, vertex).size();
//                }
//            }
//            int redEdges = (graph.getNeighbors(source).size() +  graph.getNeighbors(twin).size() - 2 * maxNeighborDifference - 2);
//            twinWidth = Math.max(redEdges, twinWidth);
//            graph.mergeVertices(source, twin);
//            mergedEdges.add(new Edge(source, twin));
//        }
//        return convertEdgesToStrings(mergedEdges);
//    }

    static <E> E getRandomSetElement(Set<E> set) {
        return set.stream().skip(new Random().nextInt(set.size())).findFirst().orElse(null);
    }

    // find two most similar vertices
    public static LinkedList<String> findContractionSequence(Graph graph){
        LinkedList<Edge> mergedEdges = new LinkedList<>();
        while (graph.getSize() != 1){
            Vertex source = null;
            Vertex twin = null;
            int minNeighborDifference = Integer.MAX_VALUE;
            for (Vertex v : graph.getVertices()){
                for (Vertex w : graph.getVertices()){
                    if (v.equals(w)) continue;
                    if (graph.getDifferentNeighbors(v, w).size() < minNeighborDifference) {
                        source = v;
                        twin = w;
                        minNeighborDifference = graph.getDifferentNeighbors(v, w).size();
                    }
                }
            }
            graph.mergeVertices(source, twin);
//            twinWidth = Math.max(graph.getCurrentTwinWidth(), twinWidth);
            mergedEdges.add(new Edge(source, twin));
        }
        return convertEdgesToStrings(mergedEdges);
    }

    public static LinkedList<String> convertEdgesToStrings(LinkedList<Edge> edges) {
        LinkedList<String> result = new LinkedList<>();
        for (Edge e : edges) {
            result.add(String.format("%d %d", e.getV1().getId(), e.getV2().getId()));
        }
        return result;
    }


    public static void main(String[] args) throws IOException {
        BufferedReader bi = new BufferedReader(new InputStreamReader(System.in));
//        BufferedReader bi = new BufferedReader(new FileReader("tests/custom-graphs/small-grid.gr"));
//        BufferedReader bi = new BufferedReader(new FileReader("tests/heuristic-public/heuristic_002.gr"));
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

//        System.out.println("Graph:");
//        System.out.println(graph);

        StringBuilder sb = new StringBuilder();
        for (String v : findContractionSequence(graph)){
            sb.append(v);
            sb.append("\n");
        }
        sb.append("c twin width: ").append(graph.getTwinWidth()).append("\n");
        String output = sb.toString();
        System.out.println(output);
    }
}
