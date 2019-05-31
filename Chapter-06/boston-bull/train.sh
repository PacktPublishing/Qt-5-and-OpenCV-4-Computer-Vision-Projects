#!/bin/bash

# download data
if [ ! -f images.tar ]; then
    curl -O http://vision.stanford.edu/aditya86/ImageNetDogs/images.tar
    tar xvf images.tar
fi

# prepare negative smaples

NEG_SRC_DIR=Images/n02105251-briard
rm negative -fr
cp -r $NEG_SRC_DIR negative
ls negative/* >bg.txt

# prepare positive smaples
POS_SRC_DIR=Images/n02096585-Boston_bull
rm positive -fr
cp -r $POS_SRC_DIR positive

## annotate the dog-face, this command will produce the "info.txt" file,
## the annotating process is tedious, so I prepare a "info.txt" for you,
## with that "info.txt", this command can be omitted.
# opencv_annotation --annotations=info.txt --images=positive

# create samples: 183 samples will be created
opencv_createsamples -info info.txt -vec samples.vec -w 32 -h 32

# train
mkdir -p classifier
opencv_traincascade -data classifier -vec samples.vec -bg bg.txt -numPos 180 -numNeg 180 -h 32 -w 32


# mkdir -p visualisation
# opencv_visualisation --image=./test-visualisation.png --model=./cascade.xml \
#                      --data=./visualisation/
