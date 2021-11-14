#!/usr/bin/env python

from influxdb import InfluxDBClient
import matplotlib.pyplot as plt
import matplotlib
from datetime import datetime

# config
host = '10.23.42.14'
port = 8086
user = 'root'
password = 'root'
dbname = 'giessomat'

# data
client = InfluxDBClient(host, port, user, password, dbname)
print("Querying DB " + dbname + " on " + host)
result = client.query('SELECT "duration" FROM "calibrated_filling";')
#print("Result: {0}".format(result))

data = list(result.get_points())
print("Got " + str(len(data)) + " datapoints")
#print(data)

times = ([datetime.strptime(d['time'], '%Y-%m-%dT%H:%M:%S.%fZ') for d in data])
durations = ([d['duration'] for d in data])

max_t = max(durations)
min_t = min(durations)

simple_average = min_t + ((max_t - min_t) / 2)
average = 0
for d in durations:
    average += d
average /= len(durations)

sorted_durations = sorted(durations)
median = sorted_durations[int(len(sorted_durations) / 2)]

print("Min. Filling Time: " + str(min_t))
print("Max. Filling Time: " + str(max_t))
print("Average Filling Time:")
print("  Simple = " + str(simple_average))
print("  Average = " + str(average))
print("  Median = " + str(median))

# plot results
plt.ioff()

# ---------------------------

fig, ax = plt.subplots()

dates = matplotlib.dates.date2num(times)
ax.plot_date(dates, durations, '-', label='filling')

ax.set_xlabel('Time')
ax.set_ylabel('Duration')
ax.set_title('Filling Durations')
ax.legend()

# ---------------------------

plt.show()
