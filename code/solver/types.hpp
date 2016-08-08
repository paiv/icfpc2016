#pragma once

namespace paiv {


static inline u8
near(r64 a, r64 b, r64 eps)
{
  return fabs(b - a) < eps;
}


template <typename T>
struct vertex_t {
  T x;
  T y;

  bool operator == (const vertex_t<T>& other) const {
    return x == other.x && y == other.y;
  }
  bool operator != (const vertex_t<T>& other) const {
    return !(*this == other);
  }
};

typedef r32 fraction;
typedef vertex_t<fraction> vertex;

typedef vector<vertex> poly;
typedef vector<poly> polys;
typedef vector<u32> vrefs;

typedef struct {
  u32 problem_id;
  polys outline;
  poly skeleton;
} problem;

typedef struct {
  u32 problem_id;
  poly vertices;
  vector<vrefs> facets;
  poly destination;
} solution;


struct shape;

typedef struct edge {
  vertex pa;
  vertex pb;
  shape* owner;
  shape* neighbor;

  bool operator == (const edge& other) const {
    return (pa == other.pa && pb == other.pb)
      || (pa == other.pb && pb == other.pa);
  }
  bool operator != (const edge& other) const {
    return !(*this == other);
  }

  vertex otherEnd(const vertex& v) const { return v == pa ? pb : pa; }

} edge;

typedef struct shape {
  poly body;
  poly dest;
  vector<edge> edges;

  shape() {}
  ~shape() {}
  shape(const shape& other);
  shape(const shape& other, u8 preserveLinks);
  shape(shape&& other) noexcept;
  shape& operator = (shape&& other) noexcept;
  shape& operator = (const shape& other);

  bool operator == (const shape& other) const {
    return body == other.body;
  }

  vector<edge>::reference find(const edge& g);
  vector<edge>::const_reference find(const edge& g) const;

  u8 isclockwise(const vertex& a, const vertex& b) const;

} shape;

class Graph {
  vector<shape> shapes;
public:
  Graph() {}
  explicit Graph(vector<shape>&& shapes);
  Graph(const Graph& other);
  Graph(Graph&& other) noexcept;
  Graph& operator = (Graph&& other) noexcept;
  Graph& operator = (const Graph& other);

  size_t size() const { return shapes.size(); }
  poly vertices(u8 destination = false) const;
  poly unique_vertices(u8 destination = false) const;
  const vector<edge> edges() const;
  polys outline() const;
  vector<vrefs> facets_as_indices(const poly& refer) const;
  size_t hashcode() const;
  u8 isempty() const { return shapes.size() == 0; }

  bool operator == (const Graph& other) const {
    return shapes == other.shapes;
  }

  shape* at(const Graph& other, const shape* os);
  const shape* at(const Graph& other, const shape* os) const;

  const vector<edge> edgesAt(const vertex& p) const;
  vector<edge*> edgesAt(const edge& g);
  const vector<shape> facetsOnEdgePoints(const edge& g) const;
  const vector<shape> facetsAt(const vertex& p) const;

  Graph copy_flip(const edge& g) const;

};

class Origami {
public:
  Graph graph;
  polys outline;

  Origami() {}
  explicit Origami(Graph&& other);

  static Origami from(const polys& outline, const poly& skeleton);

  // origami copy() const;
  u8 isempty() const { return graph.isempty() || outline.size() == 0; }

  bool operator == (const Origami& other) const { return graph == other.graph; }

  Origami unfold(const edge& g) const;

};


typedef struct search_state {
  u8 terminal;
  Origami origami;

  search_state() : terminal(false) {}

  u8 isgoal() const;
  u8 isterminal() const;
  solution get_solution() const;
  vector<search_state> children() const;

} search_state;


template <typename T>
ostream& operator << (ostream& so, const vertex_t<T>& v) {
  so << v.x << "," << v.y;
  return so;
}

template <typename T>
ostream& operator << (ostream& so, const vector<vertex_t<T>>& p) {
  so << p.size() << endl;
  for (auto v : p)
    so << v << endl;
  return so;
}

ostream& operator << (ostream& so, const problem& p) {
  so << p.outline.size() << endl;
  for (auto x : p.outline)
    so << x;
  so << p.skeleton;
  return so;
}

ostream& operator << (ostream& so, const solution& s) {
  so << s.vertices;
  so << s.facets.size() << endl;
  for (auto x : s.facets) {
    so << x.size();
    for (auto i : x)
      so << " " << i;
    so << endl;
  }
  for (auto v : s.destination)
    so << v << endl;

  return so;
}

ostream& operator << (ostream& so, const vector<vertex>& s) {
  so << "{";
  for (auto& v : s)
    so << "(" << v.x << "," << v.y << "), ";
  so << "}";
  return so;
}

ostream& operator << (ostream& so, const search_state& s) {
  so << "outline: ";
  for (auto& fig : s.origami.outline)
    so << fig;
  return so;
}

}


namespace std
{
  using namespace paiv;

  template <>
  class hash<vertex>
  {
  public:
    size_t operator() (const vertex& a) const
    {
      hash<fraction> h;
      size_t seed = 0;
      seed ^= h(a.x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
      seed ^= h(a.y) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
      return seed;
    }
  };

  template <>
  class hash<poly>
  {
  public:
    size_t operator() (const poly& a) const
    {
      hash<vertex> h;
      size_t seed = 0;
      for (auto v : a) {
        seed ^= h(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
      }
      return seed;
    }
  };

  template <>
  class hash<shape>
  {
  public:
    size_t operator() (const shape& a) const
    {
      hash<poly> h;
      size_t seed = 0;
      seed ^= h(a.body) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
      return seed;
    }
  };

  template <>
  class hash<Graph>
  {
  public:
    size_t operator() (const Graph& a) const
    {
      return a.hashcode();
    }
  };

  template <>
  class hash<Origami>
  {
  public:
    size_t operator() (const Origami& a) const
    {
      hash<Graph> h;
      size_t seed = 0;
      seed ^= h(a.graph) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
      return seed;
    }
  };

  template <>
  class hash<search_state>
  {
  public:
    size_t operator() (const search_state& a) const
    {
      hash<Origami> h;
      size_t seed = 0;
      seed ^= h(a.origami) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
      return seed;
    }
  };

  template <>
  struct equal_to<search_state>
  {
    inline bool operator() (const search_state& a, const search_state& b) const
    {
      return a.origami == b.origami;
    }
  };
}
