#!/bin/bash
minicom -b 115200 -D /dev/ttyUSB0 -S /home/raidenv/PIDPlot/MINICOMscript.txt -C /home/raidenv/PIDPlot/MINICOMrecord.txt
