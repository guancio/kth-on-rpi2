/*
 * copyright (c) 2005-2009 Imperas Ltd., www.imperas.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied.
 *
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
   TC */
#include <stdio.h>
#include <ctype.h>

#include "icm/icmCpuManager.h"

#include "string.h"


/******************************************************/
void createPlatform(
                    const char *cpuname,
                    unsigned long ram_base,
                    unsigned long ram_size,
                    int debug, int verbose,
                    icmProcessorP *cpu_, icmBusP *bus_) 
{
    
    int diag_level = verbose ? 7 : 0;
    Uns32 icmAttrs = ICM_ATTR_SIMEX; // simulate exceptions
    Uns32 icmInitAttrs = ICM_STOP_ON_CTRLC | ICM_VERBOSE;    
    
    /////////////////////////////////////////////////////////////////////////
    if (debug)
        icmInit(ICM_VERBOSE, "localhost", 9999);
    else
        icmInit(ICM_VERBOSE, 0, 0);
    
    icmSetSimulationTimeSlice(0.001000);
    
    
    icmBusP bus1_b = icmNewBus( "bus1_b", 32);
    icmBusP membus_b = icmNewBus( "membus_b", 32);
    
    
    const char *arm1_path = icmGetVlnvString(
        0, "arm.ovpworld.org", 0, "arm", 0, "model");

    icmAttrListP arm1_attr = icmNewAttrList();
    icmAddStringAttr(arm1_attr, "variant", cpuname);
    icmAddStringAttr(arm1_attr, "compatibility", "ISA");
    icmAddStringAttr(arm1_attr, "showHiddenRegs", "0");
    icmAddDoubleAttr(arm1_attr, "mips", 200.000000);
    icmAddStringAttr(arm1_attr, "endian", "little");

    if(debug) icmAttrs |= ICM_ATTR_DEBUG;

    icmProcessorP arm1_c = icmNewProcessor(
        "arm1",  "arm", 0, 0, 32, arm1_path,
        "modelAttrs", icmAttrs, arm1_attr, 0, 0);

    icmConnectProcessorBusses( arm1_c, bus1_b, bus1_b );
    
    
    /* PSE cm */
    const char *cm_path = icmGetVlnvString(0, 0, 0, "CoreModule9x6", 0, "pse");    
    icmAttrListP cm_attr = icmNewAttrList();
    icmPseP cm_p = icmNewPSE("cm", cm_path, cm_attr, 0, 0);
    icmConnectPSEBus( cm_p, bus1_b, "bport1", 0, 0x10000000, 0x10000fff);

    /* PSE pic1 */
    const char *pic1_path = icmGetVlnvString(0, 0, 0, "IntICP", 0, "pse");
    icmAttrListP pic1_attr = icmNewAttrList();
    icmPseP pic1_p = icmNewPSE( "pic1", pic1_path, pic1_attr, 0, 0);
    icmConnectPSEBus( pic1_p, bus1_b, "bport1", 0, 0x14000000, 0x14000fff);
    
    /* PSE pic2 */
    const char *pic2_path = icmGetVlnvString(0, 0, 0, "IntICP", 0, "pse");
    icmAttrListP pic2_attr = icmNewAttrList();
    icmPseP pic2_p = icmNewPSE("pic2", pic2_path, pic2_attr, 0, 0);
    icmConnectPSEBus( pic2_p, bus1_b, "bport1", 0, 0xca000000, 0xca000fff);    
    
    /* PSE ethlan (dummy) */
    const char *ethlan_path = icmGetVlnvString(0, "smsc.ovpworld.org",
                                               "peripheral", "LAN9118", "1.0", "pse");
    icmAttrListP ethlan_attr = icmNewAttrList();    
    icmPseP ethlan_p = icmNewPSE("ethlan", ethlan_path, ethlan_attr, 0, 0);
    icmConnectPSEBus( ethlan_p, bus1_b, "bport1", 0, 0xc8000000, 0xc8000fff);
    
    /* PSE pit */
    const char *pit_path = icmGetVlnvString(0, 0, 0, "IcpCounterTimer", 0, "pse");
    icmAttrListP pit_attr = icmNewAttrList();
    icmPseP pit_p = icmNewPSE("pit", pit_path, pit_attr, 0, 0);
    icmConnectPSEBus( pit_p, bus1_b, "bport1", 0, 0x13000000, 0x13000fff);    
    
    
    /* PSE icp */
    const char *icp_path = icmGetVlnvString(0, 0, 0, "IcpControl", 0, "pse");
    icmAttrListP icp_attr = icmNewAttrList();
    icmPseP icp_p = icmNewPSE("icp", icp_path, icp_attr, 0, 0);
    icmConnectPSEBus( icp_p, bus1_b, "bport1", 0, 0xcb000000, 0xcb00000f);

    /* PSE ld1 */
    const char *ld1_path = icmGetVlnvString(0, 0, 0, "DebugLedAndDipSwitch", 0, "pse");
    icmAttrListP ld1_attr = icmNewAttrList();
    icmPseP ld1_p = icmNewPSE("ld1", ld1_path, ld1_attr, 0, 0);
    icmConnectPSEBus( ld1_p, bus1_b, "bport1", 0, 0x1a000000, 0x1a000fff);
    
    /* PSE kb1 */
    const char *kb1_path = icmGetVlnvString(0, 0, 0, "KbPL050", 0, "pse");
    const char *kb1_pe = icmGetVlnvString(0, 0, 0, "KbPL050", 0, "model");
    icmAttrListP kb1_attr = icmNewAttrList();
    icmAddUns64Attr(kb1_attr, "isMouse", 0);
    icmAddUns64Attr(kb1_attr, "grabDisable", 0);

    icmPseP kb1_p = icmNewPSE( "kb1", kb1_path, kb1_attr, kb1_pe, "modelAttrs");
    icmConnectPSEBus( kb1_p, bus1_b, "bport1", 0, 0x18000000, 0x18000fff);

    /* PSE ms1 */
    const char *ms1_path = icmGetVlnvString(0, 0, 0, "KbPL050", 0, "pse");
    const char *ms1_pe = icmGetVlnvString(0, 0, 0, "KbPL050", 0, "model");

    icmAttrListP ms1_attr = icmNewAttrList();
    icmAddUns64Attr(ms1_attr, "isMouse", 1);
    icmAddUns64Attr(ms1_attr, "grabDisable", 1);
    icmPseP ms1_p = icmNewPSE("ms1", ms1_path, ms1_attr, ms1_pe, "modelAttrs");
    icmConnectPSEBus( ms1_p, bus1_b, "bport1", 0, 0x19000000, 0x19000fff);
    
    
    /* PSE rtc */
    const char *rtc_path = icmGetVlnvString(0, 0, 0, "RtcPL031", 0, "pse");
    icmAttrListP rtc_attr = icmNewAttrList();
    icmPseP rtc_p = icmNewPSE("rtc", rtc_path, rtc_attr, 0, 0);
    icmConnectPSEBus( rtc_p, bus1_b, "bport1", 0, 0x15000000, 0x15000fff);


    /* PSR uart1 */
    const char *uart1_path = icmGetVlnvString(0, 0, 0, "UartPL011", 0, "pse");
    icmAttrListP uart1_attr = icmNewAttrList();
    icmAddDoubleAttr(uart1_attr, "console", 1);
    // icmAddDoubleAttr(uart1_attr, "portnum", uartPort);       
    icmAddDoubleAttr(uart1_attr, "finishOnDisconnect", 1);       
    icmAddStringAttr(uart1_attr, "outfile", "uart1.log");
    icmAddStringAttr(uart1_attr, "variant", "ARM");

    icmPseP uart1_p = icmNewPSE("uart1", uart1_path, uart1_attr, 0, 0);
    icmConnectPSEBus( uart1_p, bus1_b, "bport1", 0, 0x16000000, 0x16000fff);
    icmSetPSEdiagnosticLevel(uart1_p, 1);
    
    /* PSR uart2 */
    const char *uart2_path = icmGetVlnvString(0, 0, 0, "UartPL011", 0, "pse");
    icmAttrListP uart2_attr = icmNewAttrList();

    icmAddStringAttr(uart2_attr, "variant", "ARM");
    icmPseP uart2_p = icmNewPSE("uart2", uart2_path, uart2_attr, 0, 0);
    icmConnectPSEBus( uart2_p, bus1_b, "bport1", 0, 0x17000000, 0x17000fff);
    
    /* PSE mmci */
    const char *mmci_path = icmGetVlnvString(0, 0, 0, "MmciPL181", 0, "pse");
    const char *mmci_pe = icmGetVlnvString(0, 0, 0, "MmciPL181", 0, "model");
    icmAttrListP mmci_attr = icmNewAttrList();

    icmPseP mmci_p = icmNewPSE("mmci", mmci_path, mmci_attr, mmci_pe, "modelAttrs");
    icmConnectPSEBus( mmci_p, bus1_b, "bport1", 0, 0x1c000000, 0x1c000fff);

#if 0    
    /* PSE lcd */
    const char *lcd_path = icmGetVlnvString(0, 0, 0, "LcdPL110", 0, "pse");
    const char *lcd_pe = icmGetVlnvString(0, 0, 0, "LcdPL110", 0, "model");
    icmAttrListP lcd_attr = icmNewAttrList();

    icmAddUns64Attr(lcd_attr, "scanDelay", 50000);
    // icmAddDoubleAttr(lcd_attr, "noGraphics", True);
    icmAddDoubleAttr(lcd_attr, "busOffset",  0x80000000);
    icmPseP lcd_p = icmNewPSE("lcd", lcd_path, lcd_attr, lcd_pe, "modelAttrs");
    icmConnectPSEBus( lcd_p, bus1_b, "bport1", 0, 0xc0000000, 0xc0000fff);
    icmConnectPSEBusDynamic( lcd_p, membus_b, "memory", 0);
#endif    
    
    
    /* RAM1 */
    icmMemoryP ram1_m = icmNewMemory("ram1_m", 0x7, ram_size - 1);
    icmConnectMemoryToBus( membus_b, "sp1", ram1_m, ram_base);

    
    /* dummy amba */
    icmMemoryP ambaDummy_m = icmNewMemory("ambaDummy_m", 0x7, 0xfff);
    icmConnectMemoryToBus( bus1_b, "sp1", ambaDummy_m, 0x1d000000);
    
    /* RAM1 brdige */
    icmNewBusBridge(bus1_b, membus_b, "ram1Bridge", "sp", "mp", 
                    ram_base, ram_base + ram_size - 1, ram_base);
#if 0
    /* RAM2 bridge */
    icmNewBusBridge(bus1_b, membus_b, "ram2Bridge", "sp1", "mp", 
                    ram_base, ram_base + ram_size - 1, 0x80000000);
#endif    
    
    /* CONNECTIONS */
    icmNetP irq_n = icmNewNet("irq_n" );
    icmConnectProcessorNet( arm1_c, irq_n, "irq", ICM_INPUT);
    icmConnectPSENet( pic1_p, irq_n, "irq", ICM_OUTPUT);
    
    icmNetP fiq_n = icmNewNet("fiq_n" );
    icmConnectProcessorNet( arm1_c, fiq_n, "fiq", ICM_INPUT);
    icmConnectPSENet( pic1_p, fiq_n, "fiq", ICM_OUTPUT);

    icmNetP ir1_n = icmNewNet("ir1_n" );
    icmConnectPSENet( pic1_p, ir1_n, "ir1", ICM_INPUT);
    icmConnectPSENet( uart1_p, ir1_n, "irq", ICM_OUTPUT);

    icmNetP ir2_n = icmNewNet("ir2_n" );
    icmConnectPSENet( pic1_p, ir2_n, "ir2", ICM_INPUT);
    icmConnectPSENet( uart2_p, ir2_n, "irq", ICM_OUTPUT);

    icmNetP ir3_n = icmNewNet("ir3_n" );
    icmConnectPSENet( pic1_p, ir3_n, "ir3", ICM_INPUT);
    icmConnectPSENet( kb1_p, ir3_n, "irq", ICM_OUTPUT);

    icmNetP ir4_n = icmNewNet("ir4_n" );
    icmConnectPSENet( pic1_p, ir4_n, "ir4", ICM_INPUT);
    icmConnectPSENet( ms1_p, ir4_n, "irq", ICM_OUTPUT);

    icmNetP ir5_n = icmNewNet("ir5_n" );
    icmConnectPSENet( pic1_p, ir5_n, "ir5", ICM_INPUT);
    icmConnectPSENet( pit_p, ir5_n, "irq0", ICM_OUTPUT);

    icmNetP ir6_n = icmNewNet("ir6_n" );
    icmConnectPSENet( pic1_p, ir6_n, "ir6", ICM_INPUT);
    icmConnectPSENet( pit_p, ir6_n, "irq1", ICM_OUTPUT);

    icmNetP ir7_n = icmNewNet("ir7_n" );
    icmConnectPSENet( pic1_p, ir7_n, "ir7", ICM_INPUT);
    icmConnectPSENet( pit_p, ir7_n, "irq2", ICM_OUTPUT);

    icmNetP ir8_n = icmNewNet("ir8_n" );
    icmConnectPSENet( pic1_p, ir8_n, "ir8", ICM_INPUT);
    icmConnectPSENet( rtc_p, ir8_n, "irq", ICM_OUTPUT);

    icmNetP ir23_n = icmNewNet("ir23_n" );
    icmConnectPSENet( pic1_p, ir23_n, "ir23", ICM_INPUT);
    icmConnectPSENet( mmci_p, ir23_n, "irq0", ICM_OUTPUT);

    icmNetP ir24_n = icmNewNet("ir24_n" );
    icmConnectPSENet( pic1_p, ir24_n, "ir24", ICM_INPUT);
    icmConnectPSENet( mmci_p, ir24_n, "irq1", ICM_OUTPUT);

    
    icmPrintf("End of platform code\n");
    /* return create stuff */
    *cpu_ = arm1_c;
    *bus_ = bus1_b;    
}
