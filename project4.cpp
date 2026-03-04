#include <bits/stdc++.h>
using namespace std;
#define float double
#define stof stod

enum Type {none, int_type, float_type, string_type, symbol_type, 
			bool_type, reserved_words_type, quote_type, function_type,
			pair_type, list_type, defined_type, clean_environment_type,
			error_type}; 

// syntex tree
struct node {
	string token; 
	node* l;
	node* r;
	int type;
	node() {
		token = "";
		l = NULL;
		r = NULL;
		type = 0; // default none
	}
	
	node(node *give) {
		token = give->token;
		type = give->type;
		l = NULL;
		r = NULL;
	};
	
};

node* parser_tree;
node* before_edit_tree; // no use
int line, col;
int stline, stcol;
char last_char; // start form next char of already used
string token;
// define map is match by symbol
map<string, node*> global_define_map;
// only different of lambda is node, so using node to find node			
// mapping the node whose type is lambda (lambda node) -> (lambda exp)
map<node*, node*> global_function_map; 
bool eofflag = false; // true if next char is eof 
bool verbose;
vector<string> reserved_words = {"cons", "list", "quote", "define", "car", "cdr", "atom?", "pair?", 
								 "list?", "integer?", "real?", "number?", "string?", 
								 "boolean?", "symbol?", "+", "-", "*", "/", "not", "and", "or",
								 ">", ">=", "<", "<=", "=", "string-append", "string>?", "string<?", 
								 "string=?", "eqv?", "equal?", "begin", "if", "cond", "clean-environment",
							 	 "exit", "let", "lambda", "read", "write", "set!", "eval", "display-string",
							 	 "newline", "symbol->string", "number-string", "create-error-object",
							 	 "error-object?"};
int eval_s_exp(node* cur, int & errortag);
bool is_int(string token);
bool is_float(string token);
bool is_string(string token);
bool is_bool(string token);
bool is_symbol(string token);
bool is_reserved_words(string token);
void print_s_exp(node* cur, int dep);
void print_s_exp_no_endl(node* cur, int dep);
node* finding_map(string token) ;

void get_next_char(int & errortag) { 
	// check eof
	// keep track of line and row
	if (last_char == '\n') {
		line++;
		col = 0;
	}
	if (scanf("%c", &last_char)==EOF) { 
		// check every case for eof
		// if got EOF this time set eofflag to true
		// so get_token next time will set errortag = 4
		// so the setting errortag = 4 job always in get_token
		
		eofflag = true;
	}
	col++;
}

void get_useful_char(int & errortag) { 
	// ignore space, tab, \n
	// return useful char
	while (last_char == ' ' || last_char == '\t' || last_char == '\n' || last_char == '\0') {
		// the first time last_char == '\0'
		get_next_char(errortag);
		if (eofflag) {
			errortag = 4;
			return ;
		}
	}
}

void go_next_line(int & errortag) {
	while (last_char != '\n') {
		get_next_char(errortag);
		if (eofflag) {
			errortag = 4;
			return ;
		}
	}
}

void get_token(int & errortag) {
	// check the char that get last time is eof or not
	// recognize if '(', ')', '\'', ';' need to stop
	// else if "\"" need to read to another pair and need to check error
	// else if '.' need to check if it is a dot or a symbol
	// else if ';' auto run to next line and start
	// else read to ending char
	if (eofflag) {
		errortag = 4;
		return;
	}
	get_useful_char(errortag);
	if (errortag) {
		return ;
	}
	while (!errortag && last_char == ';') { // ensure start is not comment
		go_next_line(errortag);
		get_useful_char(errortag); // EOF can be double scanf
		if (eofflag) {
			// ;error\n
			errortag = 4;
			return;
		}
	}
	if (errortag) {
		return ;
	}
	stline = line;
	stcol = col;
	token = last_char;
	if (token == "(" || token == ")") {
		get_next_char(errortag);
		return ;
	}
	else if (token == "'") {
		// token = "quote";
		get_next_char(errortag);
		return ;
	}
	else if (token == ".") { 
		get_next_char(errortag);
		if (errortag) { // .EOF is errortag = 1
			errortag = 1;
			return ;
		}
		else if (last_char == ' ' || last_char == '\t' || last_char == '\n' 
			|| last_char == '(' || last_char == ')' || last_char == '\'' 
			|| last_char == ';' || last_char == '"' || eofflag == true) { 
			return ;
		}
		else { 
			// handle the symbol start form "." and float
			while (last_char != ' ' && last_char != '\t' && last_char != '\n' 
				&& last_char != '(' && last_char != ')' && last_char != '\'' 
				&& last_char != ';' && last_char != '"' && eofflag == false) {
				token += last_char;
				get_next_char(errortag);
				if (eofflag) {
					return ;
				}
			}
			return ;
		}
	}
	else if (token == "\"") { // ';' in string is ok
		get_next_char(errortag);
		while (last_char != '"' && last_char!='\n' && eofflag == false) {
			if (last_char=='\\') {
				get_next_char(errortag);
				if (last_char != '"' && (last_char=='\n' || eofflag == true)) {
					break;
				}
				if (last_char=='t') {
					token += '\t';
				}
				else if (last_char=='\\') {
					token += '\\';
				}
				else if (last_char=='n') {
					token += '\n';
				}
				else if (last_char=='\"') {
					token += '\"';
				}
				else if (last_char=='\\') {
					token += '\\';
				}
				else {
					token += '\\';
					token += last_char;
				}
				last_char = '\0';
				get_next_char(errortag);
				continue;
			}
			token += last_char;
			get_next_char(errortag);
		}
		if (last_char=='\n' || eofflag == true) { // "fef no quote
			errortag = 3;
			return ;
		} 
		token += "\"";
		get_next_char(errortag);
		return ;
	}
	
	token = "";
	while (last_char != ' ' && last_char != '\t' && last_char != '\n' 
			&& last_char != '(' && last_char != ')' && last_char != '\'' 
			&& last_char != ';' && last_char != '\"' && eofflag == false) {
		// the symbol and the number need to be distinguish here or maybe not
		token += last_char;
		get_next_char(errortag);
		if (eofflag) {
			break;
		}
	}
	// special case
	if (token == "#f") {
		token = "nil";
	}
	else if (token == "t") {
		token = "#t";
	}
}

int get_type(string s) {
	if (is_int(s)) {
		return int_type;
	}
	if (is_float(s)) {
		return float_type;
	}
	if (is_string(s)) {
		return string_type;
	}
	if (is_bool(s)) {
		return bool_type;
	}
	if (is_symbol(s)) {
		return symbol_type;
	}
	cout << "ERROR : no type" << endl;
	return none;
}

node* s_exp(node* cur, int & errortag) {
	node* head = cur;
	if (errortag) {
		return cur;
	}
	if (token == "(") {
		get_token(errortag);
		if (errortag) {
			return cur;
		}
		if (token == ")") { // () case
			// the parent layer will set nil to -1
			cur->token = "nil";
			cur->type = bool_type;
			return cur;
		}
		
		cur->l = new node();
		cur->l = s_exp(cur->l, errortag);
		if (errortag) {
			return cur;
		}
		get_token(errortag);
		if (errortag) {
			return cur;
		}
		else if (token == ")") { 
			return cur;
		}
		else if (token == ".") {
			// it same case with many s-exp 
		}
		else {
			while (token != ")" && token != ".") {
				cur->r = new node();
				cur = cur->r;
				cur->l = new node();
				cur->l = s_exp(cur->l, errortag);
				if (errortag) {
					return cur;
				}
				get_token(errortag);
				if (errortag) {
					return cur;
				}
			}
		}
	}
	else if (token == "'") {
		cur->l = new node();
		cur->l->token = "quote";
		cur->l->type = symbol_type;
		cur->r = new node();
		cur->r->l = new node();
		get_token(errortag);
		if (errortag) {
			return cur;
		}
		cur->r->l = s_exp(cur->r->l, errortag);
		if (errortag) {
			return cur;
		}
		return cur;
	}
	else if (token == "." || token == ")") {
		errortag = 1;
		return cur;
	}
	else {
		// atom
		cur->token = token;
		cur->type = get_type(token);
		return cur;
	}
	
	if (token == ".") {
		get_token(errortag);
		if (errortag) {
			return cur;
		}
		cur->r = new node();
		cur->r = s_exp(cur->r, errortag);
		if (errortag) {
			return cur;
		}
		get_token(errortag); 
		if (token != ")" || errortag == 4) {
			// ( 1
			errortag = 2;
			return cur;
		}
		else if (cur->r->token == "nil" || 
				cur->r->token == "#f") {
			// (1 . ())
			cur->r = NULL;
		}
	}
	return head;
}

bool check_arg_numbers(node* cur, int numbers) {
	if (cur == NULL) { 
		if (numbers==0) {
			return false; 
		}
		return true;
	}
	if (cur->l == NULL) {
		if (numbers==1) {
			return false; 
		}
		return true;
	}
	cur = cur->r;
	int i;
	for (i = 1; i <= numbers && cur != NULL; i++) {
		cur = cur->r;
	}	
	if (i != numbers || cur != NULL) {
		return false;
	}
	return true;
}

bool is_float(string token) {
	int i = 0;
	bool find = false;
	if (token[0]=='+' || token[0]=='-') {
		i++;
	}
	for (; i < token.size() && token[i] >= '0' && token[i] <= '9'; i++, find = true) ;
	if (i < token.size() && token[i]=='.') {
		i++;
	}
	else {
		return false;
	}
	for (; i < token.size() && token[i] >= '0' && token[i] <= '9'; i++, find = true) ;
	if ((i == token.size() || (token[i]=='f' && i+1 == token.size()) || 
		(token[i]=='d' && i+1 == token.size())) && find) {
		// 3f || 3d == 3.000
		return true;
	}
	return false;
}

bool is_int(string token) {
	int i = 0;
	bool find = false;
	if (token[0]=='+' || token[0]=='-') {
		i++;
	}
	for (; i < token.size() && token[i] >= '0' && token[i] <= '9'; i++, find = true) ;
	if (i == token.size() && find) {
		return true;
	}
	return false;
}

bool is_string(string token) {
	return token[0] == '"';
}


bool is_bool(string token) {
	// only #t, nil
	return token == "#t" || token == "nil";
}

bool is_symbol(string token) {
	return !is_int(token) && !is_float(token) && !is_bool(token) && !is_string(token) && token != "";
}

bool is_reserved_words(string token) {
	return find(reserved_words.begin(), reserved_words.end(), token) != reserved_words.end();
}

bool is_pure_list(node* cur) {
	while (cur != NULL) {
		if (cur->l==NULL) {
			return false;
		}
		cur = cur->r;
	}
	return true;
}

bool is_top_level(node* cur) {
	return cur == parser_tree;
}

node* create_subtree(node* head) {
    if (!head) {
    	return NULL;
	}
    node* newNode = new node(head);
    newNode->l = create_subtree(head->l);  
    newNode->r = create_subtree(head->r);
    return newNode;
}

class S_EXP {
	
public: 
	// define map mean local varible
	// so if getting varible first here and next global
	map<string, node*> define_map;
	
	S_EXP(map<string, node*> original_define_map) {
		define_map = original_define_map;
	};
	
	node* do_cons(node* cur, int & errortag) {
		cur->l = eval_s_exp(cur->l, errortag);
		if (errortag) {
			if (errortag == 9) {
				errortag = 18;
			}
			return cur->l;
		}
		cur->r = eval_s_exp(cur->r->l, errortag);
		if (errortag) {
			if (errortag == 9) {
				errortag = 18;
			}
			return cur->r;
		}
		if (cur->r->token == "nil") {
			cur->r = NULL;
		}
		return cur;
	}
	
	node* do_list(node* cur, int & errortag) {
		node* t = cur;
		while (t != NULL) {
			t->l = eval_s_exp(t->l, errortag);
			if (errortag) {
				if (errortag == 9) {
					errortag = 18;
				}
				return t->l;
			}
			t = t->r;
		}
		return cur;
	}
	
	bool define_format(node* cur) {
		if (check_arg_numbers(cur, 1) || check_arg_numbers(cur, 2)) {
			return false;
		}
		if (cur->r->l->token != "") {
			if (!check_arg_numbers(cur, 3)) {
		 		return false;
		 	}
			if (cur->r->l->token == "" ||
				cur->r->l->type!=symbol_type) {
				return false;
			}
			if (is_reserved_words(cur->r->l->token)) {
				return false;
		 	}
		}
		else {
			node* t = cur->r;
			node* tt = t->l;
			if (tt->token != "nil") {
				if (!is_pure_list(tt)) {
					return false;;
				}
				while (tt!=NULL) {
					if (tt->l->type != symbol_type) {	
						return false;
					}
					if (is_reserved_words(tt->l->token)) {
						return false;
				 	}
					tt = tt->r;
				}
			}
			t = t->r;
			if (t->l == NULL) {
				return false;
			}
		}
		return true;
	}
	
	node* do_define(node* cur, int & errortag) {
		node* t = cur->r;
		if (cur->r->l->token != "") {
			node* a = eval_s_exp(t->r->l, errortag);
			if (errortag) {
				// top level must no return value
				return a;
			}
			if (define_map[t->l->token]!=NULL) {
				define_map[t->l->token] = a;
			}
			else {
				global_define_map[t->l->token] = a;
			}
			cur->type = defined_type;
			cur->token = t->l->token + " defined";
		}
		else {
			node* tt = t->l;
			t->l = tt->r; // first is name second is arg or -1
			cur->l->token = tt->l->token;
			cur->l->type = function_type; // cur->l is a function node mapping the function
			global_function_map[cur->l] = cur->r; // map root->l to root->r which is function_type
			if (define_map[cur->l->token]!=NULL) {
				define_map[cur->l->token] = cur->l;
			}
			else {
				global_define_map[cur->l->token] = cur->l;
			}
			cur->type = defined_type;
			cur->token = cur->l->token + " defined";
		}
		cur->l = NULL;
		cur->r = NULL;
		return cur;
	}
	
	node* do_set(node* cur, int & errortag) {
		node* t = cur->r;
		node* a = eval_s_exp(t->r->l, errortag);
		if (errortag) {
			return a;
		}
		
		if (define_map[t->l->token]!=NULL) {
			define_map[t->l->token] = a;
		}
		else {
			global_define_map[t->l->token] = a;
		}
		return a;
	}
	
	node* do_car(node* cur, int & errortag) {
		cur->r->l = eval_s_exp(cur->r->l, errortag);
		if (errortag) {
			if (errortag == 9) {
				errortag = 18;
			}
			return cur->r->l;
		}
		if (cur->r->l->token != "") {
			errortag = 7;
			return cur;
		}
		return cur->r->l->l;
	}
	
	node* do_cdr(node* cur, int & errortag) {
		cur->r->l = eval_s_exp(cur->r->l, errortag);
		if (errortag) {
			if (errortag == 9) {
				errortag = 18;
			}
			return cur->r->l;
		}
		if (cur->r->l->token != "") {
			errortag = 7;
			return cur;
		}
		if (cur->r->l->r == NULL) {
			cur->token = "nil";
			return cur;
		}
		return cur->r->l->r;
	}
	
	node* check_pair(node* cur, int & errortag) {
		// only (pair? 3) nil
		cur->r->l = eval_s_exp(cur->r->l, errortag);
		if (errortag) {
			if (errortag == 9) {
				errortag = 18;
			}
			return cur->r->l;
		}
		if (cur->r->l->token != "") {
			cur->token = "nil";
		}
		else {
			cur->token = "#t";
		}
		cur->type = bool_type;
		cur->l = NULL;
		cur->r = NULL;
		return cur;
	}
	
	node* check_list(node* cur, int & errortag) {
		cur->r->l = eval_s_exp(cur->r->l, errortag);
		if (errortag) {
			if (errortag == 9) {
				errortag = 18;
			}
			return cur->r->l;
		}
		if (!is_pure_list(cur->r->l) || 
			cur->r->l->token != "") {
			cur->token = "nil";
		}
		else {
			cur->token = "#t";
		}
		cur->type = bool_type;
		cur->l = NULL;
		cur->r = NULL;
		return cur;
	}
	
	node* check_atom(node* cur, int & errortag) {
		cur->r->l = eval_s_exp(cur->r->l, errortag);
		if (errortag) {
			if (errortag == 9) {
				errortag = 18;
			}
			return cur->r->l;
		}
		if (cur->r->l->token == "") {
			cur->token = "nil";
		}
		else {
			cur->token = "#t";
		}
		cur->type = bool_type;
		cur->l = NULL;
		cur->r = NULL;
		return cur;
	}
	
	node* check_null(node* cur, int & errortag) {
		cur->r->l = eval_s_exp(cur->r->l, errortag);
		if (errortag) {
			if (errortag == 9) {
				errortag = 18;
			}
			return cur->r->l;
		}
		if (cur->r->l->token != "nil") {
			cur->token = "nil";
		}
		else {
			cur->token = "#t";
		}
		cur->type = bool_type;
		cur->l = NULL;
		cur->r = NULL;
		return cur;
	}
	
	node* check_int(node* cur, int & errortag) {
		cur->r->l = eval_s_exp(cur->r->l, errortag);
		if (errortag) {
			if (errortag == 9) {
				errortag = 18;
			}
			return cur->r->l;
		}
		if (cur->r->l->type == int_type) {
			cur->token = "#t";
		}
		else {
			cur->token = "nil";
		}
		cur->type = bool_type;
		cur->l = NULL;
		cur->r = NULL;
		return cur;
	}
	
	node* check_real(node* cur, int & errortag) {
		cur->r->l = eval_s_exp(cur->r->l, errortag);
		if (errortag) {
			if (errortag == 9) {
				errortag = 18;
			}
			return cur->r->l;
		}
		if (cur->r->l->type == int_type || 
			cur->r->l->type == float_type) {
			cur->token = "#t";
		}
		else {
			cur->token = "nil";
		}
		cur->type = bool_type;
		cur->l = NULL;
		cur->r = NULL;
		return cur;
	}
	
	node* check_string(node* cur, int & errortag) {
		cur->r->l = eval_s_exp(cur->r->l, errortag);
		if (errortag) {
			if (errortag == 9) {
				errortag = 18;
			}
			return cur->r->l;
		}
		if (cur->r->l->type == string_type) {
			cur->token = "#t";
		}
		else {
			cur->token = "nil";
		}
		cur->type = bool_type;
		cur->l = NULL;
		cur->r = NULL;
		return cur;
	}
	
	node* check_bool(node* cur, int & errortag) {
		cur->r->l = eval_s_exp(cur->r->l, errortag);
		if (errortag) {
			if (errortag == 9) {
				errortag = 18;
			}
			return cur->r->l;
		}
		if (cur->r->l->type == bool_type) {
			cur->token = "#t";
		}
		else {
			cur->token = "nil";
		}
		cur->type = bool_type;
		cur->l = NULL;
		cur->r = NULL;
		return cur;
	}
	
	node* check_symbol(node* cur, int & errortag) {
		cur->r->l = eval_s_exp(cur->r->l, errortag);
		if (errortag) {
			if (errortag == 9) {
				errortag = 18;
			}
			return cur->r->l;
		}
		if (cur->r->l->type == symbol_type || 
			cur->r->l->type == reserved_words_type) {
			cur->token = "#t";
		}
		else {
			cur->token = "nil";
		}
		cur->type = bool_type;
		cur->l = NULL;
		cur->r = NULL;
		return cur;
	}
	
	node* check_verbose(node* cur) {
		if (verbose==true) {
			cur->token = "#t";
		}
		else {
			cur->token = "nil";
		}
		cur->type = bool_type;
		cur->l = NULL;
		cur->r = NULL;
		return cur;
	}
	
	node* plus_operation(node* cur, int & errortag) {
		double ans = 0;
		node* t = cur->r;
		bool get_float = false;
		while (t != NULL) {
			t->l = eval_s_exp(t->l, errortag);
			if (errortag) {
				if (errortag == 9) {
					errortag = 18;
				}
				return t->l;
			}
			if (t->l->type != int_type && 
				t->l->type != float_type) {
				errortag = 7;
				cur->r->l = t->l;
				return cur;
			}
			get_float |= t->l->type == float_type;
			if (!get_float) {
	            ans = int(ans);
	        }
			ans += stod(t->l->token);
			t = t->r;
		}
		if (!get_float) {
			cur->type = int_type;
			cur->token = to_string(int(ans));
		}
		else {
			cur->type = float_type;
			cur->token = to_string(ans);
		}
		cur->l = NULL;
		cur->r = NULL;
		return cur;
	}
	
	node* minus_operation(node* cur, int & errortag) {
		double ans = 0;
		node* t = cur->r;
		bool get_float = false;
		t->l = eval_s_exp(t->l, errortag);
		if (errortag) {
			if (errortag == 9) {
				errortag = 18;
			}
			return t->l;
		}	
		if (t->l->type != int_type && 
			t->l->type != float_type) {
			errortag = 7;
			cur->r->l = t->l;
			return cur;
		}
		get_float |= t->l->type == float_type;
		if(!get_float) { 
	        ans = int(ans);
	    }
	    ans = stod(t->l->token);    
		t = t->r;
		while (t != NULL) {
			t->l = eval_s_exp(t->l, errortag);
			if (errortag) {
				if (errortag == 9) {
					errortag = 18;
				}
				return t->l;
			}	
			if (t->l->type != int_type && 
				t->l->type != float_type) {
				errortag = 7;
				cur->r->l = t->l;
				return cur;
			}
			get_float |= t->l->type == float_type;
			if (!get_float) {
	            ans = int(ans);
	        }
			ans -= stof(t->l->token);
			t = t->r;
		}
		if (!get_float) {
			cur->type = int_type;
			cur->token = to_string(int(ans));
		}
		else {
			cur->type = float_type;
			cur->token = to_string(ans);
		}
		cur->l = NULL;
		cur->r = NULL;
		return cur;
	}
	
	node* mul_operation(node* cur, int & errortag) {
		float ans = 1;
		node* t = cur->r;
		bool get_float = false;
		while (t != NULL) {
			t->l = eval_s_exp(t->l, errortag);
			if (errortag) {
				if (errortag == 9) {
					errortag = 18;
				}
				return t->l;
			}	
			if (t->l->type != int_type && 
				t->l->type != float_type) {
				errortag = 7;
				cur->r->l = t->l;
				return cur;
			}
			get_float |= t->l->type == float_type;
			if (!get_float) {
	            ans = int(ans);
	        }
			ans *= stof(t->l->token);
			t = t->r;
		}
		if (!get_float) {
			cur->type = int_type;
			cur->token = to_string(int(ans));
		}
		else {
			cur->type = float_type;
			cur->token = to_string(ans);
		}
		cur->l = NULL;
		cur->r = NULL;
		return cur;
	}
	
	node* div_operation(node* cur, int & errortag) {
		float ans = 0;
		node* t = cur->r;
		t->l = eval_s_exp(t->l, errortag);
	    bool get_float = false;
		if (errortag) {
			if (errortag == 9) {
				errortag = 18;
			}
			return t->l;
		}	
	    if (t->l->type != int_type && 
			t->l->type != float_type) {
			errortag = 7;
			cur->r->l = t->l;
			return cur;
		}
		get_float |= t->l->type == float_type;
	    ans = stof(t->l->token);
	    if(!get_float) { 
	        ans = int(ans);
	    }
		t = t->r;
		while (t != NULL) {
			t->l = eval_s_exp(t->l, errortag);
			if (errortag) {
				if (errortag == 9) {
					errortag = 18;
				}
				return t->l;
			}	
			if (stof(t->l->token) == 0) {
				errortag = 11;
				return cur->l;
			}
	        if (t->l->type != int_type && 
				t->l->type != float_type) {
				errortag = 7;
				cur->r->l = t->l;
				return cur;
			}
			get_float |= t->l->type == float_type;
			if (!get_float) {
	            ans = int(ans);
	        }
			ans /= stof(t->l->token);
			t = t->r;
		}
		if (get_float) {
			cur->type = float_type;
			cur->token = to_string(ans);
		}
		else {
			cur->type = int_type;
			cur->token = to_string(int(ans));
		}
		cur->l = NULL;
		cur->r = NULL;
		return cur;
	}
	
	node* not_operation(node* cur, int & errortag) {
		cur->r->l = eval_s_exp(cur->r->l, errortag);
		if (errortag) {
			if (errortag == 9) {
				errortag = 18;
			}
			return cur->r->l;
		}
		if (cur->r->l->token == "nil") {
			cur->token = "#t";
		}
		else {
			cur->token = "nil";
		}
		cur->type = bool_type;
		cur->l = NULL;
		cur->r = NULL;
		return cur;
	}
	
	node* and_operation(node* cur, int & errortag) {
		node* t = cur;
		node* pre = t;
		while (t != NULL) {
			t->l = eval_s_exp(t->l, errortag);
			if (errortag) {
				if (errortag == 9) {
					errortag = 20;
				}
				return t->l;
			}	
			if (t->l->token == "nil") {
				return t->l;
			}
			pre = t;
			t = t->r;
		}
		return pre->l;
	}
	
	node* or_operation(node* cur, int & errortag) {
		node* t = cur;
		node* pre = t;
		while (t != NULL) {
			t->l = eval_s_exp(t->l, errortag);
			if (errortag) {
				if (errortag == 9) {
					errortag = 20;
				}
				return t->l;
			}	
			if (t->l->token != "nil") {
				return t->l;
			}
			pre = t;
			t = t->r;
		}
		return pre->l;
	}
	
	node* gt_operation(node* cur, int & errortag) {
		node* t = cur->r;
		float prenum, curnum;
		bool ans = true;
		t->l = eval_s_exp(t->l, errortag);
		if (errortag) {
			if (errortag == 9) {
				errortag = 18;
			}
			return t->l;
		}
		if (t->l->type==int_type || 
			t->l->type==float_type) {
			prenum = stof(t->l->token);
		}
		else {
			errortag = 7;
			cur->r->l = t->l;
			return cur;
		}
		t = t->r;
		while (t != NULL) {
			t->l = eval_s_exp(t->l, errortag);
			if (errortag) {
				if (errortag == 9) {
					errortag = 18;
				}
				return t->l;
			}
			if (t->l->type==int_type || 
				t->l->type==float_type) {
				curnum = stof(t->l->token);
			}
			else {
				errortag = 7;
				cur->r->l = t->l;
				return cur;
			}
			ans &= prenum > curnum;
			prenum = curnum;
			t = t->r;
		}
		cur->type = bool_type;
		cur->token = ans ? "#t" : "nil";
		cur->l = NULL;
		cur->r = NULL;
		return cur;
	}
	
	node* ge_operation(node* cur, int & errortag) {
		node* t = cur->r;
		float prenum, curnum;
		bool ans = true;
		t->l = eval_s_exp(t->l, errortag);
		if (errortag) {
			if (errortag == 9) {
				errortag = 18;
			}
			return t->l;
		}
		if (t->l->type == int_type || 
			t->l->type == float_type) {
			prenum = stof(t->l->token);
		}
		else {
			errortag = 7;
			cur->r->l = t->l;
			return cur;
		}
		t = t->r;
		while (t != NULL) {
			t->l = eval_s_exp(t->l, errortag);
			if (errortag) {
				if (errortag == 9) {
					errortag = 18;
				}
				return t->l;
			}
			if (t->l->type == int_type || 
				t->l->type == float_type) {
				curnum = stof(t->l->token);
			}
			else {
				errortag = 7;
				cur->r->l = t->l;
				return cur;
			}
			ans &= prenum >= curnum;
			prenum = curnum;
			t = t->r;
		}
		cur->type = bool_type;
		cur->token = ans ? "#t" : "nil";
		cur->l = NULL;
		cur->r = NULL;
		return cur;
	}
	
	node* lt_operation(node* cur, int & errortag) {
		node* t = cur->r;
		float prenum, curnum;
		bool ans = true;
		t->l = eval_s_exp(t->l, errortag);
		if (errortag) {
			if (errortag == 9) {
				errortag = 18;
			}
			return t->l;
		}
		if (t->l->type == int_type || 
			t->l->type == float_type) {
			prenum = stof(t->l->token);
		}
		else {
			errortag = 7;
			cur->r->l = t->l;
			return cur;
		}
		t = t->r;
		while (t != NULL) {
			t->l = eval_s_exp(t->l, errortag);
			if (errortag) {
				if (errortag == 9) {
					errortag = 18;
				}
				return t->l;
			}
			if (t->l->type == int_type || 
				t->l->type == float_type) {
				curnum = stof(t->l->token);
			}
			else {
				errortag = 7;
				cur->r->l = t->l;
				return cur;
			}
			ans &= prenum < curnum;
			prenum = curnum;
			t = t->r;
		}
		cur->type = bool_type;
		cur->token = ans ? "#t" : "nil";
		cur->l = NULL;
		cur->r = NULL;
		return cur;
	}
	
	node* le_operation(node* cur, int & errortag) {
		node* t = cur->r;
		float prenum, curnum;
		bool ans = true;
		t->l = eval_s_exp(t->l, errortag);
		if (errortag) {
			if (errortag == 9) {
				errortag = 18;
			}
			return t->l;
		}
		if (t->l->type == int_type || 
			t->l->type == float_type) {
			prenum = stof(t->l->token);
		}
		else {
			errortag = 7;
			cur->r->l = t->l;
			return cur;
		}
		t = t->r;
		while (t != NULL) {
			t->l = eval_s_exp(t->l, errortag);
			if (errortag) {
				if (errortag == 9) {
					errortag = 18;
				}
				return t->l;
			}
			if (t->l->type == int_type || 
				t->l->type == float_type) {
				curnum = stof(t->l->token);
			}
			else {
				errortag = 7;
				cur->r->l = t->l;
				return cur;
			}
			ans &= prenum <= curnum;
			prenum = curnum;
			t = t->r;
		}
		cur->type = bool_type;
		cur->token = ans ? "#t" : "nil";
		cur->l = NULL;
		cur->r = NULL;
		return cur;
	}
	
	node* eq_operation(node* cur, int & errortag) {
		node* t = cur->r;
		float prenum, curnum;
		bool ans = true;
		t->l = eval_s_exp(t->l, errortag);
		if (errortag) {
			if (errortag == 9) {
				errortag = 18;
			}
			return t->l;
		}
		if (t->l->type == int_type || 
			t->l->type == float_type) {
			prenum = stof(t->l->token);
		}
		else {
			errortag = 7;
			cur->r->l = t->l;
			return cur;
		}
		t = t->r;
		while (t != NULL) {
			t->l = eval_s_exp(t->l, errortag);
			if (errortag) {
				if (errortag == 9) {
					errortag = 18;
				}
				return t->l;
			}
			if (t->l->type == int_type || 
				t->l->type == float_type) {
				curnum = stof(t->l->token);
			}
			else {
				errortag = 7;
				cur->r->l = t->l;
				return cur;
			}
			ans &= prenum == curnum;
			prenum = curnum;
			t = t->r;
		}
		cur->type = bool_type;
		cur->token = ans ? "#t" : "nil";
		cur->l = NULL;
		cur->r = NULL;
		return cur;
	}
	
	node* string_append(node* cur, int & errortag) {
		string ans = "\"";
		node* t = cur->r;
		while (t != NULL) {
			t->l = eval_s_exp(t->l, errortag);
			if (errortag) {
				if (errortag == 9) {
					errortag = 18;
				}
				return t->l;
			}	
			if (t->l->type!=string_type) {
				errortag = 7;
				cur->r->l = t->l;
				return cur;
			}
			t->l->token.erase(t->l->token.begin());
			t->l->token.erase(t->l->token.end()-1);
			ans += t->l->token;
			t = t->r;
		}
		cur->token = ans + "\"";
		cur->type = string_type;
		cur->l = NULL;
		cur->r = NULL;
		return cur;
	}
	
	node* strlt_operation(node* cur, int & errortag) {
		node* t = cur->r;
		string prestr, curstr;
		bool ans = true;
		t->l = eval_s_exp(t->l, errortag);
		if (errortag) {
			if (errortag == 9) {
				errortag = 18;
			}
			return t->l;
		}
		if (t->l->type == string_type) {
			prestr = t->l->token;
		}
		else {
			errortag = 7;
			cur->r->l = t->l;
			return cur;
		}
		t = t->r;
		while (t != NULL) {
			t->l = eval_s_exp(t->l, errortag);
			if (errortag) {
				if (errortag == 9) {
					errortag = 18;
				}
				return t->l;
			}
			if (t->l->type == string_type) {
				curstr = t->l->token;
			}
			else {
				errortag = 7;
				cur->r->l = t->l;
				return cur;
			}
			ans &= prestr < curstr;
			prestr = curstr;
			t = t->r;
		}
		cur->type = bool_type;
		cur->token = ans ? "#t" : "nil";
		cur->l = NULL;
		cur->r = NULL;
		return cur;
	}
	
	node* strgt_operation(node* cur, int & errortag) {
		node* t = cur->r;
		string prestr, curstr;
		bool ans = true;
		t->l = eval_s_exp(t->l, errortag);
		if (errortag) {
			if (errortag == 9) {
				errortag = 18;
			}
			return t->l;
		}
		if (t->l->type == string_type) {
			prestr = t->l->token;
		}
		else {
			errortag = 7;
			cur->r->l = t->l;
			return cur;
		}
		t = t->r;
		while (t != NULL) {
			t->l = eval_s_exp(t->l, errortag);
			if (errortag) {
				if (errortag == 9) {
					errortag = 18;
				}
				return t->l;
			}
			if (t->l->type == string_type) {
				curstr = t->l->token;
			}
			else {
				errortag = 7;
				cur->r->l = t->l;
				return cur;
			}
			ans &= prestr > curstr;
			prestr = curstr;
			t = t->r;
		}
		cur->type = bool_type;
		cur->token = ans ? "#t" : "nil";
		cur->l = NULL;
		cur->r = NULL;
		return cur;
	}
	
	node* streq_operation(node* cur, int & errortag) {
		node* t = cur->r;
		string prestr, curstr;
		bool ans = true;
		t->l = eval_s_exp(t->l, errortag);
		if (errortag) {
			if (errortag == 9) {
				errortag = 18;
			}
			return t->l;
		}
		if (t->l->type == string_type) {
			prestr = t->l->token;
		}
		else {
			errortag = 7;
			cur->r->l = t->l;
			return cur;
		}
		t = t->r;
		while (t != NULL) {
			t->l = eval_s_exp(t->l, errortag);
			if (errortag) {
				if (errortag == 9) {
					errortag = 18;
				}
				return t->l;
			}
			if (t->l->type == string_type) {
				curstr = t->l->token;
			}
			else {
				errortag = 7;
				cur->r->l = t->l;
				return cur;
			}
			ans &= prestr == curstr;
			prestr = curstr;
			t = t->r;
		}
		cur->type = bool_type;
		cur->token = ans ? "#t" : "nil";
		cur->l = NULL;
		cur->r = NULL;
		return cur;
	}
	
	node* eqv_operation(node* cur, int & errortag) {
		node* t = cur;
		node* a;
		node* b;
		a = eval_s_exp(t->r->l, errortag);
		if (errortag) {
			if (errortag == 9) {
				errortag = 18;
			}
			return a;
		}
		t = t->r;
		b = eval_s_exp(t->r->l, errortag);
		if (errortag) {
			if (errortag == 9) {
				errortag = 18;
			}
			return b;
		}
		if (a == b) {
			cur->token = "#t";
		}
	    else if (a->token != "") {
	        if (a->token == b->token) {
	        	if (t->l->type == string_type) {
	        		cur->token = "nil";
	        	}
	        	else {
	        		cur->token = "#t";
	        	}
	        }
	        else {
	            cur->token = "nil";
	        }
	    }
		else {
			cur->token = "nil";
		}
		cur->type = bool_type;
		cur->l = NULL;
		cur->r = NULL;
		return cur;
	}
	
	bool check_same_tree(node* cur1, node* cur2) {
		if (cur1==NULL || cur2==NULL) {
			if (cur1==cur2) {
				return true;
			}
			else {
				return false;
			}
		}
		if (cur1->token != cur2->token) {
			return false;
		}
		bool l = check_same_tree(cur1->l, cur2->l); 
		bool r = check_same_tree(cur1->r, cur2->r);
		return l && r;
	}
	
	node* equal_operation(node* cur, int & errortag) {
		node* t = cur;
		node* a;
		node* b;
		t->r->l = eval_s_exp(t->r->l, errortag);
		if (errortag) {
			if (errortag == 9) {
				errortag = 18;
			}
			return t->r->l;
		}
		a = t->r->l;
		t = t->r;
		t->r->l = eval_s_exp(t->r->l, errortag);
		if (errortag) {
			if (errortag == 9) {
				errortag = 18;
			}
			return t->r->l;
		}
		b = t->r->l;
		if (check_same_tree(a, b)) {
			cur->token = "#t";
		}
		else {
			cur->token = "nil";
		}
		cur->type = bool_type;
		cur->l = NULL;
		cur->r = NULL;
		return cur;
	}
	
	node* begin_operation(node* cur, int & errortag) {
		node* ori = create_subtree(cur);
		node* t = cur->r;
		node* pre = t;
		while (t != NULL) {
			t->l = eval_s_exp(t->l, errortag);
			if (t->l->type == defined_type || t->l->type == clean_environment_type) {
				if (verbose) {
					cout << t->l->token << endl;
				}
				errortag = 9;
			}
			if (errortag) {
				if (errortag == 9) {
					if (t->r==NULL) {
						errortag = 9;
						return ori;
					}
					else {
						errortag = 0;
					}
				}
				else {
					return t->l;
				}
			}	
			pre = t;
			t = t->r;
		}
		return pre->l;
	}
	
	node* if_operation(node* cur, int & errortag) {
		node* t = cur;
		node* tr;
		node* fa;
		t = t->r;
		node* ori = create_subtree(cur);
		t->l = eval_s_exp(t->l, errortag);
		if (errortag) {
			if (errortag == 9) {
				errortag = 18;
			}
			return t->l;
		}	
		if (t->l->token != "nil") {
			t = t->r;
			tr = eval_s_exp(t->l, errortag);
			if (errortag) {
				if (errortag == 9) {
					return ori;
				}
				return tr;
			}
			return tr;
		}
		// false;
		t = t->r;
		if (t==NULL) {
			errortag = 9;
			return ori;
		}
		t = t->r;
		if (t==NULL) {
			errortag = 9;
			return ori;
		}
		fa = eval_s_exp(t->l, errortag);
		if (errortag) {
			if (errortag == 9) {
				return ori;
			}
			return fa;
		}
		return fa;
	}
	
	void cond_format(node* cur, int & errortag) {
		node* t = cur;
		node* tt;
		t = cur->r;
		while (t != NULL) {
			tt = t->l;
			if (tt->token != "") {
				errortag = 14;
				return ;
			}
			// check format no matter true false
			tt = tt->r;
			if (tt==NULL) {
				errortag = 14;
				return ;
			}
	        t = t->r;
		}
	}
	
	node* cond_operation(node* cur, int & errortag) {
	    // if more than one things get after true get the last one
		node* t = cur;
		node* tt;
		t = cur->r;
		bool get_else;
	    node* ori = create_subtree(cur);
		while (t != NULL) {
			tt = t->l;
			get_else = tt->l->token == "else";
			tt->l = eval_s_exp(tt->l, errortag);
			if (get_else && t->r==NULL) {
				errortag = 0;
			}
			if (errortag) {
				if (errortag == 9) {
					errortag = 18;
				}
				return tt->l;
			}
			if (tt->l->token == "nil") {
				t = t->r;
				if (get_else && t==NULL) {	
					// last else then no continue
				}
				else {
					continue;
				}
			}
	        
	        // the condition is true, start find the answer
			tt = tt->r;
			if (tt==NULL) {
				errortag = 14;
				return cur;
			}
			
	        while (true) {
	            tt->l = eval_s_exp(tt->l, errortag);
	            if (tt->l->type == defined_type || tt->l->type == clean_environment_type) {
					if (verbose) {
						cout << tt->l->token << endl;
					}
					errortag = 9;
				}
	            if (errortag) {
		            if (errortag == 9) {
		            	if (tt->r == NULL) {
		                	return ori;
		                }
		                errortag = 0;
		            }
		        	else {
		            	return tt->l;
		            }
	            }
	            if (tt->r == NULL) {
	                break;
	            }
	            tt = tt->r;
	        }
			return tt->l; // success
		}
		errortag = 9;
		return ori;
	}
	
	bool let_format(node* cur) {
		node* t = cur->r;
		node* tt = t->l;
		while (tt!=NULL && tt->token != "nil") {
			if (!is_pure_list(tt->l) || 
				!check_arg_numbers(tt->l, 2)) {	
				return false;
			}
			// start with two arguments
			// check first argument is symbol
			node* ttt = tt->l;
			if (ttt->l->type != symbol_type) {
				return false;
			}
			tt = tt->r;
		}
		t = t->r;
		if (t->l == NULL) {
			return false;
		}
		return true;
	}
	
	node* let_operation(node* cur, int & errortag) {
		map<string, node*> cur_define_map = define_map;
	    node* t = cur->r;
	    node* tt = t->l;
	    node* ori = create_subtree(cur);
	    
	    while (tt!=NULL && tt->token != "nil") {
	    	node* ttt = tt->l;
	    	if (ttt->l->type!=symbol_type) {
				errortag = 16;
				return cur;
			}
			string key = ttt->l->token;
			ttt = ttt->r;
			
	    	node* a = eval_s_exp(ttt->l, errortag);
	    	
			if (errortag) {
				// like define this is no return value
				// but fixed
				if (errortag == 9) {
					errortag = 21;
				}
				return a;
			}
			cur_define_map[key] = a;
			tt = tt->r;
	    }
		t = t->r;
		node* pre = NULL;
		// if someone is edit keep use the edit version
		S_EXP S_exp = S_EXP(cur_define_map);
		while (t != NULL) {
			t->l = S_exp.eval_s_exp(t->l, errortag);
			if (t->l->type == defined_type || t->l->type == clean_environment_type) {
				if (verbose) {
					cout << t->l->token << endl;
				}
				errortag = 9;
			}
			if (errortag) {
				if (errortag == 9) {
					if (t->r==NULL) {
						return ori;
					}
					else {
						errortag = 0;
					}
				}
				else {
					return t->l;
				}
			}
			pre = t->l;
			t = t->r;
		}
		if (pre == NULL) {
			errortag = 9;
			return ori;
		}
		return pre;
	}
	
	bool lambda_format(node* cur) {
		node* t = cur->r;
		node* tt = t->l;
		if (check_arg_numbers(cur, 1) || check_arg_numbers(cur, 2)) {
			return false;
		}
		if (tt->token != "nil") {
			if (!is_pure_list(tt)) {
				return false;;
			}
			while (tt!=NULL) {
				if (tt->l->type != symbol_type) {	
					return false;
				}
				tt = tt->r;
			}
		}
		t = t->r;
		if (t->l == NULL) {
			return false;
		}
		return true;
	}
	
	node* lambda_operation(node* cur, int & errortag) {
		// dont need to eval here
		// adding current node into function_map
		// so if anyone call current node jump over to function_map to find
		cur->token = "lambda";
		cur->type = function_type; // so when this lambda is defined type is symbol
		global_function_map[cur] = cur->r;
		cur->l = NULL;
		cur->r = NULL;
		return cur;
	}
		
	int count_arg_number(node* cur) {
		int ans = 0;
		while (cur != NULL && cur->token != "nil") {
			cur = cur->r;
			ans++;
		}
		return ans;
	}

	node* do_function(node* cur, int & errortag) {
		// save current define_map
		// adding parameters
		// calculate by going copy function form lambda map
		// and use original parameter to run function
		map<string, node*> cur_define_map;
		node* function_cur ;
		if (cur->l->token == "lambda") {
			// the left node is already a function node
			function_cur = global_function_map[cur->l];
		}
		else {
			// need to go to define_map to find the function node
			function_cur = global_function_map[finding_map(cur->l->token)];
		}
		
		// copy s subtree for the function
		// the return value will set in this subtree so original tree can repeat use
		function_cur = create_subtree(function_cur);
		
		// copy parameter no return value will print original tree
		node* ori = create_subtree(cur); 

		if (count_arg_number(cur->r) != count_arg_number(function_cur->l)) {
			errortag = 6;
			if (cur->l->token == "lambda") {
				cur->token = "lambda";
			}
			else {
				cur->token = finding_map(cur->l->token)->token;
			}
			cur->l = NULL;
			cur->r = NULL;
			return cur;
		}
		node* t1 = cur->r;
		node* t2 = function_cur->l;
	
		// giving argument
		for (; t1!=NULL && cur->token != "nil"; t1=t1->r, t2=t2->r) {
			t1->l = eval_s_exp(t1->l, errortag);
			
			if (errortag) {
				if (errortag == 9) {
					errortag = 18;
				}
				return t1->l;
			}
	
			cur_define_map[t2->l->token] = create_subtree(t1->l);
			if (t1->l->type == function_type) {
				// if the argument is a function node which point to function in function map
				// the new create node to to point to it too
				global_function_map[cur_define_map[t2->l->token]] = global_function_map[t1->l];
			}
		}
		
		// find last s-exp to return
		function_cur = function_cur->r;
		node* pre = NULL;
		
		while (function_cur != NULL) {
			S_EXP S_exp = S_EXP(cur_define_map);
			function_cur->l = S_exp.eval_s_exp(function_cur->l, errortag);
			if (function_cur->l->type == defined_type || function_cur->l->type == clean_environment_type) {
				if (verbose) {
					cout << function_cur->l->token << endl;
				}
				errortag = 9;
			}
			if (errortag == 9 || errortag == 21) {
				if (function_cur->r == NULL) {
					// change to function no return value
					// if 21 dont move
					if (errortag == 9) {
						return ori;
					}
					return function_cur->l;
				} 
				else {
					errortag = 0;
				}
			}
			else if (errortag) {
				return function_cur->l;
			}
			pre = function_cur->l;
			function_cur = function_cur->r;
		}
		
		return pre;
	}
	
	node* verbose_operation(node* cur, int & errortag) {
		cur->r->l = eval_s_exp(cur->r->l, errortag);
		if (errortag) {
			if (errortag == 9) {
				errortag = 18;
			}
			return cur->r->l;
		}
		if (cur->r->l->type != bool_type) {
			errortag = 7;
			return cur;
		}
		if (cur->r->l->token == "nil") {
			verbose = false;
			cur->token = "nil";
		}
		else {
			verbose = true;
			cur->token = "#t";
		}
		cur->type = bool_type;
		cur->l = NULL;
		cur->r = NULL;
		return cur;
	}
	
	node* do_write(node* cur, int & errortag) {
		cur->r->l = eval_s_exp(cur->r->l, errortag);
		if (errortag) {
			if (errortag == 9) {
				errortag = 18;
			}
			return cur->r->l;
		}
		print_s_exp_no_endl(cur->r->l, 1);
		return cur->r->l;
	}
	
	node* do_create_error_object(node* cur, int & errortag) {
		cur->r->l = eval_s_exp(cur->r->l, errortag);
		if (errortag) {
			if (errortag == 9) {
				errortag = 18;
			}
			return cur->r->l;
		}
		if (cur->r->l->type != string_type) {
			errortag = 7;
			return cur;
		}
		cur->r->l->type = error_type;
		return cur->r->l;
	}
	
	node* check_error_object(node* cur, int & errortag) {
		cur->r->l = eval_s_exp(cur->r->l, errortag);
		if (errortag) {
			if (errortag == 9) {
				errortag = 18;
			}
			return cur->r->l;
		}
		if (cur->r->l->type == error_type) {
			cur->token = "#t";
		}
		else {
			cur->token = "nil";
		}
		cur->l = NULL;
		cur->r = NULL;
		cur->type = bool_type;
		return cur;
	}
	
	node* do_newline() {
		cout << endl;
		node* ans = new node();
		ans->token = "nil";
		return ans;
	}
	
	node* symbol_to_string(node* cur, int & errortag) {
		cur->r->l = eval_s_exp(cur->r->l, errortag);
		if (errortag) {
			if (errortag == 9) {
				errortag = 18;
			}
			return cur->r->l;
		}
		if (cur->r->l->type != symbol_type) {
			errortag = 7;
			return cur;
		}
		cur->token = "\"";
		cur->token += cur->r->l->token;
		cur->token += "\"";
		cur->l = NULL;
		cur->r = NULL;
		cur->type = string_type;
		return cur;
	}
	
	node* number_to_string(node* cur, int & errortag) {
		cur->r->l = eval_s_exp(cur->r->l, errortag);
		if (errortag) {
			if (errortag == 9) {
				errortag = 18;
			}
			return cur->r->l;
		}
		if (cur->r->l->type != int_type &&
			cur->r->l->type != float_type) {
			errortag = 7;
			return cur;
		}
		cur->token = "\"";
		cur->token += cur->r->l->token;
		cur->token += "\"";
		cur->l = NULL;
		cur->r = NULL;
		cur->type = string_type;
		return cur;
	}
	
	node* do_display_string(node* cur, int & errortag) {
		cur->r->l = eval_s_exp(cur->r->l, errortag);
		if (errortag) {
			if (errortag == 9) {
				errortag = 18;
			}
			return cur->r->l;
		}
		if (cur->r->l->type != string_type) {
			errortag = 7;
			return cur;
		}
		cur = cur->r->l;
		for (int i = 1; i < cur->token.size()-1; i++) {
			cout << cur->token[i];
		}
		return cur;
	}
	
	node* finding_map(string token) {
		// finding varible from local to global
		if (define_map[token] != NULL) {
			return define_map[token];
		}
		if (global_define_map[token] != NULL) {
			return global_define_map[token];
		}
		return NULL;
	}
	
	node* eval_s_exp(node* cur, int & errortag) {
		if (cur->token != "" && !is_symbol(cur->token)) {
			// not symbol
			return cur;
		}
		else if (is_reserved_words(cur->token)) {
			cur->type = reserved_words_type;
			return cur;
		}
		else if (is_symbol(cur->token)) {
			if (finding_map(cur->token) != NULL) { 
				return finding_map(cur->token);
			}
			else {
				errortag = 10;
				return cur;
			}
		}
		else {
			// (...)
			if (!is_pure_list(cur)) {
				errortag = 5;
				return cur;
			}
			else if (cur->l->token != "" && 
					!is_symbol(cur->l->token)) {
				errortag = 8;
				return cur->l;
			}
			else if (finding_map(cur->l->token) != NULL && 
					finding_map(cur->l->token)->type != function_type) {
				// the key of map but not function name
				cur->l = finding_map(cur->l->token);
				if (cur->l->type != reserved_words_type) {
					errortag = 8;
					return cur->l;
				}
			}
		
			// start 
			// must be the list
			// need to check arg number
			if (cur->l->token == "cons") {
				if (!check_arg_numbers(cur, 3)) {	
					errortag = 6;
					cur = cur->l;
				}
				else {
					cur = cur->r;
					cur = do_cons(cur, errortag);
				}
			}
			else if (cur->l->token == "list") {
				if (check_arg_numbers(cur, 1)) {
					cur->type = bool_type;
					cur->token = "nil";
				}
				else {
					cur = cur->r;
					cur = do_list(cur, errortag);
				}
			}
			else if (cur->l->token == "quote") {
				// must legal
				cur = cur->r->l;
			}
			else if (cur->l->token == "define") {	
				if (!is_top_level(cur)) {
					errortag = 15;
					cur = cur->l;
				}
				else if (!define_format(cur)) {
					errortag = 12;
				}
				else {
					cur = do_define(cur, errortag);
				}
			}
			else if (cur->l->token == "car") {
				if (!check_arg_numbers(cur, 2)) {
					errortag = 6;
					cur = cur->l;
				}
				else {
					cur = do_car(cur, errortag);
				}
			}
			else if (cur->l->token == "cdr") {
				if (!check_arg_numbers(cur, 2)) {
					errortag = 6;
					cur = cur->l;
				}
				else {
					cur = do_cdr(cur, errortag);
				}
			}
			else if (cur->l->token == "atom?") {
				if (!check_arg_numbers(cur, 2)) {
					errortag = 6;
					cur = cur->l;
				}
				else {
					cur = check_atom(cur, errortag);
				}
			}
		  	else if (cur->l->token == "pair?") {
		  		if (!check_arg_numbers(cur, 2)) {
		  			errortag = 6;
		  			cur = cur->l;
				}
				else {
					cur = check_pair(cur, errortag);
				}
			}
		  	else if (cur->l->token == "list?") {
		  		if (!check_arg_numbers(cur, 2)) {
					errortag = 6;
					cur = cur->l;
				}
				else {
					cur = check_list(cur, errortag);
				}
		  	}
		  	else if (cur->l->token == "null?") {
		  		if (!check_arg_numbers(cur, 2)) {
		  			errortag = 6;
		  			cur = cur->l;
				}
				else {
					cur = check_null(cur, errortag);
				}
		  	}
		  	else if (cur->l->token == "integer?") {
		  		if (!check_arg_numbers(cur, 2)) {
					errortag = 6;
					cur = cur->l;
				}
				else {
					cur = check_int(cur, errortag);
				}
		  	}
		  	else if (cur->l->token == "real?") {
		  		if (!check_arg_numbers(cur, 2)) {
		  			errortag = 6;
		  			cur = cur->l;
				}
				else {
					cur = check_real(cur, errortag);
				}
		  	}
		  	else if (cur->l->token == "number?") {
		  		if (!check_arg_numbers(cur, 2)) {
		  			errortag = 6;
		  			cur = cur->l;
				}
				else {
					cur = check_real(cur, errortag); 
				}
		  	}
		  	else if (cur->l->token == "string?") {
		  		if (!check_arg_numbers(cur, 2)) {
		  			errortag = 6;
		  			cur = cur->l;
				}
				else {
					cur = check_string(cur, errortag);
				}
		  	}
		  	else if (cur->l->token == "boolean?") {
		  		if (!check_arg_numbers(cur, 2)) {
		  			errortag = 6;
		  			cur = cur->l;
				}
				else {
					cur = check_bool(cur, errortag);
				}
		  	}
		  	else if (cur->l->token == "symbol?") {
				if (!check_arg_numbers(cur, 2)) {
					errortag = 6;
		  			cur = cur->l;
				}
				else {
					
					cur = check_symbol(cur, errortag);
				}
			}
			else if (cur->l->token=="verbose?") {
				if (!check_arg_numbers(cur, 1)) {
					errortag = 6;
		  			cur = cur->l;
				}
				else {
					cur = check_verbose(cur);
				}
			}
			else if (cur->l->token == "+") {
				if (check_arg_numbers(cur, 1) || check_arg_numbers(cur, 2)) {
					errortag = 6;
		  			cur = cur->l;
				}
				else {
					cur = plus_operation(cur, errortag);
				}
			}
			else if (cur->l->token == "-") {
				if (check_arg_numbers(cur, 1) || check_arg_numbers(cur, 2)) {
					errortag = 6;
		  			cur = cur->l;
					 
				}
				else {
					cur = minus_operation(cur, errortag);
				}
			}
			else if (cur->l->token == "*") {
				if (check_arg_numbers(cur, 1) || check_arg_numbers(cur, 2)) {
					errortag = 6;
		  			cur = cur->l;
				}
				else {
					cur = mul_operation(cur, errortag);
				}
			}
			else if (cur->l->token == "/") {
				if (check_arg_numbers(cur, 1) || check_arg_numbers(cur, 2)) {
					errortag = 6;
		  			cur = cur->l;
				}
				else {
					cur = div_operation(cur, errortag);
				}
			}
			else if (cur->l->token == "not") {
				if (!check_arg_numbers(cur, 2)) {
					errortag = 6;
		  			cur = cur->l;
				}
				else {
					cur = not_operation(cur, errortag);
				}
			}
			else if (cur->l->token == "and") {
				if (check_arg_numbers(cur, 1) || check_arg_numbers(cur, 2)) {
					errortag = 6;
		  			cur = cur->l;
				}
				else {
					cur = and_operation(cur->r, errortag);
				}
			}
			else if (cur->l->token == "or") {
				if (check_arg_numbers(cur, 1) || check_arg_numbers(cur, 2)) {
					errortag = 6;
		  			cur = cur->l;
				}
				else {
					cur = or_operation(cur->r, errortag);
				}
			}
			else if (cur->l->token == ">") { 
				if (check_arg_numbers(cur, 1) || check_arg_numbers(cur, 2)) {
					errortag = 6;
		  			cur = cur->l;
				}
				else {
					cur = gt_operation(cur, errortag);
				}
			}
			else if (cur->l->token == ">=") {
				if (check_arg_numbers(cur, 1) || check_arg_numbers(cur, 2)) {
					errortag = 6;
		  			cur = cur->l;
				}
				else {
					cur = ge_operation(cur, errortag);
				}
			}
			else if (cur->l->token == "<") {
				if (check_arg_numbers(cur, 1) || check_arg_numbers(cur, 2)) {
					errortag = 6;
		  			cur = cur->l;
				}
				else {
					cur = lt_operation(cur, errortag);
				}
			}
			else if (cur->l->token == "<=") {
				if (check_arg_numbers(cur, 1) || check_arg_numbers(cur, 2)) {
					errortag = 6;
		  			cur = cur->l;
				}
				else {
					cur = le_operation(cur, errortag);
				}
			}
			else if (cur->l->token == "=") {
				if (check_arg_numbers(cur, 1) || check_arg_numbers(cur, 2)) {
					errortag = 6;
		  			cur = cur->l;
				}
				else {
					cur = eq_operation(cur, errortag);
				}
			}
			else if (cur->l->token == "string-append") { 
				if (check_arg_numbers(cur, 1) || check_arg_numbers(cur, 2)) {
					errortag = 6;
		  			cur = cur->l;
				}
				else {
					cur = string_append(cur, errortag);
				}
			}
			else if (cur->l->token == "string>?") { 
				if (check_arg_numbers(cur, 1) || check_arg_numbers(cur, 2)) {
					errortag = 6;
		  			cur = cur->l;
				}
				else {
					cur = strgt_operation(cur, errortag);
				}
			}
			else if (cur->l->token == "string<?") {
				if (check_arg_numbers(cur, 1) || check_arg_numbers(cur, 2)) {
					errortag = 6;
		  			cur = cur->l;
				}
				else {
					cur = strlt_operation(cur, errortag);
				}
			}
			else if (cur->l->token == "string=?") {
				if (check_arg_numbers(cur, 1) || check_arg_numbers(cur, 2)) {
					errortag = 6;
		  			cur = cur->l;
				}
				else {
					cur = streq_operation(cur, errortag);
				}
			}
			else if (cur->l->token == "eqv?") {
				if (!check_arg_numbers(cur, 3)) {
					errortag = 6;
		  			cur = cur->l;
				}
				else {
					cur = eqv_operation(cur, errortag);}
			}
			else if (cur->l->token == "equal?") {
				if (!check_arg_numbers(cur, 3)) {
					errortag = 6;
		  			cur = cur->l;
				}
				else {
					cur = equal_operation(cur, errortag);
				}
			}
			else if (cur->l->token == "begin") {
				if (check_arg_numbers(cur, 1)) {
					errortag = 6;
		  			cur = cur->l;
				}
				else {
					cur = begin_operation(cur, errortag);
				}
			}
			else if (cur->l->token == "if") {
				if (!check_arg_numbers(cur, 3) && !check_arg_numbers(cur, 4)) {
					errortag = 6;
		  			cur = cur->l;
				}
				else {
					cur = if_operation(cur, errortag);
				}
			}
			else if (cur->l->token == "cond") {
				if (check_arg_numbers(cur, 1)) {
					errortag = 14;
				}
				else {
					cond_format(cur, errortag);
					if (errortag) {
						return cur;
					}
					cur = cond_operation(cur, errortag);
				}
			}
			else if (cur->l->token == "") {
				// ((...) .. )
				cur->l = eval_s_exp(cur->l, errortag);
				if (errortag) {
					if (errortag == 9) {
						errortag = 21;
					}
					return cur->l;
				}
				if (cur->l->token == "") {
					errortag = 8;
					return cur->l;
				}
				cur = eval_s_exp(cur, errortag);
			}
			else if (cur->l->token=="clean-environment") {
				if (!is_top_level(cur)) {
					errortag = 15;
					cur = cur->l;
				}
				else if (!check_arg_numbers(cur, 1)) {
					errortag = 6;
					cur = cur->l;
				}
				else {
					global_define_map.clear();
					global_function_map.clear();
					cur->l->token = "environment cleaned";
					cur->l->type = clean_environment_type;
					cur = cur->l;
				}
			}
			else if (cur->l->token=="exit") {
				if (!is_top_level(cur)) {
					errortag = 15;
					cur = cur->l;
				}
				else if (!check_arg_numbers(cur, 1)) {
					errortag = 6;
					cur = cur->l;
				}
				else {
					cur->l->type = reserved_words_type;
				}
			}
			else if (cur->l->token=="let") {
				if (check_arg_numbers(cur, 1) || check_arg_numbers(cur, 2)) {
					errortag = 16;
				}
				else {
					if (!let_format(cur)) {
						errortag = 16;
					}
					else {	
						cur = let_operation(cur, errortag);
					}
				}
			}
			else if (cur->l->token=="lambda") {
				// there are two type reserved words (lambda ..., (lambda)
				if (cur->l->type == symbol_type) {
					if (check_arg_numbers(cur, 1)) {
						errortag = 17;
					}
					else {
						if (!lambda_format(cur)) {
							errortag = 17;
						}
						else {	
							cur = lambda_operation(cur, errortag);
						}
					}
				}
				else if (cur->l->type == function_type) {
					// self define function name will auto change into lambda
					cur = do_function(cur, errortag);
				}
			}
			else if (finding_map(cur->l->token) != NULL && 
					finding_map(cur->l->token)->type == function_type) {
				// symbol type of lambda
				// define_map[cur->l->token]->token given symbol finding function node
				// function_map can giving function node give the function
				cur = do_function(cur, errortag);
			}
			else if (cur->l->token=="verbose") {
				if (!check_arg_numbers(cur, 2)) {
					errortag = 6;
		  			cur = cur->l;
				}
				else {
					cur = verbose_operation(cur, errortag);
				}
			}
			else if (cur->l->token=="read") {
				if (!check_arg_numbers(cur, 1)) {
					errortag = 6;
		  			cur = cur->l;
				}
				else {
					get_token(errortag); // the token may be get before s-exp
					if (line>1) { 
						// if next input is in next line, next line is the start
						stline--;
						line--;
					}
					cur = s_exp(new node(), errortag);
				}
			}
			else if (cur->l->token=="write") {
				if (!check_arg_numbers(cur, 2)) {
					errortag = 6;
		  			cur = cur->l;
				}
				else {
					cur = do_write(cur, errortag);
				}
			}
			else if (cur->l->token=="eval") {
				if (!check_arg_numbers(cur, 2)) {
					errortag = 6;
		  			cur = cur->l;
				}
				else {
					// (eval '(car '(1 2 3)))
					cur = eval_s_exp(cur->r->l, errortag);
					if (errortag) {
						return cur;
					}
					// when (x (eval ...)) declare varible
					// create a subtree for it
					parser_tree = create_subtree(cur);
					cur = eval_s_exp(parser_tree, errortag); 
				}
			}
			else if (cur->l->token=="set!") {
				if (!check_arg_numbers(cur, 3)) {
					errortag = 6;
		  			cur = cur->l;
				}
				else {
					cur = do_set(cur, errortag);
				}
			}
			else if (cur->l->token=="display-string") {
				if (!check_arg_numbers(cur, 2)) {
					errortag = 6;
		  			cur = cur->l;
				}
				else {
					cur = do_display_string(cur, errortag);
				}
			}
			else if (cur->l->token=="newline") {
				if (!check_arg_numbers(cur, 1)) {
					errortag = 6;
		  			cur = cur->l;
				}
				else {
					cur = do_newline();
				}
			}
			else if (cur->l->token=="symbol->string") {
				if (!check_arg_numbers(cur, 2)) {
					errortag = 6;
		  			cur = cur->l;
				}
				else {
					cur = symbol_to_string(cur, errortag);
				}
			}
			else if (cur->l->token=="number->string") {
				if (!check_arg_numbers(cur, 2)) {
					errortag = 6;
		  			cur = cur->l;
				}
				else {
					cur = number_to_string(cur, errortag);
				}
			}
			else if (cur->l->token=="create-error-object") {
				if (!check_arg_numbers(cur, 2)) {
					errortag = 6;
		  			cur = cur->l;
				}
				else {
					cur = do_create_error_object(cur, errortag);
				}
			}
			else if (cur->l->token=="error-object?") {
				if (!check_arg_numbers(cur, 2)) {
					errortag = 6;
		  			cur = cur->l;
				}
				else {
					cur = check_error_object(cur, errortag);
				}
			}
			else {
				// unbounded symbol or something like int, float
				if (cur->l->type!=symbol_type) {
					errortag = 8;
					cur = cur->l;
				}
				else {
					errortag = 10;
					cur = cur->l;
				}
			}
		}
		return cur;
	}
};

void space(int a) {
	for (int i = 0; i < a; i++, cout << " ") ;
}

void print_s_exp(node* cur, int dep) {
	
	if (cur->token != "") {
		if (cur->type == float_type) {
			float ans = stod(cur->token);
			printf("%.3f\n", ans);
		}
		else if (cur->type == int_type) {
			int a = stoi(cur->token);
			printf("%d\n", a);
		}
		else if (cur->type == defined_type || cur->type == clean_environment_type) {
			if (verbose) {
				cout << cur->token << endl;
			}
		}
		else {
	        if (cur->type == function_type || 
	        	cur->type == reserved_words_type) {
	        	cout << "#<procedure " + cur->token + ">" << endl;
	        }
			else {
				cout << cur->token << endl;
			}
		}
		return ;
	}
	cout << "( ";
	print_s_exp(cur->l, dep+1);
	while (cur->r != NULL && cur->r->token == "") {
		cur = cur->r;
		space(dep*2);
		print_s_exp(cur->l, dep+1);
	}
	if (cur->r != NULL) {
		// (1 . nil) vs (1) 
		space(dep*2);
		cout << "." << endl;
		space(dep*2);
		print_s_exp(cur->r, dep+1);
		
	}
	space((dep-1)*2);
	cout << ")" << endl;
}

void print_s_exp_no_endl(node* cur, int dep) {
	if (cur->token != "") {
		if (cur->type == float_type) {
			float ans = stod(cur->token);
			printf("%.3f", ans);
		}
		else if (cur->type == int_type) {
			int a = stoi(cur->token);
			printf("%d", a);
		}
		else if (cur->type == defined_type || cur->type == clean_environment_type) {
			if (verbose) {
				cout << cur->token;
			}
		}
		else {
	        if (cur->type == function_type || 
	        	cur->type == reserved_words_type) {
	        	cout << "#<procedure " + cur->token + ">";
	        }
			else {
				cout << cur->token;
			}
		}
		return ;
	}
	cout << "( ";
	print_s_exp_no_endl(cur->l, dep+1);
	while (cur->r != NULL && cur->r->token == "") {
		cout << endl;
		cur = cur->r;
		space(dep*2);
		print_s_exp_no_endl(cur->l, dep+1);
	}
	if (cur->r != NULL) {
		// (1 . nil) vs (1) 
		cout << endl;
		space(dep*2);
		cout << "." << endl;
		space(dep*2);
		print_s_exp_no_endl(cur->r, dep+1);
		
	}
	cout << endl;
	space((dep-1)*2);
	cout << ")";
}

void print_error(node* start, int & errortag) {
	if (errortag == 1) {
		cout << "ERROR (unexpected token) : atom or '(' expected when token at Line " << stline << 
				" Column " << stcol << " is >>" << token << "<<" << endl;
		errortag = 0;
		go_next_line(errortag);
	}
	else if (errortag == 2) {
		cout << "ERROR (unexpected token) : ')' expected when token at Line " << stline << 
				" Column " << stcol << " is >>" << token << "<<" << endl;
		errortag = 0;
		go_next_line(errortag);
	}
	else if (errortag == 3) {
		cout << "ERROR (no closing quote) : END-OF-LINE encountered at Line " << line <<
			  " Column " << col << endl;
		errortag = 0;
		go_next_line(errortag);
	}
	else if (errortag == 4) {
		cout << "ERROR (no more input) : END-OF-FILE encountered" << endl;
		return ;
	}
	else if (errortag == 5) {
		cout << "ERROR (non-list) : ";
		print_s_exp(start, 1); 
	}
	else if (errortag == 6) {
		cout << "ERROR (incorrect number of arguments) : ";
		cout << start->token << endl;
	}
	else if (errortag == 7) {
		cout << "ERROR ("; 
		cout << start->l->token;
		cout << " with incorrect argument type) : ";
		start = start->r;
		print_s_exp(start->l, 1) ;
	}
	else if (errortag == 8) {
		cout << "ERROR (attempt to apply non-function) : ";
		print_s_exp(start, 1);
	}
	else if (errortag == 9) {
		cout << "ERROR (no return value) : ";
		print_s_exp(start, 1);
	}
	else if (errortag == 10) {
		cout << "ERROR (unbound symbol) : ";
		print_s_exp(start, 1);
	}
	else if (errortag == 11) {
		cout << "ERROR (division by zero) : ";
		print_s_exp(start, 1);
	}
	else if (errortag == 12) {
		cout << "ERROR (DEFINE format) : ";
		print_s_exp(start, 1);
	}
	else if (errortag == 14) {
		cout << "ERROR (COND format) : ";
		print_s_exp(start, 1);
	}
	else if (errortag == 15) {
		cout << "ERROR (level of "; 
        for (char c : start->token) {
            cout << char(toupper(c));
        }
		cout << ")" << endl;
	}
	else if (errortag == 16) {
		cout << "ERROR (LET format) : " ;
		print_s_exp(start, 1);
	}
	else if (errortag == 17) {
		cout << "ERROR (LAMBDA format) : " ;
		print_s_exp(start, 1);
	}
	else if (errortag == 18) {
		cout << "ERROR (unbound parameter) : ";
		print_s_exp(start, 1);
	}
	else if (errortag == 19) {
		cout << "ERROR (unbound test-condition) : ";
		print_s_exp(start, 1);
	}
	else if (errortag == 20) {
		cout << "ERROR (unbound condition) : ";
		print_s_exp(start, 1);
	}
	else if (errortag == 21) {
		// ((...) ...) let -> fixed
		cout << "ERROR (no return value) : ";
		print_s_exp(start, 1);
	}
	cout << endl;
	if (errortag==4) {
		cout << "> ";
		print_error(parser_tree, errortag);
	}
	else {
		errortag = 0;
	}
}

int main() {
	cout << "Welcome to OurScheme!" << endl << endl;
	bool eof = false;
	line = 1;
	col = 0;
	cin >> token;
	verbose = true;
	while (!eof) {
		cout << "> ";
		int errortag = 0;	
		get_token(errortag); // the token may be get before s-exp
		if (line>1) { 
			// if next input is in next line, next line is the start
			stline--;
			line--;
		}
		
		if (errortag==0) {
			parser_tree = new node();
			parser_tree = s_exp(parser_tree, errortag);
		}
		if (errortag==0) {
			before_edit_tree = create_subtree(parser_tree);
			S_EXP S_exp = S_EXP(map<string, node*>());
			parser_tree = S_exp.eval_s_exp(parser_tree, errortag);
		}
		if (errortag==0) {
			if (parser_tree->l != NULL 
				&& parser_tree->l->token == "exit"
				&& parser_tree->r == NULL && 
				parser_tree->l->type==reserved_words_type) {
				// exit detect
				eof = true;
				cout << endl;
				continue;
			}
			print_s_exp(parser_tree, 1); 
			cout << endl;
		}
		else {
			print_error(parser_tree, errortag);
			if (errortag == 4) {
				eof = true;
			}
		}
		line = 1;
		col = 1;
	}
	cout << "Thanks for using OurScheme!";
    return 0;
}