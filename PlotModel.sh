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

	surface $fbmtmp2 -T$ts -G$fgrd -I$dlon/$dlat $REG

	# plot starts
	if [ $# -ge 3 ]; then
		fcpt=$3
	else
		TXT2CPT_rev $_fin
		fcpt=${_fin}.cpt
	fi

	sms=`echo $dlon $dlat | awk '{print $1/10.0"/"$2/10.0}'`
	fgrds=${fintmp}.grds
	ftobedeleted[idel]=$fgrds; let idel++
	grdsample $fgrd -Q -G$fgrds $REG -I$sms
	grdimage $SCA $REG $fgrds -C$fcpt -O -K >> $psout
	
	pscoast $SCA $REG -A100 -N1/2/80/80/80 -N2/2/80/80/80 -O -K -W3,white >> $psout

	#psxy $fdens -R -J -Sc0.3 -C$fcpt -O -K >> $psout
	psscale -C$fcpt -L -D5./-1./10./0.3h -O -K >> $psout
}


### main ###
if [ $# != 1 ]; then
	echo "Usage: "$0" [in_file]"
	exit
fi

fin=$1
fcpt=/projects/yeti4009/DataIncubator/Project/model_daily.cpt

#lon_range="285.9 286.3"; dlon=0.01
#lat_range="40.58 40.90"; dlat=0.01
lon_range="285.96 286.08"; dlon=0.001
lat_range="40.68 40.84"; dlat=0.001
dlab=a0.05f0.01
SCA=-JN286.02/10
bdis=0.1

# gmt settings
gmtset HEADER_FONT_SIZE 15
gmtset HEADER_OFFSET 0.
gmtset LABEL_FONT_SIZE 12
gmtset ANNOT_FONT_SIZE 10
gmtset BASEMAP_TYPE plain
gmtset PLOT_DEGREE_FORMAT ddd.xxx
gmtset HEADER_FONT_SIZE 15

# plot starts
psout=${fin}.ps
REG=`echo $lon_range $lat_range | awk '{print "-R"$1"/"$2"/"$3"/"$4}'`

pwd | psxy $REG $SCA -B${dlab}:."Demand model (with 0.3 km grid size)":WeSn -X4.5 -Y5.5 -K -P > $psout
PlotMap $fin $bdis $fcpt

# finalize
pwd | psxy -R -J -K -O >> $psout
echo $psout
echo ${ftobedeleted[@]} | xargs rm -f
