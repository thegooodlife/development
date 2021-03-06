#!/usr/bin/python
import urllib2
import json
import sys
from tick import Tick
from tick import show_all_instruments
import time

def start_tick_retriever(instruments):
    i = 0
    t = Tick(instruments)
    while True:
        try:
                t.retrieve()
                time.sleep(1)
                i += 1
                print '------', i
        except Exception as e:
            print 'error: ', e
            time.sleep(1)
        



if __name__ == '__main__':
    if '-c' in  sys.argv:
        index = sys.argv.index('-c') + 1
        if index >= len(sys.argv):
            filename = 'instruments'
        else:
            filename = sys.argv[index]
        instruments = [ strg.rstrip() for strg in open(filename)]
        start_tick_retriever(instruments)
    elif '-i':
       show_all_instruments()
    else:
        pass

            
        



