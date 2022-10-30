#include <CwshLib.h>

int
main(int argc, char **argv)
{
  auto *cwsh = new Cwsh;

  try {
    cwsh->init(argc, argv);

    cwsh->mainLoop();
  }
  catch (struct CwshErr *err) {
    err->print();
  }
  catch (...) {
    std::cerr << "Unhandled exception\n";
  }

  return 0;
}
