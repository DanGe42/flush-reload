import csv
import matplotlib.pyplot as plt

CUTOFF = 120
MAX_CUTOFF = 1000

with open('out.txt', 'rb') as outfile:
    probereader = csv.reader(outfile, delimiter=' ')
    rows = [{'slot': int(row[0]), 'addr': int(row[1]), 'time': int(row[2])} for row in probereader]
    hits = [row for row in rows if row['time'] < CUTOFF]
    print len(hits)
    plt.scatter([hit['slot'] for hit in hits], [hit['time'] for hit in hits], c=[hit['addr'] for hit in hits])
    plt.show()
