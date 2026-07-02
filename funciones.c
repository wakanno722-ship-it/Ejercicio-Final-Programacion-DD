#include "funciones.h"

Zona     zonas[MAX_ZONAS];
Medicion mediciones[MAX_MEDICIONES];
int      cantidadZonas       = 0;
int      cantidadMediciones  = 0;

static void filaSep(int *anchos, int n) {
    int i, j;
    printf("+");
    for (i = 0; i < n; i++) {
        for (j = 0; j < anchos[i] + 2; j++) printf("-");
        printf("+");
    }
    printf("\n");
}

static void filaTextos(const char **textos, int *anchos, int n) {
    int i, j, len, izq, der;
    printf("|");
    for (i = 0; i < n; i++) {
        len = (int)strlen(textos[i]);
        izq = (anchos[i] - len) / 2;
        der = anchos[i] - len - izq;
        printf(" ");
        for (j = 0; j < izq; j++) printf(" ");
        printf("%s", textos[i]);
        for (j = 0; j < der; j++) printf(" ");
        printf(" |");
    }
    printf("\n");
}

void limpiarBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void leerCadena(char *dest, int tam) {
    int i = 0, c;
    while (i < tam - 1) {
        c = getchar();
        if (c == '\n' || c == EOF) break;
        dest[i++] = (char)c;
    }
    dest[i] = '\0';
}

int pedirEnteroRango(const char *msg, int a, int b) {
    int v;
    char buf[64];
    while (1) {
        printf("%s", msg);
        if (fgets(buf, sizeof(buf), stdin) == NULL) { v = a; break; }
        if (sscanf(buf, "%d", &v) == 1 && v >= a && v <= b) break;
        printf("  Valor invalido. Ingrese un numero entre %d y %d.\n", a, b);
    }
    return v;
}

float pedirFlotanteRango(const char *msg, float a, float b) {
    float v;
    char buf[64];
    while (1) {
        printf("%s", msg);
        if (fgets(buf, sizeof(buf), stdin) == NULL) { v = a; break; }
        if (sscanf(buf, "%f", &v) == 1 && v >= a && v <= b) break;
        printf("  Valor invalido. Ingrese un numero entre %.1f y %.1f.\n", a, b);
    }
    return v;
}

void obtenerFechaHoy(char *buf, int tam) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    snprintf(buf, tam, "%02d/%02d/%04d",
             tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900);
}

static int validarFecha(const char *f) {
    int d, m, a;
    if (strlen(f) != 10) return 0;
    if (f[2] != '/' || f[5] != '/') return 0;
    if (sscanf(f, "%d/%d/%d", &d, &m, &a) != 3) return 0;
    if (d < 1 || d > 31) return 0;
    if (m < 1 || m > 12) return 0;
    if (a < 1900 || a > 2100) return 0;
    return 1;
}

static void pedirFecha(char *buf) {
    char tmp[64];
    while (1) {
        printf("  Fecha (dd/mm/aaaa): ");
        leerCadena(tmp, sizeof(tmp));
        if (validarFecha(tmp)) { strncpy(buf, tmp, 11); buf[10] = '\0'; break; }
        printf("  Formato incorrecto. Use dd/mm/aaaa.\n");
    }
}

int menu(void) {
    printf("\n============================================\n");
    printf("   SIGPCA - Sistema de Monitoreo del Aire  \n");
    printf("============================================\n");
    printf("  1.  Registrar zona\n");
    printf("  2.  Listar zonas\n");
    printf("  3.  Eliminar zona\n");
    printf("  4.  Registrar medicion actual\n");
    printf("  5.  Agregar medicion historica\n");
    printf("  6.  Monitorear contaminacion actual\n");
    printf("  7.  Ver alertas y recomendaciones\n");
    printf("  8.  Prediccion a 24 horas\n");
    printf("  9.  Promedio historico 30 dias\n");
    printf("  10. Generar reporte\n");
    printf("  11. Salir\n");
    printf("--------------------------------------------\n");
    return pedirEnteroRango("Seleccione una opcion: ", 1, 11);
}

int buscarZonaPorNombre(const char *nombre) {
    int i;
    for (i = 0; i < cantidadZonas; i++) {
        if (strcmp(zonas[i].nombre, nombre) == 0) return i;
    }
    return -1;
}

void mostrarZonasNumeradas(void) {
    int i;
    int anchos[2] = {4, 30};
    const char *hdr[2] = {"#", "Zona"};
    char numBuf[16], nombreBuf[50];
    const char *fila[2];

    if (cantidadZonas == 0) {
        printf("  No hay zonas registradas.\n");
        return;
    }
    filaSep(anchos, 2);
    filaTextos(hdr, anchos, 2);
    filaSep(anchos, 2);
    for (i = 0; i < cantidadZonas; i++) {
        snprintf(numBuf, sizeof(numBuf), "%d", i + 1);
        snprintf(nombreBuf, sizeof(nombreBuf), "%s", zonas[i].nombre);
        fila[0] = numBuf;
        fila[1] = nombreBuf;
        filaTextos(fila, anchos, 2);
    }
    filaSep(anchos, 2);
}

int seleccionarZona(void) {
    int sel;
    mostrarZonasNumeradas();
    if (cantidadZonas == 0) return -1;
    sel = pedirEnteroRango("  Seleccione zona (numero): ", 1, cantidadZonas);
    return sel - 1;
}

void registrarZona(void) {
    char nombre[50];
    int i, ok, conf;

    printf("\n--- Registrar nueva zona ---\n");
    if (cantidadZonas >= MAX_ZONAS) {
        printf("  Limite de zonas alcanzado (%d).\n", MAX_ZONAS);
        return;
    }

    while (1) {
        printf("  Nombre de la zona: ");
        leerCadena(nombre, sizeof(nombre));
        if (strlen(nombre) == 0) { printf("  El nombre no puede estar vacio.\n"); continue; }

        ok = 1;
        for (i = 0; nombre[i] != '\0'; i++) {
            if (!isalpha((unsigned char)nombre[i]) && nombre[i] != ' ') {
                ok = 0; break;
            }
        }
        if (!ok) { printf("  Solo letras y espacios.\n"); continue; }
        if (buscarZonaPorNombre(nombre) >= 0) { printf("  Ya existe una zona con ese nombre.\n"); continue; }

        conf = pedirEnteroRango("  ¿El nombre ingresado es correcto? (1=Si, 2=No): ", 1, 2);
        if (conf == 1) {
            break; 
        } else {
            printf("  Vuelva a ingresar el nombre de la zona.\n");
        }
    }

    zonas[cantidadZonas].id = cantidadZonas;
    strncpy(zonas[cantidadZonas].nombre, nombre, 49);
    zonas[cantidadZonas].nombre[49] = '\0';
    cantidadZonas++;
    printf("  Zona '%s' registrada correctamente.\n", nombre);
}

void listarZonas(void) {
    printf("\n--- Zonas registradas ---\n");
    mostrarZonasNumeradas();
}

void eliminarZona(void) {
    int idx, i, conf;
    char nombre[50];

    printf("\n--- Eliminar zona ---\n");
    if (cantidadZonas == 0) { printf("  No hay zonas registradas.\n"); return; }
    if (cantidadZonas <= 1) { printf("  El sistema requiere al menos una zona.\n"); return; }

    idx = seleccionarZona();
    if (idx < 0) return;

    strncpy(nombre, zonas[idx].nombre, 49);
    nombre[49] = '\0';

    conf = pedirEnteroRango("  Confirmar eliminacion (1=Si, 2=No): ", 1, 2);
    if (conf != 1) { printf("  Cancelado.\n"); return; }

    i = 0;
    while (i < cantidadMediciones) {
        if (strcmp(mediciones[i].zonaNombre, nombre) == 0) {
            mediciones[i] = mediciones[cantidadMediciones - 1];
            cantidadMediciones--;
        } else {
            i++;
        }
    }

    for (i = idx; i < cantidadZonas - 1; i++) {
        zonas[i] = zonas[i + 1];
        zonas[i].id = i;
    }
    cantidadZonas--;
    printf("  Zona '%s' eliminada.\n", nombre);
}

static void ingresarDatosMedicion(Medicion *m) {
    m->pm25            = pedirFlotanteRango("  PM2.5     (ug/m3, 0-1000) : ", 0.0f, 1000.0f);
    m->no2             = pedirFlotanteRango("  NO2       (ug/m3, 0-1000) : ", 0.0f, 1000.0f);
    m->so2             = pedirFlotanteRango("  SO2       (ug/m3, 0-1000) : ", 0.0f, 1000.0f);
    m->co2             = pedirFlotanteRango("  CO2       (ppm, 0-50000)  : ", 0.0f, 50000.0f);
    m->temperatura     = pedirFlotanteRango("  Temp.     (C, -15 a 45)   : ", -15.0f, 45.0f);
    m->humedad         = pedirFlotanteRango("  Humedad   (%%, 0-100)      : ", 0.0f, 100.0f);
    m->velocidadViento = pedirFlotanteRango("  Viento    (km/h, 0-150)   : ", 0.0f, 150.0f);
}

void registrarMedicionActual(void) {
    int idx;
    char fechaHoy[12];
    Medicion nueva;

    printf("\n--- Registrar medicion actual (hoy) ---\n");
    if (cantidadZonas == 0) { printf("  No hay zonas registradas.\n"); return; }

    idx = seleccionarZona();
    if (idx < 0) return;

    obtenerFechaHoy(fechaHoy, sizeof(fechaHoy));

    {
        int i;
        for (i = 0; i < cantidadMediciones; i++) {
            if (strcmp(mediciones[i].zonaNombre, zonas[idx].nombre) == 0 &&
                strcmp(mediciones[i].fecha, fechaHoy) == 0) {
                printf("  Ya existe una medicion para '%s' en la fecha %s.\n",
                       zonas[idx].nombre, fechaHoy);
                printf("  Desea sobreescribirla? (1=Si, 2=No): ");
                {
                    int conf = pedirEnteroRango("", 1, 2);
                    if (conf != 1) { printf("  Cancelado.\n"); return; }
                    printf("  --- Ingrese los nuevos datos de contaminacion ---\n");
                    ingresarDatosMedicion(&mediciones[i]);
                    printf("  Medicion actualizada para '%s' - %s.\n",
                           zonas[idx].nombre, fechaHoy);
                    return;
                }
            }
        }
    }

    if (cantidadMediciones >= MAX_MEDICIONES) {
        printf("  Limite de mediciones alcanzado.\n"); return;
    }

    memset(&nueva, 0, sizeof(Medicion));
    strncpy(nueva.zonaNombre, zonas[idx].nombre, 49);
    strncpy(nueva.fecha, fechaHoy, 11);

    printf("\n  Zona: %s  |  Fecha: %s\n", zonas[idx].nombre, fechaHoy);
    printf("  --- Ingrese los datos de contaminacion ---\n");
    ingresarDatosMedicion(&nueva);

    mediciones[cantidadMediciones++] = nueva;
    printf("  Medicion registrada correctamente para '%s' - %s.\n",
           zonas[idx].nombre, fechaHoy);
}

void agregarMedicionHistorica(void) {
    int idx, i;
    char fecha[12];
    Medicion nueva;

    printf("\n--- Agregar medicion historica ---\n");
    if (cantidadZonas == 0) { printf("  No hay zonas registradas.\n"); return; }

    idx = seleccionarZona();
    if (idx < 0) return;

    pedirFecha(fecha);

    /* Verificar si ya hay datos para esa fecha y permitir sobreescritura */
    for (i = 0; i < cantidadMediciones; i++) {
        if (strcmp(mediciones[i].zonaNombre, zonas[idx].nombre) == 0 &&
            strcmp(mediciones[i].fecha, fecha) == 0) {
            printf("  Ya existe una medicion para '%s' en la fecha %s.\n", zonas[idx].nombre, fecha);
            int conf = pedirEnteroRango("  ¿Desea sobreescribirla? (1=Si, 2=No): ", 1, 2);
            if (conf != 1) { 
                printf("  Cancelado.\n"); 
                return; 
            }
            printf("  --- Ingrese los nuevos datos para %s ---\n", fecha);
            ingresarDatosMedicion(&mediciones[i]);
            printf("  Medicion historica actualizada.\n");
            return;
        }
    }

    if (cantidadMediciones >= MAX_MEDICIONES) {
        printf("  Limite de mediciones alcanzado.\n"); return;
    }

    memset(&nueva, 0, sizeof(Medicion));
    strncpy(nueva.zonaNombre, zonas[idx].nombre, 49);
    strncpy(nueva.fecha, fecha, 11);

    printf("\n  Zona: %s  |  Fecha: %s\n", zonas[idx].nombre, fecha);
    printf("  --- Ingrese los datos de contaminacion ---\n");
    ingresarDatosMedicion(&nueva);

    mediciones[cantidadMediciones++] = nueva;
    printf("  Medicion historica registrada para '%s' - %s.\n",
           zonas[idx].nombre, fecha);
}

const char *nivelPM25(float v) {
    if (v <= LIMITE_PM25)          return "NORMAL";
    if (v <= LIMITE_PM25 * 2.0f)  return "MODERADO";
    return "ALTO";
}
const char *nivelNO2(float v) {
    if (v <= LIMITE_NO2)          return "NORMAL";
    if (v <= LIMITE_NO2 * 2.0f)  return "MODERADO";
    return "ALTO";
}
const char *nivelSO2(float v) {
    if (v <= LIMITE_SO2)          return "NORMAL";
    if (v <= LIMITE_SO2 * 2.0f)  return "MODERADO";
    return "ALTO";
}
const char *nivelCO2(float v) {
    if (v <= LIMITE_CO2)          return "NORMAL";
    if (v <= LIMITE_CO2 * 1.5f)  return "MODERADO";
    return "ALTO";
}
const char *nivelTemp(float v) {
    if (v < TEMP_MIN)  return "MUY BAJA";
    if (v > TEMP_MAX)  return "MUY ALTA";
    return "NORMAL";
}
const char *nivelHumedad(float v) {
    if (v < HUMEDAD_MIN)  return "MUY BAJA";
    if (v > HUMEDAD_MAX)  return "MUY ALTA";
    return "NORMAL";
}
const char *nivelViento(float v) {
    if (v < VIENTO_MIN)  return "MUY BAJO";
    if (v > VIENTO_MAX)  return "MUY ALTO";
    return "NORMAL";
}

const char *recPM25(float v) {
    if (v <= LIMITE_PM25)         return "Niveles seguros. Mantener condiciones actuales.";
    if (v <= LIMITE_PM25 * 2.0f) return "Reducir trafico vehicular y evitar quema de materiales.";
    return "Restriccion de circulacion. Uso de mascarilla obligatorio. Alerta sanitaria.";
}
const char *recNO2(float v) {
    if (v <= LIMITE_NO2)         return "Niveles aceptables. Sin medidas adicionales.";
    if (v <= LIMITE_NO2 * 2.0f) return "Incentivar transporte publico. Revision de emisiones industriales.";
    return "Suspension de actividades exteriores. Cierre temporal de industrias contaminantes.";
}
const char *recSO2(float v) {
    if (v <= LIMITE_SO2)         return "Niveles seguros. Monitoreo continuo.";
    if (v <= LIMITE_SO2 * 2.0f) return "Revisar emisiones de plantas industriales y combustibles.";
    return "Alerta maxima. Evacuacion de zonas criticas y cierre industrial inmediato.";
}
const char *recCO2(float v) {
    if (v <= LIMITE_CO2)         return "Concentracion normal. Ventilacion adecuada.";
    if (v <= LIMITE_CO2 * 1.5f) return "Aumentar ventilacion. Reducir uso de vehiculos y calefaccion.";
    return "Nivel critico. Ventilacion urgente. Evacuar espacios cerrados afectados.";
}
const char *recTemp(float v) {
    if (v < TEMP_MIN)  return "Temperatura muy baja. Riesgo de hipotermia. Ropa abrigada obligatoria.";
    if (v > TEMP_MAX)  return "Temperatura muy alta. Evitar exposicion solar directa. Hidratacion constante.";
    return "Temperatura dentro del rango saludable.";
}
const char *recHumedad(float v) {
    if (v < HUMEDAD_MIN)  return "Humedad muy baja. Riesgo de irritacion respiratoria y piel seca.";
    if (v > HUMEDAD_MAX)  return "Humedad muy alta. Propicio para moho y enfermedades respiratorias.";
    return "Humedad dentro del rango saludable.";
}
const char *recViento(float v) {
    if (v < VIENTO_MIN)  return "Viento muy debil. Contaminantes se acumulan. Mayor monitoreo requerido.";
    if (v > VIENTO_MAX)  return "Viento muy fuerte. Riesgo de accidentes y dispersion de particulas.";
    return "Viento en rango normal. Favorece la dispersion de contaminantes.";
}

void monitorearContaminacion(void) {
    int idx, i;
    char fechaHoy[12];
    Medicion *m = NULL;
    char buf[32];

    int anchos[6] = {16, 10, 10, 10, 10, 10};
    const char *hdr[6] = {"Zona", "PM2.5", "NO2", "SO2", "CO2", "Alerta"};
    const char *fila[6];
    char pm[16], no2[16], so2[16], co2[16];

    printf("\n--- Monitoreo de contaminacion actual ---\n");
    if (cantidadZonas == 0) { printf("  No hay zonas registradas.\n"); return; }

    idx = seleccionarZona();
    if (idx < 0) return;

    obtenerFechaHoy(fechaHoy, sizeof(fechaHoy));

    for (i = 0; i < cantidadMediciones; i++) {
        if (strcmp(mediciones[i].zonaNombre, zonas[idx].nombre) == 0 &&
            strcmp(mediciones[i].fecha, fechaHoy) == 0) {
            m = &mediciones[i];
            break;
        }
    }

    if (m == NULL) {
        printf("  No hay medicion registrada hoy (%s) para '%s'.\n",
               fechaHoy, zonas[idx].nombre);
        return;
    }

    printf("\n  Zona: %s  |  Fecha: %s\n\n", m->zonaNombre, m->fecha);

    printf("  Contaminantes:\n");
    filaSep(anchos, 6);
    filaTextos(hdr, anchos, 6);
    filaSep(anchos, 6);

    {
        const char *niv;
        const char *niveles[4];
        const char *alerta;

        niveles[0] = nivelPM25(m->pm25);
        niveles[1] = nivelNO2(m->no2);
        niveles[2] = nivelSO2(m->so2);
        niveles[3] = nivelCO2(m->co2);

        alerta = "NORMAL";
        for (i = 0; i < 4; i++) {
            niv = niveles[i];
            if (strcmp(niv, "ALTO") == 0)     { alerta = "ALTO";     break; }
            if (strcmp(niv, "MODERADO") == 0)   alerta = "MODERADO";
        }

        snprintf(pm,  sizeof(pm),  "%.2f", m->pm25);
        snprintf(no2, sizeof(no2), "%.2f", m->no2);
        snprintf(so2, sizeof(so2), "%.2f", m->so2);
        snprintf(co2, sizeof(co2), "%.1f", m->co2);

        fila[0] = m->zonaNombre; fila[1] = pm; fila[2] = no2;
        fila[3] = so2; fila[4] = co2; fila[5] = alerta;
        filaTextos(fila, anchos, 6);
        filaSep(anchos, 6);

        printf("  Limites OMS Alerta: PM2.5=%.0f | NO2=%.0f | SO2=%.0f | CO2=%.0f (ug/m3 / ppm)\n\n",
               LIMITE_PM25, LIMITE_NO2, LIMITE_SO2, LIMITE_CO2);
    }

    {
        int aw[4] = {18, 14, 14, 14};
        const char *ah[4] = {"Factor", "Valor", "Estado", "Rango saludable"};
        const char *af[4];
        char vbuf[16];

        printf("  Factores ambientales:\n");
        filaSep(aw, 4);
        filaTextos(ah, aw, 4);
        filaSep(aw, 4);

        snprintf(vbuf, sizeof(vbuf), "%.1f C", m->temperatura);
        af[0]="Temperatura"; af[1]=vbuf; af[2]=nivelTemp(m->temperatura); af[3]="5 C - 30 C";
        filaTextos(af, aw, 4);

        snprintf(vbuf, sizeof(vbuf), "%.1f %%", m->humedad);
        af[0]="Humedad"; af[1]=vbuf; af[2]=nivelHumedad(m->humedad); af[3]="30% - 70%";
        filaTextos(af, aw, 4);

        snprintf(buf, sizeof(buf), "%.1f km/h", m->velocidadViento);
        af[0]="Viento"; af[1]=buf; af[2]=nivelViento(m->velocidadViento); af[3]="2 - 50 km/h";
        filaTextos(af, aw, 4);

        filaSep(aw, 4);
    }
}

void mostrarAlertasRecomendaciones(void) {
    int idx, i;
    char fechaHoy[12];
    Medicion *m = NULL;

    int aw[3] = {16, 10, 46};
    const char *ah[3] = {"Contaminante", "Nivel", "Recomendacion"};
    const char *af[3];

    printf("\n--- Alertas y recomendaciones ---\n");
    if (cantidadZonas == 0) { printf("  No hay zonas registradas.\n"); return; }

    idx = seleccionarZona();
    if (idx < 0) return;

    obtenerFechaHoy(fechaHoy, sizeof(fechaHoy));

    for (i = 0; i < cantidadMediciones; i++) {
        if (strcmp(mediciones[i].zonaNombre, zonas[idx].nombre) == 0 &&
            strcmp(mediciones[i].fecha, fechaHoy) == 0) {
            m = &mediciones[i];
            break;
        }
    }

    if (m == NULL) {
        printf("  No hay medicion registrada hoy (%s) para '%s'.\n",
               fechaHoy, zonas[idx].nombre);
        return;
    }

    printf("\n  Zona: %s  |  Fecha: %s\n\n", m->zonaNombre, m->fecha);

    printf("  Alertas de contaminantes:\n");
    filaSep(aw, 3);
    filaTextos(ah, aw, 3);
    filaSep(aw, 3);

    af[0]="PM2.5";  af[1]=nivelPM25(m->pm25); af[2]=recPM25(m->pm25);   filaTextos(af,aw,3);
    af[0]="NO2";    af[1]=nivelNO2(m->no2);   af[2]=recNO2(m->no2);     filaTextos(af,aw,3);
    af[0]="SO2";    af[1]=nivelSO2(m->so2);   af[2]=recSO2(m->so2);     filaTextos(af,aw,3);
    af[0]="CO2";    af[1]=nivelCO2(m->co2);   af[2]=recCO2(m->co2);     filaTextos(af,aw,3);
    filaSep(aw, 3);

    printf("\n  Alertas de factores ambientales:\n");
    filaSep(aw, 3);
    filaTextos(ah, aw, 3);
    filaSep(aw, 3);

    af[0]="Temperatura"; af[1]=nivelTemp(m->temperatura);       af[2]=recTemp(m->temperatura);       filaTextos(af,aw,3);
    af[0]="Humedad";     af[1]=nivelHumedad(m->humedad);        af[2]=recHumedad(m->humedad);        filaTextos(af,aw,3);
    af[0]="Viento";      af[1]=nivelViento(m->velocidadViento); af[2]=recViento(m->velocidadViento); filaTextos(af,aw,3);
    filaSep(aw, 3);
}

float obtenerCampoMedicion(Medicion *m, int campo) {
    switch (campo) {
        case CAMPO_PM25:   return m->pm25;
        case CAMPO_NO2:    return m->no2;
        case CAMPO_SO2:    return m->so2;
        case CAMPO_CO2:    return m->co2;
        case CAMPO_TEMP:   return m->temperatura;
        case CAMPO_HUM:    return m->humedad;
        case CAMPO_VIENTO: return m->velocidadViento;
    }
    return 0.0f;
}

float promedioPonderado30(const char *zona, int campo) {
    float suma = 0.0f;
    float pesos = 0.0f;
    int i;

    float vals[30];
    int   n = 0;

    for (i = 0; i < cantidadMediciones && n < 30; i++) {
        if (strcmp(mediciones[i].zonaNombre, zona) == 0) {
            vals[n++] = obtenerCampoMedicion(&mediciones[i], campo);
        }
    }

    if (n == 0) return 0.0f;

    for (i = 0; i < n; i++) {
        float peso = (i >= n - 7) ? 2.0f : 1.0f;
        suma  += vals[i] * peso;
        pesos += peso;
    }
    return (pesos > 0.0f) ? suma / pesos : 0.0f;
}

float prediccionClimatica(float base, float viento, float humedad, float temp) {
    float fv, fh, ft;

    if (base < 0.0f) base = 0.0f;

    fv = 1.0f - (viento / 150.0f) * 0.30f;
    if (fv < 0.70f) fv = 0.70f;

    fh = 1.0f + (humedad / 100.0f) * 0.15f;

    if (temp < 10.0f)       ft = 1.15f;
    else if (temp > 25.0f)  ft = 0.90f;
    else                    ft = 1.00f;

    return base * fv * fh * ft;
}

void predecirContaminacion(void) {
    int idx, i;
    char fechaHoy[12];
    Medicion *mHoy = NULL;
    float viento, humedad, temp;

    int aw[5] = {16, 10, 10, 10, 10};
    const char *ah[5] = {"Zona", "PM2.5", "NO2", "SO2", "CO2"};
    const char *af[5];
    char pm[16], no2[16], so2[16], co2[16];

    printf("\n--- Prediccion de contaminacion a 24 horas ---\n");
    if (cantidadZonas == 0) { printf("  No hay zonas registradas.\n"); return; }

    idx = seleccionarZona();
    if (idx < 0) return;

    obtenerFechaHoy(fechaHoy, sizeof(fechaHoy));

    for (i = 0; i < cantidadMediciones; i++) {
        if (strcmp(mediciones[i].zonaNombre, zonas[idx].nombre) == 0 &&
            strcmp(mediciones[i].fecha, fechaHoy) == 0) {
            mHoy = &mediciones[i];
            break;
        }
    }

    if (mHoy == NULL) {
        printf("  Error: Debe registrar la medicion actual de hoy para predecir a 24 horas.\n");
        return;
    }

    viento  = mHoy->velocidadViento;
    humedad = mHoy->humedad;
    temp    = mHoy->temperatura;

    {
        float p_pm25 = prediccionClimatica(promedioPonderado30(zonas[idx].nombre, CAMPO_PM25), viento, humedad, temp);
        float p_no2  = prediccionClimatica(promedioPonderado30(zonas[idx].nombre, CAMPO_NO2),  viento, humedad, temp);
        float p_so2  = prediccionClimatica(promedioPonderado30(zonas[idx].nombre, CAMPO_SO2),  viento, humedad, temp);
        float p_co2  = prediccionClimatica(promedioPonderado30(zonas[idx].nombre, CAMPO_CO2),  viento, humedad, temp);

        printf("\n  Zona: %s  |  Factores actuales: Temp=%.1fC  Humedad=%.1f%%  Viento=%.1f km/h\n\n",
               zonas[idx].nombre, temp, humedad, viento);

        printf("  Prediccion de Contaminantes a 24 horas (ug/m3 / ppm):\n");
        filaSep(aw, 5);
        filaTextos(ah, aw, 5);
        filaSep(aw, 5);

        snprintf(pm,  sizeof(pm),  "%.2f", p_pm25);
        snprintf(no2, sizeof(no2), "%.2f", p_no2);
        snprintf(so2, sizeof(so2), "%.2f", p_so2);
        snprintf(co2, sizeof(co2), "%.1f", p_co2);
        af[0]=zonas[idx].nombre; af[1]=pm; af[2]=no2; af[3]=so2; af[4]=co2;
        filaTextos(af, aw, 5);
        filaSep(aw, 5);

        {
            int bw[3] = {10, 10, 46};
            const char *bh[3] = {"Contam.", "Prediccion", "Alerta"};
            const char *bf[3];
            char vbuf[16];

            printf("\n  Alertas basadas en prediccion:\n");
            filaSep(bw, 3);
            filaTextos(bh, bw, 3);
            filaSep(bw, 3);

            snprintf(vbuf,sizeof(vbuf),"%.2f",p_pm25);
            bf[0]="PM2.5"; bf[1]=vbuf; bf[2]=nivelPM25(p_pm25); filaTextos(bf,bw,3);
            snprintf(vbuf,sizeof(vbuf),"%.2f",p_no2);
            bf[0]="NO2";   bf[1]=vbuf; bf[2]=nivelNO2(p_no2);   filaTextos(bf,bw,3);
            snprintf(vbuf,sizeof(vbuf),"%.2f",p_so2);
            bf[0]="SO2";   bf[1]=vbuf; bf[2]=nivelSO2(p_so2);   filaTextos(bf,bw,3);
            snprintf(vbuf,sizeof(vbuf),"%.1f",p_co2);
            bf[0]="CO2";   bf[1]=vbuf; bf[2]=nivelCO2(p_co2);   filaTextos(bf,bw,3);
            filaSep(bw, 3);
        }

        /* Nueva tabla para prediccion de factores ambientales */
        {
            int cw[4] = {16, 12, 12, 12};
            const char *ch[4] = {"Zona", "Temp (C)", "Humedad (%)", "Viento (km/h)"};
            const char *cf[4];
            char pTemp[16], pHum[16], pViento[16];

            printf("\n  Prediccion a 24 horas (Factores Ambientales):\n");
            filaSep(cw, 4);
            filaTextos(ch, cw, 4);
            filaSep(cw, 4);

            snprintf(pTemp, sizeof(pTemp), "%.1f", temp);
            snprintf(pHum, sizeof(pHum), "%.1f", humedad);
            snprintf(pViento, sizeof(pViento), "%.1f", viento);

            cf[0] = zonas[idx].nombre; cf[1] = pTemp; cf[2] = pHum; cf[3] = pViento;
            filaTextos(cf, cw, 4);
            filaSep(cw, 4);
        }

        printf("  Nota: Prediccion de contaminantes ajustada por factores actuales.\n");
        printf("        Prediccion ambiental basada en metodo de persistencia (24h).\n");
    }
}

void calcularPromedioHistorico(void) {
    int idx, i, n;
    float prom;

    int aw[6] = {16, 10, 10, 10, 10, 12};
    const char *ah[6] = {"Zona", "PM2.5", "NO2", "SO2", "CO2", "Mediciones"};
    const char *af[6];
    char pm[16], no2[16], so2[16], co2[16], cnt[16];

    printf("\n--- Promedio historico ultimos 30 dias ---\n");
    if (cantidadZonas == 0) { printf("  No hay zonas registradas.\n"); return; }

    idx = seleccionarZona();
    if (idx < 0) return;

    n = 0;
    for (i = 0; i < cantidadMediciones; i++) {
        if (strcmp(mediciones[i].zonaNombre, zonas[idx].nombre) == 0) n++;
    }

    if (n == 0) {
        printf("  No hay mediciones registradas para '%s'.\n", zonas[idx].nombre);
        return;
    }

    printf("\n  Zona: %s  |  Total mediciones disponibles: %d\n\n", zonas[idx].nombre, n);

    snprintf(pm,  sizeof(pm),  "%.2f", promedioPonderado30(zonas[idx].nombre, CAMPO_PM25));
    snprintf(no2, sizeof(no2), "%.2f", promedioPonderado30(zonas[idx].nombre, CAMPO_NO2));
    snprintf(so2, sizeof(so2), "%.2f", promedioPonderado30(zonas[idx].nombre, CAMPO_SO2));
    snprintf(co2, sizeof(co2), "%.1f", promedioPonderado30(zonas[idx].nombre, CAMPO_CO2));
    snprintf(cnt, sizeof(cnt), "%d",   n > 30 ? 30 : n);

    printf("  Promedios ponderados (peso doble ultimos 7 dias):\n");
    filaSep(aw, 6);
    filaTextos(ah, aw, 6);
    filaSep(aw, 6);
    af[0]=zonas[idx].nombre; af[1]=pm; af[2]=no2; af[3]=so2; af[4]=co2; af[5]=cnt;
    filaTextos(af, aw, 6);
    filaSep(aw, 6);

    {
        int lw[4] = {16, 10, 12, 10};
        const char *lh[4] = {"Contaminante", "Promedio", "Limite OMS", "Estado"};
        const char *lf[4];
        char lprom[16];

        printf("\n  Comparacion con limites OMS:\n");
        filaSep(lw, 4);
        filaTextos(lh, lw, 4);
        filaSep(lw, 4);

        prom = promedioPonderado30(zonas[idx].nombre, CAMPO_PM25);
        snprintf(lprom, sizeof(lprom), "%.2f", prom);
        lf[0]="PM2.5"; lf[1]=lprom; lf[2]="15 ug/m3"; lf[3]=nivelPM25(prom); filaTextos(lf,lw,4);

        prom = promedioPonderado30(zonas[idx].nombre, CAMPO_NO2);
        snprintf(lprom, sizeof(lprom), "%.2f", prom);
        lf[0]="NO2"; lf[1]=lprom; lf[2]="25 ug/m3"; lf[3]=nivelNO2(prom); filaTextos(lf,lw,4);

        prom = promedioPonderado30(zonas[idx].nombre, CAMPO_SO2);
        snprintf(lprom, sizeof(lprom), "%.2f", prom);
        lf[0]="SO2"; lf[1]=lprom; lf[2]="40 ug/m3"; lf[3]=nivelSO2(prom); filaTextos(lf,lw,4);

        prom = promedioPonderado30(zonas[idx].nombre, CAMPO_CO2);
        snprintf(lprom, sizeof(lprom), "%.1f", prom);
        lf[0]="CO2"; lf[1]=lprom; lf[2]="1000 ppm"; lf[3]=nivelCO2(prom); filaTextos(lf,lw,4);

        filaSep(lw, 4);
    }

    {
        int ew[3] = {16, 12, 12};
        const char *eh[3] = {"Factor", "Promedio", "Estado"};
        const char *ef[3];
        char evbuf[16];

        printf("\n  Promedios de factores ambientales:\n");
        filaSep(ew, 3);
        filaTextos(eh, ew, 3);
        filaSep(ew, 3);

        prom = promedioPonderado30(zonas[idx].nombre, CAMPO_TEMP);
        snprintf(evbuf, sizeof(evbuf), "%.1f C", prom);
        ef[0]="Temperatura"; ef[1]=evbuf; ef[2]=nivelTemp(prom); filaTextos(ef,ew,3);

        prom = promedioPonderado30(zonas[idx].nombre, CAMPO_HUM);
        snprintf(evbuf, sizeof(evbuf), "%.1f %%", prom);
        ef[0]="Humedad"; ef[1]=evbuf; ef[2]=nivelHumedad(prom); filaTextos(ef,ew,3);

        prom = promedioPonderado30(zonas[idx].nombre, CAMPO_VIENTO);
        snprintf(evbuf, sizeof(evbuf), "%.1f km/h", prom);
        ef[0]="Viento"; ef[1]=evbuf; ef[2]=nivelViento(prom); filaTextos(ef,ew,3);

        filaSep(ew, 3);
    }
}

static void escribirReporte(FILE *fp, const char *fechaHoy) {
    int i, j, n;
    int hayDatos = 0;

    fprintf(fp, "=================================================\n");
    fprintf(fp, "   SIGPCA - Reporte de Monitoreo del Aire\n");
    fprintf(fp, "   Fecha de generacion: %s\n", fechaHoy);
    fprintf(fp, "   Zonas registradas  : %d\n", cantidadZonas);
    fprintf(fp, "=================================================\n\n");

    /* TABLA 1A: PROMEDIOS HISTÓRICOS CONTAMINANTES */
    fprintf(fp, "TABLA 1A: Promedios historicos ponderados - Contaminantes (ultimos 30 dias)\n");
    fprintf(fp, "+------------------+----------+----------+----------+----------+------------+\n");
    fprintf(fp, "| %-16s | %-8s | %-8s | %-8s | %-8s | %-10s |\n",
            "Zona","PM2.5","NO2","SO2","CO2","Mediciones");
    fprintf(fp, "+------------------+----------+----------+----------+----------+------------+\n");

    for (i = 0; i < cantidadZonas; i++) {
        n = 0;
        for (j = 0; j < cantidadMediciones; j++) {
            if (strcmp(mediciones[j].zonaNombre, zonas[i].nombre) == 0) n++;
        }
        fprintf(fp, "| %-16s | %8.2f | %8.2f | %8.2f | %8.1f | %10d |\n",
                zonas[i].nombre,
                promedioPonderado30(zonas[i].nombre, CAMPO_PM25),
                promedioPonderado30(zonas[i].nombre, CAMPO_NO2),
                promedioPonderado30(zonas[i].nombre, CAMPO_SO2),
                promedioPonderado30(zonas[i].nombre, CAMPO_CO2),
                n > 30 ? 30 : n);
    }
    fprintf(fp, "+------------------+----------+----------+----------+----------+------------+\n");
    fprintf(fp, "Limites OMS Alerta: PM2.5=15 | NO2=25 | SO2=40 | CO2=1000 (ug/m3 / ppm)\n\n");

    /* TABLA 1B: PROMEDIOS HISTÓRICOS AMBIENTALES */
    fprintf(fp, "TABLA 1B: Promedios historicos ponderados - Factores Ambientales (ultimos 30 dias)\n");
    fprintf(fp, "+------------------+------------+-------------+---------------+\n");
    fprintf(fp, "| %-16s | %-10s | %-11s | %-13s |\n", "Zona", "Temp (C)", "Humedad (%)", "Viento (km/h)");
    fprintf(fp, "+------------------+------------+-------------+---------------+\n");
    for (i = 0; i < cantidadZonas; i++) {
        fprintf(fp, "| %-16s | %10.1f | %11.1f | %13.1f |\n",
                zonas[i].nombre,
                promedioPonderado30(zonas[i].nombre, CAMPO_TEMP),
                promedioPonderado30(zonas[i].nombre, CAMPO_HUM),
                promedioPonderado30(zonas[i].nombre, CAMPO_VIENTO));
    }
    fprintf(fp, "+------------------+------------+-------------+---------------+\n\n");

    /* TABLA 2A: MEDICIONES ACTUALES CONTAMINANTES */
    fprintf(fp, "TABLA 2A: Mediciones actuales del dia %s - Contaminantes\n", fechaHoy);
    fprintf(fp, "+------------------+----------+----------+----------+----------+------------+\n");
    fprintf(fp, "| %-16s | %-8s | %-8s | %-8s | %-8s | %-10s |\n",
            "Zona","PM2.5","NO2","SO2","CO2","Alerta");
    fprintf(fp, "+------------------+----------+----------+----------+----------+------------+\n");

    for (i = 0; i < cantidadZonas; i++) {
        int encontrado = 0;
        for (j = 0; j < cantidadMediciones; j++) {
            if (strcmp(mediciones[j].zonaNombre, zonas[i].nombre) == 0 &&
                strcmp(mediciones[j].fecha, fechaHoy) == 0) {

                const char *niv, *alerta = "NORMAL";
                const char *niveles[4];
                niveles[0] = nivelPM25(mediciones[j].pm25);
                niveles[1] = nivelNO2(mediciones[j].no2);
                niveles[2] = nivelSO2(mediciones[j].so2);
                niveles[3] = nivelCO2(mediciones[j].co2);
                for (n = 0; n < 4; n++) {
                    niv = niveles[n];
                    if (strcmp(niv,"ALTO")==0)     { alerta="ALTO"; break; }
                    if (strcmp(niv,"MODERADO")==0)   alerta="MODERADO";
                }

                fprintf(fp, "| %-16s | %8.2f | %8.2f | %8.2f | %8.1f | %-10s |\n",
                        zonas[i].nombre,
                        mediciones[j].pm25, mediciones[j].no2,
                        mediciones[j].so2,  mediciones[j].co2,
                        alerta);
                encontrado = 1;
                hayDatos   = 1;
                break;
            }
        }
        if (!encontrado) {
            fprintf(fp, "| %-16s | %-8s | %-8s | %-8s | %-8s | %-10s |\n",
                    zonas[i].nombre, "---","---","---","---","Sin datos");
        }
    }
    fprintf(fp, "+------------------+----------+----------+----------+----------+------------+\n");
    if (!hayDatos) {
        fprintf(fp, "  Ninguna zona tiene medicion registrada para el dia de hoy.\n");
    }
    fprintf(fp, "\n");

    /* TABLA 2B: MEDICIONES ACTUALES AMBIENTALES */
    fprintf(fp, "TABLA 2B: Mediciones actuales del dia %s - Factores Ambientales\n", fechaHoy);
    fprintf(fp, "+------------------+------------+-------------+---------------+\n");
    fprintf(fp, "| %-16s | %-10s | %-11s | %-13s |\n", "Zona", "Temp (C)", "Humedad (%)", "Viento (km/h)");
    fprintf(fp, "+------------------+------------+-------------+---------------+\n");
    
    for (i = 0; i < cantidadZonas; i++) {
        int encontrado = 0;
        for (j = 0; j < cantidadMediciones; j++) {
            if (strcmp(mediciones[j].zonaNombre, zonas[i].nombre) == 0 &&
                strcmp(mediciones[j].fecha, fechaHoy) == 0) {
                fprintf(fp, "| %-16s | %10.1f | %11.1f | %13.1f |\n",
                        zonas[i].nombre,
                        mediciones[j].temperatura,
                        mediciones[j].humedad,
                        mediciones[j].velocidadViento);
                encontrado = 1;
                break;
            }
        }
        if (!encontrado) {
            fprintf(fp, "| %-16s | %-10s | %-11s | %-13s |\n",
                    zonas[i].nombre, "---", "---", "---");
        }
    }
    fprintf(fp, "+------------------+------------+-------------+---------------+\n\n");
}

void mostrarReporte(void) {
    char fechaHoy[12];
    char nombreArch[64];
    FILE *fp = NULL;
    int num = 1;

    obtenerFechaHoy(fechaHoy, sizeof(fechaHoy));

    do {
        snprintf(nombreArch, sizeof(nombreArch), "reporte_SIGPCA_%03d.txt", num);
        fp = fopen(nombreArch, "r");
        if (fp != NULL) {
            fclose(fp);
            num++;
        }
    } while (fp != NULL && num <= 999);

    printf("\n--- Reporte general ---\n");
    printf("  Fecha: %s\n\n", fechaHoy);

    {
        int i, j, n;

        /* TABLA 1A - Contaminantes Historicos */
        int aw[6] = {16, 10, 10, 10, 10, 12};
        const char *ah[6] = {"Zona","PM2.5","NO2","SO2","CO2","Mediciones"};
        const char *af[6];
        char pm[16], no2[16], so2[16], co2[16], cnt[16];

        printf("  TABLA 1A: Promedios historicos ponderados - Contaminantes (ultimos 30 dias)\n");
        filaSep(aw, 6);
        filaTextos(ah, aw, 6);
        filaSep(aw, 6);

        for (i = 0; i < cantidadZonas; i++) {
            n = 0;
            for (j = 0; j < cantidadMediciones; j++) {
                if (strcmp(mediciones[j].zonaNombre, zonas[i].nombre) == 0) n++;
            }
            snprintf(pm,  sizeof(pm),  "%.2f", promedioPonderado30(zonas[i].nombre, CAMPO_PM25));
            snprintf(no2, sizeof(no2), "%.2f", promedioPonderado30(zonas[i].nombre, CAMPO_NO2));
            snprintf(so2, sizeof(so2), "%.2f", promedioPonderado30(zonas[i].nombre, CAMPO_SO2));
            snprintf(co2, sizeof(co2), "%.1f", promedioPonderado30(zonas[i].nombre, CAMPO_CO2));
            snprintf(cnt, sizeof(cnt), "%d",   n > 30 ? 30 : n);
            af[0]=zonas[i].nombre; af[1]=pm; af[2]=no2; af[3]=so2; af[4]=co2; af[5]=cnt;
            filaTextos(af, aw, 6);
        }
        filaSep(aw, 6);
        printf("  Limites OMS Alerta: PM2.5=15 | NO2=25 | SO2=40 | CO2=1000 (ug/m3 / ppm)\n\n");

        /* TABLA 1B - Ambientales Historicos */
        int bw[4] = {16, 10, 12, 14};
        const char *bh[4] = {"Zona", "Temp (C)", "Humedad (%)", "Viento (km/h)"};
        const char *bf[4];
        char t_str[16], h_str[16], v_str[16];

        printf("  TABLA 1B: Promedios historicos ponderados - Factores Ambientales (ultimos 30 dias)\n");
        filaSep(bw, 4);
        filaTextos(bh, bw, 4);
        filaSep(bw, 4);

        for (i = 0; i < cantidadZonas; i++) {
            snprintf(t_str, sizeof(t_str), "%.1f", promedioPonderado30(zonas[i].nombre, CAMPO_TEMP));
            snprintf(h_str, sizeof(h_str), "%.1f", promedioPonderado30(zonas[i].nombre, CAMPO_HUM));
            snprintf(v_str, sizeof(v_str), "%.1f", promedioPonderado30(zonas[i].nombre, CAMPO_VIENTO));
            bf[0]=zonas[i].nombre; bf[1]=t_str; bf[2]=h_str; bf[3]=v_str;
            filaTextos(bf, bw, 4);
        }
        filaSep(bw, 4);
        printf("\n");

        /* TABLA 2A - Contaminantes Actuales */
        int cw[6] = {16, 10, 10, 10, 10, 10};
        const char *ch[6] = {"Zona","PM2.5","NO2","SO2","CO2","Alerta"};
        const char *cf[6];

        printf("  TABLA 2A: Mediciones actuales del dia %s - Contaminantes\n", fechaHoy);
        filaSep(cw, 6);
        filaTextos(ch, cw, 6);
        filaSep(cw, 6);

        for (i = 0; i < cantidadZonas; i++) {
            int enc = 0;
            for (j = 0; j < cantidadMediciones; j++) {
                if (strcmp(mediciones[j].zonaNombre, zonas[i].nombre) == 0 &&
                    strcmp(mediciones[j].fecha, fechaHoy) == 0) {
                    const char *niv, *alerta = "NORMAL";
                    const char *niveles[4];
                    int k;
                    niveles[0]=nivelPM25(mediciones[j].pm25);
                    niveles[1]=nivelNO2(mediciones[j].no2);
                    niveles[2]=nivelSO2(mediciones[j].so2);
                    niveles[3]=nivelCO2(mediciones[j].co2);
                    for (k=0;k<4;k++){niv=niveles[k];if(strcmp(niv,"ALTO")==0){alerta="ALTO";break;}if(strcmp(niv,"MODERADO")==0)alerta="MODERADO";}
                    snprintf(pm,sizeof(pm),"%.2f",mediciones[j].pm25);
                    snprintf(no2,sizeof(no2),"%.2f",mediciones[j].no2);
                    snprintf(so2,sizeof(so2),"%.2f",mediciones[j].so2);
                    snprintf(co2,sizeof(co2),"%.1f",mediciones[j].co2);
                    cf[0]=zonas[i].nombre;cf[1]=pm;cf[2]=no2;cf[3]=so2;cf[4]=co2;cf[5]=alerta;
                    filaTextos(cf,cw,6);
                    enc=1; break;
                }
            }
            if (!enc) {
                cf[0]=zonas[i].nombre;cf[1]="---";cf[2]="---";cf[3]="---";cf[4]="---";cf[5]="Sin datos";
                filaTextos(cf,cw,6);
            }
        }
        filaSep(cw, 6);
        printf("\n");

        /* TABLA 2B - Ambientales Actuales */
        printf("  TABLA 2B: Mediciones actuales del dia %s - Factores Ambientales\n", fechaHoy);
        filaSep(bw, 4);
        filaTextos(bh, bw, 4);
        filaSep(bw, 4);
        for (i = 0; i < cantidadZonas; i++) {
            int enc = 0;
            for (j = 0; j < cantidadMediciones; j++) {
                if (strcmp(mediciones[j].zonaNombre, zonas[i].nombre) == 0 &&
                    strcmp(mediciones[j].fecha, fechaHoy) == 0) {
                    snprintf(t_str, sizeof(t_str), "%.1f", mediciones[j].temperatura);
                    snprintf(h_str, sizeof(h_str), "%.1f", mediciones[j].humedad);
                    snprintf(v_str, sizeof(v_str), "%.1f", mediciones[j].velocidadViento);
                    bf[0]=zonas[i].nombre; bf[1]=t_str; bf[2]=h_str; bf[3]=v_str;
                    filaTextos(bf, bw, 4);
                    enc=1; break;
                }
            }
            if (!enc) {
                bf[0]=zonas[i].nombre; bf[1]="---"; bf[2]="---"; bf[3]="---";
                filaTextos(bf, bw, 4);
            }
        }
        filaSep(bw, 4);
    }

    fp = fopen(nombreArch, "w");
    if (fp == NULL) {
        printf("  Error: no se pudo crear el archivo de reporte.\n");
        return;
    }
    escribirReporte(fp, fechaHoy);
    fclose(fp);
    printf("\n  Reporte guardado en: %s\n", nombreArch);
}

void guardarArchivos(void) {
    FILE *fp = fopen("historial.bin", "wb");
    if (fp == NULL) { printf("  Error al guardar historial.\n"); return; }
    fwrite(&cantidadZonas,      sizeof(int),     1,                fp);
    fwrite(&cantidadMediciones, sizeof(int),     1,                fp);
    fwrite(zonas,               sizeof(Zona),    cantidadZonas,    fp);
    fwrite(mediciones,          sizeof(Medicion),cantidadMediciones,fp);
    fclose(fp);
    printf("  Datos guardados correctamente.\n");
}

void cargarArchivos(void) {
    FILE *fp = fopen("historial.bin", "rb");
    int cz, cm;

    if (fp == NULL) { cantidadZonas = 0; cantidadMediciones = 0; return; }

    if (fread(&cz, sizeof(int), 1, fp) != 1 ||
        fread(&cm, sizeof(int), 1, fp) != 1 ||
        cz < 0 || cz > MAX_ZONAS ||
        cm < 0 || cm > MAX_MEDICIONES) {
        fclose(fp);
        cantidadZonas = 0; cantidadMediciones = 0;
        return;
    }

    if (fread(zonas,     sizeof(Zona),     cz, fp) != (size_t)cz ||
        fread(mediciones,sizeof(Medicion), cm, fp) != (size_t)cm) {
        fclose(fp);
        cantidadZonas = 0; cantidadMediciones = 0;
        return;
    }

    fclose(fp);
    cantidadZonas      = cz;
    cantidadMediciones = cm;
    printf("Historial cargado: %d zonas, %d mediciones.\n", cz, cm);
}

void generarBaseDatosMock(void) {
    int i, d;

    const char *nombres[5] = {
        "El Camal",      
        "La Mariscal",   
        "Cotocollao",    
        "Tumbaco",       
        "Guapulo"        
    };

    float base[5][7] = {
        { 42.0f,  30.0f,  38.0f, 950.0f,  15.0f, 68.0f, 10.0f },
        { 22.0f,  48.0f,  18.0f, 680.0f,  16.0f, 65.0f,  5.0f },
        { 28.0f,  28.0f,  22.0f, 620.0f,  14.0f, 72.0f, 12.0f },
        {  8.0f,  10.0f,   6.0f, 420.0f,  19.0f, 55.0f, 22.0f },
        { 18.0f,  20.0f,  15.0f, 590.0f,  17.0f, 63.0f, 14.0f }
    };

    float varFactor[7] = { 3.0f, 4.0f, 2.5f, 40.0f, 1.5f, 4.0f, 2.0f };
    int   varMod[7]    = {    7,    6,    5,     8,    5,    6,    4   };
    int   varOff[7]    = {    3,    2,    2,     4,    2,    2,    1   };

    cantidadZonas      = 0;
    cantidadMediciones = 0;

    for (i = 0; i < 5; i++) {
        zonas[i].id = i;
        strncpy(zonas[i].nombre, nombres[i], 49);
        zonas[i].nombre[49] = '\0';
        cantidadZonas++;

        for (d = 0; d < 30; d++) {
            Medicion m;
            int campo;
            float val;
            {
                time_t t = time(NULL) - (time_t)(30 - d) * 86400L;
                struct tm *tinfo = localtime(&t);
                char fbuf[40];
                snprintf(fbuf, sizeof(fbuf), "%02d/%02d/%04d",
                         tinfo->tm_mday, tinfo->tm_mon + 1, tinfo->tm_year + 1900);
                strncpy(m.fecha, fbuf, 11);
                m.fecha[11] = '\0';
            }
            strncpy(m.zonaNombre, nombres[i], 49);
            m.zonaNombre[49] = '\0';

            for (campo = 0; campo < 7; campo++) {
                val = base[i][campo] +
                      ((d % varMod[campo]) - varOff[campo]) * varFactor[campo];
                if (val < 0.0f) val = 0.0f;
                switch (campo) {
                    case 0: m.pm25           = val; break;
                    case 1: m.no2            = val; break;
                    case 2: m.so2            = val; break;
                    case 3: m.co2            = val; break;
                    case 4: m.temperatura    = val; break;
                    case 5: m.humedad        = (val > 100.0f ? 100.0f : val); break;
                    case 6: m.velocidadViento= val; break;
                }
            }

            if (cantidadMediciones < MAX_MEDICIONES) {
                mediciones[cantidadMediciones++] = m;
            }
        }
    }

    printf("Base de datos cargada: %d zonas, %d mediciones historicas.\n",
           cantidadZonas, cantidadMediciones);
    guardarArchivos();
}