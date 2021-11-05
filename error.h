// error.h
// Řešení IJC-DU1, příklad b), 19.3.2020
// Autor: Václav Sysel, FIT
// Přeloženo: gcc 7.5.0
//
// využito k projektu ISA
// drobné úpravy provedeny
// error.h version 2

#ifndef ERROR_H
#define ERROR_H

#include <stdlib.h>
void warning_msg(const char *fmt, ...);
void error_exit(int exit_code, const char *fmt, ...);
void printf_error(const char *fmt, ...);

#endif //ERROR_H