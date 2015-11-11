
# load data from file on given columns
def LoadFile( fname, coln_x, dataX, coln_y, dataY, coln_z_L, dataZ ):
	#global nbadline
	global nbadline
	with open(fname) as fin:
		for line in fin:
			data = [ float(f) for f in line.split() ]
			if True in [ x!=x for x in data ]:
				nbadline += 1
				continue
			jday = data[coln_x]
			date = date0 + datetime.timedelta(jday-1)
			#dataX.append([date.month, date.weekday(), jday])
			dataX.append( [date.month, jday] + [ int(i == date.weekday()) for i in range(6) ] )
			dataY.append(data[coln_y])
			#dataZ.append( [ data[index] for index in coln_z_L ] )


def ndays(y, m, d):
	return (datetime.date(y,m,d) - date0).days

### main ###
import numpy
import datetime

date0 = datetime.date(2013,1,1)

# load data
dataX = []; dataY = []; dataZ = []
#fname="A_at_286.009_40.7504.txt"
fname="A_at_286.041_40.718.txt"
fname="A_at_285.996_40.741.txt"
coln_z_L = [2] # complementary info
nbadline = 0
LoadFile(fname, 0, dataX, 1, dataY, coln_z_L, dataZ)
print len(dataY), " daily records loaded (with ", nbadline, " bad lines discarded)..."
print dataX[0:10]
print dataY[0:10]

# try linear regression on day_num-demands
from sklearn import linear_model
from sklearn import preprocessing
from sklearn import cross_validation
from sklearn import pipeline

scores_L = [ [0, 0.0, 0.0] ]
for degree in range(1,7):
	model = pipeline.Pipeline([ ('pf', preprocessing.PolynomialFeatures(degree)), ('lr', linear_model.LinearRegression(fit_intercept=False)) ])
	model = model.fit(dataX, dataY)
	scores = cross_validation.cross_val_score( model, dataX, dataY, cv=5)
	#print model.named_steps['lr'].intercept_
	#print model.named_steps['lr'].coef_
	mscore = model.score(dataX, dataY)
	print( "score_train = %0.2f, score_cv = %0.2f sstd_cv = %0.2f)" % (mscore, scores.mean(), scores.std()) )
	scores_L.append( [degree, mscore, scores.mean()] )

	dataY_pred = model.predict(dataX)
	with open(fname+"_pred_deg"+str(degree),'w') as fout:
		for P,Y,X in zip(dataY_pred,dataY,dataX):
			fout.write( "%s %s %s %s %s %s %s %s %s %s\n" % tuple([P,Y] + X) )

import csv
with open(fname+"_CV.txt", "wb") as fout:
	csv.writer(fout,delimiter=' ').writerows(scores_L)
	

import os
os._exit(1)

#poly = preprocessing.PolynomialFeatures(degree)
#dataXp = poly.fit_transform(dataX)
#clf = linear_model.LinearRegression(fit_intercept=False)
#clf.fit(dataXp, dataY)
#print clf.intercept_, clf.coef_
#print clf.score(dataXp, dataY)
