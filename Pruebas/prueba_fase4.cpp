#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include "../MiniKV.h"
using namespace std;

int main() {
    MiniKV motor;
    int fallos = 0;

    if (motor.guardarConOpciones("edad", "20", "int", false, false, 0) != GUARDADO) fallos++;
    if (motor.guardarConOpciones("precio", "2.5", "float", false, false, 0) != GUARDADO) fallos++;
    if (motor.guardarConOpciones("activo", "true", "bool", false, false, 0) != GUARDADO) fallos++;
    if (motor.guardarConOpciones("texto", "20", "string", false, false, 0) != GUARDADO) fallos++;
    if (motor.cantidad() != 4) fallos++;
    if (motor.buscar("edad")->tipo != "int") fallos++;
    if (motor.buscar("precio")->tipo != "float") fallos++;
    if (motor.buscar("activo")->tipo != "bool") fallos++;
    if (motor.buscar("texto")->tipo != "string") fallos++;
    if (motor.ttl("edad") != -1) fallos++;
    if (motor.ttl("ausente") != -2) fallos++;

    if (motor.guardarConOpciones("edad", "99", "int", true, false, 0) != EXISTENTE) fallos++;
    if (motor.buscar("edad")->valor != "20") fallos++;
    if (motor.guardarConOpciones("edad", "99", "int", false, false, 0) != GUARDADO) fallos++;
    if (motor.buscar("edad")->valor != "99") fallos++;
    if (motor.cantidad() != 4) fallos++;

    if (motor.guardarConOpciones("a_temporal", "uno", "string", false, true, 1) != GUARDADO) fallos++;
    if (motor.ttl("a_temporal") != 1) fallos++;
    if (!motor.existe("a_temporal")) fallos++;

    if (motor.guardarConOpciones("b_reemplazo", "viejo", "string", false, true, 1) != GUARDADO) fallos++;
    if (motor.guardarConOpciones("b_reemplazo", "nuevo", "string", false, false, 0) != GUARDADO) fallos++;
    if (motor.ttl("b_reemplazo") != -1) fallos++;

    if (motor.guardarConOpciones("c_extender", "sigue", "string", false, true, 1) != GUARDADO) fallos++;
    if (!motor.expirar("c_extender", 3)) fallos++;

    this_thread::sleep_for(chrono::milliseconds(1300));
    if (motor.buscar("a_temporal") != nullptr) fallos++;
    if (motor.existe("a_temporal")) fallos++;
    if (motor.ttl("a_temporal") != -2) fallos++;
    if (motor.buscar("b_reemplazo") == nullptr || motor.buscar("b_reemplazo")->valor != "nuevo") fallos++;
    if (motor.ttl("b_reemplazo") != -1) fallos++;
    if (!motor.existe("c_extender")) fallos++;
    if (motor.ttl("c_extender") < 1) fallos++;
    if (!motor.consistente()) fallos++;

    if (motor.guardarConOpciones("a_temporal", "nuevo", "string", true, false, 0) != GUARDADO) fallos++;
    if (motor.guardarConOpciones("a_temporal", "otro", "string", true, false, 0) != EXISTENTE) fallos++;
    if (motor.buscar("a_temporal")->valor != "nuevo") fallos++;
    if (!motor.expirar("c_extender", 0)) fallos++;
    if (motor.existe("c_extender")) fallos++;
    if (motor.ttl("c_extender") != -2) fallos++;
    if (motor.expirar("no_existe", 10)) fallos++;
    if (!motor.consistente()) fallos++;

    if (motor.guardarConOpciones("z_expira", "x", "string", false, true, 1) != GUARDADO) fallos++;
    this_thread::sleep_for(chrono::milliseconds(1150));
    if (motor.contar("a", "zzzz") != 6) fallos++;
    if (motor.existe("z_expira")) fallos++;
    vector<Registro> registros = motor.rango("a", "zzzz");
    if (registros.size() != 6) fallos++;
    if (motor.prefijo("z_").size() != 0) fallos++;
    if (motor.volcado(-1).size() != 6) fallos++;
    if (!motor.consistente()) fallos++;

    cout << "Registros finales: " << motor.cantidad() << endl;
    cout << "Fase 4: " << (fallos == 0 ? "OK" : "ERROR") << endl;
    cout << "Fallos: " << fallos << endl;
    return fallos == 0 ? 0 : 1;
}
