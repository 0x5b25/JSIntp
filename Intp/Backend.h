#pragma once
#include <string>
#include <list>
#include <vector>
#include <unordered_map>
#include <tuple>

#include "Frontend.h"

namespace GScript {

	class GEnv;
	class GThread;
	class GVariable;
	class GObject;
	class GOperator;

	//Runtime enviornment
	class GEnv {
	public:
		//#TODO:finalize loader interface
		void LoadSyntaxTree(const Parser::Phrase &syntaxTree);
		void LoadBinary();

		GThread* CreateThread(const std::string& methodName);
	};

	class GThread {
		friend class GEnv;
		friend class GOperator;

		std::vector<GVariable*> varStack;
		std::vector<std::tuple<GVariable*,GOperator*>> callStack;//current context:current instruction

	public:
		//Instruction pointer
		GOperator* IP;
		GVariable* context;

		std::vector<GVariable*>& GetStack() { return varStack; }
		bool Step();
		bool Exec();
		void PushStat();
		bool PopStat();

	};



	class GObject {
		friend class GVariable;

		//Inheritance chain, used for casting
		std::vector<GObject*> parents;
		GObject* child;

		//reference number tracking, used for gc
		int refNum;

		//Object type id
		int id;
		std::unordered_map<std::string, GVariable*> objects;
		std::unordered_map<std::string, GVariable*> attrs;
	public:
		GObject();
		GObject(const GObject& obj, GVariable * parentVal);
		~GObject();

		GVariable* Find(const std::string& name);

		GVariable* FindAttr(const std::string& name);

		void Add(const std::string& name, GVariable *object);

		bool Delete(const std::string& name);

		bool IsChildOf(int objectid);

	private:
		std::unordered_map<std::string, GVariable*>::iterator _find(const std::string& name);

		std::unordered_map<std::string, GVariable*>::iterator _findAttr(const std::string& name);


	};

	class GVariable {
	protected:
		GVariable * parent;
		std::list<GVariable*> refs;
	public:
		enum class VarType {
			Integer, Float, Boolean, String, Ref, Null
		};
	protected:
		//literal value
		VarType varType;
		union {
			long	ival;
			double	fval;
			bool	bval;
			std::string* sval;
			GObject* objref;
		}val;

		//attributes
		std::string varName;
	public:
		GVariable(GVariable* parent = nullptr);
		GVariable(const std::string&name, GVariable* parent = nullptr);

		GVariable(const GVariable::VarType &type, GVariable* parent = nullptr);
		GVariable(const GVariable::VarType &type, const std::string& name, GVariable* parent = nullptr);

		GVariable(const GVariable&o);

		GVariable(const GVariable&o, const std::string& name);

		GVariable(const GVariable&o, GVariable* parent);

		GVariable(const GVariable&o, GVariable* parent, const std::string& name);
		~GVariable();
	public:
		//functions

		//Used to get custom flags for inherited classes
		virtual int GetFlag(int flag) { return 0; };

		//Make a deep copy
		GVariable* Copy(GVariable * parent = nullptr) { return Copy("", parent); };

		//Inherited class can make a full copy by overriding this function
		virtual GVariable* Copy(const std::string& name, GVariable * parent = nullptr) { return new GVariable(this, parent, name); };

		virtual bool Assign(const GVariable* var);

		virtual GVariable* AccessMember(const std::string& name);

		virtual GVariable* AccessMemberByIndex(const std::string& index);

		virtual bool AddMember(const std::string& name, GVariable*object);

		virtual GVariable* Invoke(GVariable* context, GVariable* arg);
	};

	class GOperator
	{
	protected:
		
	public:
		GOperator* prev, *next;

		GOperator() { prev = next = nullptr; };
		GOperator(GOperator* next) { prev = nullptr; this->next = next; };
		GOperator(GOperator* prev, GOperator* next) { ; this->prev = prev; this->next = next; };

		virtual GVariable* Exec(std::vector<GVariable*>& stack, GThread*context) { return nullptr; };
		GOperator* GetNextOperator() { return next; };
		virtual int GetFlag(int flag) { return 0; };
	};
	
}