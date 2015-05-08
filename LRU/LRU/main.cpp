//
//  main.cpp
//  LRU
//
//  Created by Iker Arbulu Lozano on 5/8/15.
//  Copyright (c) 2015 Iker Arbulu Lozano. All rights reserved.
//

#include <iostream>
#include "Pagina.h"
#include <fstream>
#include <iostream>

using namespace std;


int main(int argc, const char * argv[]) {
    
    ifstream entrada;
    entrada.open ("/Users/Balbina/Documents/8vo semestre/Sistemas Operativo/2015/proyecto fina/LRU/LRU/LRU/input.txt");
    string linea;
    if (entrada.is_open()) {
        while (!entrada.eof()) {
            
            
            entrada >>linea;
            cout <<linea;
        }
    }
    entrada.close();
    
    ofstream salida;
    salida.open("/Users/Balbina/Documents/8vo semestre/Sistemas Operativo/2015/proyecto fina/LRU/LRU/LRU/output.txt");
    salida <<"escribir o variables al text.";
    salida.close();
    
    return 0;
}
