###Hardware Overview


In this folder are the gerber files for the PCB, BOM, and a photo of the assembled board attached to an Arduino UNO. In the photo, we see an ESP32 dev board being the "Device Under Test":

![image](https://github.com/IDMIL/bletests/blob/master/hardware/LatencyRig.jpeg?raw=true)


The "Tester" Arduino UNO must be flashed with the firmware under software/embedded/testerUno, which is based on the code [here](http://isophonics.net/latency-measurements) that contains additional information on the original publication that first described the latensy test procedure (McPherson, A., Jack, R.H., Moro, G. "Are our tools fast enough?", NIME 2016).

The hardware was reproduced based on the description of the features and components from the paper, plus a few minor modifications:

- Additional status LEDs for output
- Exposing additional header pins for other parts of the signal chain including: after the comparator out (to measure digital output trigger response times)
- Adding a second analog comparator channel to perform analog audio in to analog audio out latency measurements.
- Addition of a push button switch (why not).
