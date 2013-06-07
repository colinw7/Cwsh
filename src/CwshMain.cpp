#include <CwshLib.h>

int
main(int argc, char **argv)
{
  Cwsh *cwsh = new Cwsh;

  try {
    cwsh->init(argc, argv);

    cwsh->mainLoop();
  }
  catch (struct CwshErr *err) {
    err->print();
  }
  catch (...) {
    std::cerr << "Unhandled exception" << std::endl;
  }

  return 0;
}
