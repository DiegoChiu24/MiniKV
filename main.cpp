#include <climits>
#include <cmath>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "MiniKV.h"
using namespace std;

struct Parte {
    string texto;
    bool citado;
};

bool dividirLinea(const string& linea, vector<Parte>& partes) {
    string actual;
    bool dentro = false;
    bool iniciado = false;
    bool citado = false;

    for (size_t i = 0; i < linea.size(); i++) {
        char c = linea[i];

        if (dentro && c == '\\' && i + 1 < linea.size() &&
            (linea[i + 1] == '"' || linea[i + 1] == '\\')) {
            actual += linea[++i];
            iniciado = true;
        }
        else if (c == '"') {
            dentro = !dentro;
            iniciado = true;
            citado = true;
        }
        else if (isspace((unsigned char)c) && !dentro) {
            if (iniciado) {
                partes.push_back({actual, citado});
                actual.clear();
                iniciado = false;
                citado = false;
            }
        }
        else {
            actual += c;
            iniciado = true;
        }
    }

    if (dentro) return false;
    if (iniciado) partes.push_back({actual, citado});
    return true;
}

bool leerNumero(const string& texto, long long& resultado) {
    if (texto.empty()) return false;
    stringstream ss(texto);
    ss >> resultado;
    if (ss.fail()) return false;
    char sobrante;
    return !(ss >> sobrante);
}

bool leerLimite(const vector<Parte>& partes, int inicio, int defecto,
                bool aceptaNumero, int& limite) {
    limite = defecto;
    int restantes = (int)partes.size() - inicio;
    if (restantes == 0) return true;

    string numero;
    if (restantes == 2 && partes[inicio].texto == "LIMIT" && !partes[inicio].citado) {
        numero = partes[inicio + 1].texto;
    }
    else if (restantes == 1 && aceptaNumero) {
        numero = partes[inicio].texto;
    }
    else return false;

    long long valor;
    if (!leerNumero(numero, valor) || valor < 0 || valor > INT_MAX) return false;
    limite = (int)valor;
    return true;
}

bool esEntero(const string& texto) {
    long long valor;
    if (!leerNumero(texto, valor)) return false;
    return valor >= INT_MIN && valor <= INT_MAX;
}

bool esReal(const string& texto) {
    if (texto.find('.') == string::npos && texto.find('e') == string::npos &&
        texto.find('E') == string::npos) return false;
    char* final = nullptr;
    double numero = strtod(texto.c_str(), &final);
    return final != texto.c_str() && *final == '\0' && isfinite(numero);
}

string identificarTipo(const string& valor, bool forzarString) {
    if (forzarString) return "string";
    if (valor == "true" || valor == "false") return "bool";
    if (esEntero(valor)) return "int";
    if (esReal(valor)) return "float";
    return "string";
}

int main() {
    MiniKV motor;
    string linea;

    cout << "Bienvenido a MiniKV" << endl;
    cout << "Motor de almacenamiento clave-valor" << endl;

    while (true) {
        cout << "\nminikv> ";
        if (!getline(cin, linea)) break;

        vector<Parte> partes;
        if (!dividirLinea(linea, partes)) {
            cout << "ERROR: Comillas sin cerrar." << endl;
            continue;
        }
        if (partes.empty()) continue;
        string op = partes[0].texto;

        if (op == "HELP" && partes.size() == 1) {
            cout << "SET clave valor [EX segundos] [NX]" << endl;
            cout << "GET clave | EXISTS clave | DEL clave" << endl;
            cout << "EXPIRE clave segundos | TTL clave" << endl;
            cout << "RANGE inicio fin [LIMIT n] | PREFIX prefijo [LIMIT n]" << endl;
            cout << "COUNT inicio fin | SUCCESSOR clave | PREDECESSOR clave" << endl;
            cout << "MIN | MAX | DUMP [LIMIT n] | STATS" << endl;
            cout << "HASHINFO | CHECK | EXIT" << endl;
            cout << "Ejemplo: SET texto \"20\" EX 5 NX" << endl;
            cout << "Prefijo vacio: PREFIX \"\" LIMIT 5" << endl;
        }
        else if (op == "SET") {
            if (partes.size() < 3 || partes[1].texto.empty()) {
                cout << "ERROR: Uso: SET clave valor [EX segundos] [NX]" << endl;
                continue;
            }

            int fin = (int)partes.size();
            bool nx = false;
            bool conTTL = false;
            bool valido = true;
            long long segundos = 0;

            if (fin > 2 && partes[fin - 1].texto == "NX" && !partes[fin - 1].citado) {
                nx = true;
                fin--;
            }
            if (fin >= 4 && partes[fin - 2].texto == "EX" && !partes[fin - 2].citado) {
                conTTL = true;
                valido = leerNumero(partes[fin - 1].texto, segundos) &&
                         segundos > 0 && segundos <= INT_MAX;
                fin -= 2;
            }
            if (fin <= 2 || !valido) {
                cout << "ERROR: Uso: SET clave valor [EX segundos] [NX]" << endl;
                continue;
            }

            string valor;
            for (int i = 2; i < fin; i++) {
                if (i > 2) valor += " ";
                valor += partes[i].texto;
            }
            bool forzarString = (fin == 3 && partes[2].citado);
            string tipo = identificarTipo(valor, forzarString);

            ResultadoSet resultado = motor.guardarConOpciones(partes[1].texto, valor, tipo,
                                                              nx, conTTL, segundos);
            if (resultado == GUARDADO) cout << "OK: Registro guardado." << endl;
            else if (resultado == EXISTENTE) cout << "ERROR: La clave ya existe (NX)." << endl;
            else cout << "ERROR: No se pudo guardar el registro." << endl;
        }
        else if (op == "GET" || op == "EXISTS" || op == "DEL" ||
                 op == "TTL" || op == "SUCCESSOR" || op == "PREDECESSOR") {
            if (partes.size() != 2 || partes[1].texto.empty()) {
                cout << "ERROR: Uso: " << op << " clave" << endl;
                continue;
            }
            string clave = partes[1].texto;
            if (op == "GET") {
                Registro* r = motor.buscar(clave);
                if (r == nullptr) cout << "ERROR: Registro no encontrado." << endl;
                else {
                    cout << "Clave: " << r->clave << endl;
                    cout << "Valor: " << r->valor << endl;
                    cout << "Tipo: " << r->tipo << endl;
                }
            }
            else if (op == "EXISTS") cout << (motor.existe(clave) ? 1 : 0) << endl;
            else if (op == "DEL") {
                cout << (motor.eliminar(clave) ? "OK: Registro eliminado." :
                         "ERROR: Registro no encontrado.") << endl;
            }
            else if (op == "TTL") cout << motor.ttl(clave) << endl;
            else {
                string resultado;
                bool hay = op == "SUCCESSOR" ? motor.sucesor(clave, resultado) :
                                                      motor.predecesor(clave, resultado);
                cout << (hay ? resultado : "No existe.") << endl;
            }
        }
        else if (op == "EXPIRE") {
            long long segundos;
            if (partes.size() != 3 || partes[1].texto.empty() ||
                !leerNumero(partes[2].texto, segundos) || segundos > INT_MAX) {
                cout << "ERROR: Uso: EXPIRE clave segundos" << endl;
                continue;
            }
            cout << (motor.expirar(partes[1].texto, segundos) ? 1 : 0) << endl;
        }
        else if (op == "RANGE" || op == "COUNT") {
            int limite = -1;
            if (partes.size() < 3 ||
                (op == "COUNT" ? partes.size() != 3 : !leerLimite(partes, 3, -1, false, limite))) {
                cout << "ERROR: Uso: " << op << " inicio fin [LIMIT n para RANGE]" << endl;
                continue;
            }
            if (op == "COUNT") cout << motor.contar(partes[1].texto, partes[2].texto) << endl;
            else {
                vector<Registro> registros = motor.rango(partes[1].texto, partes[2].texto, limite);
                for (const Registro& r : registros) cout << r.clave << " = " << r.valor << endl;
                cout << "Resultados: " << registros.size() << endl;
            }
        }
        else if (op == "PREFIX") {
            int limite = -1;
            if (partes.size() < 2 || !leerLimite(partes, 2, -1, false, limite)) {
                cout << "ERROR: Uso: PREFIX prefijo [LIMIT n]" << endl;
                continue;
            }
            vector<Registro> registros = motor.prefijo(partes[1].texto, limite);
            for (const Registro& r : registros) cout << r.clave << " = " << r.valor << endl;
            cout << "Resultados: " << registros.size() << endl;
        }
        else if (op == "DUMP") {
            int limite = 50;
            if (!leerLimite(partes, 1, 50, true, limite)) {
                cout << "ERROR: Uso: DUMP [LIMIT n]" << endl;
                continue;
            }
            vector<string> claves = motor.volcado(limite);
            for (const string& clave : claves) cout << clave << endl;
            cout << "Resultados: " << claves.size() << endl;
        }
        else if ((op == "MIN" || op == "MAX") && partes.size() == 1) {
            string clave;
            bool hay = op == "MIN" ? motor.minimo(clave) : motor.maximo(clave);
            cout << (hay ? clave : "Arbol vacio.") << endl;
        }
        else if (op == "STATS" && partes.size() == 1) motor.estadisticas();
        else if (op == "HASHINFO" && partes.size() == 1) motor.informacionHash();
        else if (op == "CHECK" && partes.size() == 1) {
            cout << (motor.consistente() ? "Indices consistentes." :
                     "ERROR: Indices inconsistentes.") << endl;
        }
        else if (op == "EXIT" && partes.size() == 1) {
            cout << "Cerrando MiniKV..." << endl;
            break;
        }
        else cout << "Comando no reconocido o argumentos incorrectos." << endl;
    }
    return 0;
}
