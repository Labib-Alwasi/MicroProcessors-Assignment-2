#include "hardware/regs/addressmap.h"
#include "hardware/regs/io_bank0.h"
#include "hardware/regs/timer.h"
#include "hardware/regs/m0plus.h"

.syntax unified                                                 @ Specify unified assembly syntax
.cpu    cortex-m0plus                                           @ Specify CPU type is Cortex M0+
.thumb                                                          @ Specify thumb assembly for RP2040
.global main_asm                                                @ Provide program starting address to the linker
.align 4                                                        @ Specify code alignment


.equ    GPIO_BTN, 21                                            @ GPIO pin for button
.equ    GPIO_LED, 25                                            @ GPIO pin for LED
.equ    GPIO_DIR_IN, 0                                          @ GPIO pin direction input
.equ    GPIO_DIR_OUT, 1                                         @ GPIO pin direction output

.equ    GPIO_LED_ON, 1                                          @ GPIO pin value high
.equ    GPIO_LED_OFF, 0                                         @ GPIO pin value low

.equ    GPIO_BTN_DN_MSK, 0x00400000                             @ For detecting falling-edge event on GP21
.equ    GPIO_BTN_UP_MSK, 0x00800000                             @ For detecting rising-edge event on GP21


.equ    GPIO_ISR_OFFSET, 0x74                                   @ GPIO is IRQ #13
.equ    ALARM_ISR_OFFSET, 0x40                                  @ ALARM0/TIMER0 is IRQ #0

.equ    INIT_STATE, 0                                           @ Initial state of the timer
.equ    TIMER_PERIOD, 2000000                                   @ Timer period set to 2 seconds

@ Entry point to the ASM portion of the program
main_asm:
    push    {lr}
    bl      init_leds                                           @ Call init_leds() to initialise the LED
    bl      init_gpio                                           @ Call init_gpio() to initialise the GPIO and set falling edge interrupt
    bl      gpio_isr_installer                                  @ Call gpio_isr_installer() to install the GPIO interrupt handler
    bl      alarm_isr_installer                                 @ Call alarm_isr_installer() to install the ALARM interrupt handler
    pop     {pc}

@ Waits for button press, gets the current time, gets the duration of button press, if duration is greater than 1 second, branches to dash, else dot and loops
loop:
    bl      set_alarm                                           @ Loop to reset the alarm
    wfi                                                         @ Wait for interrupt
    wfi
    ldr     r2, =timer                                          @ Check if timer has run out
    ldr     r1, [r2]                                            @ Load value into r1
    movs    r0, #1                                              
    cmp     r0, r1                                              @ Check if Timer has run out (2 seconds have passed since)
    bne     loop                                                @ If not, loop back to check again 
    movs    r0, #0                                              
    str     r0, [r2]                                            @ Reset timer           
    movs    r0, #3                                              
    movs    r1, #0
    bl      set_input                                           @ Send End of transmission signal
    pop     {pc}


init_leds:
    push    {lr}
    movs    r0, #GPIO_LED                                       @ Set GPIO pin for LED
    bl      asm_gpio_init                                       @ Call asm_gpio_init() to initialise the GPIO pin
    movs    r0, #GPIO_LED
    movs    r1, #GPIO_DIR_OUT                                   @ Set GPIO pin direction to output
    bl      asm_gpio_set_dir                                    @ Call asm_gpio_set_dir() to set the GPIO pin direction
    pop     {pc}

init_gpio:
    push    {lr}
    movs    r0, GPIO_BTN                                        @ Set GPIO pin for button
    bl      asm_gpio_init                                       @ Call asm_gpio_init() to initialise the GPIO pin
    movs    r0, GPIO_BTN
    movs    r1, GPIO_DIR_IN                                     @ Set GPIO pin direction to input
    bl      asm_gpio_set_irq                                    @ Set falling edge interrupt for GPIO pin                                    
    movs    r0, GPIO_BTN
    bl      asm_gpio_set_dir                                    @ Call asm_gpio_set_dir() to set the GPIO pin direction
    pop     {pc}

set_alarm:
    push    {lr}
    ldr     r2,=TIMER_BASE                                      @ Get the TIMER_BASE register address
    movs    r1, #1                                              
    str     r1, [r2, #TIMER_INTE_OFFSET]                        @ Enable the timer interrupt
    ldr     r1, [r2, #TIMER_TIMELR_OFFSET]                      @ Get the lower Time registers
    ldr     r3, =endtimer                                       
    ldr     r0, [r3]                                            @ Load the value of endtimer into r0
    add     r1, r0                                              @ Update current alarm interval value
    str     r1, [r2, #TIMER_ALARM0_OFFSET]                      @ Set the new alarm
    ldr     r2, =TIMER_BASE                       
    movs    r1, #1                                              @ Write 1 to r0
    str     r1, [r2, #TIMER_INTE_OFFSET]                                            @ Store 1 in timer
    pop     {pc}                                         


alarm_isr_installer:
    ldr     r3, =(PPB_BASE + M0PLUS_VTOR_OFFSET)                @ Get the address of the RAM vector table using the (PPb_BASE + M0PLUS_VTOR_OFFSET)
    ldr     r1, [r3]
    movs    r3, #ALARM_ISR_OFFSET                               @ Set the offset of the alarm interrupt handler
    adds    r3, r1                                              @ Add the offset to the vector table address
    ldr     r0, =alarm_isr                                      
    str     r0, [r2]
    movs    r0, #1
    ldr     r1, =(PPB_BASE + M0PLUS_NVIC_ICPR_OFFSET)           @ Get the address of the NVIC ICPR register and add to base address
    str     r0, [r1]                                            @ Disable the ALARM0 IRQ
    ldr     r1, =(PPB_BASE + M0PLUS_NVIC_ISER_OFFSET)           @ Adds the base address of the CPU to NVIC ISER offset address
    str     r0, [r1]                                            @ Enable the ALARM0 IRQ
    bx      lr                                                  @ Branch and exchange the last instruction

.thumb_func
alarm_isr:
    push    {lr}                          
    ldr     r2, =TIMER_BASE                                     @ Get the TIMER_BASE address
    movs    r1, #1                                              @ 1 is the appropriate value to set the alarm
    str     r1, [r2, #TIMER_INTR_OFFSET]                        @ Reset the alarm 
    ldr     r2, =timer                                          @ Get addrress of timer variable
    movs    r0, #1                                              @ Write 1 to r0
    str     r0, [r2]                                            @ Store 1 in timer
    pop     {pc}                          

gpio_isr_installer:
    ldr     r1, =(PPB_BASE + M0PLUS_VTOR_OFFSET)                @ Get the address of the RAM vector table using the (PPB_BASE + M0PLUS_VTOR_OFFSET) register
    ldr     r2, [r1]
    movs    r1, #GPIO_ISR_OFFSET                              
    add     r1, r2                                             
    ldr     r0, =gpio_isr
    str     r0, [r1]                                            @ Store the address of the GPIO interrupt handler in the vector table
    @Disable the GPIO interrupt using the (PPB_BASE + M0PLUS_NVIC_ICPR_OFFSET) register
    ldr     r1, =(PPB_BASE + M0PLUS_NVIC_ICPR_OFFSET)
    ldr     r0, [r1]
    @Enable the GPIO interrupt using the (PPB_BASE + M0PLUS_NVIC_ISER_OFFSET) register
    ldr     r1, =(PPB_BASE + M0PLUS_NVIC_ISER_OFFSET)
    ldr     r0, [r1]
    bx      lr                                                  @ Return from interrupt

.thumb_func
gpio_isr:
    @ Read the interrupt status event from the (IO_BANK0_BASE + IO_BANK0_PROC0_INTS2_OFFSET) register
    push    {lr}
    
    ldr     r2, =(IO_BANK0_BASE + IO_BANK0_INTR2_OFFSET)  @ Get the address of the interrupt status register
    ldr     r1, [r2]                                            @ Read the interrupt status event
    ldr     r0, =GPIO_BTN_DN_MSK                                @ Set Mask for falling edge interrupt
    cmp     r0, r1                                              @ Compare the interrupt status event with the mask for falling edge
    beq     btn_pressed
    ldr     r0, =GPIO_BTN_UP_MSK                                @ Set Mask for rising edge interrupt
    cmp     r0, r1                                              @ Compare the interrupt status event with the mask for rising edge
    beq     btn_released

    pop {pc}

set_timer:
    bl      start_timer                                         @ C function to detect current time
    pop     {pc}

btn_pressed:
    ldr     r2, =(IO_BANK0_BASE + IO_BANK0_INTR2_OFFSET)        @ Get the address of the interrupt status register
    ldr     r1, =GPIO_BTN_DN_MSK                                @ Set Mask for falling edge interrupt
    str     r1, [r2]                                            @ Reset the interrupt status event
    bl      end_timer                                           @ C function to calculate the time difference
    movs    r1, #1                                              @ Set the time difference to 1 second
    cmp     r0, r1                                              @ Check if a second has passed
    bgt     add_space                                           @ If a second has passed, go to add_space
    b       dot                                                 @ Else go to dot

btn_released:
    ldr     r2, =(IO_BANK0_INTR2_OFFSET)                        @ Get the address of the interrupt status register
    ldr     r1, =GPIO_BTN_UP_MSK                                @ Set Mask for rising edge interrupt
    str     r1, [r2]                                            @ Reset the interrupt status event
    bl      end_timer                                           @ C function to calculate the time difference
    movs    r1, #1                                              @ Set the time difference to 1 second
    cmp     r0, r1                                              @ Check if a second has passed
    bgt     dash                                                @ If a second has passed, go to dash
    b       set_timer                                           @ Else go to set_timer and finish interrupt

dot:
    movs    r0, #0                                              
    @ movs    r1, #0
    bl      set_input                                           @ Call C function to add dot to the input string
    b       set_timer                                           @ Go to set_timer and finish interrupt

dash:
    movs    r0, #1
    @ movs    r1, #1
    bl      set_input                                           @ Call C function to add dash to the input string
    b       set_timer                                           @ Go to set_timer and finish interrupt

add_space:
    movs    r0, #2
    @ movs    r1, #0
    bl      set_input                                           @ Call C function to add space to the input string
    b       set_timer                                           @ Go to set_timer and finish interrupt

@ Set data alignment
.data
    .align 4
    timer:         .word   INIT_STATE @ Timer has not been started
    endtimer:      .word   TIMER_PERIOD @ Stores the time difference