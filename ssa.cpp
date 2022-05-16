#include "ssa.h"


SSAValue SSAValue::SSACreate(opcode operation, SSAValue* x, SSAValue* y) {
	SSAValue* result = new SSAValue();
	result->id = maxID++;
	result->op = operation;
	result->operand1 = x;
	result->operand2 = y;

}

SSA::SSA() {
	instCounter = 0;
}

void SSA::addSSAValue(SSAValue* newSSAVal) {
	if (instCounter == 0) {
		instList = newSSAVal;
		instTail = newSSAVal;

	} else {
		instTail->next = newSSAVal;
		newSSAVal->prev = instTail;
		instTail = newSSAVal;
	}
	instCounter += 1;
}

void SSA::addConditionalBlock() {
	context previousCtx = currentCtx;

	BasicBlock* splitBlock = new BasicBlock();
	BasicBlock* thenBlock = new BasicBlock();
	BasicBlock* elseBlock = new BasicBlock();
	BasicBlock* joinBlock = new BasicBlock();


	currentCtx.currentJoinBlock = joinBlock;
	currentCtx.currentFTBlock = splitBlock;
	currentCtx.isWhile = false;
}

void SSA::addWhileBlock() {
	context previousCtx = currentCtx;

	BasicBlock* joinBlock = new BasicBlock();
	BasicBlock* followBlock = new BasicBlock();

	currentCtx.currentJoinBlock = joinBlock;
	currentCtx.currentFTBlock = followBlock;
	currentCtx.isWhile = true;
}

