#ifndef FUZZYDISTANCEMATRIX_H
#define FUZZYDISTANCEMATRIX_H

#include "FuzzySector.h"

class FuzzyDistanceMatrix{
    public:
        FuzzyDistanceMatrix(Sector** sectors, int sectorCount):
            sectors(sectors),
            sectorCount(sectorCount)
        {
            this->fuzzySectors = (FuzzySector**)malloc(sizeof(FuzzySector*) * this->sectorCount);
            for(int i=0; i<this->sectorCount; i++){ this->fuzzySectors[i] = new FuzzySector(sectors[i]); }

            this->distances = (double**)malloc(sizeof(double*)*sectorCount);
            for(int i=0; i<this->sectorCount; i++){
                this->distances[i] = (double*)malloc(sizeof(double) * this->sectorCount-i-1);
                for(int j=i+1; j<this->sectorCount; j++){
                    this->distances[i][j-i-1] = FuzzySector::distance(this->fuzzySectors[i], this->fuzzySectors[j]);
                }
            }
        }

        ~FuzzyDistanceMatrix(){
            for(int i=0; i<this->sectorCount; i++){ delete this->fuzzySectors[i]; }
            free(this->fuzzySectors);
            free(this->distances);
        }

        double getDistance(int index1, int index2){
            if(index1 < index2){ return this->distances[index1][index2-index1-1]; }
            if(index1 > index2){ return this->distances[index2][index1-index2-1]; }
            return 0;
        }

        // returns an n*2 array if integer indexes, neighboring indexes are closest
        int* getClosestN(int n, double** distances_=NULL){
            int* closestsIndexes = (int*)calloc(sizeof(int), n * 2);
            double* closestDistances = (double*)malloc(sizeof(double) * n);
            for(int i=0; i<n; i++){ closestDistances[i] = INFINITE; }

            for(int i=0; i<this->sectorCount; i++){
                for(int j=i+1; j<this->sectorCount; j++){
                    double distance = this->distances[i][j-i-1];

                    if(distance < closestDistances[n-1]){
                        int insertionIndex = n-1;
                        while(insertionIndex && distance < closestDistances[insertionIndex-1]){
                            insertionIndex -= 1;
                        }

                        memmove(closestsIndexes+(insertionIndex+1)*2, closestsIndexes+insertionIndex*2, sizeof(int)*(n-insertionIndex-1)*2);
                        memmove(closestDistances+insertionIndex+1, closestDistances+insertionIndex, sizeof(double)*(n-insertionIndex-1));

                        closestsIndexes[insertionIndex*2] = i;
                        closestsIndexes[insertionIndex*2+1] = j;
                        closestDistances[insertionIndex] = distance;
                    }
                }
            }

            if(distances_){ *distances_ = closestDistances; }
            else{ free(closestDistances); }

            return closestsIndexes;
        }

        FuzzySector** fuzzySectors;
        Sector** sectors;
        double** distances;
        int sectorCount;
};

#endif