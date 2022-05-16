#include "basicBlock.h"

BasicBlock::BasicBlock() {
	instCount = 0;
}

void BasicBlock::setStartInst(SSAValue* start) {
	startInst = start;
}

void BasicBlock::setEndInst(SSAValue* end) {
	endInst = end;
}

void BasicBlock::setPrevious(BasicBlock* previousBB, int position) {
	previous[position] = previousBB;
}

void BasicBlock::setNext(BasicBlock* nextBB, int position) {
	next[position] = nextBB;
}


void BasicBlock::setDom(BasicBlock* dom) {
	dominator = dom;
}

void BasicBlock::addValue(vtItem val) {
	valueTable.push_back(val);
}

void BasicBlock::addInst(SSAValue* newSSAValue) {
	if (instCount == 0) {
		startInst = newSSAValue;
		endInst = newSSAValue;
	} else {
		endInst->next = newSSAValue;
		newSSAValue->prev = endInst;
		endInst = newSSAValue;
	}
	instCount += 1;

}
