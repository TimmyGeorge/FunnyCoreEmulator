Usage: `otg.exe program [-D'|-D] [r1] [r2] [r3] [r4] [r5] [r6] [r7] [r8] [r9] [r10] [r11] [r12] [r13] [r14] [r15]`
  - `program`
      - the name of file containing the program to run
  - `D'`
      - specifies the program is not the division program in our coursework.
      - use this to run any program you want
      - by default, this flag is specified
  - `D`
      - specifies the program is the division program in our coursework
      - the emulator will run automated tests over all the possible values for the divisor and dividend registers
      - no more command line arguments should be specified
      - by default, this flag is not specified
  - `rn`
      - a integer constant (either hexadecimal or decimal) specifying the initial value of register `n` in the emulator
  
