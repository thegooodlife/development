#!/usr/bin/python

'''
This utility compares the hourly ticks generated by oanda
tick data to the data that was exported by metatrader
'''


mt_filename =  "/home/hfcg/development/development/forex/historical-ticks/hourly-data/EURUSD60.csv"
oanda_filename =  "/home/hfcg/development/development/forex/historical-ticks/hourly-data/oanda_based_hourly_data.csv"

def load_data( filename):
         d = {}
         for line in open(filename):
                t = line.split(',')
                d [ (t[0],t[1])  ] = (t[2], t[3], t[4], t[5])
         
         return d

if __name__ == '__main__':
    mt = load_data(mt_filename)
    oanda = load_data(oanda_filename)

    keys = sorted(mt.keys())

    for key in keys:
        day = key[0]+ ' '+ key[1]
        if key not in oanda:
            print day, ' not in oanda'
        else:
            print day, (float(mt[key][0]) - float(oanda[key][0])) * 10000
