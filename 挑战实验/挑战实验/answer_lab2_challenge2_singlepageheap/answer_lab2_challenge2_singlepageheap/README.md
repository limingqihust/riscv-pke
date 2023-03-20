## About riscv-pke (Proxy Kernel for Education) ##
----------

Documents in Chinese can be found [here](https://gitee.com/syivester/pke-doc).

PKE is an open source project (see [LICENSE.txt](./LICENSE.txt) for license information) for the educational purpose of the Operating System course, given to undergraduate students majored in CS (computer science) in many universities.

PKE provides a series of labs that cover the engineering-side knowledge points of the Operating System, including:

Lab1: traps (syscalls), exceptions and interrupts (IRQs in Intel terminology).   

Lab2: memory management.

Lab3: processes.

Lab4: devices.

Lab5: file systems.


Different from many famous OS educational projects (like [xv6](https://pdos.csail.mit.edu/6.828/2020/xv6.html) (JOS when earlier) used in MIT 6.828 and [ucore](https://github.com/oscourse-tsinghua/ucore-rv) taught in Tsinghua University) that use complete or near-complete OS kernels containing almost everything like process management, file systems and many other modules, *PKE is **NOT** a complete OS kernel (actually, PKE never intends to be one of them.)*.


PKE is built around the idea of Proxy Kernel (proposed in [PK](https://github.com/riscv/riscv-pk), an open source project of the RISC-V software ecology), that emphasizes to construct a "just-enough" OS kernel for a given application. With such an idea, we design a series of labs in PKE that gradually "upgrades" the OS kernel by giving a set of applications, from simple to complex. During the upgradations, you can learn more and more sophisticated ideas of modern operating systems, and more importantly, play with them by following the labs, one after another. 


In each lab, PKE starts with an application (placed in the *./user/* folder) and an incomplete proxy OS kernel. During the lab, you need to 1) understand the interaction between applicaion and proxy OS kernel (sometimes, also the RISC-V machine emulated by using [Spike](https://github.com/riscv/riscv-isa-sim)); 2) follow the code from the given application to the OS kernel based on the understanding; 3) complete the proxy OS kernel to make the application (or the system) executing correctly and smoothly.       
    

In the labs of PKE, we tried our best to control and minimize the code scale of each lab, hoping to help you to stay focus on the key components of Operating System, and minimize their efforts at the same time. Contact us if you have further suggestions on reducing the code scale, thanks in advance! 


Environment configuration  
----------

**1. Install Operating system (Virtual machine or WSL)**

(preferred) Ubuntu 16.04LTS or higher, 64-bit

**2. Install tools for building cross-compiler and emluator** 

    $ sudo apt-get install autoconf automake autotools-dev curl python3 libmpc-dev libmpfr-dev libgmp-dev gawk build-essential bison flex

**3. Install the risc-v cross-compiler**

    $ export RISCV=/path-to-install-RISCV-toolchains
    $ git clone --recursive https://github.com/riscv/riscv-gnu-toolchain.git
    $ cd riscv-gnu-toolchain
    $ ./configure --prefix=$RISCV
    $ make -j$(nproc)
    $ sudo make install

In above commands, *$(nproc)* stands for the number of threads you want to invoke during building. Generelly, *$(nproc)* should equal to the number of cores that your computer have. After this step, you should find executables, like riscv64-unknown-elf-gcc, riscv64-unknown-elf-gdb, riscv64-unknown-elf-objdump, and many others (with the name prefix of "riscv64-unknown-elf-") in your */path-to-install-RISCV-toolchains/bin* directory.


**4. Install the emulator (Spike)**

    $ sudo apt-get install device-tree-compiler
    $ git clone https://github.com/riscv/riscv-isa-sim.git
    $ cd riscv-isa-sim
    $ mkdir build
    $ cd build
    $ ../configure --prefix=$RISCV
    $ make -j$(nproc)
    $ sudo make install

After this step, you should find executables like spike, spike-dasm in your */path-to-install-RISCV-toolchains/bin* directory. 

**5. Clone PKE source code (you may have already done this)**

    $ git clone https://github.com/mrshawcode/pke.git

After this step, you will have the pke directory containing the PKE labs. 

**6. Build PKE OS kernel** 

    $ make [run/test]



**7. (optional) Install OpenOCD for debugging**

	$ git clone https://github.com/riscv/riscv-openocd.git
	$ cd openocd
	$ ./bootstrap (when building from the git repository) 
	$ ./configure --prefix=$RISCV 
	$ make -j$(nproc)
	$ sudo make install 

After installing OpenOCD, you can debug the PKE kernel. Simply use following command: 

    $ make gdb

Start the first lab  
----------

In this lab, we are going to learn the basic priciples of trap (also called as the **syscall** in many textbooks).

A trap (for example, *printf* that is in our daily use) is generally issued by an application, and evetually handled by the kernel. It is very important for an OS to provide such facility, since applications running in less priviledged modes (e.g., User-mode in RISC-V) need to frequently perform legal operations like I/Os that require to be conducted in higher priviledge modes (e.g., Supervisor or Machine modes in RISC-V). 

Lab1_1 gives an application in "user/lab1_1_helloworld.c", whose main() function calls a function *printu* that has the same functionality as *printf*, but under a slightly different name. *printu* is defined in "user/do_print.c", and actually invokes the trap by the *ecall* instruction (see the inline assemblies in the function of *do_user_print*).          


#### Code structure of Lab1_1
----------
The structure of Lab1_1 is listed in the following:

    ├── LICENSE.txt
    ├── Makefile
    ├── README.md
    ├── kernel
    │   ├── assert.c
    │   ├── assert.h
    │   ├── file.c
    │   ├── file.h
    │   ├── frontend.c
    │   ├── frontend.h
    │   ├── include
    │   │   ├── atomic.h
    │   │   ├── config.h
    │   │   ├── functions.h
    │   │   ├── riscv.h
    │   │   └── types.h
    │   ├── kernel.c
    │   ├── kernel.lds
    │   ├── machine
    │   │   ├── fdt.c
    │   │   ├── fdt.h
    │   │   ├── htif.c
    │   │   ├── htif.h
    │   │   ├── mentry.S
    │   │   ├── minit.c
    │   │   ├── mtrap.c
    │   │   └── mtrap.h
    │   ├── proc.c
    │   ├── proc.h
    │   ├── syscall.c
    │   ├── syscall.h
    │   ├── trap.c
    │   ├── trap.h
    │   ├── usertrap.S
    │   └── util
    │   	├── elf.c
    │   	├── elf.h
    │   	├── riscv.S
    │   	├── snprintf.c
    │   	├── snprintf.h
    │   	├── string.c
    │   	└── string.h
    ├── spike.cfg
    ├── thinos_dump
    └── user
        ├── do_print.c
        ├── lab1_1_helloworld.c
        ├── user.h
        └── user.lds

The root directory mainly contains the documents (i.e., the md files), the license text and importantly, the make file (named as *Makefile*). The *kernel* sub-directory contains the OS kernel, while the *user* sub-directory contains the given application (in *lab1_1_helloworld.c*) as well as the source-code files containing the supporting routings, which should be placed in the user library in full-pledged OS like Linux.

To understand the PKE OS kernel (of Lab1_1) and accomplish the lab, you should start from the given application. Therefore, we start the tourism from *user/lab1_1_helloworld.c*:          

      1 /*
      2  * Below is the given app of lab1_1.
      3  *
      4  * You can build this app (as well as our PKE OS kernel) by command:
      5  * $ make
      6  *
      7  * Or run this app (with the support from PKE OS kernel) by command:
      8  * $ make run
      9  */
     10
     11 #include "user.h"
     12
     13 int main(void) {
     14   printu("Hello world!\n");
     15   return 0;
     16 }

From the code, we can observe that there is a newly defined function called *printu*, whose functionality equals to *printf* of our daily use. The reason we define a new function instead of using *printf* is that *printf* is already defined in the newlib of the RISC-V cross-compiling tool chain.

The prototype and implementation of *printu* can be found in *user/user.h* and *user/do_print.c* respectively.      

