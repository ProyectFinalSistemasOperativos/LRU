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
    int estado;
    int pageFaults;
    time_t creacion;
    time_t ultimaModificacion;
    time_t borrado;
    
public:
    Pagina(int idProc);
    void referenciar();
    void modificar();
    void remover();
    void pageFault();
    void swap();
};


#endif
