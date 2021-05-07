#ifndef SECTORDISTANCELIST_H
#define SECTORDISTANCELIST_H

#include "DistanceList.h"
#include "Sector.h"

class SectorDistanceList: public DistanceList{
    public:
        SectorDistanceList(Sector** sectors, int sectorCount, int maxOffset, int widthOffset):
            DistanceList((void**)sectors, sectorCount)
        {

        }

    protected:
        double distance(){

        }

    private:
        int widthOffset;
        int maxOffset;
}


#endif