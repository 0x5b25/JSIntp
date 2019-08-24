#include "Operators.h"
namespace GScript {

	inline GOFuncCall::GOFuncCall(GOperator * funcEntry) { 
		this->funcEntry = funcEntry; 
		argEntry.next = funcCall;
		funcCall->prev = &argEntry;
	}

	inline GOFuncCall::GOFuncCall(GOperator * funcEntry, const std::vector<GOExpr*>& args){
		if (args.size() <= 0 || funcEntry == nullptr) {
			GOFuncCall(funcEntry);
			return;
		}
		
		for (int i = 0; i < args.size();i++) {
			if (i != 0) {
				args[i]->prev = args[i - 1];
			}
			if (i != args.size() - 1) {
				args[i]->next = args[i + 1];
			}
			this->args.push_back(args[i]);
		}

		this->argEntry.next = this->args.front();
		this->args.front()->prev = &this->argEntry;

		this->funcEntry = funcEntry;
		this->args.back()->next = funcCall;
		funcCall->prev = this->args.back();
	}

	GVariable * GOFuncCall::Exec(std::vector<GVariable*>& stack, GThread * context)
	{
		if (funcEntry == nullptr)
			return nullptr;

		context->IP = next;
		context->PushStat();
		context->IP = &argEntry;

		return nullptr;
	}
}