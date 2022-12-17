! Spring 2022 Revisions by Andrej Vrtanoski

! This program executes pow as a test program using the LC 22 calling convention
! Check your registers ($v0) and memory to see if it is consistent with this program

! vector table
vector0:
        .fill 0x00000000                        ! device ID 0
        .fill 0x00000000                        ! device ID 1
        .fill 0x00000000                        ! ...
        .fill 0x00000000
        .fill 0x00000000
        .fill 0x00000000
        .fill 0x00000000
        .fill 0x00000000                        ! device ID 7
        ! end vector table

main:	lea $sp, initsp                         ! initialize the stack pointer
        lw $sp, 0($sp)                          ! finish initialization

        lea $t0, vector0                        ! DONE: Install timer interrupt handler into vector table
        lea $t1, timer_handler
        sw  $t1, 0($t0)

        lea $t1, toaster_handler                ! DONE: Install toaster interrupt handler into vector table
        sw  $t1, 1($t0)

        lea $t0, minval
        lw $t0, 0($t0)
        addi $t1, $zero, 65534                  ! store 0000ffff into minval (to make comparisons easier)
        sw $t1, 0($t0)

        ei                                      ! Enable interrupts

        lea $a0, BASE                           ! load base for pow
        lw $a0, 0($a0)
        lea $a1, EXP                            ! load power for pow
        lw $a1, 0($a1)
        lea $at, POW                            ! load address of pow
        jalr $ra, $at                           ! run pow
        lea $a0, ANS                            ! load base for pow
        sw $v0, 0($a0)

        halt                                    ! stop the program here
        addi $v0, $zero, -1                     ! load a bad value on failure to halt

BASE:   .fill 2
EXP:    .fill 8
ANS:	.fill 0                                 ! should come out to 256 (BASE^EXP)

POW:    addi $sp, $sp, -1                       ! allocate space for old frame pointer
        sw $fp, 0($sp)

        addi $fp, $sp, 0                        ! set new frame pointer

        bgt $a1, $zero, BASECHK                 ! check if $a1 is zero
        br RET1                                 ! if the exponent is 0, return 1

BASECHK:bgt $a0, $zero, WORK                    ! if the base is 0, return 0
        br RET0

WORK:   addi $a1, $a1, -1                        ! decrement the power
        lea $at, POW                            ! load the address of POW
        addi $sp, $sp, -2                       ! push 2 slots onto the stack
        sw $ra, -1($fp)                         ! save RA to stack
        sw $a0, -2($fp)                         ! save arg 0 to stack
        jalr $ra, $at                           ! recursively call POW
        add $a1, $v0, $zero                     ! store return value in arg 1
        lw $a0, -2($fp)                         ! load the base into arg 0
        lea $at, MULT                           ! load the address of MULT
        jalr $ra, $at                           ! multiply arg 0 (base) and arg 1 (running product)
        lw $ra, -1($fp)                         ! load RA from the stack
        addi $sp, $sp, 2

        br FIN                                  ! unconditional branch to FIN

RET1:   add $v0, $zero, $zero                   ! return a value of 0
	addi $v0, $v0, 1                        ! increment and return 1
        br FIN                                  ! unconditional branch to FIN

RET0:   add $v0, $zero, $zero                   ! return a value of 0

FIN:	lw $fp, 0($fp)                          ! restore old frame pointer
        addi $sp, $sp, 1                        ! pop off the stack
        jalr $zero, $ra

MULT:   add $v0, $zero, $zero                   ! return value = 0
        addi $t0, $zero, 0                      ! sentinel = 0
AGAIN:  add $v0, $v0, $a0                       ! return value += argument0
        addi $t0, $t0, 1                        ! increment sentinel
        blt $t0, $a1, AGAIN                     ! while sentinel < argument, loop again
        jalr $zero, $ra                         ! return from mult

timer_handler:
        addi $sp, $sp, -1                       ! space for $k0
        sw $k0, 0($sp)                          ! save $k0
        ei                                      ! enable interrupt

        addi $sp, $sp, -1                       ! space for $t0
        sw $t0, 0($sp)                          ! save $t0
        addi $sp, $sp, -1                       ! space for $t1
        sw $t1, 0($sp)                          ! save $t1
        addi $sp, $sp, -1                       ! space for $t2
        sw $t2, 0($sp)                          ! save $t1

        lea $t0, ticks                          ! load ticks label
        lw $t0, 0($t0)                          ! $t0 = ticks address
        lw $t1, 0($t0)                          ! $t1 = val at address
        addi $t1, $t1, 1                        ! increment $t1
        sw $t1, 0($t0)                          ! new val at ticks address

        lw $t2, 0($sp)                          ! restore $t2
        addi $sp, $sp, 1                        ! move ptr
        lw $t1, 0($sp)                          ! restore $t1
        addi $sp, $sp, 1                        ! move ptr
        lw $t0, 0($sp)                          ! restore $t0
        addi $sp, $sp, 1                        ! move ptr

        di                                      ! disable interrupt
        lw $k0, 0($sp)                          ! restore $k0
        addi $sp, $sp, 1                        ! pop $k0
        reti                                    ! return


toaster_handler:
        ! retrieve the data from the device and check if it is a minimum or maximum value
        ! then calculate the difference between minimum and maximum value
        ! (hint: think about what ALU operations you could use to implement subract using 2s compliment)

        addi $sp, $sp, -1                       ! space for $k0
        sw $k0, 0($sp)                          ! save $k0
        ei

        addi $sp, $sp, -1                       ! space for $t0
        sw $t0, 0($sp)                          ! save $t0
        addi $sp, $sp, -1                       ! space for $t1
        sw $t1, 0($sp)                          ! save $t1
        addi $sp, $sp, -1                       ! space for $t2
        sw $t2, 0($sp)                          ! save $t2

        in $t0, 1                               ! get recent temp

        checkmax:
            lea $t1, maxval                     ! $t1 = maxval label
            lw $t1, 0($t1)                      ! $t1 = maxval address
            lw $t2, 0($t1)                      ! $t2 = maxval
            bgt $t0, $t2, setmax                ! if val > oldval then branch
            br checkmin                         ! branch to checkmin

        setmax:
            sw $t0, 0($t1)                      ! store $t0

        checkmin:
            lea $t1, minval                     ! $t1 = minval label
            lw $t1, 0($t1)                      ! $t1 = minval address
            lw $t2, 0($t1)                      ! $t2 = minval
            blt $t0, $t2, setmin                ! if val < minval then branch
            br diff                             ! branch to diff

        setmin:
            sw $t0, 0($t1)                      ! store $t0
           
        diff:
            lea $t0, maxval                     ! $t0 = maxval label
            lw $t0, 0($t0)                      ! $t0 = maxval address
            lw $t0, 0($t0)                      ! $t0 = maxval
            lea $t1, minval                     ! $t1 = minval label
            lw $t1, 0($t1)                      ! $t1 = minval address
            lw $t1, 0($t1)                      ! $t1 = minval
            lea $t2, range                      ! $t2 = range label
            lw $t2, 0($t2)                      ! $t2 = range address
            nand $t1, $t1, $t1                  ! nand $t1
            addi $t1, $t1, 1                    ! $t1 = -$t1
            add $t0, $t0, $t1                   ! $t0 = max - min
            sw $t0, 0($t2)                      ! store $t0


        lw $t2, 0($sp)                          ! restore $t2
        addi $sp, $sp, 1                        ! move ptr
        lw $t1, 0($sp)                          ! restore $t1
        addi $sp, $sp, 1                        ! move ptr
        lw $t0, 0($sp)                          ! restore $t0
        addi $sp, $sp, 1                        ! move ptr

        di                                      ! disable interrupt
        lw $k0, 0($sp)                          ! restore $k0
        addi $sp, $sp, 1                        ! pop $k0
        reti                                    ! return



initsp: .fill 0xA000
ticks:  .fill 0xFFFF
range:  .fill 0xFFFE
maxval: .fill 0xFFFD
minval: .fill 0xFFFC