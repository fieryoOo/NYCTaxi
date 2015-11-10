
def loadDF(fname):
	#5 pickup_datetime
	#6 dropoff_datetime
	#7 passenger_count
	#8 trip_time_in_secs
	#9 trip_distance
	#10 pickup_longitude
	#11 pickup_latitude
	#12 dropoff_longitude
	#13 dropoff_latitude
	print fname
	#datafr = pandas.read_csv(fname, nrows=1000, usecols=[5,9,10,11], skipinitialspace=True)
	datafr = pandas.read_csv(fname, usecols=[5,9,10,11], skipinitialspace=True)
	print datafr.head()
	return datafr


# Figure 1.
def extractLocDis(datafr):
	# select columns
	df1 = datafr.loc[(datafr['pickup_longitude']<-50.) & (datafr['pickup_longitude']>-90.) & (datafr['pickup_latitude']>20) & (datafr['pickup_latitude']<60)].dropna().loc[:,['pickup_longitude','pickup_latitude','trip_distance','pickup_datetime']]
	dfdate = df1.loc[:,['pickup_datetime']]
	df1 = df1.loc[:,['pickup_longitude','pickup_latitude','trip_distance']]
	# compute year, month, day, dow, hour, jday
	dt0 = datetime.datetime(2013,1,1)
	v = zip(*[[dt.year,dt.month,dt.day,dt.weekday(),dt.hour,dt.minute,(dt-dt0).days] for dt in [datetime.datetime.strptime(datestr,'%Y-%m-%d %H:%M:%S') for datestr in dfdate['pickup_datetime']]])
	# append new lists to the dataframe
	df1.loc[:,'year'] = pandas.Series(v[0], index=df1.index)
	df1.loc[:,'month'] = pandas.Series(v[1], index=df1.index)
	df1.loc[:,'day'] = pandas.Series(v[2], index=df1.index)
	df1.loc[:,'dow'] = pandas.Series(v[3], index=df1.index) # day_of_week: Monday is 0, Sunday is 6
	df1.loc[:,'hour'] = pandas.Series(v[4], index=df1.index)
	df1.loc[:,'minute'] = pandas.Series(v[5], index=df1.index)
	df1.loc[:,'jday'] = pandas.Series(v[6], index=df1.index)
	# sort by jday
	df1.sort(columns=['jday'],inplace=True)
	# set index to jday (without dropping it)
	df1.set_index(keys=['jday'],drop=False,inplace=True)
	# loop over jdays
	for jday in df1['jday'].unique():
		outname = "./data_by_jday/loc_tripdis_date_"+str(jday)+".txt"
		# output
		df1.loc[df1.jday==jday].to_csv(outname, sep=' ', header=False, index=False)


# Figure 2.
def writeList(L,fname):
	import csv
	with open(fname,"wb") as fout:
		csv.writer(fout,delimiter=' ').writerows(L)

def extractNByDates(datafr):
	df2 = datafr.loc[:,['pickup_datetime']].dropna()
	hour_L = []; dow_L = []; day_L = []; month_L = []
	for datestr in df2['pickup_datetime']:
		dt = datetime.datetime.strptime(datestr,'%Y-%m-%d %H:%M:%S')
		# day_of_week: Monday is 0, Sunday is 6
		hour_L.append(dt.hour)
		dow_L.append(dt.weekday())
		day_L.append(dt.day)
		month_L.append(dt.month)
	hourC_L = Counter(hour_L).most_common()
	dowC_L = Counter(dow_L).most_common()
	dayC_L = Counter(day_L).most_common()
	monthC_L = Counter(month_L).most_common()
	writeList(hourC_L, 'hourC.txt')
	writeList(dowC_L, 'dowC.txt')
	writeList(dayC_L, 'dayC.txt')
	#writeList(monthC_L, 'monthC.txt')

### main ###
 
import pandas
import numpy
from collections import Counter
import datetime

# load
for fno in range(3,13):
	fname = "./data/trip_data_"+str(fno)+".csv"
	datafr = loadDF( fname )
	Ntotal = len(datafr.index)

	# Figure #1
	extractLocDis(datafr)

	# Figure #2
	#extractNByDates(datafr)

