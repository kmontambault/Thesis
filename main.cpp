#include "E:/Desktop/Libraries/C++/fft.h"
#include "E:/Desktop/Libraries/C++/Image/bmp.h"
#include "E:/Desktop/Libraries/C++/timer.h"
#include "E:/Desktop/Libraries/C++/Graphing/graph.h"

#include "classes/FuzzyDistanceMatrix.h"
#include "classes/DistanceList.h"
#include "classes/Sector.h"






int getInvCount(int* sectorOrder, int sectorCount){
    int inversions = 0;
    for (int i=0; i<sectorCount-1; i++){
        for (int j=i+1; j<sectorCount; j++){
            if(sectorOrder[i] > -1 && sectorOrder[j] > -1 && sectorOrder[i] > sectorOrder[j]){
                inversions += 1;
            }
        }
    }
 
    return inversions;
}





double sectorDistance(Sector* s1, Sector* s2, int offset){
    double range = s1->dataLength - offset;
    double avgDistance = 0;

    // int sumS1Change = 0;
    // int sumS2Change = 0;
    // int lastS1 = 0;
    // int lastS2 = 0;

    for(int i=0; i<s1->dataLength-offset; i++){
        int s1v = s1->data[i+offset];
        int s2v = s2->data[i];
        
        int difference = s1v - s2v;
        // int s1Change = s1v - lastS1;
        // int s2Change = s2v - lastS2;

        // sumS1Change += s1Change * s1Change;
        // sumS2Change += s2Change * s2Change;
        avgDistance += difference * difference;

        // lastS1 = s1v;
        // lastS2 = s2v;
    }

    avgDistance /= 255*255 * range;
    // sumS1Change /= range;
    // sumS2Change /= range;

    // return avgDistance / sumS1Change / sumS2Change;
    return avgDistance;
}

double stackDistance(Sector** sectors, int sectorSize, int widthOffset, SectorStack* s1, SectorStack* s2, int offset){
    double avgDistance = 0;
    int used = 0;
    int start;
    int end;

    offset += 1;

    if(offset < 0){
        start = 0;
        end = s1->height>s2->height ? s2->height+offset : s1->height;
    }else{
        start = offset;
        end = s2->height+offset<s1->height ? s2->height+offset : s1->height;
    }

    for(int i=start; i<end; i++){
        if(s1->indexes[i] >=0 && s2->indexes[i-offset] >= 0){
            avgDistance += sectorDistance(sectors[s1->indexes[i]], sectors[s2->indexes[i-offset]], sectorSize-widthOffset);
            used += 1;
        }
    }

    return avgDistance / (255*255 * used);
}


int initializeSectors(const char* imagePath, int sectorSize, Sector*** sectors, int* sectorCount){
    BMP img(imagePath);
    if(img.getDataLength() == 0){ return 0; }

    unsigned char* data = img.getRawData();
    int dataLen = img.getDataLength();

    *sectorCount = (dataLen+sectorSize-1) / sectorSize;
    *sectors = (Sector**)malloc(sizeof(Sector*) * (*sectorCount));

    for(int i=0; i<(*sectorCount)-1; i++){ (*sectors)[i] = new Sector(data+i*sectorSize, sectorSize, sectorSize); }
    (*sectors)[(*sectorCount)-1] = new Sector(data+((*sectorCount)-1)*sectorSize, sectorSize, dataLen%sectorSize);

    int newSectorCount = 0;
    for(int i=0; i<(*sectorCount); i++){
        // if(rand() % 100 < 50){ delete (*sectors)[i]; continue; }
        (*sectors)[newSectorCount++] = (*sectors)[i];
    }
    *sectorCount = newSectorCount;

    return img.getWidth() * 3;
}

int getWidthOffset(FuzzyDistanceMatrix* fuzzyDistances, int sectorSize, double useOnly, double* confidence_){
    Sector** sectors = fuzzyDistances->sectors;
    int sectorCount = fuzzyDistances->sectorCount;
    int n = sectorCount * useOnly;

    int* closest = fuzzyDistances->getClosestN(n);
    int* counts = (int*)calloc(sizeof(int), sectorSize/2);


    FILE* outfile = fopen("output.txt", "a");
    int widthOffset = 0;
    int largestCount = 0;
    for(int i=0; i<n; i++){

        double distance;
        int offset = Sector::findBestOffset(sectors[closest[i*2]], sectors[closest[i*2+1]], &distance);

        fprintf(outfile, "%d,", abs(closest[i*2]-closest[i*2+1]));
        counts[offset] += 1;
        if(counts[offset] > largestCount){
            largestCount = counts[offset];
            widthOffset = offset;
        }
    }

    free(closest);

    fprintf(outfile, "\n");
    fclose(outfile);

    if(confidence_){
        // Graph g;
        // // g.setAttribute("width", "1850");
        // g.setAttribute("yLabel", "Frequency");
        // g.setAttribute("xLabel", "Offset");
        // g.setAttribute("title", "Frequency of offset among top 5% most similar sectors");

        // PlotLayer* layer = g.addLayer(" ");
        // layer->setAttribute("type", "bar");


        // for(int i=0; i<sectorSize/2; i++){
        //     (*layer) << counts[i];
        // }

        // g.show();

        *confidence_ = largestCount / (double)n;
    }
    return widthOffset;
}

int getByteDepth(Sector** sectors, int sectorCount, int sectorSize, double useOnly, double* confidence_){
    int possibleDepths[3] = {3, 4, 6};
    double depthScores[3] = {0, 0, 0};
    const int possibilityCount = sizeof(possibleDepths) / sizeof(int);

    for(int i=0; i<sectorCount*useOnly; i++){
        int index = rand() % sectorCount;
        Sector* s = sectors[index];

        for(int j=0; j<possibilityCount; j++){
            int lastSum = 0;
            for(int k=0; k<possibleDepths[j]; k++){
                lastSum += s->data[k];
            }

            int count = sectorSize/possibleDepths[j]-1;
            for(int k=0; k<count; k++){
                int newSum = 0;
                for(int l=0; l<possibleDepths[j]; l++){
                    newSum += s->data[k*possibleDepths[j]+l];
                }

                depthScores[j] += (double)(newSum-lastSum)*(newSum-lastSum) / count;
                lastSum = newSum;
            }
        }
    }

    int minIndex = 0;
    for(int i=1; i<possibilityCount; i++){
        if(depthScores[i] < depthScores[minIndex]){
            minIndex = i;
        }
    }

    if(confidence_){
        double scoreSum = 0;
        for(int i=0; i<possibilityCount; i++){
            scoreSum += depthScores[i];
        }

        *confidence_ = 1 - (depthScores[minIndex] / scoreSum);
    }

    return possibleDepths[minIndex];
}

double* getHorizontalBreaks(Sector** sectors, int sectorSize, SectorStack* stack, int byteDepth, int widthOffset){
    double* differences = (double*)calloc(sizeof(double), (stack->height*widthOffset+sectorSize));
    int* counts = (int*)calloc(sizeof(int), (stack->height*widthOffset+sectorSize));

    int index = 0;
    for(int i=0; i<stack->height; i++){
        if(stack->indexes[i] >= 0){
            for(int j=byteDepth; j<sectorSize-byteDepth; j++){
                int v1 = 0;
                int v2 = 0;
                for(int k=0; k<byteDepth; k++){
                    v1 += sectors[stack->indexes[i]]->data[j+k];
                    v2 += sectors[stack->indexes[i]]->data[j-byteDepth+k];
                }
                differences[index + j] += (v1-v2) * (v1-v2);
                counts[index + j] += 1;
            }
        }

        index += widthOffset;
    }

    free(counts);

    return differences;
}

// TODO: should test other multiples of the "minimum width"
int getWidth(double* horizontalBreaks, int breakCount, int minimumWidth1, int minimumWidth2, int widthIncriment, char* suspectedUpsideDown, double* suspectedWidthConfidence, double* suspectedUpsideDownConfidence){
    int bestWidth1 = 0;
    long long bestValue1 = 0;

    int testWidth = minimumWidth1;
    while(testWidth < breakCount){
        long long sumValue = 0;
        for(int i=0; i<breakCount-testWidth; i++){ sumValue += horizontalBreaks[i] * horizontalBreaks[i+testWidth]; }

        if(sumValue > bestValue1){
            bestValue1 = sumValue;
            bestWidth1 = testWidth;
        }

        testWidth += widthIncriment;
    }

    int bestWidth2 = 0;
    long long bestValue2 = 0;
    testWidth = minimumWidth2;
    while(testWidth < breakCount){
        long long sumValue = 0;
        for(int i=0; i<breakCount-testWidth; i++){ sumValue += horizontalBreaks[i] * horizontalBreaks[i+testWidth]; }

        if(sumValue > bestValue2){
            bestValue2 = sumValue;
            bestWidth2 = testWidth;
        }

        testWidth += widthIncriment;
    }

    if(bestValue1 > bestValue2){
        *suspectedUpsideDownConfidence = bestValue1 / (double)(bestValue1 + bestValue2);
        *suspectedUpsideDown = 0;
        return bestWidth1;
    }

    *suspectedUpsideDownConfidence = bestValue2 / (double)(bestValue1 + bestValue2);
    *suspectedUpsideDown = 1;
    return bestWidth2;
}

int getWidth2(int stitchHeight, int minimumWidth, int widthIncriment){
    int minimumIndex = 0;
    double minimumValue = 1;
    for(int i=0; i<stitchHeight; i++){
        double sectorWidth = stitchHeight / (double)(minimumWidth + i*widthIncriment);
        double value = fabs(1-(sectorWidth - (int)sectorWidth));
        if(value < minimumValue){
            minimumValue = value;
            minimumIndex = i;
        }
    }

    int bestWidth = minimumWidth + minimumIndex * widthIncriment;

    return bestWidth;
}



int* stitchStacks(Sector** sectors, int sectorSize, SectorStack* stack, int widthOffset, int suspectedWidth, int maxOffset, char upsideDown, double (*distanceFunction)(Sector*, Sector*, int), int* newSectorCount){
    int stackCount = (int)(suspectedWidth / (double)sectorSize + 0.5);
    int roughStackHeight = stack->height / stackCount;

    SectorStack* stacks = (SectorStack*)malloc(sizeof(SectorStack) * stackCount);

    int lastIndex = 0;
    for(int i=0; i<stackCount-1; i++){
        int j = (i+1) * roughStackHeight - maxOffset;
        double worstDistance = 0;
        int worstIndex = j;
        while(j<worstIndex + maxOffset*2){

            int fromIndex = j;
            while(stack->indexes[fromIndex] < 0){ fromIndex += 1; }

            int toIndex = fromIndex+1;
            while(stack->indexes[toIndex] < 0){ toIndex += 1; }

            double distance = distanceFunction(sectors[stack->indexes[fromIndex]], sectors[stack->indexes[toIndex]], (toIndex-fromIndex)*widthOffset);
            if(distance > worstDistance){
                worstDistance = distance;
                worstIndex = toIndex;
            }

            j = toIndex;
        }

        stacks[i].indexes = stack->indexes + lastIndex;
        stacks[i].height = worstIndex - lastIndex;
        lastIndex = worstIndex;
    }

    stacks[stackCount-1].indexes = stack->indexes + lastIndex;
    stacks[stackCount-1].height = stack->height - lastIndex;

    // trim any empty spaces off of the ends of the stacks
    for(int i=0; i<stackCount; i++){
        while(stacks[i].height && stacks[i].indexes[stacks[i].height-1] == -1){ stacks[i].height -= 1; }
        while(stacks[i].height && stacks[i].indexes[0] == -1){ stacks[i].indexes += 1; stacks[i].height -= 1; }
    }

    // find the largest difference between stack sizes to properly find the number of offsets possible
    int largestHeight = stacks[0].height;
    int smallestHeight = stacks[0].height;
    for(int i=1; i<stackCount; i++){
        if(stacks[i].height > largestHeight){
            largestHeight = stacks[i].height;
        }else if(stacks[i].height < smallestHeight){
            smallestHeight = stacks[i].height;
        }
    }

    typedef struct pair{
        int a;
        int b;
        int offset;
        float distance;
    } pair;

    // initialize the list to store distances
    pair* adjacencyList = (pair*)malloc(sizeof(pair) * stackCount*(stackCount-1)*(maxOffset*2 + largestHeight-smallestHeight));
    int pairCount = 0;

    // find the distances between all stacks at all possible offsets
    for(int i=0; i<stackCount; i++){
        for(int j=0; j<stackCount; j++){
            if(i == j){ continue; }

            for(int k=-maxOffset; k<abs(stacks[i].height - stacks[j].height)+maxOffset; k++){
                adjacencyList[pairCount].a = i;
                adjacencyList[pairCount].b = j;
                adjacencyList[pairCount].offset = k;
                adjacencyList[pairCount].distance = stackDistance(sectors, sectorSize, widthOffset, &stacks[i], &stacks[j], k);
                pairCount += 1;
            }
        }
    }

    int* fromSet = (int*)malloc(sizeof(int) * stackCount);
    int* toSet = (int*)malloc(sizeof(int) * stackCount);
    int* stitchId = (int*)malloc(sizeof(int) * stackCount);
    int* toOffsets = (int*)malloc(sizeof(int) * stackCount);
    for(int i=0; i<stackCount; i++){
        stitchId[i] = i;
        fromSet[i] = -1;
        toSet[i] = -1;
    }

    for(int edgeCount=0; edgeCount<stackCount-1; edgeCount++){
        int minIndex = 0;
        for(int i=1; i<pairCount; i++){
            if(adjacencyList[i].distance < adjacencyList[minIndex].distance){
                minIndex = i;
            }
        }

        pair minPair = adjacencyList[minIndex];
        fromSet[minPair.b] = minPair.a;
        toSet[minPair.a] = minPair.b;
        toOffsets[minPair.a] = minPair.offset;

        int oldStitchId = stitchId[minPair.b];
        for(int i=0; i<stackCount; i++){
            if(stitchId[i] == oldStitchId){
                stitchId[i] = stitchId[minPair.a];
            }
        }

        int newPairCount = 0;
        for(int i=0; i<pairCount; i++){
            if(adjacencyList[i].a == minPair.a){ continue; }
            if(adjacencyList[i].b == minPair.b){ continue; }
            if(stitchId[adjacencyList[i].a] == stitchId[adjacencyList[i].b]){ continue; }

            adjacencyList[newPairCount] = adjacencyList[i];
            newPairCount += 1;
        }

        pairCount = newPairCount;
    }

    free(adjacencyList);
    free(stitchId);

    int start = 0;
    for(int i=0; i<stackCount; i++){
        if(fromSet[i] < 0){
            start = i;
            break;
        }
    }
    free(fromSet);

    int currentOffset = 0;
    int minOffsetValue = 0;
    int maxHeight = stacks[start].height;

    int index = start;
    while(toSet[index] >= 0){
        currentOffset += toOffsets[index];
        if(currentOffset < minOffsetValue){ minOffsetValue = currentOffset; }

        if(currentOffset + stacks[toSet[index]].height > maxHeight){
            maxHeight = currentOffset + stacks[toSet[index]].height;
        }

        index = toSet[index];
    }
    maxHeight -= minOffsetValue;

    int** stackOrder = (int**)malloc(sizeof(int*) * stackCount);
    currentOffset = -minOffsetValue;
    index = start;
    for(int i=0; i<stackCount; i++){
        stackOrder[i] = (int*)malloc(sizeof(int) * maxHeight);
        for(int j=0; j<currentOffset; j++){ stackOrder[i][j] = -1; }
        memcpy(stackOrder[i]+currentOffset, stacks[index].indexes, sizeof(int)*stacks[index].height);
        for(int j=currentOffset+stacks[index].height; j<maxHeight; j++){ stackOrder[i][j] = -1; }
        currentOffset += toOffsets[index];
        index = toSet[index];
    }
    free(toSet);
    free(toOffsets);

    int sectorCount = maxHeight * stackCount;
    int* sectorOrder = (int*)malloc(sizeof(int) * (sectorCount));

    int k = 0;
    if(upsideDown){
        for(int i=0; i<maxHeight; i++){
            for(int j=0; j<stackCount; j++){
                sectorOrder[k++] = stackOrder[j][i];
            }
        }
    }else{
        for(int i=maxHeight-1; i>=0; i--){
            for(int j=0; j<stackCount; j++){
                sectorOrder[k++] = stackOrder[j][i];
            }
        }
    }

    for(int i=0; i<stackCount; i++){
        free(stackOrder[i]);
    }
    free(stackOrder);

    // trims empty sectors from the front and back
    int trimStart = 0;
    int trimEnd = sectorCount - 1;
    while(sectorOrder[trimStart] < 0){ trimStart += 1; }
    while(sectorOrder[trimEnd] < 0){ trimEnd -= 1; }
    *newSectorCount = trimEnd - trimStart;

    int* trimmedSectorOrder = (int*)malloc(sizeof(int) * (*newSectorCount));
    memcpy(trimmedSectorOrder, sectorOrder+trimStart, sizeof(int) * (*newSectorCount));

    free(sectorOrder);

    return trimmedSectorOrder;
}

void fixImageOffset(Sector** sectors, int** sectorOrder, int* sectorCount, int sectorSize, int widthOffset, int suspectedWidth, int widthIncriment, int byteDepth, char suspectedUpsideDown){
    unsigned char* tmp = (unsigned char*)malloc(sizeof(char) * byteDepth*2);
    
    int bestPixelOffset = 0;
    double bestValue = 0;
    double sumDifference = 0;
    for(int i=0; i<suspectedWidth; i+=byteDepth){
        double difference = 0;
        int count = 0;
        unsigned char* imageData;

        for(int j=i; j<(*sectorCount)*sectorSize; j+=suspectedWidth){

            int startIndex = (j % sectorSize);
            if(startIndex + byteDepth*2 > sectorSize){
                if(j/sectorSize+1 == (*sectorCount)){ break; }

                int sectorIndex1 = (*sectorOrder)[j / sectorSize];
                int sectorIndex2 = (*sectorOrder)[j / sectorSize + 1];
                if(sectorIndex1<0 || sectorIndex2<0){ imageData = NULL; }
                else{
                    imageData = tmp;
                    int used1 = sectorSize - startIndex;
                    int used2 = byteDepth*2 - used1;

                    memcpy(tmp, sectors[sectorIndex2]->data+startIndex, sizeof(char)*used1);
                    memcpy(tmp+used1, sectors[sectorIndex2]->data, sizeof(char)*used2);
                }

            }else{
                int sectorIndex = (*sectorOrder)[j / sectorSize];
                if(sectorIndex < 0){ imageData = NULL; }
                else{ imageData = sectors[sectorIndex]->data + startIndex; }
            }

            if(imageData){
                for(int k=0; k<byteDepth; k++){
                    int v1 = imageData[k];
                    int v2 = imageData[k + byteDepth];

                    difference += (v1-v2) * (v1-v2);
                    count += 1;
                }
            }
        }

        difference /= count;
        sumDifference += difference;

        if(difference > bestValue){
            bestValue = difference;
            bestPixelOffset = i;
        }
    }

    // // find the sector combination that will yeild within 'byteDepth' bytes of the pixel offset
    // int paddingSectorsNeeded = 1;
    // // if(suspectedUpsideDown){
    // //     paddingSectorsNeeded = bestPixelOffset / (sectorSize-widthOffset);
    // // }else{
    // //     paddingSectorsNeeded = bestPixelOffset / widthOffset;
    // // }

    // printf("%d %d\n", paddingSectorsNeeded, bestPixelOffset);

    // // printf("%d %d %d\n", bestPixelOffset, (widthIncriment / sectorSize), (widthIncriment / sectorSize) - paddingSectorsNeeded);
    // paddingSectorsNeeded = (widthIncriment / sectorSize) - paddingSectorsNeeded;


    // int newSectorCount = (*sectorCount) + paddingSectorsNeeded;
    // int* newSectorOrder = (int*)malloc(sizeof(int) * newSectorCount);

    // for(int i=0; i<paddingSectorsNeeded; i++){ newSectorOrder[i] = -1; }
    // memcpy(newSectorOrder+paddingSectorsNeeded, *sectorOrder, (*sectorCount)*sizeof(int));

    *sectorOrder += 3;
    *sectorCount -= 3;

    // free(*sectorOrder);
    // *sectorOrder = newSectorOrder;
    // *sectorCount = newSectorCount;

    free(tmp);
}


void constructImage(Sector** sectors, int sectorSize, int* sectorOrder, int newSectorCount, int width, int byteDepth, const char* path){
    BMP image((newSectorCount*sectorSize + width-1) / width, width/byteDepth);
    unsigned char* data = image.getRawData();
    int dataLength = image.getDataLength();

    int j = 0;
    for(int i=0; i<newSectorCount; i++){
        if(sectorOrder[i] < 0){
            memset(data+j, 0, sectorSize);
        }else{
            memcpy(data+j, sectors[sectorOrder[i]]->data, sectorSize);
        }

        j += sectorSize;
    }

    memset(data+j, 0, dataLength-j);

    image.save(path);
}

void saveStackImage(Sector** sectors, int sectorSize, int* sectorOrder, int sectorCount, int widthOffset, int byteDepth){
    int width = ((widthOffset*(sectorCount-1) + sectorSize) + byteDepth - 1) / byteDepth;
    int height = sectorCount;

    BMP image(height, width);
    unsigned char* data = image.getRawData();

    for(int i=0; i<sectorCount; i++){
        if(sectorOrder[i] >= 0){
            memcpy(data+i*width*byteDepth + i*widthOffset, sectors[sectorOrder[i]]->data, sectorSize);
        }
    }

    image.save("images/stack.bmp");
}

double getAccuracy(int* stitchOrder, int sectorCount, int targetOffset){
    int correct = 0;
    for(int i=1; i<sectorCount; i++){
        correct += abs(stitchOrder[i] - stitchOrder[i-1]) == targetOffset;
    }

    return (double)correct / (sectorCount-1);
}




int main(int argc, char** argv){
    if(argc < 2){
        printf("image path required\n");
        exit(0);
    }
    const char* imagePath = argv[1];
    const char* resultsPath = argc>=3 ? argv[2] : NULL;

    const int sectorSize = 512;
    const int maxOffset = 1;

    timer t(true);

    // initiailze the sectors from a provided image
    // in reality, this would simply load data from a file
    printf("%s Loading sectors\n", t.lapString());
    Sector** sectors;
    int sectorCount;
    int trueWidth = initializeSectors(imagePath, sectorSize, &sectors, &sectorCount);
    int trueByteDepth = 3;
    bool upsideDown = trueWidth%sectorSize > sectorSize/2;
    int trueOffset = upsideDown ? sectorSize-(trueWidth%sectorSize) : trueWidth%sectorSize;
    if(trueWidth == 0){ printf("Failed to load image\n"); exit(0); }
    printf("         Loaded %d sectors\n", sectorCount);

    printf("%s Determining byte depth\n", t.lapString());
    double byteDepthConfidence;
    int byteDepth = getByteDepth(sectors, sectorCount, sectorSize, .2, &byteDepthConfidence);
    printf("         Byte depth: %d (%.2lf%%)\n", byteDepth, byteDepthConfidence*100);

    // create the distance matrix for rough distances
    printf("%s Calculating rough distances\n", t.lapString());
    FuzzyDistanceMatrix fuzzyDistances(sectors, sectorCount);

    int trueSectorGap = (int)((trueWidth / (double)sectorSize)+0.5);
    FILE* outfile = fopen("output.txt", "a");
    fprintf(outfile, "%d ", trueSectorGap);
    fclose(outfile);

    // discovering the most likely width offset (difference between the width and the sector size)
    // TODO: change the confidence to be closer to an actual statistical confidence
    printf("%s Determining width offset\n", t.lapString());
    double useOnly = 0.3;
    double widthOffsetConfidence;
    int widthOffset = getWidthOffset(&fuzzyDistances, sectorSize, useOnly, &widthOffsetConfidence);
    printf("         Width offset: %d (%.2lf%%)\n", widthOffset, widthOffsetConfidence*100);
    exit(0);

    // determine the incriment that the width could grow in
    int widthIncriment = sectorSize;
    while(widthIncriment % byteDepth != 0){ widthIncriment += sectorSize; }
    printf("         Width incriment: %d pixels\n", widthIncriment / 3);

    // creates the list that the closest pairs will be pulled from
    printf("%s Calculating adjacency matrix\n", t.lapString());
    DistanceList distanceList(sectors, sectorCount, maxOffset, widthOffset, sectorDistance);

    // creates the ordered stack of sectors to later be split
    printf("%s Stacking sectors\n", t.lapString());
    SectorStack stack = distanceList.stitch();

    saveStackImage(sectors, sectorSize, stack.indexes, stack.height, widthOffset, byteDepth);

    // determine what the smallest size of the image could be
    // minimumWidth1 is best if the stack is up-side up
    int minimumWidth1 = sectorSize + widthOffset;
    for(int i=0; minimumWidth1 % byteDepth != 0; i++){
        minimumWidth1 += sectorSize;
        if(i == byteDepth){
            minimumWidth1 = INFINITY;
            break;
        }
    }

    // minimumWidth2 is best if the stack is up-side down
    int minimumWidth2 = sectorSize - widthOffset;
    for(int i=0; minimumWidth2 % byteDepth != 0; i++){
        minimumWidth2 += sectorSize;
        if(i == byteDepth){
            minimumWidth2 = INFINITY;
            break;
        }
    }

    // attempts to find vertical lines across the horizontal stack
    printf("%s Determining width\n", t.lapString());
    double* horizontalBreaks = getHorizontalBreaks(sectors, sectorSize, &stack, byteDepth, widthOffset);
    int breakCount = stack.height*widthOffset + sectorSize;
    char suspectedUpsideDown;
    double suspectedWidthConfidence;
    double suspectedUpsideDownConfidence;
    int suspectedWidth = getWidth(horizontalBreaks, breakCount, minimumWidth1, minimumWidth2, widthIncriment, &suspectedUpsideDown, &suspectedWidthConfidence, &suspectedUpsideDownConfidence);
    free(horizontalBreaks);
    printf("         Suspected width: %d (%.2lf%%)\n", suspectedWidth/3, suspectedWidthConfidence*100);
    printf("         Suspected upside down: %s (%.2lf%%)\n", suspectedUpsideDown?"True":"False", suspectedUpsideDownConfidence*100);

    // split the stack and stitch them back together in a rectangle
    printf("%s Stitching sector stacks\n", t.lapString());
    int newSectorCount;
    int* sectorOrder = stitchStacks(sectors, sectorSize, &stack, widthOffset, suspectedWidth, maxOffset, suspectedUpsideDown, sectorDistance, &newSectorCount);
    free(stack.indexes);

    // printf("%s Fixing horizontal offset\n", t.lapString());
    // fixImageOffset(sectors, &sectorOrder, &newSectorCount, sectorSize, widthOffset, suspectedWidth, widthIncriment, byteDepth, suspectedUpsideDown);


    // printf("%s Constructing image\n", t.lapString());
    // constructImage(sectors, sectorSize, sectorOrder, newSectorCount, suspectedWidth, byteDepth, "images/output2.bmp");

    double accuracy = 1 - (getInvCount(sectorOrder, newSectorCount) / (sectorCount*(sectorCount-1)/2.0));

    printf("%d\n", trueOffset);

    if(resultsPath){
        FILE* outfile = fopen(resultsPath, "w");
        fprintf(outfile, "Runtime: %d\n", t.milliseconds());
        fprintf(outfile, "Sectors: %d\n", sectorCount);
        fprintf(outfile, "Byte depth: %s\n", trueByteDepth==byteDepth ? "Pass" : "Fail");
        fprintf(outfile, "Offset: %s\n", trueOffset==widthOffset ? "Pass" : "Fail");
        fprintf(outfile, "Orientation: %s\n", upsideDown==suspectedUpsideDown ? "Pass" : "Fail");
        fprintf(outfile, "Width: %s\n", trueWidth==suspectedWidth ? "Pass" : "Fail");
        fprintf(outfile, "Reconstruction accuracy: %.2lf%%\n", accuracy*100);
        fclose(outfile);
    }

    // printf("%s Testing accuracy\n", t.lapString());
    // double accuracy = getAccuracy(stitchOrder, sectorCount, 4);
    // printf("         Accuracy: %.2lf%%\n", accuracy*100);

    printf("%s Finished\n", t.lapString());

    return 0;
}