#ifndef _CPU_GIC_H_
#define _CPU_GIC_H_

/*
 * The standard ARM component Generic Interrupt Controler (GIC)
 */

/* DISTRIBUTER */
#define GICD_CTLR               0
#define GICD_TYPER              1
#define GICD_IIDR               2

#define GICD_IGROUPRn          32
#define GICD_ISENABLERn         64
#define GICD_ICENABLERn         96
#define GICD_ISPENDRn           128
#define GICD_ICPENDRn           160
#define GICD_ISACTIVERn                192
#define GICD_ICACTIVERn                224
#define GICD_IPRIORITYn                256
#define GICD_ITARGETSRn         512
#define GICD_ICFGRn             768
#define GICD_NSACRn             896

#define GICD_SGIR               960

#define GICD_CPENDSGIRn                964
#define GICD_SPENDSGIRn                968

/* CONTROLLER */
#define GICC_CTLR               0
#define GICC_PMR                1
#define GICC_BPR                2
#define GICC_IAR                3
#define GICC_EOIR               4
#define GICC_RPR                5
#define GICC_HPPIR              6
#define GICC_ABPR               7
#define GICC_AIAR               8
#define GICC_AEOIR              9
#define GICC_AHPPIR             10
#define GICC_APRn                52
#define GICC_NSAPRn             56
#define GICC_IIDR               63
#define GICC_DIR               1024


#endif /* _CPU_GIC_H_ */
