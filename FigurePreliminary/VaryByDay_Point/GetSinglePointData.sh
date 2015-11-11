#/bin/bash

### main ###
dirmod=../../run_by_day/Models
if [ $# != 2 ]; then
	echo "Usage: "$0" [lon] [lat]"
	exit
fi

lon=$1
lat=$2

fout=A_at_${lon}_${lat}.txt; rm -f $fout
for ((day=0; day<365; day++)); do
	fmod=${dirmod}/loc_tripdis_date_${day}.txt_model
	if [ ! -e $fmod ]; then continue; fi
	A=`awk -v clon=$lon -v clat=$lat 'BEGIN{mis=999; A=-1}{miscur=($1-clon)**2+($2-clat)**2; if(mis>miscur){mis=miscur; A=$3}}END{print A}' $fmod`
	trueday=`echo $day | awk '{print $1+1}'`
	echo $trueday $A >> $fout
done
echo $fout

