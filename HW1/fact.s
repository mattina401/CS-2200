!============================================================
! CS-2200 Homework 1
! name: Jongyeon Kim
! GTID#: 903018469
!
! Please do not change main's functionality, 
! except to change the argument for factorial or to meet your 
! calling convention
!============================================================

main:		la $sp, stack		! load address of stack label into $sp
    	                                ! FIXME: load desired value of the stack 
                                        ! (defined at the label below) into $sp
        	la $at, factorial	! load address of factorial label into $at
                addi $a0, $zero, 5 	! $a0 = 5, the number to factorialize
                jalr $at, $ra		! jump to factorial, set $ra to return addr
                halt			! when we return, just halt

factorial:	lw $sp, 0($sp)
		addi $sp, $sp, -2	! move up stack pointer by 2	
		sw $a0, 1($sp)		! store value on stack
		sw $ra, 0($sp)		! store return address on stack
		addi $sp, $sp, -1	! move up stack pointer by 1	
		sw $fp, 0($sp)		! store old frame ponter on stack
		addi $fp, $sp, -1	! frame pointer points upper from old frame pointer
		beq $a0, $zero, todown	! if $a0 is 0 go to todown
		addi $a0, $a0, -1	! decrease $a0 by 1
		la $at, factorial	! load address of factorial label into $at
		jalr $at, $ra		! jump to factorial, set $ra to return addr
		beq $zero, $zero, keepon ! go to keepon
	
todown:		addi $v0, $zero, 1	! add 1 to $v0
		beq $zero, $zero, down	! go to down

keepon:		addi $sp, $fp, 1	! stack pointer points old frame pointer
		lw $a0, 2($sp)		! load value by stack pointer
		addi $a1, $v0, 0	! $a0 to return value
		addi $v0, $zero, 0	! clear return value
		la $at, multiple	! load address of multiple label into $at
		jalr $at, $ra		! jump to multiple, set $ra to return addr

down:	 	lw $fp, 0($sp)		! frame pointer points to old frame point
		lw $ra, 1($sp)		! load return address
		jalr $ra, $t0		! jump to return address
	
multiple:	add $v0, $v0, $a0	! $v0 = $v0 + $a0
		addi $a1, $a1, -1	! decrease $a1 by 1
		beq $zero, $a1, finish	! if $a1 is 0, go to finish
		beq $zero, $zero, multiple ! go to multiple	

finish: 	jalr $ra, $t0		! finish
	   

stack:	        .word 0x4000		! the stack begins here (for example, that is)

