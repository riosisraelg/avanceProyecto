#!/bin/bash
# Demo interactivo del Process Manager con sinónimos

echo "╔════════════════════════════════════════════════════════════╗"
echo "║     DEMO INTERACTIVO - Process Manager Multilenguaje      ║"
echo "╚════════════════════════════════════════════════════════════╝"
echo ""

# Colores
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Función para enviar comando
send_cmd() {
    local cmd="$1"
    echo -e "${CYAN}>>> $cmd${NC}"
    echo "$cmd" | nc -w 3 127.0.0.1 5002 2>/dev/null | head -20
    echo ""
}

# Verificar si el servidor está corriendo
if ! nc -z 127.0.0.1 5002 2>/dev/null; then
    echo -e "${RED}❌ Error: El servidor no está corriendo en el puerto 5002${NC}"
    echo ""
    echo "Inicia el servidor primero con:"
    echo "  ./server_bin"
    echo ""
    exit 1
fi

echo -e "${GREEN}✓ Servidor detectado en 127.0.0.1:5002${NC}"
echo ""

# Demo 1: Listar procesos con diferentes sinónimos
echo -e "${YELLOW}═══ DEMO 1: Listar Procesos (Sinónimos) ═══${NC}"
echo ""
echo "Probando diferentes formas de listar procesos:"
echo ""

send_cmd "LIST"
sleep 1

send_cmd "listar"
sleep 1

send_cmd "ps"
sleep 1

send_cmd "mostrar"
sleep 1

# Demo 2: Crear procesos
echo -e "${YELLOW}═══ DEMO 2: Crear Procesos ═══${NC}"
echo ""
echo "Creando procesos con diferentes comandos:"
echo ""

send_cmd "START sleep 60"
sleep 1

send_cmd "iniciar sleep 45"
sleep 1

send_cmd "ejecutar sleep 30"
sleep 1

send_cmd "crear ping -c 20 google.com"
sleep 2

# Demo 3: Verificar procesos creados
echo -e "${YELLOW}═══ DEMO 3: Verificar Procesos Creados ═══${NC}"
echo ""
send_cmd "LIST" | grep -E "sleep|ping"
echo ""

# Demo 4: Obtener PIDs y detener procesos
echo -e "${YELLOW}═══ DEMO 4: Detener Procesos (Sinónimos) ═══${NC}"
echo ""

# Obtener PIDs de sleep
PIDS=($(ps aux | grep "sleep [0-9]" | grep -v grep | awk '{print $2}'))

if [ ${#PIDS[@]} -gt 0 ]; then
    echo "Procesos sleep encontrados: ${PIDS[@]}"
    echo ""
    
    # Detener con diferentes sinónimos
    if [ ${#PIDS[@]} -ge 1 ]; then
        echo "Deteniendo con STOP:"
        send_cmd "STOP ${PIDS[0]}"
        sleep 1
    fi
    
    if [ ${#PIDS[@]} -ge 2 ]; then
        echo "Deteniendo con matar:"
        send_cmd "matar ${PIDS[1]}"
        sleep 1
    fi
    
    if [ ${#PIDS[@]} -ge 3 ]; then
        echo "Deteniendo con kill:"
        send_cmd "kill ${PIDS[2]}"
        sleep 1
    fi
else
    echo "No se encontraron procesos sleep"
fi

# Demo 5: Manejo de errores
echo -e "${YELLOW}═══ DEMO 5: Manejo de Errores ═══${NC}"
echo ""

echo "Error 1: PID inexistente"
send_cmd "STOP 99999"
sleep 1

echo "Error 2: PID inválido (texto)"
send_cmd "STOP abc123"
sleep 1

echo "Error 3: Comando sin argumentos"
send_cmd "START"
sleep 1

echo "Error 4: Comando desconocido"
send_cmd "comando_que_no_existe"
sleep 1

# Demo 6: Comandos mixtos (inglés/español)
echo -e "${YELLOW}═══ DEMO 6: Uso Mixto (Inglés/Español) ═══${NC}"
echo ""

echo "Usando comandos en inglés y español mezclados:"
echo ""

send_cmd "list"
sleep 1

send_cmd "iniciar sleep 15"
sleep 1

SLEEP_PID=$(ps aux | grep "sleep 15" | grep -v grep | awk '{print $2}' | head -1)
if [ ! -z "$SLEEP_PID" ]; then
    send_cmd "kill $SLEEP_PID"
fi
sleep 1

# Limpieza
echo -e "${YELLOW}═══ LIMPIEZA ═══${NC}"
echo ""
echo "Deteniendo procesos restantes..."

# Detener todos los sleep que creamos
for pid in $(ps aux | grep "sleep [0-9]" | grep -v grep | awk '{print $2}'); do
    echo "Deteniendo PID $pid"
    echo "STOP $pid" | nc -w 2 127.0.0.1 5002 2>/dev/null > /dev/null
done

# Detener ping si sigue corriendo
for pid in $(ps aux | grep "ping -c" | grep -v grep | awk '{print $2}'); do
    echo "Deteniendo ping PID $pid"
    echo "STOP $pid" | nc -w 2 127.0.0.1 5002 2>/dev/null > /dev/null
done

echo ""
echo -e "${GREEN}✓ Demo completado${NC}"
echo ""
echo "═══════════════════════════════════════════════════════════"
echo "Resumen de funcionalidades probadas:"
echo "  ✓ Listar procesos (LIST, listar, ps, mostrar)"
echo "  ✓ Crear procesos (START, iniciar, ejecutar, crear)"
echo "  ✓ Detener procesos (STOP, matar, kill)"
echo "  ✓ Manejo de errores (PIDs inválidos, comandos vacíos)"
echo "  ✓ Uso mixto inglés/español"
echo "═══════════════════════════════════════════════════════════"
echo ""
echo "Para más información, consulta: COMANDOS.md"
