#include <iostream>
#include <sstream>
#include <string>
#include "AVLTree.h"
using namespace std;

bool leerLimite(stringstream& ss, int& limite, int defecto, bool permiteNumeroSolo) {
    limite = defecto;
    string palabra;
    if (!(ss >> palabra)) return true;

    string numero;
    if (palabra == "LIMIT") {
        if (!(ss >> numero)) return false;
    } else if (permiteNumeroSolo) {
        numero = palabra;
    } else {
        return false;
    }

    stringstream convertir(numero);
    if (!(convertir >> limite) || limite < 0) return false;
    char sobrante;
    if (convertir >> sobrante) return false;
    string extra;
    return !(ss >> extra);
}

bool sinExtras(stringstream& ss) {
    string extra;
    return !(ss >> extra);
}

int main() {
    AVLTree arbol;
    string linea;
    cout << "MiniKV - Prueba independiente de AVL (fase 2)\n";
    cout << "Escribe HELP para ver los comandos.\n";

    while (true) {
        cout << "\navl> ";
        if (!getline(cin, linea)) break;
        stringstream ss(linea);
        string comando;
        ss >> comando;

        if (comando == "HELP" && sinExtras(ss)) {
            cout << "INSERT clave | REMOVE clave | FIND clave\n";
            cout << "RANGE inicio fin [LIMIT n] | PREFIX prefijo [LIMIT n]\n";
            cout << "COUNT inicio fin | SUCCESSOR clave | PREDECESSOR clave\n";
            cout << "MIN | MAX | DUMP [LIMIT n] | INFO | CHECK | EXIT\n";
            cout << "Para prefijo vacio: PREFIX \"\" LIMIT 5\n";
        }
        else if (comando == "INSERT" || comando == "REMOVE" || comando == "FIND" ||
                 comando == "SUCCESSOR" || comando == "PREDECESSOR") {
            string clave, extra;
            if (!(ss >> clave) || (ss >> extra)) {
                cout << "ERROR: Uso: " << comando << " clave\n";
                continue;
            }
            if (comando == "INSERT")
                cout << (arbol.insertar(clave) ? "OK: Clave insertada." : "Clave repetida.") << endl;
            else if (comando == "REMOVE")
                cout << (arbol.eliminar(clave) ? "OK: Clave eliminada." : "Clave no encontrada.") << endl;
            else if (comando == "FIND")
                cout << (arbol.buscar(clave) ? 1 : 0) << endl;
            else {
                string resultado;
                bool existe = comando == "SUCCESSOR" ?
                    arbol.sucesor(clave, resultado) : arbol.predecesor(clave, resultado);
                cout << (existe ? resultado : "No existe.") << endl;
            }
        }
        else if (comando == "RANGE" || comando == "COUNT") {
            string inicio, fin;
            int limite = -1;
            if (!(ss >> inicio >> fin) ||
                (comando == "COUNT" ? !sinExtras(ss) : !leerLimite(ss, limite, -1, false))) {
                cout << "ERROR: Uso: " << comando << " inicio fin [LIMIT n para RANGE]\n";
                continue;
            }
            if (comando == "COUNT") cout << arbol.contarRango(inicio, fin) << endl;
            else {
                int resultados = arbol.mostrarRango(inicio, fin, limite);
                cout << "Resultados: " << resultados << endl;
            }
        }
        else if (comando == "PREFIX") {
            string prefijo;
            int limite = -1;
            if (!(ss >> prefijo) || !leerLimite(ss, limite, -1, false)) {
                cout << "ERROR: Uso: PREFIX prefijo [LIMIT n]\n";
                continue;
            }
            if (prefijo == "\"\"" || prefijo == "''") prefijo = "";
            int resultados = arbol.mostrarPrefijo(prefijo, limite);
            cout << "Resultados: " << resultados << endl;
        }
        else if (comando == "DUMP") {
            int limite = 50;
            if (!leerLimite(ss, limite, 50, true)) {
                cout << "ERROR: Uso: DUMP [LIMIT n]\n";
                continue;
            }
            int resultados = arbol.mostrarOrden(limite);
            cout << "Resultados: " << resultados << endl;
        }
        else if ((comando == "MIN" || comando == "MAX") && sinExtras(ss)) {
            string clave;
            bool existe = comando == "MIN" ? arbol.minimo(clave) : arbol.maximo(clave);
            cout << (existe ? clave : "Arbol vacio.") << endl;
        }
        else if (comando == "INFO" && sinExtras(ss)) {
            cout << "Claves: " << arbol.cantidad() << "\nAltura: " << arbol.alturaArbol() << endl;
        }
        else if (comando == "CHECK" && sinExtras(ss)) {
            cout << (arbol.validar() ? "AVL valido." : "ERROR: AVL inconsistente.") << endl;
        }
        else if (comando == "EXIT" && sinExtras(ss)) {
            cout << "Cerrando prueba AVL...\n";
            break;
        }
        else if (!comando.empty()) {
            cout << "Comando no reconocido o argumentos incorrectos.\n";
        }
    }
    return 0;
}
