# DORK OS
Lightweight terminal-based DOS-inspired OS

# INFO
I did use an online tutorial, but I fixed the code on the tutorial to match the assembly used in the Ubuntu terminal. Source later on.  
I programmed the entire OS in Ubuntu (not the desktop environment).  
This is actually my final project for an engineering course.  
It does come with a few commands that I will list later on.

# COMMANDS
craft /message/      - Print a message to the screen  
exit                 - Quit the OS  
clear                - Clear the screen  
help                 - Open a help menu of commands in the OS


# SOURCES
https://dev.to/frosnerd/series/9585

# INSTALL PROCESS
This is programmed using the Ubuntu terminal for WSL, so a couple dependencies are required to emulate this.  
1. Install QEMU using `sudo apt install -y qemu-kvm libvirt-daemon-system libvirt-clients bridge-utils`
2. Install `make` (the build tool used to run) with `sudo apt install build-essential`, which will install the GCC compiler, and Makefile
3. Install NASM (the assembler which also comes with the language itself) with `sudo apt install nasm`  

Now that we have the installations, we can now run the OS

# RUN PROCESS
In order to run, we just need two commands.
1. In order to properly run the OS, in the terminal, locate to the file that contains all the code (most importantly the Makefile)
2. run `make clean` in the terminal, which will remove any existing output files. (Important to do this before running every time if editing the OS)
3. run `make` afterwards to run the OS
