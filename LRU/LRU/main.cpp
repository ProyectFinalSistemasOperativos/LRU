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

using namespace std;


int main(int argc, const char * argv[]) {
    
    ifstream entrada;
    entrada.open ("input.txt");
    string linea;
    entrada >>linea;//P 2048 1
    entrada.close();
    
    ofstream salida;
    salida.open("output.txt");
    salida <<"escribir o variables al text. \n";
    salida.close();
    
    return 0;
}
