import serial
import time
import csv
import matplotlib
matplotlib.use("tkAgg")
import matplotlib.pyplot as plt
import numpy as np

with open("DAQ.csv", "w"):
    pass

COM_PORT = 'COM6'
BAUD     = 38400
ser      = serial.Serial(COM_PORT, BAUD)
ser.flushInput()

plot_window = 5000
y = np.array(np.zeros([plot_window]))

plt.ion()
fig, ax = plt.subplots()
line, = ax.plot(y)
ax.set_ylim([0, 1023])

while True:
    try:
        ser_bytes = ser.readline()
        try:
            decoded_data = float(ser_bytes[:-1].decode("utf-8"))
            print(decoded_data)
        except:
            continue
        with open("DAQ.csv", "a", newline='') as f:
            writer = csv.writer(f, delimiter=",")
            writer.writerow([decoded_data])
        y = np.append(y, decoded_data)
        y = y[1:plot_window+1]
        line.set_ydata(y)
        ax.relim()
        ax.autoscale_view()
        fig.canvas.draw()
        fig.canvas.flush_events()
    except:
        print("Keyboard Interrupt")
        break