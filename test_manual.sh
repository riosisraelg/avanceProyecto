#!/bin/bash
# Script para probar manualmente los comandos

echo "=== Prueba Manual de Comandos ==="
echo ""
echo "Iniciando servidor en segundo plano..."
./server_bin &
SERVER_PID=$!
echo "Servidor iniciado con PID: $SERVER_PID"
sleep 2

echo ""
echo "=== TEST 1: Comando LIST (español e inglés) ==="
echo "list" | nc -w 2 127.0.0.1 5002 | head -10
echo ""
echo "listar" | nc -w 2 127.0.0.1 5002 | head -10
echo ""

echo "=== TEST 2: Comando START (crear proceso) ==="
echo "START sleep 30" | nc -w 2 127.0.0.1 5002
echo ""
echo "iniciar ping -c 5 google.com" | nc -w 2 127.0.0.1 5002
echo ""

echo "Esperando 2 segundos..."
sleep 2

echo "=== TEST 3: Verificar procesos creados ==="
echo "list" | nc -w 2 127.0.0.1 5002 | grep -E "sleep|ping"
echo ""

echo "=== TEST 4: Obtener PID de sleep para detenerlo ==="
SLEEP_PID=$(ps aux | grep "sleep 30" | grep -v grep | awk '{print $2}' | head -1)
if [ ! -z "$SLEEP_PID" ]; then
    echo "PID de sleep encontrado: $SLEEP_PID"
    echo "STOP $SLEEP_PID" | nc -w 2 127.0.0.1 5002
    echo ""
    echo "Verificando que se detuvo:"
    echo "list" | nc -w 2 127.0.0.1 5002 | grep sleep || echo "✓ Proceso sleep detenido correctamente"
else
    echo "No se encontró proceso sleep"
fi
echo ""

echo "=== TEST 5: Probar sinónimos en español ==="
echo "START sleep 20" | nc -w 2 127.0.0.1 5002
sleep 1
SLEEP_PID=$(ps aux | grep "sleep 20" | grep -v grep | awk '{print $2}' | head -1)
if [ ! -z "$SLEEP_PID" ]; then
    echo "matar $SLEEP_PID" | nc -w 2 127.0.0.1 5002
fi
echo ""

echo "=== TEST 6: Manejo de errores ==="
echo "STOP 99999" | nc -w 2 127.0.0.1 5002
echo ""
echo "STOP abc" | nc -w 2 127.0.0.1 5002
echo ""
echo "START" | nc -w 2 127.0.0.1 5002
echo ""
echo "comando_invalido" | nc -w 2 127.0.0.1 5002
echo ""

echo "=== Limpieza ==="
echo "Deteniendo servidor..."
kill $SERVER_PID 2>/dev/null
echo "Limpiando procesos sleep restantes..."
pkill -f "sleep 30" 2>/dev/null
pkill -f "sleep 20" 2>/dev/null
echo ""
echo "=== Pruebas completadas ==="
