#!/bin/bash

fpred1="A_at_285.996_40.741.txt_pred_deg2" # Greenwich Village
fCV1="A_at_285.996_40.741.txt_CV.txt"
ffrec1="A_at_285.996_40.741_freq.txt"
fpred2="A_at_286.041_40.718.txt_pred_deg2" # Music Hall of Williamsburg
fCV2="A_at_286.041_40.718.txt_CV.txt"
ffrec2="A_at_286.041_40.718_freq.txt"

# output ps file
psout=SinglePointVariations.ps; rm -f $psout

# gmt settings
gmtset HEADER_FONT_SIZE 15
gmtset HEADER_OFFSET -0.05
gmtset LABEL_FONT_SIZE 12
gmtset LABEL_OFFSET -0.05
gmtset ANNOT_FONT_SIZE 10
gmtset BASEMAP_TYPE plain
gmtset PLOT_DEGREE_FORMAT ddd.xxx
gmtset HEADER_FONT_SIZE 15

# plot cross-validation results
REG=-R0./4./-1/1
SCA=-JX8.5/5.3
labels=a1f1:"Polynomial Degree":/a0.5f0.1:"R^2 Score"::."Cross Validation Results":
pwd | psbasemap -B"${labels}"WeSn $REG $SCA -K -X6 -Y12 > $psout
awk '{print $1,$3}' $fCV1 | psxy -R -J -A -W3,brown -O -K >> $psout
awk '{print $1,$3}' $fCV1 | psxy -R -J -A -Sc0.3 -Glightred -O -K >> $psout
awk '{print $1,$3}' $fCV2 | psxy -R -J -A -W3,blue -O -K >> $psout
awk '{print $1,$3}' $fCV2 | psxy -R -J -A -Sc0.3 -Gsteelblue -O -K >> $psout

# plot freq component
REG=-R0.02/0.4/0/20000
SCA=-JX8.5/5.3
labels=a0.1f0.02:"Frequency (1/#days)":/a5000f1000:"Spectral Amplitude"::."Frequency Contents of Model Amp":
pwd | psbasemap -B"${labels}"WeSn $REG $SCA -O -K -X11.5 >> $psout
awk '{print $1,$2}' $ffrec1 | psxy -R -J -A -W3,brown -O -K >> $psout
awk '{print $1,$2}' $ffrec2 | psxy -R -J -A -W3,blue -O -K >> $psout

# plot fpred1
REG=-R0/365/30/1000
SCA=-JX20/7.3l
labels=a100f20:"Day of the Year":/a500f100:"Demand Model Amplitude"::."Model Variation With Time (2013)":
pwd | psbasemap -B"${labels}"WeSn $REG $SCA -O -K -X-11.5 -Y-9.8 >> $psout
awk '{print $4,$2}' $fpred1 | psxy -R -J -A -W3,lightred -O -K >> $psout
awk '{print $4,$2}' $fpred1 | psxy -R -J -A -Sc0.1 -Glightred -O -K >> $psout
awk '{print $4,$1}' $fpred1 | psxy -R -J -A -W3,brown -O -K >> $psout
awk '{print $4,$2}' $fpred2 | psxy -R -J -A -W3,steelblue -O -K >> $psout
awk '{print $4,$2}' $fpred2 | psxy -R -J -A -Sc0.1 -Glightblue -O -K >> $psout
awk '{print $4,$1}' $fpred2 | psxy -R -J -A -W3,blue -O -K >> $psout

# legends
gmtset ANNOT_FONT_SIZE 12
pslegend -R -J -D258/900/6.3/1.8/LB -F -Gwhite -O -K >> $psout <<- EOF
	G 0.2
	S 0.6 - 1.0 - 3,brown 1.5 Greenwich Village
	G 0.2
	S 0.6 - 1.0 - 3,blue 1.5 Williamsburg Music Hall
EOF

# finalize
pwd | psxy -R -J -O >> $psout
echo $psout
