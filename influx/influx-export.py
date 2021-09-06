#!/usr/bin/env python

from influxdb import InfluxDBClient
from datetime import datetime
import xlsxwriter

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

workbook = xlsxwriter.Workbook('InfluxExport.xlsx')
worksheet = workbook.add_worksheet()

bold = workbook.add_format({'bold': 1})
date_format = workbook.add_format({'num_format': 'dd.mm.yyyy hh:mm'})

for i in range(len(ids)):
    worksheet.write_string(0, (i * 3) + 0, "ID", bold)
    worksheet.write_number(0, (i * 3) + 1, int(ids[i]))

    worksheet.write_string(1, (i * 3) + 0, "Time", bold)
    worksheet.set_column((i * 3) + 0, (i * 3) + 0, 20)
    for j in range(len(times[i])):
        worksheet.write_datetime(j + 2, (i * 3) + 0, times[i][j], date_format)

    worksheet.write_string(1, (i * 3) + 1, "Duration", bold)
    for j in range(len(durations[i])):
        worksheet.write_number(j + 2, (i * 3) + 1, durations[i][j])

workbook.close()
