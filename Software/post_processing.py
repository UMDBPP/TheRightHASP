import os
import numpy as np
import matplotlib.pyplot as plt

import serialdownlink

file = open('1.raw', 'ra')

file = np.array(file)

file = file.shape(61, (len(file)*8)/61)
length = np.array(range(0, len(file)))

#Creates arrays for data
for row in file:

	#Raw packet information
	bme_pressure     = bme_pressure.append(row[0])
	bme_temp         = bme_temp.append(row[1])
	bme_humid        = bme_humid.append(row[2])
	ssc_pressure     = ssc_pressure.append(row[3])
	ssc_temp         = ssc_temp.append(row[4])
	bno_temp         = bno_temp.append(row[5])
	mcp_temp         = mcp_temp.append(row[6])
	teensy           = teensy.append(row[7])
	imu_callibration = imu_callibration.append(row[8])
	imu_accel_callib = imu_accel_callib.append(row[9])
	imu_gyro_callib  = imu_gyro_callib.append(row[10])
	imu_mag_callib   = imu_mag_callib.append(row[11])
	accel_x          = accel_x.append(row[12])
	accel_y          = accel_y.append(row[13])
	accel_z          = accel_z.append(row[14])
	gyro_x           = gyro_x.append(row[15])
	gyro_y           = gyro_y.append(row[16])
	gyro_z           = gyro_z.append(row[17])
    mag_x            = mag_x.append(row[18])
    mag_y            = mag_y.append(row[19])
    mag_z            = mag_z.append(row[20])

    #Calibrated Accelerometer
    x_accel          = row[12] - row[9]
    y_accel          = row[13] - row[9]
    z_accel          = row[14] - row[9]
 	
 	#Calibrated Gyroscope
 	x_gyro           = row[15] - row[10]
 	y_gyro           = row[16] - row[10]
 	z_gyro           = row[17] - row[10]

 	#Calibrated Magnetometer
 	x_mag            = row[18] - row[11]
 	y_mag            = row[19] - row[11]
 	z_mag            = row[20] - row[11]

#Creates plots
n=1
plt.figure(n)
plt.subplot(221)
plt.scatter(length, bme_pressure)
plt.xlabel('Time')
plt.ylabel('BME Pressure')
plt.title('BME Pressure vs. Time')
plt.grid(True)

plt.subplot(222)
plt.scatter(length, ssc_pressure)
plt.xlabel('Time')
plt.ylabel('SSC Pressure')
plt.title('SSC Pressure vs. Time')
plt.grid(True)

plt.subplot(223)
plt.scatter(length, bme_humid)
plt.xlabel('Time')
plt.ylabel('BME Humidity')
plt.title('BME Humidity vs. Time')
plt.grid(True)
n=n+1

plt.figure(n)
plt.subplot(221)
plt.scatter(length, bme_temp)
plt.xlabel('Time')
plt.ylabel('BME Temperature')
plt.title('BME Temperature vs. Time')
plt.grid(True)

plt.subplot(222)
plt.scatter(length, ssc_temp)
plt.xlabel('Time')
plt.ylabel('SSC Temperature')
plt.title('SSC Temperature vs. Time')
plt.grid(True)

plt.subplot(223)
plt.scatter(length, bno_temp)
plt.xlabel('Time')
plt.ylabel('BNO Temperature')
plt.title('BNO Temperature vs. Time')
plt.grid(True)

plt.subplot(224)
plt.scatter(length, mcp_temp)
plt.xlabel('Time')
plt.ylabel('MCP Temperature')
plt.title('MCP Temperature vs. Time')
plt.grid(True)
n=n+1

plt.figure(n)
plt.subplot(221)
plt.scatter(length, x_accel)
plt.xlabel('Time')
plt.ylabel('X Acceleration')
plt.title('X Acceleration vs. Time')
plt.grid(True)

plt.subplot(222)
plt.scatter(length, y_accel)
plt.xlabel('Time')
plt.ylabel('Y Acceleration')
plt.title('Y Acceleration vs. Time')
plt.grid(True)

plt.subplot(223)
plt.scatter(length, z_accel)
plt.xlabel('Time')
plt.ylabel('Z Acceleration')
plt.title('Z Acceleration vs. Time')
plt.grid(True)
n=n+1

plt.figure(n)
plt.subplot(221)
plt.scatter(length, x_gyro)
plt.xlabel('Time')
plt.ylabel('X Gyroscope')
plt.title('X Gyroscope vs. Time')
plt.grid(True)

plt.subplot(222)
plt.scatter(length, y_gyro)
plt.xlabel('Time')
plt.ylabel('Y Gyroscope')
plt.title('Y Gyroscope vs. Time')
plt.grid(True)

plt.subplot(223)
plt.scatter(length, z_gyro)
plt.xlabel('Time')
plt.ylabel('Z Gyroscope')
plt.title('Z Gyroscope vs. Time')
plt.grid(True)
n=n+1

plt.figure(n)
plt.subplot(221)
plt.scatter(length, x_mag)
plt.xlabel('Time')
plt.ylabel('X Magnetometer')
plt.title('X Magnetometer vs. Time')
plt.grid(True)

plt.subplot(222)
plt.scatter(length, y_mag)
plt.xlabel('Time')
plt.ylabel('Y Magnetometer')
plt.title('Y Magnetometer vs. Time')
plt.grid(True)

plt.subplot(223)
plt.scatter(length, z_mag)
plt.xlabel('Time')
plt.ylabel('Z Magnetometer')
plt.title('Z Magnetometer vs. Time')
plt.grid(True)

plt.show()
file.close()
