#pragma once
#include <string>
#include <vector>
#include <list>
#include <functional>

namespace GScript {
	class Parser;
	class Executable;
	class Scope;

	class Executable {

	};

	class Parser{
	public:
		Parser();
		~Parser();
		Executable* Parse(std::string s);

	private:
		enum class LexerState {
			entry,string,iconst,fconst,identifier, punctuation
		};
		enum class TokenType {
			string, iconst, fconst,identifier,punctuation,optr
		};
		struct Token {
			TokenType type;
			std::string content;

			Token(const TokenType &type, const std::string &content) {
				this->type = type, this->content = content;
			}

			Token(const TokenType &type, const char content) {
				this->type = type, this->content = content;
			}
		};

		enum class PhraseType {
			//Code structures
			expr, subexpr/*()*/, indexer/*[]*/, block/*{}*/
			//Lexical terms
			, keyword, identifier, optr, string, iconst, fconst, punctuation
			//Seperators
			,forLoop,whileLoop
			,indexing
			//interprocessing terms
			, classDef
			, funcDecl
			, funcCall
			, varDecl
		};

	public:
		struct Phrase {
			PhraseType type;
			std::string content;
			std::vector<Phrase*> subPhrase;

			Phrase() {

			}

			Phrase(const PhraseType &type,const std::string &content) {
				this->type = type;
				this->content = content;
			}

			void Clear() {
				for (Phrase *p : subPhrase) {
					p->Clear();
					delete p;
				}
			}
		};
	private:
		enum class OpPos {
			left,right,both,none
		};
		enum class OpCombine {
			FromLeft, FromRight
		};
		struct Optr {
			OpPos pos;
			OpCombine combine;
			std::string content;

			Optr(const OpPos &pos, const std::string &str, const OpCombine& combine) {
				this->pos = pos, this->content = str;
				this->combine = combine;
			}

			Optr(const OpPos &pos, const std::string &str) {
				this->pos = pos, this->content = str;
				this->combine = OpCombine::FromLeft;
			}

			/*Optr(OpPos &&pos, std::string &&str) {
				this->pos = pos, this->content = str;
			}*/

			Optr(const std::string &str) {
				this->content = str, this->pos = OpPos::none;
				this->combine = OpCombine::FromLeft;
			}

			Optr(std::string &&str) {
				this->content = str, this->pos = OpPos::none;
				this->combine = OpCombine::FromLeft;
			}
		};

		std::vector<std::vector<Optr>> optrs ={
			{{OpPos::none,"this"},{OpPos::right,"new"},{OpPos::right,"delete"}},
			{{OpPos::both,"."},{OpPos::left,"++"},{OpPos::left,"--"} },
			{ {OpPos::right,"!"},{OpPos::right,"*"},{OpPos::right,"+" },{ OpPos::right,"-" },{ OpPos::right,"++" },{ OpPos::right,"--" } },
			{ { OpPos::both,"/" },{ OpPos::both,"*" } },
			{ { OpPos::both,"+" },{ OpPos::both,"-" } },
			{ { OpPos::both,">" },{ OpPos::both,"<" },{ OpPos::both,">=" },{ OpPos::both,"<=" } },
			{ { OpPos::both,"==" },{ OpPos::both,"!=" }},
			{ { OpPos::both,"^" } },
			{ { OpPos::both,"&&" } },
			{ { OpPos::both,"||" } },
			{{OpPos::both,":"}},
			{	{ OpPos::both,"=",OpCombine::FromRight },{ OpPos::both,"+=",OpCombine::FromRight },
				{ OpPos::both,"-=",OpCombine::FromRight },{ OpPos::both,"*=",OpCombine::FromRight },
				{ OpPos::both,"/=" ,OpCombine::FromRight}
			},
			{{OpPos::both,","}}
		};

		std::vector<std::string> keywords = {
			//  Code structure  :
			"var", "func", "def", "inst", "using", "class",
			//  Flow control    :
			"if","else", "switch","case", "return", "for","while", "continue", "break",
			//  Accessibility   :
			"public", "virtual", "override", "static",
			//  Variable modify :
			"this","new","delete"
		};

		struct ParseRule {
			PhraseType retType;
			std::vector<Phrase> pattern;
			ParseRule(const PhraseType& type,const std::vector<Phrase>& pattern) {
				retType = type; this->pattern = pattern;
			}
		};

		

		bool IsOptrSubstr(const std::string &s);
		bool IsOptr(const std::string &s);
		Optr* FindOptr(const std::string &s);
		bool IsKeyword(std::string &s);
		std::vector<Token>* Str2Token(std::string &s);
		Phrase* Token2Phrase(std::vector<Token> &tokens);
//		Phrase* FindPhrasePattern(PhraseType retType, std::vector<Phrase*> &ph, std::vector<Phrase> &pattern) { return FindPhrasePattern(retType, ph, pattern, nullptr, [](Phrase* t) {return; }); }
		static void FindPhrasePattern(std::vector<Phrase*> &ph,const std::vector<Phrase> &pattern, std::function<void(int)> callBack);
//		Phrase* FindPatternByRule(std::vector<Phrase*> &ph, std::vector<ParseRule> &rules) { Phrase* t = nullptr; FindPatternByRule(ph, rules, [&](Phrase* p, int i) {t = p; return; }); return t; };
// 		bool FindPatternByRule(std::vector<Phrase*> &ph, std::vector<ParseRule> &rules, std::function<void(Phrase*, int)> callBack) { return FindPatternByRule(ph, rules, callBack, nullptr, [](Phrase* p) {}); };
// 		bool FindPatternByRule(std::vector<Phrase*> &ph, std::vector<ParseRule> &rules, std::function<void(Phrase*, int)>callBack);
	};

	class SubScope {
		friend class Scope;
	public: 
		struct VarName {
			std::vector<std::string> type;
			std::string name;
			VarName(std::string name, std::string type) {
				this->name = name; this->type.push_back(type);
			}
			VarName(std::string name,const std::vector<std::string>& type) {
				this->name = name; this->type=type;
			}
		};

		struct TypeName {
			std::string type;
			bool isTemplate;
			TypeName(const std::string& type, bool isTemplate) {
				this->type = type;
				this->isTemplate = isTemplate;
			}
			TypeName(const std::string& type) {
				TypeName(type, false);
			}
		};
		std::string scopeName;
	private:
		int index;
		std::vector<std::string> typeNames;
		std::vector<VarName> varNames;
		SubScope * parent;
		std::vector<SubScope *>children;
	public:
		~SubScope()
		{
			for (SubScope *p : children) {
				delete p;
			}
		}

		bool FindTypeName(std::string name) {
			if (find(typeNames.begin(), typeNames.end(), name) == typeNames.end()) {
				if (parent == nullptr)
					return false;
				else return parent->FindTypeName(name);
			}
			else return true;
		}

		SubScope* FindVarName(std::string name) {
			for (int i = 0; i < varNames.size();i++)
			{
				if (varNames[i].name == name) {
					return this;
				}
			}
			if (parent == nullptr)
				return nullptr;
			else return parent->FindVarName(name);
		}

		SubScope* FindScopeInParent(std::string name) {
			for (int i = 0; i < children.size(); i++) {
				if (children[i]->scopeName == name)
					return children[i];
			}
			if (parent == nullptr)
				return nullptr;
			else return parent->FindScopeInParent(name);
		}

		SubScope* FindScopeByVarName(std::string name) {
			SubScope * nativeScope = FindVarName(name);
			if (nativeScope != nullptr) {
				for (int i = 0; i < nativeScope->varNames.size(); i++) {
					if (nativeScope->varNames[i].name == name) {
						SubScope *s = FindScopeInParent(nativeScope->varNames[i].type[0]);
						if (s != nullptr) {
							for (int depth = 1; depth < nativeScope->varNames[i].type.size(); depth++) {
								s = s->FindSubScope(nativeScope->varNames[i].type[depth]);
								if (s == nullptr)
									break;
							}
						}
						return s;
					}
				}
			}
			return nullptr;
		}

		SubScope* FindSubScope(std::string name) {
			for (int i = 0; i < children.size(); i++) {
				if (children[i]->scopeName == name)
					return children[i];
			}
			return nullptr;
		}
	};

	class Scope {
		SubScope root;
		SubScope* currNode;
		int previousVisited;
	public:
		Scope() {
			root.index = 0;
			root.parent = nullptr;
			currNode = &root;
			previousVisited = -1;
		}
		void ResetScopePosition() {
			currNode = &root;
			previousVisited = -1;
		}

		SubScope* CreateScope(std::string name = "") {
			SubScope * t = new SubScope();
			t->index = currNode->children.size();
			t->parent = currNode;
			t->scopeName = name;
			currNode->children.push_back(t);
			currNode = t;
			return currNode;
		}

		SubScope* EnterScope() {
			if (currNode->children.size() > previousVisited + 1) {
				currNode = currNode->children[previousVisited + 1];
				return currNode;
			}
			return nullptr;
		}

		SubScope* LeaveScope() {
			if (currNode->parent != nullptr) {
				previousVisited = currNode->index;
				currNode = currNode->parent;
				return currNode;
			}
			return nullptr;
		}

		SubScope* FindScope(std::string name) {
			return root.FindSubScope(name);
		}

		SubScope* GetCurrentScope() {
			return currNode;
		}

		void AddTypeNameEntry(std::string name) {
			if (find(currNode->typeNames.begin(), currNode->typeNames.end(), name) == currNode->typeNames.end()) {
				currNode->typeNames.push_back(name);
			}
			
		}

		void AddVarNameEntry(std::string type, std::string name) {
			for (int i = 0; i < currNode->varNames.size(); i++) {
				if (currNode->varNames[i].name == name)
					return;
			}
			currNode->varNames.emplace_back(name,type);
		}

		void AddVarNameEntry(const std::vector<std::string>& type, std::string name) {
			for (int i = 0; i < currNode->varNames.size(); i++) {
				if (currNode->varNames[i].name == name)
					return;
			}
			currNode->varNames.emplace_back(name, type);
		}

		bool FindVarName(std::string name) { return currNode->FindVarName(name); }
		bool FindTypeName(std::string name) { return currNode->FindTypeName(name); }
	};
}
