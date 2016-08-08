#!/usr/bin/env python

def generate3():
  vs = [(0,0), (4,0), (10,0)]
  ds = [(2,2), (6,2), (6,8)]
  facets = []

  for i in range(1, 11):
    vs.extend([ (0,i), ( 4 + (i % 2), i), (10,i) ])

    if (i % 2) == 1:
      ds.extend([ (2,3), (7,3), (7,8) ])
    else:
      ds.extend([ (2,2), (6,2), (6,8) ])

    x = (i - 1) * 3
    facets.extend([ [0+x, 3+x, 4+x, 1+x], [1+x, 4+x, 5+x, 2+x] ])

  def pv(v):
    return '%d/10,%d/10' % (v[0], v[1])
  def pf(v):
    return '%d %s' % (len(v), ' '.join([str(x) for x in v]))

  print('%d\n%s\n%d\n%s\n%s\n' % (len(vs), '\n'.join([pv(x) for x in vs]),
    len(facets), '\n'.join([pf(x) for x in facets]),
    '\n'.join([pv(x) for x in ds])))


def generate4():
  vs = [(0,0), (3,0), (7,0)]
  ds = [(1,2), (4,2), (4,6)]
  facets = []

  for i in range(1, 8):
    vs.extend([ (0,i), ( 3 + (i % 2), i), (7,i) ])

    if (i % 2) == 1:
      ds.extend([ (1,3), (5,3), (5,6) ])
    else:
      ds.extend([ (1,2), (4,2), (4,6) ])

    x = (i - 1) * 3
    facets.extend([ [0+x, 3+x, 4+x, 1+x], [1+x, 4+x, 5+x, 2+x] ])

  def pv(v):
    return '%d/7,%d/7' % (v[0], v[1])
  def pf(v):
    return '%d %s' % (len(v), ' '.join([str(x) for x in v]))

  print('%d\n%s\n%d\n%s\n%s\n' % (len(vs), '\n'.join([pv(x) for x in vs]),
    len(facets), '\n'.join([pf(x) for x in facets]),
    '\n'.join([pv(x) for x in ds])))


def generate5():
  vs = [(0,0), (3,0), (7,0)]
  ds = [(1,2), (4,2), (4,6)]
  facets = []

  for i in range(1, 8):
    vs.extend([ (0,i), ( 3 + (i % 2), i), (7,i) ])

    if (i % 2) == 1:
      ds.extend([ (1,3), (5,3), (5,6) ])
    else:
      ds.extend([ (1,2), (4,2), (4,6) ])

    x = (i - 1) * 3
    facets.extend([ [0+x, 3+x, 4+x, 1+x], [1+x, 4+x, 5+x, 2+x] ])

  def pz(v):
    return '%d/7,%d/7' % (v[0], v[1])
  def xx(x):
    return x + 39567675487036172071
  def pv(v):
    return '%d/7,%d/7' % (xx(v[0]), xx(v[1]))
  def pf(v):
    return '%d %s' % (len(v), ' '.join([str(x) for x in v]))

  print('%d\n%s\n%d\n%s\n%s\n' % (len(vs), '\n'.join([pz(x) for x in vs]),
    len(facets), '\n'.join([pf(x) for x in facets]),
    '\n'.join([pv(x) for x in ds])))


def generate6():
  vs = [(0,0), (3,0), (7,0)]
  ds = [(1,2), (4,2), (4,6)]
  facets = []

  for i in range(1, 8):
    vs.extend([ (0,i), ( 3 + (i % 2), i), (7,i) ])

    if (i % 2) == 1:
      ds.extend([ (1,3), (5,3), (5,6) ])
    else:
      ds.extend([ (1,2), (4,2), (4,6) ])

    x = (i - 1) * 3
    facets.extend([ [0+x, 3+x, 4+x, 1+x], [1+x, 4+x, 5+x, 2+x] ])

  def xx(x):
    return x - 39567675487036172071
  def yy(x):
    return x * 40016471545131252108

  def pz(v):
    return '%d/%d,%d/%d' % (yy(v[0]), yy(7), yy(v[1]), yy(7))
  def pv(v):
    return '%d/7,%d/7' % (xx(v[0]), xx(v[1]))
  def pf(v):
    return '%d %s' % (len(v), ' '.join([str(x) for x in v]))

  print('%d\n%s\n%d\n%s\n%s\n' % (len(vs), '\n'.join([pz(x) for x in vs]),
    len(facets), '\n'.join([pf(x) for x in facets]),
    '\n'.join([pv(x) for x in ds])))


def strip_template(W, A=0, B=1):
  vs = [(0,0), (W//2,0), (W,0)]
  ds = [(1,2), (W//2+1,2), (W//2+1, W//2+3)]
  facets = []

  for i in range(1, W+1):
    vs.extend([ (0,i), ( W//2 + (i % 2), i), (W,i) ])

    if (i % 2) == 1:
      ds.extend([ (1,3), (W//2+2,3), (W//2+2, W//2+3) ])
    else:
      ds.extend([ (1,2), (W//2+1,2), (W//2+1, W//2+3) ])

    x = (i - 1) * 3
    facets.extend([ [0+x, 3+x, 4+x, 1+x], [1+x, 4+x, 5+x, 2+x] ])

  def xx(x):
    return x + A
  def yy(x):
    return x * B

  def pz(v):
    return '%d/%d,%d/%d' % (yy(v[0]), yy(W), yy(v[1]), yy(W))
  def pv(v):
    return '%d/%d,%d/%d' % (xx(v[0]), W, xx(v[1]), W)
  def pf(v):
    return '%d %s' % (len(v), ' '.join([str(x) for x in v]))

  return '%d\n%s\n%d\n%s\n%s' % (len(vs), '\n'.join([pz(x) for x in vs]),
    len(facets), '\n'.join([pf(x) for x in facets]),
    '\n'.join([pv(x) for x in ds]))

def generate_from_template(w, rate=10):
  n = 10000
  a = '-4200945621317508869573122807639138161480544015479627550171233257932319831'
  b = '9124468192981565484317339560234413347432566341725731120048679801263241760'
  if w > 25:
    a = a[:(60-w)]
    b = b[:(60-w)]
  a = int(a)
  b = int(b)
  while n > 5000:
    prob = strip_template(w, a, b)
    n = len(prob)
    a //= (rate - 3)
    b //= (rate + 1)
  return prob


def generate7():
  print(strip_template(59))

def generate8():
  print(strip_template(3, A=-4200945621317508869573122807639138161480544015479627550171233257932319831,
    B=9124468192981565484317339560234413347432566341725731120048679801263241760))

def generate9():
  print(generate_from_template(21))

def generate10():
  print(generate_from_template(19))

def generate11():
  print(generate_from_template(17))

def generate12():
  print(generate_from_template(15))

def generate13():
  print(generate_from_template(13))

def generate14():
  print(generate_from_template(23))

def generate15():
  print(generate_from_template(25))

def generate16():
  print(generate_from_template(27))

def generate17():
  print(generate_from_template(29))

def generate18():
  print(generate_from_template(31))

def generate19():
  print(generate_from_template(33))

def generate20():
  print(generate_from_template(35))

def generate21():
  print(generate_from_template(37))

def generate22():
  print(generate_from_template(39))

def generate23():
  print(generate_from_template(41))

def generate24():
  print(generate_from_template(43))

def generate25():
  print(generate_from_template(45))

def generate26():
  print(generate_from_template(47))

def generate27():
  print(strip_template(49))

def generate28():
  print(strip_template(51))

def generate29():
  print(strip_template(53))

def generate30():
  print(strip_template(55))

generate30()
