/* $Id: xsysmon_intr_example.c,v 1.1.2.1 2010/07/27 06:35:23 sadanan Exp $ */
/******************************************************************************
*
* (c) Copyright 2007-2009 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information of Xilinx, Inc.
* and is protected under U.S. and international copyright and other
* intellectual property laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any rights to the
* materials distributed herewith. Except as otherwise provided in a valid
* license issued to you by Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
* and (2) Xilinx shall not be liable (whether in contract or tort, including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature related to, arising under or in connection with these
* materials, including for any direct, or any indirect, special, incidental,
* or consequential loss or damage (including loss of data, profits, goodwill,
* or any type of loss or damage suffered as a result of any action brought by
* a third party) even if such damage or loss was reasonably foreseeable or
* Xilinx had been advised of the possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe performance, such as life-support or
* safety devices or systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any other applications
* that could lead to death, personal injury, or severe property or
* environmental damage (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and liability of any use of
* Xilinx products in Critical Applications, subject only to applicable laws
* and regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
******************************************************************************/
/****************************************************************************/
/**
*
* @file xsysmon_intr_example.c
*
* This file contains a design example using the driver functions
* of the System Monitor/ADC driver. This example here shows the usage of the
* driver/device in interrupt mode to handle on-chip temperature and voltage
* alarm interrupts.
*
*
* @note
*
* This code assumes that no Operating System is being used.
*
* The values of the on-chip temperature and the on-chip Vccaux voltage are read
* from the device and then the alarm thresholds are set in such a manner that
* the alarms occur.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.00a xd/sv  05/22/07 First release
* 2.00a sv     06/22/08 Modified the function description of the interrupt
*			handler
* 2.00a sdm    09/26/08 Added code to return temperature value to the main
*			function. TestappPeripheral prints the temperature
* 4.00a ktn    10/22/09 Updated the example to use HAL processor APIs/macros.
*		        Updated the example to use macros that have been
*		        renamed to remove _m from the name of the macro.
* 5.00 srinivasa attili  2-2-12 modified for Kintex7 Demo TRD
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include <stdio.h>
#include "xsysmon.h"
#include "xparameters.h"
#include "xstatus.h"
#include "xintc.h"
#include "xil_exception.h"
#include "xuartns550_l.h"
#include "board_test_app.h"


/************************** Constant Definitions ****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef TESTAPP_GEN
#define SYSMON_DEVICE_ID	XPAR_SYSMON_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
#define INTR_ID				XPAR_MICROBLAZE_SUBSYSTEM_INTERRUPT_CNTLR_MICROBLAZE_SUBSYSTEM_AXI_XADC_0_IP2INTC_IRPT_INTR //XPAR_INTC_0_SYSMON_0_VEC_ID
#endif

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Function Prototypes *****************************/

int SysMonIntrExample(XIntc* IntcInstancePtr,
			XSysMon* SysMonInstPtr,
			u16 SysMonDeviceId,
			u16 SysMonIntrId,
			int *Temp);

#if 0
static void SysMonInterruptHandler(void *CallBackRef);
static int SysMonSetupInterruptSystem(XIntc* IntcInstancePtr,
				      XSysMon *SysMonPtr,
				      u16 IntrId );
#endif

/************************** Variable Definitions ****************************/

#ifndef TESTAPP_GEN
static XSysMon SysMonInst; 	  /* System Monitor driver instance */
static XSysMon_Config *SysMonConfigPtr;
static XIntc InterruptController; /* Instance of the XIntc driver. */
static int sysmon_init=0;
#endif

/*
 * Shared variables used to test the callbacks.
 */
volatile static int TemperatureIntr = FALSE; 	/* Temperature alarm intr */
volatile static int VccauxIntr = FALSE;	  	/* VCCAUX alarm interrupt */

#ifndef TESTAPP_GEN

float t, vint, vaux;

int SysMonIntrExample2(XSysMon* SysMonInstPtr, u16 SysMonDeviceId, u16 SysMonIntrId,
		char *adcData, int printable);

/****************************************************************************/
/**
*
* Main function that invokes the Interrupt example.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
* @note		None.
*
*****************************************************************************/
int main_sysmonIntr2(char *adcData, int printable)
{

	int Status;

	/*
	 * Run the SysMonitor interrupt example, specify the parameters that
	 * are generated in xparameters.h.
	 */
	//xil_printf("XSysMon Intr test\r\n");
	Status = SysMonIntrExample2(&SysMonInst,
				   SYSMON_DEVICE_ID,
				   INTR_ID,
				   adcData, printable);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;

}

int main_sysmonIntr(void)
{

	int Status;
	int Temp;

    xil_printf("\n\r********************************************************");
    xil_printf("\n\r********************************************************");
    xil_printf("\n\r**     KC705 - XADC Test                              **");
    xil_printf("\n\r********************************************************");
    xil_printf("\n\r********************************************************\r\n");

	/*
	 * Run the SysMonitor interrupt example, specify the parameters that
	 * are generated in xparameters.h.
	 */
	//xil_printf("XSysMon Intr test\r\n");
	Status = SysMonIntrExample(&InterruptController,
				   &SysMonInst,
				   SYSMON_DEVICE_ID,
				   INTR_ID,
				   &Temp);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;

}
#endif /* TESTAPP_GEN */

/****************************************************************************/
/**
*
* This function runs a test on the System Monitor/ADC device using the
* driver APIs.
*
* The function does the following tasks:
*	- Initiate the System Monitor/ADC device driver instance
*	- Run self-test on the device
*	- Reset the device
*	- Set up alarms for on-chip temperature and VCCAUX
*	- Set up sequence registers to continuously monitor on-chip temperature
*	and VCCAUX
*	- Setup interrupt system
*	- Enable interrupts
*	- Set up configuration registers to start the sequence
*	- Wait until temperature alarm interrupt or VCCAUX alarm interrupt
*	occurs
*
* @param	IntcInstancePtr is a pointer to the Interrupt Controller
*		driver Instance.
* @param	SysMonInstPtr is a pointer to the XSysMon driver Instance.
* @param	SysMonDeviceId is the XPAR_<SYSMON_ADC_instance>_DEVICE_ID value
*		from xparameters.h.
* @param	SysMonIntrId is
*		XPAR_<INTC_instance>_<SYSMON_ADC_instance>_VEC_ID value from
*		xparameters.h
* @param	Temp is an output parameter, it is a pointer through which the
*		current temperature value is returned to the main function.
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
* @note		This function may never return if no interrupt occurs.
*
****************************************************************************/
int SysMonIntrExample2(XSysMon* SysMonInstPtr,
			u16 SysMonDeviceId, u16 SysMonIntrId, char *adcData, int printable)
{
	int Status;
	u32 TempData;
	u32 VccauxData;
	u32 VccintData;
	u32 IntrStatus;
	int j;
	unsigned int whole1, thousandths1;
	unsigned int whole2, thousandths2;
	unsigned int whole3, thousandths3;

	/*
	 * Initialize the SysMon driver.
	 */

	if(sysmon_init==0)
	{
		//xil_printf("XSysMon intr example\r\n");
		SysMonConfigPtr = XSysMon_LookupConfig(SysMonDeviceId);
		if (SysMonConfigPtr == NULL) {
			return XST_FAILURE;
		}
		//xil_printf("XSysMon_CfgInitialize\r\n");
		XSysMon_CfgInitialize(SysMonInstPtr, SysMonConfigPtr, SysMonConfigPtr->BaseAddress);

		/*
		 * Self Test the System Monitor/ADC device.
		 */
		//xil_printf("XSysMon_SelfTest\r\n");
		Status = XSysMon_SelfTest(SysMonInstPtr);
		if (Status != XST_SUCCESS) {
			xil_printf("XSysMon_SelfTest failed\r\n");
			return XST_FAILURE;
		}
		sysmon_init = 1;
	}


	/*
	 * Disable the Channel Sequencer before configuring the Sequence
	 * registers.
	 */
	XSysMon_SetSequencerMode(SysMonInstPtr, XSM_SEQ_MODE_SAFE);

	/*
	 * Setup the Averaging to be done for the channels in the
	 * Configuration 0 register as 16 samples:
	 */
	XSysMon_SetAvg(SysMonInstPtr, XSM_AVG_16_SAMPLES);

	/*
	 * Enable the averaging on the following channels in the Sequencer
	 * registers:
	 * 	- On-chip Temperature
	 * 	- On-chip VCCAUX supply sensor
	 * 	- 1st Auxiliary Channel
	 * 	- 16th Auxiliary Channel
	 */
	Status =  XSysMon_SetSeqAvgEnables(SysMonInstPtr, XSM_SEQ_CH_TEMP |
						/* XSM_SEQ_CH_AUX00 | */
						/* XSM_SEQ_CH_AUX15 | */
						XSM_SEQ_CH_VCCINT |
						XSM_SEQ_CH_VCCAUX );
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable the following channels in the Sequencer registers:
	 * 	- On-chip Temperature
	 * 	- On-chip VCCAUX supply sensor
	 * 	- 1st Auxiliary Channel
	 * 	- 16th Auxiliary Channel
	 */
	Status =  XSysMon_SetSeqChEnables(SysMonInstPtr, XSM_SEQ_CH_TEMP |
						/* XSM_SEQ_CH_AUX00 | */
						/* XSM_SEQ_CH_AUX15 | */
						XSM_SEQ_CH_VCCINT |
						XSM_SEQ_CH_VCCAUX );
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Set the ADCCLK frequency equal to 1/32 of System clock for the System
	 * Monitor/ADC in the Configuration Register 2.
	 */
	XSysMon_SetAdcClkDivisor(SysMonInstPtr, 32);


	/*
	 * Enable the Channel Sequencer in continuous sequencer cycling mode.
	 */
	XSysMon_SetSequencerMode(SysMonInstPtr, XSM_SEQ_MODE_CONTINPASS);

	/*
	 * Disable all the alarms in the Configuration Register 1.
	 */
	XSysMon_SetAlarmEnables(SysMonInstPtr, 0x0);

	/*
	 * Wait till the End of Sequence occurs
	 */
	XSysMon_GetStatus(SysMonInstPtr); /* Clear the old status */

	do
	{
		if((XSysMon_GetStatus(SysMonInstPtr) & XSM_SR_EOS_MASK) ==
					XSM_SR_EOS_MASK)
			break;
	} while(1);

	/*
	 * Read the ADC converted Data from the data registers for on-chip
	 * temperature and on-chip VCCAUX voltage.
	 */
	TempData = XSysMon_GetAdcData(SysMonInstPtr, XSM_CH_TEMP);
	VccauxData = XSysMon_GetAdcData(SysMonInstPtr, XSM_CH_VCCAUX);
	VccintData = XSysMon_GetAdcData(SysMonInstPtr, XSM_CH_VCCINT);

	/*
	 * Convert the ADC data into temperature
	 */
	//*Temp = XSysMon_RawToTemperature(TempData);

	t    = (float) XSysMon_RawToTemperature(TempData);
	vint = (float) XSysMon_RawToVoltage(VccintData);
	vaux = (float) XSysMon_RawToVoltage(VccauxData);

	whole1 = t;
	thousandths1 = (t - whole1) * 1000;
	whole2 = vint;
	thousandths2 = (vint - whole2) * 1000;
	whole3 = vaux;
	thousandths3 = (vaux - whole3) * 1000;

	//xil_printf("Temperature: 0x%x, Vccint: 0x%x Vccaux: 0x%x  [Exit: press any key]\r\n",
	//		TempData, VccintData, VccauxData);
	if(printable==1)
	{
		// send complete string with units
		sprintf(adcData, "Temperature:%d.%-3dC, Vccint:%d.%-3dV,  Vccaux: %d.%-3dV",
				whole1, thousandths1, whole2, thousandths2, whole3, thousandths3);
	}
	else
	{
		// send ONLY the numbers (used for graph plot by the webserver
		sprintf(adcData, "%d.%-d, %d.%-d, %d.%-d",
				whole1, thousandths1, whole2, thousandths2, whole3, thousandths3);
	}
	//printf("Temperature: %d.%3dC, Vccint: %d.%3dV Vccaux: %d.%3dV\r\n",
	//		whole1, thousandths1, whole2, thousandths2, whole3, thousandths3);

	for(j=0; j<10000;j++);	// some delay

	//if(!XUartNs550_IsReceiveData(STDIN_BASEADDRESS))
	//{
	//	continue;
	//}
	//else
	//{
	//	XUartNs550_ReadReg(STDIN_BASEADDRESS, XUN_RBR_OFFSET);
	//	return 0;
	//}

	/*
	 * Clear any bits set in the Interrupt Status Register.
	 */
	IntrStatus = XSysMon_IntrGetStatus(SysMonInstPtr);
	XSysMon_IntrClear(SysMonInstPtr, IntrStatus);

	/*
	 * Disable global interrupt of System Monitor.
	 */
	XSysMon_IntrGlobalDisable(SysMonInstPtr);

	return XST_SUCCESS;
}

int SysMonIntrExample(XIntc* IntcInstancePtr, XSysMon* SysMonInstPtr,
			u16 SysMonDeviceId, u16 SysMonIntrId, int *Temp)
{
	int Status;
	XSysMon_Config *ConfigPtr;
	u32 TempData;
	u32 VccauxData;
	u32 VccintData;
	u32 IntrStatus;
	int i, j;
	unsigned int whole1, thousandths1;
	unsigned int whole2, thousandths2;
	unsigned int whole3, thousandths3;


	/*
	 * Initialize the SysMon driver.
	 */

	//xil_printf("XSysMon intr example\r\n");
	ConfigPtr = XSysMon_LookupConfig(SysMonDeviceId);
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}
	//xil_printf("XSysMon_CfgInitialize\r\n");
	XSysMon_CfgInitialize(SysMonInstPtr, ConfigPtr, ConfigPtr->BaseAddress);

	/*
	 * Self Test the System Monitor/ADC device.
	 */
	//xil_printf("XSysMon_SelfTest\r\n");
	Status = XSysMon_SelfTest(SysMonInstPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("XSysMon_SelfTest failed\r\n");
		return XST_FAILURE;
	}


	/*
	 * Disable the Channel Sequencer before configuring the Sequence
	 * registers.
	 */
	XSysMon_SetSequencerMode(SysMonInstPtr, XSM_SEQ_MODE_SINGCHAN);

	/*
	 * Setup the Averaging to be done for the channels in the
	 * Configuration 0 register as 16 samples:
	 */
	XSysMon_SetAvg(SysMonInstPtr, XSM_AVG_16_SAMPLES);

	#if 0
	/*
	 * Setup the Sequence register for 1st Auxiliary channel
	 * Setting is:
	 *	- Add acquisition time by 6 ADCCLK cycles.
	 *	- Bipolar Mode
	 *
	 * Setup the Sequence register for 16th Auxiliary channel
	 * Setting is:
	 *	- Add acquisition time by 6 ADCCLK cycles.
	 *	- Unipolar Mode
	 */
	Status = XSysMon_SetSeqInputMode(SysMonInstPtr, XSM_SEQ_CH_AUX00);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XSysMon_SetSeqAcqTime(SysMonInstPtr, XSM_SEQ_CH_AUX15 |
						XSM_SEQ_CH_AUX00);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	#endif

	/*
	 * Enable the averaging on the following channels in the Sequencer
	 * registers:
	 * 	- On-chip Temperature
	 * 	- On-chip VCCAUX supply sensor
	 * 	- 1st Auxiliary Channel
	 * 	- 16th Auxiliary Channel
	 */
	Status =  XSysMon_SetSeqAvgEnables(SysMonInstPtr, XSM_SEQ_CH_TEMP |
						/* XSM_SEQ_CH_AUX00 | */
						/* XSM_SEQ_CH_AUX15 | */
						XSM_SEQ_CH_VCCINT |
						XSM_SEQ_CH_VCCAUX );
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable the following channels in the Sequencer registers:
	 * 	- On-chip Temperature
	 * 	- On-chip VCCAUX supply sensor
	 * 	- 1st Auxiliary Channel
	 * 	- 16th Auxiliary Channel
	 */
	Status =  XSysMon_SetSeqChEnables(SysMonInstPtr, XSM_SEQ_CH_TEMP |
						/* XSM_SEQ_CH_AUX00 | */
						/* XSM_SEQ_CH_AUX15 | */
						XSM_SEQ_CH_VCCINT |
						XSM_SEQ_CH_VCCAUX );
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Set the ADCCLK frequency equal to 1/32 of System clock for the System
	 * Monitor/ADC in the Configuration Register 2.
	 */
	XSysMon_SetAdcClkDivisor(SysMonInstPtr, 32);


	/*
	 * Enable the Channel Sequencer in continuous sequencer cycling mode.
	 */
	XSysMon_SetSequencerMode(SysMonInstPtr, XSM_SEQ_MODE_CONTINPASS);

	/*
	 * Disable all the alarms in the Configuration Register 1.
	 */
	XSysMon_SetAlarmEnables(SysMonInstPtr, 0x0);

	/*
	 * Wait till the End of Sequence occurs
	 */
	XSysMon_GetStatus(SysMonInstPtr); /* Clear the old status */

	i = 0;
	while(i < 10)
	{
		if((XSysMon_GetStatus(SysMonInstPtr) & XSM_SR_EOS_MASK) !=
					XSM_SR_EOS_MASK)
			continue;

		/*
		 * Read the ADC converted Data from the data registers for on-chip
		 * temperature and on-chip VCCAUX voltage.
		 */
		TempData = XSysMon_GetAdcData(SysMonInstPtr, XSM_CH_TEMP);
		VccauxData = XSysMon_GetAdcData(SysMonInstPtr, XSM_CH_VCCAUX);
		VccintData = XSysMon_GetAdcData(SysMonInstPtr, XSM_CH_VCCINT);

		/*
		 * Convert the ADC data into temperature
		 */
		*Temp = XSysMon_RawToTemperature(TempData);

		t    = (float) XSysMon_RawToTemperature(TempData);
		vint = (float) XSysMon_RawToVoltage(VccintData);
		vaux = (float) XSysMon_RawToVoltage(VccauxData);

		whole1 = t;
	    thousandths1 = (t - whole1) * 1000;
		whole2 = vint;
	    thousandths2 = (vint - whole2) * 1000;
		whole3 = vaux;
	    thousandths3 = (vaux - whole3) * 1000;

		//xil_printf("Temperature: 0x%x, Vccint: 0x%x Vccaux: 0x%x  [Exit: press any key]\r\n",
		//		TempData, VccintData, VccauxData);
		printf("Temperature: %d.%3dC, Vccint: %d.%3dV Vccaux: %d.%3dV\r\n",
				whole1, thousandths1, whole2, thousandths2, whole3, thousandths3);

		for(j=0; j<10000;j++);	// some delay

		//if(!XUartNs550_IsReceiveData(STDIN_BASEADDRESS))
		//{
		//	continue;
		//}
		//else
		//{
		//	XUartNs550_ReadReg(STDIN_BASEADDRESS, XUN_RBR_OFFSET);
		//	return 0;
		//}
		i++;
	}

	#if 0
	/*
	 * Set up Alarm threshold registers for the on-chip temperature and
	 * VCCAUX High limit and lower limit so that the alarms DONOT occur.
	 */
	XSysMon_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_TEMP_UPPER, 0xFFFF);
	XSysMon_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_TEMP_LOWER, 0xFFFF);

	XSysMon_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_VCCAUX_UPPER, 0xFFFF);
	XSysMon_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_VCCAUX_LOWER, 0x0);

	XSysMon_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_VCCINT_UPPER, 0xFFFF);
	XSysMon_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_VCCINT_LOWER, 0x0);

	/*
	 * Setup the interrupt system.
	 */
	Status = SysMonSetupInterruptSystem(IntcInstancePtr,
					    SysMonInstPtr,
					    SysMonIntrId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	#endif

	/*
	 * Clear any bits set in the Interrupt Status Register.
	 */
	IntrStatus = XSysMon_IntrGetStatus(SysMonInstPtr);
	XSysMon_IntrClear(SysMonInstPtr, IntrStatus);

	#if 0
	/*
	 * Enable Alarm 0 interrupt for on-chip temperature and Alarm 2
	 * interrupt for on-chip VCCAUX.
	 */
	XSysMon_IntrEnable(SysMonInstPtr,
				XSM_IPIXR_TEMP_MASK |
				XSM_IPIXR_VCCAUX_MASK);
	/*
	 * Enable global interrupt of System Monitor.
	 */
	XSysMon_IntrGlobalEnable(SysMonInstPtr);

	/*
	 * Set up Alarm threshold registers for
	 * On-chip Temperature High limit
	 * On-chip Temperature Low limit
	 * VCCAUX High limit
	 * VCCAUX Low limit
	 */
	XSysMon_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_TEMP_UPPER,
						TempData - 0x007F);
	XSysMon_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_TEMP_LOWER,
						TempData - 0x007F);
	XSysMon_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_VCCAUX_UPPER,
						VccauxData - 0x007F);
	XSysMon_SetAlarmThreshold(SysMonInstPtr, XSM_ATR_VCCAUX_LOWER,
						VccauxData + 0x007F);
	/*
	 * Enable Alarm 0 for on-chip temperature and Alarm 2 for on-chip
	 * VCCAUX in the Configuration Register 1.
	 */
	XSysMon_SetAlarmEnables(SysMonInstPtr, (XSM_CFR1_ALM_VCCAUX_MASK |
						XSM_CFR1_ALM_TEMP_MASK));

	/*
	 * Wait until an Alarm 0 or Alarm 2 interrupt occurs.
	 */
	while (1) {
		if (TemperatureIntr == TRUE) {
			/*
			 * Alarm 0 - Temperature alarm interrupt has occurred.
			 * The required processing should be put here.
			 */
			break;
		}

		if (VccauxIntr == TRUE) {
			/*
			 * Alarm 2 - VCCAUX alarm interrupt has occurred.
			 * The required processing should be put here.
			 */
			break;
		}
	}
	#endif

	/*
	 * Disable global interrupt of System Monitor.
	 */
	XSysMon_IntrGlobalDisable(SysMonInstPtr);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is the Interrupt Service Routine for the System Monitor device.
* It will be called by the processor whenever an interrupt is asserted
* by the device.
*
* There are 10 different interrupts supported
*	- Over Temperature
*	- ALARM 0
*	- ALARM 1
*	- ALARM 2
*	- End of Sequence
*	- End of Conversion
*	- JTAG Locked
*	- JATG Modified
*	- Over Temperature DeActive
*	- ALARM 0 DeActive
*
* This function only handles ALARM 0 and ALARM 2 interrupts. User of this
* code may need to modify the code to meet needs of the application.
*
* @param	CallBackRef is the callback reference passed from the Interrupt
*		controller driver, which in our case is a pointer to the
*		driver instance.
*
* @return	None.
*
* @note		This function is called within interrupt context.
*
******************************************************************************/
#if 0
static void SysMonInterruptHandler(void *CallBackRef)
{
	u32 IntrStatusValue;
	XSysMon *SysMonPtr = (XSysMon *)CallBackRef;

	/*
	 * Get the interrupt status from the device and check the value.
	 */
	IntrStatusValue = XSysMon_IntrGetStatus(SysMonPtr);

	if (IntrStatusValue & XSM_IPIXR_TEMP_MASK) {
		/*
		 * Set Temperature interrupt flag so the code
		 * in application context can be aware of this interrupt.
		 */
		TemperatureIntr = TRUE;
	}

	if (IntrStatusValue & XSM_IPIXR_VCCAUX_MASK) {
		/*
		 * Set VCCAUX interrupt flag so the code in application context
		 * can be aware of this interrupt.
		 */

		VccauxIntr = TRUE;
	}

	/*
	 * Clear all bits in Interrupt Status Register.
	 */
	XSysMon_IntrClear(SysMonPtr, IntrStatusValue);
 }
#endif

/****************************************************************************/
/**
*
* This function sets up the interrupt system so interrupts can occur for the
* System Monitor/ADC.  The function is application-specific since the actual
* system may or may not have an interrupt controller. The System Monitor/ADC
* device could be directly connected to a processor without an interrupt
* controller. The user should modify this function to fit the application.
*
* @param	IntcInstancePtr is a pointer to the Interrupt Controller
*		driver Instance.
* @param	SysMonPtr is a pointer to the driver instance for the System
* 		Monitor device which is going to be connected to the interrupt
*		controller.
* @param	IntrId is XPAR_<INTC_instance>_<SYSMON_ADC_instance>_VEC_ID
*		value from xparameters.h.
*
* @return	XST_SUCCESS if successful, or XST_FAILURE.
*
* @note		None.
*
*
****************************************************************************/
#if 0
static int SysMonSetupInterruptSystem(XIntc* IntcInstancePtr,
				      XSysMon *SysMonPtr,
				      u16 IntrId )
{
	int Status;

#ifndef TESTAPP_GEN
	/*
	 * Initialize the interrupt controller driver so that it's ready to use.
	 */
	Status = XIntc_Initialize(IntcInstancePtr, INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif
	/*
	 * Connect the handler that will be called when an interrupt
	 * for the device occurs, the handler defined above performs the
	 * specific interrupt processing for the device.
	 */
	Status = XIntc_Connect(IntcInstancePtr,
		 		IntrId,
				(XInterruptHandler) SysMonInterruptHandler,
				SysMonPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

#ifndef TESTAPP_GEN
	/*
	 * Start the interrupt controller so interrupts are enabled for all
	 * devices that cause interrupts. Specify real mode so that the System
	 * Monitor/ACD device can cause interrupts through the interrupt
	 * controller.
	 */
	Status = XIntc_Start(IntcInstancePtr, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif
	/*
	 * Enable the interrupt for the System Monitor/ADC device.
	 */
	XIntc_Enable(IntcInstancePtr, IntrId);

#ifndef TESTAPP_GEN

	/*
	 * Initialize the exception table.
	 */
	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				(Xil_ExceptionHandler) XIntc_InterruptHandler,
				IntcInstancePtr);
	/*
	 * Enable exceptions.
	 */
	Xil_ExceptionEnable();

#endif /* TESTAPP_GEN */

	return XST_SUCCESS;
}
#endif

