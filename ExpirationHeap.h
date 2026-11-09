#ifndef EXPIRATIONHEAP_H
#define EXPIRATIONHEAP_H

#include <string>
#include <utility>
#include <vector>
using namespace std;

struct EventoExpiracion {
    string clave;
    long long expiraEn;
    unsigned long long version;
};

class ExpirationHeap {
private:
    vector<EventoExpiracion> datos;

    bool antes(const EventoExpiracion& a, const EventoExpiracion& b) const {
        if (a.expiraEn != b.expiraEn) return a.expiraEn < b.expiraEn;
        return a.version < b.version;
    }

public:
    bool vacio() const { return datos.empty(); }
    int cantidad() const { return (int)datos.size(); }

    EventoExpiracion primero() const { return datos[0]; }

    void agregar(EventoExpiracion evento) {
        datos.push_back(evento);
        int i = (int)datos.size() - 1;
        while (i > 0) {
            int padre = (i - 1) / 2;
            if (!antes(datos[i], datos[padre])) break;
            swap(datos[i], datos[padre]);
            i = padre;
        }
    }

    void retirar() {
        if (datos.empty()) return;
        datos[0] = datos.back();
        datos.pop_back();
        int i = 0;
        int n = (int)datos.size();
        while (true) {
            int izq = i * 2 + 1;
            int der = i * 2 + 2;
            int menor = i;
            if (izq < n && antes(datos[izq], datos[menor])) menor = izq;
            if (der < n && antes(datos[der], datos[menor])) menor = der;
            if (menor == i) break;
            swap(datos[i], datos[menor]);
            i = menor;
        }
    }

    void intercambiar(ExpirationHeap& otro) {
        datos.swap(otro.datos);
    }
};

#endif
