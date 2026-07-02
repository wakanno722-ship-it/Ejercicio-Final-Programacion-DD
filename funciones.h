#ifndef FUNCIONES_H
#define FUNCIONES_H

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define MAX_ZONAS        10
#define MAX_MEDICIONES   500
#define DIAS_HISTORICO   30

#define LIMITE_PM25   15.0f
#define LIMITE_NO2    25.0f
#define LIMITE_SO2    40.0f
#define LIMITE_CO2    1000.0f

#define TEMP_MIN      5.0f
#define TEMP_MAX      30.0f
#define HUMEDAD_MIN   30.0f
#define HUMEDAD_MAX   70.0f
#define VIENTO_MIN    2.0f
#define VIENTO_MAX    50.0f

#define CAMPO_PM25   0
#define CAMPO_NO2    1
#define CAMPO_SO2    2
#define CAMPO_CO2    3
#define CAMPO_TEMP   4
#define CAMPO_HUM    5
#define CAMPO_VIENTO 6

typedef struct {
    int  id;
    char nombre[50];
} Zona;

typedef struct {
    char  zonaNombre[50];
    char  fecha[12];   
    float pm25;
    float no2;
    float so2;
    float co2;
    float temperatura;
    float humedad;
    float velocidadViento;
} Medicion;

extern Zona     zonas[MAX_ZONAS];
extern Medicion mediciones[MAX_MEDICIONES];
extern int      cantidadZonas;
extern int      cantidadMediciones;

int menu(void);

void registrarZona(void);
void listarZonas(void);
void eliminarZona(void);

void registrarMedicionActual(void);
void agregarMedicionHistorica(void);

void monitorearContaminacion(void);
void mostrarAlertasRecomendaciones(void);
void predecirContaminacion(void);
void calcularPromedioHistorico(void);
void mostrarReporte(void);

void guardarArchivos(void);
void cargarArchivos(void);
void generarBaseDatosMock(void);

int   pedirEnteroRango(const char *msg, int a, int b);
float pedirFlotanteRango(const char *msg, float a, float b);
void  leerCadena(char *dest, int tam);
void  limpiarBuffer(void);

int buscarZonaPorNombre(const char *nombre);
void mostrarZonasNumeradas(void);
int seleccionarZona(void);

float obtenerCampoMedicion(Medicion *m, int campo);
float promedioUltimos30(const char *zona, int campo);
float promedioUltimos7(const char *zona, int campo);
float promedioPonderado30(const char *zona, int campo);
float prediccionClimatica(float base, float viento, float humedad, float temp);
Medicion *obtenerMedicionHoy(const char *zona, const char *fechaHoy);

const char *nivelPM25(float v);
const char *nivelNO2(float v);
const char *nivelSO2(float v);
const char *nivelCO2(float v);
const char *nivelTemp(float v);
const char *nivelHumedad(float v);
const char *nivelViento(float v);
const char *recPM25(float v);
const char *recNO2(float v);
const char *recSO2(float v);
const char *recCO2(float v);
const char *recTemp(float v);
const char *recHumedad(float v);
const char *recViento(float v);

void obtenerFechaHoy(char *buf, int tam);

void imprimirSeparador(int ancho);
void imprimirLinea(const char *col1, int a1,
                   const char *col2, int a2,
                   const char *col3, int a3,
                   const char *col4, int a4,
                   const char *col5, int a5);

#endif
