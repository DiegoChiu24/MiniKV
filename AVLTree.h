#ifndef AVLTREE_H
#define AVLTREE_H

#include <iostream>
#include <string>
#include <vector>
using namespace std;

struct NodoAVL {
    string clave;
    int altura;
    int tamano;
    NodoAVL* izquierda;
    NodoAVL* derecha;

    NodoAVL(string c) {
        clave = c;
        altura = 1;
        tamano = 1;
        izquierda = nullptr;
        derecha = nullptr;
    }
};

class AVLTree {
private:
    NodoAVL* raiz;
    int elementos;

    int altura(NodoAVL* nodo) const {
        return nodo == nullptr ? 0 : nodo->altura;
    }

    int tamano(NodoAVL* nodo) const {
        return nodo == nullptr ? 0 : nodo->tamano;
    }

    int maximoEntero(int a, int b) const {
        return a > b ? a : b;
    }

    void actualizar(NodoAVL* nodo) {
        nodo->altura = 1 + maximoEntero(altura(nodo->izquierda), altura(nodo->derecha));
        nodo->tamano = 1 + tamano(nodo->izquierda) + tamano(nodo->derecha);
    }

    int factorBalance(NodoAVL* nodo) const {
        return nodo == nullptr ? 0 : altura(nodo->izquierda) - altura(nodo->derecha);
    }

    NodoAVL* rotacionDerecha(NodoAVL* y) {
        NodoAVL* x = y->izquierda;
        NodoAVL* temporal = x->derecha;
        x->derecha = y;
        y->izquierda = temporal;
        actualizar(y);
        actualizar(x);
        return x;
    }

    NodoAVL* rotacionIzquierda(NodoAVL* x) {
        NodoAVL* y = x->derecha;
        NodoAVL* temporal = y->izquierda;
        y->izquierda = x;
        x->derecha = temporal;
        actualizar(x);
        actualizar(y);
        return y;
    }

    NodoAVL* balancear(NodoAVL* nodo) {
        if (nodo == nullptr) return nullptr;
        actualizar(nodo);
        int balance = factorBalance(nodo);

        if (balance > 1) {
            if (factorBalance(nodo->izquierda) < 0)
                nodo->izquierda = rotacionIzquierda(nodo->izquierda);
            return rotacionDerecha(nodo);
        }
        if (balance < -1) {
            if (factorBalance(nodo->derecha) > 0)
                nodo->derecha = rotacionDerecha(nodo->derecha);
            return rotacionIzquierda(nodo);
        }
        return nodo;
    }

    NodoAVL* insertarRec(NodoAVL* nodo, string clave, bool& insertado) {
        if (nodo == nullptr) {
            insertado = true;
            return new NodoAVL(clave);
        }
        if (clave < nodo->clave)
            nodo->izquierda = insertarRec(nodo->izquierda, clave, insertado);
        else if (clave > nodo->clave)
            nodo->derecha = insertarRec(nodo->derecha, clave, insertado);
        else
            return nodo;
        return balancear(nodo);
    }

    NodoAVL* menorNodo(NodoAVL* nodo) const {
        while (nodo != nullptr && nodo->izquierda != nullptr)
            nodo = nodo->izquierda;
        return nodo;
    }

    NodoAVL* mayorNodo(NodoAVL* nodo) const {
        while (nodo != nullptr && nodo->derecha != nullptr)
            nodo = nodo->derecha;
        return nodo;
    }

    NodoAVL* eliminarRec(NodoAVL* nodo, const string& clave, bool& eliminado) {
        if (nodo == nullptr) return nullptr;

        if (clave < nodo->clave) {
            nodo->izquierda = eliminarRec(nodo->izquierda, clave, eliminado);
        } else if (clave > nodo->clave) {
            nodo->derecha = eliminarRec(nodo->derecha, clave, eliminado);
        } else {
            eliminado = true;
            if (nodo->izquierda == nullptr || nodo->derecha == nullptr) {
                NodoAVL* hijo = nodo->izquierda != nullptr ? nodo->izquierda : nodo->derecha;
                delete nodo;
                return hijo;
            }
            NodoAVL* sucesor = menorNodo(nodo->derecha);
            nodo->clave = sucesor->clave;
            bool ignorar = false;
            nodo->derecha = eliminarRec(nodo->derecha, sucesor->clave, ignorar);
        }
        return balancear(nodo);
    }

    void liberar(NodoAVL* nodo) {
        if (nodo == nullptr) return;
        liberar(nodo->izquierda);
        liberar(nodo->derecha);
        delete nodo;
    }

    bool comienzaCon(const string& clave, const string& prefijo) const {
        if (prefijo.size() > clave.size()) return false;
        for (size_t i = 0; i < prefijo.size(); i++)
            if (clave[i] != prefijo[i]) return false;
        return true;
    }

    bool limiteAlcanzado(int limite, int mostrados) const {
        return limite >= 0 && mostrados >= limite;
    }

    void rangoRec(NodoAVL* nodo, const string& inicio, const string& fin,
                  int limite, vector<string>& claves) const {
        if (nodo == nullptr || limiteAlcanzado(limite, (int)claves.size())) return;
        if (inicio < nodo->clave)
            rangoRec(nodo->izquierda, inicio, fin, limite, claves);
        if (limiteAlcanzado(limite, (int)claves.size())) return;
        if (inicio <= nodo->clave && nodo->clave <= fin)
            claves.push_back(nodo->clave);
        if (nodo->clave < fin)
            rangoRec(nodo->derecha, inicio, fin, limite, claves);
    }

    void prefijoRec(NodoAVL* nodo, const string& prefijo,
                   int limite, vector<string>& claves) const {
        if (nodo == nullptr || limiteAlcanzado(limite, (int)claves.size())) return;
        if (nodo->clave < prefijo) {
            prefijoRec(nodo->derecha, prefijo, limite, claves);
            return;
        }
        if (!comienzaCon(nodo->clave, prefijo)) {
            prefijoRec(nodo->izquierda, prefijo, limite, claves);
            return;
        }
        prefijoRec(nodo->izquierda, prefijo, limite, claves);
        if (limiteAlcanzado(limite, (int)claves.size())) return;
        claves.push_back(nodo->clave);
        prefijoRec(nodo->derecha, prefijo, limite, claves);
    }

    void ordenRec(NodoAVL* nodo, int limite, vector<string>& claves) const {
        if (nodo == nullptr || limiteAlcanzado(limite, (int)claves.size())) return;
        ordenRec(nodo->izquierda, limite, claves);
        if (limiteAlcanzado(limite, (int)claves.size())) return;
        claves.push_back(nodo->clave);
        ordenRec(nodo->derecha, limite, claves);
    }

    int menoresQue(const string& clave) const {
        int total = 0;
        NodoAVL* actual = raiz;
        while (actual != nullptr) {
            if (clave <= actual->clave) {
                actual = actual->izquierda;
            } else {
                total += 1 + tamano(actual->izquierda);
                actual = actual->derecha;
            }
        }
        return total;
    }

    int menoresOIguales(const string& clave) const {
        int total = 0;
        NodoAVL* actual = raiz;
        while (actual != nullptr) {
            if (clave < actual->clave) {
                actual = actual->izquierda;
            } else {
                total += 1 + tamano(actual->izquierda);
                actual = actual->derecha;
            }
        }
        return total;
    }

    bool validarRec(NodoAVL* nodo, const string* inferior, const string* superior,
                    int& alturaReal, int& tamanoReal) const {
        if (nodo == nullptr) {
            alturaReal = 0;
            tamanoReal = 0;
            return true;
        }
        if (inferior != nullptr && nodo->clave <= *inferior) return false;
        if (superior != nullptr && nodo->clave >= *superior) return false;

        int ai = 0, ad = 0, ti = 0, td = 0;
        if (!validarRec(nodo->izquierda, inferior, &nodo->clave, ai, ti)) return false;
        if (!validarRec(nodo->derecha, &nodo->clave, superior, ad, td)) return false;
        alturaReal = 1 + maximoEntero(ai, ad);
        tamanoReal = 1 + ti + td;
        int diferencia = ai - ad;
        return diferencia >= -1 && diferencia <= 1 &&
               nodo->altura == alturaReal && nodo->tamano == tamanoReal;
    }

public:
    AVLTree() {
        raiz = nullptr;
        elementos = 0;
    }

    ~AVLTree() {
        liberar(raiz);
    }

    AVLTree(const AVLTree&) = delete;
    AVLTree& operator=(const AVLTree&) = delete;

    bool insertar(string clave) {
        bool insertado = false;
        raiz = insertarRec(raiz, clave, insertado);
        if (insertado) elementos++;
        return insertado;
    }

    bool eliminar(string clave) {
        bool eliminado = false;
        raiz = eliminarRec(raiz, clave, eliminado);
        if (eliminado) elementos--;
        return eliminado;
    }

    bool buscar(string clave) const {
        NodoAVL* actual = raiz;
        while (actual != nullptr) {
            if (clave == actual->clave) return true;
            if (clave < actual->clave) actual = actual->izquierda;
            else actual = actual->derecha;
        }
        return false;
    }

    bool minimo(string& resultado) const {
        NodoAVL* nodo = menorNodo(raiz);
        if (nodo == nullptr) return false;
        resultado = nodo->clave;
        return true;
    }

    bool maximo(string& resultado) const {
        NodoAVL* nodo = mayorNodo(raiz);
        if (nodo == nullptr) return false;
        resultado = nodo->clave;
        return true;
    }

    bool sucesor(string clave, string& resultado) const {
        NodoAVL* actual = raiz;
        NodoAVL* candidato = nullptr;
        while (actual != nullptr) {
            if (clave < actual->clave) {
                candidato = actual;
                actual = actual->izquierda;
            } else {
                actual = actual->derecha;
            }
        }
        if (candidato == nullptr) return false;
        resultado = candidato->clave;
        return true;
    }

    bool predecesor(string clave, string& resultado) const {
        NodoAVL* actual = raiz;
        NodoAVL* candidato = nullptr;
        while (actual != nullptr) {
            if (clave > actual->clave) {
                candidato = actual;
                actual = actual->derecha;
            } else {
                actual = actual->izquierda;
            }
        }
        if (candidato == nullptr) return false;
        resultado = candidato->clave;
        return true;
    }

    int contarRango(string inicio, string fin) const {
        if (inicio > fin) return 0;
        return menoresOIguales(fin) - menoresQue(inicio);
    }

    vector<string> clavesRango(string inicio, string fin, int limite = -1) const {
        vector<string> claves;
        if (inicio <= fin) rangoRec(raiz, inicio, fin, limite, claves);
        return claves;
    }

    vector<string> clavesPrefijo(string prefijo, int limite = -1) const {
        vector<string> claves;
        prefijoRec(raiz, prefijo, limite, claves);
        return claves;
    }

    vector<string> clavesOrden(int limite = 50) const {
        vector<string> claves;
        ordenRec(raiz, limite, claves);
        return claves;
    }

    int mostrarRango(string inicio, string fin, int limite = -1) const {
        vector<string> claves = clavesRango(inicio, fin, limite);
        for (const string& clave : claves) cout << clave << endl;
        return (int)claves.size();
    }

    int mostrarPrefijo(string prefijo, int limite = -1) const {
        vector<string> claves = clavesPrefijo(prefijo, limite);
        for (const string& clave : claves) cout << clave << endl;
        return (int)claves.size();
    }

    int mostrarOrden(int limite = 50) const {
        vector<string> claves = clavesOrden(limite);
        for (const string& clave : claves) cout << clave << endl;
        return (int)claves.size();
    }

    int cantidad() const { return elementos; }
    int alturaArbol() const { return altura(raiz); }

    bool validar() const {
        int alturaReal = 0, tamanoReal = 0;
        return validarRec(raiz, nullptr, nullptr, alturaReal, tamanoReal) &&
               tamanoReal == elementos;
    }
};

#endif
