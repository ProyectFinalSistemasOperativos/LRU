//
//  main.cpp
//  LRU
//
//  Created by Iker Arbulu Lozano on 5/8/15.
//  Copyright (c) 2015 Iker Arbulu Lozano. All rights reserved.
//


//falta imprimir, agregar pagefaults, (validar), turnaround

#include <iostream>
#include "Pagina.h"
#include <fstream>
#include <iostream>
#include <vector>

using namespace std;

struct Proceso{
    long int id;
    int tamReal;
    clock_t stampCreacion;
    clock_t stampLiberacion;
    bool activo;
};

int espacioDisponible= 256;

Pagina memoriaReal[256], memoriaSwap[512];

vector<Proceso> procesos;

void swapLRU1(Pagina nuevaPag){ //el swap de cuando se crea un proceso
    int sub=0;
    clock_t menor= memoriaReal[sub].getUltimaModificacion();
    for (int i=1; i<256; i++) { //primero se encuentra el LRU en la memoria real
        if (memoriaReal[i].getUltimaModificacion()<menor) {
            menor= memoriaReal[i].getUltimaModificacion();
            sub= i;
        }
    }
    bool encontrado=false;
    for (int i=0; i<512&&(!encontrado); i++) { // Aqui se busca un espacio libre en el swap para hacer el cambio
        if (memoriaSwap[i].getIdProceso()==(-1)) {
            memoriaSwap[i]= memoriaReal[sub];
            nuevaPag.referenciar(); //para actualizar el timestamp
            memoriaReal[sub]= nuevaPag;
        }
    }
    
}

void swapLRU2(int sub){
    int sub2= 0;
    clock_t menor= memoriaReal[0].getUltimaModificacion();
    for (int i=1; i<256; i++) { // se encuentra el LRU en la memoria real
        if (memoriaReal[i].getUltimaModificacion()<menor) {
            menor= memoriaReal[i].getUltimaModificacion();
            sub2= i;
        }
    }
    Pagina aux= memoriaReal[sub2];
    memoriaReal[sub2]= memoriaSwap[sub];
    memoriaReal[sub2].referenciar();
    memoriaSwap[sub]= aux;
}

void cargarProceso(int tam, long int pId){
    int cont=0;
    Proceso p;
    p.id= pId;
    p.tamReal= tam;
    p.activo= true;
    if (tam%8!=0) {
        tam /= 8;
        tam++;
    }
    else
        tam /= 8;
    if (espacioDisponible>=tam){ // Si hay espacio disponible
        espacioDisponible -= tam;
        for (int i=0; i<256 && (cont<tam); i++) {
            if (memoriaReal[i].getIdProceso()==-1) { // si ese marco esta disponible en memoria
                Pagina auxPag(pId, cont);
                memoriaReal[i]= auxPag;
                cont++;
                espacioDisponible--;
            }
        }
    }
    else{ // si el espacio total libre en memoria no es suficiente para cargar el proceso completo
        for (int i=0; i<256 && (espacioDisponible>0); i++) {
            if (memoriaReal[i].getIdProceso()==-1) { // si ese marco esta disponible en memoria
                Pagina auxPag(pId, cont);
                memoriaReal[i]= auxPag;
                cont++;
                espacioDisponible--;
            }
        }
        for (int i=0; cont<tam; i++) {
            Pagina auxPag(pId, cont);
            swapLRU1(auxPag);
            cont++;
        }
    }
    p.stampCreacion= clock();
    procesos.push_back(p);
}

void accesarDireccion(int dir, int pId, bool m){
    if (dir%8!=0) {
        dir /= 8;
        dir++;
    }
    else
        dir /= 8;
    bool encontrado=false;
    for (int i=0; i<256&&(!encontrado); i++) {
        if (memoriaReal[i].getIdProceso()==pId&&(memoriaReal[i].getNumPagina()==dir)) { //si el marco estaba en la memoria real
            encontrado= true;
            memoriaReal[i].referenciar();
        }
    }
    if (!encontrado) {// si el marco no estaba cargado en memoria
        int sub;
        for (int i=0; i<512&&(!encontrado); i++) { //encuentra el marco deseado en la memoria de swap
            if (memoriaSwap[i].getIdProceso()==pId&&(memoriaSwap[i].getNumPagina()==dir)) {
                encontrado= true;
                sub= i;
            }
        }
        if (espacioDisponible>0) { //no estaba en memorio real pero no hay necesidad de hacer swap
            for (int i=0; i<256; i++) {
                if(memoriaReal[i].getIdProceso()==(-1)){
                    memoriaReal[i]= memoriaSwap[sub];
                    memoriaReal[i].referenciar();
                    espacioDisponible--;
                    Pagina nuevo;
                    memoriaSwap[sub]= nuevo; //con esto se borra el marco de la memoria de swap
                }
            }
        }
        else{ // no estaba en memoria principal pero si se tiene que hacer swap
            swapLRU2(sub);
        }
    }
}

void liberarProceso(int pId){
    Pagina nuevo;
    for (int i=0; i<256; i++) {
        if (memoriaReal[i].getIdProceso()==pId) {
            memoriaReal[i]= nuevo;
            espacioDisponible++;
        }
    }
    for (int i=0; i<512; i++) {
        if (memoriaSwap[i].getIdProceso()==pId) {
            memoriaSwap[i]= nuevo;
        }
    }
    bool encontrado= false;
    for (int i=0; i<procesos.size() && (!encontrado); i++) {
        if (procesos[i].id==pId) {
            procesos[i].stampLiberacion= clock();
            procesos[i].activo= false;
            encontrado= true;
        }
    }
}


int main(int argc, const char * argv[]) {
    
    ifstream entrada;
    entrada.open ("/Users/Balbina/Documents/8vo semestre/Sistemas Operativo/2015/proyecto fina/LRU/LRU/LRU/input.txt");
    string linea;
    char tipoProceso;
    int bytes, numProceso;
    int get();
    if (entrada.is_open()) {
        char opcion;
        while (entrada>>opcion) {
            //entrada.get(tipoProceso);
            //getline(entrada, linea);
            //entrada >>linea;
            //entrada >>tipoProceso >>bytes >>numProceso;
            //cout <<tipoProceso <<" " <<bytes <<" " <<numProceso <<endl;
            cout <<opcion <<endl;
            
            switch (opcion) {
                case 'P':
                    int a, b;
                    entrada>>a>>b;
                    cout <<"funciona \n";
                    break;
                case 'A':
                    
                    
                default:
                    break;
            }
        }
    }
    entrada.close();
    
    ofstream salida;
    salida.open("/Users/Balbina/Documents/8vo semestre/Sistemas Operativo/2015/proyecto fina/LRU/LRU/LRU/output.txt");
    salida <<"escribir o variables al text. \n" <<"hola \n";
    salida.close();
    
    return 0;
}
