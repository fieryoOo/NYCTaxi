#!/bin/bash

PlotMap() {
	local _fin=$1
	local _bdis=$2
	# compute surface
	ts=0.2

	fintmp=${_fin}
	fbmtmp1=${fintmp}.bm1
	idel=0
	ftobedeleted[idel]=$fbmtmp1; let idel++
	fbmtmp2=${fintmp}.bm2
	ftobedeleted[idel]=$fbmtmp2; let idel++

	blockmean $fintmp $REG -I$_bdis'km' > $fbmtmp1
	blockmean $fbmtmp1 $REG -F -I$_bdis'km' > $fbmtmp2

	fgrd=${fintmp}.grd
	ftobedeleted[idel]=$fgrd; let idel++

	surface $fbmtmp2 -T$ts -G$fgrd -I$res $REG

	# plot starts
	#TXT2CPT_rev $_fin
	#fcpt=${_fin}.cpt
	fcpt=$3

	sms=`echo $res | awk '{print $1/10.0}'`
	fgrds=${fintmp}.grds
	ftobedeleted[idel]=$fgrds; let idel++
	grdsample $fgrd -Q -G$fgrds $REG -I$sms
	grdimage $SCA $REG $fgrds -C$fcpt -O -K >> $psout
	
	pscoast $SCA $REG -A100 -N1/2/80/80/80 -N2/2/80/80/80 -O -K -W3,white >> $psout

	#psxy $fdens -R -J -Sc0.3 -C$fcpt -O -K >> $psout
	psscale -C$fcpt -L -D5./-1./10./0.3h -O -K >> $psout
}


### main ###
lon_range="285.9 286.3"; dlon=0.01
lat_range="40.58 40.90"; dlat=0.01
hdis=1.0
dlab=a0.1f0.02
SCA=-JN286.1/10
res=0.01
bdis1=0.5; bdis2=1.0
#lon_range="270. 308."; dlon=1.0
#lat_range="25.0 55.0"; dlat=1.0
#hdis=100.
#dlab=a10f2
#SCA=-JN286.1/10
#res=1.
#bdis1=20; bdis2=20

fdata=loc_tripdis.txt
fdensdis=`echo $fdata | sed s/'.txt'/'_density.txt'/`
#./MapPointDensity $fdata ${lon_range} ${dlon} ${lat_range} ${dlat} ${hdis} $fdensdis
echo $fdensdis" produced"

# gmt settings
gmtset HEADER_FONT_SIZE 15
gmtset HEADER_OFFSET 0.
gmtset LABEL_FONT_SIZE 12
gmtset ANNOT_FONT_SIZE 10
gmtset BASEMAP_TYPE plain
gmtset PLOT_DEGREE_FORMAT ddd.xxx
gmtset HEADER_FONT_SIZE 15

# plot starts
psout=${fdensdis}.ps
REG=`echo $lon_range $lat_range | awk '{print "-R"$1"/"$2"/"$3"/"$4}'`

# trip density map
fdens=${fdensdis}.dens
ftobedeleted[idel]=$fdens; let idel++
awk '{print $1,$2,$3}' $fdensdis > $fdens
pwd | psxy $REG $SCA -B${dlab}:."Trip Number (no. per km^2 area)":WeSn -X4.5 -Y5.5 -K > $psout
PlotMap $fdens $bdis1 Density.cpt

# average trip distance
fdis=${fdensdis}.dis
ftobedeleted[idel]=$fdis; let idel++
awk '$4>0{print $1,$2,$4}' $fdensdis > $fdis
pwd | psxy $REG $SCA -B${dlab}:."Average Trip Distance (km)":WeSn -X12 -O -K >> $psout
PlotMap $fdis $bdis2 Distance.cpt

# finalize
pwd | psxy -R -J -K -O >> $psout
echo $psout
echo ${ftobedeleted[@]} | xargs rm -f
