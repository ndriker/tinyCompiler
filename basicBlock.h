#ifndef __BASICBLOCK_H_
#define __BASICBLOCK_H__

#include "ssa.h"

#include <vector>



class BasicBlock {
	public:
		BasicBlock();
		void setStartInst(SSAValue* start);
		void setEndInst(SSAValue* end);

		void setPrevious(BasicBlock* previousBB, int position);
		void setNext(BasicBlock* nextBB, int position);

		void setDom(BasicBlock* dom);
		//void addValue(vtItem val);
		void addInst(SSAValue* newSSAValue);
		

	private:
		SSAValue* startInst;
		SSAValue* endInst;

		// convention is as follows:
		//		fall through is always index 0
		//		branch is always index 1
		//		fall through always to the left
		//		branch always to the right
		BasicBlock* previous[2];
		BasicBlock* next[2];


		BasicBlock* dominator;
		//std::vector<vtItem> valueTable;
		int instCount;
};

#endif