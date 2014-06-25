#!/bin/bash

FILES=../res/images/*

for f in $FILES
do

    echo "Resizing to 32x32: $f"
    convert $f -resize 32x32\! $f

done
