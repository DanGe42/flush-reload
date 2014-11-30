from __future__ import print_function

from collections import namedtuple
import csv
import matplotlib.pyplot as plt

Hit = namedtuple('Hit', ['slot', 'addr', 'time'])

CUTOFF = 160
colors = ['#FF0059', '#2C00E8', '#00F1FF', '#31D40C', '#BF9A00', '#FFFFFF']
markers = ['x', 'x', 'o', '^', '^']

with open('out.txt', 'rb') as outfile:
    probereader = csv.reader(outfile, delimiter=' ')
    rows = [Hit(slot=int(row[0]), addr=int(row[1]), time=int(row[2]))
            for row in probereader]
    hits = [row for row in rows if row.time < CUTOFF]
    seen = {}
    for hit in hits:
        if hit.slot not in seen:
            seen[hit.slot] = 0.0
        seen[hit.slot] += 1.0
    print(sum(seen.values()) / len(seen.values()))
    print("Slots >= 2 {:d}".format(len([s for s in seen.keys() if seen[s] >= 2])))
    print("Slots == 0 {:d}".format(len([s + 1 for s in seen.keys() if (s + 1) not in seen])))
    print("Slots, tot {:d}".format(len(hits)))
    hits = hits[30:-30]

    fig = plt.figure()
    axis = fig.add_subplot(111)
    # Somewhat of a hack right now, but we assume that there are 5 addresses
    # range instead of xrange for forwards compatibility
    # Also, have to do each address separately because scatter() does not accept
    # a list of markers >_>
    for addr in range(5):
        addr_hits = [hit for hit in hits if hit.addr == addr]
        axis.scatter(
            [hit.slot for hit in addr_hits],
            [hit.time for hit in addr_hits],
            c=colors[addr],
            marker=markers[addr],
        )
    # plt.plot([hit.slot for hit in hits], [hit.time for hit in hits])
    plt.show()
