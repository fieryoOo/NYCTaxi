import datetime

jday = raw_input("Enter jday of 2013: ")
date0 = datetime.date(2013,1,1)
date = date0 + datetime.timedelta(int(jday)-1)
print date, date.strftime('%A')

#.strptime(datestr,'%Y-%m-%d %H:%M:%S')
