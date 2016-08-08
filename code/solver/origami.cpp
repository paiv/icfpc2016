
namespace paiv {

static function<poly(const poly&)>
unique_mapping(const poly& vs) {
  unordered_set<vertex> x(begin(vs), end(vs));
  poly unique(begin(x), end(x));
  vector<size_t> indexes;

  for (auto v : unique) {
    auto i = find(begin(vs), end(vs), v) - begin(vs);
    indexes.push_back(i);
  }

  return [indexes] (const poly& us) {
    poly res;
    for (size_t i = 0; i < indexes.size(); i++) {
      res.push_back(us[indexes[i]]);
    }
    return res;
  };
}


shape::shape(const shape& other)
  : shape(other, true)
{
}

shape::shape(const shape& other, u8 preserveLinks)
{
  body = other.body;
  dest = other.dest;
  edges = other.edges;
  for (auto& g : edges) {
    g.owner = this;
    if (!preserveLinks)
      g.neighbor = nullptr;
  }
}

shape::shape(shape&& other) noexcept
{
  swap(body, other.body);
  swap(dest, other.dest);
  swap(edges, other.edges);
}

shape&
shape::operator = (shape&& other) noexcept
{
  body = move(other.body);
  dest = move(other.dest);
  edges = move(other.edges);
  return *this;
}

shape&
shape::operator = (const shape& other)
{
  *this = shape(other);
  return *this;
}

vector<edge>::reference
shape::find(const edge& g)
{
  return *std::find(begin(edges), end(edges), g);
}
vector<edge>::const_reference
shape::find(const edge& g) const
{
  return *std::find(begin(edges), end(edges), g);
}

u8
shape::isclockwise(const vertex& a, const vertex& b) const
{
  for (auto& dg : edges) {
    if (dg.pa != a && dg.pa != b) {
      auto x = dg.pa;
      return is_clockwise(a, b, x);
    }
    if (dg.pb != a && dg.pb != b) {
      auto x = dg.pb;
      return is_clockwise(a, b, x);
    }
  }
  return false;
}


Graph::Graph(vector<shape>&& shapes) : shapes(shapes)
{
}


shape*
relativePointer(vector<shape>& graph, const vector<shape>& other, const shape* os)
{
  return os == nullptr ? nullptr : &*(begin(graph) + (os - &*begin(other)));
}
const shape*
relativePointer(const vector<shape>& graph, const vector<shape>& other, const shape* os)
{
  return os == nullptr ? nullptr : &*(begin(graph) + (os - &*begin(other)));
}


Graph::Graph(const Graph& other)
{
  vector<shape> res = other.shapes;

  for (auto& fig : res) {
    for (auto& g : fig.edges) {

      if (g.neighbor != nullptr) {
        g.neighbor = relativePointer(res, other.shapes, g.neighbor);
      }

    }
  }

  shapes = move(res);
}

Graph::Graph(Graph&& other) noexcept
  : shapes(other.shapes)
{
}

Graph&
Graph::operator = (Graph&& other) noexcept
{
  shapes = move(other.shapes);
  return *this;
}

Graph&
Graph::operator = (const Graph& other)
{
  *this = Graph(other);
  return *this;
}


size_t
Graph::hashcode() const {
  hash<shape> h;
  size_t seed = 0;
  for (auto& fig : shapes) {
    seed ^= h(fig) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  }
  return seed;
}

poly
Graph::vertices(u8 destination) const
{
  poly res;
  for (auto& shape : shapes) {
    for (auto v : (destination ? shape.dest : shape.body))
      res.push_back(v);
  }
  return res;
}

poly
Graph::unique_vertices(u8 destination) const
{
  unordered_set<vertex> res;
  for (auto& shape : shapes) {
    for (auto v : (destination ? shape.dest : shape.body))
      res.insert(v);
  }
  return poly(begin(res), end(res));
}

const vector<edge>
Graph::edges() const
{
  vector<edge> res;
  auto fig = outline().front();
  for (size_t i = 0; i < fig.size(); i++) {
    edge g = { fig[i], fig[(i + 1) % fig.size()] };
    res.push_back(g);
  }
  return res;
}

polys
Graph::outline() const
{
  if (isempty()) return {};

  poly res; // ~~

  // get starting point on outer edge
  auto currentEdge = shapes.front().edges.front();
  for (auto& fig : shapes) {
    for (auto& dg : fig.edges) {
      if (facetsAt(dg.pa).size() == 1) {
        currentEdge = dg;
        goto next;
      }
    }
  }
next:

  auto startPoint = currentEdge.pa;
  if (!currentEdge.owner->isclockwise(currentEdge.pa, currentEdge.pb))
    startPoint = currentEdge.pb;
  res.push_back(startPoint);

  unordered_set<vertex> seen;
  seen.insert(startPoint);

  auto p = startPoint;

  for (u8 hasNext = true; hasNext; ) {
    hasNext = false;

    for (auto& dg : edgesAt(p)) {
      if (dg != currentEdge && dg.neighbor == nullptr) {
        p = dg.otherEnd(p);
        currentEdge = dg;
        hasNext = seen.find(p) == end(seen);
        if (hasNext) {
          if (res.size() > 1 && same_line(*(end(res)-2), *(end(res)-1), p)) {
            res.pop_back();
          }
          res.push_back(p);
          seen.insert(p);
        }
        break;
      }
    }
  }

  if (res.size() > 2 && same_line(*(end(res)-2), *(end(res)-1), *begin(res))) {
    res.pop_back();
  }
  if (res.size() > 2 && same_line(*begin(res), *(begin(res)+1), *(end(res) - 1))) {
    res.erase(begin(res));
  }

  return { res };
}

static vrefs
convert_to_indices(const poly& values, const poly& refer)
{
  vrefs facet;
  for (auto v : values) {
    auto it = find(begin(refer), end(refer), v);
    if (it == end(refer))
      return {};
    facet.push_back(it - begin(refer));
  }
  return facet;
}

vector<vrefs>
Graph::facets_as_indices(const poly& refer) const
{
  vector<vrefs> res;
  for (auto& shape : shapes) {
    auto facet = convert_to_indices(shape.body, refer);
    if (facet.size() > 0)
      res.push_back(facet);
  }
  return res;
}

shape*
Graph::at(const Graph& other, const shape* os)
{
  return relativePointer(shapes, other.shapes, os);
}
const shape*
Graph::at(const Graph& other, const shape* os) const
{
  return relativePointer(shapes, other.shapes, os);
}

const vector<edge>
Graph::edgesAt(const vertex& p) const
{
  vector<edge> res;
  for (auto& fig : shapes) {
    for (auto& dg : fig.edges) {
      if (dg.pa == p || dg.pb == p)
        res.push_back(dg);
    }
  }
  return res;
}

vector<edge*>
Graph::edgesAt(const edge& targetEdge)
{
  vector<edge*> res;
  for (auto& fig : shapes) {
    for (auto& dg : fig.edges) {
      if (dg == targetEdge)
        res.push_back(&dg);
    }
  }
  return res;
}

const vector<shape>
Graph::facetsOnEdgePoints(const edge& targetEdge) const
{
  unordered_set<shape> res;

  for (auto& fig : shapes) {
    for (auto& dg : fig.edges) {
      if (dg.pa == targetEdge.pa || dg.pa == targetEdge.pb || dg.pb == targetEdge.pa || dg.pb == targetEdge.pb) {
        res.insert(fig);
        break;
      }
    }
  }

  return vector<shape>(begin(res), end(res));
}

const vector<shape>
Graph::facetsAt(const vertex& targetPoint) const
{
  vector<shape> res;
  for (auto& fig : shapes) {
    for (auto& dg : fig.edges) {
      if (dg.pa == targetPoint || dg.pb == targetPoint) {
        res.push_back(fig);
        break;
      }
    }
  }
  return res;
}

Graph
Graph::copy_flip(const edge& over) const
{
  Graph newGraph = *this;
  vector<shape> mirroredShapes;

  // for (auto& fig : facetsOnEdgePoints(over)) {
  for (auto& fig : shapes) {
    mirroredShapes.push_back(mirror(fig, over));
  }

  newGraph.shapes.insert(end(newGraph.shapes), begin(mirroredShapes), end(mirroredShapes));

  for (auto& fig : mirroredShapes) {
    for (auto& dg : fig.edges) {
      auto neighborEdges = newGraph.edgesAt(dg);
      if (neighborEdges.size() == 2) {
        neighborEdges[0]->neighbor = neighborEdges[1]->owner;
        neighborEdges[1]->neighbor = neighborEdges[0]->owner;
      }
    }
  }

  // Graph newGraph = *this;
  //
  // const shape* ownerShapeInOldGraph = over.owner;
  //
  // auto* ownerShapeInNewGraph = newGraph.at(*this, ownerShapeInOldGraph);
  // auto newfig = mirror(*ownerShapeInNewGraph, over);
  //
  // newGraph.shapes.push_back(move(newfig));
  // auto& newShapeInNewGraph = newGraph.shapes.back();
  //
  // ownerShapeInNewGraph = newGraph.at(*this, ownerShapeInOldGraph);
  //
  // auto& thisEdgeInOwnerShape = ownerShapeInNewGraph->find(over);
  // thisEdgeInOwnerShape.neighbor = &newShapeInNewGraph;
  //
  // auto& thisEdgeInNewShape = newShapeInNewGraph.find(over);
  // thisEdgeInNewShape.neighbor = ownerShapeInNewGraph;

  return newGraph;
}

Origami::Origami(Graph&& other) : graph(other)
{
  outline = graph.outline();
}

Origami
Origami::from(const polys& outline, const poly& skeleton)
{
  vector<shape> facets;

  if (outline.size() == 1 && skeleton.size() == 4) {
    for (auto fig : outline) {
      shape x;
      x.body = fig;
      x.dest = fig;

      vector<edge> edges;
      for (size_t i = 0; i < fig.size(); i++) {
        edge g = { fig[i], fig[(i + 1) % fig.size()] };
        edges.push_back(g);
      }
      x.edges = edges;

      facets.push_back(x);
    }
  }

  for (auto& fig : facets) {
    for (auto& g : fig.edges)
      g.owner = &fig;
  }

  Origami res;
  res.outline = outline;
  res.graph = Graph(move(facets));
  return res;
}

Origami
Origami::unfold(const edge& g) const {
  auto newgraph = graph.copy_flip(g);
  return Origami(move(newgraph));
}

}
