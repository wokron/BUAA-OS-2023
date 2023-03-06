#include <asm/asm.h>
#include <mmu.h>
#include <trap.h>

// clang-format off
.macro SAVE_ALL
.set noreorder
.set noat
	move    k0, sp
.set reorder
	bltz    sp, 1f
	li      sp, KSTACKTOP
.set noreorder
1:
	subu    sp, sp, TF_SIZE
	sw      k0, TF_REG29(sp)
	mfc0    k0, CP0_STATUS
	sw      k0, TF_STATUS(sp)
	mfc0    k0, CP0_CAUSE
	sw      k0, TF_CAUSE(sp)
	mfc0    k0, CP0_EPC
	sw      k0, TF_EPC(sp)
	mfc0    k0, CP0_BADVADDR
	sw      k0, TF_BADVADDR(sp)
	mfhi    k0
	sw      k0, TF_HI(sp)
	mflo    k0
	sw      k0, TF_LO(sp)
	sw      $0, TF_REG0(sp)
	sw      $1, TF_REG1(sp)
	sw      $2, TF_REG2(sp)
	sw      $3, TF_REG3(sp)
	sw      $4, TF_REG4(sp)
	sw      $5, TF_REG5(sp)
	sw      $6, TF_REG6(sp)
	sw      $7, TF_REG7(sp)
	sw      $8, TF_REG8(sp)
	sw      $9, TF_REG9(sp)
	sw      $10, TF_REG10(sp)
	sw      $11, TF_REG11(sp)
	sw      $12, TF_REG12(sp)
	sw      $13, TF_REG13(sp)
	sw      $14, TF_REG14(sp)
	sw      $15, TF_REG15(sp)
	sw      $16, TF_REG16(sp)
	sw      $17, TF_REG17(sp)
	sw      $18, TF_REG18(sp)
	sw      $19, TF_REG19(sp)
	sw      $20, TF_REG20(sp)
	sw      $21, TF_REG21(sp)
	sw      $22, TF_REG22(sp)
	sw      $23, TF_REG23(sp)
	sw      $24, TF_REG24(sp)
	sw      $25, TF_REG25(sp)
	sw      $26, TF_REG26(sp)
	sw      $27, TF_REG27(sp)
	sw      $28, TF_REG28(sp)
	sw      $30, TF_REG30(sp)
	sw      $31, TF_REG31(sp)
.set at
.set reorder
.endm
/*
 * Note that we restore the IE flags from stack. This means
 * that a modified IE mask will be nullified.
 */
.macro RESTORE_SOME
.set noreorder
.set noat
	lw      v0, TF_STATUS(sp)
	mtc0    v0, CP0_STATUS
	lw      v1, TF_LO(sp)
	mtlo    v1
	lw      v0, TF_HI(sp)
	lw      v1, TF_EPC(sp)
	mthi    v0
	mtc0    v1, CP0_EPC
	lw      $31, TF_REG31(sp)
	lw      $30, TF_REG30(sp)
	lw      $28, TF_REG28(sp)
	lw      $25, TF_REG25(sp)
	lw      $24, TF_REG24(sp)
	lw      $23, TF_REG23(sp)
	lw      $22, TF_REG22(sp)
	lw      $21, TF_REG21(sp)
	lw      $20, TF_REG20(sp)
	lw      $19, TF_REG19(sp)
	lw      $18, TF_REG18(sp)
	lw      $17, TF_REG17(sp)
	lw      $16, TF_REG16(sp)
	lw      $15, TF_REG15(sp)
	lw      $14, TF_REG14(sp)
	lw      $13, TF_REG13(sp)
	lw      $12, TF_REG12(sp)
	lw      $11, TF_REG11(sp)
	lw      $10, TF_REG10(sp)
	lw      $9, TF_REG9(sp)
	lw      $8, TF_REG8(sp)
	lw      $7, TF_REG7(sp)
	lw      $6, TF_REG6(sp)
	lw      $5, TF_REG5(sp)
	lw      $4, TF_REG4(sp)
	lw      $3, TF_REG3(sp)
	lw      $2, TF_REG2(sp)
	lw      $1, TF_REG1(sp)
.set at
.set reorder
.endm
