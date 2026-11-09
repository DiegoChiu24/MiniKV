#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <iostream>
#include <string>
#include <vector>
#include "Registro.h"
using namespace std;

struct NodoHash {
    Registro dato;
    NodoHash* siguiente;

    NodoHash(Registro r) {
        dato = r;
        siguiente = nullptr;
    }
};

class HashTable {
private:
    vector<NodoHash*> tabla;
    int capacidad;
    int elementos = 0;
    int colisiones = 0;

    int hashFuncion(string clave) {
        unsigned long long h = 5381;
        for (char c : clave) {
            h = h * 33 + (unsigned char)c;
        }
        return h % capacidad;
    }

    void rehash(int nuevaCapacidad) {
        vector<NodoHash*> nuevaTabla(nuevaCapacidad, nullptr);
        int viejaCapacidad = capacidad;
        capacidad = nuevaCapacidad;

        for (int i = 0; i < viejaCapacidad; i++) {
            NodoHash* actual = tabla[i];
            while (actual != nullptr) {
                NodoHash* siguiente = actual->siguiente;
                int index = hashFuncion(actual->dato.clave);
                actual->siguiente = nuevaTabla[index];
                nuevaTabla[index] = actual;
                actual = siguiente;
            }
        }
        tabla.swap(nuevaTabla);
    }

public:
    HashTable(int cap = 8) {
        capacidad = cap > 0 ? cap : 8;
        tabla.assign(capacidad, nullptr);
    }

    ~HashTable() {
        for (int i = 0; i < capacidad; i++) {
            NodoHash* actual = tabla[i];
            while (actual != nullptr) {
                NodoHash* aBorrar = actual;
                actual = actual->siguiente;
                delete aBorrar;
            }
        }
    }

    HashTable(const HashTable&) = delete;
    HashTable& operator=(const HashTable&) = delete;

    void insertar(Registro nuevoRegistro) {
        int index = hashFuncion(nuevoRegistro.clave);
        NodoHash* actual = tabla[index];
        while (actual != nullptr) {
            if (actual->dato.clave == nuevoRegistro.clave) {
                actual->dato = nuevoRegistro;
                return;
            }
            actual = actual->siguiente;
        }

        if ((double)(elementos + 1) / capacidad > 0.75) {
            rehash(capacidad * 2);
            index = hashFuncion(nuevoRegistro.clave);
        }

        NodoHash* nuevo = new NodoHash(nuevoRegistro);
        if (tabla[index] != nullptr) colisiones++;
        nuevo->siguiente = tabla[index];
        tabla[index] = nuevo;
        elementos++;
    }

    Registro* buscar(string clave) {
        int index = hashFuncion(clave);
        NodoHash* actual = tabla[index];
        while (actual != nullptr) {
            if (actual->dato.clave == clave) return &actual->dato;
            actual = actual->siguiente;
        }
        return nullptr;
    }

    bool existe(string clave) {
        return buscar(clave) != nullptr;
    }

    bool eliminar(string clave) {
        int index = hashFuncion(clave);
        NodoHash* actual = tabla[index];
        NodoHash* anterior = nullptr;

        while (actual != nullptr) {
            if (actual->dato.clave == clave) {
                if (anterior == nullptr) tabla[index] = actual->siguiente;
                else anterior->siguiente = actual->siguiente;
                delete actual;
                elementos--;
                return true;
            }
            anterior = actual;
            actual = actual->siguiente;
        }
        return false;
    }

    int cantidad() const { return elementos; }

    int numeroColisiones() const { return colisiones; }

    vector<Registro*> obtenerRegistros() const {
        vector<Registro*> registros;
        registros.reserve(elementos);
        for (int i = 0; i < capacidad; i++) {
            NodoHash* actual = tabla[i];
            while (actual != nullptr) {
                registros.push_back(&actual->dato);
                actual = actual->siguiente;
            }
        }
        return registros;
    }

    void mostrarResumen() const {
        cout << "Capacidad: " << capacidad << endl;
        cout << "Registros: " << elementos << endl;
        cout << "Factor de carga: " << (double)elementos / capacidad << endl;
        cout << "Colisiones de insercion acumuladas: " << colisiones << endl;
    }

    void imprimir() const {
        for (int i = 0; i < capacidad; i++) {
            cout << "[" << i << "] ";
            NodoHash* actual = tabla[i];
            if (actual == nullptr) cout << "(vacio)";
            while (actual != nullptr) {
                cout << "(" << actual->dato.clave << ", " << actual->dato.valor << ")";
                if (actual->siguiente != nullptr) cout << " -> ";
                actual = actual->siguiente;
            }
            cout << endl;
        }
    }
};

#endif
