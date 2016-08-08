

static s32
run_controller(list<string>& files)
{
  for (string fn : files)
  {
    problem p = read_problem(0, fn);
    list<solution> solved = solve_problem(p);
    for (solution& r : solved)
      cout << r;
    if (solved.size() == 0)
      return 1;
  }
  return 0;
}
