The purpose of these files is to contain all the actual simulation code.

To try and keep things relatively easy to look at, the simulation has been split into parts:

 - General components needed in other simulations
 - CTEM
 - CBED
 - STEM
 - A "Worker" that ties all of these back together
 
To make general programming easier, the Worker class inherits the STEM class, that inherits the CBED class, that inherits the CTEM class, that inherits the General class.

This might not make particular sense, but it does allow for one object to contain all the needed components. This also means that components (buffers, kerenels) can be reused, even if the simulation type is switched.

The planned usage of these classes is generally then to just use the SimulationWorker class and call the desired inherited functions.