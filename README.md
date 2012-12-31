ocland
======

OpenCL Land is the OpenCL cloud computing interface.

For the moment ocland is in a heavy development stage, so is not functionality. If you want to test it, Codeblocks IDE projects are provided.

Recompile the 3 projects in the following order:

ocland-server
ocland
test

Open a terminal with 3 tabs in this folder, and link the ocland library:

ln -s lib/libocland.so libocland.so

In the first tab execute the server:

bin/Release/ocland-server -l ocland.log

In the second tab you can track the server log info:

tail -f ocland.log

In the third one run the client test

bin/Release/test

Is magic! ;-)
