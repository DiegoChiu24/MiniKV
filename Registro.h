#ifndef REGISTRO_H
#define REGISTRO_H

#include <string>
using namespace std;

struct Registro {
    string clave;
    string valor;
    string tipo;
    long long creadoEn = 0;
    long long actualizadoEn = 0;
    long long expiraEn = 0;
    unsigned long long version = 0;
};

#endif
