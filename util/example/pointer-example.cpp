#include <iostream>
#include <cassert>
#include "pointer.h"

using namespace util;

int main(int argc, char* argv[]) {
  DensePointer p0;
  assert(p0.pointer() == nullptr);
  assert(p0.remain() == 0);
  DensePointer p1(&p0, 1);
  DensePointer p2(p1);
  assert(p1.pointer() == &p0 && p1.remain() == 1);
  assert(p1 == p2);

  AtomicDense atom_p0;
  assert(atom_p0.load() == p0);
  atom_p0.store(p1);
  assert(atom_p0.load() == p2);
  atom_p0.exchange(p0);
  assert(atom_p0.load() == p0);
  atom_p0.compare_exchange_strong(p0, p1);
  assert(atom_p0.load() == p1);

  return 0;
}