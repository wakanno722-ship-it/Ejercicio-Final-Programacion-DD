#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "funciones.h"

/* Declaraciones principales sin el uso de void */
void mostrar_menu();
void inicializar_rutas(SistemaMonitoreo *s);
void opcion_ingresar_medicion(SistemaMonitoreo *s);
void opcion_ver_niveles(SistemaMonitoreo *s);
void opcion_prediccion(SistemaMonitoreo *s);
void opcion_historico_vs_oms(SistemaMonitoreo *s);
void opcion_recomendaciones(SistemaMonitoreo *s);
void opcion_exportar_reporte(SistemaMonitoreo *s);
void opcion_agregar_zona(SistemaMonitoreo *s);
void opcion_eliminar_zona(SistemaMonitoreo *s);

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

void mostrar_menu() {
    printf("\n============================================\n");
    printf("   SIGPCA - Sistema de Monitoreo del Aire\n");
    printf("============================================\n");
    printf(" 1. Registrar medicion en una zona\n");
    printf(" 2. Ver niveles actuales y alertas\n");
    printf(" 3. Ver prediccion a 24 horas\n");
    printf(" 4. Ver promedio historico (30 dias) vs OMS\n");
    printf(" 5. Generar recomendaciones\n");
    printf(" 6. Exportar reporte a archivo\n");
    printf(" 7. Agregar nueva zona de monitoreo\n");
    printf(" 8. Eliminar zona de monitoreo\n");
    printf(" 9. Salir\n");
    printf("--------------------------------------------\n");
}

void inicializar_rutas(SistemaMonitoreo *s) {
    if (s == NULL) return;
    s->num_zonas    = NUM_ZONAS_BASE;
    s->num_reportes = 0;
    snprintf(s->archivo_historial, sizeof(s->archivo_historial), "historial.bin");
    snprintf(s->archivo_reporte,   sizeof(s->archivo_reporte),   "reporte_SIGPCA.txt");
}

void opcion_ingresar_medicion(SistemaMonitoreo *s) {
    int i;
    int seleccion;
    Zona *z;
    char mensaje[300];

    if (s == NULL) return;
    if (s->num_zonas <= 0) {
        printf("\nNo hay zonas registradas.\n");
        return;
    }

    printf("\n--- Registrar medicion de contaminacion ---\n");
    for (i = 0; i < s->num_zonas; i++) {
        printf("  %d. %s  [Alerta actual: %s]\n", i + 1, s->zonas[i].nombre, texto_nivel(s->zonas[i].nivel_alerta));
    }
    printf("  0. Volver al menu\n");

    seleccion = pedir_entero_rango("Seleccione la zona: ", 0, s->num_zonas);
    if (seleccion == 0) return;

    z = &s->zonas[seleccion - 1];
    ingresar_medicion(z);
    comparar_con_limites(z);
    actualizar_predicciones(z);
    actualizar_historial(z);

    printf("\nMedicion registrada correctamente.\n");
    mostrar_datos_zona(z);

    if (z->nivel_alerta >= 1) {
        generar_recomendaciones(z, mensaje, sizeof(mensaje));
        printf("\n*** ATENCION - Nivel %s ***\n%s\n", texto_nivel(z->nivel_alerta), mensaje);
    }
}

void opcion_ver_niveles(SistemaMonitoreo *s) {
    if (s == NULL) return;
    mostrar_resumen_zonas(s);
    printf("\nReferencias OMS: PM2.5: %.0f | NO2: %.0f | SO2: %.0f | CO2 alto: %.0f\n", LIMITE_PM25_OMS, LIMITE_NO2_OMS, LIMITE_SO2_OMS, CO2_ALTO_PPM);
}

void opcion_prediccion(SistemaMonitoreo *s) {
    int i;
    Zona *z;
    if (s == NULL || s->num_zonas <= 0) return;

    printf("\n--- Prediccion a 24 horas ---\n");
    for (i = 0; i < s->num_zonas; i++) {
        z = &s->zonas[i];
        actualizar_predicciones(z);
        printf("ZONA %d: %s -> Pred. PM2.5: %.2f | NO2: %.2f | SO2: %.2f\n", z->id, z->nombre, z->prediccion_pm25_24h, z->prediccion_no2_24h, z->prediccion_so2_24h);
    }
}

void opcion_historico_vs_oms(SistemaMonitoreo *s) {
    int i;
    Zona *z;
    if (s == NULL || s->num_zonas <= 0) return;

    printf("\n--- Historico vs Limites OMS ---\n");
    for (i = 0; i < s->num_zonas; i++) {
        z = &s->zonas[i];
        printf("Zona %s: PM2.5 prom = %.2f (OMS: %.0f)\n", z->nombre, calcular_promedio_ponderado(z->historico_pm25, DIAS_HISTORICO), LIMITE_PM25_OMS);
    }
}

void opcion_recomendaciones(SistemaMonitoreo *s) {
    int i;
    char mensaje[300];
    Zona *z;
    if (s == NULL || s->num_zonas <= 0) return;

    printf("\n--- Recomendaciones ---\n");
    for (i = 0; i < s->num_zonas; i++) {
        z = &s->zonas[i];
        generar_recomendaciones(z, mensaje, sizeof(mensaje));
        printf("Zona %s [%s]: %s\n", z->nombre, texto_nivel(z->nivel_alerta), mensaje);
    }
}

void opcion_exportar_reporte(SistemaMonitoreo *s) {
    if (s != NULL) exportar_reporte(s);
}

void opcion_agregar_zona(SistemaMonitoreo *s) {
    if (s != NULL) agregar_zona(s);
}

void opcion_eliminar_zona(SistemaMonitoreo *s) {
    if (s != NULL) eliminar_zona(s);
}