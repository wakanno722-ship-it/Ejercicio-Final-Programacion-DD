#include "funciones.h"

int main(void) {
    int opc;

    cargarArchivos();

    if (cantidadZonas == 0 && cantidadMediciones == 0) {
        generarBaseDatosMock();
    }

    do {
        opc = menu();
        switch (opc) {
            case 1:
              registrarZona();      
              guardarArchivos();
            break;
            case 2: 
             listarZonas();             
            break;
            case 3: 
             eliminarZona();       
             guardarArchivos();
            break;
            case 4:  
             registrarMedicionActual();    
             guardarArchivos(); 
            break;
            case 5:  
             agregarMedicionHistorica();
             guardarArchivos();    
            break;
            case 6:  
             monitorearContaminacion();
             guardarArchivos();     
            break;
            case 7:  
             mostrarAlertasRecomendaciones(); 
             guardarArchivos();
            break;
            case 8:  
             predecirContaminacion();     
             guardarArchivos();  
            break;
            case 9:  
             calcularPromedioHistorico();  
             guardarArchivos(); 
            break;
            case 10: 
             mostrarReporte();              
             guardarArchivos();
            break;
            case 11:
                printf("Gracias hasta luego.\n\n");
                break;
        }
    } while (opc != 11);

    return 0;
}
