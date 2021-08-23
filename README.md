# SeniorProjectFinal
This is a yearlong senior project that implements IoT solutions for Smart Manufacturing. 

A video demonstrating the capabilities and working principles can be found here: https://www.youtube.com/watch?v=HEh5HelEZMU (Starts at 1:09:10, subtitles are in English)

The main component is the indoor position finding algorithm, implemented primarily using OpenCV, that detects multiple objects (via our own neural network, RCNN, YOLO, DeepSort...) and reports their 3D position via stereo camera triangulation, yet there are many more features including baret detection, facial recognition, fall detection, noise-level analysis, predictive and preventive maintenance of machinary via sound recordings etc.

Some of the components such as predictive and preventive maintenance are not in this repository. The Android application that glues components such as facial recognition picture taking and sending, fall detection, noise-level analysis, tool location access over the server and others is in a private repository. Please contact me if you would like further access into the components that are not provided here.
