import subprocess, time
import os, sys



BASE_PATH = "E:/Datasets/Flowers/flowers_18_bmp"
RESULTS_PATH = "results"

completed = 0
startTime = time.time()

for i in range(1360):
    # subprocess.run(["main.exe", "%s/%04d.bmp" % (BASE_PATH, i+1), "%s/%04d.txt" % (RESULTS_PATH, i+1)], stdout=subprocess.DEVNULL)
    subprocess.run(["main.exe", "%s/%04d.bmp" % (BASE_PATH, i+1)], stdout=subprocess.DEVNULL)
    completed += 1
    elapsedTime = (time.time() - startTime) / 60

    print(completed, (elapsedTime*1360/completed) - elapsedTime)





# data = []
# for i in range(1360):
#     # try:
#     with open("results/%04d.txt"%(i+1), "r") as f:
#         lines = [l[:-1].split(": ")[-1] for l in f.readlines()]

#         millisecondsTaken = int(lines[0])

#         sectorCount       = int(lines[1])
#         passedByteDepth   = lines[2] == "Pass"
#         passedOffset      = lines[3] == "Pass"
#         passedOrientation = lines[4] == "Pass"
#         passedWidth       = lines[5] == "Pass"

#         accuracy = float(lines[6][:-1])

#         # print(millisecondsTaken, passedByteDepth, passedOffset, passedOrientation, passedWidth, accuracy)

#         if True:
#             data.append([
#                 sectorCount,
#                 millisecondsTaken / sectorCount,
#                 int(passedByteDepth),
#                 int(passedOffset),
#                 int(passedWidth),
#                 int(passedOrientation),
#                 accuracy
#             ])
#     # except:
#     #     pass


# names = [
#     "sectors:      ",
#     "milliseconds: ",
#     "depth:        ",
#     "offset:       ",
#     "width:        ",
#     "orientation:  ",
#     "accuracy:     "
# ]
# for i in range(len(data[0])):
#     valueSum = sum([d[i] for d in data])
#     print(names[i], valueSum / len(data))
# print("images:      ", len(data))