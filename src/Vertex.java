import java.util.Objects;

public class Vertex {
    private final int id;
    public int score = 0;
    public int degree = 0;

    public Vertex(int id) {
        this.id = id;
    }

    public int getId() {
        return id;
    }

    public int getScore() { return score; }

    public int getDegree() { return degree; }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        Vertex vertex = (Vertex) o;
        return id == vertex.id;
    }

    @Override
    public int hashCode() {
        return Objects.hash(id);
    }

    @Override
    public String toString() {
        return "Vertex{" +
                "id=" + id +
                '}';
    }
}
