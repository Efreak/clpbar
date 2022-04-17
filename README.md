Command Line Progress Bar is a simple command line tool to display information about a data transfer stream. It will display the number of bytes transfered, the speed of the transfer, and if the size of the data stream is known it will display the ETA.

The original program is [here](http://clpbar.sourceforge.net/); this repo just has some commands run to make the configure script *actually run* on modern systems. You may need to re-run the commands yourself if there's any symlinks (there shouldn't be); just check the git history. The binary included is for termux/android, aarch64.
