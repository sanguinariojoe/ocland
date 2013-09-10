ocland
======

OpenCL Land is the OpenCL cloud computing interface.

With ocland you can use devices in remote computers as OpenCL platforms installed in the local one, doing unnecessary the usage of other interfaces like MPI, resulting in a significant codes simplification. Also ocland can be used by all the applications that are accelerated with OpenCL without any modification! No source code modifications or recompilations are needed to have your OpenCL applications ready to use ocland!

You can learn more about ocland in the following web page:

http://ocland.sourceforge.net

Installing
==========

ocland software is hosted on GitHUB, so to can download it probably you want to install git. In Debian based Linux distributions you can type:

    apt-get install git

And then you can download the software typing:

    git clone https://github.com/sanguinariojoe/ocland.git

That will generate the ocland folder with the source code inside.

The main way to compile and install ocland is using CMake. To install the server (with the daemon to launch it at the operating system start), the client (with the ICD to act as an OpenCL platform), and the examples, the following commands can be used:

    cd ocland
    cmake .
    make
    make install

And reboot your computer.

Testing
=======

Open a terminal and create the ocland file:

    echo 127.0.0.1 > ocland

And launch the test example:

    /usr/share/ocland/test/test

And you will see how a small OpenCL application is executed in the local devices availables in two ways, the usual one and through the network managed by ocland (The platform name vendor and suffix have an "ocland(127.0.0.1)" prefix identifier)

It's magic!
