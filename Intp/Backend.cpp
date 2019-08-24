#include "Backend.h"
namespace GScript {

	bool GThread::Step()
	{
		if (IP == nullptr) {
			if (callStack.size() <= 0)
				return false;//Execution complete
			else {
				//block execution complete, return to previous exec flow
				IP = std::get<1>(callStack.back());
				context = std::get<0>(callStack.back());
				callStack.pop_back();
			}
		}
		GVariable* temp = IP->Exec(varStack, this);
		if (temp != nullptr)
			//push result into stack
			varStack.push_back(temp);
		IP = IP->GetNextOperator();
	}

	bool GThread::Exec() {
		//continuously execute instructions
		while (Step());
		return false;
	}

	void GThread::PushStat()
	{
		if (IP != nullptr) {
			callStack.push_back(std::make_tuple(context, IP));
		}
	}

	bool GThread::PopStat()
	{
		if (callStack.size() <= 0)
			return false;//Execution complete
		else {
			//block execution complete, return to previous exec flow
			IP = std::get<1>(callStack.back());
			context = std::get<0>(callStack.back());
			callStack.pop_back();
			return true;
		}
	}

	GObject::GObject() {
		refNum = 1;
	}
	GObject::GObject(const GObject& obj, GVariable * parentVal) {

		for (auto &po : obj.objects) {
			if (po.second != nullptr) {
				objects[po.first] = new GVariable(*po.second, parentVal);
			}
		}
	}
	GObject::~GObject() {
		for (auto &po : objects) {
			if (po.second != nullptr) {
				delete po.second;
			}
		}
	}

	GVariable* GObject::Find(const std::string& name) {
		auto it = _find(name);
		if (it != objects.end()) {
			return (*it).second;
		}
		return nullptr;
	}

	GVariable* GObject::FindAttr(const std::string& name) {
		auto it = _findAttr(name);
		if (it != attrs.end()) {
			return (*it).second;
		}
		return nullptr;
	}

	void GObject::Add(const std::string& name, GVariable *object) {
		auto it = _findAttr(name);
		if (it != attrs.end()) {
			delete (*it).second;
			(*it).second = object;
			return;
		}
		attrs[name] = object;
	}

	bool GObject::Delete(const std::string& name) {

		if (attrs.erase(name) < 1) {
			for (auto it : parents) {
				if (it->Delete(name)) {
					return true;
				}
			}
			return false;
		}
		return true;
	}

	bool GObject::IsChildOf(int objectid) {
		if (id == objectid)
			return true;
		for (auto p : parents)
		{
			if (p->IsChildOf(objectid))
				return true;
		}
		return false;
	}

	std::unordered_map<std::string, GVariable*>::iterator GObject::_find(const std::string& name) {
		auto it = objects.find(name);
		if (it != objects.end()) {
			return it;
		}
		//TODO:Find object in parent
		std::unordered_map<std::string, GVariable*>::iterator result;
		for (auto it : parents) {
			result = (*it)._find(name);
			if (result != (*it).objects.end()) {
				return result;
			}
		}
		return objects.end();
	}

	std::unordered_map<std::string, GVariable*>::iterator GObject::_findAttr(const std::string& name) {
		auto it = attrs.find(name);
		if (it != attrs.end()) {
			return it;
		}
		//TODO:Find object in parent
		std::unordered_map<std::string, GVariable*>::iterator result;
		for (auto it : parents) {
			result = (*it)._findAttr(name);
			if (result != (*it).attrs.end()) {
				return result;
			}
		}
		return attrs.end();
	}


	inline GVariable::GVariable(GVariable * parent) {
		GVariable(VarType::Null, parent);
	}

	inline GVariable::GVariable(const std::string & name, GVariable * parent) {
		GVariable(VarType::Null, name, parent);
	}

	inline GVariable::GVariable(const GVariable::VarType & type, GVariable * parent) {
		this->varType = type;
		this->parent = parent;
	}

	inline GVariable::GVariable(const GVariable::VarType & type, const std::string & name, GVariable * parent) {
		GVariable(type, parent);
		varName = name;
	}

	inline GVariable::GVariable(const GVariable & var) {
		GVariable(var, var.parent);
	}

	inline GVariable::GVariable(const GVariable & var, const std::string & name) {
		GVariable(var, var.parent, name);
	}

	inline GVariable::GVariable(const GVariable & var, GVariable * parent) {
		this->varType = var.varType;
		this->parent = parent;
		if (varType == VarType::Ref && var.val.objref != nullptr) {
			val.objref = new GObject(*var.val.objref, this);
		}
		else if (varType == VarType::String&& var.val.sval != nullptr) {
			val.sval = new std::string(*(var.val.sval));
		}
		else {
			val = var.val;
		}
	}

	inline GVariable::GVariable(const GVariable & var, GVariable * parent, const std::string & name) {
		GVariable(var, parent);
		varName = name;
	}

	inline GVariable::~GVariable() {
		if (varType == VarType::Ref && val.objref != nullptr) {
			//TODO:notify reference counter
			if (val.objref->refNum <= 1) {
				delete val.objref;
			}
			else {
				val.objref->refNum--;
			}
			GVariable* dtor = AccessMember("dtor");
			if (nullptr != dtor) {
				dtor->Invoke(this, nullptr);
			}
			else {
				dtor = AccessMemberByIndex("dtor");
				if (nullptr != dtor) {
					dtor->Invoke(this, nullptr);
				}
			}
		}
		else if (varType == VarType::String&& val.sval != nullptr) {
			delete val.sval;
		}

	}

	bool GVariable::Assign(const GVariable* var)
	{
		if (var == nullptr) {
			//TODO:disable this variable
			if (parent != nullptr) {

			}
		}
		switch (varType)
		{
		case GScript::GVariable::VarType::Integer: {
			switch (var->varType)
			{
			case VarType::Integer: {
				val.ival = var->val.ival;
			}return true;
			case VarType::Float: {
				val.ival = std::floor(var->val.fval);
			}return true;
			case VarType::Boolean: {
				val.ival = (var->val.bval) ? 1 : 0;
			}return true;
			default:return false;
			}
		}
												   break;
		case GScript::GVariable::VarType::Float: {

		}
												 break;
		case GScript::GVariable::VarType::Boolean: {

		}
												   break;
		case GScript::GVariable::VarType::String: {

		}
												  break;
		case GScript::GVariable::VarType::Ref: {

		}
											   break;
		case GScript::GVariable::VarType::Null: {
			//TODO:Create variable
		}
												break;
		}
	}

	inline GVariable * GVariable::AccessMember(const std::string & name) {
		if (varType == VarType::Ref && val.objref != nullptr) {
			return val.objref->Find(name);
		}
		return nullptr;
	}

	inline GVariable * GVariable::AccessMemberByIndex(const std::string & index) {
		if (varType == VarType::Ref && val.objref != nullptr) {
			return val.objref->FindAttr(index);
		}
		return nullptr;
	}

	inline bool GVariable::AddMember(const std::string & name, GVariable * object) {
		if (varType == VarType::Ref && val.objref != nullptr) {
			val.objref->Add(name, object);
			return true;
		}
		return false;
	}

	inline GVariable * GVariable::Invoke(GVariable * context, GVariable * arg) {
		//Call constructor
		GVariable* ctor = AccessMember("ctor");
		if (nullptr != ctor) {
			return ctor->Invoke(this, nullptr);
		}
		else {
			ctor = AccessMemberByIndex("ctor");
			if (nullptr != ctor) {
				return ctor->Invoke(this, nullptr);
			}
		}
		return nullptr;
	}

}