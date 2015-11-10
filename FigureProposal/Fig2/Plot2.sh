#!/bin/bash

PlotCurve() {
	local _fin=$1
	local _REG=$2
	local _XS=$3
	local _YS=$4
	local _style=$5
	local _labels=$6
	local _title=$7

echo $_labels
	psbasemap $_REG -JX12/3 -B${_labels}:."$_title":WeSn -X${_XS} -Y${_YS} -O -K >> $psout
	sort -gk1 $_fin | psxy -R -J -A $_style -O -K >> $psout
}

### main ###

# gmt settings
gmtset HEADER_FONT_SIZE 15
gmtset HEADER_OFFSET -0.3
gmtset LABEL_OFFSET -0.1
gmtset LABEL_FONT_SIZE 12
gmtset ANNOT_FONT_SIZE 10
gmtset BASEMAP_TYPE plain
gmtset PLOT_DEGREE_FORMAT ddd.xxx
gmtset HEADER_FONT_SIZE 15

# start plotting
psout=DateTrends.ps

pwd | psxy -Rg -JX1 -K -P > $psout

PlotCurve ./hourC.txt -R0/24/0/1000000 2.5 17 "-W8,steelblue" 'a4f1:Time_of_The_Day_(#hour):/a500000f100000:No._Trips:'
PlotCurve ./dowC.txt -R-0.5/6.5/1000000/3000000 0 -5 "-Sc0.3 -Gred" 'a1f1:Day_of_The_Week_(0=Monday):/a1000000f200000:No._Trips:'
PlotCurve ./dayC.txt -R0.5/31.5/300000/600000 0 -5 "-W8,forestgreen" 'a5f1:Day_of_The_Month:/a100000f20000:No._Trips:'

pwd | psxy -R -J -O >> $psout
echo $psout
