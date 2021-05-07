#ifndef FUZZYSECTOR_H
#define FUZZYSECTOR_H

#include <string.h>
#include <stdlib.h>

#include "Sector.h"

class FuzzySector{
    public:
        FuzzySector(Sector* s){
            this->squaredMagnitudes = (double*)malloc(sizeof(double) * s->dataLength / 2);
            this->length = s->dataLength / 2;
            this->sector = s;

            __complex__ double* tmp = (__complex__ double*)malloc(sizeof(__complex__ double) * s->dataLength);
            for(int i=0; i<s->dataLength; i++){ tmp[i] = s->data[i]; }
            int partialDataLength = s->dataLength / FuzzySector::sectionCount;

            for(int i=0; i<FuzzySector::sectionCount; i++){
                fft(tmp+i*partialDataLength, partialDataLength);
                this->squaredMagnitudes[i*partialDataLength/2] = 0;
                for(int j=1; j<partialDataLength/2; j++){
                    this->squaredMagnitudes[i*partialDataLength/2 + j] = sqrt(creal(tmp[j])*creal(tmp[j]) + cimag(tmp[j])*cimag(tmp[j]));
                }
            }

            free(tmp);
        }

        ~FuzzySector(){
            free(this->squaredMagnitudes);
        }

        Sector* getSector(){
            return this->sector;
        }

        static double distance(FuzzySector* s1, FuzzySector* s2){
            double minAvgDistance = FuzzySector::distanceAtOffset(s1, s2, 0);
            int sectionSize = s1->length / FuzzySector::sectionCount;

            for(int i=1; i<FuzzySector::sectionCount; i++){
                double distance1 = FuzzySector::distanceAtOffset(s1, s2, i*sectionSize);
                double distance2 = FuzzySector::distanceAtOffset(s2, s1, i*sectionSize);

                double avgDistance = distance1<distance2 ? distance1 : distance2;

                if(avgDistance < minAvgDistance){
                    minAvgDistance = avgDistance;
                }
            }

            return minAvgDistance;
        }

    private:
        static double distanceAtOffset(FuzzySector* s1, FuzzySector* s2, int offset){
            double avgSquaredDistance = 0;
            int range = s2->length - offset;
            
            for(int i=1; i<range; i++){
                double difference = s1->squaredMagnitudes[i+offset] - s2->squaredMagnitudes[i];
                avgSquaredDistance += difference * difference / (range-1);
            }

            return avgSquaredDistance;
        }

        double* squaredMagnitudes;
        Sector* sector;
        int length;

        static int sectionCount;
};

int FuzzySector::sectionCount = 2;

#endif