; HW_REV == 2.0, TODO: there can be less NOPs, ICs are faster
SCLK .set 0
MOSI .set 1
MISO .set 2

.macro NOP
   MOV r23, r23
.endm

    .global adc_readwrite ; code performs with 16.66 MHz, ~ 2060 ns CS low
adc_readwrite:
    MOV r24, r14 ; Save input arg (CS pin)
    LDI r20, 16 ; Load Counter for outloop
    LDI r21, 18; Load Counter for inloop
    CLR r30, r30, r24 ; Set CS low
    LDI r14, 0 ; Clear return reg

adc_outloop_head:
    SUB r20, r20, 1 ; decrement shiftloop counter
    QBBC adc_mosi_clr, r15, r20
adc_mosi_set:
    SET r30, r30, SCLK ; Set SCLK high
    SET r30, r30, MOSI ; Set MOSI high
    JMP adc_outloop_tail
adc_mosi_clr:
    SET r30, r30, SCLK ; Set SCLK high
    CLR r30, r30, MOSI ; Set MOSI low
    NOP
adc_outloop_tail:
    NOP
    CLR r30, r30, SCLK ; Set SCLK low
    QBLT adc_outloop_head, r20, 0
    NOP
    CLR r30, r30, MOSI ; clear MOSI

adc_inloop_head:
    SET r30, r30, SCLK ; Set SCLK high
    SUB r21, r21, 1 ; decrement shiftloop counter
    NOP
    NOP
    CLR r30, r30, SCLK ; Set SCLK low
    QBBC adc_miso_clr, r31, MISO
adc_miso_set:
    SET r14, r14, r21
    QBLT adc_inloop_head, r21, 0
    JMP adc_end
adc_miso_clr:
    NOP
    QBLT adc_inloop_head, r21, 0

adc_end:
    SET r30, r30, r24 ; set CS high
    JMP r3.w2


    .global dac_write  ; code performs with 25 MHz, ~ 980 ns CS low
dac_write:
    LDI r20, 24 ; Load Counter for outloop
    CLR r30, r30, r14 ; Set CS low
    NOP

dac_loop_head:
    SUB r20, r20, 1 ; Decrement counter
    QBBS dac_mosi_set, r15, r20 ; If bit number [r20] is set in value [r15]
dac_mosi_clr:
    SET r30, r30, SCLK ; Set SCLK high
    CLR r30, r30, MOSI ; Set MOSI low
    JMP dac_loop_tail
dac_mosi_set:
    SET r30, r30, SCLK ; Set SCLK high
    SET r30, r30, MOSI ; Set MOSI high
    NOP
dac_loop_tail:
    NOP
    CLR r30, r30, SCLK ; Set SCLK low
    QBLT dac_loop_head, r20, 0

dac_end:
    NOP
    CLR r30, r30, MOSI ; clear MOSI
    SET r30, r30, r14 ; set CS high
    JMP r3.w2
