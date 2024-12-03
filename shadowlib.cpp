#include <map>
#include <tuple>
#include <iostream>


extern "C" void __shadowlib_insert(void *ptr, int value, char *string);
extern "C" int __shadowlib_get(void *ptr);
extern "C" void __shadowlib_print();


std::map<void *, std::tuple<int, char *>> shadowmem;
//std::map<void *, int> shadowmem;

void __shadowlib_insert(void *ptr, int value, char *string)
{
  shadowmem[ptr] = std::make_tuple(value, string);
}

int __shadowlib_get(void *ptr)
{
  const auto &[value, name] = shadowmem[ptr];
  return value;
  //return shadowmem[ptr];
}

void __shadowlib_print() {
  std::cout << "Shadow memory contents (0 untainted, 1 tainted):" << std::endl;
  
  for (auto &[key, val] : shadowmem) {
    const auto &[value, name] = val;
    std::cout << "Name: " << name << " Taint: " << value << std::endl;
  }
}