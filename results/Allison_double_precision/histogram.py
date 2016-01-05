lmax = []
rmse = []

less1 = []
less10 = []
less100 = []
greater100 = []

i = 0
with open('result.3d', 'r') as f:
  for line in f:
    data = line.split()
    if i%3 == 1:
      lmax.append(data[6])
    elif i%3 == 2:
      rmse.append(data[6])
    i += 1

for s in rmse:
  num = float(s)
  if num < 1:
    less1.append( num )
  elif num < 10:
    less10.append( num )
  elif num < 100:
    less100.append( num )
  else:
    greater100.append( num )


print "less1: " + str(len(less1)) + ", percentage: " + str( len(less1) / 157.0 )
print "less10: " + str(len(less10)) + ", percentage: " + str( len(less10) / 157.0 )
print "less100: " + str(len(less100)) + ", percentage: " + str( len(less100) / 157.0 )
print "greater100: " + str(len(greater100)) + ", percentage: " + str( len(greater100) / 157.0 )

