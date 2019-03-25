import sys
import os
import argparse
import json
import time
import datetime
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

# checks for valid json file
def parseJson(data):
        try:
                return json.load(data)
        except ValueError as err:
                print('ERROR: invalid json: %s' % err)
                return False

# Usage and parser
parser = argparse.ArgumentParser(description = 'Creates a graph from lagscope\'s frequency table and stores the graph into an image file.')
parser.add_argument('-json', default = 'freq_table.json', help = 'freq_table.json')
parser.add_argument('-img', default = 'freq_graph.png', help = 'freq_graph.png')
args = parser.parse_args()

jsonFile = args.json

print('INFO: Graph construction from ' + jsonFile + ' is in progress...')

# load json file into data variable
try:
        with open(jsonFile, "r") as file:
                data = parseJson(file)
except FileNotFoundError:
        data = False
        print('ERROR: ' + jsonFile + ' file does not exist')

if data == False:
        print('INFO: Aborting...')
        sys.exit()

# puts data into three seperate columns: index, latency, frequency
df = pd.DataFrame(data['latencies'])

latCol = df['latency']
freqCol = df['frequency']

# spacing between x-axis ticks
idx = np.arange(min(latCol), max(latCol) + 1, 100)

# creating the graph
timeStamp = time.time()
dateAndTime = datetime.datetime.fromtimestamp(timeStamp).strftime('%m-%d-%Y %H:%M:%S')
plt.figure(figsize = (10.0, 8.0), dpi = 300)
plt.bar(latCol, freqCol)
plt.xlabel('Latency (us)', fontsize = 10)
plt.ylabel('Frequency', fontsize = 10)
plt.xticks(idx, fontsize = 7, rotation = 50)
plt.title('Lagscope Frequency Table Bar Graph (' + jsonFile + ') ' + dateAndTime)

filename = args.img

# check if image file name already exists
if os.path.isfile(filename):
        print('INFO: ' + filename + ' already exists. Overwriting ' + filename)

plt.savefig(filename, bbox_inches = 'tight')

print('INFO: Graph construction completed')
print('INFO: Graph created in: ' + filename + '\n')
