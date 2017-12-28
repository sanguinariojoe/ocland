ocland
======

OpenCL Land is the OpenCL cloud computing interface.

With ocland you can use devices in remote computers as OpenCL platforms installed in the local one, doing unnecessary the usage of other interfaces like MPI, resulting in a significant codes simplification. Also ocland can be used by all the applications that are accelerated with OpenCL without modifing nothing! No source code modifications or compilations are needed to have your OpenCL applications ready to use ocland!

Since ocland is based in a server-client structure ligthly lower performance can be expected compared with MPI based applications.

For the moment ocland is in a heavy development stage, but you can start testing it. But remember that is in alpha stage so is not recommended for production purposes.

ocland is composed by 3 subprojects (all of them provided with this package):

1) ocland server
2) ocland installable client driver (ICD)
3) ocland examples

All the ocland components compilation and install are aided with CMake.

ocland server
=============

Ocland server must be installed in all the computers that will serve computation resources remotely.

Suposing that you have downloaded ocland in a ocland.tar.gz compressed file you can install ocland server with the following commands (executed in the place where you downloaded the file):

tar -xvzf ocland.tar.gz
cd ocland
cmake -DOCLAND_SERVER:BOOL=ON -DOCLAND_SERVER_DAEMON:BOOL=ON -DOCLAND_CLIENT:BOOL=OFF -DOCLAND_CLIENT_ICD:BOOL=OFF OCLAND_EXAMPLES:BOOL=OFF .
make
make install

Note that the last command must be executed as superuser. Depending on your operative system, consider set the flag -DCMAKE_INSTALL_PREFIX:PATH=/usr too.

The daemon (set with the flag OCLAND_SERVER_DAEMON) is used in order to launch the ocland server at the operative system start, but if you don't want to launch automatically the ocland server you can disable the daemon (-DOCLAND_SERVER_DAEMON:BOOL=OFF).

When the server has been installed you can launch it typing:

ocland_server

In order to clients can access to ocland server resources several ports starting in 51000 must be opened. In ocland the port 51000 is used to stablish the connection between the client and server, but later more ports starting in 51001 will be opened to can perform asynchronously data transfers without interfere the main communication channel.

ocland ICD
==========

The ocland ICD must be installed in the computer where the OpenCL applications that you want to use remote resources will be launched.

Suposing that you have downloaded ocland in a ocland.tar.gz compressed file you can install ocland server with the following commands (executed in the place where you downloaded the file):

tar -xvzf ocland.tar.gz
cd ocland
cmake -DOCLAND_SERVER:BOOL=OFF -DOCLAND_SERVER_DAEMON:BOOL=OFF -DOCLAND_CLIENT:BOOL=ON -DOCLAND_CLIENT_ICD:BOOL=ON OCLAND_EXAMPLES:BOOL=OFF .
make
make install

Note that the last command must be executed as superuser. Depending on your operative system, consider set the flag -DCMAKE_INSTALL_PREFIX:PATH=/usr too.

The ocland driver will be installed, and a referency will be generated in the file /etc/OpenCL/vendors/ocland.icd in order to report to the ICD loader that must query ocland for available platforms.

In order to use remote resources you must create a plain text file called ocland in the folder where you will launch the OpenCL application, with the servers IP addresses (one per line). When application query for OpenCL platforms ocland will automatically connect to ocland servers specified in the ocland named file. If the file is not present, is blank, or the server are not available, simply no ocland platforms will offered, but you ever still have available the local platforms.

ocland examples
===============

The ocland examples are small tests designed to probe the ocland installation.

Suposing that you have downloaded ocland in a ocland.tar.gz compressed file you can install ocland server with the following commands (executed in the place where you downloaded the file):

tar -xvzf ocland.tar.gz
cd ocland
cmake -DOCLAND_SERVER:BOOL=OFF -DOCLAND_SERVER_DAEMON:BOOL=OFF -DOCLAND_CLIENT:BOOL=OFF -DOCLAND_CLIENT_ICD:BOOL=OFF OCLAND_EXAMPLES:BOOL=ON .
make
make install

Note that the last command must be executed as superuser. Depending on your operative system, consider set the flag -DCMAKE_INSTALL_PREFIX:PATH=/usr too.

Since the examples must be launched in the computers that has the ocland ICD installed, probably you want to install the ICD an the examples at the same time (-DOCLAND_CLIENT:BOOL=ON -DOCLAND_CLIENT_ICD:BOOL=ON OCLAND_EXAMPLES:BOOL=ON)

Supposing that you have installed the server and the client in the same computer, you can launch the first test executing (take into account the CMAKE_INSTALL_PREFIX selected flag):

echo 127.0.0.1 > ocland
/usr/local/share/ocland/test/test

And you will see how a small OpenCL application is executed in the local devices availables in two ways, the usual one and through the network managed by ocland (The platform name vendor and suffix have an "ocland(127.0.0.1)" prefix identifier)

It's magic!
