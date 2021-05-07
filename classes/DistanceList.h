#ifndef DISTANCELIST_H
#define DISTANCELIST_H

#include "E:/Desktop/Libraries/C++/debugMalloc.h"
#include "E:/Desktop/Libraries/C++/Graphing/graph.h"
#include "Sector.h"

typedef struct SectorStack{
    int* indexes;
    int height;
} SectorStack;

class DistanceList{
    public:
        DistanceList(Sector** sectors, int sectorCount, int maxOffset, int widthOffset, double (*distanceFunction)(Sector*, Sector*, int)):
            sectorCount(sectorCount),
            maxOffset(maxOffset)
        {
            this->adjacencyMatrix = (float*)malloc(sizeof(float) * this->sectorCount*this->sectorCount*this->maxOffset);
            for(int i=0; i<this->sectorCount; i++){
                for(int j=0; j<this->sectorCount; j++){
                    if(i != j){
                        for(int k=0; k<this->maxOffset; k++){
                            this->adjacencyMatrix[i*this->sectorCount*this->maxOffset + j*this->maxOffset + k] = (float)distanceFunction(sectors[i], sectors[j], (k+1)*widthOffset);
                        }
                    }
                }
            }
        }

        ~DistanceList(){
            free(this->adjacencyMatrix);
        }

        float getDistance(int a, int b, int offset) const{
            return this->adjacencyMatrix[a*this->sectorCount*this->maxOffset + b*this->maxOffset + offset];
        }

        SectorStack stitch(){

            // initialize the pairs list
            pair* pairsList = (pair*)malloc(sizeof(pair) * this->sectorCount * (this->sectorCount-1) * this->maxOffset);
            int pairCount = 0;

            for(int i=0; i<this->sectorCount; i++){
                for(int j=i+1; j<this->sectorCount; j++){
                    for(int k=1; k<=this->maxOffset; k++){
                        pairsList[pairCount].a = i;
                        pairsList[pairCount].b = j;
                        pairsList[pairCount].offset = k;
                        pairsList[pairCount].distance = this->getDistance(i, j, k-1);
                        pairCount += 1;

                        pairsList[pairCount].a = j;
                        pairsList[pairCount].b = i;
                        pairsList[pairCount].offset = k;
                        pairsList[pairCount].distance = this->getDistance(j, i, k-1);
                        pairCount += 1;
                    }
                }
            }

            // initialize the stacks
            SectorStack** stackMap = (SectorStack**)malloc(sizeof(SectorStack*) * this->sectorCount);
            for(int i=0; i<this->sectorCount; i++){ stackMap[i] = DistanceList::initializeStack(i); }

            int* offsetMap = (int*)calloc(sizeof(int), this->sectorCount);

            // int* uniqueStacks = (int*)malloc(sizeof(int) * this->sectorCount);
            // int stackCount = this->sectorCount;
            // for(int i=0; i<this->sectorCount; i++){ uniqueStacks[i] = i; }

            // for(int edgeCount=0; edgeCount<this->sectorCount-1; edgeCount++){
            for(int edgeCount=0; edgeCount<this->sectorCount-1; edgeCount++){

                // find the closeset pairs
                int minIndex = 0;
                for(int i=0; i<pairCount; i++){
                    if(pairsList[i].distance < pairsList[minIndex].distance){
                        minIndex = i;
                    }
                }
                pair minPair = pairsList[minIndex];

                // create a new stitch with the new pairs
                SectorStack* oldStackA = stackMap[minPair.a];
                SectorStack* oldStackB = stackMap[minPair.b];
                SectorStack* newStack = mergeStacks(oldStackA, oldStackB, minPair.offset);

                // if(minPair.offset > 1){
                //     printf("Closeset: %d  %d=>%d @ %d  %lf\n", minIndex, minPair.a, minPair.b, minPair.offset, minPair.distance);
                //     printf("Old SectorStack A: "); DistanceList::printStack(oldStackA);
                //     printf("Old SectorStack B: "); DistanceList::printStack(oldStackB);
                //     printf("New SectorStack: "); DistanceList::printStack(newStack);
                //     printf("%.2lf%%\n\n", edgeCount*100/(double)(this->sectorCount-1)); 
                // }

                // fix the offsets of the B indexes
                for(int i=0; i<oldStackB->height; i++){
                    if(oldStackB->indexes[i] >= 0){
                        offsetMap[oldStackB->indexes[i]] += (minPair.offset-1) + oldStackA->height;
                    }
                }

                int newPairCount = 0;
                for(int i=0; i<pairCount; i++){

                    // these stitches are no longer available since the space is now taken
                    if(pairsList[i].offset == minPair.offset){
                        if(pairsList[i].a == minPair.a){ continue; }
                        if(pairsList[i].b == minPair.b){ continue; }
                    }

                    // these stitches need to have distances updated because of changing components
                    int tmp = ((stackMap[pairsList[i].a] == oldStackA) << 3) |
                              ((stackMap[pairsList[i].b] == oldStackB) << 2) |
                              ((stackMap[pairsList[i].a] == oldStackB) << 1) |
                              ((stackMap[pairsList[i].b] == oldStackA) << 0);

                    switch(tmp){

                        // matching SectorStack maps
                        case 0b1100: { continue; }
                        case 0b0011: { continue; }

                        // update distance from newStack to pairsList[i].b
                        case 0b1000: {
                            pairsList[i].offset += oldStackB->height;
                            if(pairsList[i].offset > this->maxOffset){ continue; }
                            // pairsList[i].distance = this->stackDistance(newStack, stackMap[pairsList[i].b], pairsList[i].offset);
                            break;
                        }

                        // offset does not change since newB is pair.a; the distance between doesnt change since no new things are added between them
                        case 0b0010: {
                            // pairsList[i].distance = this->stackDistance(newStack, stackMap[pairsList[i].b], pairsList[i].offset);
                            break;
                        }

                        // update distance from pairsList[i].a to newStack
                        case 0b0100: {
                            pairsList[i].offset += oldStackA->height;
                            if(pairsList[i].offset > this->maxOffset){ continue; }
                            // pairsList[i].distance = this->stackDistance(stackMap[pairsList[i].a], newStack, pairsList[i].offset);
                            break;
                        }

                        // offset does not change since newA is pair.b; the distance between doesnt change since no new things are added between them
                        case 0b0001: {
                            // pairsList[i].distance = this->stackDistance(stackMap[pairsList[i].a], newStack, pairsList[i].offset);
                            break;
                        }
                    }

                    pairsList[newPairCount] = pairsList[i];
                    newPairCount += 1;
                }


                for(int i=0; i<sectorCount; i++){
                    if(stackMap[i] == oldStackA || stackMap[i] == oldStackB){
                        stackMap[i] = newStack;
                    }
                }

                // delete the old stacks
                free(oldStackA->indexes);
                free(oldStackB->indexes);
                free(oldStackA);
                free(oldStackB);

                pairCount = newPairCount;
            }

            SectorStack stack = *stackMap[0];

            free(pairsList);
            free(offsetMap);
            free(stackMap[0]);
            free(stackMap);
            
            return stack;
        }



    private:
        typedef struct pair{
            int a;
            int b;
            int offset;
            float distance;
        } pair;

        static int max(int a, int b){
            return a>b ? a : b;
        }

        static int min(int a, int b){
            return a<b ? a : b;
        }

        static SectorStack* initializeStack(int startIndex){
            SectorStack* newStack = (SectorStack*)malloc(sizeof(SectorStack));
            newStack->indexes = (int*)malloc(sizeof(int));
            newStack->indexes[0] = startIndex;
            newStack->height = 1;
            return newStack;
        }

        static SectorStack* mergeStacks(SectorStack* s1, SectorStack* s2, int offset){
            SectorStack* newStack = (SectorStack*)malloc(sizeof(SectorStack));

            newStack->height = s1->height + s2->height + (offset-1);
            newStack->indexes = (int*)malloc(sizeof(int) * newStack->height);
            memcpy(newStack->indexes, s1->indexes, sizeof(int)*s1->height);
            for(int i=s1->height; i<s1->height+offset-1; i++){ newStack->indexes[i] = -1; }
            memcpy(newStack->indexes+s1->height+offset-1, s2->indexes, sizeof(int)*s2->height);

            return newStack;
        }

        static void printStack(SectorStack* s){
            printf("%d", s->indexes[0]);
            for(int i=1; i<s->height; i++){
                printf("=>%d", s->indexes[i]);
            }
            printf("\n");
        }

        float stackDistance(SectorStack* s1, SectorStack* s2, int offset){
            float distance = 0;
            int maxOffset = min(min(s1->height, s2->height-offset+1), this->maxOffset);

            for(int i=offset; i<=maxOffset; i++){
                for(int j=0; j<i; j++){
                    int index1 = s1->indexes[s1->height-j-1];
                    if(index1 == -1){ continue; }

                    int index2 = s2->indexes[i-j-offset];
                    if(index2 == -1){ continue; }
                    
                    distance += this->getDistance(index1, index2, i-1);
                }
            }

            return distance;
        }

        // float*** adjacencyMatrix; // sectorCount x sectorCount x maxOffset
        float* adjacencyMatrix; // sectorCount x sectorCount x maxOffset
        int sectorCount;
        int maxOffset;
};

#endif