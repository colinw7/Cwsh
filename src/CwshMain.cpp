#include <CwshLib.h>

int
main(int argc, char **argv)
{
  auto *cwsh = new Cwsh::App;

  try {
    cwsh->init(argc, argv);

    cwsh->mainLoop();
  }
  catch (struct Cwsh::Err *err) {
    err->print();
  }
  catch (...) {
    std::cerr << "Unhandled exception\n";
  }

  return 0;
}
