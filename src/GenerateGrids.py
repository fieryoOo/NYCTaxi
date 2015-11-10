
# 0.3 * 0.3 km grids
lonmin=-74.04; lonmax=-73.92; nlon=35
latmin=40.68;  latmax=40.84;  nlat=60
for lon in [lonmin+i*(lonmax-lonmin)/(nlon-1) for i in range(nlon)]:
	for lat in [latmin+i*(latmax-latmin)/(nlat-1) for i in range(nlat)]:
		print lon, lat, 100, 0.3
