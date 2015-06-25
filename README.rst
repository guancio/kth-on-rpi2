======================
DISCLAIMER
======================
STH Hypervisor is a research project and is continuously being changed and developed and may contain stability issues and security vulnerabilities at this stage. You are free to use STH Hypervisor, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.


What is STH?
======================

The SICS Thin Hypervisor (STH) software is a small portable hypervisor designed for the ARM architecture, with the main purpose to improve the security of embedded systems through the isolation properties that virtualization can provide. This is achieved by having a very thin software layer, the hypervisor, solely running in the processors privileged mode, managing and maintaining the resource allocation and access policies to the guests running on top of it. This allows the hypervisor to limit the power of the guest by para-virtualizing it to run in unprivileged mode, effectively creating isolation between the different components. With the help of the hypervisor, one can run a commodity OS kernel such as Linux and its user appli- cations together with trusted security services in different execution environments isolated from each other.

The STH software is currently primarily used for research purposes and is dual licensed GPL/Commercial. See the accompanying license file for more details. 

ON WHAT HARDWARE DOES IT RUN?
============
The STH hypervisor currently runs on the 32-bit ARM v5 and v7 architecture. It is highly portable, and can easily be ported to most 32-bit or 64-bit architectures as long as they have a memory management unit (MMU). 

DOCUMENTATION
============
The /doc subdirectory contains STH.pdf which have more detailed information about the STH hypervisor, it's API, configurations, build instructions and the para-virtualized Linux guest. 

BUILD DIRECTORY
============
When building the STH hypervisor, all output files will be stored together in the /core/build subdirectory. Depending on the platform, it will output sth_*platform*.fw.img which is a U-boot bootable image file. This can be loaded into real hardware or simulated with Qemu or OVP (Open Virtual Platforms).

CONFIGURING 
============
The hypervisor can be built for different platforms and this is specified in the target file in the root source tree. Exactly one of the PLATFORM and one of the SOFTWARE needs to be defined in order to build successfully. 

	- # -*- Makefile -*-
	- # Target configuration
	- #PLATFORM=ovp_arm9
	- #PLATFORM=ovp_integratorCP
	- #PLATFORM=u8500_ref
	- PLATFORM=beagleboard
	- #PLATFORM=beaglebone

	- #SOFTWARE=minimal
	- SOFTWARE = trusted linux
	- #SOFTWARE = linux
	
	- #Enable this if you want to compile for OVP 
	- #SIMULATION_OVP = 1

For example this target configuration builds the hypervisor for the beagleboard platform with the trusted application and linux kernel as guests. Some specific platforms can also be compiled for simulation on OVP, to do this, remove the comment from SIMULATION_OVP.   

It is important to perform a Make clean before switching target platform or software, as old lingering object files will lead to bugs and errors.

COMPILING
============
In order to build the Hypervisor, execute the Makefile located at the root of the source tree after configuring the target file with the command

	make

For more information on suggested cross-compiler and tools, see Appendix A.1 in the STH.pdf documentation in /doc subdirectory.



