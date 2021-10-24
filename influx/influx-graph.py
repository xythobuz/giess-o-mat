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
    cumulative = []
    for j in range(len(durations[i])):
        if j == 0:
            cumulative.append(durations[i][j])
        else:
            cumulative.append(cumulative[j - 1] + durations[i][j])

    dates = matplotlib.dates.date2num(times[i])
    ax.plot_date(dates, cumulative, '-', label='id ' + ids[i])

    ax.set_xlabel('Time')
    ax.set_ylabel('Duration')
    ax.set_title('Cumulative Watering Durations')
    ax.legend()

# ---------------------------

smoothing_value = 3.0

fig, ax = plt.subplots()

for i in range(len(ids)):
    cumulative = []
    for j in range(len(durations[i])):
        if j == 0:
            cumulative.append(durations[i][j])
        else:
            cumulative.append(cumulative[j - 1] + durations[i][j])

    cumulative_clean = []
    time_clean = []
    xr = iter(range(len(durations[i]) - 1))
    try:
        for j in xr:
            dx_dt = (times[i][j + 1] - times[i][j])
            dx = dx_dt.seconds / 24 / 60 / 60 + dx_dt.days
            if dx <= smoothing_value:
                #print("combining diff=" + str(dx))
                time_clean.append(times[i][j + 1])
                cumulative_clean.append(cumulative[j + 1])
                next(xr)
            else:
                time_clean.append(times[i][j])
                cumulative_clean.append(cumulative[j])
    except:
        pass

    cumulative_clean_2 = []
    time_clean_2 = []
    xr = iter(range(len(cumulative_clean) - 1))
    try:
        for j in xr:
            dx_dt = (time_clean[j + 1] - time_clean[j])
            dx = dx_dt.seconds / 24 / 60 / 60 + dx_dt.days
            if dx <= smoothing_value:
                #print("combining diff=" + str(dx))
                time_clean_2.append(time_clean[j + 1])
                cumulative_clean_2.append(cumulative_clean[j + 1])
                next(xr)
            else:
                time_clean_2.append(time_clean[j])
                cumulative_clean_2.append(cumulative_clean[j])
    except:
        pass

    dates = matplotlib.dates.date2num(time_clean_2)
    ax.plot_date(dates, cumulative_clean_2, '-', label='id ' + ids[i])

    ax.set_xlabel('Time')
    ax.set_ylabel('Duration')
    ax.set_title('Smoothed Cumulative Watering Durations')
    ax.legend()

# ---------------------------

fig, ax = plt.subplots()

for i in range(len(ids)):
    cumulative = []
    for j in range(len(durations[i])):
        if j == 0:
            cumulative.append(durations[i][j])
        else:
            cumulative.append(cumulative[j - 1] + durations[i][j])

    cumulative_clean = []
    time_clean = []
    xr = iter(range(len(durations[i]) - 1))
    try:
        for j in xr:
            dx_dt = (times[i][j + 1] - times[i][j])
            dx = dx_dt.seconds / 24 / 60 / 60 + dx_dt.days
            if dx <= smoothing_value:
                #print("combining diff=" + str(dx))
                time_clean.append(times[i][j + 1])
                cumulative_clean.append(cumulative[j + 1])
                next(xr)
            else:
                time_clean.append(times[i][j])
                cumulative_clean.append(cumulative[j])
    except:
        pass

    cumulative_clean_2 = []
    time_clean_2 = []
    xr = iter(range(len(cumulative_clean) - 1))
    try:
        for j in xr:
            dx_dt = (time_clean[j + 1] - time_clean[j])
            dx = dx_dt.seconds / 24 / 60 / 60 + dx_dt.days
            if dx <= smoothing_value:
                #print("combining diff=" + str(dx))
                time_clean_2.append(time_clean[j + 1])
                cumulative_clean_2.append(cumulative_clean[j + 1])
                next(xr)
            else:
                time_clean_2.append(time_clean[j])
                cumulative_clean_2.append(cumulative_clean[j])
    except:
        pass

    rate_of_change = []
    for j in range(len(cumulative_clean_2)):
        if j < len(cumulative_clean_2) - 1:
            dy = cumulative_clean_2[j + 1] - cumulative_clean_2[j]
            dx_dt = (time_clean_2[j + 1] - time_clean_2[j])
            dx = dx_dt.seconds / 24 / 60 / 60 + dx_dt.days
            roc = dy / dx
            #print(str(time_clean_2[j]) + " " + str(dy) + " / " + str(dx) + " = " + str(roc))
            rate_of_change.append(roc)

    dates = matplotlib.dates.date2num(time_clean_2)
    ax.plot_date(dates[:-1], rate_of_change, '-', label='id ' + ids[i])

    ax.set_xlabel('Time')
    ax.set_ylabel('Rate of Change')
    ax.set_title('RoC of Cumulative Watering Durations')
    ax.legend()

# ---------------------------

plt.show()
