


points = []
lines = []

xmin = -500
xmax = 1500
ymin = -800
ymax =  500

for x in range(xmin,xmax, 16*4):
    lines.append("2 {} {}".format(len(points), len(points)+1))

    points.append("{} 0 {}".format(x, ymin))
    points.append("{} 0 {}".format(x, ymax))


for y in range(ymin,ymax, 16*4):
    lines.append("2 {} {}".format(len(points), len(points)+1))

    points.append("{} 0 {}".format(xmin, y))
    points.append("{} 0 {}".format(xmax, y))


#for y in range(ymin, ymax, 50):



print("# vtk DataFile Version 1.0")
print("3D triangulation data")
print("ASCII")
print("")
print("DATASET POLYDATA")
print("POINTS {} float".format(len(points)))

for pt in points:
    print(pt)


print("LINES {} {}".format(len(lines), len(lines)*3))

for line in lines:
    print(line)
