#pragma once
#include "Backend.h"

#include <functional>

namespace GScript {
	//Flags
	struct OpFlag{
		static constexpr int Callable = 0;
		static constexpr int FuncBody = 1;
		static constexpr int ChangeContext = 2;
	};

	typedef std::function<GVariable*(std::vector<GVariable *>& stack, GThread*context)> GOP;

	class GOCustomOp : public GOperator {
		GOP op;
	public:
		GOCustomOp(const GOP& op) {this->op = op;}
		GOCustomOp(const GOP& op, GOperator* next):GOperator(next) {this->op = op;}
		GOCustomOp(const GOP& op, GOperator* prev, GOperator* next):GOperator(prev,next) {this->op = op;}

		virtual GVariable* Exec(std::vector<GVariable *>& stack, GThread*context) { return op(stack, context); }
	};

	class GOVarAccess;
	class GOExpr;
	class GOVarIndex;
	class GOFuncCall;
	class GOFunction;
	class GONewObject;

	class GOVarAccess : public GOperator {
		std::string varName;
	public:
		//Ctor
		GOVarAccess(const std::string& name) { 
			varName = name;
		}
		//Copy ctor
		GOVarAccess(const GOVarAccess& var) {
			varName = var.varName;
		}
		virtual int GetFlag(int flag) override {
			switch (flag)
			{
			default:return 0;
			}
		}
		
		virtual GVariable* Exec(std::vector<GVariable *>& stack, GThread*context) {

		}
	};

	class GOExpr :public GOperator {
	protected:
		GOperator entry;
	public:

		virtual int GetFlag(int flag) override {
			switch (flag)
			{
			case OpFlag::ChangeContext:
				return 1;
			default:return 0;
			}
		}

		virtual GVariable* Exec(std::vector<GVariable *>& stack, GThread*context)override {
			context->IP = next;
			context->PushStat();
			context->IP = &entry;
			return nullptr;
		}
	};

	class GOVarIndex : public GOperator {
	public:

		virtual int GetFlag(int flag) override {
			switch (flag)
			{
			default:return 0;
			}
		}

	};

	class GOFuncCall : public GOperator {

		GOperator argEntry;//next:args[0]
		GOCustomOp *funcCall = new GOCustomOp([&](std::vector<GVariable *>& stack, GThread*context) ->GVariable* { 
			return nullptr; 
		});
		std::vector<GOExpr*> args;//args[last] next:funcEntry
		GOperator* funcEntry;//next:nullptr
	public:
		//Cant use as entry class
		GOFuncCall(){ funcEntry = nullptr; };

		GOFuncCall(GOperator* funcEntry);;
		GOFuncCall(GOperator* funcEntry, GOperator* next) :GOperator(next) { GOFuncCall(funcEntry); };
		GOFuncCall(GOperator* funcEntry, GOperator* prev, GOperator* next) :GOperator(prev,next) { GOFuncCall(funcEntry); };

		GOFuncCall(GOperator* funcEntry, const std::vector<GOExpr*>& args);
		GOFuncCall(GOperator* funcEntry, const std::vector<GOExpr*>& args, GOperator* next) :GOperator(next) { GOFuncCall(funcEntry,args); };
		GOFuncCall(GOperator* funcEntry, const std::vector<GOExpr*>& args, GOperator* prev, GOperator* next) :GOperator(prev, next) { GOFuncCall(funcEntry,args); };

		~GOFuncCall() {
			for (auto e : args) {
				delete e;
			}
			if (funcEntry != nullptr)
				delete funcEntry;
			delete funcCall;
		}

		//Override functions
	public:
		virtual int GetFlag(int flag) override {
			switch (flag)
			{
			case OpFlag::Callable:return 1;
			default:return 0;
			}
		}

		virtual GVariable* Exec(std::vector<GVariable *>& stack, GThread*context);

	};

	class GOFunction :public GOperator {
	public:
		virtual int GetFlag(int flag) override {
			switch (flag)
			{
			case OpFlag::FuncBody:return 1;
			default:return 0;
			}
		}

	};

	class GONewObject : public GOperator {

	};

}