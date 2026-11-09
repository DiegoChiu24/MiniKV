@echo off
cd /d "%~dp0.."
g++ -std=c++17 -O2 -Wall -Wextra main.cpp -o minikv.exe
if errorlevel 1 exit /b 1
g++ -std=c++17 -O2 -Wall -Wextra Pruebas\prueba_fase4.cpp -o prueba_fase4.exe
if errorlevel 1 exit /b 1
g++ -std=c++17 -O2 -Wall -Wextra Pruebas\prueba_integracion.cpp -o prueba_integracion.exe
if errorlevel 1 exit /b 1
echo === PRUEBA DE COMANDOS ===
type "Pruebas\pruebas_fase4.txt" | minikv.exe
if errorlevel 1 exit /b 1
echo === PRUEBA DE TIPOS Y EXPIRACION ===
prueba_fase4.exe
if errorlevel 1 exit /b 1
echo === REGRESION DE 100000 REGISTROS ===
prueba_integracion.exe
if errorlevel 1 exit /b 1
echo === FASE 4: PRUEBAS COMPLETADAS ===
