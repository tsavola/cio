/*
 * Copyright (c) 2010  Timo Savola
 */

	.globl	cio_start
	.type	cio_start, @function

	.globl	cio_cleanup_stack
	.type	cio_cleanup_stack, @function

	.globl	cio_cleanup
	.type	cio_cleanup, @function

	.text

/**
 * Call a function with a custom stack and clean up after it returns.
 *
 * @param %rdi  argument
 * @param %rsi  function
 * @param %rdx  stack
 */
cio_start:
	movq	%rdx, %rsp	# use custom stack
	call	*%rsi		# call routine with argument as first parameter

	call	cio_cleanup_stack
	movq	%rsp, %rdi	# make stack the first parameter
	movq	%rax, %rsp	# use cleanup stack
	jmp	cio_cleanup
