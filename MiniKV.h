#ifndef MINIKV_H
#define MINIKV_H

#include <chrono>
#include <iostream>
#include <string>
#include <vector>
#include "Registro.h"
#include "HashTable.h"
#include "AVLTree.h"
#include "ExpirationHeap.h"
using namespace std;

enum ResultadoSet {
    GUARDADO,
    EXISTENTE,
    ERROR_SET
};

class MiniKV {
private:
    HashTable hash;
    AVLTree arbol;
    ExpirationHeap expiraciones;
    unsigned long long siguienteVersion = 1;

    long long ahora() const {
        using namespace chrono;
        return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    }

    bool eliminarDirecto(const string& clave) {
        if (!hash.existe(clave)) return false;
        if (!arbol.eliminar(clave)) return false;
        return hash.eliminar(clave);
    }

    void reconstruirExpiraciones() {
        ExpirationHeap nuevo;
        vector<Registro*> registros = hash.obtenerRegistros();
        for (Registro* registro : registros) {
            if (registro->expiraEn != 0) {
                nuevo.agregar({registro->clave, registro->expiraEn, registro->version});
            }
        }
        expiraciones.intercambiar(nuevo);
    }

    void compactarExpiraciones() {
        if (expiraciones.cantidad() > 4 * (hash.cantidad() + 1) + 128) {
            reconstruirExpiraciones();
        }
    }

    void limpiarExpirados() {
        long long tiempo = ahora();
        while (!expiraciones.vacio() && expiraciones.primero().expiraEn <= tiempo) {
            EventoExpiracion evento = expiraciones.primero();
            expiraciones.retirar();
            Registro* registro = hash.buscar(evento.clave);
            if (registro != nullptr && registro->version == evento.version &&
                registro->expiraEn != 0 && registro->expiraEn <= tiempo) {
                eliminarDirecto(evento.clave);
            }
        }
        compactarExpiraciones();
    }

    vector<Registro> obtenerRegistros(const vector<string>& claves) {
        vector<Registro> resultado;
        for (const string& clave : claves) {
            Registro* registro = hash.buscar(clave);
            if (registro != nullptr) resultado.push_back(*registro);
        }
        return resultado;
    }

public:
    ResultadoSet guardarConOpciones(string clave, string valor, string tipo,
                                   bool nx, bool conTTL, long long segundos) {
        limpiarExpirados();
        if (clave.empty() || (conTTL && (segundos <= 0 || segundos > 2147483647LL))) {
            return ERROR_SET;
        }
        if (tipo != "string" && tipo != "int" && tipo != "float" && tipo != "bool") {
            return ERROR_SET;
        }

        bool encontrado = hash.existe(clave);
        if (encontrado && nx) return EXISTENTE;

        Registro nuevo;
        nuevo.clave = clave;
        nuevo.valor = valor;
        nuevo.tipo = tipo;
        nuevo.actualizadoEn = ahora();
        nuevo.creadoEn = encontrado ? hash.buscar(clave)->creadoEn : nuevo.actualizadoEn;
        nuevo.version = siguienteVersion++;
        if (conTTL) nuevo.expiraEn = nuevo.actualizadoEn + segundos * 1000;

        if (!encontrado) {
            if (!arbol.insertar(clave)) return ERROR_SET;
            try {
                hash.insertar(nuevo);
            } catch (...) {
                arbol.eliminar(clave);
                throw;
            }
        } else {
            hash.insertar(nuevo);
        }

        if (conTTL) expiraciones.agregar({clave, nuevo.expiraEn, nuevo.version});
        compactarExpiraciones();
        return GUARDADO;
    }

    bool guardar(string clave, string valor) {
        return guardarConOpciones(clave, valor, "string", false, false, 0) == GUARDADO;
    }

    Registro* buscar(string clave) {
        limpiarExpirados();
        return hash.buscar(clave);
    }

    bool existe(string clave) {
        limpiarExpirados();
        return hash.existe(clave);
    }

    bool eliminar(string clave) {
        limpiarExpirados();
        bool resultado = eliminarDirecto(clave);
        compactarExpiraciones();
        return resultado;
    }

    bool expirar(string clave, long long segundos) {
        limpiarExpirados();
        Registro* registro = hash.buscar(clave);
        if (registro == nullptr) return false;
        if (segundos <= 0) {
            eliminarDirecto(clave);
            compactarExpiraciones();
            return true;
        }
        if (segundos > 2147483647LL) return false;
        registro->actualizadoEn = ahora();
        registro->expiraEn = registro->actualizadoEn + segundos * 1000;
        registro->version = siguienteVersion++;
        expiraciones.agregar({clave, registro->expiraEn, registro->version});
        compactarExpiraciones();
        return true;
    }

    long long ttl(string clave) {
        limpiarExpirados();
        Registro* registro = hash.buscar(clave);
        if (registro == nullptr) return -2;
        if (registro->expiraEn == 0) return -1;
        long long resta = registro->expiraEn - ahora();
        if (resta <= 0) {
            limpiarExpirados();
            return -2;
        }
        return (resta + 999) / 1000;
    }

    vector<Registro> rango(string inicio, string fin, int limite = -1) {
        limpiarExpirados();
        return obtenerRegistros(arbol.clavesRango(inicio, fin, limite));
    }

    vector<Registro> prefijo(string texto, int limite = -1) {
        limpiarExpirados();
        return obtenerRegistros(arbol.clavesPrefijo(texto, limite));
    }

    vector<string> volcado(int limite = 50) {
        limpiarExpirados();
        return arbol.clavesOrden(limite);
    }

    int contar(string inicio, string fin) {
        limpiarExpirados();
        return arbol.contarRango(inicio, fin);
    }

    bool sucesor(string clave, string& resultado) {
        limpiarExpirados();
        return arbol.sucesor(clave, resultado);
    }

    bool predecesor(string clave, string& resultado) {
        limpiarExpirados();
        return arbol.predecesor(clave, resultado);
    }

    bool minimo(string& resultado) {
        limpiarExpirados();
        return arbol.minimo(resultado);
    }

    bool maximo(string& resultado) {
        limpiarExpirados();
        return arbol.maximo(resultado);
    }

    bool consistente() {
        limpiarExpirados();
        if (!arbol.validar()) return false;
        if (hash.cantidad() != arbol.cantidad()) return false;
        vector<string> claves = arbol.clavesOrden(-1);
        for (const string& clave : claves) {
            if (!hash.existe(clave)) return false;
        }
        return true;
    }

    int cantidad() {
        limpiarExpirados();
        return hash.cantidad();
    }

    void informacionHash() {
        limpiarExpirados();
        hash.mostrarResumen();
    }

    void estadisticas() {
        limpiarExpirados();
        vector<Registro*> registros = hash.obtenerRegistros();
        int strings = 0, enteros = 0, reales = 0, booleanos = 0;
        int conTTL = 0;
        long long ttlMinimo = 0, ttlMaximo = 0, sumaTTL = 0;
        long long sumaLongitudes = 0;
        long long tiempo = ahora();

        for (Registro* registro : registros) {
            if (registro->tipo == "string") strings++;
            else if (registro->tipo == "int") enteros++;
            else if (registro->tipo == "float") reales++;
            else if (registro->tipo == "bool") booleanos++;

            sumaLongitudes += (long long)registro->clave.size();
            if (registro->expiraEn != 0) {
                long long resta = registro->expiraEn - tiempo;
                long long restante = resta > 0 ? (resta + 999) / 1000 : 0;
                if (conTTL == 0 || restante < ttlMinimo) ttlMinimo = restante;
                if (conTTL == 0 || restante > ttlMaximo) ttlMaximo = restante;
                sumaTTL += restante;
                conTTL++;
            }
        }

        cout << "Registros activos: " << registros.size() << endl;
        cout << "Tipo string: " << strings << endl;
        cout << "Tipo int: " << enteros << endl;
        cout << "Tipo float: " << reales << endl;
        cout << "Tipo bool: " << booleanos << endl;
        cout << "Con TTL: " << conTTL << endl;

        if (conTTL == 0) {
            cout << "TTL minimo: N/A" << endl;
            cout << "TTL maximo: N/A" << endl;
            cout << "TTL promedio: N/A" << endl;
        } else {
            cout << "TTL minimo: " << ttlMinimo << endl;
            cout << "TTL maximo: " << ttlMaximo << endl;
            cout << "TTL promedio: " << (double)sumaTTL / conTTL << endl;
        }

        cout << "Longitud promedio de claves: " <<
             (registros.empty() ? 0.0 : (double)sumaLongitudes / registros.size()) << endl;

        string clave;
        cout << "Clave minima: " << (arbol.minimo(clave) ? clave : "N/A") << endl;
        cout << "Clave maxima: " << (arbol.maximo(clave) ? clave : "N/A") << endl;
        cout << "Colisiones hash: " << hash.numeroColisiones() << endl;
    }
};

#endif
