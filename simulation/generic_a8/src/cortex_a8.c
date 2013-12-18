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
//#include "icm/icmMessage.h"
#include "string.h"

/* what to include ? */
#define INCLUDE_USART
#define INCLUDE_CPU_HELPER
// #define INCLUDE_DMA


/******************************************************/

/*
 * this creates a Cortex-A8 platform.
 * For now, we will be using the same peripherals as the ARM9 target :(
 */

void createPlatform(int debug, int verbose, 
                    icmProcessorP *cpu_, icmBusP *bus_) 
{
    
    int diag_level = verbose ? 7 : 0;
    
    
    /////////////////////////////////////////////////////////////////////////
    if (debug)
        icmInit(ICM_VERBOSE, "localhost", 9999);
    else
        icmInit(ICM_VERBOSE, 0, 0);
    
    icmSetSimulationTimeSlice(0.001000);
    
    
    /////////////////////////////////////////////////////////////////////////
    //                      Bus   INSTANCE   mainbus
    /////////////////////////////////////////////////////////////////////////
    
    icmBusP bus = icmNewBus( "mainbus", 32);
    
    
    /////////////////////////////////////////////////////////////////////////
    //                Processor   INSTANCE    Cortex-A8
    /////////////////////////////////////////////////////////////////////////
    
    /*User defined attributes*/
    
    icmAttrListP processorAttrList = icmNewAttrList();
    icmAddStringAttr(processorAttrList, "variant", "Cortex-A8");
    
    icmAddStringAttr(processorAttrList, "endian", "little");
    icmAddDoubleAttr(processorAttrList, "mips", 200);
    
    const char *armModel = icmGetVlnvString(
                                            0,    					// path (0 if from the product directory)
                                            "arm.ovpworld.org", 	//"ovpworld.org"   // vendor
                                            "processor",    		// library
                                            "arm",					//"arm",// name
                                            "1.0",  	  			// version
                                            "model"     			// model
                                            );
    
    const char *armSemihost = icmGetVlnvString(
                                               0,
                                               "arm.ovpworld.org",
                                               "semihosting",
                                               "armNewlib",
                                               "1.0",
                                               "model"
                                               );
    
    
    Uns32 procAttrs = ICM_ATTR_SIMEX | 0x60;
    int cpu_flags = 0; // ICM_ATTR_TRACE | ICM_ATTR_TRACE_REGS_AFTER;
    if(debug) procAttrs |= ICM_ATTR_DEBUG;
    icmProcessorP processor = icmNewProcessor(
                                              "Cortex-A8",		// name
                                              "arm",   				// type
                                              0,   					// cpuId
                                              cpu_flags, 					// flags
                                              32,   					// address bits
                                              armModel,   			// model
                                              "modelAttrs",   		// symbol
                                              procAttrs,   			// procAttrs
                                              processorAttrList, 		// attribute list
                                              armSemihost,			//string_6, // semihost file
                                              "modelAttrs"    		// semihost symbol
                                              );
    
    
//    if(debug)
//        icmDebugThisProcessor(processor);
    

    icmConnectProcessorBusses( processor, bus, bus );
    
    
    /////////////////////////////////////////////////////////////////////////
    //			VAP TOOLS
    /////////////////////////////////////////////////////////////////////////
#ifdef INCLUDE_CPU_HELPER    
    const char *cpuHelper = icmGetVlnvString(0,
                                             "imperas.com", "intercept", "armCpuHelper", "1.0", "model");
    
    icmAddInterceptObject(
                          processor, 		// Processor handle
                          "cpuH", 		// name for instance of intercept lib
                          cpuHelper,		// VLNV name of intercept library
                          "modelAttrs", 	// intercept attributes symbol name
                          0 				// no user defined attributes
                          );
    
    const char *vapHelper = icmGetVlnvString(0,
                                             "imperas.com", "intercept", "vapHelper", "1.0", "model");
    icmAddInterceptObject(
                          processor, 		// Processor handle
                          "vapH", 		// name for instance of intercept lib
                          vapHelper,		// VLNV name of intercept library
                          "modelAttrs", 	// intercept attributes symbol name
                          0 				// no user defined attributes
                          );
    
    const char *vapTools = icmGetVlnvString(0,
                                            "imperas.com", "intercept", "vapTools", "1.0", "model");
    
    icmAddInterceptObject(
                          processor, 		// Processor handle
                          "vapT", 		// name for instance of intercept lib
                          vapTools,		// VLNV name of intercept library
                          "modelAttrs", 	// intercept attributes symbol name
                          0 				// no user defined attributes
                          );
    
        
#endif    
    
    /////////////////////////////////////////////////////////////////////////
    //         Peripheral (PSE)   INSTANCE   AIC
    /////////////////////////////////////////////////////////////////////////
    
    icmAttrListP aicAttrList = icmNewAttrList();
    
    const char *AIC_Model = icmGetVlnvString(
                                             0,    							// path (0 if from the product directory)
                                             "atmel.ovpworld.org",  			// vendor
                                             "peripheral",   				// library
                                             "AdvancedInterruptController",  // name
                                             "1.0",    						// version
                                             "pse"     						// model
                                             );
    
    icmPseP AIC = icmNewPSE(
                            "AIC",  			 	// name
                            AIC_Model,   			// model
                            aicAttrList,  	 	// attrlist
                            0,   					// semihost file
                            0    					// semihost symbol
                            );
    
    icmSetPSEdiagnosticLevel(AIC, diag_level);
    
    icmConnectPSEBus( AIC, bus, "bp1", 0, 0xfffff000, 0xfffffFFF);
    
    icmNetP icmNetP_33 = icmNewNet("NFIQ" );
    icmConnectProcessorNet( processor, icmNetP_33, "fiq", ICM_INPUT);
    icmConnectPSENet( AIC, icmNetP_33, "NFIQ", ICM_OUTPUT);
    
    icmNetP icmNetP_34 = icmNewNet("NIRQ" );
    icmConnectProcessorNet( processor, icmNetP_34, "irq", ICM_INPUT);
    icmConnectPSENet( AIC, icmNetP_34, "NIRQ", ICM_OUTPUT);
    
    
    
    
    /////////////////////////////////////////////////////////////////////////
    //         Peripheral (PSE)   INSTANCE   WD
    /////////////////////////////////////////////////////////////////////////
    
    icmAttrListP wdtAttrList = icmNewAttrList();
    
    const char *wctModel = icmGetVlnvString(
                                            0,    					// path (0 if from the product directory)
                                            "atmel.ovpworld.org",   // vendor
                                            "peripheral",    		// library
                                            "WatchdogTimer",    	// name
                                            "1.0",    				// version
                                            "pse"     				// model
                                            );
    
    icmPseP watchDogTimer = icmNewPSE(
                                      "WD",   				// name
                                      wctModel,   			// model
                                      wdtAttrList,   // attrlist
                                      0,   					// semihost file
                                      0  						// semihost symbol
                                      );
    
    
    icmSetPSEdiagnosticLevel(watchDogTimer, diag_level);
    
    icmConnectPSEBus( watchDogTimer, bus, "bp1", 0, 0xffff8000, 0xffffbfff);
    
    icmNetP icmNetP_40 = icmNewNet("WDIRQ" );
    icmConnectPSENet( AIC, icmNetP_40, "WDIRQ", ICM_INPUT);
    icmConnectPSENet( watchDogTimer, icmNetP_40, "IRQ", ICM_OUTPUT);
    
    
    /////////////////////////////////////////////////////////////////////////
    //         Peripheral (PSE)   INSTANCE   PS
    /////////////////////////////////////////////////////////////////////////
    
    icmAttrListP psAttrList = icmNewAttrList();
    
    const char *psModel = icmGetVlnvString(
                                           0,    					// path (0 if from the product directory)
                                           "atmel.ovpworld.org",   // vendor
                                           "peripheral",    		// library
                                           "PowerSaving",    		// name
                                           "1.0",    				// version
                                           "pse"     				// model
                                           );
    
    icmPseP powerSaving = icmNewPSE(
                                    "PS",   				// name
                                    psModel,   			// model
                                    psAttrList,   		// attrlist
                                    0,   					// semihost file
                                    0    					// semihost symbol
                                    );
    
    icmConnectPSEBus( powerSaving, bus, "bp1", 0, 0xffff4000, 0xffff7fff);
    
    
    /////////////////////////////////////////////////////////////////////////
    //         Peripheral (PSE)   INSTANCE   PIO
    /////////////////////////////////////////////////////////////////////////
    
    icmAttrListP pIOAttrList = icmNewAttrList();
    
    const char *pIOModel = icmGetVlnvString(
                                            0,    					// path (0 if from the product directory)
                                            "atmel.ovpworld.org",   // vendor
                                            "peripheral",    		// library
                                            "ParallelIOController", // name
                                            "1.0",    				// version
                                            "pse"     				// model
                                            );
    
    icmPseP pio = icmNewPSE(
                              "PIO",   				// name
                              pIOModel,   			// model
                              pIOAttrList,   			// attrlist
                              0,   					// semihost file
                              0    					// semihost symbol
                              );
    
    icmConnectPSEBus( pio, bus, "bp1", 0, 0xffff0000, 0xffff3FFF);
    icmNetP icmNetP_41 = icmNewNet("PIOIRQ" );
    icmConnectPSENet( AIC, icmNetP_41, "PIOIRQ", ICM_INPUT);
    icmConnectPSENet( pio, icmNetP_41, "IRQ", ICM_OUTPUT);
    
    
    /////////////////////////////////////////////////////////////////////////
    //         Peripheral (PSE)   INSTANCE   TC
    /////////////////////////////////////////////////////////////////////////
    
    icmAttrListP tcAttrList = icmNewAttrList();
    
    const char *tcModel = icmGetVlnvString(
                                           0,    					// path (0 if from the product directory)
                                           "atmel.ovpworld.org",   // vendor
                                           "peripheral",    		// library
                                           "TimerCounter",    		// name
                                           "1.0",    				// version
                                           "pse"     				// model
                                           );
    
    icmPseP timerCounter = icmNewPSE(
                                     "TC",   				// name
                                     tcModel,   			// model
                                     tcAttrList,   		// attrlist
                                     0,   					// semihost file
                                     0    					// semihost symbol
                                     );
    
    
    icmSetPSEdiagnosticLevel(timerCounter, diag_level);
    
    icmConnectPSEBus( timerCounter, bus, "bp1", 0, 0xfffe0000, 0xfffe3fff);
    icmNetP icmNetP_37 = icmNewNet("TC0IRQ" );
    icmConnectPSENet( AIC, icmNetP_37, "TC0IRQ", ICM_INPUT);
    icmConnectPSENet( timerCounter, icmNetP_37, "IRQ0", ICM_OUTPUT);
    
    icmNetP icmNetP_38 = icmNewNet("TC1IRQ" );
    icmConnectPSENet( AIC, icmNetP_38, "TC1IRQ", ICM_INPUT);
    icmConnectPSENet( timerCounter, icmNetP_38, "IRQ1", ICM_OUTPUT);
    
    icmNetP icmNetP_39 = icmNewNet("TC2IRQ" );
    icmConnectPSENet( AIC, icmNetP_39, "TC2IRQ", ICM_INPUT);
    icmConnectPSENet( timerCounter, icmNetP_39, "IRQ2", ICM_OUTPUT);
        
    
    
    
    icmAttrListP sfAttrList = icmNewAttrList();
    
    const char *sfModel = icmGetVlnvString(
                                           0,    					// path (0 if from the product directory)
                                           "atmel.ovpworld.org",   // vendor
                                           "peripheral",    		// library
                                           "SpecialFunction",    	// name
                                           "1.0",    				// version
                                           "pse"				    // model
                                           );
    
    icmPseP specialFunction = icmNewPSE(
                                        "SF",   				// name
                                        sfModel,   			// model
                                        sfAttrList,   		// attrlist
                                        0,   					// semihost file
                                        0    					// semihost symbol
                                        );
    
    icmConnectPSEBus( specialFunction, bus, "bp1", 0, 0xfff00000, 0xfff03fff);                   
    
    
    
    /////////////////////////////////////////////////////////////////////////
    //                   Memory   INSTANCE   extDev
    /////////////////////////////////////////////////////////////////////////
    //16 MB of flash memory
    //icmMemoryP flash = icmNewMemory("extDev", 0x7, 0x00ffffff);
    //icmConnectMemoryToBus( bus, "sp1", flash, 0x1000000);
    
    
    /////////////////////////////////////////////////////////////////////////
    //                   Memory   INSTANCE   ram0
    /////////////////////////////////////////////////////////////////////////
    
    // NOTE: I increased the RAM from 1MB to 16MB, to enable 15 separate
    //       domains in the platform (ARM domains are 1MB minimum each).
    
    //0x7 r/w/e, last argument is memory upper bound
    icmMemoryP ram = icmNewMemory("ram0", ICM_PRIV_RWX, 1024 * 1024 * 16-1);        
    icmConnectMemoryToBus( bus, "sp1", ram, 0x0);
    
    
    /* my additional 32MB of SRAM at 0x8000_0000 */
    icmMemoryP ram2 = icmNewMemory("ram2", ICM_PRIV_RWX, 1024 * 1024 * 32-1);
    icmConnectMemoryToBus( bus, "sp0", ram2, 0x80000000);
    
    
#ifdef INCLUDE_DMA    
    
    /////////////////////////////////////////////////////////////////////////
    //                   Memory   INSTANCE   ram1
    /////////////////////////////////////////////////////////////////////////
    // test memory for DMA    
    icmMemoryP dmaRAM = icmNewMemory("ram1", 0x0, 0x000fffff); //0x0 no access
    
    icmConnectMemoryToBus( bus, "sp1", dmaRAM, 0x00f00000);
    
    ////////////////////////////////////////////////////////////////////////////
    // 							DMAC Peripheral
    ////////////////////////////////////////////////////////////////////////////
    
    // instantiate the peripheral
    icmPseP icmPseP_dmac = icmNewPSE("dmac", "obj/dmacModel.pse", NULL, NULL, NULL);
    
    // connect the DMAC slave port on the bus and define the address range it occupies
    icmConnectPSEBus(icmPseP_dmac, bus, "DMACSP", False, 0xfff10000, 0xfff1013f);
    icmConnectPSEBus(icmPseP_dmac, bus, "MREAD",  True, 0x00000000, 0xffffffff);  //DMA_TODO: limit range here?
    icmConnectPSEBus(icmPseP_dmac, bus, "MWRITE", True, 0x00000000, 0xffffffff);
    
    
    // diagnostic level
    icmSetPSEdiagnosticLevel(icmPseP_dmac, diag_level);
    
    icmNetP icmNet_dmac_cpu = icmNewNet("DMACIL" ); //DMA: net for interrupt line of DMAC
    icmConnectPSENet( AIC, icmNet_dmac_cpu, "US0IRQ", ICM_INPUT); //DMA: We include the AIC instead of the CPU into the DMAC-net
    // as the interrupt ports of the ARM processor are already occupied
    icmConnectPSENet(icmPseP_dmac, icmNet_dmac_cpu, "INTTC", ICM_OUTPUT); //DMA: connect the DMAC interrupt port to the net
    
#endif /* INCLUDE_DMA */    
    
    
#ifdef INCLUDE_USART
    /////////////////////////////////////////////////////////////////////////
    //         Peripheral (PSE)   INSTANCE   USART
    /////////////////////////////////////////////////////////////////////////
    
    icmAttrListP usartAttrList = icmNewAttrList();
    icmAddDoubleAttr(usartAttrList, "log", 1);
    icmAddStringAttr(usartAttrList, "outfile", "ovp_uart.log");

    
    // icmAddDoubleAttr(usartAttrList, "portnum", 9999); 
    // icmAddStringAttr(usartAttrList, "finishOnDisconnect", "on");
    
    
    const char *USART_PSE = icmGetVlnvString(
                                             0,    							// path (0 if from the product directory)
                                             "atmel.ovpworld.org",  			// vendor
                                             "peripheral",   				// library
                                             "UsartInterface",  // name
                                             "1.0",    						// version
                                             "pse"     						// model
                                             );
    
    const char *USART_Model = icmGetVlnvString(
                                               0,    // path (0 if from the product directory)
                                               "atmel.ovpworld.org",    // vendor
                                               "peripheral",    // library
                                               "UsartInterface",    // name
                                               "1.0",    // version
                                               "model"     // model
                                               );
    
    icmPseP USART = icmNewPSE(
                            "USART0",  			 	// name
                            USART_PSE,   			// model
                            usartAttrList,  	 	// attrlist
                            USART_Model,   					// semihost file
                            "modelAttrs"    					// semihost symbol
                            );
    
    icmSetPSEdiagnosticLevel(USART, diag_level);
    
    icmConnectPSEBus( USART, bus, "asb", 1, 0x00000000, 0xFFFFFFFF);
    icmConnectPSEBus( USART, bus, "apb", 0, 0xFFFD0000, 0xFFFD3FFF);
    
    
    icmNetP usart0_irq = icmNewNet("US0IRQ" );
    icmConnectPSENet( AIC, usart0_irq, "US0IRQ", ICM_INPUT);
    icmConnectPSENet( USART, usart0_irq, "IRQ", ICM_OUTPUT);
    
#endif    
        
    
    icmSetSimulationTimeSlice(0.001000);
    
    
    // processor.addWriteCallback();
    icmPrintBusConnections(bus);
    
    /* return create stuff */
    *cpu_ = processor;
    *bus_ = bus;
}
