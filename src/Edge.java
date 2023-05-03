public class Edge {
    private final Vertex v1;
    private final Vertex v2;
    private int score = Integer.MIN_VALUE;

    public Edge(Vertex v1, Vertex v2) {
        this.v1 = v1;
        this.v2 = v2;
    }

    public Edge(Vertex v1, Vertex v2, int score) {
        this.v1 = v1;
        this.v2 = v2;
        this.score = score;
    }

    public Vertex getV1() {
        return v1;
    }

    public Vertex getV2() {
        return v2;
    }

    public int getScore() {
        return score;
    }

    public void setScore(int score) {
        this.score = score;
    }
}
