#ifndef SHADOW_LIB_H
#define SHADOW_LIB_H

extern void __shadowlib_insert(void *ptr, int value);
extern int __shadowlib_get(void *ptr);
extern void __shadowlib_assert(int __instvalue, int __expectedvalue, char *string);
extern void __shadowlib_print();

#endif // SHADOW_LIB_H