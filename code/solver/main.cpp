#include "unity.cpp"

int main(int argc, char* argv[])
{
  settings Settings = parse_settings(argc, argv);

  if (Settings.print_usage_and_exit)
  {
    cerr << "usage: solve [OPTIONS]\n"
      "	-f	FILENAME	Problem file\n"
    << endl;
    return 2;
  }

  return run_controller(Settings.files);
}
