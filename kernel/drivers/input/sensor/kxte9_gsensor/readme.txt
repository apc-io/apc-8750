The back of WM3445 MID
 ______________
| +z           |
|   \       @  |
|    \         |
|      ---- +y |
|     |        |
|     |        |
|     +x       |
|______________|
(KXTE9-2050 Accelerometer)

wmt.io.gsensor
Configure G-Sensor for Enable/Disable,X,Y,Z axis mapping and sampling rate. If driver not detect this variable, then G-sensor driver won¡¦t be loaded.
<op>:<freq>:<axis-X>:<X-dir>:<axis-Y>:<Y-dir>:<axis-Z>:<Z-dir>:<avg-count>:<8bit>
<op>:= Operation mode for the g-sensor
0 : KXTE9-2050 G-Sensor is disabled.
1 : KXTE9-2050 G-Sensor is enabled.
<freq>:= G-Sensor sampling rate. The valid values are 1,3,10,and 40.
<axis-X>:= G-Sensor axis-x will map to which axis of device.
0 : X
1 : Y
2 : Z
<X-dir>:= If G-Sensor axis-x direction is the same as device.
1 : Positive direction
-1 : Negative direction
<axis-Y>:= G-Sensor axis-y will map to which axis of device.
0 : X
1 : Y
2 : Z
<Y-dir>:= If G-Sensor axis-y direction is the same as device.
1 : Positive direction
-1 : Negative direction
<axis-Z>:= G-Sensor axis-z will map to which axis of device.
0 : X
1 : Y
2 : Z
<Z-dir>:= If G-Sensor axis-z direction is the same as device.
1 : Positive direction
-1 : Negative direction
<avg-count>:= For every new/current axes values stored, add the newly stored axes with previously (avg-count-1) stored axes values and do an average
<8bit>:= Using acceleration data as 8bit
0 : 6bit
1 : 8bit
Ex:
#KXTE9-2050 G-sensor enabled, using 40 sampling rate, X->-Y, Y->X, Z->-Z, 4 average count, using 8bit
setenv wmt.io.gsensor 1:40:1:-1:0:-1:2:-1:4:1
