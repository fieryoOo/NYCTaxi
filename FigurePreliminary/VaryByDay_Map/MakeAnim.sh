convert -quality 100 -delay 20 -loop 0 -scale 100% `ls ../../run_by_day/Models/loc_tripdis_date_*.txt_model.ps | awk -F'date_' '{print $0,$2}' | sort -g -k2 | awk '{print $1}'` animation.mp4
