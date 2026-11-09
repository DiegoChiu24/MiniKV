#include <iostream>
#include <string>
#include <vector>
#include "../MiniKV.h"
using namespace std;

string claveNumero(int numero) {
    string s = to_string(numero);
    return "k" + string(6 - s.size(), '0') + s;
}

int main() {
    MiniKV motor;
    int fallos = 0;

    for (int i = 0; i < 100000; i++) {
        string clave = claveNumero(i);
        if (!motor.guardar(clave, "valor" + to_string(i))) fallos++;
    }

    if (motor.cantidad() != 100000) fallos++;
    if (!motor.consistente()) fallos++;
    if (motor.contar("k000000", "k099999") != 100000) fallos++;

    Registro* primero = motor.buscar("k000000");
    Registro* ultimo = motor.buscar("k099999");
    if (primero == nullptr || primero->valor != "valor0") fallos++;
    if (ultimo == nullptr || ultimo->valor != "valor99999") fallos++;

    vector<Registro> r = motor.rango("k000050", "k000060", 3);
    if (r.size() != 3 || r[0].clave != "k000050" || r[2].clave != "k000052") fallos++;

    vector<Registro> p = motor.prefijo("k000", 1000);
    if (p.size() != 1000) fallos++;

    string resultado;
    if (!motor.sucesor("k000050", resultado) || resultado != "k000051") fallos++;
    if (!motor.predecesor("k000050", resultado) || resultado != "k000049") fallos++;

    if (!motor.guardar("k000050", "modificado")) fallos++;
    Registro* actualizado = motor.buscar("k000050");
    if (actualizado == nullptr || actualizado->valor != "modificado") fallos++;
    if (motor.cantidad() != 100000) fallos++;

    if (!motor.eliminar("k000050")) fallos++;
    if (motor.eliminar("k000050")) fallos++;
    if (motor.existe("k000050")) fallos++;
    if (motor.contar("k000000", "k099999") != 99999) fallos++;
    if (!motor.consistente()) fallos++;

    for (int i = 0; i < 100000; i += 3) {
        if (!motor.eliminar(claveNumero(i))) fallos++;
    }
    if (motor.cantidad() != 66665) fallos++;
    if (motor.contar("k000000", "k099999") != 66665) fallos++;
    if (motor.existe("k000000")) fallos++;
    if (!motor.existe("k000001")) fallos++;
    if (!motor.consistente()) fallos++;

    cout << "Registros finales: " << motor.cantidad() << endl;
    cout << "Prueba integral: " << (fallos == 0 ? "OK" : "ERROR") << endl;
    cout << "Fallos: " << fallos << endl;
    return fallos == 0 ? 0 : 1;
}
