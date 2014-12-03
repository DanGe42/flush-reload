from __future__ import print_function

from collections import namedtuple
import csv
import sys

Hit = namedtuple('Hit', ['slot', 'addr', 'time'])
CUTOFF = 80
SQUARE_ADDR = 0
MODULO_ADDR = 1
MULTIPLY_ADDR = 2


class TimeSlot(object):
    __slots__ = ['square', 'modulo', 'multiply']

    def __init__(self):
        self.square = False
        self.modulo = False
        self.multiply = False


def _read_csv(csvfile):
    with open(csvfile, 'rb') as f:
        probereader = csv.reader(f, delimiter=' ')
        return [Hit(slot=int(row[0]), addr=int(row[1]), time=int(row[2]))
                for row in probereader]

def _hits_to_timeslots(hits):
    num_slots = hits[-1].slot - hits[0].slot + 1
    offset = hits[-1].slot
    slots = [None] * num_slots
    for hit in hits:
        slot = hit.slot - offset
        if slots[slot] is None:
            slots[slot] = TimeSlot()

        time_slot = slots[slot]
        addr = hit.addr
        if addr == SQUARE_ADDR:
            time_slot.square = True
        elif addr == MODULO_ADDR:
            time_slot.modulo = True
        elif addr == MULTIPLY_ADDR:
            time_slot.multiply = True
        else:
            raise ValueError("Invalid addr {:d}".format(addr))

    return slots


def _to_binary(time_slots):
    # We're building a state machine here
    START = 0
    AFTER_SQUARE = 1
    AFTER_SQUARE_MOD = 2
    AFTER_MULTIPLY = 3
    AFTER_SQUARE_MOD_EMPTY = 5

    current_state = START
    output = []
    modulo_count = 0
    for time_slot in time_slots:
        square = time_slot.square if time_slot is not None else None
        modulo = time_slot.modulo if time_slot is not None else None
        multiply = time_slot.multiply if time_slot is not None else None

        if current_state == START:
            modulo_count = 0
            if time_slot is None:
                continue

            # If we see a multiply, then we don't know what we're looking at.
            if multiply:
                output.append('_')

            # Only advance if we see a square
            elif square and not multiply:
                current_state = AFTER_SQUARE

        elif current_state == AFTER_SQUARE:
            # Could end up missing a modulo here
            # If we miss a slot, we probably won't see another square
            if time_slot is None:
                current_state = AFTER_SQUARE_MOD

            # If we see a multiply so soon after square, it might be invalid
            if multiply:
                current_state = START
                output.append('_')

            # Stay if we see another square. Advance if we only see a modulo.
            elif not square and modulo:
                current_state = AFTER_SQUARE_MOD

        elif current_state == AFTER_SQUARE_MOD:
            if time_slot is None:
                current_state = AFTER_SQUARE_MOD_EMPTY

            # If we see both square and multiply, it might be invalid
            elif square and multiply:
                current_state = START
                output.append('_')

            # If we see a square, return to start and output a 0
            elif square:
                current_state = START
                output.append('0')

            elif multiply:
                current_state = AFTER_MULTIPLY

            elif modulo:
                modulo_count += 1

        elif current_state == AFTER_SQUARE_MOD_EMPTY:
            if time_slot is None:
                continue

            # We missed only a modulo
            if multiply:
                current_state = AFTER_MULTIPLY

            elif modulo:
                modulo_count += 1

            elif square:
                current_state = START
                # If we've seen three modulos already, might be worth marking
                # this as unknown
                if modulo_count >= 3:
                    output.append('?')
                else:
                    # Otherwise, we probably didn't really miss anything
                    # important
                    output.append('0')

        elif current_state == AFTER_MULTIPLY:
            # If we see only a modulo, return to start and output a 1
            # Alternatively, a missed slot could be a modulo
            if time_slot is None or (modulo and not square and not multiply):
                current_state = START
                output.append('1')

            # If we see a square so soon, this might be invalid
            elif square:
                current_state = START
                output.append('_')

    return output


def _main(argc, argv):
    if argc != 2:
        print("Usage: {:} file".format(argv[0]), file=sys.stderr)
        return 1

    hits = _read_csv(argv[1])
    hits = [row for row in hits if row.time < CUTOFF]
    print("Number of hits: {:d}".format(len(hits)), file=sys.stderr)
    print("Time slot range: {:d} - {:d}".format(hits[0].slot, hits[-1].slot),
          file=sys.stderr)

    time_slots = _hits_to_timeslots(hits)
    binary = _to_binary(time_slots)
    print(''.join(binary))

    return 0


if __name__ == '__main__':
    import pdb
    import traceback
    try:
        sys.exit(_main(len(sys.argv), sys.argv))
    except SystemExit as e:
        raise e
    except:
        type, value, tb = sys.exc_info()
        traceback.print_exc()
        pdb.post_mortem(tb)
        sys.exit(1)
