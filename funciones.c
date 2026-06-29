#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "funciones.h"

const float LIMITE_PM25_OMS = 15.0f;
const float LIMITE_NO2_OMS  = 25.0f;
const float LIMITE_SO2_OMS  = 40.0f;
const float CO2_ALTO_PPM    = 1000.0f;

const char *nombres_zonas_base[5] = {
    "El Camal", "Belisario", "Cotocollao", "Tumbaco", "Centro Historico"
};

const float pm25_base[5]    = { 42.0f, 24.0f, 28.0f, 12.0f, 32.0f };
const float no2_base[5]     = { 48.0f, 32.0f, 30.0f, 12.0f, 45.0f };
const float so2_base[5]     = { 22.0f, 18.0f, 25.0f,  8.0f, 20.0f };
const float co2_base[5]     = { 820.0f, 620.0f, 540.0f, 435.0f, 710.0f };
const float temp_base[5]    = { 16.0f, 15.0f, 14.0f, 19.0f, 14.0f };
const float viento_base[5]  = { 10.0f, 14.0f, 12.0f, 22.0f,  8.0f };
const float humedad_base[5] = { 68.0f, 63.0f, 72.0f, 53.0f, 66.0f };

const float hist_pm25_base[5][30] = {
    {42,38,45,40,33,36,41,44,39,35,43,46,38,41,37,44,40,35,38,42,45,41,36,39,43,40,37,44,41,38},
    {24,21,28,25,19,22,26,30,23,18,27,29,24,22,20,28,25,21,23,27,30,26,22,25,28,24,21,27,25,22},
    {28,25,32,28,22,26,30,34,27,23,31,33,28,26,24,32,29,25,27,31,34,30,26,29,32,28,25,31,29,26},
    {12,10,14,11, 9,11,13,15,11, 9,13,15,12,11,10,14,12,10,11,13,16,13,11,12,15,12,10,14,12,11},
    {32,28,36,31,25,29,33,38,30,26,35,37,31,29,27,35,32,28,30,34,37,33,29,32,35,31,28,34,32,29}
};


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
        z->historico_pm25[i] = z->historico_pm25[i + 1];
        z->historico_no2[i]  = z->historico_no2[i + 1];
        z->historico_so2[i]  = z->historico_so2[i + 1];
        z->historico_co2[i]  = z->historico_co2[i + 1];
    }

    z->historico_pm25[DIAS_HISTORICO - 1] = z->pm25_actual;
    z->historico_no2[DIAS_HISTORICO - 1]  = z->no2_actual;
    z->historico_so2[DIAS_HISTORICO - 1]  = z->so2_actual;
    z->historico_co2[DIAS_HISTORICO - 1]  = z->co2_actual;
}

void agregar_zona(SistemaMonitoreo *s) {
    int i;
    Zona *z;
    char nombre_nuevo[50]; 

    if (s == NULL) return;

    if (s->num_zonas >= NUM_ZONAS_MAX) {
        printf("No se pueden agregar mas zonas.\n");
        return;
    }

    printf("\n--- Agregar nueva zona de monitoreo ---\n");
    leer_nombre("  Nombre de la nueva zona: ", nombre_nuevo, sizeof(nombre_nuevo));

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

    s->num_zonas++;
    printf("\nZona '%s' agregada correctamente.\n", z->nombre);
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
    z->prediccion_pm25_24h = aplicar_factor_climatico(calcular_promedio_ponderado(z->historico_pm25, DIAS_HISTORICO), z->velocidad_viento, z->humedad, z->temperatura);
    z->prediccion_no2_24h  = aplicar_factor_climatico(calcular_promedio_ponderado(z->historico_no2,  DIAS_HISTORICO), z->velocidad_viento, z->humedad, z->temperatura);
    z->prediccion_so2_24h  = aplicar_factor_climatico(calcular_promedio_ponderado(z->historico_so2,  DIAS_HISTORICO), z->velocidad_viento, z->humedad, z->temperatura);
    z->prediccion_co2_24h  = aplicar_factor_climatico(calcular_promedio_ponderado(z->historico_co2,  DIAS_HISTORICO), z->velocidad_viento, z->humedad, z->temperatura);
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
    if (s == NULL) return;
    printf("\nResumen Zonas Monitoreadas:\n");
    for (i = 0; i < s->num_zonas; i++) {
        printf("ID: %2d | %s | Alerta: %s\n", s->zonas[i].id, s->zonas[i].nombre, texto_nivel(s->zonas[i].nivel_alerta));
    }
}

void mostrar_datos_zona(Zona *z) {
    if (z == NULL) return;
    printf("\nDatos de %s: PM2.5=%.2f, NO2=%.2f, SO2=%.2f, CO2=%.2f, Alerta=%s\n", z->nombre, z->pm25_actual, z->no2_actual, z->so2_actual, z->co2_actual, texto_nivel(z->nivel_alerta));
}

void exportar_reporte(SistemaMonitoreo *s) {
    FILE *fp;
    int i;
    char fecha[20], hora[10];

    if (s == NULL) return;
    fp = fopen(s->archivo_reporte, "a");
    if (fp == NULL) {
        printf("Error al abrir el archivo de reporte.\n");
        return;
    }

    obtener_fecha_hora(fecha, sizeof(fecha), hora, sizeof(hora));
    fprintf(fp, "\n=== REPORTE %s %s ===\nZonas monitoreadas: %d\n", fecha, hora, s->num_zonas);

    for (i = 0; i < s->num_zonas; i++) {
        fprintf(fp, "ZONA: %s [ID:%d]\n", s->zonas[i].nombre, s->zonas[i].id);
        fprintf(fp, "  PM2.5: %.2f | NO2: %.2f | SO2: %.2f | CO2: %.2f\n", 
                s->zonas[i].pm25_actual, s->zonas[i].no2_actual, s->zonas[i].so2_actual, s->zonas[i].co2_actual);
        fprintf(fp, "  Pred. 24h -> PM2.5: %.2f | NO2: %.2f | SO2: %.2f\n",
                s->zonas[i].prediccion_pm25_24h, s->zonas[i].prediccion_no2_24h, s->zonas[i].prediccion_so2_24h);
        fprintf(fp, "  Nivel Alerta: %s\n", texto_nivel(s->zonas[i].nivel_alerta));
        fprintf(fp, "----------------------------------------\n");
    }

    fclose(fp);
    printf("Reporte exportado correctamente en '%s'.\n", s->archivo_reporte);
}