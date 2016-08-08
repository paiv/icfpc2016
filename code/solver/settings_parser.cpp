
typedef struct settings_t
{
  list<string> files;
  bool print_usage_and_exit;
} settings;


static settings
parse_settings(int argc, char* argv[])
{
  int c;
  int errflg = 0;
  settings Settings = {};

  while ((c = getopt(argc, argv, "f:t:m:c:p:h")) != -1)
  {
    switch (c)
    {
      case 'f':
        if (optarg)
          Settings.files.push_back(optarg);
        break;

      case '?':
      case 'h':
        errflg++;
        break;
    }
  }

  if (errflg > 0 || Settings.files.size() == 0)
  {
    Settings.print_usage_and_exit = true;
  }

  return Settings;
}
