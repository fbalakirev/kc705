/*****************************************************************************
** This confidential and proprietary code may be used only as authorized
** by licensing agreement from Xylon d.o.o.
** In the event of publication, the following notice is applicable:
**
** Copyright 2010 - Xylon d.o.o.
** All rights reserved.
**
** The entire notice above must be reproduced on all authorized copies.
*****************************************************************************/

/** \addtogroup logiCVCdemo logiCVC driver example
* @{
*/

/**
*
* @file logiCVC_demo.c
*
* Contains required functions for the logiCVC demo.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a af   8/06/10  First version
* </pre>
*
* modified on 2,Feb,2012 srinivasa attili; modified for Kintex7 Demo TRD
*****************************************************************************/

/***************************** Include Files *********************************/
#include <stdio.h>
#include "board_test_app.h"
#include "logiCVC_init.h"
#include "logiCVC_demo.h"
#include "xparameters.h"
#include "vdma_data.h"

/*********************** Constant declarations ************************/
#define ANIMATION_ON
#define ANIMATION_DURATION 5000.0
#define SHORT_WAIT         2000.0
#define LITTLE_WAIT        50.0

#define PRINT(...)       xil_printf(__VA_ARGS__)
#define DEBUG_PRINT(...) PRINT(__VA_ARGS__)
#define ERROR_PRINT(...) PRINT(__VA_ARGS__)
#define APP_PRINT(...)   PRINT(__VA_ARGS__)

	#define	CVC_RESET		{	\
		*((volatile int *)( CVC_HSY_FP         )) = 0x00000057;    /* (88   - 1 )= (0X58 -1) = 0X57	*/	\
		*((volatile int *)( CVC_HSY_WIDTH      )) = 0x0000002B;    /* (44   - 1 )= (0X2C -1) = 0X2B	*/	\
		*((volatile int *)( CVC_HSY_BP         )) = 0x00000093;    /* (148  - 1 )= (0X94 -1) = 0X93	*/	\
		*((volatile int *)( CVC_HSY_RESOLUTION )) = 0x0000077F;    /* (1920 - 1 )= (0X780-1) = 0X77F*/	\
		*((volatile int *)( CVC_VSY_FP         )) = 0x00000003;    /* (4    - 1 )= (0X04 -1) = 0X03	*/	\
		*((volatile int *)( CVC_VSY_WIDTH      )) = 0x00000004;    /* (5    - 1 )= (0X05 -1) = 0X04	*/	\
		*((volatile int *)( CVC_VSY_BP         )) = 0x00000023;    /* (36   - 1 )= (0X24 -1) = 0X23	*/	\
		*((volatile int *)( CVC_VSY_RESOLUTION )) = 0x00000437;    /* (1080 - 1 )= (0X438-1) = 0X437*/	\
		*((volatile int *)( CVC_CTRL           )) = 0x00000115;    /* HSYNC_EN =1, VSYNC_EN=1, DE_EN =1*/ \
		*((volatile int *)( CVC_VBUFF_SEL	   )) = 0x00000002;											\
	    *((volatile int *)( CVC_L0_CTRL        )) = 0x00000000;		/* 00;*/	\
	    *((volatile int *)( CVC_L1_CTRL        )) = 0x00000000;  	/* lAYER_1_EN =1 */	\
	    *((volatile int *)( CVC_L2_CTRL        )) = 0x00000000;  	/* lAYER_2_EN =1 */	\
	    *((volatile int *)( CVC_L3_CTRL        )) = 0x00000000;  	/* lAYER_2_EN =1 */	\
	}

#define	DIAGONAL_1		1	// move from top left     to bottom right
#define	DIAGONAL_2		2	// move from top right    to bottom left
#define	DIAGONAL_3		3	// move from bottom left  to top right
#define	DIAGONAL_4		4	// move from bottom right to top left
#define	HORIZONTAL_1	5	// move from left to right (keep y-axis same)
#define	HORIZONTAL_2	6	// move from right to left (keep y-axis same)
#define	VERTICAL_1		7	// move from top to bottom (keep x-axis same)
#define	VERTICAL_2		8	// move from bottom to top (keep x-axis same)

/*********************** Global variables declarations ************************/
static logiCvcPointerT logiCvcInstance[XPAR_LOGICVC_NUM_INSTANCES];
static Xuint32 ColorPerLayer[5] =
	{GREEN_24B, RED_24B, BLUE_24B, YELLOW_24B, PURPLE_24B};
int repeatVdemo;

/*********************** Local function declarations ************************/
void colorScaleOnLayer(logiCvcPointerT pLogiCvc, int layer);
void shrinkLayers(logiCvcPointerT pLogiCvc);
void moveLayers(logiCvcPointerT pLogiCvc);
void blendDownLayer(logiCvcPointerT pLogiCvc, int layer);
void blendDownLayerStep(logiCvcPointerT pLogiCvc, int layer, OsU8 alpha);
void setLayerTransparentColor(logiCvcPointerT pLogiCvc, int layer);
void changeMemoryOffset(logiCvcPointerT pLogiCvc, int layer);
void cursorDemo(logiCvcPointerT pLogiCvc);
void bufferingDemo(logiCvcPointerT pLogiCvc, int layer);
OsU32 getRawColor(logiCvcPointerT pLogiCvc, int layer, OsU32 ARGBColor);
OsU32 getTrueColor(logiCvcPointerT pLogiCvc, int layer, OsU32 rawColor);
void blendUpLayer(logiCvcPointerT pLogiCvc, int layer);


/****************************************************************************
*
* Runs logiCVC demo
*
* @return   XST_SUCCESS if everything runs correctly\n
*           XST_DEVICE_NOT_FOUND if the device is not found
*
* @note     This demo repeats over all logiCVC instances, using all layers in
*           each instance. If a layer is 8-bit, color lookup table is
*           initialized to contain default RGB_332 palette.
*
*****************************************************************************/
XStatus init_logiCVC(logiCVC_plfInitCallbackT *plfDisplayCallbacks, int DemoMode)
{
    int id, i, l;
    int layer, layers;
    logiCvcLayerPointerT pLayer;
    logiCvcClutPointerT  pClut;
    unsigned int displayTypes[] = {
    		LOGICVC_DT_1920_1080
    };
    logiCVC_plfInitCallbackT *pPlfDisplayCallbacks = NULL;

    APP_PRINT("\n\rlogiCVC initialization:\n\r");
    APP_PRINT("************************************************\n\r\n\r");

    /* Initialize driver and get logiCVC data */
    for(id = 0; id < XPAR_LOGICVC_NUM_INSTANCES; id++)
    {
        OsU32 flags = skipClearVideoMemory | skipCvcPowerControl; //(skipNone);
        logiCvcInstance[id] =
            logiCVC_init(id, displayTypes[id], flags, pPlfDisplayCallbacks);
        if(logiCvcInstance[id] != NULL)
        {
            DEBUG_PRINT("logiCVC %d initialised successfully!\n\r", id);
        }
        else
        {
            ERROR_PRINT("Error initializing logiCVC %d\n\r", id);
            return 0;
        }

        /* Find if there are any 8bpp layers and initialize default CLUT */
        /* as CLUT is otherwise initialized to zeros                     */
        /* (default CLUT will set simple RRRGGGBB (332) color palette)   */
        layers = logiCVC_GetCommonParam(logiCvcInstance[id], CVC_NUM_LAYERS_E);
        for (layer = 0; layer < layers; layer++)
        {
            pLayer = logiCVC_GetLayerPointer(logiCvcInstance[id], layer);
            OsU32 bitsPerPix = logiCVC_GetLayerParam(pLayer, CVC_LAYER_COLOR_BITS_E);
            if (bitsPerPix == 8)
            {
                pClut = logiCVC_GetClutPointer(pLayer, 0);
                for (i = 0; i < 256; i++)
                {
                    OsU32 color = ALPHA_24B | ((i & 0xe0)<<16) | ((i & 0x1c)<<11) | ((i & 3)<<6);
                    logiCVC_ClutWrite(pClut, i, color);
                }
            }
        }

		/***** Disable all layers *****/
		// layer0:DVI_IN_0;  layer1:TPG_0  layer2:DVI_1  layer3:TPG_2  layer4:Background
		for(l=0; l<4; l++)
		{
			pLayer = logiCVC_GetLayerPointer(logiCvcInstance[id], l);
			logiCVC_SetExternalBufferSwitchingCtrl(pLayer, TRUE);
			logiCVC_LayerEnDis(pLayer, FALSE);
		}

		// Enable background
		//xil_printf("Enable background layer \r\n");
		//pLayer = logiCVC_GetLayerPointer(logiCvcInstance[0], 4);
		//logiCVC_LayerEnDis(pLayer, TRUE);

		/* This function always accepts RGB_888 format */
		// byte0: bits7:6 = Blue
		// byte1: bits7:5 = Green
		// byte2: bits7:5 = Red
		logiCVC_SetBackgroundColor(logiCvcInstance[id], 0x660066); /* dark purple */
    }

    return 0;
}

/****************************************************************************
*
* Reset the LogiCVC hardware
*
* @return   nothing
*
* @note     nothing
*
*****************************************************************************/
XStatus reset_logiCVC(void)
{
	CVC_RESET;
	return 0;
}

/****************************************************************************
*
* Reset the LogiCVC State machine (software state machine)
*
* @return   nothing
*
* @note     nothing
*
*****************************************************************************/
void reset_logiCVC_DTYPE(void)
{
	*((volatile int *)( (XPAR_LOGICVC_0_REGS_BASEADDR+0x48) )) = 0x00000000;
	return;
}

/****************************************************************************
*
* disable the logiCVC layers
*
* @return   XST_SUCCESS
*
* @note     This action stops the CVC to fetch Video Frames from memory.
* 			Thus, it stops the VDMA activity.
* 			Background layer control is not in software. it is either
* 			enabled or disabled in the hardware once at boot time.
*
*****************************************************************************/
XStatus disable_logiCVC_layers(void)
{
	int l;
    logiCvcLayerPointerT pLayer;

	/***** Disable all layers, including background layer *****/
	// layer0:DVI_IN_0;  layer1:TPG_0  layer2:DVI_1  layer3:TPG_2  layer4:Background
	for(l=0; l<4; l++)
	{
		pLayer = logiCVC_GetLayerPointer(logiCvcInstance[0], l);
		logiCVC_SetExternalBufferSwitchingCtrl(pLayer, FALSE);
		logiCVC_LayerEnDis(pLayer, FALSE);
        //logiCVC_SetLayerAlphaValue(pLayer, 255);
	}

	logiCVC_ClearVsyncState(logiCvcInstance[0]);
    while(!logiCVC_IsSynced(logiCvcInstance[0]))
    {
        ;
    }

	repeatVdemo = 0;
	return 0;
}

/****************************************************************************
*
* setup the logiCVC layers as per the current selection of video streams
*
* @return   XST_SUCCESS
*
* @note
*	 *******************************************************************************
*	 *     Screen Layout:                                                          *
*	 *                +----------------+----------------+                          *
*	 *                |                |                |                          *
*	 *                |   DVI_IN_0     |      TPG_0     |                          *
*	 *                |    TPG_1       |                |                          *
*	 *                +----------------+----------------+                          *
*	 *                |                |                |                          *
*	 *                |    TPG_2       |    DVI_IN_1    |                          *
*	 *                |                |     TPG_3      |                          *
*	 *                +----------------+----------------+                          *
*	 *     		STREAM0 => Layer0 : DVI_IN_0 or TPG_1 (full or scaled, top left)   *
*	 *     		STREAM1 => Layer2 : DVI_IN_1 or TPG_3 (full or scaled, bot right)  *
*	 *     		TPG_0   => Layer1 : TPG_0 (always scaled, top right)               *
*	 *     		TPG_2   => Layer3 : TPG_2 (always scaled, bot left)                *
*	 *     		           LAYER4 : Background                                     *
*	 *******************************************************************************
*
*****************************************************************************/
#define FRAME_TO_TRIGGER 1
XStatus setup_logiCVC(int selection)
{
    XRECT r_full  = {0,    0, 1920, 1080};	// 1920, 1080		DVI_IN_0/DVI_IN_1
    XRECT r_scl_0 = {0,    0,  960,  540};	// 1920/2, 1080/2	DVI_IN_0
    XRECT r_scl_1 = {960,  0, 1920,  540};	// 1920/2, 1080/2	TPG_1
    XRECT r_scl_2 = {0,  540,  960, 1080};	// 1920/2, 1080/2	TPG_2
    XRECT r_scl_3 = {960,540, 1920, 1080};	// 1920/2, 1080/2	DVI_IN_1
    logiCvcLayerPointerT pLayer[5];
    int notUseBg;

    pLayer[0] = logiCVC_GetLayerPointer(logiCvcInstance[0], 0);
    pLayer[1] = logiCVC_GetLayerPointer(logiCvcInstance[0], 1);
    pLayer[2] = logiCVC_GetLayerPointer(logiCvcInstance[0], 2);
    pLayer[3] = logiCVC_GetLayerPointer(logiCvcInstance[0], 3);
    //pLayer[4] = logiCVC_GetLayerPointer(logiCvcInstance[0], 4);

    /* Set background color, if last layer is employed for it */
    notUseBg = logiCVC_GetCommonParam(logiCvcInstance[0], CVC_USE_BACKGROUND_E) ? 0 : 1;
    if(notUseBg == 0)
    {
        /* This function always accepts RGB_888 format */
    	// byte0: bits7:6 = Blue
    	// byte1: bits7:5 = Green
    	// byte2: bits7:5 = Red
        logiCVC_SetBackgroundColor(logiCvcInstance[0], 0x660066); /* dark purple */
    }

	switch(selection)
	{
		case SYS_STREAM0_FULL:
			// Layer_0: Full Screen (DVI_IN_0 or TPG_1)
			//logiCVC_ChangeBuffer(pLayer[0], 0);
		    logiCVC_SetMemoryOffset(pLayer[0], 0, 0);
        	logiCVC_SetExternalBufferSwitchingCtrl(pLayer[0], TRUE);
			logiCVC_SetOutputPosition(pLayer[0], &r_full);
			#if 0
			while(1){
				if(get_currentFrmaeNumber_VDMA(selection) == FRAME_TO_TRIGGER)
					break;
			}
			#endif
			logiCVC_LayerEnDis(pLayer[0], TRUE);
			break;

		case SYS_STREAM1_FULL:
			// Layer_2: Full Screen (DVI_IN_1 or TPG_3)
			//logiCVC_ChangeBuffer(pLayer[2], 0);
		    logiCVC_SetMemoryOffset(pLayer[2], 0, 0);
        	logiCVC_SetExternalBufferSwitchingCtrl(pLayer[2], TRUE);
            logiCVC_SetOutputPosition(pLayer[2], &r_full);
			#if 0
            while(1){
				if(get_currentFrmaeNumber_VDMA(selection) == FRAME_TO_TRIGGER)
					break;
			}
			#endif
            logiCVC_LayerEnDis(pLayer[2], TRUE);
			break;

		case SYS_STREAM0_SCALED:
			// Layer_0: Scaled Screen (DVI_IN_0 or TPG_1)
			//logiCVC_ChangeBuffer(pLayer[0], 0);
		    logiCVC_SetMemoryOffset(pLayer[0], 0, 0);
        	logiCVC_SetExternalBufferSwitchingCtrl(pLayer[0], TRUE);
            logiCVC_SetOutputPosition(pLayer[0], &r_scl_0);
            //xil_printf("wait for frame 2, %d \r\n", selection);
			#if 0
            while(1){
				if(get_currentFrmaeNumber_VDMA(selection) == FRAME_TO_TRIGGER)
					break;
			}
			#endif
            logiCVC_LayerEnDis(pLayer[0], TRUE);
			break;

		case SYS_STREAM1_SCALED:
			// Layer_2: Scaled Screen (DVI_IN_1 or TPG_3)
			//logiCVC_ChangeBuffer(pLayer[2], 0);
		    logiCVC_SetMemoryOffset(pLayer[2], 0, 0);
        	logiCVC_SetExternalBufferSwitchingCtrl(pLayer[2], TRUE);
            logiCVC_SetOutputPosition(pLayer[2], &r_scl_3);
            //xil_printf("wait for frame 2, %d \r\n", selection);
			#if 0
            while(1){
				if(get_currentFrmaeNumber_VDMA(selection) == FRAME_TO_TRIGGER)
					break;
			}
			#endif
            logiCVC_LayerEnDis(pLayer[2], TRUE);
			break;

		case SYS_TPG0_SCALED:
			// Layer_1: Scaled Screen (always TPG_0)
			//logiCVC_ChangeBuffer(pLayer[1], 0);
		    logiCVC_SetMemoryOffset(pLayer[1], 0, 0);
        	logiCVC_SetExternalBufferSwitchingCtrl(pLayer[1], TRUE);
            logiCVC_SetOutputPosition(pLayer[1], &r_scl_1);
            //xil_printf("wait for frame 2, %d \r\n", selection);
			#if 0
            while(1){
				if(get_currentFrmaeNumber_VDMA(selection) == FRAME_TO_TRIGGER)
					break;
			}
			#endif
            logiCVC_LayerEnDis(pLayer[1], TRUE);
			break;

		case SYS_TPG2_SCALED:
			// Layer_3: Scaled Screen (always TPG_2)
			//logiCVC_ChangeBuffer(pLayer[3], 0);
		    logiCVC_SetMemoryOffset(pLayer[3], 0, 0);
        	logiCVC_SetExternalBufferSwitchingCtrl(pLayer[3], TRUE);
            logiCVC_SetOutputPosition(pLayer[3], &r_scl_2);
            //xil_printf("wait for frame 2, %d \r\n", selection);
			#if 0
            while(1){
				if(get_currentFrmaeNumber_VDMA(selection) == FRAME_TO_TRIGGER)
					break;
			}
			#endif
            logiCVC_LayerEnDis(pLayer[3], TRUE);
			break;
	}
	// Enable background
	//logiCVC_LayerEnDis(pLayer[4], TRUE);

	return 0;
}

/****************************************************************************
*
* setup the logiCVC layers as per the current selection of video streams
*
* @return   XST_SUCCESS
*
* @note
*	 *******************************************************************************
*	 *     Screen Layout:                                                          *
*	 *                +----------------+----------------+                          *
*	 *                |                |                |                          *
*	 *                |   DVI_IN_0     |      TPG_0     |                          *
*	 *                |    TPG_1       |                |                          *
*	 *                +----------------+----------------+                          *
*	 *                |                |                |                          *
*	 *                |    TPG_2       |    DVI_IN_1    |                          *
*	 *                |                |     TPG_3      |                          *
*	 *                +----------------+----------------+                          *
*	 *     		STREAM0 => Layer0 : DVI_IN_0 or TPG_1 (full or scaled, top left)   *
*	 *     		STREAM1 => Layer2 : DVI_IN_1 or TPG_3 (full or scaled, bot right)  *
*	 *     		TPG_0   => Layer1 : TPG_0 (always scaled, top right)               *
*	 *     		TPG_2   => Layer3 : TPG_2 (always scaled, bot left)                *
*	 *     		           LAYER4 : Background                                     *
*	 *******************************************************************************
*
*****************************************************************************/
XStatus repeat_patterns_logiCVC(int vsel, int psel, int ssel)
{
	int layer;
	int player = 0xf;	// default All Layers
    XRECT r_scl_0 = {0,    0,  960,  540};	// 1920/2, 1080/2	DVI_IN_0
    XRECT r_scl_1 = {960,  0, 1920,  540};	// 1920/2, 1080/2	TPG_1
    XRECT r_scl_2 = {0,  540,  960, 1080};	// 1920/2, 1080/2	TPG_2
    XRECT r_scl_3 = {960,540, 1920, 1080};	// 1920/2, 1080/2	DVI_IN_1
    logiCvcLayerPointerT pLayer[5];

	/*******************************************************************
	 * Step 1: get the handles for individual layers
	 * *****************************************************************/
    pLayer[0] = logiCVC_GetLayerPointer(logiCvcInstance[0], 0);
    pLayer[1] = logiCVC_GetLayerPointer(logiCvcInstance[0], 1);
    pLayer[2] = logiCVC_GetLayerPointer(logiCvcInstance[0], 2);
    pLayer[3] = logiCVC_GetLayerPointer(logiCvcInstance[0], 3);

	if(psel==0)						// no patterns are asked for
		return 0;

	/*******************************************************************
	 * Step 2: select the layers that are to be applied with patterns
	 * *****************************************************************/
	if( (new_vsel == WS_STREAM0_FULL) ||
		((new_vsel == WS_SCALED_STREAMS) && (ssel == WS_STREAM0_SCALED)) )
		player = 0;

	if( (new_vsel == WS_STREAM1_FULL) ||
		((new_vsel == WS_SCALED_STREAMS) && (ssel == WS_STREAM1_SCALED)) )
		player = 2;

	if((new_vsel == WS_SCALED_STREAMS) && (ssel == WS_TPG0_SCALED) )
		player = 1;

	if((new_vsel == WS_SCALED_STREAMS) && (ssel == WS_TPG2_SCALED) )
		player = 3;

	/************************************************************************
	 * Step 3: select the patterns that are to be applied on selected layers
	 * 'psel' is the variable contains combination of patterns as follows
	 * 		bit0 = 1 : alpha blend
	 * 		bit1 = 1 : move around
	 * **********************************************************************/
	if( (psel&0x3) == 0x3)	// move around + alpha blend
	{
		/* Move shrunk layers from screen edge to center */
		if(vsel == WS_SCALED_STREAMS)
			moveLayers(logiCvcInstance[0]);
		//shrinkLayers(logiCvcInstance[0]);

		//if(psel & 0x8)
		{
			/* Reduce alpha blending to zero for each layer */
			/* avoid last layer (there is nothing to show below it) */
			//maxlayers = cvc->NumLayers;
			if(player == 0xf)
			{
				for (layer = 0; layer < 4; layer++)
				{
					blendDownLayer(logiCvcInstance[0], layer);
					if(new_psel==0) return 0;
				}
			}
			else
			{
				blendDownLayer(logiCvcInstance[0], player);
				if(new_psel==0) return 0;
			}
			OsMsDelay(SHORT_WAIT);

			/* Return full opacity to all layers */
			if(player == 0xf)
			{
				for (layer = 3; layer >= 0; layer--)
				{
					//blendDownLayerStep(cvc, layer, 255);
					blendUpLayer(logiCvcInstance[0], layer);
					if(new_psel==0) return 0;
				}
			}
			else
			{
				blendUpLayer(logiCvcInstance[0], player);
				if(new_psel==0) return 0;
			}
			OsMsDelay(SHORT_WAIT);
		}
	}
	else
	{
		if( (psel&0x1) == 1)	// alpha blend
		{
			if(player == 0xf)
			{
				for (layer = 0; layer < 4; layer++)
				{
					blendDownLayer(logiCvcInstance[0], layer);
					if(new_psel==0) return 0;
				}
			}
			else
			{
				blendDownLayer(logiCvcInstance[0], player);
				if(new_psel==0) return 0;
			}
			OsMsDelay(SHORT_WAIT);

			/* Return full opacity to all layers */
			if(player == 0xf)
			{
				for (layer = 0; layer < 4; layer++)
				{
					//blendDownLayerStep(cvc, layer, 255);
					blendUpLayer(logiCvcInstance[0], layer);
					if(new_psel==0) return 0;
				}
			}
			else
			{
				blendUpLayer(logiCvcInstance[0], player);
				if(new_psel==0) return 0;
			}
			OsMsDelay(SHORT_WAIT);
		}

		if( (psel&0x2) == 2)	// move around
		{
			/* Move shrunk layers from screen edge to center */
			if(vsel == WS_SCALED_STREAMS)
				moveLayers(logiCvcInstance[0]);
		}
	}
	if(new_psel==0) return 0;

	if(player == 0xf)
	{
        logiCVC_SetOutputPosition(pLayer[0], &r_scl_0);
        logiCVC_SetOutputPosition(pLayer[2], &r_scl_3);
        logiCVC_SetOutputPosition(pLayer[1], &r_scl_1);
        logiCVC_SetOutputPosition(pLayer[3], &r_scl_2);
	}
	return 0;
}

#if 0
cvcRetT logiCVCDemo(logiCvcPointerT *logiCvcInstance)
{
    int id, i, layer, layers, notUseBg;
    cvcRetT status = cvcRetSuccessE;
    logiCvcLayerPointerT pLayer;
    logiCvcBufferPointerT pBuffer;

    APP_PRINT("\n\rlogiCVC demo run:\n\r");
    APP_PRINT("************************************************\n\r");
    APP_PRINT("This demo is intended for mulitple CVC layers\n\r");
    APP_PRINT("demonstrating visual interaction between layers.\n\r");
    APP_PRINT("If current IP Core configuration defines only 1 layer,\n\r");
    APP_PRINT("multiple-layer examples will not be applicable.\n\r\n\r");

    /* Run demo for each display instance */
    for(id = 0; id < XPAR_LOGICVC_NUM_INSTANCES; id++)
    {
        APP_PRINT("Demo for logiCVC_%d\n\r", id);

        layers = logiCVC_GetCommonParam(logiCvcInstance[id], CVC_NUM_LAYERS_E);
        notUseBg =
            logiCVC_GetCommonParam(logiCvcInstance[id], CVC_USE_BACKGROUND_E) ? 0 : 1;

        /* Set background color, if last layer is employed for it */
        if(notUseBg == 0)
        {
            /* This function always accepts RGB_888 format */
            logiCVC_SetBackgroundColor(logiCvcInstance[id], 0x660066); /* dark purple */
        }

        /* Fill each layer with different color scale */
        /* (if last layer is used for background, cannot write to it) */
        for (layer = 0; layer < layers; layer++)
            colorScaleOnLayer(logiCvcInstance[id], layer);

        /* Decrease each layer size */
        shrinkLayers(logiCvcInstance[id]);

        /* Set transparent color for each layer */
        /* avoid last layer (there is nothing to show below it) */
        for (layer = 0; layer < layers - notUseBg; layer++)
            setLayerTransparentColor(logiCvcInstance[id], layer);

        OsMsDelay(SHORT_WAIT);

        /* Move shrunk layers from screen edge to center */
        moveLayers(logiCvcInstance[id]);

        /* Reduce alpha blending to zero for each layer */
        /* avoid last layer (there is nothing to show below it) */
        for (layer = 0; layer < layers - notUseBg; layer++)
            blendDownLayer(logiCvcInstance[id], layer);

        OsMsDelay(SHORT_WAIT);

        /* Return full opacity to all layers */
        for (layer = 0; layer < layers - notUseBg; layer++)
            blendDownLayerStep(logiCvcInstance[id], layer, 255);

        OsMsDelay(SHORT_WAIT);

        /* Change memory offset for all layers */
        /* (if last layer is used for background, cannot move it) */
        for (layer = 0; layer < layers; layer++)
            changeMemoryOffset(logiCvcInstance[id], layer);

        /* Show how HW cursor (e.g. mouse pointer) could be simulated */
        cursorDemo(logiCvcInstance[id]);

        /* Re-expand all layers to full screen and clean them */
        for (layer = 0; layer < layers - notUseBg; layer++)
        {
            XRECT full = {0, 0, logiCVC_GetCommonParam(logiCvcInstance[id], CVC_HRES_E), logiCVC_GetCommonParam(logiCvcInstance[id], CVC_VRES_E)};
            OsU32 bkgd = getRawColor(logiCvcInstance[id], layer, ColorPerLayer[layer] & 0xff666666);
            pLayer = logiCVC_GetLayerPointer(logiCvcInstance[id], layer);
            logiCVC_SetOutputPosition(pLayer, &full);
            pBuffer = logiCVC_GetBufferPointer(pLayer, 0);
            logiCVC_FillRectangle(pBuffer, &full, bkgd);
        }

        /* Show demo with single and double buffering for each layer */
        /* (if last layer is used for background, cannot use it) */
        for (layer = 0; layer < layers; layer++)
        {
            bufferingDemo(logiCvcInstance[id], layer);
            /* Disable current layer, so that layer below is visible */
            pLayer = logiCVC_GetLayerPointer(logiCvcInstance[id], layer);
            logiCVC_LayerEnDis(pLayer, FALSE);
        }

        /* Re-enable all layers */
        for (layer = 0; layer < layers; layer++)
        {
            pLayer = logiCVC_GetLayerPointer(logiCvcInstance[id], layer);
            logiCVC_LayerEnDis(pLayer, TRUE);
        }
    }

    APP_PRINT("\n\rlogiCVC demo done (status %d)\n\r", status);

    return status;
}

#endif

/**
*
* Displays color scale on specified layer.
*
* @param    pLogiCvc is a pointer to the logiCVC instance.
* @param    layer is a layer number.
*
* @return   None.
*
* @note     Displays color scale using vertical bars. Starts with full color
*           on left side that is getting dimmer after each step to the right.
*           For better distinction between layers, each layer uses different
*           starting color (see ColorPerLayer[] table) and layers with
*           less bits per pixel (bpp) have smaller steps.
*           After that, several horizontal lines are drawn in full color;
*           lines are thicker for each subsequent layer.
*           Finally, at 10% of screen size from top left corner, a number
*           of short white vertical lines is displayed; top layer - one line,
*           second layer - two lines, etc.
*
*****************************************************************************/
void colorScaleOnLayer(logiCvcPointerT pLogiCvc, int layer)
{
    int x, y, width, step, c;
    OsU32 mask = 0xffffffff;
    OsU32 color = ColorPerLayer[layer];
    XRECT rect;
    logiCvcLayerPointerT pLayer   = logiCVC_GetLayerPointer(pLogiCvc, layer);
    logiCvcBufferPointerT pBuffer = logiCVC_GetBufferPointer(pLayer, 0);

    APP_PRINT("Color scale on layer %d...\n\r", layer);

    rect.x_pos = 0;
    rect.y_pos = 0;
    rect.width = 1;
    rect.height = logiCVC_GetCommonParam(pLogiCvc, CVC_VRES_E);

    width = logiCVC_GetCommonParam(pLogiCvc, CVC_HRES_E);

    /* Change scale faster for layers with less bpp */
    step = logiCVC_GetLayerParam(pLayer, CVC_LAYER_COLOR_BITS_E) / 8;

    /* Draw color scale using vertical rectangles */
    for (x=0; x<width; x++)
    {
        c = color & mask;
        c = getRawColor(pLogiCvc, layer, c);

        rect.x_pos = x;

        logiCVC_FillRectangle(pBuffer, &rect, c);

        /* "mask" decreases color strength after each "step" number of pixels */
        if ((x+1) % step == 0)
        {
            if (mask == 0xff000000) mask = 0xffffffff;
            else                    mask -= 0x010101;
        }
    }

    /* Draw few horizontal lines in full color.
     * Use different y-offset and thickness for each layer */
    c = getRawColor(pLogiCvc, layer, color);
    rect.x_pos = 0;
    rect.width = width;
    rect.height = 3 + layer * 3;
    for (y=0; y<5; y++)
    {
        rect.y_pos = 2 + y * 32 + layer * 3;
        logiCVC_FillRectangle(pBuffer, &rect, c);
    }

    /* Add small white pattern near the top left corner of layer */
    /* Draw 1 vertical line for 1st layer, 2 for 2nd layer, etc. */
    c = getRawColor(pLogiCvc, layer, WHITE_24B);
    rect.x_pos = logiCVC_GetCommonParam(pLogiCvc, CVC_HRES_E) / 10 - 8;
    rect.y_pos = logiCVC_GetCommonParam(pLogiCvc, CVC_VRES_E) / 10 - 8;
    rect.width = 5 - layer;
    rect.height = 16;
    for (x=0; x<=layer; x++)
    {
        logiCVC_FillRectangle(pBuffer, &rect, c);
        rect.x_pos += rect.width+2;
    }
}

/**
*
* Reduces output rectangle of all layers, one by one.
*
* @param    pLogiCvc is a pointer to the logiCVC instance.
*
* @return   None.
*
* @note     First layer is reduced to 1/5 of display size (both dimensions).
*           Next layer is a bit bigger ( + 1/8 of display size), etc.
*           Last layer is not reduced, as there is nothing to show below
*           (output would be undefined). Each layer is moved to a different
*           position on display, like this:
*              ------------------------------
*              | 333         22         444 |
*              |                        444 |
*              | 1                          |
*              |                            |
*              |                            |
*              ------------------------------
*           With defined ANIMATION_ON, layers are moved from starting position
*           and size (fullscreen) to final position/size in animated manner.
*           Otherwise they are just set to the final state.
*
*****************************************************************************/
void shrinkLayers(logiCvcPointerT pLogiCvc)
{
	//int i, j, k;

    /* Shrink each layer to different size */
    /* Top layer will be the smallest (displays size / 5) */
    int hRes = logiCVC_GetCommonParam(pLogiCvc, CVC_HRES_E);
    int vRes = logiCVC_GetCommonParam(pLogiCvc, CVC_VRES_E);
    int width = hRes / 5;
    /* Next layer will be a bit bigger */
    int width_diff = hRes / 8;
    /* Prepare dimensions for all 5 possible layers */
    XRECT rect[5];
    int layer, layers;
    int notUseBg = logiCVC_GetCommonParam(pLogiCvc, CVC_USE_BACKGROUND_E) ? 0 : 1;

    APP_PRINT("Shrinking layer sizes...\n\r");

    layers = logiCVC_GetCommonParam(pLogiCvc, CVC_NUM_LAYERS_E);
    /* Do not shrink last layer (nothing to show below it) */
    for (layer = 0; layer < layers; layer++, width += width_diff)
    {
        XRECT *prect = &rect[layer];
        prect->width = width;
        /* Calculate height that keeps the aspect ratio */
        prect->height = vRes * width / hRes;

        switch (layer)
        {
            case 0:
                /* Place top layer on left-middle position on screen */
                prect->x_pos = 0;
                prect->y_pos = (vRes - prect->height) / 2;
                break;
            case 1:
                /* Place 2nd layer on top-middle position on screen */
                prect->x_pos = (hRes - width) / 2;
                prect->y_pos = 0;
                break;
            case 2:
                /* Place 3rd layer on top-left position on screen */
                prect->x_pos = 0;
                prect->y_pos = 0;
                break;
            case 3:
                /* Place 4th layer on top-right position on screen */
                prect->x_pos = hRes - width;
                prect->y_pos = 0;
                break;
        }
    }

    /* Shrink all layers step by step, starting with top layer */
    /* Do not shrink last layer (nothing to show below it) */
    for (layer = 0; layer < layers - notUseBg; layer++)
    {
        logiCvcLayerPointerT pLayer = logiCVC_GetLayerPointer(pLogiCvc, layer);

        /* This is the final layer position and size */
        XRECT *prect = &rect[layer];
#ifndef ANIMATION_ON
        /* Just set layers to final positions/sizes */
        logiCVC_SetOutputPosition(pLayer, prect);
        OsMsDelay(ANIMATION_DURATION);
#else
        int num_steps = hRes - prect->width;
        int step;
        XRECT r2;
        /* Set step delay so that total animation length is 5 seconds */
        float delay = ANIMATION_DURATION / num_steps;

        for (step = 1; step <= num_steps; step++)
        {
            /* Calculate layer size and position for each step */
            /* starting with fullscreen, ending with *prect */
            r2.width = hRes - (hRes - prect->width) * step / num_steps;
            r2.height = vRes - (vRes - prect->height) * step / num_steps;
            r2.x_pos = prect->x_pos * step / num_steps;
            r2.y_pos = prect->y_pos * step / num_steps;

            logiCVC_SetOutputPosition(pLayer, &r2);

            OsMsDelay(delay);
        }
#endif /* ANIMATION_ON */
    }
}

/**
*
* Moves all reduced layers to the center of display.
*
* @param    pLogiCvc is a pointer to the logiCVC instance.
*
* @return   None.
*
* @note     This demo should be executed after shrinkLayers(), because it
*           first reads current output position for each layer from logiCVC.
*           Layers are all moved concurrently from their original positions to
*           the center of display, demonstrating their relative z-order
*           appearance.
*           With defined ANIMATION_ON, layers are moved in animated manner.
*           Otherwise they are just set to the final position.
*
*****************************************************************************/
void moveLayers(logiCvcPointerT pLogiCvc)
{
    /* Prepare dimensions for all 4 possible layers */
    XRECT rect[5];

    float delay;
    int step, num_steps, layer;
    int hRes = logiCVC_GetCommonParam(pLogiCvc, CVC_HRES_E);
    int vRes = logiCVC_GetCommonParam(pLogiCvc, CVC_VRES_E);
    int layers = logiCVC_GetCommonParam(pLogiCvc, CVC_NUM_LAYERS_E);
    int notUseBg = logiCVC_GetCommonParam(pLogiCvc, CVC_USE_BACKGROUND_E) ? 0 : 1;

    APP_PRINT("Moving layers around...\n\r");

    /* Set final positions in the middle of display */
    for (layer = 0; layer < layers - notUseBg; layer++)
    {
        logiCvcLayerPointerT pLayer = logiCVC_GetLayerPointer(pLogiCvc, layer);
        rect[layer].x_pos = logiCVC_GetLayerParam(pLayer, CVC_LAYER_GET_OUTPUT_POSITION_X_E);
        rect[layer].y_pos = logiCVC_GetLayerParam(pLayer, CVC_LAYER_GET_OUTPUT_POSITION_Y_E);
        rect[layer].width = logiCVC_GetLayerParam(pLayer, CVC_LAYER_GET_OUTPUT_POSITION_WIDTH_E);
        rect[layer].height = logiCVC_GetLayerParam(pLayer, CVC_LAYER_GET_OUTPUT_POSITION_HEIGHT_E);

        rect[layer].x_pos = (hRes - rect[layer].width) / 2;
        rect[layer].y_pos = (vRes - rect[layer].height) / 2;
		//sys_sleep(100);		// milli seconds
    }

#ifndef ANIMATION_ON
    /* Just set layers to final positions */
    for (layer = 0; layer < layers - notUseBg; layer++)
    {
        logiCvcLayerPointerT pLayer = logiCVC_GetLayerPointer(pLogiCvc, layer);
        logiCVC_SetOutputPosition(pLayer, &rect[layer]);
    }
    OsMsDelay(ANIMATION_DURATION);
#else

    num_steps = rect[0].x_pos;
    delay = ANIMATION_DURATION / num_steps;

    /* Move layers in steps */
    for (step = 1; step <= num_steps; step++)
    {
        /* Do not move last layer (nothing to show below it) */
        for (layer = 0; layer < layers - notUseBg; layer++)
        {
            logiCvcLayerPointerT pLayer = logiCVC_GetLayerPointer(pLogiCvc, layer);
            XRECT r2 = rect[layer];
            switch (layer)
            {
            case 0:
                /* Move top layer right */
                r2.x_pos = step;
                break;

            case 1:
                /* Move 2nd layer down */
                r2.y_pos = rect[1].y_pos * step / num_steps;
                break;

            case 2:
                /* Move 3nd layer down+right */
                r2.x_pos = rect[2].x_pos * step / num_steps;
                r2.y_pos = rect[2].y_pos * step / num_steps;
                break;

            case 3:
                /* Move 4th layer down+left */
                r2.x_pos = hRes - r2.width - rect[3].x_pos * step / num_steps;
                r2.y_pos = rect[3].y_pos * step / num_steps;
                break;
            }
            logiCVC_SetOutputPosition(pLayer, &r2);
            if(new_psel==0) return;
        }
        OsMsDelay(delay);
    }
#endif /* ANIMATION_ON */
}

/**
*
* Blends down layer from full opaque to full transparent state.
*
* @param    pLogiCvc   Pointer to the logiCVC instance.
* @param    layer Layer number.
*
* @return   None.
*
* @note     Blends down layer by reducing its alpha channel from 255 to 0.
*           With defined ANIMATION_ON, layers are blend down in animated manner
*           using all values from 255 to 0.
*           Otherwise they are blend down in 3 steps.
*
*****************************************************************************/
void blendDownLayer(logiCvcPointerT pLogiCvc, int layer)
{
    float delay;
    int a;

    APP_PRINT("Blending down layer %d...\n\r", layer);

#ifndef ANIMATION_ON
    /* Just 3 example steps: alpha on 2/3, 1/3 and 0 */
    blendDownLayerStep(pLogiCvc, layer, 255 * 2 / 3);
    OsMsDelay(ANIMATION_DURATION / 3);

    blendDownLayerStep(pLogiCvc, layer, 255 / 3);
    OsMsDelay(ANIMATION_DURATION / 3);

    blendDownLayerStep(pLogiCvc, layer, 0);
    OsMsDelay(ANIMATION_DURATION / 3);
#else
    /* Set delay so that we step thru max alpha in 5 seconds */
    delay = ANIMATION_DURATION / 256;

    /* Animate alpha blending in steps down to zero */
    for (a = 255; a >= 0; a--)
    {
        blendDownLayerStep(pLogiCvc, layer, a);
		if(new_psel==0)
		{
			for(a=0; a<4; a++)
			{
				// exit, put back normal blend
				logiCVC_SetLayerAlphaValue(
						(logiCVC_GetLayerPointer(logiCvcInstance[0], a)), 255);
				//sys_sleep(100);		// milli seconds
			}
			return;
		}
        OsMsDelay(delay);
    }
#endif /* ANIMATION_ON */
}

void blendUpLayer(logiCvcPointerT pLogiCvc, int layer)
{
    int a;
    float delay;

    xil_printf("Blending up layer %d...\n\r", layer);

#ifndef ANIMATION_ON
    /* Just 3 example steps: alpha on 2/3, 1/3 and 0 */
    blendDownLayerStep(cvc, layer, 255 * 2 / 3);
    OsMsDelay(ANIMATION_DURATION / 3);

    blendDownLayerStep(cvc, layer, 255 / 3);
    OsMsDelay(ANIMATION_DURATION / 3);

    blendDownLayerStep(cvc, layer, 0);
    OsMsDelay(ANIMATION_DURATION / 3);
#else
    /* Set delay so that we step thru max alpha in 5 seconds */
    delay = ANIMATION_DURATION / 256;

    /* Animate alpha blending in steps down to zero */
    for (a = 0; a <= 255; a++)
    {
        blendDownLayerStep(pLogiCvc, layer, a);
		if(new_psel==0)
		{
			for(a=0; a<4; a++)
			{
				// exit, put back normal blend
				logiCVC_SetLayerAlphaValue(
						(logiCVC_GetLayerPointer(logiCvcInstance[0], a)), 255);
				//sys_sleep(100);		// milli seconds
			}
			return;
		}
        OsMsDelay(delay);
    }
#endif /* ANIMATION_ON */
}
/**
*
* Sets alpha blend value for a layer.
*
* @param    pLogiCvc   Pointer to the logiCVC instance.
* @param    layer Layer number.
* @param    alpha Alpha value to be set.
*
* @return   None.
*
* @note     Alpha value is set accoring to available alpha mode for the layer.
*           - LAYER alpha mode: alpha is set for entire layer.
*           - PIXEL alpha mode: alpha is set for all pixels in top-left 50x50
*             rectangle, while other pixels remain opaque.
*           - CLUT alpha mode, alpha is set for all colors in lookup table
*
*****************************************************************************/
void blendDownLayerStep(logiCvcPointerT pLogiCvc, int layer, OsU8 alpha)
{
    OsU32 maxAlpha;
    OsU32 alphaMask;
    OsU32 alphaBitPos;
    int i,x,y;
    logiCvcLayerPointerT pLayer = logiCVC_GetLayerPointer(pLogiCvc, layer);
    OsU32 bitsPerPix = logiCVC_GetLayerParam(pLayer, CVC_LAYER_COLOR_BITS_E);
    OsU32 alphaMode = logiCVC_GetLayerParam(pLayer, CVC_LAYER_ALPHA_MODE_E);
    logiCvcBufferPointerT pBuffer;
    logiCvcClutPointerT pClut;

    /* Only one blending mode is possible in each layer */
    /* Perform blending based on available mode */
    switch (alphaMode)
    {
    case LOGICVC_ALPHA_MODE_LAYER:
        /* Blending equally entire layer */
        /* Input alpha is ranged 0..255, so it needs to be adapted */
        logiCVC_GetMaxLayerAlphaValue(pLayer, &maxAlpha);
        if (maxAlpha == 7)
            alpha >>= 5;
        else if (maxAlpha == 63)
            alpha >>= 2;

        logiCVC_SetLayerAlphaValue(pLayer, alpha);
        break;

    case LOGICVC_ALPHA_MODE_PIXEL:
        /* Each pixel has its own blending value */

        /* Pixel alpha position and size depend on bpp */
        if (bitsPerPix == 8)
        {
            alphaMask = ALPHA_8B;
            alphaBitPos = 8;
        }
        else if (bitsPerPix == 16)
        {
            alphaMask = ALPHA_16B;
            alphaBitPos = 24;
        }
        else /* in case of 24bpp */
        {
            alphaMask = ALPHA_24B;
            alphaBitPos = 24;
        }

        /* In this demo we blend down only top-left 50x50 square */
        pBuffer = logiCVC_GetBufferPointer(pLayer, 0);
        for(y=0; y<50; y++)
            for(x=0; x<50; x++)
            {
                OsU32 pixel = logiCVC_GetPixel(pBuffer, x, y);
                pixel = (pixel & ~alphaMask) | (alpha << alphaBitPos);
                logiCVC_DrawPixel(pBuffer, x, y, pixel);
            }
        break;

    case LOGICVC_ALPHA_MODE_CLUT_INTERNAL_ARGB_6565:
        /* Input alpha is ranged 0..255, so it needs to be adapted */
        /* In this case, maxAlpha is 63; */
        alpha >>= 2;
        /* (no break here) */

    case LOGICVC_ALPHA_MODE_CLUT_INTERNAL_ARGB_8888:
        /* Each color from CLUT has its own blending value */
        /* In this demo we blend down all colors */
        /* CLUT might be unreadable, so recalculate values written into CLUT */
        pClut = logiCVC_GetClutPointer(pLayer, 0);
        for(i=0; i<256; i++)
        {
            OsU32 clut = (alpha << 24) | ((i & 0xe0)<<16) | ((i & 0x1c)<<11) | ((i & 3)<<6);
            logiCVC_ClutWrite(pClut, i, clut);
        }
        break;
    }
}

/**
*
* Sets transparent color for a layer.
*
* @param    pLogiCvc is a pointer to the logiCVC instance.
* @param    layer Layer number.
*
* @return   None.
*
* @note     Reads pixel from layer coordinates (40,0) and sets transparent
*           color according to obtained color - making entire vertical bar at
*           x=40 invisible, so that layers below can be seen through.
*           With defined ANIMATION_ON, this demo first runs over all pixels in
*           top row (x,0) setting the transparent color to each read pixel.
*
*****************************************************************************/
void setLayerTransparentColor(logiCvcPointerT pLogiCvc, int layer)
{
#ifdef ANIMATION_ON
    int x;
    OsU32 color;
    logiCvcLayerPointerT pLayer = logiCVC_GetLayerPointer(pLogiCvc, layer);
    OsU32 bitsPerPix = logiCVC_GetLayerParam(pLayer, CVC_LAYER_COLOR_BITS_E);
    OsU32 outputRectWidth = logiCVC_GetLayerParam(pLayer, CVC_LAYER_GET_OUTPUT_POSITION_WIDTH_E);
    logiCvcBufferPointerT pBuffer = logiCVC_GetBufferPointer(pLayer, 0);
    float delay = ANIMATION_DURATION / outputRectWidth;

    APP_PRINT("Setting transparent color for layer %d...\n\r", layer);

    /* We will set transparent color to color of pixels */
    for (x = 0; x < outputRectWidth; x++)
    {
        color = logiCVC_GetPixel(pBuffer, x, 0);

        /* To get the color from 8bpp layer, we need to read it from CLUT, but CLUT
           might be unreadable, so we recalculate the value written into CLUT */
        if (bitsPerPix == 8)
            color = ALPHA_24B | ((color & 0xe0)<<16) | ((color & 0x1c)<<11) | ((color & 3)<<6);

        logiCVC_SetTransparentColor(pLayer, color);

        OsMsDelay(delay);
    }
#endif /* ANIMATION_ON */

    /* We will set transparent color to color of pixel at position (40,0) */
    color = logiCVC_GetPixel(pBuffer, 40, 0);

    APP_PRINT("Setting transparent color for layer %d...\n\r", layer);

    /* To get the color from 8bpp layer, we need to read it from CLUT, but CLUT
       might be unreadable, so we recalculate the value written into CLUT */
    if (bitsPerPix == 8)
        color = ALPHA_24B | ((color & 0xe0)<<16) | ((color & 0x1c)<<11) | ((color & 3)<<6);

    logiCVC_SetTransparentColor(pLayer, color);

#ifndef ANIMATION_ON
    OsMsDelay(ANIMATION_DURATION);
#endif
}

/**
*
* Moves memory offset of the layer.
*
* @param    pLogiCvc   Pointer to the logiCVC instance.
* @param    layer Layer number.
*
* @return   None.
*
* @note     Memory offset of moved to starting points (50,100), then (100,100),
*           (100,50), and then back to (0,0).
*           With defined ANIMATION_ON, memory offset is moved between these
*           points in animated manner.
*
*****************************************************************************/
void changeMemoryOffset(logiCvcPointerT pLogiCvc, int layer)
{
    int x,y;
    logiCvcLayerPointerT pLayer = logiCVC_GetLayerPointer(pLogiCvc, layer);

#ifndef ANIMATION_ON
    APP_PRINT("Moving layer %d memory offset...\n\r", layer);

    /* Move offset to (50,100) */
    logiCVC_SetMemoryOffset(pLayer, 50, 100);
    OsMsDelay(ANIMATION_DURATION / 4);

    /* Move offset to (50,100) */
    logiCVC_SetMemoryOffset(pLayer, 100, 100);
    OsMsDelay(ANIMATION_DURATION / 4);

    /* Move offset to (50,100) */
    logiCVC_SetMemoryOffset(pLayer, 100, 50);
    OsMsDelay(ANIMATION_DURATION / 4);

    /* Move offset to (50,100) */
    logiCVC_SetMemoryOffset(pLayer, 0, 0);
    OsMsDelay(ANIMATION_DURATION / 4);
#else

    float delay = ANIMATION_DURATION / 200;

    APP_PRINT("Moving layer %d memory offset...\n\r", layer);

    /* Move offset stepwise to (50,100) */
    for(x=1,y=2; x<50; x++,y+=2)
    {
        logiCVC_SetMemoryOffset(pLayer, x, y);
        OsMsDelay(delay);
    }
    /* Move offset stepwise to (100,100) */
    for(; x<100; x++)
    {
        logiCVC_SetMemoryOffset(pLayer, x, y);
        OsMsDelay(delay);
    }
    /* Move offset stepwise to (100,50) */
    for(; y>50; y--)
    {
        logiCVC_SetMemoryOffset(pLayer, x, y);
        OsMsDelay(delay);
    }
    /* Move offset stepwise to (0,0) */
    for(; x>=0; x-=2,y--)
    {
        logiCVC_SetMemoryOffset(pLayer, x, y);
        OsMsDelay(delay);
    }
#endif /* ANIMATION_ON */
}

/**
*
* Demonstrates possibility to use top layer as mouse cursor.
*
* @param    pLogiCvc   Pointer to the logiCVC instance.
*
* @return   None.
*
* @note     Entire layer 0 is filled with transparent color, except shape of
*           the arrow in top left corner. Cursor is moved by moving the
*           output position of top layer. This demo moves cursor around the
*           rectangle from 1/4 to 3/4 of display dimensions, moving faster
*           each cycle.
*
*****************************************************************************/
void cursorDemo(logiCvcPointerT pLogiCvc)
{
    /* HW cursor will be simulated by asigning top layer for it */

    /* Here is a nice arrow shape for this demo */
    static char CursorShape[16][16] = {
        "XXXXXXXXXX______",
        "XXXXXXXXXXX_____",
        "XXXXX___________",
        "XXXXXX__________",
        "XXXXXXX_________",
        "XX_XXXXX________",
        "XX__XXXXX_______",
        "XX___XXXXX______",
        "XX____XXXXX_____",
        "XX_____XXXXX____",
        "_X______XXXXX___",
        "_________XXXXX__",
        "__________XXXXX_",
        "___________XXXXX",
        "____________XXXX",
        "_____________XXX",
    };

    int x,y,step;
    int x0,x1,y0,y1;
    OsU32 savedTranspColor;
    logiCvcLayerPointerT pLayer = logiCVC_GetLayerPointer(pLogiCvc, 0);
    OsU32 bitsPerPix = logiCVC_GetLayerParam(pLayer, CVC_LAYER_COLOR_BITS_E);
    OsU32 hRes = logiCVC_GetCommonParam(pLogiCvc, CVC_HRES_E);
    OsU32 vRes = logiCVC_GetCommonParam(pLogiCvc, CVC_VRES_E);
    logiCvcBufferPointerT pBuffer = logiCVC_GetBufferPointer(pLayer, 0);
    logiCvcClutPointerT pClut = logiCVC_GetClutPointer(pLayer, 0);

    /* Set size of layer 0 to only 16x16 pixels */
    XRECT rect = {0,0,16,16};

    /* In LAYER alpha mode this demo cannot be utilized */
    if (logiCVC_GetLayerParam(pLayer, CVC_LAYER_ALPHA_MODE_E) == LOGICVC_ALPHA_MODE_LAYER)
    {
        return;
    }

    APP_PRINT("Running HW cursor simulation demo...\n\r");

    logiCVC_SetOutputPosition(pLayer, &rect);

    OsU32 raw_white = getRawColor(pLogiCvc, 0, WHITE_24B);

    if (bitsPerPix == 8)
    {
       /* Set CLUT only for black and white; black will have alpha=0 */
       logiCVC_ClutWrite(pClut, raw_white, WHITE_24B);
       logiCVC_ClutWrite(pClut, 0, 0);
    }
    /* Set black (0) to be transparent color */
    savedTranspColor = logiCVC_GetLayerParam(pLayer, CVC_LAYER_GET_TRANSPARENT_COLOR_E);
    logiCVC_SetTransparentColor(pLayer, 0);

    /* Draw arrow shape to top layer top-left corner */
    for (x=0; x<16; x++)
    {
        for (y=0; y<16; y++)
        {
            if (CursorShape[y][x] == '_')
                logiCVC_DrawPixel(pBuffer, x, y, 0);
            else
                logiCVC_DrawPixel(pBuffer, x, y, raw_white);
        }
    }

    /* Do some animation - cursor moves around square path from
       1/4 to 3/4 of display dimensions */
    step = 1;
    x0 = hRes / 4;
    x1 = x0 + hRes / 2;
    y0 = vRes / 4;
    y1 = y0 + vRes / 2;

    rect.x_pos = x0;
    rect.y_pos = y0;

    /* Make 100 cycles around the square path, each time faster by 1 */
    for (step=1; step<=100; step++)
    {
        for(; rect.x_pos<x1; rect.x_pos+=step)
        {
            logiCVC_SetOutputPosition(pLayer, &rect);
            OsMsDelay(1);
        }
        rect.x_pos = x1;

        for(; rect.y_pos<y1; rect.y_pos+=step)
        {
            logiCVC_SetOutputPosition(pLayer, &rect);
            OsMsDelay(1);
        }
        rect.y_pos = y1;

        for(; rect.x_pos>x0; rect.x_pos-=step)
        {
            logiCVC_SetOutputPosition(pLayer, &rect);
            OsMsDelay(1);
        }
        rect.x_pos = x0;

        for(; rect.y_pos>y0; rect.y_pos-=step)
        {
            logiCVC_SetOutputPosition(pLayer, &rect);
            OsMsDelay(1);
        }
        rect.y_pos = y0;
    }

    /* Return default CLUT and transparent color settings */
    if (bitsPerPix == 8)
    {
       logiCVC_ClutWrite(pClut, 0, BLACK_24B);
    }
    logiCVC_SetTransparentColor(pLayer, savedTranspColor);
}

/**
*
* Demonstrates benefit of double buffering when screen needs to be redrawn.
*
* @param    pLogiCvc   Pointer to the logiCVC instance.
* @param    layer Layer number.
*
* @return   None.
*
* @note     Small (100,50) rectangle is moved over the screen, but each time
*           screen is redrawn, it is visible as a flicker. With double buffer,
*           new screen is always drawn to invisible buffer, and visual display
*           is switched to that buffer only after the drawing is finished.
*
*****************************************************************************/
void bufferingDemo(logiCvcPointerT pLogiCvc, int layer)
{
    int b=1;  /* This number defines how many buffers are used */
    OsU32 tick, totalTicks;
    OsU32 color = getRawColor(pLogiCvc, layer, MED_GRAY_24B); // grey
    OsU32 bkgd = getRawColor(pLogiCvc, layer, ColorPerLayer[layer] & 0xff666666);
    OsSizeT timer;
    OsU32 hRes = logiCVC_GetCommonParam(pLogiCvc, CVC_HRES_E);
    OsU32 vRes = logiCVC_GetCommonParam(pLogiCvc, CVC_VRES_E);
    XRECT full = {0, 0, hRes, vRes};
    logiCvcLayerPointerT pLayer = logiCVC_GetLayerPointer(pLogiCvc, layer);
    OsSizeT bufNum = 0;
    logiCvcBufferPointerT pBuffer = logiCVC_GetBufferPointer(pLayer, bufNum);

    /* Show single and double buffer example */
    for (b=1; b<=2; b++)
    {
        if (b == 2)
        {
            /* Check is double-buffering properly supported by display and
               video memory resolution */
            if (logiCVC_GetLayerParam(pLayer, CVC_LAYER_MAX_VRES_USE_BUFFERING_E) < vRes)
            {
                APP_PRINT("Display VRES bigger than video memory VRES.\n\r");
                APP_PRINT("Double buffering demo skipped.\n\r");
                return;
            }
            /* When starting double buffering, first switch drawing pen
               to buffer that is currently not visible */
            bufNum = 1;
            pBuffer = logiCVC_GetBufferPointer(pLayer, bufNum);
        }

        timer = OsTimeGetElapsedMs(0);
        totalTicks = 5000;

        /* Move some solid color rectangle over the display */
        XRECT rect = {0, 0, 100, 50};
        while ((tick = OsTimeGetElapsedMs(timer)) < totalTicks)
        {
            logiCVC_FillRectangle(pBuffer, &full, bkgd);

            rect.y_pos = tick * (vRes - rect.height) / totalTicks;
            rect.x_pos = rect.y_pos;
            logiCVC_FillRectangle(pBuffer, &rect, color);

            if (b == 2)
            {
                /* This operation will switch visible buffer and set drawing pen
                   back to buffer that is currently not visible */
                logiCVC_ChangeBuffer(pLayer, bufNum);
                logiCVC_Sync(pLogiCvc, TRUE);
                bufNum = !bufNum;
                pBuffer = logiCVC_GetBufferPointer(pLayer, bufNum);
            }
        }
    }

    /* Return drawing pointer to visible buffer (back to single buffering) */
    logiCVC_ChangeBuffer(pLayer, 0);
    logiCVC_Sync(pLogiCvc, TRUE);
}

/**
*
* Returns raw color based on layer bit per pixel configuration.
*
* @param    pLogiCvc is a pointer to logiCVC instance.
* @param    layer Layer number.
* @param    ARGBColor is a true color with alpha channel.
*
* @return   Raw color to be used on specified layer.
*
* @note     In 8-bit layers, output value does not match the CLUT color,
*           but assumes default RGB_332 palette.
*           Alpha channel is always present in output value, disregarding
*           alpha mode of the layer.
*
*****************************************************************************/
OsU32 getRawColor(logiCvcPointerT pLogiCvc, int layer, OsU32 ARGBColor)
{
    logiCvcLayerPointerT pLayer = logiCVC_GetLayerPointer(pLogiCvc, layer);
    OsU32 bitsPerPix = logiCVC_GetLayerParam(pLayer, CVC_LAYER_COLOR_BITS_E);

    if (bitsPerPix == 8)
        return ((ARGBColor >> 21) & 0x700) |  /* Alpha value */
               ((ARGBColor >> 16) & 0xe0) |   /* Red value */
               ((ARGBColor >> 11) & 0x1c) |   /* Green value */
               ((ARGBColor >> 6) & 0x03);     /* Blue value */
    else if (bitsPerPix == 16)
        return ((ARGBColor >> 2) & 0x3f000000) | /* Alpha value */
               ((ARGBColor >> 8) & 0xf800) |     /* Red value */
               ((ARGBColor >> 5) & 0x07e0) |     /* Green value */
               ((ARGBColor >> 3) & 0x001f);      /* Blue value */

    return ARGBColor;
}

/**
*
* Returns true color from raw color based on layer bit per pixel.
*
* @param    pLogiCvc is a pointer to logiCVC instance.
* @param    layer Layer number.
* @param    rawColor Raw color for the specified layer.
*
* @return   True color.
*
* @note     In 8-bit layers, output value is read from CLUT based on supplied
*           raw value.
*           Alpha channel is either expanded from input, read from CLUT,
*           or from layer alpha value in case if LAYER alpha mode is used.
*
*****************************************************************************/
OsU32 getTrueColor(logiCvcPointerT pLogiCvc, int layer, OsU32 rawColor)
{
    OsU32 color=0;
    logiCvcLayerPointerT pLayer = logiCVC_GetLayerPointer(pLogiCvc, layer);
    OsU32 bitsPerPix = logiCVC_GetLayerParam(pLayer, CVC_LAYER_COLOR_BITS_E);
    OsU32 bytePerPix = logiCVC_GetLayerParam(pLayer, CVC_LAYER_BYTE_PER_PIX_E);
    OsU32 alphaMode = logiCVC_GetLayerParam(pLayer, CVC_LAYER_ALPHA_MODE_E);
    logiCvcClutPointerT pClut = logiCVC_GetClutPointer(pLayer, 0);

    if (bitsPerPix == 8)
    {
        color = logiCVC_ClutRead(pClut, (rawColor & 0xff));
        if (bytePerPix == 2)
            /* Expand 3-bit Alpha to 8-bit (ABC -> ABCABCAB) */
            color |= ((rawColor & 0x700) << 21) | ((rawColor & 0x700) << 18) | ((rawColor & 0x600) << 15);

        /* set CLUT color to rawColor in case of AGRB_6565 CLUT */
        rawColor = color;
    }

    if (bitsPerPix == 16 || alphaMode == LOGICVC_ALPHA_MODE_CLUT_INTERNAL_ARGB_6565)
    {
        color = ((rawColor & 0xf800) << 8) | ((rawColor & 0xe000) << 3) | /* expand 5-bit Red to 8-bit */
                ((rawColor & 0x07e0) << 5) | ((rawColor & 0x0600) >> 1) | /* expand 6-bit Green to 8-bit */
                ((rawColor & 0x001f) << 3) | ((rawColor & 0x001c) >> 2);  /* expand 5-bit Blue to 8-bit */
        if (bytePerPix == 4)
            /* Expand 6-bit Alpha to 8-bit */
            color |= ((rawColor & 0x3f000000) << 2) | ((rawColor & 0x30000000) >> 4);
    }

    /* Set full opacity if layer alpha mode is selected (at any bpp) */
    if (alphaMode == LOGICVC_ALPHA_MODE_LAYER)
        color |= ALPHA_24B;

    return color;
}

/* @} */

