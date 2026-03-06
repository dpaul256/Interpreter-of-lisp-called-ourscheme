#include <bits/stdc++.h>
using namespace std;

// syntez tree
struct node {
	string token; // if int only need atoi
	int l, r;
	int type; // 0 normal, 1 quote
	node() {
		token = "";
		l = -1;
		r = -1;
		type = 0;
	}
};

vector<node> parser_tree, before_edit_tree;
int before_edit_pointer;
int line, col;
int stline, stcol;
char last_char; // start form next char of already used
string token;
bool eofflag = false; // true if next char is eof 
map<string, int> define_map;
int tree_pointer;
vector<string> reserved_words = {"cons", "list", "quote", "define", "car", "cdr", "atom?", "pair?", 
								 "list?", "null", "integer?", "real?", "number?", "string?", 
								 "boolean?", "symbol?", "+", "-", "*", "/", "not", "and", "or",
								 ">", ">=", "<", "<=", "=", "string-append", "string>?", "string<?", 
								 "string=?", "eqv?", "equal?", "begin", "if", "cond", "clean-environment", "exit"};

int eval_s_exp(int cur, int & errortag);

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

void s_exp(int cur, int & errortag) {
	// cout << "cur : " << cur << " " << token << endl;
	// none token named "nil" only index = -1
	if (errortag) {
		return;
	}
	if (token == "(") {
		get_token(errortag);
		if (errortag) {
			return ;
		}
		if (token == ")") { // () case
			// the parent layer will set nil to -1
			parser_tree[cur].token = "nil";
			return ;
		}
		node n = node();
		parser_tree.push_back(n);
		parser_tree[cur].l = parser_tree.size()-1;
		s_exp(parser_tree[cur].l, errortag);
		if (errortag) {
			return;
		}
		get_token(errortag);
		if (errortag) {
			return;
		}
		else if (token == ")") { 
			return ;
		}
		else if (token == ".") {
			// it same case with many s-exp 
		}
		else {
			while (token != ")" && token != ".") {
				node n = node();
				parser_tree.push_back(n);
				parser_tree[cur].r = parser_tree.size()-1;
				cur = parser_tree[cur].r;
				n = node();
				parser_tree.push_back(n);
				parser_tree[cur].l = parser_tree.size()-1;
				s_exp(parser_tree[cur].l, errortag);
				if (errortag) {
					return;
				}
				get_token(errortag);
				if (errortag) {
					return;
				}
			}
		}
	}
	else if (token == "'") {
		node n = node();
		parser_tree.push_back(n);
		parser_tree[cur].l = parser_tree.size()-1;
		parser_tree[parser_tree[cur].l].token = "quote";
		n = node();
		parser_tree.push_back(n);
		parser_tree[cur].r = parser_tree.size()-1;
		cur = parser_tree[cur].r;
		n = node();
		parser_tree.push_back(n);
		parser_tree[cur].l = parser_tree.size()-1;
		get_token(errortag);
		if (errortag) {
			return ;
		}
		s_exp(parser_tree[cur].l, errortag);
		if (errortag) {
			return;
		}
		return ;
	}
	else if (token == "." || token == ")") {
		errortag = 1;
		return;
	}
	else {
		// atom
		parser_tree[cur].token = token;
		return ;
	}
	
	if (token == ".") {
		get_token(errortag);
		if (errortag) {
			return ;
		}
		node n = node();
		parser_tree.push_back(n);
		parser_tree[cur].r = parser_tree.size()-1;
		s_exp(parser_tree[cur].r, errortag);
		if (errortag) {
			return ;
		}
		get_token(errortag); 
		if (token != ")" || errortag == 4) {
			// ( 1
			errortag = 2;
			return ;
		}
		else if (parser_tree[parser_tree[cur].r].token == "nil" || 
				parser_tree[parser_tree[cur].r].token == "#f") {
			// (1 . ())
			parser_tree[cur].r = -1;
		}
	}
}

bool check_arg_numbers(int cur, int numbers) {
	if (cur == -1) { 
		if (numbers==0) {
			return false; 
		}
		return true;
	}
	if (parser_tree[cur].l == -1) {
		if (numbers==1) {
			return false; 
		}
		return true;
	}
	cur = parser_tree[cur].r;
	int i;
	for (i = 1; i <= numbers && cur != -1; i++) {
		cur = parser_tree[cur].r;
	}	
	if (i != numbers || cur != -1) {
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

bool is_pure_list(int cur) {
	while (cur != -1) {
		if (parser_tree[cur].l==-1) {
			return false;
		}
		cur = parser_tree[cur].r;
	}
	return true;
}

bool is_top_level(int cur) {
	return cur == tree_pointer;
}

int do_cons(int cur, int & errortag) {
	parser_tree[cur].l = eval_s_exp(parser_tree[cur].l, errortag);
	if (errortag) {
		return parser_tree[cur].l;
	}
	parser_tree[cur].r = eval_s_exp(parser_tree[parser_tree[cur].r].l, errortag);
	if (errortag) {
		return parser_tree[cur].r;
	}
	if (parser_tree[parser_tree[cur].r].token == "nil") {
		parser_tree[cur].r = -1;
	}
	return cur;
}

int do_list(int cur, int & errortag) {
	int t = cur;
	while (t != -1) {
		parser_tree[t].l = eval_s_exp(parser_tree[t].l, errortag);
		if (errortag) {
			return parser_tree[t].l;
		}
		t = parser_tree[t].r;
	}
	return cur;
}

void do_quote(int cur) {
	if (cur == -1) {
		return ;
	}
	do_quote(parser_tree[cur].l);
	parser_tree[cur].type = 1;
	do_quote(parser_tree[cur].r);
}

int do_define(int cur, int & errortag) {
	int t = parser_tree[cur].r;
	int a = eval_s_exp(parser_tree[parser_tree[t].r].l, errortag);
	if (errortag) {
		return a;
	}
	define_map[parser_tree[parser_tree[t].l].token] = a;
	parser_tree[cur].token = parser_tree[parser_tree[t].l].token + " defined";
	parser_tree[cur].l = -1;
	parser_tree[cur].r = -1;
	return cur;
}


int do_car(int cur, int & errortag) {
	// return the node of first arg
	parser_tree[parser_tree[cur].r].l = eval_s_exp(parser_tree[parser_tree[cur].r].l, errortag);
	if (errortag) {
		return parser_tree[parser_tree[cur].r].l;
	}
	if (parser_tree[parser_tree[parser_tree[cur].r].l].token != "") {
		errortag = 7;
		return cur;
	}
	return parser_tree[parser_tree[parser_tree[cur].r].l].l;
}

int do_cdr(int cur, int & errortag) {
	parser_tree[parser_tree[cur].r].l = eval_s_exp(parser_tree[parser_tree[cur].r].l, errortag);
	if (errortag) {
		return parser_tree[parser_tree[cur].r].l;
	}
	if (parser_tree[parser_tree[parser_tree[cur].r].l].token != "") {
		errortag = 7;
		return cur;
	}
	if (parser_tree[cur].r == -1) {
		parser_tree[cur].token = "nil";
		return cur;
	}
	return parser_tree[parser_tree[parser_tree[cur].r].l].r;
}

int check_pair(int cur, int & errortag) {
	// only (pair? 3) nil
	parser_tree[parser_tree[cur].r].l = eval_s_exp(parser_tree[parser_tree[cur].r].l, errortag);
	if (errortag) {
		return parser_tree[parser_tree[cur].r].l;
	}
	if (parser_tree[parser_tree[parser_tree[cur].r].l].token != "") {
		parser_tree[cur].token = "nil";
	}
	else {
		parser_tree[cur].token = "#t";
	}
	parser_tree[cur].l = -1;
	parser_tree[cur].r = -1;
	return cur;
}

int check_list(int cur, int & errortag) {
	parser_tree[parser_tree[cur].r].l = eval_s_exp(parser_tree[parser_tree[cur].r].l, errortag);
	if (errortag) {
		return parser_tree[parser_tree[cur].r].l;
	}
	if (!is_pure_list(parser_tree[parser_tree[cur].r].l) || 
		parser_tree[parser_tree[parser_tree[cur].r].l].token != "") {
		parser_tree[cur].token = "nil";
	}
	else {
		parser_tree[cur].token = "#t";
	}
	parser_tree[cur].l = -1;
	parser_tree[cur].r = -1;
	return cur;
}

int check_atom(int cur, int & errortag) {
	parser_tree[parser_tree[cur].r].l = eval_s_exp(parser_tree[parser_tree[cur].r].l, errortag);
	if (errortag) {
		return parser_tree[parser_tree[cur].r].l;
	}
	if (parser_tree[parser_tree[parser_tree[cur].r].l].token == "") {
		parser_tree[cur].token = "nil";
	}
	else {
		parser_tree[cur].token = "#t";
	}
	parser_tree[cur].l = -1;
	parser_tree[cur].r = -1;
	return cur;
}

int check_null(int cur, int & errortag) {
	// (null? (> 3 2)
	parser_tree[parser_tree[cur].r].l = eval_s_exp(parser_tree[parser_tree[cur].r].l, errortag);
	if (errortag) {
		return parser_tree[parser_tree[cur].r].l;
	}
	if (parser_tree[parser_tree[parser_tree[cur].r].l].token != "nil") {
		parser_tree[cur].token = "nil";
	}
	else {
		parser_tree[cur].token = "#t";
	}
	parser_tree[cur].l = -1;
	parser_tree[cur].r = -1;
	return cur;
}

int check_int(int cur, int & errortag) {
	parser_tree[parser_tree[cur].r].l = eval_s_exp(parser_tree[parser_tree[cur].r].l, errortag);
	if (errortag) {
		return parser_tree[parser_tree[cur].r].l;
	}
	if (is_int(parser_tree[parser_tree[parser_tree[cur].r].l].token)) {
		parser_tree[cur].token = "#t";
	}
	else {
		parser_tree[cur].token = "nil";
	}
	parser_tree[cur].l = -1;
	parser_tree[cur].r = -1;
	return cur;
}

int check_real(int cur, int & errortag) {
	parser_tree[parser_tree[cur].r].l = eval_s_exp(parser_tree[parser_tree[cur].r].l, errortag);
	if (errortag) {
		return parser_tree[parser_tree[cur].r].l;
	}
	if (is_int(parser_tree[parser_tree[parser_tree[cur].r].l].token) || 
		is_float(parser_tree[parser_tree[parser_tree[cur].r].l].token)) {
		parser_tree[cur].token = "#t";
	}
	else {
		parser_tree[cur].token = "nil";
	}
	parser_tree[cur].l = -1;
	parser_tree[cur].r = -1;
	return cur;
}

int check_string(int cur, int & errortag) {
	parser_tree[parser_tree[cur].r].l = eval_s_exp(parser_tree[parser_tree[cur].r].l, errortag);
	if (errortag) {
		return parser_tree[parser_tree[cur].r].l;
	}
	if (is_string(parser_tree[parser_tree[parser_tree[cur].r].l].token)) {
		parser_tree[cur].token = "#t";
	}
	else {
		parser_tree[cur].token = "nil";
	}
	parser_tree[cur].l = -1;
	parser_tree[cur].r = -1;
	return cur;
}

int check_bool(int cur, int & errortag) {
	parser_tree[parser_tree[cur].r].l = eval_s_exp(parser_tree[parser_tree[cur].r].l, errortag);
	if (errortag) {
		return parser_tree[parser_tree[cur].r].l;
	}
	if (parser_tree[parser_tree[parser_tree[cur].r].l].token == "nil" || 
		parser_tree[parser_tree[parser_tree[cur].r].l].token == "#t") {
		parser_tree[cur].token = "#t";
	}
	else {
		parser_tree[cur].token = "nil";
	}
	parser_tree[cur].l = -1;
	parser_tree[cur].r = -1;
	return cur;
}

int check_symbol(int cur, int & errortag) {
	parser_tree[parser_tree[cur].r].l = eval_s_exp(parser_tree[parser_tree[cur].r].l, errortag);
	if (errortag) {
		return parser_tree[parser_tree[cur].r].l;
	}
	if (is_symbol(parser_tree[parser_tree[parser_tree[cur].r].l].token)) {
		parser_tree[cur].token = "#t";
	}
	else {
		parser_tree[cur].token = "nil";
	}
	parser_tree[cur].l = -1;
	parser_tree[cur].r = -1;
	return cur;
}

int plus_operation(int cur, int & errortag) {
	float ans = 0;
	int t = parser_tree[cur].r;
	while (t != -1) {
		parser_tree[t].l = eval_s_exp(parser_tree[t].l, errortag);
		if (errortag) {
			return parser_tree[t].l;
		}
		if (!is_int(parser_tree[parser_tree[t].l].token) && 
			!is_float(parser_tree[parser_tree[t].l].token)) {
			errortag = 7;
			parser_tree[parser_tree[cur].r].l = parser_tree[t].l;
			return cur;
		}
		ans += stof(parser_tree[parser_tree[t].l].token);
		t = parser_tree[t].r;
	}
	if (ans==int(ans)) {
		parser_tree[cur].token = to_string(int(ans));
	}
	else {
		parser_tree[cur].token = to_string(ans);
	}
	parser_tree[cur].l = -1;
	parser_tree[cur].r = -1;
	return cur;
}

int minus_operation(int cur, int & errortag) {
	float ans = 0;
	int t = parser_tree[cur].r;
	parser_tree[t].l = eval_s_exp(parser_tree[t].l, errortag);
	if (errortag) {
		return parser_tree[t].l;
	}	
	if (!is_int(parser_tree[parser_tree[t].l].token) && 
		!is_float(parser_tree[parser_tree[t].l].token)) {
		errortag = 7;
		parser_tree[parser_tree[cur].r].l = parser_tree[t].l;
		return cur;
	}
	ans = stof(parser_tree[parser_tree[t].l].token);
	t = parser_tree[t].r;
	while (t != -1) {
		parser_tree[t].l = eval_s_exp(parser_tree[t].l, errortag);
		if (errortag) {
			return parser_tree[t].l;
		}	
		if (!is_int(parser_tree[parser_tree[t].l].token) && 
			!is_float(parser_tree[parser_tree[t].l].token)) {
			errortag = 7;
			parser_tree[parser_tree[cur].r].l = parser_tree[t].l;
			return cur;
		}
		ans -= stof(parser_tree[parser_tree[t].l].token);
		t = parser_tree[t].r;
	}
	if (ans==int(ans)) {
		parser_tree[cur].token = to_string(int(ans));
	}
	else {
		parser_tree[cur].token = to_string(ans);
	}
	parser_tree[cur].l = -1;
	parser_tree[cur].r = -1;
	return cur;
}

int mul_operation(int cur, int & errortag) {
	float ans = 1;
	int t = parser_tree[cur].r;
	while (t != -1) {
		parser_tree[t].l = eval_s_exp(parser_tree[t].l, errortag);
		if (errortag) {
			return parser_tree[t].l;
		}	
		if (!is_int(parser_tree[parser_tree[t].l].token) && 
			!is_float(parser_tree[parser_tree[t].l].token)) {
			errortag = 7;
			parser_tree[parser_tree[cur].r].l = parser_tree[t].l;
			return cur;
		}
		ans *= stof(parser_tree[parser_tree[t].l].token);
		t = parser_tree[t].r;
	}
	if (ans==int(ans)) {
		parser_tree[cur].token = to_string(int(ans));
	}
	else {
		parser_tree[cur].token = to_string(ans);
	}
	parser_tree[cur].l = -1;
	parser_tree[cur].r = -1;
	return cur;
}

int div_operation(int cur, int & errortag) {
	// (TODO) float control only divide need because will automatically correct
	float ans = 0;
	int t = parser_tree[cur].r;
	parser_tree[t].l = eval_s_exp(parser_tree[t].l, errortag);
    bool get_float = false;
	if (errortag) {
		return parser_tree[t].l;
	}	
	if (!is_int(parser_tree[parser_tree[t].l].token) && 
		!is_float(parser_tree[parser_tree[t].l].token)) {
		errortag = 7;
		parser_tree[parser_tree[cur].r].l = parser_tree[t].l;
		return cur;
	}
    get_float |= is_float(parser_tree[parser_tree[t].l].token);
    ans = stof(parser_tree[parser_tree[t].l].token);
    if(!get_float) { 
        ans = int(ans);
    }
	t = parser_tree[t].r;
	while (t != -1) {
		parser_tree[t].l = eval_s_exp(parser_tree[t].l, errortag);
		if (errortag) {
			return parser_tree[t].l;
		}	
		if (!is_int(parser_tree[parser_tree[t].l].token) && 
			!is_float(parser_tree[parser_tree[t].l].token)) {
			errortag = 7;
			parser_tree[parser_tree[cur].r].l = parser_tree[t].l;
			return cur;
		}
		if (stof(parser_tree[parser_tree[t].l].token) == 0) {
			errortag = 11;
			return parser_tree[cur].l;
		}
        get_float |= is_float(parser_tree[parser_tree[t].l].token);
		ans /= stof(parser_tree[parser_tree[t].l].token);
        if (!get_float) {
            ans = int(ans);
        }
		t = parser_tree[t].r;
	}
	if (get_float) {
		parser_tree[cur].token = to_string(ans);
	}
	else {
		parser_tree[cur].token = to_string(int(ans));
	}
	parser_tree[cur].l = -1;
	parser_tree[cur].r = -1;
	return cur;
}

int not_operation(int cur, int & errortag) {
	parser_tree[parser_tree[cur].r].l = eval_s_exp(parser_tree[parser_tree[cur].r].l, errortag);
	if (errortag) {
		return parser_tree[parser_tree[cur].r].l;
	}
	if (parser_tree[parser_tree[parser_tree[cur].r].l].token == "nil") {
		parser_tree[cur].token = "#t";
	}
	else {
		parser_tree[cur].token = "nil";
	}
	parser_tree[cur].l = -1;
	parser_tree[cur].r = -1;
	return cur;
}

int and_operation(int cur, int & errortag) {
	int t = cur;
	int pre = t;
	while (t != -1) {
		parser_tree[t].l = eval_s_exp(parser_tree[t].l, errortag);
		if (errortag) {
			return parser_tree[t].l;
		}	
		if (parser_tree[parser_tree[t].l].token == "nil") {
			return parser_tree[t].l;
		}
		pre = t;
		t = parser_tree[t].r;
	}
	return parser_tree[pre].l;
}

int or_operation(int cur, int & errortag) {
	int t = cur;
	int pre = t;
	while (t != -1) {
		parser_tree[t].l = eval_s_exp(parser_tree[t].l, errortag);
		if (errortag) {
			return parser_tree[t].l;
		}	
		if (parser_tree[parser_tree[t].l].token != "nil") {
			return parser_tree[t].l;
		}
		pre = t;
		t = parser_tree[t].r;
	}
	return parser_tree[pre].l;
}

int gt_operation(int cur, int & errortag) {
	int t = parser_tree[cur].r;
	float prenum, curnum;
	bool ans = true;
	parser_tree[t].l = eval_s_exp(parser_tree[t].l, errortag);
	if (errortag) {
		return parser_tree[t].l;
	}
	if (is_int(parser_tree[parser_tree[t].l].token) || 
		is_float(parser_tree[parser_tree[t].l].token)) {
		prenum = stof(parser_tree[parser_tree[t].l].token);
	}
	else {
		errortag = 7;
		parser_tree[parser_tree[cur].r].l = parser_tree[t].l;
		return cur;
	}
	t = parser_tree[t].r;
	while (t != -1) {
		// cout << parser_tree[parser_tree[t].l].token << endl;
		parser_tree[t].l = eval_s_exp(parser_tree[t].l, errortag);
		if (errortag) {
			return parser_tree[t].l;
		}
		if (is_int(parser_tree[parser_tree[t].l].token) || 
			is_float(parser_tree[parser_tree[t].l].token)) {
			curnum = stof(parser_tree[parser_tree[t].l].token);
		}
		else {
			errortag = 7;
			parser_tree[parser_tree[cur].r].l = parser_tree[t].l;
			return cur;
		}
		ans &= prenum > curnum;
		prenum = curnum;
		t = parser_tree[t].r;
	}
	parser_tree[cur].token = ans ? "#t" : "nil";
	parser_tree[cur].l = -1;
	parser_tree[cur].r = -1;
	return cur;
}

int ge_operation(int cur, int & errortag) {
	int t = parser_tree[cur].r;
	float prenum, curnum;
	bool ans = true;
	parser_tree[t].l = eval_s_exp(parser_tree[t].l, errortag);
	if (errortag) {
		return parser_tree[t].l;
	}
	if (is_int(parser_tree[parser_tree[t].l].token) || 
		is_float(parser_tree[parser_tree[t].l].token)) {
		prenum = stof(parser_tree[parser_tree[t].l].token);
	}
	else {
		errortag = 7;
		parser_tree[parser_tree[cur].r].l = parser_tree[t].l;
		return cur;
	}
	t = parser_tree[t].r;
	while (t != -1) {
		// cout << parser_tree[parser_tree[t].l].token << endl;
		parser_tree[t].l = eval_s_exp(parser_tree[t].l, errortag);
		if (errortag) {
			return parser_tree[t].l;
		}
		if (is_int(parser_tree[parser_tree[t].l].token) || 
			is_float(parser_tree[parser_tree[t].l].token)) {
			curnum = stof(parser_tree[parser_tree[t].l].token);
		}
		else {
			errortag = 7;
			parser_tree[parser_tree[cur].r].l = parser_tree[t].l;
			return cur;
		}
		ans &= prenum >= curnum;
		prenum = curnum;
		t = parser_tree[t].r;
	}
	parser_tree[cur].token = ans ? "#t" : "nil";
	parser_tree[cur].l = -1;
	parser_tree[cur].r = -1;
	return cur;
}

int lt_operation(int cur, int & errortag) {
	int t = parser_tree[cur].r;
	float prenum, curnum;
	bool ans = true;
	parser_tree[t].l = eval_s_exp(parser_tree[t].l, errortag);
	if (errortag) {
		return parser_tree[t].l;
	}
	if (is_int(parser_tree[parser_tree[t].l].token) || 
		is_float(parser_tree[parser_tree[t].l].token)) {
		prenum = stof(parser_tree[parser_tree[t].l].token);
	}
	else {
		errortag = 7;
		parser_tree[parser_tree[cur].r].l = parser_tree[t].l;
		return cur;
	}
	t = parser_tree[t].r;
	while (t != -1) {
		// cout << parser_tree[parser_tree[t].l].token << endl;
		parser_tree[t].l = eval_s_exp(parser_tree[t].l, errortag);
		if (errortag) {
			return parser_tree[t].l;
		}
		if (is_int(parser_tree[parser_tree[t].l].token) || 
			is_float(parser_tree[parser_tree[t].l].token)) {
			curnum = stof(parser_tree[parser_tree[t].l].token);
		}
		else {
			errortag = 7;
			parser_tree[parser_tree[cur].r].l = parser_tree[t].l;
			return cur;
		}
		ans &= prenum < curnum;
		prenum = curnum;
		t = parser_tree[t].r;
	}
	parser_tree[cur].token = ans ? "#t" : "nil";
	parser_tree[cur].l = -1;
	parser_tree[cur].r = -1;
	return cur;
}

int le_operation(int cur, int & errortag) {
	int t = parser_tree[cur].r;
	float prenum, curnum;
	bool ans = true;
	parser_tree[t].l = eval_s_exp(parser_tree[t].l, errortag);
	if (errortag) {
		return parser_tree[t].l;
	}
	if (is_int(parser_tree[parser_tree[t].l].token) || 
		is_float(parser_tree[parser_tree[t].l].token)) {
		prenum = stof(parser_tree[parser_tree[t].l].token);
	}
	else {
		errortag = 7;
		parser_tree[parser_tree[cur].r].l = parser_tree[t].l;
		return cur;
	}
	t = parser_tree[t].r;
	while (t != -1) {
		// cout << parser_tree[parser_tree[t].l].token << endl;
		parser_tree[t].l = eval_s_exp(parser_tree[t].l, errortag);
		if (errortag) {
			return parser_tree[t].l;
		}
		if (is_int(parser_tree[parser_tree[t].l].token) || 
			is_float(parser_tree[parser_tree[t].l].token)) {
			curnum = stof(parser_tree[parser_tree[t].l].token);
		}
		else {
			errortag = 7;
			parser_tree[parser_tree[cur].r].l = parser_tree[t].l;
			return cur;
		}
		ans &= prenum <= curnum;
		prenum = curnum;
		t = parser_tree[t].r;
	}
	parser_tree[cur].token = ans ? "#t" : "nil";
	parser_tree[cur].l = -1;
	parser_tree[cur].r = -1;
	return cur;
}

int eq_operation(int cur, int & errortag) {
	int t = parser_tree[cur].r;
	float prenum, curnum;
	bool ans = true;
	parser_tree[t].l = eval_s_exp(parser_tree[t].l, errortag);
	if (errortag) {
		return parser_tree[t].l;
	}
	if (is_int(parser_tree[parser_tree[t].l].token) || 
		is_float(parser_tree[parser_tree[t].l].token)) {
		prenum = stof(parser_tree[parser_tree[t].l].token);
	}
	else {
		errortag = 7;
		parser_tree[parser_tree[cur].r].l = parser_tree[t].l;
		return cur;
	}
	t = parser_tree[t].r;
	while (t != -1) {
		// cout << parser_tree[parser_tree[t].l].token << endl;
		parser_tree[t].l = eval_s_exp(parser_tree[t].l, errortag);
		if (errortag) {
			return parser_tree[t].l;
		}
		if (is_int(parser_tree[parser_tree[t].l].token) || 
			is_float(parser_tree[parser_tree[t].l].token)) {
			curnum = stof(parser_tree[parser_tree[t].l].token);
		}
		else {
			errortag = 7;
			parser_tree[parser_tree[cur].r].l = parser_tree[t].l;
			return cur;
		}
		ans &= prenum == curnum;
		prenum = curnum;
		t = parser_tree[t].r;
	}
	parser_tree[cur].token = ans ? "#t" : "nil";
	parser_tree[cur].l = -1;
	parser_tree[cur].r = -1;
	return cur;
}

int string_append(int cur, int & errortag) {
	string ans = "\"";
	int t = parser_tree[cur].r;
	while (t != -1) {
		parser_tree[t].l = eval_s_exp(parser_tree[t].l, errortag);
		if (errortag) {
			return parser_tree[t].l;
		}	
		if (!is_string(parser_tree[parser_tree[t].l].token)) {
			errortag = 7;
			parser_tree[parser_tree[cur].r].l = parser_tree[t].l;
			return cur;
		}
		parser_tree[parser_tree[t].l].token.erase(parser_tree[parser_tree[t].l].token.begin());
		parser_tree[parser_tree[t].l].token.erase(parser_tree[parser_tree[t].l].token.end()-1);
		ans += parser_tree[parser_tree[t].l].token;
		t = parser_tree[t].r;
	}
	parser_tree[cur].token = ans + "\"";
	parser_tree[cur].l = -1;
	parser_tree[cur].r = -1;
	return cur;
}

int strlt_operation(int cur, int & errortag) {
	int t = parser_tree[cur].r;
	string prestr, curstr;
	bool ans = true;
	parser_tree[t].l = eval_s_exp(parser_tree[t].l, errortag);
	if (errortag) {
		return parser_tree[t].l;
	}
	if (is_string(parser_tree[parser_tree[t].l].token)) {
		prestr = parser_tree[parser_tree[t].l].token;
	}
	else {
		errortag = 7;
		parser_tree[parser_tree[cur].r].l = parser_tree[t].l;
		return cur;
	}
	t = parser_tree[t].r;
	while (t != -1) {
		// cout << parser_tree[parser_tree[t].l].token << endl;
		parser_tree[t].l = eval_s_exp(parser_tree[t].l, errortag);
		if (errortag) {
			return parser_tree[t].l;
		}
		if (is_string(parser_tree[parser_tree[t].l].token)) {
			curstr = parser_tree[parser_tree[t].l].token;
		}
		else {
			errortag = 7;
			parser_tree[parser_tree[cur].r].l = parser_tree[t].l;
			return cur;
		}
		ans &= prestr < curstr;
		prestr = curstr;
		t = parser_tree[t].r;
	}
	parser_tree[cur].token = ans ? "#t" : "nil";
	parser_tree[cur].l = -1;
	parser_tree[cur].r = -1;
	return cur;
}

int strgt_operation(int cur, int & errortag) {
	int t = parser_tree[cur].r;
	string prestr, curstr;
	bool ans = true;
	parser_tree[t].l = eval_s_exp(parser_tree[t].l, errortag);
	if (errortag) {
		return parser_tree[t].l;
	}
	if (is_string(parser_tree[parser_tree[t].l].token)) {
		prestr = parser_tree[parser_tree[t].l].token;
	}
	else {
		errortag = 7;
		parser_tree[parser_tree[cur].r].l = parser_tree[t].l;
		return cur;
	}
	t = parser_tree[t].r;
	while (t != -1) {
		// cout << parser_tree[parser_tree[t].l].token << endl;
		parser_tree[t].l = eval_s_exp(parser_tree[t].l, errortag);
		if (errortag) {
			return parser_tree[t].l;
		}
		if (is_string(parser_tree[parser_tree[t].l].token)) {
			curstr = parser_tree[parser_tree[t].l].token;
		}
		else {
			errortag = 7;
			parser_tree[parser_tree[cur].r].l = parser_tree[t].l;
			return cur;
		}
		ans &= prestr > curstr;
		prestr = curstr;
		t = parser_tree[t].r;
	}
	parser_tree[cur].token = ans ? "#t" : "nil";
	parser_tree[cur].l = -1;
	parser_tree[cur].r = -1;
	return cur;
}

int streq_operation(int cur, int & errortag) {
	int t = parser_tree[cur].r;
	string prestr, curstr;
	bool ans = true;
	parser_tree[t].l = eval_s_exp(parser_tree[t].l, errortag);
	if (errortag) {
		return parser_tree[t].l;
	}
	if (is_string(parser_tree[parser_tree[t].l].token)) {
		prestr = parser_tree[parser_tree[t].l].token;
	}
	else {
		errortag = 7;
		parser_tree[parser_tree[cur].r].l = parser_tree[t].l;
		return cur;
	}
	t = parser_tree[t].r;
	while (t != -1) {
		// cout << parser_tree[parser_tree[t].l].token << endl;
		parser_tree[t].l = eval_s_exp(parser_tree[t].l, errortag);
		if (errortag) {
			return parser_tree[t].l;
		}
		if (is_string(parser_tree[parser_tree[t].l].token)) {
			curstr = parser_tree[parser_tree[t].l].token;
		}
		else {
			errortag = 7;
			parser_tree[parser_tree[cur].r].l = parser_tree[t].l;
			return cur;
		}
		ans &= prestr == curstr;
		prestr = curstr;
		t = parser_tree[t].r;
	}
	parser_tree[cur].token = ans ? "#t" : "nil";
	parser_tree[cur].l = -1;
	parser_tree[cur].r = -1;
	return cur;
}

int eqv_operation(int cur, int & errortag) {
	int t = cur;
	int a, b;
	a = eval_s_exp(parser_tree[parser_tree[t].r].l, errortag);
	if (errortag) {
		return a;
	}
	t = parser_tree[t].r;
	b = eval_s_exp(parser_tree[parser_tree[t].r].l, errortag);
	if (errortag) {
		return b;
	}
	if (a == b) {
        // only case of parser_tree[cur].token == "" get true
		parser_tree[cur].token = "#t";
	}
    else if (parser_tree[a].token != "") {
        if (parser_tree[a].token == parser_tree[b].token) {
            if (is_int(parser_tree[a].token) || is_float(parser_tree[a].token)) {
                parser_tree[cur].token = "#t";
            } 
            else if (is_symbol(parser_tree[a].token)) {
                parser_tree[cur].token = "#t";
            }
            else {
                parser_tree[cur].token = "nil";
            }
        }
        else {
            parser_tree[cur].token = "nil";
        }
    }
	else {
		parser_tree[cur].token = "nil";
	}
	parser_tree[cur].l = -1;
	parser_tree[cur].r = -1;
	return cur;
}

bool check_same_tree(int cur1, int cur2) {
	if (cur1==-1 || cur2==-1) {
		if (cur1==cur2) {
			return true;
		}
		else {
			return false;
		}
	}
	if (parser_tree[cur1].token != parser_tree[cur2].token) {
		return false;
	}
	bool l = check_same_tree(parser_tree[cur1].l, parser_tree[cur2].l); 
	bool r = check_same_tree(parser_tree[cur1].r, parser_tree[cur2].r);
	return l && r;
}

int equal_operation(int cur, int & errortag) {
	int t = cur;
	int a, b;
	parser_tree[parser_tree[t].r].l = eval_s_exp(parser_tree[parser_tree[t].r].l, errortag);
	if (errortag) {
		return parser_tree[parser_tree[t].r].l;
	}
	a = parser_tree[parser_tree[t].r].l;
	t = parser_tree[t].r;
	parser_tree[parser_tree[t].r].l = eval_s_exp(parser_tree[parser_tree[t].r].l, errortag);
	if (errortag) {
		return parser_tree[parser_tree[t].r].l;
	}
	b = parser_tree[parser_tree[t].r].l;
	if (check_same_tree(a, b)) {
		parser_tree[cur].token = "#t";
	}
	else {
		parser_tree[cur].token = "nil";
	}
	parser_tree[cur].l = -1;
	parser_tree[cur].r = -1;
	return cur;
}

int begin_operation(int cur, int & errortag) {
	int t = cur;
	int pre = t;
	while (t != -1) {
		parser_tree[t].l = eval_s_exp(parser_tree[t].l, errortag);
		if (errortag) {
			return parser_tree[t].l;
		}	
		pre = t;
		t = parser_tree[t].r;
	}
	return parser_tree[pre].l;
}

int if_operation(int cur, int & errortag) {
	int t = cur;
	int tr, fa;
	t = parser_tree[t].r;
	parser_tree[t].l = eval_s_exp(parser_tree[t].l, errortag);
	if (errortag) {
		return parser_tree[t].l;
	}	
	if (parser_tree[parser_tree[t].l].token != "nil") {
		t = parser_tree[t].r;
		tr = eval_s_exp(parser_tree[t].l, errortag);
		return tr;
	}
	// false;
	t = parser_tree[t].r;
	if (t==-1) {
		errortag = 9;
		return cur;
	}
	t = parser_tree[t].r;
	if (t==-1) {
		errortag = 9;
		return cur;
	}
	fa = eval_s_exp(parser_tree[t].l, errortag);
	return fa;
}

void cond_format(int cur, int & errortag) {
	int t = cur, tt;
	t = parser_tree[cur].r;
	while (t != -1) {
		tt = parser_tree[t].l;
		if (parser_tree[tt].token != "") {
			errortag = 14;
			return ;
		}
		// check format no matter true false
		tt = parser_tree[tt].r;
		if (tt==-1) {
			errortag = 14;
			return ;
		}
        t = parser_tree[t].r;
	}
}

int cond_operation(int cur, int & errortag) {
    // if more than one things get after true get the last one
	int t = cur, tt;
	t = parser_tree[cur].r;
	bool get_else;
	while (t != -1) {
		tt = parser_tree[t].l;
		get_else = parser_tree[parser_tree[tt].l].token == "else";
		parser_tree[tt].l = eval_s_exp(parser_tree[tt].l, errortag);
		if (get_else && parser_tree[t].r==-1) {
			errortag = 0;
		}
		if (errortag) {
			return parser_tree[tt].l;
		}
		// cout << "mid " << parser_tree[parser_tree[tt].l].token << endl;
		if (parser_tree[parser_tree[tt].l].token == "nil") {
			t = parser_tree[t].r;
			if (get_else && t==-1) {	
				// last else then no continue
			}
			else {
				continue;
			}
		}
        
        // the condition is true, start find the answer
		tt = parser_tree[tt].r;
		if (tt==-1) {
			errortag = 14;
			return cur;
		}
        while (true) {
            parser_tree[tt].l = eval_s_exp(parser_tree[tt].l, errortag);
            if (errortag) {
                return parser_tree[tt].l;
            }
            if (parser_tree[tt].r == -1) {
                break;
            }
            tt = parser_tree[tt].r;
        }
		return parser_tree[tt].l; // success
	}
	errortag = 9;
	return cur;
}

int eval_s_exp(int cur, int & errortag) {
	// cout << "cur " << cur << " " << parser_tree[parser_tree[cur].l].token << endl;
	// cout << "cur " << cur << " " << parser_tree[cur].token << endl;
	// return the node number
	if ((parser_tree[cur].token != "" && !is_symbol(parser_tree[cur].token)) || 
		find(reserved_words.begin(), reserved_words.end(), parser_tree[cur].token) != reserved_words.end()) {
		return cur;
	}
	else if (is_symbol(parser_tree[cur].token)) {
		if (define_map[parser_tree[cur].token]) {
			return define_map[parser_tree[cur].token];
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
		else if (parser_tree[parser_tree[cur].l].token != "" && 
				!is_symbol(parser_tree[parser_tree[cur].l].token)) {
			errortag = 8;
			return parser_tree[cur].l;
		}
		else if (define_map[parser_tree[parser_tree[cur].l].token]) {
			parser_tree[cur].l = define_map[parser_tree[parser_tree[cur].l].token];
		}	
		else if (parser_tree[parser_tree[cur].l].type==1) {
			// ('+ 8 9)
			errortag = 8;
			return parser_tree[cur].l;
		}
		// start 
		// must be the list
		// need to check arg number
		if (parser_tree[parser_tree[cur].l].token == "cons") {
			if (!check_arg_numbers(cur, 3)) {	
				errortag = 6;
				cur = parser_tree[cur].l;
			}
			else {
				cur = parser_tree[cur].r;
				cur = do_cons(cur, errortag);
			}
		}
		else if (parser_tree[parser_tree[cur].l].token == "list") {
			if (check_arg_numbers(cur, 1)) {
				parser_tree[cur].token = "nil";
			}
			else {
				cur = parser_tree[cur].r;
				cur = do_list(cur, errortag);
			}
		}
		else if (parser_tree[parser_tree[cur].l].token == "quote") {
			// must legal
			do_quote(cur);
			cur = parser_tree[parser_tree[cur].r].l;
		}
		else if (parser_tree[parser_tree[cur].l].token == "define") {	
			if (!is_top_level(cur)) {
				errortag = 15;
				cur = parser_tree[cur].l;
			}
			else if (!check_arg_numbers(cur, 3)) {
				errortag = 12;
			}
			else if (!is_symbol(parser_tree[parser_tree[parser_tree[cur].r].l].token) || 
				find(reserved_words.begin(), reserved_words.end(), parser_tree[parser_tree[parser_tree[cur].r].l].token) != reserved_words.end()) {
				errortag = 12;
			}
			else {
				cur = do_define(cur, errortag);
			}
		}
		else if (parser_tree[parser_tree[cur].l].token == "car") {
			if (!check_arg_numbers(cur, 2)) {
				errortag = 6;
				cur = parser_tree[cur].l;
			}
			else {
				cur = do_car(cur, errortag);
			}
		}
		else if (parser_tree[parser_tree[cur].l].token == "cdr") {
			if (!check_arg_numbers(cur, 2)) {
				errortag = 6;
				cur = parser_tree[cur].l;
			}
			else {
				cur = do_cdr(cur, errortag);
			}
		}
		else if (parser_tree[parser_tree[cur].l].token == "atom?") {
			if (!check_arg_numbers(cur, 2)) {
				errortag = 6;
				cur = parser_tree[cur].l;
			}
			else {
				cur = check_atom(cur, errortag);
			}
		}
	  	else if (parser_tree[parser_tree[cur].l].token == "pair?") {
	  		if (!check_arg_numbers(cur, 2)) {
	  			errortag = 6;
	  			cur = parser_tree[cur].l;
			}
			else {
				cur = check_pair(cur, errortag);
			}
		}
	  	else if (parser_tree[parser_tree[cur].l].token == "list?") {
	  		if (!check_arg_numbers(cur, 2)) {
				errortag = 6;
				cur = parser_tree[cur].l;
			}
			else {
				cur = check_list(cur, errortag);
			}
	  	}
	  	else if (parser_tree[parser_tree[cur].l].token == "null?") {
	  		if (!check_arg_numbers(cur, 2)) {
	  			errortag = 6;
	  			cur = parser_tree[cur].l;
			}
			else {
				cur = check_null(cur, errortag);
			}
	  	}
	  	else if (parser_tree[parser_tree[cur].l].token == "integer?") {
	  		if (!check_arg_numbers(cur, 2)) {
				errortag = 6;
				cur = parser_tree[cur].l;
			}
			else {
				cur = check_int(cur, errortag);
			}
	  	}
	  	else if (parser_tree[parser_tree[cur].l].token == "real?") {
	  		if (!check_arg_numbers(cur, 2)) {
	  			errortag = 6;
	  			cur = parser_tree[cur].l;
			}
			else {
				cur = check_real(cur, errortag);
			}
	  	}
	  	else if (parser_tree[parser_tree[cur].l].token == "number?") {
	  		if (!check_arg_numbers(cur, 2)) {
	  			errortag = 6;
	  			cur = parser_tree[cur].l;
			}
			else {
				cur = check_real(cur, errortag); 
			}
	  	}
	  	else if (parser_tree[parser_tree[cur].l].token == "string?") {
	  		if (!check_arg_numbers(cur, 2)) {
	  			errortag = 6;
	  			cur = parser_tree[cur].l;
			}
			else {
				cur = check_string(cur, errortag);
			}
	  	}
	  	else if (parser_tree[parser_tree[cur].l].token == "boolean?") {
	  		if (!check_arg_numbers(cur, 2)) {
	  			errortag = 6;
	  			cur = parser_tree[cur].l;
			}
			else {
				cur = check_bool(cur, errortag);
			}
	  	}
	  	else if (parser_tree[parser_tree[cur].l].token == "symbol?") {
			if (!check_arg_numbers(cur, 2)) {
				errortag = 6;
	  			cur = parser_tree[cur].l;
			}
			else {
				
				cur = check_symbol(cur, errortag);
			}
		}
		else if (parser_tree[parser_tree[cur].l].token == "+") {
			if (check_arg_numbers(cur, 1) || check_arg_numbers(cur, 2)) {
				errortag = 6;
	  			cur = parser_tree[cur].l;
			}
			else {
				cur = plus_operation(cur, errortag);
			}
		}
		else if (parser_tree[parser_tree[cur].l].token == "-") {
			if (check_arg_numbers(cur, 1) || check_arg_numbers(cur, 2)) {
				errortag = 6;
	  			cur = parser_tree[cur].l;
				 
			}
			else {
				cur = minus_operation(cur, errortag);
			}
		}
		else if (parser_tree[parser_tree[cur].l].token == "*") {
			if (check_arg_numbers(cur, 1) || check_arg_numbers(cur, 2)) {
				errortag = 6;
	  			cur = parser_tree[cur].l;
			}
			else {
				cur = mul_operation(cur, errortag);
			}
		}
		else if (parser_tree[parser_tree[cur].l].token == "/") {
			if (check_arg_numbers(cur, 1) || check_arg_numbers(cur, 2)) {
				errortag = 6;
	  			cur = parser_tree[cur].l;
			}
			else {
				cur = div_operation(cur, errortag);
			}
		}
		else if (parser_tree[parser_tree[cur].l].token == "not") {
			if (!check_arg_numbers(cur, 2)) {
				errortag = 6;
	  			cur = parser_tree[cur].l;
			}
			else {
				cur = not_operation(cur, errortag);
			}
		}
		else if (parser_tree[parser_tree[cur].l].token == "and") {
			if (check_arg_numbers(cur, 1) || check_arg_numbers(cur, 2)) {
				errortag = 6;
	  			cur = parser_tree[cur].l;
			}
			else {
				cur = and_operation(parser_tree[cur].r, errortag);
			}
		}
		else if (parser_tree[parser_tree[cur].l].token == "or") {
			if (check_arg_numbers(cur, 1) || check_arg_numbers(cur, 2)) {
				errortag = 6;
	  			cur = parser_tree[cur].l;
			}
			else {
				cur = or_operation(parser_tree[cur].r, errortag);
			}
		}
		else if (parser_tree[parser_tree[cur].l].token == ">") { 
			if (check_arg_numbers(cur, 1) || check_arg_numbers(cur, 2)) {
				errortag = 6;
	  			cur = parser_tree[cur].l;
			}
			else {
				cur = gt_operation(cur, errortag);
			}
		}
		else if (parser_tree[parser_tree[cur].l].token == ">=") {
			if (check_arg_numbers(cur, 1) || check_arg_numbers(cur, 2)) {
				errortag = 6;
	  			cur = parser_tree[cur].l;
			}
			else {
				cur = ge_operation(cur, errortag);
			}
		}
		else if (parser_tree[parser_tree[cur].l].token == "<") {
			if (check_arg_numbers(cur, 1) || check_arg_numbers(cur, 2)) {
				errortag = 6;
	  			cur = parser_tree[cur].l;
			}
			else {
				cur = lt_operation(cur, errortag);
			}
		}
		else if (parser_tree[parser_tree[cur].l].token == "<=") {
			if (check_arg_numbers(cur, 1) || check_arg_numbers(cur, 2)) {
				errortag = 6;
	  			cur = parser_tree[cur].l;
			}
			else {
				cur = le_operation(cur, errortag);
			}
		}
		else if (parser_tree[parser_tree[cur].l].token == "=") {
			if (check_arg_numbers(cur, 1) || check_arg_numbers(cur, 2)) {
				errortag = 6;
	  			cur = parser_tree[cur].l;
			}
			else {
				cur = eq_operation(cur, errortag);
			}
		}
		else if (parser_tree[parser_tree[cur].l].token == "string-append") { 
			if (check_arg_numbers(cur, 1) || check_arg_numbers(cur, 2)) {
				errortag = 6;
	  			cur = parser_tree[cur].l;
			}
			else {
				cur = string_append(cur, errortag);
			}
		}
		else if (parser_tree[parser_tree[cur].l].token == "string>?") { 
			if (check_arg_numbers(cur, 1) || check_arg_numbers(cur, 2)) {
				errortag = 6;
	  			cur = parser_tree[cur].l;
			}
			else {
				cur = strgt_operation(cur, errortag);
			}
		}
		else if (parser_tree[parser_tree[cur].l].token == "string<?") {
			if (check_arg_numbers(cur, 1) || check_arg_numbers(cur, 2)) {
				errortag = 6;
	  			cur = parser_tree[cur].l;
			}
			else {
				cur = strlt_operation(cur, errortag);
			}
		}
		else if (parser_tree[parser_tree[cur].l].token == "string=?") {
			if (check_arg_numbers(cur, 1) || check_arg_numbers(cur, 2)) {
				errortag = 6;
	  			cur = parser_tree[cur].l;
			}
			else {
				cur = streq_operation(cur, errortag);
			}
		}
		else if (parser_tree[parser_tree[cur].l].token == "eqv?") {
			if (!check_arg_numbers(cur, 3)) {
				errortag = 6;
	  			cur = parser_tree[cur].l;
			}
			else {
				cur = eqv_operation(cur, errortag);}
		}
		else if (parser_tree[parser_tree[cur].l].token == "equal?") {
			if (!check_arg_numbers(cur, 3)) {
				errortag = 6;
	  			cur = parser_tree[cur].l;
			}
			else {
				cur = equal_operation(cur, errortag);
			}
		}
		else if (parser_tree[parser_tree[cur].l].token == "begin") {
			if (check_arg_numbers(cur, 1)) {
				errortag = 6;
	  			cur = parser_tree[cur].l;
			}
			else {
				cur = begin_operation(parser_tree[cur].r, errortag);
			}
		}
		else if (parser_tree[parser_tree[cur].l].token == "if") {
			if (!check_arg_numbers(cur, 3) && !check_arg_numbers(cur, 4)) {
				errortag = 6;
	  			cur = parser_tree[cur].l;
			}
			else {
				cur = if_operation(cur, errortag);
			}
		}
		else if (parser_tree[parser_tree[cur].l].token == "cond") {
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
		else if (parser_tree[parser_tree[cur].l].token == "") {
			// ((...) .. )
			parser_tree[cur].l = eval_s_exp(parser_tree[cur].l, errortag);
			if (errortag) {
				return parser_tree[cur].l;
			}
			if (parser_tree[parser_tree[cur].l].token == "") {
				errortag = 8;
				return parser_tree[cur].l;
			}
			// used correct arg1 go second time (weird)
			cur = eval_s_exp(cur, errortag);
		}
		else if (parser_tree[parser_tree[cur].l].token=="clean-environment") {
			if (!is_top_level(cur)) {
				errortag = 15;
				cur = parser_tree[cur].l;
			}
			else if (!check_arg_numbers(cur, 1)) {
				errortag = 6;
				cur = parser_tree[cur].l;
			}
			else {
				define_map.clear();
				parser_tree[parser_tree[cur].l].token = "environment cleaned";
				cur = parser_tree[cur].l;
			}
		}
		else if (parser_tree[parser_tree[cur].l].token=="exit") {
			if (!is_top_level(cur)) {
				errortag = 15;
				cur = parser_tree[cur].l;
			}
			else if (!check_arg_numbers(cur, 1)) {
				errortag = 6;
				cur = parser_tree[cur].l;
			}
		}
		else {
			// apply non-function or unbounded symbol
			if (!is_symbol(parser_tree[parser_tree[cur].l].token) || 
				find(reserved_words.begin(), reserved_words.end(), parser_tree[parser_tree[cur].l].token) != reserved_words.end()) {
				// cout << "down errortag 8 should not happen" << endl;
				errortag = 8;
				cur = parser_tree[cur].l;
			}
			else {
				errortag = 10;
				cur = parser_tree[cur].l;
			}
		}
	}
	return cur;
}

void space(int a) {
	for (int i = 0; i < a; i++, cout << " ") ;
}

void print_s_exp(int cur, int dep, int errortag) {
	if (parser_tree[cur].token != "") {
		if (is_float(parser_tree[cur].token)) {
			float f = stof(parser_tree[cur].token);
			printf("%.3f\n", f);
		}
		else if (is_int(parser_tree[cur].token)) {
			int a = stoi(parser_tree[cur].token);
			printf("%d\n", a);
		}
		else {
	        if (find(reserved_words.begin(), reserved_words.end(), parser_tree[cur].token) != reserved_words.end()) {
	            if (parser_tree[cur].type == 0) {
	            	// (TODO) if error need procedure
	            	cout << "#<procedure " + parser_tree[cur].token + ">" << endl;
	            }
	            else {
	            	cout << parser_tree[cur].token << endl;
	            }
	        }
	        else {
	        	cout << parser_tree[cur].token << endl;
	        }
			
		}
		return ;
	}
	cout << "( ";
	print_s_exp(parser_tree[cur].l, dep+1, errortag);
	while (parser_tree[cur].r >= 0 && parser_tree[parser_tree[cur].r].token == "") {
		cur = parser_tree[cur].r;
		space(dep*2);
		print_s_exp(parser_tree[cur].l, dep+1, errortag);
	}
	if (parser_tree[cur].r >= 0) {
		// (1 . nil) vs (1) 
		space(dep*2);
		cout << "." << endl;
		space(dep*2);
		print_s_exp(parser_tree[cur].r, dep+1, errortag);
		
	}
	space((dep-1)*2);
	cout << ")" << endl;
}

void print_error(int start, int & errortag) {
	bool eof;
	// cout << "errortag : " << errortag << endl;
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
		print_s_exp(start, 1, errortag); 
	}
	else if (errortag == 6) {
		cout << "ERROR (incorrect number of arguments) : ";
		print_s_exp(start, 1, errortag);
	}
	else if (errortag == 7) {
		cout << "ERROR ("; 
		cout << parser_tree[parser_tree[start].l].token;
		cout << " with incorrect argument type) : ";
		start = parser_tree[start].r;
		print_s_exp(parser_tree[start].l, 1, errortag) ;
	}
	else if (errortag == 8) {
		cout << "ERROR (attempt to apply non-function) : ";
		print_s_exp(start, 1, 0);
	}
	else if (errortag == 9) { // (TODO) too strang
		cout << "ERROR (no return value) : ";
		parser_tree = before_edit_tree;
		print_s_exp(start, 1, errortag);
	}
	else if (errortag == 10) {
		cout << "ERROR (unbound symbol) : ";
		print_s_exp(start, 1, errortag);
	}
	else if (errortag == 11) {
		cout << "ERROR (division by zero) : ";
		print_s_exp(start, 1, errortag);
	}
	else if (errortag == 12) {
		cout << "ERROR (DEFINE format) : ";
		print_s_exp(start, 1, errortag);
	}
	else if (errortag == 14) {
		cout << "ERROR (COND format) : ";
		print_s_exp(start, 1, errortag);
	}
	else if (errortag == 15) {
		cout << "ERROR (level of "; // (TODO) uppercase
        for (char c : parser_tree[start].token) {
            cout << char(toupper(c));
        }
		cout << ")" << endl;
	}
	cout << endl;
	if (errortag==4) {
		cout << "> ";
		print_error(tree_pointer, errortag);
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
	cin >> token; // first number
	parser_tree = vector<node>();
	tree_pointer = 0;
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
			parser_tree.push_back(node());
			tree_pointer = parser_tree.size()-1;
			s_exp(tree_pointer, errortag);
		}
		if (errortag==0) {
			before_edit_tree = parser_tree;
			before_edit_pointer = tree_pointer;
			tree_pointer = eval_s_exp(tree_pointer, errortag);
		}
		if (errortag==0) {
			if (parser_tree[tree_pointer].l > 0 
				&& parser_tree[parser_tree[tree_pointer].l].token == "exit"
				&& parser_tree[tree_pointer].r == -1 && 
				parser_tree[parser_tree[tree_pointer].l].type==0) {
				// exit detect
				eof = true;
				
			}
			else {
				print_s_exp(tree_pointer, 1, errortag); 
			}
			cout << endl;
		}
		else {
			print_error(tree_pointer, errortag);
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