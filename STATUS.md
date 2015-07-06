Current status of the hypervisor port to RPi2.
==============================================

Here, we describe the current status of the RPi2 port of the hypervisor.

- bin: Done. Nothing to do here.
- core: still TODO. Looked at everything, core/config/hw/raspberrypi2.cfg still has open questions. core/hw/board/raspberrypi2 directory still has open questions. core/hw/ld/raspberrypi2.ld still has many open questions. core/hw/soc/bcm2836 is still TODO. Everything else should be OK.
- doc: Currently writing guide on how to add support for new stuff.
- drivers: DONE! I dislike the way the original UART driver is structured, but I've adapted it to the RPi2.
- library: No idea if anything is needed to be done here.
- rpi2-port: Test files for the RPi2 - should be done. Just need to revise the guide eventually.
- library: No idea if anything is needed to be done here.
- templates: DONE! Fixed templates/cpu/cortex_a7.cfg, templates/platform/raspberrypi2.cfg
- test: No idea if anything is needed to be done here.
- utils: No idea if anything is needed to be done here.
