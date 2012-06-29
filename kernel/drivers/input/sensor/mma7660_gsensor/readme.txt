(MMA7660FC 3-Axis Orientation/Motion Detection Sensor)

wmt.io.gsensor
Configure G-Sensor for Enable/Disable,X,Y,Z axis mapping and sampling rate. If driver not detect this variable, then G-sensor driver won¡¦t be loaded.
<op>:<freq>:<axis-X>:<X-dir>:<axis-Y>:<Y-dir>:<axis-Z>:<Z-dir>:<avg-count>
<op>:= Operation mode for the g-sensor
0 : MMA7660FC G-Sensor is disabled.
2 : MMA7660FC G-Sensor is enabled.
<freq>:= G-Sensor sampling rate. The valid values are 1,2,4,8,16,32,64,and 128.
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
Ex:
#MMA7660FC G-sensor enabled, using 4 sampling rate, X->-Y, Y->X, Z->-Z, 4 average count
setenv wmt.io.gsensor 2:32:1:-1:0:-1:2:-1:4
