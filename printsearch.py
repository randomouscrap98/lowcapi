import csv
import sys
reader = csv.reader(sys.stdin)
for row in reader:
   print("{:>7} - {}".format(row[6], row[0]))
