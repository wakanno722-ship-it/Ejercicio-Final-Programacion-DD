#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "funciones.h"

const float LIMITE_PM25_OMS = 15.0f;
const float LIMITE_NO2_OMS  = 25.0f;
const float LIMITE_SO2_OMS  = 40.0f;
const float CO2_ALTO_PPM    = 1000.0f;

const char *nombres_zonas_base[5] = {
    "Quito", "El Camal", "Belisario", "Cotocollao", "Tumbaco"
};

const float pm25_base[5]    = { 18.0f, 42.0f, 24.0f, 28.0f, 12.0f };
const float no2_base[5]     = { 24.0f, 48.0f, 32.0f, 30.0f, 12.0f };
const float so2_base[5]     = { 15.0f, 22.0f, 18.0f, 25.0f,  8.0f };
const float co2_base[5]     = { 620.0f, 820.0f, 620.0f, 540.0f, 435.0f };
const float temp_base[5]    = { 16.0f, 16.0f, 15.0f, 14.0f, 19.0f };
const float viento_base[5]  = {  8.0f, 10.0f, 14.0f, 12.0f, 22.0f };
const float humedad_base[5] = { 70.0f, 68.0f, 63.0f, 72.0f, 53.0f };

const float hist_pm25_base[5][30] = {
    {18.0f, 19.2f, 20.5f, 20.0f, 19.0f, 18.5f, 20.0f, 21.3f, 20.1f, 19.4f, 18.8f, 19.7f, 20.4f, 21.0f, 19.6f, 20.2f, 19.8f, 20.1f, 19.2f, 18.7f, 20.0f, 20.6f, 19.9f, 20.3f, 19.7f, 18.9f, 20.1f, 21.0f, 20.4f, 19.5f},
    {24.0f, 21.0f, 28.0f, 25.0f, 19.0f, 22.0f, 26.0f, 30.0f, 23.0f, 18.0f, 27.0f, 29.0f, 24.0f, 22.0f, 20.0f, 28.0f, 25.0f, 21.0f, 23.0f, 27.0f, 30.0f, 26.0f, 22.0f, 25.0f, 28.0f, 24.0f, 21.0f, 27.0f, 25.0f, 22.0f},
    {28.0f, 25.0f, 32.0f, 28.0f, 22.0f, 26.0f, 30.0f, 34.0f, 27.0f, 23.0f, 31.0f, 33.0f, 28.0f, 26.0f, 24.0f, 32.0f, 29.0f, 25.0f, 27.0f, 31.0f, 34.0f, 30.0f, 26.0f, 29.0f, 32.0f, 28.0f, 25.0f, 31.0f, 29.0f, 26.0f},
    {12.0f, 10.0f, 14.0f, 11.0f,  9.0f, 11.0f, 13.0f, 15.0f, 11.0f,  9.0f, 13.0f, 15.0f, 12.0f, 11.0f, 10.0f, 14.0f, 12.0f, 10.0f, 11.0f, 13.0f, 16.0f, 13.0f, 11.0f, 12.0f, 15.0f, 12.0f, 10.0f, 14.0f, 12.0f, 11.0f},
    {32.0f, 28.0f, 36.0f, 31.0f, 25.0f, 29.0f, 33.0f, 38.0f, 30.0f, 26.0f, 35.0f, 37.0f, 31.0f, 29.0f, 27.0f, 35.0f, 32.0f, 28.0f, 30.0f, 34.0f, 37.0f, 33.0f, 29.0f, 32.0f, 35.0f, 31.0f, 28.0f, 34.0f, 32.0f, 29.0f}
};


static float calcular_promedio_integrado(const float *historico, int n, float actual) {
    float promedio_historico;

    if (historico == NULL || n <= 0) return actual;

    promedio_historico = calcular_promedio_ponderado((float *)historico, n);
    if (actual <= 0.0f) return promedio_historico;

    return (promedio_historico * 1.0f + actual * 3.0f) / 4.0f;
}

static void registrar_historico_para_zona(Zona *z) {
    int i;
    int dias;

    if (z == NULL) return;

    printf("\n--- Registrar datos historicos previos ---\n");
    dias = pedir_entero_rango("  Cuantos dias anteriores desea registrar (0-30): ", 0, DIAS_HISTORICO);
    if (dias <= 0) return;

    for (i = 0; i < DIAS_HISTORICO; i++) {
        z->historico_pm25[i] = 0.0f;
        z->historico_no2[i] = 0.0f;
        z->historico_so2[i] = 0.0f;
        z->historico_co2[i] = 0.0f;
        z->historico_temp[i] = 0.0f;
        z->historico_viento[i] = 0.0f;
        z->historico_humedad[i] = 0.0f;
    }

    for (i = 0; i < dias; i++) {
        printf("\n  Dia %d/%d\n", i + 1, dias);
        z->historico_pm25[i] = pedir_flotante_rango("    PM2.5 (ug/m3, 0-500): ", 0.0f, 500.0f);
        z->historico_no2[i] = pedir_flotante_rango("    NO2 (ug/m3, 0-500): ", 0.0f, 500.0f);
        z->historico_so2[i] = pedir_flotante_rango("    SO2 (ug/m3, 0-500): ", 0.0f, 500.0f);
        z->historico_co2[i] = pedir_flotante_rango("    CO2 (ppm, 0-5000): ", 0.0f, 5000.0f);
        z->historico_temp[i] = pedir_flotante_rango("    Temperatura (C, -50 a 60): ", -50.0f, 60.0f);
        z->historico_viento[i] = pedir_flotante_rango("    Viento (km/h, 0-200): ", 0.0f, 200.0f);
        z->historico_humedad[i] = pedir_flotante_rango("    Humedad (%, 0-100): ", 0.0f, 100.0f);
    }
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
    int i;
    Zona *z;
    if (s == NULL) return;

    mostrar_resumen_zonas(s);
    printf("\nReferencias OMS: PM2.5: %.0f | NO2: %.0f | SO2: %.0f | CO2 alto: %.0f\n",
           LIMITE_PM25_OMS, LIMITE_NO2_OMS, LIMITE_SO2_OMS, CO2_ALTO_PPM);

    printf("\n--- Mediciones actuales y referencia historica ---\n");
    printf("+------------------+----------+----------+----------+----------+----------+---------------+----------+\n");
    printf("| Zona             | PM2.5    | NO2      | SO2      | CO2      | Temp(C)  | Viento(km/h)  | Humedad%% |\n");
    printf("+------------------+----------+----------+----------+----------+----------+---------------+----------+\n");
    for (i = 0; i < s->num_zonas; i++) {
        z = &s->zonas[i];
        printf("| %-16s | %8.2f | %8.2f | %8.2f | %8.1f | %8.1f | %13.1f | %8.1f |\n",
               z->nombre,
               z->pm25_actual, z->no2_actual, z->so2_actual, z->co2_actual,
               calcular_promedio_integrado(z->historico_temp, DIAS_HISTORICO, z->temperatura),
               calcular_promedio_integrado(z->historico_viento, DIAS_HISTORICO, z->velocidad_viento),
               calcular_promedio_integrado(z->historico_humedad, DIAS_HISTORICO, z->humedad));
    }
    printf("+------------------+----------+----------+----------+----------+----------+---------------+----------+\n");
    printf("  Los valores climaticos muestran el promedio integrado entre historial y medicion actual.\n");
}

void opcion_prediccion(SistemaMonitoreo *s) {
    int i;
    Zona *z;
    if (s == NULL || s->num_zonas <= 0) return;

    printf("\n--- Prediccion de Contaminantes a 24 Horas ---\n");
    printf("+------------------+---------+---------+---------+-----------+\n");
    printf("| Zona             | PM2.5   | NO2     | SO2     | CO2       |\n");
    printf("|                  | ug/m3   | ug/m3   | ug/m3   | ppm       |\n");
    printf("+------------------+---------+---------+---------+-----------+\n");
    for (i = 0; i < s->num_zonas; i++) {
        z = &s->zonas[i];
        actualizar_predicciones(z);
        printf("| %-16s | %7.2f | %7.2f | %7.2f | %9.1f |\n",
               z->nombre,
               z->prediccion_pm25_24h, z->prediccion_no2_24h,
               z->prediccion_so2_24h,  z->prediccion_co2_24h);
    }
    printf("+------------------+---------+---------+---------+-----------+\n");

    printf("\n--- Factores Ambientales Actuales por Zona ---\n");
    printf("+------------------+----------+---------------+----------+\n");
    printf("| Zona             | Temp(C)  | Viento(km/h)  | Humedad%% |\n");
    printf("+------------------+----------+---------------+----------+\n");
    for (i = 0; i < s->num_zonas; i++) {
        z = &s->zonas[i];
        printf("| %-16s | %8.1f | %13.1f | %8.1f |\n",
               z->nombre,
               z->temperatura, z->velocidad_viento, z->humedad);
    }
    printf("+------------------+----------+---------------+----------+\n");
    printf("  (Prediccion basada en promedio ponderado historico + factores climaticos)\n");
}

void opcion_historico_vs_oms(SistemaMonitoreo *s) {
    int i;
    float prom_pm25, prom_no2, prom_so2, prom_co2;
    float prom_temp, prom_viento, prom_humedad;
    Zona *z;
    if (s == NULL || s->num_zonas <= 0) return;

    printf("\n--- Promedio combinado historico + actual vs limites OMS ---\n");
    printf("+------------------+---------+---------+---------+-----------+\n");
    printf("| Zona             | PM2.5   | NO2     | SO2     | CO2       |\n");
    printf("|                  | ug/m3   | ug/m3   | ug/m3   | ppm       |\n");
    printf("+------------------+---------+---------+---------+-----------+\n");

    for (i = 0; i < s->num_zonas; i++) {
        z = &s->zonas[i];
        prom_pm25 = calcular_promedio_integrado(z->historico_pm25, DIAS_HISTORICO, z->pm25_actual);
        prom_no2  = calcular_promedio_integrado(z->historico_no2,  DIAS_HISTORICO, z->no2_actual);
        prom_so2  = calcular_promedio_integrado(z->historico_so2,  DIAS_HISTORICO, z->so2_actual);
        prom_co2  = calcular_promedio_integrado(z->historico_co2,  DIAS_HISTORICO, z->co2_actual);
        printf("| %-16s | %7.2f | %7.2f | %7.2f | %9.1f |\n",
               z->nombre, prom_pm25, prom_no2, prom_so2, prom_co2);
    }

    printf("+------------------+---------+---------+---------+-----------+\n");
    printf("| Limite OMS       | %7.1f | %7.1f | %7.1f | %9.1f |\n",
           LIMITE_PM25_OMS, LIMITE_NO2_OMS, LIMITE_SO2_OMS, CO2_ALTO_PPM);
    printf("+------------------+---------+---------+---------+-----------+\n");

    printf("\n--- Promedio combinado de factores ambientales ---\n");
    printf("+------------------+----------+---------------+----------+\n");
    printf("| Zona             | Temp(C)  | Viento(km/h)  | Humedad%% |\n");
    printf("+------------------+----------+---------------+----------+\n");

    for (i = 0; i < s->num_zonas; i++) {
        z = &s->zonas[i];
        prom_temp    = calcular_promedio_integrado(z->historico_temp,    DIAS_HISTORICO, z->temperatura);
        prom_viento  = calcular_promedio_integrado(z->historico_viento,  DIAS_HISTORICO, z->velocidad_viento);
        prom_humedad = calcular_promedio_integrado(z->historico_humedad, DIAS_HISTORICO, z->humedad);
        printf("| %-16s | %8.1f | %13.1f | %8.1f |\n",
               z->nombre, prom_temp, prom_viento, prom_humedad);
    }
    printf("+------------------+----------+---------------+----------+\n");
    printf("  (*) El promedio combina el historial de la base binaria con la medicion actual registrada por el usuario.\n");
}

void opcion_recomendaciones(SistemaMonitoreo *s) {
    int i;
    char mensaje[300];
    Zona *z;
    if (s == NULL || s->num_zonas <= 0) return;

    printf("\n--- Recomendaciones por Zona ---\n");
    printf("+------------------+------------+\n");
    printf("| Zona             | Nivel      |\n");
    printf("+------------------+------------+\n");
    for (i = 0; i < s->num_zonas; i++) {
        z = &s->zonas[i];
        printf("| %-16s | %-10s |\n", z->nombre, texto_nivel(z->nivel_alerta));
    }
    printf("+------------------+------------+\n\n");

    for (i = 0; i < s->num_zonas; i++) {
        z = &s->zonas[i];
        generar_recomendaciones(z, mensaje, sizeof(mensaje));
        printf("  [%s] %s\n    -> %s\n\n", texto_nivel(z->nivel_alerta), z->nombre, mensaje);
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


void obtener_fecha_hora(char *fecha, int tf, char *hora, int th) {
    if (fecha != NULL && tf > 0) snprintf(fecha, tf, "%s", __DATE__);
    if (hora != NULL && th > 0) snprintf(hora, th, "%s", __TIME__);
}

const char *texto_nivel(int nivel) {
    if (nivel == 1) return "PRECAUCION";
    if (nivel == 2) return "ALERTA";
    if (nivel == 3) return "PELIGRO";
    return "BUENO";
}

int validar_float_ok(float v, float minimo, float maximo) {
    if (v < minimo || v > maximo) return 0;
    return 1;
}

void validar_zona(Zona *z, int indice) {
    int i;
    if (z == NULL) return;

    if (z->id < 0 || z->id >= NUM_ZONAS_MAX) z->id = indice;

    if (z->nombre[0] == '\0') {
        snprintf(z->nombre, sizeof(z->nombre), "Zona %d", indice);
    }
    z->nombre[sizeof(z->nombre) - 1] = '\0';

    if (!validar_float_ok(z->pm25_actual, 0.0f, 500.0f))   z->pm25_actual = 0.0f;
    if (!validar_float_ok(z->no2_actual,  0.0f, 500.0f))   z->no2_actual  = 0.0f;
    if (!validar_float_ok(z->so2_actual,  0.0f, 500.0f))   z->so2_actual  = 0.0f;
    if (!validar_float_ok(z->co2_actual,  0.0f, 5000.0f))  z->co2_actual  = 0.0f;

    if (!validar_float_ok(z->temperatura,      -50.0f, 60.0f))  z->temperatura      = 15.0f;
    if (!validar_float_ok(z->velocidad_viento,   0.0f, 200.0f)) z->velocidad_viento = 10.0f;
    if (!validar_float_ok(z->humedad,            0.0f, 100.0f)) z->humedad          = 60.0f;

    for (i = 0; i < DIAS_HISTORICO; i++) {
        if (!validar_float_ok(z->historico_pm25[i], 0.0f, 500.0f))  z->historico_pm25[i] = 0.0f;
        if (!validar_float_ok(z->historico_no2[i],  0.0f, 500.0f))  z->historico_no2[i]  = 0.0f;
        if (!validar_float_ok(z->historico_so2[i],  0.0f, 500.0f))  z->historico_so2[i]  = 0.0f;
        if (!validar_float_ok(z->historico_co2[i],  0.0f, 5000.0f)) z->historico_co2[i]  = 0.0f;
        if (!validar_float_ok(z->historico_temp[i],    -50.0f, 60.0f))  z->historico_temp[i]    = 15.0f;
        if (!validar_float_ok(z->historico_viento[i],   0.0f, 200.0f))  z->historico_viento[i]  = 10.0f;
        if (!validar_float_ok(z->historico_humedad[i],  0.0f, 100.0f))  z->historico_humedad[i] = 60.0f;
    }

    if (!validar_float_ok(z->prediccion_pm25_24h, 0.0f, 2000.0f))  z->prediccion_pm25_24h = 0.0f;
    if (!validar_float_ok(z->prediccion_no2_24h,  0.0f, 2000.0f))  z->prediccion_no2_24h  = 0.0f;
    if (!validar_float_ok(z->prediccion_so2_24h,  0.0f, 2000.0f))  z->prediccion_so2_24h  = 0.0f;
    if (!validar_float_ok(z->prediccion_co2_24h,  0.0f, 20000.0f)) z->prediccion_co2_24h  = 0.0f;

    if (z->nivel_alerta < 0 || z->nivel_alerta > 3) z->nivel_alerta = 0;
}

void validar_sistema(SistemaMonitoreo *s) {
    int i;
    if (s == NULL) return;

    if (s->num_zonas < 1 || s->num_zonas > NUM_ZONAS_MAX) s->num_zonas = NUM_ZONAS_BASE;
    
    if (s->archivo_historial[0] == '\0') {
        snprintf(s->archivo_historial, sizeof(s->archivo_historial), "historial.bin");
    }
    if (s->archivo_reporte[0] == '\0') {
        snprintf(s->archivo_reporte, sizeof(s->archivo_reporte), "reporte_SIGPCA.txt");
    }

    for (i = 0; i < s->num_zonas; i++) {
        validar_zona(&s->zonas[i], i);
        comparar_con_limites(&s->zonas[i]);
        actualizar_predicciones(&s->zonas[i]);
    }
}

int leer_linea(char *buffer, int size) {
    size_t longitud;
    int c;

    if (buffer == NULL || size <= 0) return 0;

    if (fgets(buffer, size, stdin) == NULL) return -1;

    longitud = strlen(buffer);
    if (longitud > 0 && buffer[longitud - 1] == '\n') {
        buffer[--longitud] = '\0';
    } else if (longitud == (size_t)(size - 1)) {
        while ((c = getchar()) != '\n' && c != EOF);
    }
    return (int)longitud;
}

int pedir_entero_rango(const char *mensaje, int minimo, int maximo) {
    char buffer[128];
    int valor;
    int tmp;

    if (mensaje == NULL) mensaje = "Ingrese un entero: ";
    if (minimo > maximo) { tmp = minimo; minimo = maximo; maximo = tmp; }

    while (1) {
        printf("%s", mensaje);
        fflush(stdout);

        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            clearerr(stdin);
            return maximo;
        }

        if (sscanf(buffer, "%d", &valor) != 1) {
            printf("  Entrada invalida. Ingrese solo numeros enteros.\n");
            continue;
        }

        if (valor < minimo || valor > maximo) {
            printf("  Valor fuera de rango. Debe estar entre %d y %d.\n", minimo, maximo);
            continue;
        }
        return valor;
    }
}

float pedir_flotante_rango(const char *mensaje, float minimo, float maximo) {
    char buffer[128];
    float valor;
    float tmp;

    if (mensaje == NULL) mensaje = "Ingrese un numero: ";
    if (minimo > maximo) { tmp = minimo; minimo = maximo; maximo = tmp; }

    while (1) {
        printf("%s", mensaje);
        fflush(stdout);

        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            clearerr(stdin);
            return maximo;
        }

        if (sscanf(buffer, "%f", &valor) != 1) {
            printf("  Entrada invalida. Use punto decimal.\n");
            continue;
        }

        if (valor < minimo || valor > maximo) {
            printf("  Valor fuera de rango. Debe estar entre %.2f y %.2f.\n", minimo, maximo);
            continue;
        }
        return valor;
    }
}

void leer_nombre(const char *mensaje, char *destino, int tam) {
    int leidos;
    int k;
    int tiene_visible;

    if (destino == NULL || tam <= 0) return;
    if (mensaje == NULL) mensaje = "Ingrese un nombre: ";

    while (1) {
        printf("%s", mensaje);
        fflush(stdout);

        leidos = leer_linea(destino, tam);

        if (leidos < 0) {
            strncpy(destino, "Desconocido", tam - 1);
            destino[tam - 1] = '\0';
            return;
        }

        if (leidos == 0) {
            printf("  El nombre no puede estar vacio.\n");
            continue;
        }

        tiene_visible = 0;
        for (k = 0; k < leidos; k++) {
            if (isprint((unsigned char)destino[k]) && destino[k] != ' ') {
                tiene_visible = 1;
                break;
            }
        }

        if (!tiene_visible) {
            printf("  El nombre debe contener caracteres validos.\n");
            continue;
        }

        destino[tam - 1] = '\0';
        return;
    }
}

void inicializar_datos_base(SistemaMonitoreo *s) {
    int i, j;
    Zona *z;
    float var_no2, var_so2, var_co2;

    if (s == NULL) return;

    s->num_zonas    = NUM_ZONAS_BASE;
    s->num_reportes = 0;

    for (i = 0; i < NUM_ZONAS_BASE; i++) {
        z = &s->zonas[i];

        z->id = i;
        strncpy(z->nombre, nombres_zonas_base[i], sizeof(z->nombre) - 1);
        z->nombre[sizeof(z->nombre) - 1] = '\0';

        z->pm25_actual      = pm25_base[i];
        z->no2_actual       = no2_base[i];
        z->so2_actual       = so2_base[i];
        z->co2_actual       = co2_base[i];
        z->temperatura      = temp_base[i];
        z->velocidad_viento = viento_base[i];
        z->humedad          = humedad_base[i];

        for (j = 0; j < DIAS_HISTORICO; j++) {
            z->historico_pm25[j] = hist_pm25_base[i][j];
        }

        for (j = 0; j < DIAS_HISTORICO; j++) {
            var_no2 = no2_base[i] + ((j % 7) - 3) * 2.0f;
            var_so2 = so2_base[i] + ((j % 6) - 2) * 1.5f;
            var_co2 = co2_base[i] + ((j % 8) - 4) * 15.0f;

            z->historico_no2[j] = (var_no2 > 0.0f) ? var_no2 : 0.0f;
            z->historico_so2[j] = (var_so2 > 0.0f) ? var_so2 : 0.0f;
            z->historico_co2[j] = (var_co2 > 0.0f) ? var_co2 : 0.0f;

            z->historico_temp[j]    = temp_base[i]   + ((j % 5) - 2) * 1.0f;
            z->historico_viento[j]  = viento_base[i] + ((j % 4) - 1) * 2.0f;
            z->historico_humedad[j] = humedad_base[i] + ((j % 6) - 2) * 3.0f;
            if (z->historico_viento[j]  < 0.0f)   z->historico_viento[j]  = 0.0f;
            if (z->historico_humedad[j] < 0.0f)   z->historico_humedad[j] = 0.0f;
            if (z->historico_humedad[j] > 100.0f)  z->historico_humedad[j] = 100.0f;
        }

        z->prediccion_pm25_24h = 0.0f;
        z->prediccion_no2_24h  = 0.0f;
        z->prediccion_so2_24h  = 0.0f;
        z->prediccion_co2_24h  = 0.0f;
        z->nivel_alerta        = 0;

        comparar_con_limites(z);
        actualizar_predicciones(z);
    }
    printf("Historial no encontrado. Se han cargado los datos base.\n");
}

void inicializar_sistema(SistemaMonitoreo *s) {
    if (s == NULL) return;

    s->num_zonas    = NUM_ZONAS_BASE;
    s->num_reportes = 0;

    if (s->archivo_historial[0] == '\0') {
        snprintf(s->archivo_historial, sizeof(s->archivo_historial), "historial.bin");
    }
    if (s->archivo_reporte[0] == '\0') {
        snprintf(s->archivo_reporte, sizeof(s->archivo_reporte), "reporte_SIGPCA.txt");
    }

    cargar_historial(s);
}

void cargar_historial(SistemaMonitoreo *s) {
    FILE *fp;
    int num_guardadas;
    size_t leidos;

    if (s == NULL) return;

    fp = fopen(s->archivo_historial, "rb");
    if (fp == NULL) {
        inicializar_datos_base(s);
        return;
    }

    if (fread(&num_guardadas, sizeof(int), 1, fp) != 1) {
        fclose(fp);
        inicializar_datos_base(s);
        return;
    }

    if (num_guardadas < 1 || num_guardadas > NUM_ZONAS_MAX) {
        fclose(fp);
        inicializar_datos_base(s);
        return;
    }

    leidos = fread(s->zonas, sizeof(Zona), num_guardadas, fp);
    if (leidos != (size_t)num_guardadas || ferror(fp)) {
        fclose(fp);
        inicializar_datos_base(s);
        return;
    }

    fclose(fp);
    s->num_zonas = num_guardadas;
    validar_sistema(s);
    printf("Historial cargado correctamente (%d zonas).\n", s->num_zonas);
}

void guardar_historial(SistemaMonitoreo *s) {
    FILE *fp;
    size_t escritos;

    if (s == NULL) return;

    fp = fopen(s->archivo_historial, "wb");
    if (fp == NULL) {
        printf("Error: no se pudo guardar el historial.\n");
        return;
    }

    if (fwrite(&s->num_zonas, sizeof(int), 1, fp) != 1) {
        fclose(fp);
        return;
    }

    escritos = fwrite(s->zonas, sizeof(Zona), s->num_zonas, fp);
    if (escritos != (size_t)s->num_zonas) {
        fclose(fp);
        return;
    }

    fflush(fp);
    fclose(fp);
}

void ingresar_medicion(Zona *z) {
    if (z == NULL) return;

    printf("\n--- Datos de contaminantes para: %s ---\n", z->nombre);
    z->pm25_actual = pedir_flotante_rango("  PM2.5    (ug/m3, rango 0-500)  : ", 0.0f, 500.0f);
    z->no2_actual  = pedir_flotante_rango("  NO2      (ug/m3, rango 0-500)  : ", 0.0f, 500.0f);
    z->so2_actual  = pedir_flotante_rango("  SO2      (ug/m3, rango 0-500)  : ", 0.0f, 500.0f);
    z->co2_actual  = pedir_flotante_rango("  CO2      (ppm,   rango 0-5000) : ", 0.0f, 5000.0f);

    printf("\n--- Factores climaticos ---\n");
    z->temperatura      = pedir_flotante_rango("  Temperatura       (C,    -50 a 60)  : ", -50.0f, 60.0f);
    z->velocidad_viento = pedir_flotante_rango("  Velocidad viento  (km/h,  0 a 200)  : ", 0.0f, 200.0f);
    z->humedad          = pedir_flotante_rango("  Humedad relativa  (%,     0 a 100)  : ", 0.0f, 100.0f);
}

void actualizar_historial(Zona *z) {
    int i;
    if (z == NULL) return;

    for (i = 0; i < DIAS_HISTORICO - 1; i++) {
        z->historico_pm25[i]    = z->historico_pm25[i + 1];
        z->historico_no2[i]     = z->historico_no2[i + 1];
        z->historico_so2[i]     = z->historico_so2[i + 1];
        z->historico_co2[i]     = z->historico_co2[i + 1];
        z->historico_temp[i]    = z->historico_temp[i + 1];
        z->historico_viento[i]  = z->historico_viento[i + 1];
        z->historico_humedad[i] = z->historico_humedad[i + 1];
    }

    z->historico_pm25[DIAS_HISTORICO - 1]    = z->pm25_actual;
    z->historico_no2[DIAS_HISTORICO - 1]     = z->no2_actual;
    z->historico_so2[DIAS_HISTORICO - 1]     = z->so2_actual;
    z->historico_co2[DIAS_HISTORICO - 1]     = z->co2_actual;
    z->historico_temp[DIAS_HISTORICO - 1]    = z->temperatura;
    z->historico_viento[DIAS_HISTORICO - 1]  = z->velocidad_viento;
    z->historico_humedad[DIAS_HISTORICO - 1] = z->humedad;
}

void agregar_zona(SistemaMonitoreo *s) {
    int i;
    int registrar_datos;
    Zona *z;
    char nombre_nuevo[50]; 

    if (s == NULL) return;

    if (s->num_zonas >= NUM_ZONAS_MAX) {
        printf("No se pueden agregar mas zonas.\n");
        return;
    }

    printf("\n--- Agregar nueva zona de monitoreo ---\n");
    {
        int k, solo_letras;
        do {
            leer_nombre("  Nombre de la nueva zona (solo letras y espacios): ", nombre_nuevo, sizeof(nombre_nuevo));
            solo_letras = 1;
            for (k = 0; nombre_nuevo[k] != '\0'; k++) {
                if (!isalpha((unsigned char)nombre_nuevo[k]) && nombre_nuevo[k] != ' ') {
                    solo_letras = 0;
                    break;
                }
            }
            if (!solo_letras) {
                printf("  El nombre solo puede contener letras y espacios, sin numeros ni simbolos.\n");
            }
        } while (!solo_letras);
    }

    for (i = 0; i < s->num_zonas; i++) {
        if (strcmp(s->zonas[i].nombre, nombre_nuevo) == 0) {
            printf("Ya existe una zona con ese nombre.\n");
            return;
        }
    }

    z = &s->zonas[s->num_zonas];
    memset(z, 0, sizeof(Zona));

    z->id = s->num_zonas;
    strncpy(z->nombre, nombre_nuevo, sizeof(z->nombre) - 1);
    z->nombre[sizeof(z->nombre) - 1] = '\0';

    z->temperatura      = 15.0f;
    z->velocidad_viento = 10.0f;
    z->humedad          = 60.0f;

    registrar_datos = pedir_entero_rango("  Desea registrar medicion actual y datos historicos para esta zona? (1=Si, 2=No): ", 1, 2);
    if (registrar_datos == 1) {
        ingresar_medicion(z);
        registrar_historico_para_zona(z);
        comparar_con_limites(z);
        actualizar_predicciones(z);
    } else {
        comparar_con_limites(z);
        actualizar_predicciones(z);
    }

    s->num_zonas++;
    printf("\nZona '%s' agregada correctamente.\n", z->nombre);
    guardar_historial(s);
    printf("Base de datos actualizada.\n");
}

void eliminar_zona(SistemaMonitoreo *s) {
    int i;
    int seleccion;
    int confirmacion;
    char nombre_eliminada[50];

    if (s == NULL) return;
    if (s->num_zonas <= 1) {
        printf("\nEl sistema requiere al menos una zona.\n");
        return;
    }

    printf("\n--- Eliminar zona de monitoreo ---\n");
    for (i = 0; i < s->num_zonas; i++) {
        printf("  %d. %s\n", i + 1, s->zonas[i].nombre);
    }
    printf("  0. Cancelar\n");

    seleccion = pedir_entero_rango("Seleccione la zona a eliminar: ", 0, s->num_zonas);

    if (seleccion == 0) return;

    strncpy(nombre_eliminada, s->zonas[seleccion - 1].nombre, sizeof(nombre_eliminada) - 1);
    nombre_eliminada[sizeof(nombre_eliminada) - 1] = '\0';

    confirmacion = pedir_entero_rango("Confirma (1=Si, 2=No): ", 1, 2);
    if (confirmacion != 1) return;

    for (i = seleccion - 1; i < s->num_zonas - 1; i++) {
        s->zonas[i] = s->zonas[i + 1];
        s->zonas[i].id = i;
    }
    s->num_zonas--;
    printf("\nZona '%s' eliminada.\n", nombre_eliminada);
    guardar_historial(s);
    printf("Base de datos actualizada.\n");
}

float calcular_promedio_ponderado(float *historico, int n) {
    int i;
    float suma_ponderada = 0.0f;
    float suma_pesos     = 0.0f;
    float peso;

    if (historico == NULL || n <= 0) return 0.0f;

    for (i = 0; i < n; i++) {
        peso = (float)(i + 1);
        suma_ponderada += historico[i] * peso;
        suma_pesos     += peso;
    }

    if (suma_pesos <= 0.0f) return 0.0f;
    return suma_ponderada / suma_pesos;
}

float aplicar_factor_climatico(float pred_base, float viento_kmh, float humedad_pct, float temp_c) {
    float f_viento, f_humedad, f_temp;

    if (pred_base < 0.0f) pred_base = 0.0f;
    if (viento_kmh > 100.0f) viento_kmh = 100.0f;
    if (viento_kmh < 0.0f)   viento_kmh = 0.0f;

    f_viento = 1.0f - (viento_kmh / 100.0f) * 0.30f;
    if (f_viento < 0.70f) f_viento = 0.70f;

    if (humedad_pct > 100.0f) humedad_pct = 100.0f;
    if (humedad_pct < 0.0f)   humedad_pct = 0.0f;
    f_humedad = 1.0f + (humedad_pct / 100.0f) * 0.15f;

    if (temp_c < 10.0f) {
        f_temp = 1.20f;
    } else if (temp_c > 20.0f) {
        f_temp = 0.90f;
    } else {
        f_temp = 1.00f;
    }

    return pred_base * f_viento * f_humedad * f_temp;
}

int clasificar_nivel(float valor, float limite_oms) {
    float porcentaje;
    if (limite_oms <= 0.0f) return 0;
    porcentaje = valor / limite_oms;
    if (porcentaje <= 1.0f) return 0;
    if (porcentaje <= 1.5f) return 1;
    if (porcentaje <= 2.0f) return 2;
    return 3;
}

void comparar_con_limites(Zona *z) {
    int nivel_max;
    if (z == NULL) return;

    nivel_max = clasificar_nivel(z->pm25_actual, LIMITE_PM25_OMS);
    if (clasificar_nivel(z->no2_actual, LIMITE_NO2_OMS) > nivel_max) nivel_max = clasificar_nivel(z->no2_actual, LIMITE_NO2_OMS);
    if (clasificar_nivel(z->so2_actual, LIMITE_SO2_OMS) > nivel_max) nivel_max = clasificar_nivel(z->so2_actual, LIMITE_SO2_OMS);

    if (z->co2_actual > CO2_ALTO_PPM && nivel_max < 1) nivel_max = 1;
    z->nivel_alerta = nivel_max;
}

void actualizar_predicciones(Zona *z) {
    if (z == NULL) return;
    z->prediccion_pm25_24h = aplicar_factor_climatico(calcular_promedio_integrado(z->historico_pm25, DIAS_HISTORICO, z->pm25_actual), z->velocidad_viento, z->humedad, z->temperatura);
    z->prediccion_no2_24h  = aplicar_factor_climatico(calcular_promedio_integrado(z->historico_no2,  DIAS_HISTORICO, z->no2_actual), z->velocidad_viento, z->humedad, z->temperatura);
    z->prediccion_so2_24h  = aplicar_factor_climatico(calcular_promedio_integrado(z->historico_so2,  DIAS_HISTORICO, z->so2_actual), z->velocidad_viento, z->humedad, z->temperatura);
    z->prediccion_co2_24h  = aplicar_factor_climatico(calcular_promedio_integrado(z->historico_co2,  DIAS_HISTORICO, z->co2_actual), z->velocidad_viento, z->humedad, z->temperatura);
}

void generar_recomendaciones(Zona *z, char *mensaje, int tam_mensaje) {
    if (z == NULL || mensaje == NULL || tam_mensaje <= 0) return;
    
    if (z->nivel_alerta == 0) {
        snprintf(mensaje, tam_mensaje, "Ningun contaminante supera limites. Mantener condiciones actuales.");
    } else if (z->nivel_alerta == 1) {
        snprintf(mensaje, tam_mensaje, "PRECAUCION: Reduccion del trafico vehicular e incentivar transporte publico.");
    } else if (z->nivel_alerta == 2) {
        snprintf(mensaje, tam_mensaje, "ALERTA: Suspender actividades al aire libre y aplicar restricciones de movilidad.");
    } else {
        snprintf(mensaje, tam_mensaje, "PELIGRO: Cierre temporal de industrias y suspension de actividades. Alerta sanitaria.");
    }
}

void mostrar_resumen_zonas(SistemaMonitoreo *s) {
    int i;
    Zona *z;
    if (s == NULL) return;

    printf("\n+----+------------------+----------+----------+----------+----------+------------+\n");
    printf("| ID | Zona             | PM2.5    | NO2      | SO2      | CO2      | Alerta     |\n");
    printf("+----+------------------+----------+----------+----------+----------+------------+\n");

    for (i = 0; i < s->num_zonas; i++) {
        z = &s->zonas[i];
        if (z->pm25_actual == 0.0f && z->no2_actual == 0.0f &&
            z->so2_actual  == 0.0f && z->co2_actual == 0.0f) {
            printf("| %2d | %-16s | %-8s | %-8s | %-8s | %-8s | %-10s |\n",
                z->id, z->nombre,
                "---", "---", "---", "---",
                texto_nivel(z->nivel_alerta));
        } else {
            printf("| %2d | %-16s | %6.2f   | %6.2f   | %6.2f   | %6.1f   | %-10s |\n",
                z->id, z->nombre,
                z->pm25_actual, z->no2_actual,
                z->so2_actual,  z->co2_actual,
                texto_nivel(z->nivel_alerta));
        }
    }
    printf("+----+------------------+----------+----------+----------+----------+------------+\n");

    printf("\n  Zonas sin medicion actual registrada:\n");
    for (i = 0; i < s->num_zonas; i++) {
        z = &s->zonas[i];
        if (z->pm25_actual == 0.0f && z->no2_actual == 0.0f &&
            z->so2_actual  == 0.0f && z->co2_actual == 0.0f) {
            printf("  - %s: aun no se han ingresado mediciones actuales para esta zona.\n", z->nombre);
        }
    }
}

void mostrar_datos_zona(Zona *z) {
    if (z == NULL) return;
    printf("\n  Datos registrados para: %s\n", z->nombre);
    printf("  +------------------+----------+----------+\n");
    printf("  | Contaminante     | Valor    | Limite   |\n");
    printf("  +------------------+----------+----------+\n");
    printf("  | PM2.5  (ug/m3)   | %8.2f | %8.1f |\n", z->pm25_actual, LIMITE_PM25_OMS);
    printf("  | NO2    (ug/m3)   | %8.2f | %8.1f |\n", z->no2_actual,  LIMITE_NO2_OMS);
    printf("  | SO2    (ug/m3)   | %8.2f | %8.1f |\n", z->so2_actual,  LIMITE_SO2_OMS);
    printf("  | CO2    (ppm)     | %8.1f | %8.1f |\n", z->co2_actual,  CO2_ALTO_PPM);
    printf("  +------------------+----------+----------+\n");
    printf("  Temp: %.1f C | Viento: %.1f km/h | Humedad: %.1f%%\n",
           z->temperatura, z->velocidad_viento, z->humedad);
    printf("  Nivel de alerta: %s\n", texto_nivel(z->nivel_alerta));
}

void exportar_reporte(SistemaMonitoreo *s) {
    FILE *fp;
    int i;
    char fecha[20], hora[10];
    char nombre_archivo[120];
    int numero;

    if (s == NULL) return;

    /* Generar nombre unico: reporte_SIGPCA_001.txt, _002.txt, etc. */
    numero = 1;
    do {
        snprintf(nombre_archivo, sizeof(nombre_archivo), "reporte_SIGPCA_%03d.txt", numero);
        fp = fopen(nombre_archivo, "r");
        if (fp != NULL) {
            fclose(fp);
            numero++;
        }
    } while (fp != NULL && numero < 9999);

    fp = fopen(nombre_archivo, "w");
    if (fp == NULL) {
        printf("Error al crear el archivo de reporte.\n");
        return;
    }

    obtener_fecha_hora(fecha, sizeof(fecha), hora, sizeof(hora));

    fprintf(fp, "=================================================\n");
    fprintf(fp, "  SIGPCA - Reporte de Monitoreo del Aire\n");
    fprintf(fp, "  Fecha: %s  Hora: %s\n", fecha, hora);
    fprintf(fp, "  Zonas monitoreadas: %d\n", s->num_zonas);
    fprintf(fp, "=================================================\n\n");

    fprintf(fp, "+------------------+----------+----------+----------+----------+------------+\n");
    fprintf(fp, "| Zona             | PM2.5    | NO2      | SO2      | CO2      | Alerta     |\n");
    fprintf(fp, "+------------------+----------+----------+----------+----------+------------+\n");
    for (i = 0; i < s->num_zonas; i++) {
        fprintf(fp, "| %-16s | %8.2f | %8.2f | %8.2f | %8.1f | %-10s |\n",
                s->zonas[i].nombre,
                s->zonas[i].pm25_actual, s->zonas[i].no2_actual,
                s->zonas[i].so2_actual,  s->zonas[i].co2_actual,
                texto_nivel(s->zonas[i].nivel_alerta));
    }
    fprintf(fp, "+------------------+----------+----------+----------+----------+------------+\n\n");

    fprintf(fp, "--- Prediccion a 24 horas ---\n");
    fprintf(fp, "+------------------+---------+---------+---------+-----------+\n");
    fprintf(fp, "| Zona             | PM2.5   | NO2     | SO2     | CO2       |\n");
    fprintf(fp, "+------------------+---------+---------+---------+-----------+\n");
    for (i = 0; i < s->num_zonas; i++) {
        fprintf(fp, "| %-16s | %7.2f | %7.2f | %7.2f | %9.1f |\n",
                s->zonas[i].nombre,
                s->zonas[i].prediccion_pm25_24h, s->zonas[i].prediccion_no2_24h,
                s->zonas[i].prediccion_so2_24h,  s->zonas[i].prediccion_co2_24h);
    }
    fprintf(fp, "+------------------+---------+---------+---------+-----------+\n\n");

    fprintf(fp, "Limites OMS: PM2.5=%.0f | NO2=%.0f | SO2=%.0f | CO2 alto=%.0f\n",
            LIMITE_PM25_OMS, LIMITE_NO2_OMS, LIMITE_SO2_OMS, CO2_ALTO_PPM);

    fclose(fp);
    printf("Reporte exportado correctamente en '%s'.\n", nombre_archivo);
}