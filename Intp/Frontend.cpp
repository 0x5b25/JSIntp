#include "Frontend.h"
#include <cstdio>
#include <algorithm>
#include <functional>
namespace GScript {


	Parser::Parser()
	{
	}


	Parser::~Parser()
	{
	}

	Executable* Parser::Parse(std::string s) {
		std::vector<Token> *tokens = Str2Token(s);
		Phrase *phrases = Token2Phrase(*tokens);
		std::function<void(const Phrase&,int)> display = [&](const Phrase& p, int depth) {
			auto printMargin = [](int m) {
				for (int i = 0;i<m;i++)
				{
					printf("     |");
				}
			};
			//Print type and content
			printf("[");
			switch (p.type)
			{
			default:printf("unknown");
				break;
				case PhraseType::expr:printf("expression");
					break;
				case PhraseType::subexpr:printf("subexpr");
					break;
				case PhraseType::indexer:printf("indexer");
					break;
				case PhraseType::block:printf("block");
					break;
				case PhraseType::keyword:printf("keyword");
					break;
				case PhraseType::identifier:printf("identifier");
					break;
				case PhraseType::optr:printf("operator");
					break;
				case PhraseType::string:printf("string");
					break;
				case PhraseType::iconst:printf("constant");
					break;
				case PhraseType::fconst:printf("constant");
					break;
				case PhraseType::punctuation:printf("punc");
					break;
				case PhraseType::forLoop:printf("forloop");
					break;
				case PhraseType::whileLoop:printf("whileloop");
					break;
				case PhraseType::indexing:printf("index access");
					break;
				case PhraseType::classDef:printf("class definition");
					break;
				case PhraseType::funcDecl:printf("function declaration");
					break;
				case PhraseType::funcCall:printf("function call");
					break;
				case PhraseType::varDecl:printf("variable declaration");
					break;
			}
			printf("]");
			if(p.content != "")
				printf(("{" + p.content + "}").c_str());
			printf("\n");
			//Print child:
			for (int i = 0; i < p.subPhrase.size();i++)
			{
				printMargin(depth+1);
				printf("-->");
				display(*(p.subPhrase[i]), depth + 1);
			}
		};
		
		display(*phrases, 0);
		phrases->Clear();

		delete phrases;

		return nullptr;
	}

	bool Parser::IsOptrSubstr(const std::string &s)
	{
		for (std::vector<Optr> &optr : optrs) {
			for (Parser::Optr &o : optr)
			{
				if (o.content.find(s) != std::string::npos)
					return true;
			}
		}
		return false;
	}

	bool Parser::IsOptr(const std::string &s)
	{
		return ((FindOptr(s) == nullptr) ? false : true);
	}

	GScript::Parser::Optr* Parser::FindOptr(const std::string &s)
	{
		for (std::vector<Optr> &optr : optrs) {
			for (Parser::Optr &o : optr)
			{
				if (o.content == s)
					return &o;
			}
		}
		return nullptr;
	}

	bool Parser::IsKeyword(std::string & s)
	{
		for (std::string &k : keywords) {
			if (k == s)
				return true;
		}
		return false;
	}

	std::vector<Parser::Token>* Parser::Str2Token(std::string & s)
	{
		//Add a termination to ensure the state machine can return to start point
		s += '\0';
		std::function<bool(const char&)> IsEmptyChar = [](const char &c) ->bool {
			if (c == ' ' || c == '\n' || c == '\r' || c == '\t')
				return true;
			return false;
		};
		std::vector<Token> *tokens = new std::vector<Token>;
		LexerState state = LexerState::entry;
		std::string buffer;
		for (int i = 0; i != s.length();) {
			switch (state)
			{
			case LexerState::entry: {
				if (s[i] == ' ' || s[i] == '\n' || s[i] == '\t') {
					i++; continue;
				}
				if (s[i] == '"') { state = LexerState::string; buffer.clear(); i++; continue; }
				if (s[i] <= '9' && s[i] >= '0') { state = LexerState::iconst; buffer.clear(); continue; }
				//if (s[i] == '.') { state = LexerState::fconst; buffer.clear(); i++; continue; }
				if ((s[i] <= 'Z' && s[i] >= 'A') ||
					(s[i] <= 'z' && s[i] >= 'a') ||
					s[i] == '_') {
					state = LexerState::identifier; buffer.clear(); continue;
				}
				state = LexerState::punctuation; buffer.clear(); continue;
			}
			break;
			case LexerState::string: {
				if (s[i] == '"') {
					if (s[i - 1] != '\\') {
						//generate token
						tokens->emplace_back(TokenType::string, buffer);
						state = LexerState::entry; i++; continue;
					}
				}
				buffer += s[i];
				i++;
			}
			break;
			case LexerState::iconst: {
				if (s[i] <= '9' && s[i] >= '0') {
					buffer += s[i];
					i++;
					continue;
				}
				if (s[i] == '.') {
					buffer += s[i];
					i++;
					state = LexerState::fconst; continue;
				}
				//generate token
				tokens->emplace_back(TokenType::iconst, buffer);
				state = LexerState::entry; continue;
			}
			break;
			case LexerState::fconst: {
				if (buffer.length() == 0) buffer += "0.";
				if (s[i] <= '9' && s[i] >= '0') {
					buffer += s[i];
					i++;
					continue;
				}
				//generate token
				tokens->emplace_back(TokenType::fconst, buffer);
				state = LexerState::entry; continue;
			}
			break;
			case LexerState::identifier: {
				if ((s[i] <= 'Z' && s[i] >= 'A') ||
					(s[i] <= 'z' && s[i] >= 'a') ||
					(s[i] <= '9' && s[i] >= '0') ||
					s[i] == '_')
				{
					buffer += s[i];
					i++;
					continue;
				}
				//generate token
				tokens->emplace_back(TokenType::identifier, buffer);
				state = LexerState::entry; continue;
			}
			break;
			case LexerState::punctuation: {
				if (!(s[i] <= 'Z' && s[i] >= 'A') &&
					!(s[i] <= 'z' && s[i] >= 'a') &&
					!(s[i] <= '9' && s[i] >= '0') && 
					s[i] != '_' && s[i]!=' ') {
					//Make sure everything isn't alphanumeric
					if (IsOptrSubstr(buffer + s[i])) {
						buffer += s[i];
						i++;
						continue;
					}
				}
				if (!buffer.empty()) {
					tokens->emplace_back(TokenType::optr, buffer);
				}
				else{
					tokens->emplace_back(TokenType::punctuation, s[i]);
					i++;
				}
				state = LexerState::entry;
			}
			break;
			}

		}
		return tokens;
	}

	Parser::Phrase* Parser::Token2Phrase(std::vector<Token>& tokens)
	{
		unsigned int i = 0; std::vector<Phrase*> ph;
		//Convert tokens to phrases and resolve blocks
		while (i < tokens.size()) {
			if (tokens[i].type == TokenType::identifier&&IsKeyword(tokens[i].content)) {
				Phrase *p = new Phrase();
				//Ensure alphanumeric operators can be recogonized correctly
				if (IsOptr(tokens[i].content))
					p->type = PhraseType::optr;
				else
					p->type = PhraseType::keyword;
				p->content = tokens[i].content;
				ph.push_back(p);
				i++;
				continue;
			}

			Phrase *p = new Phrase();
			switch (tokens[i].type)
			{
			case TokenType::string:p->type = PhraseType::string;
				break;
			case TokenType::iconst:p->type = PhraseType::iconst;
				break;
			case TokenType::fconst:p->type = PhraseType::fconst;
				break;
			case TokenType::identifier:p->type = PhraseType::identifier;
				break;
			case TokenType::punctuation:p->type = PhraseType::punctuation;
				break;
			case TokenType::optr:p->type = PhraseType::optr;
				break;
			}
			p->content = tokens[i].content;
			ph.push_back(p);
			i++;
		}

		std::function<Phrase*(std::vector<Phrase*>&,PhraseType, int, int, bool)> chop = [](std::vector<Phrase*>& ph, PhraseType type,int start, int end, bool exclude)->Phrase* {
			//Chop and reconstruct
			if (end <= start)
				return nullptr;
			Phrase* tp = new Phrase();
			tp->type = type;
			for (int i = exclude?start + 1:start; i < (exclude?end - 1:end); i++) {
				//Attach nodes to the first node
				tp->subPhrase.push_back(ph[i]);
			}
			if (end - start > 1) {
				if (exclude) {
					delete ph[start];
					delete ph[end - 1];
				}
				ph.erase(ph.begin() + start + 1, ph.begin() + end);
			}
							
			ph[start] = tp;
			return tp;
		};
		
		//Find function call, function declaration, class definition and variable declaration
		std::function<void(std::vector<Phrase*>&)> processExpr = [&](std::vector<Phrase*>& ph) {
			auto evalOpPos = [&](const Optr& optr, int pos) {
				if(ph[pos]->type != PhraseType::optr || ph[pos]->content != optr.content)
					return false;
				switch (optr.pos)
				{
				default:
					break;
				case OpPos::left: {
					if(pos >= ph.size() - 1){
						return true;
					}
				}
					break;
				case OpPos::right: {
					if (pos <= 0 ) {
						return true;
					}
				}
					break;
				case OpPos::both: {
					if (pos > 0 && 
						pos < ph.size() - 1 ) {
						return true;
					}
				}
					break;
				case OpPos::none: {
					if (ph.size() == 1 ) {
						return true;
					}
				}
					break;
				}
				return false;
			};
			auto divideExpr = [&](int pos) {
				Phrase *p = ph[pos];
				std::vector<Phrase*> e;
				for (int i = 0; i < pos; i++) {
					e.push_back(ph[i]);
				}
				if (!e.empty()) {
					processExpr(e);
					p->subPhrase.insert(p->subPhrase.end(), e.begin(), e.end());
					//p->subPhrase.push_back(e[0]);
				}
				e.clear();
				for (int i = pos + 1; i < ph.size(); i++) {
					e.push_back(ph[i]);
				}
				if (!e.empty()) {
					processExpr(e);
					p->subPhrase.insert(p->subPhrase.end(), e.begin(), e.end());
				}
				ph.clear();
				ph.push_back(p);
			};
			if (ph.size() <= 1)
				return;
			for (int i = optrs.size() - 1; i >=0; i--) {
				for (int j = 0; j < optrs[i].size(); j++) {
					if (optrs[i][j].combine == OpCombine::FromLeft) {
						for (int phIndex = 0; phIndex < ph.size(); phIndex++) {
							if (evalOpPos(optrs[i][j], phIndex)) {
								divideExpr(phIndex);
								return;
							}
						}
					}
					else {
						for (int phIndex = ph.size() - 1; phIndex >=0; phIndex--) {
							if (evalOpPos(optrs[i][j], phIndex)) {
								divideExpr(phIndex);
								return;
							}
						}
					}
				}
			}
		};
		std::function<void(std::vector<Phrase*>&)> processPhrase = [&](std::vector<Phrase*>& ph) {
			for (int i = 0; i < ph.size(); i++) {
				if (ph[i]->content == ")" || ph[i]->content == "}" || ph[i]->content == "]") {
					//TODO:report chop error
				}
			}
			//Find pattern:class definition
			for (int i = 0; i < ph.size(); i++) {
				if (ph[i]->type == PhraseType::keyword && ph[i]->content == "class") {
					if (i + 1 < ph.size()) {
						if (ph[i + 1]->type == PhraseType::identifier) {
							ph[i]->type = PhraseType::classDef;
							ph[i]->content = ph[i + 1]->content;
							delete ph[i + 1];
							if (i + 2 < ph.size()) {
								if (ph[i + 2]->content == ":") {
									//#TODO:check whether i + 4 is in range
									delete ph[i + 2];
									for (int j = i + 4; j < ph.size(); ) {
										if (ph[j-1]->type == PhraseType::identifier) {
											ph[i]->subPhrase.push_back(ph[j-1]);
											if (ph[j]->content == ",") {
												delete ph[j];
												j += 2;
												continue;
											}else if (ph[j]->type == PhraseType::block) {
												ph[i]->subPhrase.push_back(ph[j]);
												ph.erase(ph.begin() + i + 1, ph.begin() + j + 1);
												break;
											}
											//#TODO:syntax error:operator not allowed
											break;
										}
										else {
											//#TODO:syntax error:inherited class must have at least one parent
											break;
										}
										
									}
								}
								else if (ph[i + 2]->type == PhraseType::block) {
									ph[i]->subPhrase.push_back(ph[i+2]);
									ph.erase(ph.begin() + i + 1, ph.begin() + i + 3);
								}
								else {
									//#TODO:syntax error:operator not allowed
								}
							}
							else {
								//#TODO:syntax error:class must have a body
							}
						}
						else {
							//#TODO:syntax error:class must have a name
						}
					}
				}
			}
			//,function declaration
			for (int i = 0; i < ph.size(); i++) {
				if (ph[i]->type == PhraseType::keyword && ph[i]->content == "func") {
					//Phrase* p = nullptr;
					if (i + 1 < ph.size())
					{
						if (ph[i + 1]->type == PhraseType::subexpr) {
							if (i + 2 >= ph.size() || ph[i + 2]->type != PhraseType::block) {
								//#TODO:report syntax error:func decl must have body
								continue;
							}
							else {
								ph[i]->type = PhraseType::funcDecl;
								ph[i]->subPhrase.push_back(ph[i + 1]);
								ph[i]->subPhrase.push_back(ph[i + 2]);
								ph.erase(ph.begin() + i + 1, ph.begin() + i + 3);
								//p = chop(ph, PhraseType::funcDecl, i, i + 3, false);
							}
						}
						else if (ph[i + 1]->type == PhraseType::identifier) {
							if (ph[i + 2]->type == PhraseType::subexpr) {
								if (i + 3 >= ph.size() || ph[i + 3]->type != PhraseType::block) {
									//#TODO:report syntax error:func decl must have a body
									continue;
								}
								else {
									ph[i]->type = PhraseType::funcDecl;
									ph[i]->content = ph[i + 1]->content;
									delete ph[i + 1];
									ph[i]->subPhrase.push_back(ph[i + 2]);
									ph[i]->subPhrase.push_back(ph[i + 3]);
									ph.erase(ph.begin() + i + 1, ph.begin() + i + 4);
									//p = chop(ph, PhraseType::funcDecl, i, i + 4, false);
								}
							}
							else {
								//#TODO:report syntax error:func decl must have arg list
								continue;
							}
						}
						else {
							//#TODO:report syntax error:anonymous func must have arg list
							continue;
						}
					}
					/*if (p == nullptr) {
						//TODO:report syntax error:unknown func decl syntax
					}*/
				}
			}
			//index,function call
			for (int i = 1; i < ph.size(); i++) {
				if (ph[i - 1]->type == PhraseType::identifier ||
					ph[i - 1]->type == PhraseType::funcCall||
					ph[i - 1]->type == PhraseType::indexing) {
					if (ph[i]->type == PhraseType::subexpr) {
						chop(ph, PhraseType::funcCall, i - 1, i + 1, false);
						i--;
					}
					else if(ph[i]->type == PhraseType::indexer){
						chop(ph, PhraseType::indexing, i - 1, i + 1, false);
						i--;
					}
				}
			}
			//for loop
			for (int i = 2; i < ph.size(); i++) {
				if (ph[i - 1]->type == PhraseType::subexpr && ph[i - 2]->type == PhraseType::keyword && ph[i - 2]->content == "for") {
					if (ph[i]->type != PhraseType::block) {
						for (int j = i + 2; j < ph.size(); j++) {
							if (ph[j]->content == ";") {
								Phrase *p = chop(ph, PhraseType::block, i + 2, j + 1, false);
								processPhrase(p->subPhrase);
							}
						}
					}
					chop(ph, PhraseType::forLoop, i - 2, i + 1, false);
				}
			}
			//while loop
			for (int i = 2; i < ph.size(); i++) {
				if (ph[i - 1]->type == PhraseType::subexpr && ph[i - 2]->type == PhraseType::keyword && ph[i - 2]->content == "while") {
					if (ph[i]->type != PhraseType::block) {
						for (int j = i + 2; j < ph.size(); j++) {
							if (ph[j]->content == ";") {
								Phrase *p = chop(ph, PhraseType::block, i + 2, j + 1, false);
								processPhrase(p->subPhrase);
							}
						}
					}
					chop(ph, PhraseType::forLoop, i - 2, i + 1, false);
				}
			}
			//branch
			for (int i = ph.size() - 3; i >= 0; i--) {
				if (ph[i]->type == PhraseType::keyword && ph[i]->content == "if" ) {
					//pattern:[if + subexpr"()" + block"{}"
					if (ph[i + 1]->type == PhraseType::subexpr) {
						ph[i]->subPhrase.push_back(ph[i + 1]);
						if (i + 2 < ph.size()){
							if (ph[i + 2]->type != PhraseType::block) {
								for (int j = i + 2; j < ph.size(); j++) {
									if (ph[j]->content == ";") {
										Phrase *p = chop(ph, PhraseType::block, i + 2, j + 1, false);
										processPhrase(p->subPhrase);
									}
								}
							}
							ph[i]->subPhrase.push_back(ph[i + 2]);
							//pattern:[else + block"{}"]
							if (i + 4 < ph.size() && ph[i + 3]->content == "else") {
								if (ph[i + 4]->type != PhraseType::block) {
									for (int j = i + 4; j < ph.size(); j++) {
										if (ph[j]->content == ";") {
											Phrase *p = chop(ph, PhraseType::block, i + 4, j + 1, false);
											processPhrase(p->subPhrase);
										}
									}
								}
								ph[i]->subPhrase.push_back(ph[i + 4]);
								/*block
								    |->keyword"if"
									|    |->subexpr
									|    |->block(true)
									|    |->block(false)
									|    |__
									|__
								*/
								//cleanup
								//reuse keyword"else"block
								Phrase * p = ph[i + 3];
								p->content = "";
								p->type = PhraseType::block;
								p->subPhrase.push_back(ph[i]);
								ph.erase(ph.begin() + i + 1, ph.begin() + i + 5);
								ph[i] = p;
							}
							else {
								/*block
									|->keyword"if"
									|    |->subexpr
									|    |->block(true)
									|    |__
									|__
								*/
								//cleanup
								//no "useless" keyword"else" block, allocate a new block
								Phrase * p = new Phrase(PhraseType::block,"");
								p->subPhrase.push_back(ph[i]);
								ph.erase(ph.begin() + i + 1, ph.begin() + i + 3);
								ph[i] = p;
							}
						}
						else {

						}
					}
					else {

					}
				}
			}
			//seperate expressions
			{
				Phrase * p = nullptr;
				for (int begin = 0, i = 0; i <= ph.size(); i++) {
					if (i < ph.size() &&
						(ph[i]->type == PhraseType::identifier ||
						ph[i]->type == PhraseType::optr || 
						ph[i]->type == PhraseType::fconst||
						ph[i]->type == PhraseType::iconst ||
						ph[i]->type == PhraseType::string||
						ph[i]->type == PhraseType::funcCall||
						ph[i]->type == PhraseType::indexing||
						ph[i]->type == PhraseType::subexpr ||
						ph[i]->type == PhraseType::block)) {
						if (p == nullptr)
						{
							p = new Phrase(PhraseType::expr, "");
						}
						p->subPhrase.push_back(ph[i]);
					}
					else {
						if (p!=nullptr&&p->subPhrase.size() > 0) {
							ph.erase(ph.begin() + begin + 1, ph.begin() + i);
							processExpr(p->subPhrase);
							ph[begin] = p;
							p = nullptr;
							i = begin;
						}
						begin++;
					}
				}
			}
			//variable decl
			for (int i = 1; i < ph.size(); i++) {
				if (ph[i - 1]->content == "var") {
					if (ph[i]->type == PhraseType::expr) {
						ph[i - 1]->type = PhraseType::varDecl;
						ph[i - 1]->content = "";
						ph[i - 1]->subPhrase.push_back(ph[i]);
						ph.erase(ph.begin() + i, ph.begin() + i + 1);
						//chop(ph, PhraseType::varDecl, i - 1, i + 1, false);
					}
					else {
						//#TODO:report syntax error:no variable declared
					}
				}

			}
		};

		std::function<void(int)> chopTokens = [&](int startPoint) {
			int i = startPoint;
			if (ph[i]->content == "(" || ph[i]->content == "{" || ph[i]->content == "[") {
				i++;
			}
			for (; i < ph.size(); i++) {
				if (ph[i]->content == "(" || ph[i]->content == "{" || ph[i]->content == "[") {
					chopTokens(i);
				}
				else if (ph[i]->content == ")" || ph[i]->content == "}" || ph[i]->content == "]" || i == ph.size() - 1) {
					//#TODO retract expr
					if (i == ph.size() - 1) {
						chop(ph, PhraseType::block, startPoint, i + 1, false);
					}
					else {
						if (ph[i]->content == ")") {
							if (ph[startPoint]->content != "(") {

							}
							chop(ph, PhraseType::subexpr, startPoint, i + 1, true);

						}
						else {
							if (ph[i]->content == "}") {
								if (ph[startPoint]->content != "{") {
								}
								chop(ph, PhraseType::block, startPoint, i + 1, true);
							}
							else {
								if (ph[i]->content == "]") {
									if (ph[startPoint]->content != "[") {

										//#TODO:exception bracket don't match
									}
									chop(ph, PhraseType::indexer, startPoint, i + 1, true);
								}


								else {
									chop(ph, PhraseType::block, startPoint, i + 1, false);
									/*#TODO:
										expression evaluation
										funcCall
										expression array*/
								}
							}
						}
					}
					processPhrase(ph[startPoint]->subPhrase);
					return;
				}
			}
		};

		chopTokens(0);
		return ph[0];
	}


	void Parser::FindPhrasePattern(std::vector<Phrase*>& ph,const std::vector<Phrase>& pattern, std::function<void(int)> callBack)
	{
		int startPos = 0;
		int currPos = 0;
		bool match = true;
		while (startPos + currPos < ph.size()) {
			while (currPos < pattern.size()) {
				if (pattern[currPos].type == ph[currPos + startPos]->type &&
					(pattern[currPos].content=="" || pattern[currPos].content == ph[currPos + startPos]->content)) {
					if (currPos == pattern.size() - 1) {
						callBack(startPos);
						//Find a match
						/*//Chop and reconstruct
						Phrase* tp = new Phrase();
						tp->type = retType;
						for (int i = 0; i < pattern.size(); i++) {
							//Attach nodes to the first node
							//if (ph[startPos + i]->content != "") {
								tp->subPhrase.push_back(ph[startPos + i]);
							/ *}
							else {
								//Flattern empty nodes
								for (Phrase *sp : ph[startPos + i]->subPhrase) {
									tp->subPhrase.push_back(sp);
								}
								delete ph[startPos + i];
							}* /
						}
						if (pattern.size() > 1)
							ph.erase(ph.begin() + startPos + 1, ph.begin() + startPos + pattern.size());
						//Change node type
						ph[startPos]=tp;*/
						
						//TODO:return
						//return tp;
					}
					else currPos++;
				}
				else
				{
					break;
				}
			}
			currPos = 0;
			startPos++;
		}
	}

/*
	bool Parser::FindPatternByRule(std::vector<Phrase*> &ph, std::vector<ParseRule> &rules, std::function<void(Phrase*, int)>callBack, std::vector<Phrase>* specialMark, std::function<void(Phrase*)> specialCallBack)
	{
		bool changed = false;
		bool found;
		do {
			found = false;
			for (int i = 0; i < rules.size();i++) {
				Parser::Phrase * t = FindPhrasePattern(rules[i].retType, ph, rules[i].pattern, specialMark,specialCallBack);
				if (t != nullptr) {
					callBack(t,i);
					found = true;
					changed = true;
				}
			}
		} while (found);
		return changed;
	}*/

}