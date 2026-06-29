#include <stdio.h>
#include "funciones.h"

int main() {
    SistemaMonitoreo sistema;
    int opcion;

    inicializar_rutas(&sistema);
    inicializar_sistema(&sistema);

    do {
        mostrar_menu();
        opcion = pedir_entero_rango("Seleccione una opcion: ", 1, 9);

        switch (opcion) {
            case 1: opcion_ingresar_medicion(&sistema);  break;
            case 2: opcion_ver_niveles(&sistema);        break;
            case 3: opcion_prediccion(&sistema);         break;
            case 4: opcion_historico_vs_oms(&sistema);   break;
            case 5: opcion_recomendaciones(&sistema);    break;
            case 6: opcion_exportar_reporte(&sistema);   break;
            case 7: opcion_agregar_zona(&sistema);       break;
            case 8: opcion_eliminar_zona(&sistema);      break;
            case 9:
                printf("\nGuardando historial y saliendo...\n");
                guardar_historial(&sistema);
                printf("Hasta luego.\n\n");
                break;
            default:
                printf("Opcion no reconocida.\n");
                break;
        }

    } while (opcion != 9);

    return 0;
}