#!/usr/bin/python

class Tick:
    def __init__(self, line):
        line = line.rstrip()
        tokens = line.split(',')
        t = tokens[0]
        tt = t.split(' ')
        self.day = tt[0]
        self.time = tt[1]
        self.bid = float(tokens[1])
        self.ask = float(tokens[2])

    def __repr__(self):
        return '{0} {1} {2} {3}'.format(self.day,self.time, self.bid, self.ask)


def get_tick(filename):
    for line in open(filename):
        yield Tick(line)


if __name__ == '__main__':
    
    for tick in get_tick('./historical-ticks/EUR_USD.csv'):
        print tick



