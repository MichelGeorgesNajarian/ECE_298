################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
driverlib/MSP430F5xx_6xx/%.obj: ../driverlib/MSP430F5xx_6xx/%.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccs901/ccs/tools/compiler/ti-cgt-msp430_18.12.1.LTS/bin/cl430" -vmspx --data_model=restricted --use_hw_mpy=F5 --include_path="C:/ti/ccs901/ccs/ccs_base/msp430/include" --include_path="C:/Users/migeo/OneDrive - University of Waterloo/2B/ECE298/bike_radar/timer_a_ex1_pwmSingle" --include_path="C:/Users/migeo/OneDrive - University of Waterloo/2B/ECE298/bike_radar/timer_a_ex1_pwmSingle/driverlib/MSP430F5xx_6xx" --include_path="C:/ti/ccs901/ccs/tools/compiler/ti-cgt-msp430_18.12.1.LTS/include" --advice:power="none" --define=__MSP430F5529__ --define=DEPRECATED -g --printf_support=minimal --diag_warning=225 --diag_wrap=off --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --preproc_with_compile --preproc_dependency="driverlib/MSP430F5xx_6xx/$(basename $(<F)).d_raw" --obj_directory="driverlib/MSP430F5xx_6xx" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

