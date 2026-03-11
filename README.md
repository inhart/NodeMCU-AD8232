# NodeMCU-AD8232

<img src="./img/gui.png" width="600" height="300">

I've seen a lot of example code for the NodeMCU board that reads the analog output of the AD8232 circuit.

Even so, I couldn't get a clean ECG.

So here's the code I've developed.

In summary, NodeMCU creates an AP and a web server that displays an interface that receives data from the NodeMCU via websocket; the main page's JavaScript code receives, processes, and displays the data, both in the time domain and a fast FFT showing the frequency domain.

A button for downloading data to a csv file added.

Ideal wave shape vs output data

<img src="./img/gui.png" width="300" height="200"> <img src="./img/Captura.png" width="300" height="200">
