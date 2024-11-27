#include <stdio.h>

int main(int argc, char *argv[]) {
  int i;
  int j = 0;

  printf("Please enter an integer: ");
  scanf("%d", &i);
  printf("You entered: %d\n", i);

  printf("j before assignment: %d\n", j);
  j = i;
  printf("j after assignment: %d\n", j);

  return 0;
}