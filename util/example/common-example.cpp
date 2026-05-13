#include <iostream>
#include "common.h"

using namespace util;

int main(int argc, char* argv[]) {
  int num = 0xFF;
  std::cout << "-- common example" << std::endl;

  std::cout << "-- popcount: " << popcount(num) << std::endl;
  std::cout << "-- countl_zero: " << countl_zero(num) << std::endl;
  std::cout << "-- countl_one: " << countl_one(num) << std::endl;
  std::cout << "-- countr_zero: " << countr_zero(num) << std::endl;
  std::cout << "-- countr_one: " << countr_one(num) << std::endl;
  std::cout << "-- index_least1: " << index_least1(num) << std::endl;
  std::cout << "-- index_least0: " << index_least0(num) << std::endl;
  std::cout << "-- index_most1: " << index_most1(num) << std::endl;
  std::cout << "-- index_most0: " << index_most0(num) << std::endl;
  std::cout << "-- byte_swap: " << byte_swap(num) << std::endl;
  std::cout << "-- parity: " << parity(num) << std::endl;

  std::cout << "-- roundup: " << roundup(num, 256) << std::endl;
  std::cout << "-- rounddown: " << rounddown(num, 256) << std::endl;

  std::cout << "-- logical cpu num: " << ncpus_online() << std::endl;

  bool ret = 0;
  __m128i vec;
  ((uint64_t*) &vec)[0] = 1;
  ((uint64_t*) &vec)[1] = 2;
  __m128i expected;
  ((uint64_t*) &expected)[0] = 1;
  ((uint64_t*) &expected)[1] = 2;
  __m128i desired;
  ((uint64_t*) &desired)[0] = 3;
  ((uint64_t*) &desired)[1] = 4;

  std::cout << ret << " " << ((uint64_t*) &vec)[0] << " " << ((uint64_t*) &vec)[1] << " "
            << ((uint64_t*) &expected)[0] << " " << ((uint64_t*) &expected)[1] << " "
            << ((uint64_t*) &desired)[0] << " " << ((uint64_t*) &desired)[1] << std::endl;

  ret = cas2(&vec, &expected, desired);
  std::cout << ret << " " << ((uint64_t*) &vec)[0] << " " << ((uint64_t*) &vec)[1] << " "
            << ((uint64_t*) &expected)[0] << " " << ((uint64_t*) &expected)[1] << " "
            << ((uint64_t*) &desired)[0] << " " << ((uint64_t*) &desired)[1] << std::endl;

  ret = cas2(&vec, (uint64_t*) &desired, (uint64_t*) &desired + 1,
             ((uint64_t*) &expected)[0], ((uint64_t*) &expected)[1]);
  std::cout << ret << " " << ((uint64_t*) &vec)[0] << " " << ((uint64_t*) &vec)[1] << " "
            << ((uint64_t*) &expected)[0] << " " << ((uint64_t*) &expected)[1] << " "
            << ((uint64_t*) &desired)[0] << " " << ((uint64_t*) &desired)[1] << std::endl;
  return 0;
}