#include <iostream>
#include "debug.h"

int main(int argc, char* argv[]) {
  std::cout << "-- log example" << std::endl;
  DEBUG_LOG("normal log: %i", 0);
  DEBUG_COND_LOG(argc == 1, "conditional log: %i", 1);
  DEBUG_COND_ERROR(false, "conditional error: %i", 2);
  DEBUG_ERROR("normal error: %i", 3);
}