//
//  main.cpp
//  LRU
//
//  Created by
//Equipo 8
//Axel Suárez 		A01195895
//Balbina Santana 	A00812215
//Iker Arbulu 		A01190690
//Mauricio Mendez 	A00812794
//  Copyright (c) 2015. All rights reserved.
//
//
//El programa simula un manejador de memoria virtual utilizando
//paginación y mide su rendimiento.
//La estrategia de reemplazo de páginas para este programa es de LRU.

#include <iostream>
#include "Pagina.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <sstream>
#include <ctype.h>

using namespace std;

ofstream salida;

struct Proceso{
    long int id;
    int tamReal;
    int swaps=0;
    clock_t stampCreacion;
    clock_t stampLiberacion;
    bool activo;
    int faults=0;
};

int espacioDisponible= 256;
int espacioDisponibleSwap= 512;
int swapintotales = 0;
int swapouttotales = 0;

Pagina memoriaReal[256], memoriaSwap[512];

//procesos creados
vector<Proceso> procesos;
//string de impresion para reporte p
vector<string> cambiados;

//impresion del reporte para caso P
void reporteP(long int pId){
    salida<<"Se asignaron los marcos de pagina: ";
    for (int i=0; i<256; i++) {
        if (memoriaReal[i].getIdProceso()==pId) {
            salida<<i<<" ";
        }
    }
    salida<<"al proceso "<<pId<<endl;
    for (int i=0; i<cambiados.size(); i++) {
        salida<<cambiados[i]<<endl;
    }
    cambiados.clear();
}

//primer caso de swap: cuando se crea un proceso
void swapLRU1(Pagina nuevaPag){
    int sub=0;
    //string que imprime las paginas cambiadas, todo el texto
    string cambiado;
    clock_t menor= clock();
    for (int i=0; i<256; i++) { //primero se encuentra el LRU en la memoria real
        if (memoriaReal[i].getUltimaModificacion()<menor&&(memoriaReal[i].getIdProceso()!=-1)) {
            menor= memoriaReal[i].getUltimaModificacion();
            sub= i;
        }
    }
    //encuentra espacio vacio en el swap
    bool encontrado=false;
    for (int i=0; i<512&&(!encontrado); i++) { // Aqui se busca un espacio libre en el swap para hacer el cambio
        if (memoriaSwap[i].getIdProceso()==(-1)) {
            memoriaSwap[i]= memoriaReal[sub];
            espacioDisponibleSwap--;
            nuevaPag.referenciar(); //para actualizar el timestamp
            memoriaReal[sub]= nuevaPag;
            swapouttotales++;
            encontrado= true;
            cambiado="La pagina ";
            cambiado+= to_string(sub);
            cambiado+=" del proceso ";
            cambiado+= to_string(memoriaSwap[i].getIdProceso());
            cambiado+=" fue Swapeada a la posicion ";
            cambiado+= to_string(i);
            cambiado+=" del area de swaping";
            cambiados.push_back(cambiado);
        }
    }
}

//segundo caso de swap: cuando se accesa a una direccion virtual
void swapLRU2(int sub, int corrimiento, long int pId, int dir){
    int sub2= 0;
    
    clock_t menor= clock();
    for (int i=0; i<256; i++) { // se encuentra el LRU en la memoria real
        if (memoriaReal[i].getUltimaModificacion()<menor&&(memoriaReal[i].getIdProceso()!=-1)) {
            menor= memoriaReal[i].getUltimaModificacion();
            sub2= i;
        }
    }
    
    salida<<(sub2*8)+corrimiento<<endl;
    salida<<"Se localizo la pagina "<<dir/8<<" del proceso "<<pId<<" que estaba en la posicion "<<sub<<" de Swapping y se cargo al marco "<<sub2<<endl;
    Pagina aux= memoriaReal[sub2];
    memoriaReal[sub2]= memoriaSwap[sub];
    memoriaReal[sub2].referenciar();
    
    //cuando encuentras espacio en memoria swap
    bool encontrado= false;
    for (int i=0; i<512&&(!encontrado); i++) {
        if (memoriaSwap[i].getIdProceso()==-1) {
            memoriaSwap[i]=aux;
            salida<<"La pagina "<<sub<<" del proceso "<<pId<<" fue Swappeada a la poicion "<<i<<" del area de Swapping"<<endl;
            salida<<endl<<endl;
            encontrado= true;
        }
    }
    swapintotales++;
    swapouttotales++;
}

//caso 'P' de cargar el proceso recibe tamanio del proceso y numero de proceso
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

//caso 'A' de accesar a direccion virtual, ya sea para leer o modificar
//recibe la direccion, el numero de proceso y si se lee o modifica
void accesarDireccion(int dir, long int pId, bool modif){
    salida<<"Direccion Virtual: "<<dir<<" Direccion Real: ";
    int corrimiento= dir%8;
    dir /= 8;
    bool encontrado=false;
    for (int i=0; i<256&&(!encontrado); i++) {
        if (memoriaReal[i].getIdProceso()==pId&&(memoriaReal[i].getNumPagina()==dir)) { //si el marco estaba en la memoria real
            encontrado= true;
            salida<<(i*8)+corrimiento<<endl;
            salida<<endl<<endl;
            memoriaReal[i].referenciar();
        }
    }
    if (!encontrado) {// si el marco no estaba cargado en memoria
        bool faultsumada = false;
        for (int i = 0; i<procesos.size()&&!faultsumada; i++) {
            if (procesos[i].id == pId) {
                procesos[i].faults++;
                faultsumada = true;
            }
        }
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
                    salida<<(i*8)+corrimiento<<endl;
                    salida<<"Se localizo la pagina "<<dir/8<<" del proceso "<<pId<<" que estaba en la posicion "<<sub<<" de Swapping y se cargo al marco "<<i<<endl;
                    salida<<endl<<endl;
                    memoriaReal[i]= memoriaSwap[sub];
                    memoriaReal[i].referenciar();
                    espacioDisponible--;
                    Pagina nuevo;
                    memoriaSwap[sub]= nuevo; //con esto se borra el marco de la memoria de swap
                }
            }
        }
        else{ // no estaba en memoria principal pero si se tiene que hacer swap
            swapLRU2(sub, corrimiento, pId, dir);
        }
    }
}

//caso 'L' para liberar procesos de la memoria
void liberarProceso(long int pId){
    Pagina nuevo;
    for (int i=0; i<256; i++) {
        if (memoriaReal[i].getIdProceso()==pId) {
            salida<<i<<" ";
            memoriaReal[i]= nuevo;
            espacioDisponible++;
        }
    }
    salida<<endl<<"Los marcos de pagina liberados en memoria de swap: ";
    for (int i=0; i<512; i++) {
        if (memoriaSwap[i].getIdProceso()==pId) {
            salida<<i<<" ";
            memoriaSwap[i]= nuevo;
            espacioDisponibleSwap++;
        }
    }
    salida<<endl<<"Que ocupaba el proceso: "<<pId<<endl;
    salida<<endl<<endl;
    bool encontrado= false;
    for (int i=0; i<procesos.size() && (!encontrado); i++) {
        if (procesos[i].id==pId) {
            procesos[i].stampLiberacion= clock();
            procesos[i].activo= false;
            encontrado= true;
        }
    }
}

//convierte de string a int cada linea del archivo de texto leido
int convierteANum(string palabra){
    bool esNum= true;
    for (int i=0; i<palabra.size()&&esNum; i++) {
        if (!isdigit(palabra[i])) {
            esNum=false;
        }
    }
    if(esNum){
        return stoi(palabra);
    }
    else
        return -1;
}

//convierte de string a long cada linea del archivo de texto leido
long int convierteALong(string palabra){
    bool esNum= true;
    for (int i=0; i<palabra.size()&&esNum; i++) {
        if (!isdigit(palabra[i])) {
            esNum=false;
        }
    }
    if(esNum){
        return stol(palabra);
    }
    else
        return -1;
}

//nos dice si existe el proceso para checarlo al final del reporte
bool existeProceso(long int pId){
    bool existe= false;
    for (int i=0; i<procesos.size()&&(!existe); i++) {
        if (pId==procesos[i].id) {
            existe= true;
        }
    }
    return existe;
}

//checamos el tamanio del proceso
int tamProceso(long int id){
    for (int i=0; i<procesos.size(); i++) {
        if (id==procesos[i].id) {
            return procesos[i].tamReal;
        }
    }
    return 0;
}

//impresion del reporte completo por cada 'F'
void reporte(){
    //procesos activos
    bool activos= false;
    
    for (int i=0; i<procesos.size()&&(!activos); i++) {
        if (procesos[i].activo) {
            activos= true;
        }
    }
    if (activos) {
        salida <<"Error. Proceso(s) todavia en memoria: ";
        for (int i=0; i<procesos.size(); i++) {
            if (procesos[i].activo) {
                salida <<procesos[i].id <<" ";
            }
        }
        salida<<endl;
    }
    
    double turnaround, turnaroundprom = 0;
    
    //procesos inactivos
    int inactivos = 0;
    
    salida<<"Fin. Reporte de salida: " <<endl <<"Turnarounds: \n";
    for(int i=0; i < procesos.size(); i++){
        if (!procesos[i].activo) {
            inactivos++;
            turnaround = (double)(procesos[i].stampLiberacion-procesos[i].stampCreacion);
            salida<<"Proceso " <<procesos[i].id <<" " <<turnaround<<endl;
            turnaroundprom += turnaround;
        }
    }
    if (inactivos>0) {
        turnaroundprom /= (double)inactivos;
        salida <<endl <<"Turnaround promedio: " <<turnaroundprom <<endl <<"Swaps: " <<endl;
    }
    else
        salida <<endl <<"Turnaround promedio: " <<0 <<endl <<"Swaps: " <<endl;
    salida <<"SwapIn: "<<swapintotales<<endl;
    salida<<"SwapOut: "<<swapouttotales<<endl;
    salida<<"Page faults: "<<endl;
    for (int i=0; i<procesos.size(); i++) {
        salida<<"Proceso "<<procesos[i].id<<": "<<procesos[i].faults<<endl;
    }
    salida<<endl<<endl;
}

//se reinician todos los parametros despues de imprimir los reportes
void reinicioBestial(){
    espacioDisponible= 256;
    espacioDisponibleSwap= 512;
    swapintotales = 0;
    swapouttotales = 0;
    
    Pagina nuevo;
    for (int i=0; i<256; i++) {
        memoriaReal[i]=nuevo;
        memoriaSwap[i]=nuevo;
    }
    for (int i=256; i<512; i++) {
        memoriaSwap[i]=nuevo;
    }
    
    procesos.clear();
}

int main(int argc, const char * argv[]) {
    
    ifstream entrada;
    entrada.open ("input.txt");
    salida.open("output.txt");
    
    //leer por linea en el archivo
    string linea;
    //separar la linea
    vector<string> lineaSeparada;
    
    while(getline(entrada, linea)) {
        stringstream lineaStream (linea);
        string aux;
        while (lineaStream>>aux) {
            lineaSeparada.push_back(aux);
        }
        if (lineaSeparada[0].size()==1) {
            char opcion= lineaSeparada[0][0];
            switch(opcion){
                    case 'P':
                        if(lineaSeparada.size()==3){
                            int tam= convierteANum(lineaSeparada[1]);
                            if (tam!=-1) {
                                long int id= convierteALong(lineaSeparada[2]);
                                if (id!=-1) {
                                    if (tam<=2048) {
                                        if (!existeProceso(id)) {
                                            for (int i=0; i<lineaSeparada.size(); i++) {
                                                salida<<lineaSeparada[i]<<" ";
                                            }
                                            salida<<endl;
                                            cargarProceso(tam, id);
                                            reporteP(id);
                                            salida<<endl<<endl;
                                        }
                                        else{
                                            for (int i=0; i<lineaSeparada.size(); i++) {
                                                salida<<lineaSeparada[i]<<" ";
                                            }
                                            salida<<endl;
                                            salida<<"Error: ese proceso ya existe"<<endl;
                                            salida<<endl<<endl;
                                        }
                                    }
                                    else{
                                        for (int i=0; i<lineaSeparada.size(); i++) {
                                            salida<<lineaSeparada[i]<<" ";
                                        }
                                        salida<<endl;
                                        salida<<"Error: el tamaño del proceso es muy grande para cargarse en memoria real"<<endl;
                                        salida<<endl<<endl;
                                    }
                                }
                                else{
                                    for (int i=0; i<lineaSeparada.size(); i++) {
                                        salida<<lineaSeparada[i]<<" ";
                                    }
                                    salida<<endl;
                                    salida<<"Error: id invalido"<<endl;
                                    salida<<endl<<endl;
                                }
                            }
                            else{
                                for (int i=0; i<lineaSeparada.size(); i++) {
                                    salida<<lineaSeparada[i]<<" ";
                                }
                                salida<<endl;
                                salida<<"Error: tamanio invalido"<<endl;
                                salida<<endl<<endl;
                            }
                        }
                        else{
                            for (int i=0; i<lineaSeparada.size(); i++) {
                                salida<<lineaSeparada[i]<<" ";
                            }
                            salida<<endl;
                            salida<<"Error: parametros incorrectos"<<endl;
                            salida<<endl<<endl;
                        }
                    break;
                    
                    case 'A':
                        if (lineaSeparada.size()==4&&(lineaSeparada[3].size()==1)&&(lineaSeparada[3][0]=='0'||lineaSeparada[3][0]=='1')) {
                            long int id= convierteALong(lineaSeparada[2]);
                            if (id!=-1) {
                                if (existeProceso(id)) {
                                    int dir= convierteANum(lineaSeparada[1]);
                                    if (dir<tamProceso(id)) {
                                        if (lineaSeparada[3][0]=='0'){
                                            for (int i=0; i<lineaSeparada.size(); i++) {
                                                salida<<lineaSeparada[i]<<" ";
                                            }
                                            salida<<endl;
                                            accesarDireccion(dir, id, false);
                                        }
                                        else{
                                            for (int i=0; i<lineaSeparada.size(); i++) {
                                                salida<<lineaSeparada[i]<<" ";
                                            }
                                            salida<<endl;
                                            accesarDireccion(dir, id, true);
                                        }
                                    }
                                    else{
                                        for (int i=0; i<lineaSeparada.size(); i++) {
                                            salida<<lineaSeparada[i]<<" ";
                                        }
                                        salida<<endl;
                                        salida<<"Error: direccion fuera de rango"<<endl;
                                        salida<<endl<<endl;
                                    }
                                }
                                else{
                                    for (int i=0; i<lineaSeparada.size(); i++) {
                                        salida<<lineaSeparada[i]<<" ";
                                    }
                                    salida<<endl;
                                    salida<<"Error: ese proceso no existe"<<endl;
                                    salida<<endl<<endl;
                                }
                            }
                            else{
                                for (int i=0; i<lineaSeparada.size(); i++) {
                                    salida<<lineaSeparada[i]<<" ";
                                }
                                salida<<endl;
                                salida<<"Error: id invalido"<<endl;
                                salida<<endl<<endl;
                            }
                        }
                        else{
                            for (int i=0; i<lineaSeparada.size(); i++) {
                                salida<<lineaSeparada[i]<<" ";
                            }
                            salida<<endl;
                            salida<<"Error: parametros incorrectos"<<endl;
                            salida<<endl<<endl;
                        }
                    break;
                    
                    case 'L':
                        if (lineaSeparada.size()==2) {
                            long int id= convierteALong(lineaSeparada[1]);
                            if (id!=-1) {
                                if (existeProceso(id)) {
                                    for (int i=0; i<lineaSeparada.size(); i++) {
                                        salida<<lineaSeparada[i]<<" ";
                                    }
                                    salida<<endl;
                                    salida<<"Los marcos de pagina liberados en memoria real fueron: ";
                                    liberarProceso(id);
                                }
                                else{
                                    for (int i=0; i<lineaSeparada.size(); i++) {
                                        salida<<lineaSeparada[i]<<" ";
                                    }
                                    salida<<endl;
                                    salida<<"Error: ese proceso no existe"<<endl;
                                    salida<<endl<<endl;
                                }
                            }
                            else{
                                for (int i=0; i<lineaSeparada.size(); i++) {
                                    salida<<lineaSeparada[i]<<" ";
                                }
                                salida<<endl;
                                salida<<"Error: invalido"<<endl;
                                salida<<endl<<endl;
                            }
                        }
                        else{
                            for (int i=0; i<lineaSeparada.size(); i++) {
                                salida<<lineaSeparada[i]<<" ";
                            }
                            salida<<endl;
                            salida<<"Error: parametros incorrectos"<<endl;
                            salida<<endl<<endl;
                        }
                    break;
                    
                    case 'F':
                        if (lineaSeparada.size()==1) {
                            for (int i=0; i<lineaSeparada.size(); i++) {
                                salida<<lineaSeparada[i]<<" ";
                            }
                            salida<<endl;
                            reporte();
                            reinicioBestial();
                        }else{
                            for (int i=0; i<lineaSeparada.size(); i++) {
                                salida<<lineaSeparada[i]<<" ";
                            }
                            salida<<endl;
                            salida<<"Error: parametros incorrectos"<<endl;
                            salida<<endl<<endl;
                        }
                    break;
                    
                    case 'E':
                        salida<<"babay"<<endl;
                        entrada.close();
                        return 0;
                    break;
                    
                    default:
                    break;
            }
        }
        else{
        }
        lineaSeparada.clear();
    }
    entrada.close();
    salida.close();
    
    return 0;
}
