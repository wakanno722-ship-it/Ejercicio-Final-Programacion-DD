
enum {
    NUM_ZONAS_MAX = 10,
    NUM_ZONAS_BASE = 5,
    NUM_CONTAMINANTES = 4,
    DIAS_HISTORICO = 30,
    MAX_REPORTES = 100,
    MAX_NOMBRE_ZONA = 50,
    MAX_TEXTO = 300
};

extern const float LIMITE_PM25_OMS;
extern const float LIMITE_NO2_OMS;
extern const float LIMITE_SO2_OMS;
extern const float CO2_ALTO_PPM;

typedef struct {
    int   id;
    char  nombre[50]; 

    float pm25_actual;
    float no2_actual;
    float so2_actual;
    float co2_actual;

    float temperatura;
    float velocidad_viento;
    float humedad;

    float historico_pm25[30]; 
    float historico_no2[30];
    float historico_so2[30];
    float historico_co2[30];

    float prediccion_pm25_24h;
    float prediccion_no2_24h;
    float prediccion_so2_24h;
    float prediccion_co2_24h;

    int nivel_alerta;
} Zona;

typedef struct {
    char  fecha[20];
    char  hora[10];
    int   zona_id;
    float valores[4]; 
    float predicciones[4];
    int   alertas[4];
    char  recomendacion[300]; 
} Reporte;

typedef struct {
    Zona    zonas[10]; 
    int     num_zonas;
    Reporte reportes[100];
    int     num_reportes;
    char    archivo_historial[100];
    char    archivo_reporte[100];
} SistemaMonitoreo;

int   leer_linea(char *buffer, int size);
int   pedir_entero_rango(const char *mensaje, int minimo, int maximo);
float pedir_flotante_rango(const char *mensaje, float minimo, float maximo);
void  leer_nombre(const char *mensaje, char *destino, int tam);

void inicializar_sistema(SistemaMonitoreo *s);
void inicializar_datos_base(SistemaMonitoreo *s);
void cargar_historial(SistemaMonitoreo *s);
void guardar_historial(SistemaMonitoreo *s);

void ingresar_medicion(Zona *z);
void actualizar_historial(Zona *z);
void agregar_zona(SistemaMonitoreo *s);
void eliminar_zona(SistemaMonitoreo *s);

float calcular_promedio_ponderado(float *historico, int n);
float aplicar_factor_climatico(float pred_base, float viento_kmh, float humedad_pct, float temp_c);
int   clasificar_nivel(float valor, float limite_oms);
void  comparar_con_limites(Zona *z);
void  actualizar_predicciones(Zona *z);
void  generar_recomendaciones(Zona *z, char *mensaje, int tam_mensaje);

void mostrar_resumen_zonas(SistemaMonitoreo *s);
void mostrar_datos_zona(Zona *z);
void exportar_reporte(SistemaMonitoreo *s);

int  validar_float_ok(float v, float minimo, float maximo);
void validar_zona(Zona *z, int indice);
void validar_sistema(SistemaMonitoreo *s);

void obtener_fecha_hora(char *fecha, int tf, char *hora, int th);
const char *texto_nivel(int nivel);

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