#include <map>
#include <iostream>


extern "C" void __shadowlib_insert(void *ptr, int value);
extern "C" int __shadowlib_get(void *ptr);
extern "C" void __shadowlib_assert(int __instvalue, int __expectedvalue, char *string);
extern "C" void __shadowlib_print();

std::map<void *, int> shadowmem;


//void __shadowlib_initialize


void __shadowlib_insert(void *ptr, int value)
{
  shadowmem[ptr] = value;
}

int __shadowlib_get(void *ptr)
{
  return shadowmem[ptr];
}

void __shadowlib_assert(int __instvalue, int __expectedvalue, char *string) {
    if (__instvalue == __expectedvalue) {
        return;
    }

    printf(string);
}

void __shadowlib_print() {
  std::cout << "Shadow memory contents (0 untainted, 1 tainted):" << std::endl;
  
  for (auto &[key, value] : shadowmem) {
    std::cout << "Address: " << key << " Taint: " << value << std::endl;
  }
}

/*
int main(int argc, char **argv)
{
    std::map<void *, int> m;
    int a;
    char b;

    m[&a] = 1;
    m[&b] = 2;


    std::cout << "A value " << m[&a] << " B value " << m[&b] << std::endl;


    return 0;
}
*/