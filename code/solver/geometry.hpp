#pragma once

namespace paiv {

static inline r64
distance2(const vertex& a, const vertex& b)
{
  return (b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y);
}


static vertex
mirror(const vertex& v, const edge& over) {
  auto dx = over.pb.x - over.pa.x;
  auto dy = over.pb.y - over.pa.y;

  auto a = (dx*dx - dy*dy) / (dx*dx + dy*dy);
  auto b = 2 * dx*dy / (dx*dx + dy*dy);

  auto x = a * (v.x - over.pa.x) + b * (v.y - over.pa.y) + over.pa.x;
  auto y = b * (v.x - over.pa.x) - a * (v.y - over.pa.y) + over.pa.y;

  return { x, y };
}

static shape
mirror(const shape& fig, const edge& over) {
  shape newfig = shape(fig, false);
  for (auto& g : newfig.edges) {
    g.pa = mirror(g.pa, over);
    g.pb = mirror(g.pb, over);
  }
  for (auto& v : newfig.body) {
    v = mirror(v, over);
  }
  return newfig;
}

u8
same_line(const vertex& a, const vertex& b, const vertex& c)
{
  return near((b.y - a.y) * (c.x - b.x), (c.y - b.y) * (b.x - a.x), 0.00001);
}

u8
is_clockwise(const vertex& a, const vertex& b, const vertex& c) {
  auto t = (b.x - a.x)*(b.y + a.y) + (c.x - b.x)*(c.y + b.y) + (a.x - c.x)*(a.y + c.y);
  return t >= 0;
}

}
