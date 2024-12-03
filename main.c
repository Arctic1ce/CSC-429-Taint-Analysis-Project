#include <math.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  int i;
  int j = 0;
  double k = 5.2;
  double p = 144.0;
  int l;

  printf("Please enter an integer: ");
  scanf("%d", &i);
  printf("You entered: %d\n", i);

  printf("j before assignment: %d\n", j);
  j = i + 12;
  k = sqrt(p);
  l = 100;
  printf("j after assignment: %d\n", j);

  return 0;
}