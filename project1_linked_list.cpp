#include <bits/stdc++.h>
using namespace std;

enum token_type{SYMBOL, FlOAT, STRING, };

// syntez tree
struct node {
	string token; // if int only need atoi
	node* l;
	node* r;
	node() {
		token = "";
		l = NULL;
		r = NULL;
	}
};


node* parser_tree;
int line, col;
int stline, stcol;
char last_char; // start form next char of already used
string token;
bool eofflag = false; // true if next char is eof 

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

void space(int a) {
	for (int i = 0; i < a; i++, cout << " ") ;
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

void print_s_exp(node* cur, int dep, bool & eof) {
	if (cur == NULL) {
		return ;
	}
	if (cur==parser_tree && cur->l != NULL 
		&& cur->l->token == "exit"
		&& cur->r == NULL) {
		// exit detect
		eof = true;
		return ;
	}
	if (cur->token != "") {
		if (is_float(cur->token)) {
			float f = stof(cur->token);
			printf("%.3f\n", f);
		}
		else if (is_int(cur->token)) {
			int a = stoi(cur->token);
			printf("%d\n", a);
		}
		else {
			cout << cur->token << endl;
		}
		return ;
	}
	cout << "( ";
	print_s_exp(cur->l, dep+1, eof);
	while (cur->r != NULL && cur->r->token == "") {
		cur = cur->r;
		space(dep*2);
		print_s_exp(cur->l, dep+1, eof);
	}
	if (cur->r != NULL) {
		// (1 . nil) vs (1) 
		space(dep*2);
		cout << "." << endl;
		space(dep*2);
		print_s_exp(cur->r, dep+1, eof);
		
	}
	space((dep-1)*2);
	cout << ")" << endl;
}

void print_error(int & errortag) {
	// need to clear the tree
	// '(' and atom error appear more ')', notthing in fornt of dot
	// ())
	// (. 1)
	// ')' error appear when too mamy dot, too many atom after dot
	// (1 . 1 . 1)
	// (1 . 1 1) 	
	// no closing quote
	// "ggg
	// eof checking ? 
	// error can be detected once it appear
	// after all run to next line
	parser_tree = NULL;
	if (errortag == 1) {
		cout << "ERROR (unexpected token) : atom or '(' expected when token at Line " << stline << 
				" Column " << stcol << " is >>" << token << "<<" << endl << endl;
		errortag = 0;
		go_next_line(errortag);
	}
	else if (errortag == 2) {
		cout << "ERROR (unexpected token) : ')' expected when token at Line " << stline << 
				" Column " << stcol << " is >>" << token << "<<" << endl << endl;
		errortag = 0;
		go_next_line(errortag);
	}
	else if (errortag == 3) {
		cout << "ERROR (no closing quote) : END-OF-LINE encountered at Line " << line <<
			  " Column " << col << endl << endl;
		errortag = 0;
		go_next_line(errortag);
	}
	else if (errortag == 4) {
		cout << "ERROR (no more input) : END-OF-FILE encountered" << endl;
		return ;
	}
	
	if (errortag == 4) {
		cout << "> ";
		print_error(errortag);
	}
}

int main() {
	cout << "Welcome to OurScheme!" << endl << endl;
	bool eof = false;
	line = 1;
	col = 0;
	string s;
	cin >> s;
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
			s_exp(parser_tree, errortag);
		}
		if (errortag==0) {
			print_s_exp(parser_tree, 1, eof); 
			cout << endl;
		}
		else {
			print_error(errortag);
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
