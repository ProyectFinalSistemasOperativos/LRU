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
#include <sstream>
#include <ctype.h>

using namespace std;

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

vector<Proceso> procesos;
vector<string> cambiados;

void reporteP(long int pId){
    cout<<"Se asignaron los marcos de pagina: ";
    for (int i=0; i<256; i++) {
        if (memoriaReal[i].getIdProceso()==pId) {
            cout<<i<<" ";
        }
    }
    cout<<"al proceso "<<pId<<endl;
    for (int i=0; i<cambiados.size(); i++) {
        cout<<cambiados[i]<<endl;
    }
    cambiados.clear();
}

void swapLRU1(Pagina nuevaPag){ //el swap de cuando se crea un proceso
    int sub=0;
    string cambiado;
    clock_t menor= clock();
    for (int i=0; i<256; i++) { //primero se encuentra el LRU en la memoria real
        if (memoriaReal[i].getUltimaModificacion()<menor&&(memoriaReal[i].getIdProceso()!=-1)) {
            menor= memoriaReal[i].getUltimaModificacion();
            sub= i;
        }
    }
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

void swapLRU2(int sub, int corrimiento, long int pId, int dir){ //el swap cuando se accesa a una direccion virtual
    int sub2= 0;
    bool encontrado= false;
    clock_t menor= memoriaReal[0].getUltimaModificacion();
    for (int i=0; i<256; i++) { // se encuentra el LRU en la memoria real
        if (memoriaReal[i].getUltimaModificacion()<menor) {
            menor= memoriaReal[i].getUltimaModificacion();
            sub2= i;
        }
    }
    
    cout<<(sub2*8)+corrimiento<<endl;
    cout<<"Se localizo la pagina "<<dir/8<<" del proceso "<<pId<<" que estaba en la posicion "<<sub<<" de Swapping y se cargo al marco "<<sub2<<endl;
    Pagina aux= memoriaReal[sub2];
    memoriaReal[sub2]= memoriaSwap[sub];
    memoriaReal[sub2].referenciar();
    for (int i=0; i<512&&(!encontrado); i++) {
        if (memoriaSwap[i].getIdProceso()==-1) {
            memoriaSwap[i]=aux;
            cout<<"La pagina "<<sub<<" del proceso "<<pId<<" fue Swappeada a la poicion "<<i<<" del area de Swapping"<<endl;
            cout<<endl<<endl;
            encontrado= true;
        }
    }
    swapintotales++;
    swapouttotales++;
}

void cargarProceso(int tam, long int pId){
    vector<Pagina> cambiados;
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

void accesarDireccion(int dir, long int pId, bool modif){
    cout<<"Direccion Virtual: "<<dir<<" Direccion Real: ";
    int corrimiento= dir%8;
    dir /= 8;
    bool encontrado=false;
    for (int i=0; i<256&&(!encontrado); i++) {
        if (memoriaReal[i].getIdProceso()==pId&&(memoriaReal[i].getNumPagina()==dir)) { //si el marco estaba en la memoria real
            encontrado= true;
            cout<<(i*8)+corrimiento<<endl;
            cout<<endl<<endl;
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
                    cout<<(i*8)+corrimiento<<endl;
                    cout<<"Se localizo la pagina "<<dir/8<<" del proceso "<<pId<<" que estaba en la posicion "<<sub<<" de Swapping y se cargo al marco "<<i<<endl;
                    cout<<endl<<endl;
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

void liberarProceso(long int pId){
    Pagina nuevo;
    for (int i=0; i<256; i++) {
        if (memoriaReal[i].getIdProceso()==pId) {
            cout<<i<<" ";
            memoriaReal[i]= nuevo;
            espacioDisponible++;
        }
    }
    cout<<endl<<"Los marcos de pagina liberados en memoria de swap: ";
    for (int i=0; i<512; i++) {
        if (memoriaSwap[i].getIdProceso()==pId) {
            cout<<i<<" ";
            memoriaSwap[i]= nuevo;
            espacioDisponibleSwap++;
        }
    }
    cout<<endl<<"Que ocupaba el proceso: "<<pId<<endl;
    cout<<endl<<endl;
    bool encontrado= false;
    for (int i=0; i<procesos.size() && (!encontrado); i++) {
        if (procesos[i].id==pId) {
            procesos[i].stampLiberacion= clock();
            procesos[i].activo= false;
            encontrado= true;
        }
    }
}

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

bool existeProceso(long int pId){
    bool existe= false;
    for (int i=0; i<procesos.size()&&(!existe); i++) {
        if (pId==procesos[i].id) {
            existe= true;
        }
    }
    return existe;
}

int tamProceso(long int id){
    for (int i=0; i<procesos.size(); i++) {
        if (id==procesos[i].id) {
            return procesos[i].tamReal;
        }
    }
    return 0;
}

void reporte(){
    bool activos= false;
    for (int i=0; i<procesos.size()&&(!activos); i++) {
        if (procesos[i].activo) {
            activos= true;
        }
    }
    if (activos) {
        cout <<"Error. Proceso(s) todavia en memoria: ";
        for (int i=0; i<procesos.size(); i++) {
            if (procesos[i].activo) {
                cout <<procesos[i].id <<" ";
            }
        }
        cout<<endl;
    }
    double turnaround, turnaroundprom = 0;
    int inactivos = 0;
    cout<<"Fin. Reporte de salida: " <<endl <<"Turnarounds: \n";
    for(int i=0; i < procesos.size(); i++){
        if (!procesos[i].activo) {
            inactivos++;
            turnaround = (double)(procesos[i].stampLiberacion-procesos[i].stampCreacion);
            cout<<"Proceso " <<procesos[i].id <<" " <<turnaround<<endl;
            turnaroundprom += turnaround;
        }
    }
    if (inactivos>0) {
        turnaroundprom /= (double)inactivos;
        cout <<endl <<"Turnaround promedio: " <<turnaroundprom <<endl <<"Swaps: " <<endl;
    }
    else
        cout <<endl <<"Turnaround promedio: " <<0 <<endl <<"Swaps: " <<endl;
    cout <<"SwapIn: "<<swapintotales<<endl;
    cout<<"SwapOut: "<<swapouttotales<<endl;
    cout<<"Page faults: "<<endl;
    for (int i=0; i<procesos.size(); i++) {
        cout<<"Proceso "<<procesos[i].id<<": "<<procesos[i].faults<<endl;
    }
    cout<<endl<<endl;
}

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
    entrada.open ("/Users/axelsuarez/LRU/LRU/LRU/trve.txt");
    string linea;
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
                                                cout<<lineaSeparada[i]<<" ";
                                            }
                                            cout<<endl;
                                            cargarProceso(tam, id);
                                            reporteP(id);
                                            cout<<endl<<endl;
                                        }
                                        else{
                                            for (int i=0; i<lineaSeparada.size(); i++) {
                                                cout<<lineaSeparada[i]<<" ";
                                            }
                                            cout<<endl;
                                            cout<<"Error: ese proceso ya existe"<<endl;
                                            cout<<endl<<endl;
                                        }
                                    }
                                    else{
                                        for (int i=0; i<lineaSeparada.size(); i++) {
                                            cout<<lineaSeparada[i]<<" ";
                                        }
                                        cout<<endl;
                                        cout<<"Error: el tamaño del proceso es muy grande para cargarse en memoria real"<<endl;
                                        cout<<endl<<endl;
                                    }
                                }
                                else{
                                    for (int i=0; i<lineaSeparada.size(); i++) {
                                        cout<<lineaSeparada[i]<<" ";
                                    }
                                    cout<<endl;
                                    cout<<"Error: id invalido"<<endl;
                                    cout<<endl<<endl;
                                }
                            }
                            else{
                                for (int i=0; i<lineaSeparada.size(); i++) {
                                    cout<<lineaSeparada[i]<<" ";
                                }
                                cout<<endl;
                                cout<<"Error: tamanio invalido"<<endl;
                                cout<<endl<<endl;
                            }
                        }
                        else{
                            for (int i=0; i<lineaSeparada.size(); i++) {
                                cout<<lineaSeparada[i]<<" ";
                            }
                            cout<<endl;
                            cout<<"Error: parametros incorrectos"<<endl;
                            cout<<endl<<endl;
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
                                                cout<<lineaSeparada[i]<<" ";
                                            }
                                            cout<<endl;
                                            accesarDireccion(dir, id, false);
                                        }
                                        else{
                                            for (int i=0; i<lineaSeparada.size(); i++) {
                                                cout<<lineaSeparada[i]<<" ";
                                            }
                                            cout<<endl;
                                            accesarDireccion(dir, id, true);
                                        }
                                    }
                                    else{
                                        for (int i=0; i<lineaSeparada.size(); i++) {
                                            cout<<lineaSeparada[i]<<" ";
                                        }
                                        cout<<endl;
                                        cout<<"Error: direccion fuera de rango"<<endl;
                                        cout<<endl<<endl;
                                    }
                                }
                                else{
                                    for (int i=0; i<lineaSeparada.size(); i++) {
                                        cout<<lineaSeparada[i]<<" ";
                                    }
                                    cout<<endl;
                                    cout<<"Error: ese proceso no existe"<<endl;
                                    cout<<endl<<endl;
                                }
                            }
                            else{
                                for (int i=0; i<lineaSeparada.size(); i++) {
                                    cout<<lineaSeparada[i]<<" ";
                                }
                                cout<<endl;
                                cout<<"Error: id invalido"<<endl;
                                cout<<endl<<endl;
                            }
                        }
                        else{
                            for (int i=0; i<lineaSeparada.size(); i++) {
                                cout<<lineaSeparada[i]<<" ";
                            }
                            cout<<endl;
                            cout<<"Error: parametros incorrectos"<<endl;
                            cout<<endl<<endl;
                        }
                    break;
                    
                    case 'L':
                        if (lineaSeparada.size()==2) {
                            long int id= convierteALong(lineaSeparada[1]);
                            if (id!=-1) {
                                if (existeProceso(id)) {
                                    for (int i=0; i<lineaSeparada.size(); i++) {
                                        cout<<lineaSeparada[i]<<" ";
                                    }
                                    cout<<endl;
                                    cout<<"Los marcos de pagina liberados en memoria real fueron: ";
                                    liberarProceso(id);
                                }
                                else{
                                    for (int i=0; i<lineaSeparada.size(); i++) {
                                        cout<<lineaSeparada[i]<<" ";
                                    }
                                    cout<<endl;
                                    cout<<"Error: ese proceso no existe"<<endl;
                                    cout<<endl<<endl;
                                }
                            }
                            else{
                                for (int i=0; i<lineaSeparada.size(); i++) {
                                    cout<<lineaSeparada[i]<<" ";
                                }
                                cout<<endl;
                                cout<<"Error: invalido"<<endl;
                                cout<<endl<<endl;
                            }
                        }
                        else{
                            for (int i=0; i<lineaSeparada.size(); i++) {
                                cout<<lineaSeparada[i]<<" ";
                            }
                            cout<<endl;
                            cout<<"Error: parametros incorrectos"<<endl;
                            cout<<endl<<endl;
                        }
                    break;
                    
                    case 'F':
                        if (lineaSeparada.size()==1) {
                            for (int i=0; i<lineaSeparada.size(); i++) {
                                cout<<lineaSeparada[i]<<" ";
                            }
                            cout<<endl;
                            reporte();
                            reinicioBestial();
                        }else{
                            for (int i=0; i<lineaSeparada.size(); i++) {
                                cout<<lineaSeparada[i]<<" ";
                            }
                            cout<<endl;
                            cout<<"Error: parametros incorrectos"<<endl;
                            cout<<endl<<endl;
                        }
                    break;
                    
                    case 'E':
                        cout<<"babay"<<endl;
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
    
    ofstream salida;
    salida.open("/Users/Balbina/Documents/8vo semestre/Sistemas Operativo/2015/proyecto fina/LRU/LRU/LRU/output.txt");
    salida <<"escribir o variables al text. \n" <<"hola \n";
    salida.close();
    
    return 0;
}
