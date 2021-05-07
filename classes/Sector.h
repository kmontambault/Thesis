#ifndef SECTOR_H
#define SECTOR_H

#include <string.h>
#include <stdlib.h>

class Sector{
    public:
        Sector(const unsigned char* data, int sectorSize, int inputLength):
            dataLength(sectorSize)
        {
            this->data = (unsigned char*)calloc(sizeof(char), sectorSize);
            memcpy(this->data, data, sizeof(char) * inputLength);
        }

        ~Sector(){
            free(this->data);
        }


        static double distanceAtOffset(Sector* s1, Sector* s2, int offset){
            if(offset < 0){ offset += s1->dataLength; }

            double range = s1->dataLength - offset;
            double avgDistance = 0;

            for(int i=0; i<s1->dataLength-offset; i++){
                int s1v = s1->data[i+offset];
                int s2v = s2->data[i];
                int difference = s1v - s2v;

                avgDistance += difference*difference / range;
            }

            return avgDistance;
            // return avgDistance / sqrt(range);
            // return avgDistance / (255*255) / sqrt(range);
        }

        static int findBestOffset(Sector* s1, Sector* s2, double* distance){
            double minDistance = Sector::distanceAtOffset(s2, s1, 0);
            int minOffset = 0;

            for(int i=1; i<s1->dataLength/2; i++){
                double distance1 = Sector::distanceAtOffset(s1, s2, i);
                double distance2 = Sector::distanceAtOffset(s2, s1, i);

                double avgDistance = distance1<distance2 ? distance1 : distance2;

                if(avgDistance < minDistance){
                    minDistance = avgDistance;
                    minOffset = i;
                }
            }

            *distance = minDistance;
            return minOffset;
        }

        unsigned char* data;
        int dataLength;
};

#endif