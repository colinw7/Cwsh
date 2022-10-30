#include <CBash.h>
#include <CBashErr.h>
#include <iostream>

int
main(int argc, char **argv)
{
  auto *bash = new CBash::App;

  try {
    bash->init(argc, argv);

    bash->mainLoop();
  }
  catch (struct CBashErr *err) {
    err->print();
  }
  catch (...) {
    std::cerr << "Unhandled exception\n";
  }

  return 0;
}
