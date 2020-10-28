import serial
import matplotlib.pyplot as plt

COM_PORT = 'COM6'
BAUD     = 38400
ser      = serial.Serial(COM_PORT, BAUD)

ADC = []

try:
    while True:
        while ser.in_waiting:
            data = ser.readline().decode()
            print(float(data))
            ADC.append(float(data[:-1]))
            if len(ADC) > 50:
                break
        if len(ADC) > 50:
            break
        
except KeyboardInterrupt:
    ser.close()
    print(COM_PORT, 'Closed.')

fig = plt.figure()
plt.plot(ADC)
plt.grid()
plt.show()