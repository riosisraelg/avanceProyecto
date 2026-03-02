#!/bin/bash
# Script de prueba para verificar los comandos del servidor

SERVER_IP="127.0.0.1"
SERVER_PORT="5002"

echo "=== Test de Comandos del Process Manager ==="
echo ""

# Función para enviar comando y mostrar respuesta
send_command() {
    local cmd="$1"
    echo ">>> Enviando: $cmd"
    echo "$cmd" | nc -w 3 $SERVER_IP $SERVER_PORT
    echo ""
    sleep 1
}

echo "1. Probando comandos LIST (listar procesos)"
echo "-------------------------------------------"
send_command "LIST"
send_command "listar"
send_command "ls"
send_command "ps"
send_command "mostrar"
send_command "ver"

echo "2. Probando comandos START (iniciar proceso)"
echo "---------------------------------------------"
send_command "START sleep 5"
send_command "iniciar sleep 5"
send_command "run sleep 5"
send_command "ejecutar sleep 5"
send_command "lanzar sleep 5"
send_command "crear sleep 5"

echo "3. Esperando 2 segundos para que los procesos se inicien..."
sleep 2

echo "4. Listando procesos para ver los PIDs de sleep"
echo "------------------------------------------------"
send_command "LIST" | grep sleep

echo "5. Probando comandos STOP (detener proceso)"
echo "--------------------------------------------"
# Obtener PID de un proceso sleep
SLEEP_PID=$(ps aux | grep "sleep 5" | grep -v grep | head -1 | awk '{print $2}')

if [ ! -z "$SLEEP_PID" ]; then
    echo "Encontrado proceso sleep con PID: $SLEEP_PID"
    send_command "STOP $SLEEP_PID"
    send_command "LIST" | grep sleep
else
    echo "No se encontró proceso sleep para detener"
fi

echo "6. Probando sinónimos de STOP"
echo "------------------------------"
SLEEP_PID=$(ps aux | grep "sleep 5" | grep -v grep | head -1 | awk '{print $2}')
if [ ! -z "$SLEEP_PID" ]; then
    send_command "matar $SLEEP_PID"
fi

SLEEP_PID=$(ps aux | grep "sleep 5" | grep -v grep | head -1 | awk '{print $2}')
if [ ! -z "$SLEEP_PID" ]; then
    send_command "kill $SLEEP_PID"
fi

SLEEP_PID=$(ps aux | grep "sleep 5" | grep -v grep | head -1 | awk '{print $2}')
if [ ! -z "$SLEEP_PID" ]; then
    send_command "detener $SLEEP_PID"
fi

echo "7. Probando manejo de errores"
echo "------------------------------"
send_command "STOP 99999"
send_command "STOP abc"
send_command "STOP"
send_command "START"
send_command "comando_invalido"

echo "8. Probando comando EXIT"
echo "------------------------"
send_command "salir"

echo ""
echo "=== Pruebas completadas ==="
