
namespace paiv {

static bool
is_unit_square(const poly& shape)
{
  return near(distance2(shape[0], shape[1]), 1, 0.00001)
    && near(distance2(shape[1], shape[2]), 1, 0.00001)
    && near(distance2(shape[2], shape[3]), 1, 0.00001)
    && near(distance2(shape[3], shape[0]), 1, 0.00001)
  ;
}

u8
search_state::isgoal() const
{
  if (origami.outline.size() == 1) {
    auto shape = origami.outline.front();
    return is_unit_square(shape);
  }
  return false;
}

u8
search_state::isterminal() const
{
  return origami.isempty()
    || isgoal();
}


static poly
transform_to_initial(const poly& shape, const polys& outline)
{
  static const poly InitialSquare = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
  static const matrix3d us = {{ {0, 0, 1}, {1, 0, 1}, {1, 1, 1} }};

  auto& v = outline.front();
  matrix3d ys = {{ {v[0].x, v[0].y, 1}, {v[1].x, v[1].y, 1}, {v[2].x, v[2].y, 1} }};

  matrix3d transform = us.mul(ys.inverse());

  poly res;
  for (auto p : shape) {
    vector3d v = {{ p.x, p.y, 1 }};
    vector3d u = transform.mul(v);
    auto r = 1e6f;
    res.push_back({ round(u.v.x * r) / r, round(u.v.y * r) / r });
  }

  return res;
}

solution
search_state::get_solution() const
{
  solution sol = {};
  if (terminal) {
    auto vs = origami.graph.vertices();
    auto mapping = unique_mapping(vs);
    sol.vertices = mapping(vs);
    sol.facets = origami.graph.facets_as_indices(sol.vertices);
    sol.vertices = transform_to_initial(sol.vertices, origami.outline);
    sol.destination = mapping(origami.graph.vertices(true));
  }
  return sol;
}

vector<search_state>
search_state::children() const {
  vector<search_state> res;
  if (terminal) return res;

  for (auto g : origami.graph.edges()) {

    auto newo = origami.unfold(g);
    if (newo.isempty()) continue;

    search_state next = {};
    next.origami = move(newo);
    next.terminal = next.isterminal();
    res.push_back(move(next));
  }

  return res;
}


typedef queue<search_state> fringe;
typedef unordered_set<search_state> history;

static list<solution>
solve_problem(problem& prob)
{
  list<solution> results;

  fringe fringe;
  history visited;

  search_state state = {};
  state.origami = Origami::from(prob.outline, prob.skeleton);
  state.terminal = state.isterminal();

  fringe.push(state);

  while (fringe.size() > 0) {
    auto& state = fringe.front();

    // ~~
    // clog << state << endl;

    if (visited.find(state) == end(visited)) {
      visited.insert(state);

      if (state.isgoal()) {
        auto sol = state.get_solution();
        if (sol.facets.size() > 0) {
          results.push_back(sol);
          break;
        }
      }

      auto children = state.children();
      fringe.pop();
      for (auto& child : children) {
        fringe.push(child);
      }
    }
    else {
      fringe.pop();
    }
  }

  return results;
}


}
