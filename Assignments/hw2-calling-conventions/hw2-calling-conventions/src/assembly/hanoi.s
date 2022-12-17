!============================================================
! CS 2200 Homework 2 Part 2: Tower of Hanoi
!
! Apart from initializing the stack,
! please do not edit mains functionality.
!============================================================

main:
    add     $zero, $zero, $zero     ! TODO: Here, you need to get the address of the stack
    lea     $sp, stack              ! using the provided label to initialize the stack pointer.
                                    ! load the label address into $sp and in the next instruction,
    lw      $sp, 0($sp)             ! use $sp as base register to load the value (0xFFFF) into $sp.


    lea     $at, hanoi              ! loads address of hanoi label into $at

    lea     $a0, testNumDisks3      ! loads address of number into $a0
    lw      $a0, 0($a0)             ! loads value of number into $a0

    jalr    $ra, $at                ! jump to hanoi, set $ra to return addr
    halt                            ! when we return, just halt

hanoi:
    add     $zero, $zero, $zero     ! TODO: perform post-call portion of
                                    ! the calling convention. Make sure to
                                    ! save any registers you will be using!

    addi    $sp, $sp, -1            ! space for frame pointer
    sw      $fp, 0($sp)             ! save old frame pointer
    addi    $fp, $sp, 0             ! set new frame pointer

    add     $zero, $zero, $zero     ! TODO: Implement the following pseudocode in assembly:
                                    ! IF ($a0 == 1)
                                    !    GOTO base
                                    ! ELSE
                                    !    GOTO else

    add     $t0, $zero, $zero        ! load 1 into $t0
    addi    $t0, $t0, 1              
    bgt     $a0, $t0, else          ! n > 1, jump to else
    br      base                    ! n == 1, jump to base

else:
    add     $zero, $zero, $zero     ! TODO: perform recursion after decrementing
                                    ! the parameter by 1. Remember, $a0 holds the
                                    ! parameter value.

    addi    $a0, $a0, -1            ! decrement parameter by 1
    lea     $at, hanoi              ! call hanoi recursively
    addi    $sp, $sp, -1            ! allocate space for ra
    sw      $ra, -1($fp)            ! save ra
    jalr    $ra, $at                ! jump to hanoi, set $ra to return addr

    lw      $ra, -1($fp)            ! restore ra
    addi    $sp, $sp, 1             ! pop ra

    add     $zero, $zero, $zero     ! TODO: Implement the following pseudocode in assembly:
                                    ! $v0 = 2 * $v0 + 1
                                    ! RETURN $v0

    add     $v0, $v0, $v0           ! multiply by 2
    addi    $v0, $v0, 1             ! add 1
    br      teardown                ! return

base:
    add     $zero, $zero, $zero     ! TODO: Return 1
    addi    $v0, $zero, 1           ! load 1 into $v0
    br teardown                     ! teardown

teardown:
    add     $zero, $zero, $zero     ! TODO: perform pre-return portion
                                    ! of the calling convention

    lw      $fp, 0($fp)             ! restore frame pointer
    addi    $sp, $sp, 1             ! pop frame pointer
    jalr    $zero, $ra              ! return to caller



stack: .word 0xFFFF                 ! the stack begins here


! Words for testing \/

! 1
testNumDisks1:
    .word 0x0001

! 10
testNumDisks2:
    .word 0x000a

! 20
testNumDisks3:
    .word 0x0014
