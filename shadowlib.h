#ifndef SHADOW_LIB_H
#define SHADOW_LIB_H

extern void __shadowlib_insert(void *ptr, int value, char *string);
extern int __shadowlib_get(void *ptr);
extern void __shadowlib_print();

#endif // SHADOW_LIB_H
