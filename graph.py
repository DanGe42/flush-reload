import csv
import matplotlib.pyplot as plt

CUTOFF = 120
colors = ['#FF0059', '#2C00E8', '#00F1FF', '#31D40C', '#BF9A00', '#FFFFFF']

with open('out.txt', 'rb') as outfile:
    probereader = csv.reader(outfile, delimiter=' ')
    rows = [{'slot': int(row[0]), 'addr': int(row[1]), 'time': int(row[2])}
            for row in probereader]
    hits = [row for row in rows if row['time'] < CUTOFF]
    seen = {}
    for hit in hits:
        if hit['slot'] not in seen:
            seen[hit['slot']] = 0.0
        seen[hit['slot']] += 1.0
    print(sum(seen.values()) / len(seen.values()))
    print("Slots >= 2", len([s for s in seen.keys() if seen[s] >= 2]))
    print("Slots == 0", len([s + 1 for s in seen.keys() if (s + 1) not in seen]))
    print("Slots, tot", len(hits))
    #hits = hits[40:140]
    hits = hits[30:-30]
    plt.scatter([hit['slot'] for hit in hits],
                [hit['time'] for hit in hits],
                c=[colors[hit['addr']] for hit in hits])
    #plt.plot([hit['slot'] for hit in hits], [hit['time'] for hit in hits])
    plt.show()
