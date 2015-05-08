//
//  Pagina.h
//  LRU
//
//  Created by Iker Arbulu Lozano on 5/8/15.
//  Copyright (c) 2015 Iker Arbulu Lozano. All rights reserved.
//

#ifndef LRU_Pagina_h
#define LRU_Pagina_h

class Pagina{
private:
    int idProceso;
    int numPagina;
    bool estado;
    int pageFaults;
    clock_t creacion;
    clock_t ultimaModificacion;
    
public:
    Pagina(int idProc, int numPag);
    void referenciar();
    void pageFault();
    void swap(int numPag);
    int getIdProceso();
    int getNumPagina();
    int getEstado();
    int getPageFaults();
    clock_t getCreacion();
    clock_t getUltimaModificacion();
};

Pagina::Pagina(int idProc, int numPag){
    idProceso = idProc;
    numPagina = numPag;
    estado = 1;
    pageFaults = 0;
    creacion = clock();
    ultimaModificacion = creacion;
}

void Pagina::referenciar(){
    estado = 1;
    ultimaModificacion = clock();
}

void Pagina::pageFault(){
    pageFaults++;
}

void Pagina::swap(int numPag){
    numPagina = numPag;
    if(estado ==0){
        estado = 1;
        ultimaModificacion = clock();
    }
    else{
        estado = 0;
    }
}

int Pagina::getIdProceso(){
    return idProceso;
}

int Pagina::getNumPagina(){
    return numPagina;
}

int Pagina::getPageFaults(){
    return pageFaults;
}

clock_t Pagina::getUltimaModificacion(){
    return ultimaModificacion;
}

clock_t Pagina::getCreacion(){
    return creacion;
}

#endif
