# NodeMCU 1.0 + AD8232
<img src="https://encrypted-tbn1.gstatic.com/shopping?q=tbn:ANd9GcTs0_kHS-qO_5LuMocPUtPVxjyA4mh1a_p5uNvASwy8NkDD-W3LELJcOkQ3iUvVlCHlD2h-LAv10bhHPGxl_tfvPJsS_aCS4iB2TrH6HzgFtTDmentPo5R92fTX6CJBUgjrXrG6OrAV0Q&usqp=CAc" width="500" height="500"> <img src="https://diotronic.com/35349-large_default/nodemcu-v2-lua-based-esp8266.jpg" width="500" height="500">


	
	### The wiring betwen NodeMCU and AD8232: ###
	### NodeMCU <-------> AD8232	###
	###   3.3V out        3.3V in	###
	###   Gnd              Gnd		###
	###    D5              LO+		###
	###    D6              LO-		###
	###  Analog input    Output		###



I've seen a lot of example code for the NodeMCU board that reads the analog output of the AD8232 circuit.

Even so, I couldn't get a clean ECG.
<img src="./img/samp.png" width="800" height="600">

One of the problems was that the green and yellow wire are swapped.

## The correct wiring is:
	
	### RED: 	Rigth Arm; ###
	### GREEN: 	Left Arm;  ###
	### YELLOW: Rigth Leg; ###




#### So here's the code I've developed implementing all the lessons learned in the way.

#### In summary, NodeMCU creates an AP and a web server that displays an interface that receives data 
#### from the NodeMCU via websocket; the main page's JavaScript code receives, processes, and displays 
#### the data, both in the time domain and a fast FFT showing the frequency domain.
<img src="./img/gui.PNG" width="800" height="600">

A button for downloading data to a csv file added.

Ideal wave shape vs output data

<img src="./img/ideal.png" width="400" height="400"> <img src="./img/Captura.PNG" width="400" height="400">
