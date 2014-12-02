from __future__ import print_function

from collections import namedtuple
import csv
import matplotlib.pyplot as plt

Hit = namedtuple('Hit', ['slot', 'addr', 'time'])

CUTOFF = 120
square = {'label': 'Square', 'marker': 'o', 'color': '#FF0059', 'addrs': [0, 1, 2]}
red = {'label': 'Reduce', 'marker': 'o', 'color': '#2C00E8', 'addrs': [3]}
mult = {'label': 'Multiply', 'marker': '^', 'color': '#00F1FF', 'addrs': [4, 5]}
hit_types = [square, red, mult]

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
    print("Slots, tot {:d}".format(len(seen)))
    hits = hits[30:-30]

    for hit_type in hit_types:
        hits_of_type = [hit for hit in hits if hit.addr in hit_type['addrs']]
        slot_to_hits = {}
        for hit in hits_of_type:
            slot_to_hits[hit.slot] = hit
        print("{:s} {:d}".format(hit_type['label'], len(slot_to_hits.keys())))

    fig = plt.figure()
    axis = fig.add_subplot(111)
    plots = []
    # Somewhat of a hack right now, but we assume that there are 5 addresses
    # range instead of xrange for forwards compatibility
    # Also, have to do each address separately because scatter() does not accept
    # a list of markers >_>
    for hit_type in hit_types:
        addr_hits = [hit for hit in hits if hit.addr in hit_type['addrs']]
        slot_to_hits = {}
        for hit in addr_hits:
            slot_to_hits[hit.slot] = hit
        addr_hits = slot_to_hits.values()
        plot = axis.scatter(
            [hit.slot for hit in addr_hits],
            [hit.time for hit in addr_hits],
            c=hit_type['color'],
            marker=hit_type['marker'],
        )
        plots.append(plot)
    # plt.plot([hit.slot for hit in hits], [hit.time for hit in hits])
    plt.legend(tuple(plots),
               tuple([hit_type['label'] for hit_type in hit_types]),
               scatterpoints=1,
               loc='upper right',
               ncol=1)
    plt.show()
