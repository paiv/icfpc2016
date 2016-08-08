#!/usr/bin/env python
from __future__ import print_function, division
import json
import os
import re
import svgwrite
import sys
import time
import unirest
from datetime import datetime, timedelta
from fractions import Fraction
from xml.etree import ElementTree


class Problem:
  def __init__(self, spec, preprocessed=False):
    self.spec = spec
    self.preprocessed = preprocessed
    p = ProblemParser(spec, preprocessed=preprocessed)
    self.figure = p.parse_figure()
    self.skeleton = p.parse_skeleton()
    self.mean = p.parse_prop('mean:', 0.0)
    self.scale = p.parse_prop('scale:', 1.0)

  def to_svg(self):
    def frac(f):
      x = f if isinstance(f, float) else f[0] / f[1]
      return (x + 1) * 200
    def poly(p, color = 'magenta', width = 2):
      return svg.polygon([(frac(x), frac(y)) for (x,y) in p], stroke=color, stroke_width=width, fill='white')
    def line(l, color = 'green'):
      return svg.line(start=(frac(l[0][0]), frac(l[0][1])), end=(frac(l[1][0]), frac(l[1][1])), stroke=color, stroke_width=1)

    svg = svgwrite.Drawing()
    group = svg.add(svg.g())
    group.add(poly([((0,1),(0,1)),((1,1),(0,1)),((1,1),(1,1)),((0,1),(1,1))], color='lightgray', width=1))
    for p in self.figure:
      group.add(poly(p))
    for l in self.skeleton:
      group.add(line(l))
    return ElementTree.tostring(svg.get_xml())

  def to_str(self):
    def frac(p):
      if isinstance(p, float):
        return str(p)
      if p[1] == 1:
        return str(p[0])
      return '/'.join([str(x) for x in p])
    def vert(p):
      return ','.join([frac(x) for x in p])
    def line(p):
      return ' '.join([vert(x) for x in p])
    def poly(p):
      return '%d\n%s' % (len(p), '\n'.join([vert(x) for x in p]))
    return '%d\n%s\n%d\n%s\nmean: %d\nscale: %d' % (len(self.figure), '\n'.join([poly(x) for x in self.figure]),
      len(self.skeleton), '\n'.join([line(x) for x in self.skeleton]),
      self.mean, self.scale)

  def vertices(self):
    return [v for shape in self.figure for v in shape] + [v for line in self.skeleton for v in line]


def shift_frac(frac, mean):
  return (int(frac[0] + mean * frac[1]), frac[1])
def scale_frac(frac, scale):
  return (frac[0] * scale, frac[1] * scale)
def rescale_frac(frac, scale):
  if scale != 1:
    k = scale / frac[1]
    return (int(frac[0] * k), scale)
  return frac
def frac_to_float(frac):
  return frac[0] / frac[1]
def float_to_frac(f):
  frac = Fraction(f)
  return (frac.numerator, frac.denominator)
def mean(a):
  return sum(a) // len(a)

class ProblemPreprocessor:

  def get_mean(self, prob):
    def frac(x,y):
      return x / y
    vals = [frac(*x) for shape in prob.figure for v in shape for x in v]
    return int(mean(vals))

  def get_mean_denom(self, prob):
    vals = [y for shape in prob.figure for v in shape for (x,y) in v]
    x = int(mean(vals))
    return x if vals.count(x) == len(vals) else 1

  def process_vertex(self, vertex, fracop):
    return (fracop(vertex[0]), fracop(vertex[1]))
  def process_poly(self, poly, fracop):
    return [self.process_vertex(x, fracop) for x in poly]
  def process_polys(self, polys, fracop):
    return [self.process_poly(x, fracop) for x in polys]
  def process_line(self, line, fracop):
    return (self.process_vertex(line[0], fracop), self.process_vertex(line[1], fracop))
  def process_lines(self, lines, fracop):
    return [self.process_line(x, fracop) for x in lines]

  def convert(self, prob):

    res = Problem(prob.spec)
    mean = self.get_mean(prob)
    scale = self.get_mean_denom(prob)

    res.figure = self.process_polys(res.figure, lambda x: shift_frac(x, -mean))
    res.skeleton = self.process_lines(res.skeleton, lambda x: shift_frac(x, -mean))
    res.mean = mean

    res.figure = self.process_polys(res.figure, lambda x: scale_frac(x, 1/scale))
    res.skeleton = self.process_lines(res.skeleton, lambda x: scale_frac(x, 1/scale))
    res.scale = scale

    res.figure = self.process_polys(res.figure, frac_to_float)
    res.skeleton = self.process_polys(res.skeleton, frac_to_float)

    res.preprocessed = True
    return res

  def bestmatch_map(self, src, vertices):
    def frac(f):
      return f[0] / f[1]
    def dsq(a, b):
      d = frac(b) - frac(a)
      return d*d
    def dist2(a, b):
      return dsq(a[0], b[0]) + dsq(a[1], b[1])
    def nearest(x, vs):
      bestValue = vs[0]
      bestDist = float('inf')
      for v in vs:
        d = dist2(v, x)
        if d < bestDist:
          bestDist = d
          bestValue = v
      return bestValue
    return [nearest(v, vertices) for v in src]

  def apply(self, solu, prob):
    convertedProblem = self.convert(prob)
    x = solu.destination
    x = self.process_poly(x, float_to_frac)
    x = self.process_poly(x, lambda x: rescale_frac(x, convertedProblem.scale))
    x = self.process_poly(x, lambda x: shift_frac(x, convertedProblem.mean))
    solu.destination = self.bestmatch_map(x, prob.vertices())
    solu.vertices = self.process_poly(solu.vertices, float_to_frac)
    return solu


class ProblemParser:
  def __init__(self, spec, preprocessed=False):
    self.spec = spec
    self.preprocessed = preprocessed
    self.lines = spec.split('\n')
    self.line = 0

  def read_line(self):
    s = self.lines[self.line]
    self.line += 1
    return s

  def parse_int(self):
    return int(self.read_line())

  def parse_fraction(self, s):
    ss = s.split('/')
    num = int(ss[0])
    den = 1
    if len(ss) > 1:
      den = int(ss[1])
    return (num, den)

  def parse_vertex(self, s):
    sx,sy = s.split(',')
    if self.preprocessed:
      return (float(sx), float(sy))
    x = self.parse_fraction(sx)
    y = self.parse_fraction(sy)
    return (x,y)

  def parse_poly(self, n=None):
    res = []
    if n == None:
      n = self.parse_int()
    for i in range(n):
      s = self.read_line()
      res.append(self.parse_vertex(s))
    return res

  def parse_figure(self):
    polygons = []
    n = self.parse_int()
    for i in range(n):
      polygons.append(self.parse_poly())
    return polygons

  def parse_line(self):
    s = self.read_line()
    ss = s.split(' ')
    p1 = self.parse_vertex(ss[0])
    p2 = self.parse_vertex(ss[1])
    return (p1, p2)

  def parse_skeleton(self):
    res = []
    n = self.parse_int()
    for i in range(n):
      res.append(self.parse_line())
    return res

  def parse_prop(self, name, default):
    rx = re.compile(re.escape(name) + r'\s*([-0-9\.]+)')
    m = rx.search(self.spec)
    if m:
      return int(m.group(1))
    return default


class Solution:
  def __init__(self, spec):
    self.spec = spec
    p = SolutionParser(spec)
    self.vertices = p.parse_poly()
    self.facet_lines = p.save_n_lines()
    self.facets = p.parse_facets(self.vertices, self.facet_lines)
    self.destination = p.parse_poly(n=len(self.vertices))

  def to_str(self):
    def frac(p):
      if isinstance(p, float):
        return str(p)
      if p[1] == 1:
        return str(p[0])
      return '/'.join([str(x) for x in p])
    def vert(p):
      return ','.join([frac(x) for x in p])
    def line(p):
      return ' '.join([vert(x) for x in p])
    def poly(p):
      return '%d\n%s' % (len(p), '\n'.join([vert(x) for x in p]))
    return '%s\n%d\n%s\n%s' % (poly(self.vertices),
      len(self.facet_lines), '\n'.join(self.facet_lines),
      '\n'.join([vert(x) for x in self.destination]))

  def to_svg(self):
    def frac(f):
      x = f if isinstance(f, float) else f[0] / f[1]
      return (x + 1) * 200
    def point(p, color='lightgray', r=3):
      return svg.circle(center=(frac(p[0]), frac(p[1])), r=r, stroke=color, stroke_width=1, fill_opacity=0)
    def poly(p, color = 'magenta', width = 2):
      group = svg.add(svg.g())
      group.add(svg.polygon([(frac(x), frac(y)) for (x,y) in p], stroke=color, stroke_width=width, fill='white'))
      for i,x in enumerate(p):
        group.add(point(x, color=white(i / len(p))))
      return group
    def line(l, color = 'green'):
      return svg.line(start=(frac(l[0][0]), frac(l[0][1])), end=(frac(l[1][0]), frac(l[1][1])), stroke=color, stroke_width=1)
    def white(x):
      c = x * 256
      return 'rgb(%d,%d,%d)' % (c,c,c)

    svg = svgwrite.Drawing()
    group = svg.add(svg.g())
    group.add(poly([((0,1),(0,1)),((1,1),(0,1)),((1,1),(1,1)),((0,1),(1,1))], color='lightgray', width=1))
    for i,p in enumerate(self.vertices):
      group.add(point(p))
    for shape in self.facets:
      group.add(poly(shape))
    return ElementTree.tostring(svg.get_xml())


class SolutionParser(ProblemParser):
  def __init__(self, spec):
    ProblemParser.__init__(self, spec, preprocessed=True)

  def save_n_lines(self):
    res = []
    n = self.parse_int()
    for i in range(n):
      s = self.read_line()
      res.append(s)
    return res

  def parse_facets(self, vertices, lines):
    return [[vertices[int(x)] for x in s.split(' ')[1:]] for s in lines]



class ApiError(Exception):
  pass

class ApiClient:

    api_endpoint = 'http://2016sv.icfpcontest.org/api'
    api_key = os.environ['API_KEY']

    timestamp = None

    def endpoint(self, x):
        return '%s/%s' % (self.api_endpoint, x)

    def delay(self):
        if self.timestamp is not None:
            x = 1.0 - (datetime.now() - self.timestamp).total_seconds() + 0.1
            if x > 0:
                time.sleep(x)
        self.timestamp = datetime.now()

    def request(self, method, name, params = {}):
        self.delay()
        headers = { 'X-API-Key': self.api_key }
        m = getattr(unirest, method)
        r = m(self.endpoint(name), headers=headers, params=params)
        if r.code != 200:
            raise ApiError(r.code, str(r.headers), r.raw_body)
        return r.body

    def get(self, name):
        return self.request('get', name)

    def post(self, name, params):
        return self.request('post', name, params=params)

    def hello(self):
        return self.get('hello')

    def blob(self, hashcode):
        return self.get('blob/%s' % (hashcode))

    def snapshots(self):
        return self.get('snapshot/list')['snapshots']

    def latest_snapshot(self):
        r = self.snapshots()
        latest = sorted(r, key=lambda x: x['snapshot_time'], reverse=True)[0]
        sn = self.blob(latest['snapshot_hash'])
        with open('temp-snapshot.json', 'wb') as f:
          json.dump(sn, f)
        return sn

    def dump_problems(self, where='problems'):
        if not os.path.exists(where):
            os.makedirs(where)

        problems = self.latest_snapshot()['problems']
        for prob in problems:
            pid = prob['problem_id']

            fname = '%s/%06d-spec.txt' % (where, pid)
            fmeta = '%s/%06d-meta.json' % (where, pid)
            fsvg = '%s/%06d.svg' % (where, pid)

            try:
              if not os.path.exists(fname):
                  print(fname)
                  prob.pop('ranking')
                  with open(fmeta, 'wb') as f:
                      json.dump(prob, f)
                  spec = self.blob(prob['problem_spec_hash'])
                  with open(fname, 'wb') as f:
                      f.write(spec)
              # if not os.path.exists(fsvg):
              #     with open(fname, 'rb') as f:
              #         spec = f.read()
              #     p = Problem(spec)
              #     pp = ProblemPreprocessor()
              #     p = pp.convert(p)
              #     with open(fsvg, 'wb') as f:
              #         f.write(p.to_svg())
            except (KeyboardInterrupt, SystemExit):
              raise
            except:
              print(sys.exc_info()[0])

    def submit(self, pid, spec):
        return self.post('solution/submit', {'problem_id': pid, 'solution_spec': spec})

    def submit_problem(self, pid, spec):
        # hour = 1470441600 + (pid - 1) * 3600
        hour = (int(time.time()) // 3600 + 1 + pid) * 3600
        return self.post('problem/submit', {'publish_time': hour, 'solution_spec': spec})

    def my_problems(self):
      problems = self.latest_snapshot()['problems']
      return [p for p in problems if p['owner'] == '116']

if __name__ == '__main__':
    usage = 'usage: api hello|status|problems|submit|parse|draw|preproc|postproc|draw-solution|submit-problem|my-problems'
    api = ApiClient()

    if len(sys.argv) > 1:
        cmd = sys.argv[1]
        if cmd == 'hello':
            r = api.hello()
            print(json.dumps(r))
        elif cmd == 'status':
            r = api.latest_snapshot()
            print(json.dumps(r))
        elif cmd == 'problems':
            where = sys.argv[2] if len(sys.argv) > 2 else 'problems'
            api.dump_problems(where)
        elif cmd == 'submit':
            if len(sys.argv) < 4:
              print('usage: api submit pid specfile')
            else:
              pid = int(sys.argv[2])
              with open(sys.argv[3], 'rb') as f:
                  spec = f.read()
              r = api.submit(pid, spec)
              print(json.dumps(r))
        elif cmd == 'submit-problem':
            if len(sys.argv) < 3:
              # pid starts from 1 on 1470441600; realative hour from now
              print('usage: api submit-problem specfile [hour]')
            else:
              pid = int(sys.argv[3]) if len(sys.argv) > 3 else 0
              with open(sys.argv[2], 'rb') as f:
                  spec = f.read()
              r = api.submit_problem(pid, spec)
              print(json.dumps(r))
        elif cmd == 'my-problems':
              r = api.my_problems()
              print(json.dumps(r))
        elif cmd == 'parse':
            if len(sys.argv) < 3:
              print('usage: api parse problemfile')
            else:
              with open(sys.argv[2]) as f:
                spec = f.read()
              prob = Problem(spec, preprocessed=True)
              print(prob.to_str())
        elif cmd == 'draw':
            if len(sys.argv) < 3:
              print('usage: api draw specfile')
            else:
              with open(sys.argv[2]) as f:
                spec = f.read()
              prob = Problem(spec, preprocessed=True)
              print(prob.to_svg())
        elif cmd == 'preproc':
            if len(sys.argv) < 3:
              print('usage: api preproc problemfile')
            else:
              with open(sys.argv[2]) as f:
                spec = f.read()
              prob = Problem(spec)
              pp = ProblemPreprocessor()
              prob = pp.convert(prob)
              print(prob.to_str())
        elif cmd == 'postproc':
            if len(sys.argv) < 4:
              print('usage: api postproc problemfile solutionfile')
            else:
              with open(sys.argv[2]) as f:
                pspec = f.read()
              with (sys.stdin if sys.argv[3] == '-' else open(sys.argv[3])) as f:
                sspec = f.read()
              prob = Problem(pspec)
              sol = Solution(sspec)
              pp = ProblemPreprocessor()
              sol = pp.apply(sol, prob)
              print(sol.to_str())
        elif cmd.startswith('draw-s'):
            if len(sys.argv) < 3:
              print('usage: api draw-solution solution')
            else:
              with open(sys.argv[2]) as f:
                spec = f.read()
              solu = Solution(spec)
              print(solu.to_svg())
        else:
            print(usage)
    else:
        print(usage)
