# Logos

Logos is the namespace for threadpool & job system code. This is not a 'system' as it is underlying core unit of the engine.

Threadpool currently gets initialised at core bringup with a set number of threads and results are processed once per frame
on the main thread. This is subject to change but multithreading is not the highest priority right now.