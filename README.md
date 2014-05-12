libnettracer
============
A crude method of logging/recording network traffic to/from an application.

The main executable is the nettracer shell script.

Simple usage after compiling: nettracer <traffic_file> <executable> <params>
    Where:  <traffic_file> = File to save captured data
        <executable> = Executable whose network traffic is to be logged
        <params> = command-line parameters for <executable>
