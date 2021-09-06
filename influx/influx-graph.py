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
result = client.query('SELECT "id", "duration" FROM "plant";')
#print("Result: {0}".format(result))

data = list(result.get_points())
print("Got " + str(len(data)) + " datapoints")
#print(data)

ids = list(set([ d['id'] for d in data ]))
ids.sort()
#ids = ['1']
print("IDs found: " + str(ids))

values = []
times = []
durations = []
for id in ids:
    values.append([d for d in data if d['id'] == id])
    times.append([datetime.strptime(d['time'], '%Y-%m-%dT%H:%M:%S.%fZ') for d in data if d['id'] == id])
    durations.append([d['duration'] for d in data if d['id'] == id])

s1 = 0.0
s2 = 0.0
c2 = 0
for i in range(len(ids)):
    s = 0.0
    for j in range(len(durations[i])):
        s += durations[i][j]
        s2 += durations[i][j]
        c2 += 1
    avg = s / len(durations[i])
    print("ID=" + ids[i] + " sum=" + str(s) + " len=" + str(len(durations[i])) + " avg=" + str(avg))
    s1 += avg
avg1 = s1 / len(ids)
avg2 = s2 / c2
print("avg1=" + str(avg1) + " avg2=" + str(avg2))

# plot results
plt.ioff()

# ---------------------------

fig, ax = plt.subplots()

for i in range(len(ids)):
    dates = matplotlib.dates.date2num(times[i])
    ax.plot_date(dates, durations[i], '-', label='id ' + ids[i])

    ax.set_xlabel('Time')
    ax.set_ylabel('Duration')
    ax.set_title('Watering Durations')
    ax.legend()

# ---------------------------

fig, ax = plt.subplots()

for i in range(len(ids)):
    values = []
    for j in range(len(times[i])):
        if j == 0:
            continue
        delta = times[i][j] - times[i][j - 1]
        values.append(delta.days)

    ax.plot(range(len(values)), values, '-', label='id ' + ids[i])

    ax.set_xlabel('Watering No.')
    ax.set_ylabel('Time Difference')
    ax.set_title('Time between Waterings')
    ax.legend()

# ---------------------------

fig, ax = plt.subplots()

for i in range(len(ids)):
    ax.plot(range(len(durations[i])), durations[i], '-', label='id ' + ids[i])

    ax.set_xlabel('Watering No.')
    ax.set_ylabel('Duration')
    ax.set_title('Duration per Watering')
    ax.legend()

# ---------------------------

fig, ax = plt.subplots()

for i in range(len(ids)):
    values = []
    s = 0
    for j in range(len(times[i]) - 1):
        t_delta = times[i][j + 1] - times[i][j]
        dur = (durations[i][j] + durations[i][j + 1]) / 2.0
        #dur = durations[i][j + 1]
        #dur = durations[i][j]
        avg_per_sec = dur / t_delta.total_seconds()
        #if i == 2:
        #    print()
        #    print(dur)
        #    print(t_delta.total_seconds())
        #    print(avg_per_sec)
        avg_per_day = avg_per_sec * 60.0 * 60.0 * 24.0
        values.append(avg_per_day)
        s += avg_per_sec
    #print(s / (len(times[i]) - 1))

    ax.plot(range(len(values)), values, '-', label='id ' + ids[i])

    ax.set_xlabel('Watering No.')
    ax.set_ylabel('Duration per Day')
    ax.set_title('Watering Duration per Day')
    ax.legend()

# ---------------------------

plt.show()
