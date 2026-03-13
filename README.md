# NodeMCU 1.0 + AD8232
<p align=Centert><img src="https://encrypted-tbn1.gstatic.com/shopping?q=tbn:ANd9GcTs0_kHS-qO_5LuMocPUtPVxjyA4mh1a_p5uNvASwy8NkDD-W3LELJcOkQ3iUvVlCHlD2h-LAv10bhHPGxl_tfvPJsS_aCS4iB2TrH6HzgFtTDmentPo5R92fTX6CJBUgjrXrG6OrAV0Q&usqp=CAc" width="300" height="300"> <img src="https://diotronic.com/35349-large_default/nodemcu-v2-lua-based-esp8266.jpg" width="300" height="300">

## Wiring NodeMCU and AD8232:

<p align=center>		NodeMCU	<------->		AD8232

<p align=center>		 3.3V	<------->		3.3V

<p align=center>		 Gnd	<------->		Gnd

<p align=center>		 D5		<------->		LO+

<p align=center>		 D6		<------->		LO-

<p align=center>		 A0		<------->		Output

## The correct wiring for AD8232 electrodes is:

One of the problems was that the green and yellow wires are swapped.

<p align=center>  RED: 	Rigth Arm
<p align=center>  GREEN: 	Left Arm
<p align=center>  YELLOW: Rigth Leg

## Notes:

I've seen a lot of example code for the NodeMCU board that reads the analog output of the AD8232 circuit.

Even so, I couldn't get a clean ECG.
<p align=center> <img src="./img/samp.png" width="800" height="600">

So here's the code I've developed implementing all the lessons learned in the way.

In summary, NodeMCU creates an AP and a web server that displays an interface that receives data 
from the NodeMCU via websocket; the main page's JavaScript code receives, processes, and displays 
the data, both in the time domain and a fast FFT showing the frequency domain.
<p align=center> <img src="./img/gui.PNG" width="800" height="600">

A button for downloading data to a csv file added.

## Ideal wave shape vs Output data:

<p align=center> <img src="./img/ideal.png" width="400" height="400"> <img src="./img/Captura.PNG" width="400" height="400">
