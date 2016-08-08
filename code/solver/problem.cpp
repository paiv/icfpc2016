
namespace paiv {


static vertex
read_vertex(ifstream& fin) {
  char sep;
  fraction x, y;
  fin >> x;
  fin >> sep;
  fin >> y;
  return {x, y};
}

static poly
read_polygon(ifstream& fin) {
  u32 verticesCount = 0;
  fin >> verticesCount;
  poly res;
  for (size_t i = 0; i < verticesCount; i++) {
    auto v = read_vertex(fin);
    res.push_back(v);
  }
  return res;
}

static problem
read_problem(u32 id, string& fn)
{
  problem p = { id };

  u32 polygonsCount = 0;

  ifstream fin(fn, ifstream::binary);

  fin >> polygonsCount;
  for (size_t i = 0; i < polygonsCount; i++) {
    auto polygon = read_polygon(fin);
    p.outline.push_back(polygon);
  }

  p.skeleton = read_polygon(fin);

  return p;
}

}
