
 PARAMETER VERSION = 2.2.0


BEGIN OS
 PARAMETER OS_NAME = xilkernel
 PARAMETER OS_VER = 5.01.a
 PARAMETER PROC_INSTANCE = microblaze_subsystem_microblaze_0
 PARAMETER STDIN = microblaze_subsystem_RS232_Uart_1
 PARAMETER STDOUT = microblaze_subsystem_RS232_Uart_1
 PARAMETER SYSTMR_SPEC = true
 PARAMETER SYSTMR_DEV = microblaze_subsystem_Dual_Timer_Counter
 PARAMETER SYSINTC_SPEC = microblaze_subsystem_Interrupt_Cntlr
 PARAMETER MAX_PTHREADS = 40
 PARAMETER PTHREAD_STACK_SIZE = 32768
 PARAMETER CONFIG_PTHREAD_MUTEX = true
 PARAMETER MAX_PTHREAD_MUTEX = 20
 PARAMETER MAX_PTHREAD_MUTEX_WAITQ = 20
 PARAMETER MAX_READYQ = 20
 PARAMETER CONFIG_TIME = true
 PARAMETER MAX_TMRS = 20
 PARAMETER CONFIG_SEMA = true
 PARAMETER MAX_SEM = 20
 PARAMETER MAX_SEM_WAITQ = 20
 PARAMETER ENHANCED_FEATURES = true
 PARAMETER CONFIG_YIELD = true
 PARAMETER STATIC_PTHREAD_TABLE = ((main_thread,1))
END


BEGIN PROCESSOR
 PARAMETER DRIVER_NAME = cpu
 PARAMETER DRIVER_VER = 1.15.a
 PARAMETER HW_INSTANCE = microblaze_subsystem_microblaze_0
 PARAMETER EXTRA_COMPILER_FLAGS = -g -ffunction-sections -fdata-sections -DXILKERNEL_MB_MPU_DISABLE
END


BEGIN DRIVER
 PARAMETER DRIVER_NAME = mig_7series
 PARAMETER DRIVER_VER = 1.00.a
 PARAMETER HW_INSTANCE = DDR_Interface_DDR3_SDRAM
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = axipmon
 PARAMETER DRIVER_VER = 4.00.a
 PARAMETER HW_INSTANCE = DDR_Interface_axi_perf_mon_1
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = gpio
 PARAMETER DRIVER_VER = 3.01.a
 PARAMETER HW_INSTANCE = Display_Controller_gpio_rst_mux
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = logiCVC
 PARAMETER DRIVER_VER = 3.05.a
 PARAMETER HW_INSTANCE = Display_Controller_logicvc_1
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = axivdma
 PARAMETER DRIVER_VER = 4.05.a
 PARAMETER HW_INSTANCE = inline_scaler_1_SCALER_1_VDMA
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = tpg
 PARAMETER DRIVER_VER = 1.00.a
 PARAMETER HW_INSTANCE = inline_scaler_1_vid_source_1
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = axivdma
 PARAMETER DRIVER_VER = 4.05.a
 PARAMETER HW_INSTANCE = inline_scaler_2_SCALER_3_VDMA
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = tpg
 PARAMETER DRIVER_VER = 1.00.a
 PARAMETER HW_INSTANCE = inline_scaler_2_vid_source_3
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = axivdma
 PARAMETER DRIVER_VER = 4.05.a
 PARAMETER HW_INSTANCE = mem_scaler_1_SCALER_0_VDMA
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = axivdma
 PARAMETER DRIVER_VER = 4.05.a
 PARAMETER HW_INSTANCE = mem_scaler_2_SCALER_2_VDMA
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = axidma
 PARAMETER DRIVER_VER = 7.02.a
 PARAMETER HW_INSTANCE = microblaze_subsystem_AXI_DMA_Ethernet
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = gpio
 PARAMETER DRIVER_VER = 3.01.a
 PARAMETER HW_INSTANCE = microblaze_subsystem_DIP_Switches_4Bits
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = tmrctr
 PARAMETER DRIVER_VER = 2.05.a
 PARAMETER HW_INSTANCE = microblaze_subsystem_Dual_Timer_Counter
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = iic
 PARAMETER DRIVER_VER = 2.07.a
 PARAMETER HW_INSTANCE = microblaze_subsystem_IIC_EEPROM
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = bram
 PARAMETER DRIVER_VER = 3.02.a
 PARAMETER HW_INSTANCE = microblaze_subsystem_Internal_BRAM
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = intc
 PARAMETER DRIVER_VER = 2.06.a
 PARAMETER HW_INSTANCE = microblaze_subsystem_Interrupt_Cntlr
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = gpio
 PARAMETER DRIVER_VER = 3.01.a
 PARAMETER HW_INSTANCE = microblaze_subsystem_LCD_GPIO
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = gpio
 PARAMETER DRIVER_VER = 3.01.a
 PARAMETER HW_INSTANCE = microblaze_subsystem_LEDs_8Bits
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = emc
 PARAMETER DRIVER_VER = 3.01.a
 PARAMETER HW_INSTANCE = microblaze_subsystem_Linear_Flash
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = gpio
 PARAMETER DRIVER_VER = 3.01.a
 PARAMETER HW_INSTANCE = microblaze_subsystem_Push_Buttons_5Bits
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = gpio
 PARAMETER DRIVER_VER = 3.01.a
 PARAMETER HW_INSTANCE = microblaze_subsystem_ROTARY_GPIO
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = uartns550
 PARAMETER DRIVER_VER = 2.01.a
 PARAMETER HW_INSTANCE = microblaze_subsystem_RS232_Uart_1
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = axiethernet
 PARAMETER DRIVER_VER = 3.02.a
 PARAMETER HW_INSTANCE = microblaze_subsystem_Soft_Ethernet_MAC_axi_ethernet_buffer
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = sysmon
 PARAMETER DRIVER_VER = 5.03.a
 PARAMETER HW_INSTANCE = microblaze_subsystem_axi_xadc_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = uartlite
 PARAMETER DRIVER_VER = 2.01.a
 PARAMETER HW_INSTANCE = microblaze_subsystem_debug_module
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER DRIVER_VER = 1.00.a
 PARAMETER HW_INSTANCE = microblaze_subsystem_logisdhc_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = bram
 PARAMETER DRIVER_VER = 3.02.a
 PARAMETER HW_INSTANCE = microblaze_subsystem_microblaze_0_local_memory_LocalMemory_Cntlr_D
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = bram
 PARAMETER DRIVER_VER = 3.02.a
 PARAMETER HW_INSTANCE = microblaze_subsystem_microblaze_0_local_memory_LocalMemory_Cntlr_I
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = vtc
 PARAMETER DRIVER_VER = 4.00.a
 PARAMETER HW_INSTANCE = timebase_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = axivdma
 PARAMETER DRIVER_VER = 4.05.a
 PARAMETER HW_INSTANCE = v_in_pipe_1_DVI_IN_0_VDMA
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = tpg
 PARAMETER DRIVER_VER = 1.00.a
 PARAMETER HW_INSTANCE = v_in_pipe_1_vid_source_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = axivdma
 PARAMETER DRIVER_VER = 4.05.a
 PARAMETER HW_INSTANCE = v_in_pipe_2_DVI_IN_1_VDMA
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = tpg
 PARAMETER DRIVER_VER = 1.00.a
 PARAMETER HW_INSTANCE = v_in_pipe_2_vid_source_2
END


BEGIN LIBRARY
 PARAMETER LIBRARY_NAME = lwip140
 PARAMETER LIBRARY_VER = 1.05.a
 PARAMETER PROC_INSTANCE = microblaze_subsystem_microblaze_0
 PARAMETER API_MODE = SOCKET_API
END

BEGIN LIBRARY
 PARAMETER LIBRARY_NAME = xilmfs
 PARAMETER LIBRARY_VER = 1.00.a
 PARAMETER PROC_INSTANCE = microblaze_subsystem_microblaze_0
 PARAMETER NUMBYTES = 300000
 PARAMETER BASE_ADDRESS = 0xB0000000
END

BEGIN LIBRARY
 PARAMETER LIBRARY_NAME = xyl_oslib
 PARAMETER LIBRARY_VER = 1.06.a
 PARAMETER PROC_INSTANCE = microblaze_subsystem_microblaze_0
END

