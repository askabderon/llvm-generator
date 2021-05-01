#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>
#include <algorithm>
#include <set>
#include <stack>
using namespace std;

vector<string> tokens; //elements of processed line
vector<string> postTokens; //postfix version of tokens
set<string> usedVar; 	//set of variables that we initialized in .ll file
ofstream outfile;
int lineCount=-1;
int wcount=0;
int tcount=0;
int icount=0;
int ccount=0;
string file = "";

bool isExpr(int b, int e);
bool isMoreterms(int b, int e);
bool isTerm(int b, int e);
bool isMorefactors(int b, int e);
bool isFactor(int b, int e);
void chooseCalc(int i1,int i2, int i3, int i4, int i5);
int findIndex(int b,int e,string s){	 	//finding index of string s in the processed line
	for(int i=b;i<e;i++){
		if(tokens[i]==s)
			return i;
	}
	return e;
}

string process(string line){ 	//in order to tokenize the line, we added some spaces among some characters
	string processLine = "";
	int len = line.length();
	for(int i=0;i<len;i++){ 	
		char c = line[i];
		if(c=='=' || c=='+' || c=='-' || c=='*' || c=='/' || c=='(' || c==')' || c=='{' || c=='}' || c==',' || c=='#')
			processLine = processLine + ' ' + c + ' ';
		else 
			processLine += c;
	}
	return processLine;
}

void printError(){		//writes llvm code to produce error line
	file = "; ModuleID = 'mylang2ir'\ndeclare i32 @printf(i8*, ...) \n@print.str = constant [23 x i8] c\"Line %d: syntax error\\0A\\00\"\n";
	file += "\ndefine i32 @main() {\ncall i32 (i8*, ...)* @printf(i8* getelementptr ([23 x i8]* @print.str, i32 0, i32 0), i32 "+ to_string(lineCount) + " )";
	file += "\n ret i32 0\n}";
	outfile<<file;
	exit(0);
}

bool isInteger(const string &s){	//checks given string whether is an integer or not
	for(int i=0;i<s.length();i++){
		if(isdigit(s[i])==0){
			return false;
		}
	}
	return true;
};
bool isAlphaNumeric(const string &s){	//checks given string whether is alphanumeric or not
	if(!((s[0]<='z' && s[0]>='a') || (s[0]<='Z' && s[0]>='A')))
		return false;
	for(int i=1;i<s.length();i++){
		if(!((s[i]<='z' && s[i]>='a') || (s[i]<='Z' && s[i]>='A') || (s[i]<='9' && s[i]>='0')))
			return false;
	}
	return true;
}

bool isExpr(int b, int e){ 		//checks the vector of tokens between the indexes of b and e whether is an expression or not 
	if(findIndex(b,e,"+") == e && findIndex(b,e,"-") == e){ 	//if there is no + or - operator, the whole interval must be a term 
		if(!isTerm(b,e)) return false;
		return true;
	}
	int i1=findIndex(b,e,"+") ;
	int i2=findIndex(b,e,"-");
	int d=min(i1,i2);
	bool flag = false;
	while(!isTerm(b,d)){	//if the interval is not a term, we should expand the interval and check it again
		i1=findIndex(d+1,e,"+");
		i2=findIndex(d+1,e,"-");
		d=min(i1,i2);
		if(d==e) {
			flag = true; 	//this indicates that we cannot find a term 
			break;
		}
	}
	if(flag)
		if(!isTerm(b,e)) return false; 	//if we cannot find any term it should return false
	if(!flag)
		if(!isMoreterms(d,e)) { 	//if we found a term , we should check the rest of the interval is moreterm or not
			postTokens.pop_back(); 	//if the rest is not a moreterm, we should remove the operator element we add in the moreterm function
			return false;
		}
	return true;
}
bool isTerm(int b, int e){	//checks the vector of tokens between the indexes of b and e whether is a term or not 
	if(findIndex(b,e,"*") == e && findIndex(b,e,"/") == e){ 	//if there is no * or / operator, the whole interval must be a factor
		if(!isFactor(b,e)) return false;
		return true;
	}

	int i1=findIndex(b,e,"*");
	int i2=findIndex(b,e,"/");
	int d=min(i1,i2);
	bool flag = false;
	while(!isFactor(b,d)){ 	//if the interval is not a factor, we should expand the interval and check it again
		i1=findIndex(d+1,e,"*");
		i2=findIndex(d+1,e,"/");
		d=min(i1,i2);
		if(d==e) {
			flag = true; 	//this indicates that we cannot find a factor
			break;
		}
	}
	if(flag)
		if(!isFactor(b,e)) return false; 	//if we cannot find any factor it should return false
	if(!flag)
		if(!isMorefactors(d,e)) { 	//if we found a factor , we should check the rest of the interval is morefactor or not
			postTokens.pop_back();  //if the rest is not a morefactor, we should remove the operator element we add in the morefactor function
			return false;
		}
	return true;
	
}
bool isMoreterms(int b, int e){ 	//checks the vector of tokens between the indexes of b and e whether is a moreterm or not 
	b++;
	if(findIndex(b,e,"+") == e&& findIndex(b,e,"-") == e){ //if there is no + or - operator, the whole interval must be a term 
		if(!isTerm(b,e)) return false;
		if(tokens[b-1]=="+") postTokens.push_back("+"); 
		if(tokens[b-1]=="-") postTokens.push_back("-");
		return true;
	}

	int i1=findIndex(b,e,"+") ;
	int i2=findIndex(b,e,"-");
	int d=min(i1,i2);
	bool flag = false;
	while(!isTerm(b,d)){ //if the interval is not a term, we should expand the interval and check it again
		i1=findIndex(d+1,e,"+");
		i2=findIndex(d+1,e,"-");
		d=min(i1,i2);
		if(d==e) {
			flag = true; //this indicates that we cannot find a term
			break;
		}
	}
	if(flag)
		if(!isTerm(b,e)) return false; //if we cannot find any term it should return false
	if(tokens[b-1]=="+") postTokens.push_back("+");
	if(tokens[b-1]=="-") postTokens.push_back("-");
	if(!flag){
		if(!isMoreterms(d,e)) { 	//if we found a term , we should check the rest of the interval is moreterm or not
			postTokens.pop_back();  //if the rest is not a moreterm, we should remove the operator element we add in the moreterm function
			return false;
		}
	}
	return true;
}

bool isMorefactors(int b, int e){ 	//checks the vector of tokens between the indexes of b and e whether is a morefactor or not 
	b++;
	if(findIndex(b,e,"*") == e && findIndex(b,e,"/") == e){ //if there is no * or / operator, the whole interval must be a factor
		if(!isFactor(b,e)) return false;
		if(tokens[b-1]=="*") postTokens.push_back("*");
		if(tokens[b-1]=="/") postTokens.push_back("/");
		return true;
	}

	int i1=findIndex(b,e,"*");
	int i2=findIndex(b,e,"/");
	int d=min(i1,i2);
	bool flag = false;
	while(!isFactor(b,d)){ 	//if the interval is not a factor, we should expand the interval and check it again
		i1=findIndex(d+1,e,"*");
		i2=findIndex(d+1,e,"/");
		d=min(i1,i2);
		if(d==e) {
			flag = true; 	//this indicates that we cannot find a factor
			break;
		}
	}
	if(flag)
		if(!isFactor(b,e)) return false; 	//if we cannot find any factor it should return false
	if(tokens[b-1]=="*") postTokens.push_back("*");
	if(tokens[b-1]=="/") postTokens.push_back("/");
	if(!flag){
		if(!isMorefactors(d,e)) { 	//if we found a term , we should check the rest of the interval is morefactor or not
			postTokens.pop_back(); 	//if the rest is not a morefactor, we should remove the operator element we add in the morefactor function
			return false;
		}
	}
	return true;
}
bool isFactor(int b, int e){	//checks the vector of tokens between the indexes of b and e whether is a factor or not 
	if(tokens[b]=="("){
		if(tokens[e-1]!=")") return false;
		else {
			if(!isExpr(b+1,e-1)) return false; 	//factor may be (expr)
			else return true;
		}
	}
	else if(tokens[b] == "choose"){  	//factor may be choose function, we select the outer choose and calculate it
		if(tokens[b+1] != "(") return false;
		if(tokens[e-1] != ")") return false;
		int counter = 3;
		int ind1=-1,ind2=-1,ind3=-1; 	//finding the commas of the choose function
		for(int i=b+2;i<e;i++){
			if(tokens[i]=="choose")
				counter += 3;
			if(tokens[i]==","){
				counter--;
				if(counter ==2 && ind1 == -1) ind1 = i;
				if(counter ==1 && ind2 == -1) ind2 = i;
				if(counter ==0 && ind3 == -1) {
					ind3 = i;
					break;
				}

			}
		}
		if(ind1 != -1 && ind2!=-1 && ind3 !=-1){
			string newresult = "result"+to_string(ccount+1);
			usedVar.insert(newresult);
			postTokens.push_back(newresult);
			chooseCalc(b+2,ind1+1,ind2+1,ind3+1,e);
						
		}
		else
			return false;
		return true;
	}
	else if(e-b>1) return false;
	else if(isInteger(tokens[b])){ 	//factor may be an integer
		postTokens.push_back(tokens[b]);
		return true;
	}
	else if(isAlphaNumeric(tokens[b])){ //factor may be an alphanumeric
		if(usedVar.find(tokens[b])==usedVar.end()){
			file = "\t%" + tokens[b] + " = alloca i32\n" + "\tstore i32 0, i32* %" + tokens[b] + "\n" + file;
			usedVar.insert(tokens[b]);
		}
		postTokens.push_back(tokens[b]);
		return true;
	}
	else
		return false;
}
void initialize(string s){
	if(isInteger(s)){
			file += "\t%veriveritemporari" + to_string(tcount) + " = add i32 " + s + ", 0\n";
	}
	else if(isAlphaNumeric(s)){
		if(usedVar.find(s) == usedVar.end()){
			file = "\t%" + s+ " = alloca i32\n" + "\tstore i32 0, i32* %" + s + "\n" + file;
			usedVar.insert(s);
		}	
		file += "\t%veriveritemporari" + to_string(tcount) + " = load i32* %" + s + "\n";
	}
	else printError();
	tcount++;
}
void exprCalc(){ 	//calculates the expession from the posttokens vector
	stack<string> numbers;	//stacks that stores the numbers or variable names
	for(int i=0;i<postTokens.size();i++){
		if(isInteger(postTokens[i]) || isAlphaNumeric(postTokens[i]))	//if posttokens[i] is a number or a variable, we push it into stack
			numbers.push(postTokens[i]);	
		else { 	//if posttokens[i] is an operator, we pop 2 strings from stack and calculate them, finally push the result into stack again
			string num2=numbers.top();
			numbers.pop();
			if(isInteger(num2)){
				file += "\t%veriveritemporari" + to_string(tcount) + " = add i32 " + num2 + ", 0\n";
				tcount++;
			}
			else {
				if(usedVar.find(num2)==usedVar.end()){
					file += "\t%veriveritemporari" + to_string(tcount) + " = add i32 %" + num2 + ", 0\n";
				}
				else {
					file += "\t%veriveritemporari" + to_string(tcount) + " = load i32* %" + num2 + "\n";
				}
				tcount++;
			}
			string num1=numbers.top();
			numbers.pop();
			if(isInteger(num1)){
				file += "\t%veriveritemporari" + to_string(tcount) + " = add i32 " + num1 + ", 0\n" ;
				tcount++;
			}
			else {
				if(usedVar.find(num1)==usedVar.end()){
					file += "\t%veriveritemporari" + to_string(tcount) + " = add i32 %" + num1 + ", 0\n";
				}
				else {
					file += "\t%veriveritemporari" + to_string(tcount) + " = load i32* %" + num1 + "\n";
				}
				tcount++;
			}
			if(postTokens[i]=="+")
				file += "\t%veriveritemporari" + to_string(tcount) + " = add i32 %veriveritemporari" + to_string(tcount-1) + ", %veriveritemporari" + to_string(tcount-2) + "\n";
			else if(postTokens[i]=="-")
				file += "\t%veriveritemporari" + to_string(tcount) + " = sub i32 %veriveritemporari" + to_string(tcount-1) + ", %veriveritemporari" + to_string(tcount-2) + "\n";
			else if(postTokens[i]=="*")
				file += "\t%veriveritemporari" + to_string(tcount) + " = mul i32 %veriveritemporari" + to_string(tcount-1) + ", %veriveritemporari" + to_string(tcount-2) + "\n";
			else if(postTokens[i]=="/")
				file += "\t%veriveritemporari" + to_string(tcount) + " = sdiv i32 %veriveritemporari" + to_string(tcount-1) + ", %veriveritemporari" + to_string(tcount-2) + "\n";
			string variable = "veriveritemporari"+to_string(tcount);
			numbers.push(variable);
			tcount++;
		}

	}
}

void chooseCalc(int i1, int i2, int i3, int i4, int i5){ 	//calculates the choose function, i's indicate begin and end indexes of the expressions
	ccount++;
	int temp = ccount;
	string newresult = "%result"+to_string(temp);
	vector<string> copy(postTokens); 	
	file = "%result"+ to_string(temp) + " = alloca i32\nstore i32 0, i32* %result" + to_string(temp) + "\n" + file;
	usedVar.insert(newresult);
	postTokens.clear();
	if(!isExpr(i1,i2-1)) printError(); 	//check the first part is an expression
	exprCalc(); 	//calculate the first part
	if(i2 == i1+2)
		initialize(tokens[i1]);
	file += "\t%veriveritemporari" + to_string(tcount) + " = icmp eq i32 %veriveritemporari" + to_string(tcount-1) + ", 0\n"; 	//check first expression is equal to zero or not
	file += "\tbr i1 %veriveritemporari" + to_string(tcount) + ", label %endequal" + to_string(temp)+ ", label %ispositive" + to_string(temp) + "\n"; //check first expression is equal to zero or not
	tcount++;
	file += "endequal" + to_string(temp) + ":\n"; 	//if first expression is equal to 0
	postTokens.clear();
	if(!isExpr(i2,i3-1)) printError(); 	//check the second part is an expression
	exprCalc();		//calculate the second part
	if(i3 == i2+2)
		initialize(tokens[i2]);
	file += "\tstore i32 %veriveritemporari" + to_string(tcount-1) + ", i32* %result" + to_string(temp) + "\n";
	file += "\tbr label %continue" + to_string(temp) + "\n";
	file += "ispositive" + to_string(temp) + ":\n";   
	postTokens.clear();
	isExpr(i1,i2-1);
	exprCalc();
	if(i2 == i1+2)
		initialize(tokens[i1]);
	file += "\t%veriveritemporari" + to_string(tcount) + " = icmp sgt i32 %veriveritemporari" + to_string(tcount-1) + ", 0 \n";	//check first expression is positive or not
	file += "\tbr i1 %veriveritemporari" + to_string(tcount) + ", label %endpositive" + to_string(temp)+ ", label %endnegative" + to_string(temp) + "\n"; 
	tcount++;
	file += "endpositive" + to_string(temp) + ":\n";	//if first expression is positive
	postTokens.clear();
	if(!isExpr(i3,i4-1)) printError();	//check the third part is an expression
	exprCalc(); 	//calculate the third part
	if(i4 == i3+2)
		initialize(tokens[i3]);
	file += "\tstore i32 %veriveritemporari" + to_string(tcount-1) + ", i32 * %result" + to_string(temp) + "\n";
	file += "\tbr label %continue" + to_string(temp) + "\n";
	file += "endnegative" + to_string(temp) + ":\n"; 	//if first expression is negative
	postTokens.clear();
	if(!isExpr(i4,i5-1)) printError(); 	//check the fourth part is an expression
	exprCalc(); 	//calculate the fourth part
	if(i5 == i4+2)
		initialize(tokens[i4]);
	file += "\tstore i32 %veriveritemporari" + to_string(tcount-1) + ", i32 * %result" + to_string(temp) + "\n"; 	
	file += "\tbr label %continue" + to_string(temp) + "\n" + "continue" + to_string(temp) +":\n";

	file += "\t%veriveritemporari" + to_string(tcount) + " = load i32* %result" + to_string(temp) + "\n";
	tcount++;
	postTokens.clear();
	for(int i=0;i<copy.size();i++){ 	//we copy the old posttokens again
		postTokens.push_back(copy[i]);
	}
}
int main(int argc, char* argv[]) {
	string infile_name = argv[1];
	ifstream infile;
	infile.open(infile_name);
	outfile.open("file.ll");
	string header = "";
	header += "; ModuleID = 'mylang2ir'\ndeclare i32 @printf(i8*, ...)\n@print.str = constant [4 x i8] c\"%d\\0A\\00\"\n\n";
	header += "define i32 @main() {\n";
	string line;
	while(getline(infile,line)){ 	//process the input line by line
		lineCount++;
		int openbracket = -1;	
		string processLine = process(line);
		stringstream ss(processLine);
		string token;
		while(ss>>token){  	//tokenize the line and push them to token vector, if there is comment operator we ignore the rest of the line
			if(token =="#") break;
			tokens.push_back(token);
		}
		if(tokens.empty()){ //line may be an empty line
			continue;
		}
		if(find(tokens.begin(),tokens.end(), "while") != tokens.end()){	//line may be a while line
			if(tokens[0]!="while") printError();
			if(tokens[1]!="(") printError();
			if(tokens[tokens.size()-1]!="{") printError();
			if(tokens[tokens.size()-2]!=")") printError();
			if(!isExpr(2,tokens.size()-2)) printError(); 	//check the while condition is an expression or not
			file += "\tbr label %whcond" + to_string(wcount) + "\n";
			file += "whcond" + to_string(wcount) + ":\n";
			exprCalc(); 	//calculate the while condition part
			openbracket = lineCount;
			if(tokens.size()==5)
				initialize(tokens[2]);
			file += "\t%veriveritemporari" + to_string(tcount) + " = icmp ne i32 %veriveritemporari" + to_string(tcount-1) + ", 0\n";
			file += "\tbr i1 %veriveritemporari" + to_string(tcount) + ", label %whbody" + to_string(wcount) + ", label %whend" + to_string(wcount) + "\n";
			tcount++;
			file += "whbody" + to_string(wcount) + ":\n";
			while(getline(infile,line)){ 	//until a line with "}" we process lines in while condition
				lineCount++;
				tokens.clear();
				postTokens.clear();
				string processLine = process(line);
				stringstream ss(processLine);
				string token;
				while(ss>>token){
					if(token =="#") break;
					tokens.push_back(token);
				}
				if(tokens.empty()){
					continue;
				}
				if(tokens.size()==1 && tokens[0]=="}"){
					file += "\n";
					openbracket=-1;
					break;
				}
				if(find(tokens.begin(), tokens.end(), "print") != tokens.end()){
					if(tokens[0]!="print") printError();		
					if(tokens[1]!="(") printError();
					if(tokens[tokens.size()-1]!=")") printError();
					if(!isExpr(2,tokens.size()-1)) printError();
					exprCalc();
					if(tokens.size()==4)
						initialize(tokens[2]);
					file += "\tcall i32 (i8*, ...)* @printf(i8* getelementptr ([4 x i8]* @print.str, i32 0, i32 0), i32 %veriveritemporari" + to_string(tcount-1) + " )\n";
				}
				else if(find(tokens.begin(), tokens.end(), "=") != tokens.end()){
					if(tokens[1]!="=") printError();
					if(usedVar.find(tokens[0]) == usedVar.end()){
						file = "\t%" + tokens[0] + " = alloca i32\n" + "\tstore i32 0, i32* %" + tokens[0] + "\n" + file;
						usedVar.insert(tokens[0]);
					}
					if(!isExpr(2,tokens.size())) printError();
					exprCalc();
					if(tokens.size()==3)
						initialize(tokens[2]);
					file += "\tstore i32 %veriveritemporari" + to_string(tcount-1) + ", i32* %"+ tokens[0] + "\n";
				}
				else printError();	
			}
			file += "\tbr label %whcond" + to_string(wcount) + "\n"; //return and check the while condition again
			file += "\nwhend" + to_string(wcount) + ":\n";
			wcount++;
		}
		else if(find(tokens.begin(),tokens.end(), "if") != tokens.end()){ //line may be a if line
			if(tokens[0]!="if") printError();
			if(tokens[1]!="(") printError();
			if(tokens[tokens.size()-1]!="{") printError();
			if(tokens[tokens.size()-2]!=")") printError();
			if(!isExpr(2,tokens.size()-2)) printError(); 	//check the if condition is an expression or not
			exprCalc();	//calculate the if condition
			openbracket = lineCount;
			file += "\tbr label %ifcond" + to_string(icount) + "\n\n";
			file += "ifcond" + to_string(icount) +":\n";
			if(tokens.size()==5)
				initialize(tokens[2]);
			file += "\t%veriveritemporari" + to_string(tcount) + " = icmp ne i32 %veriveritemporari" + to_string(tcount-1) + ", 0\n";
			file += "\tbr i1 %veriveritemporari"+ to_string(tcount) + ", label %ifbody" + to_string(icount) + ", label %ifend"+ to_string(icount) + "\n";
			tcount++;
			file += "ifbody" + to_string(icount) + ":\n";
			while(getline(infile,line)){ 	//until a line with "}" we process lines in if condition
				lineCount++;
				tokens.clear();
				postTokens.clear();
				string processLine = process(line);
				stringstream ss(processLine);
				string token;
				while(ss>>token){
					if(token =="#") break;
					tokens.push_back(token);
				}
				if(tokens.empty()){
					continue;
				}
				if(tokens.size()==1 && tokens[0]=="}"){
					file+="\n";
					openbracket = -1;
					break;
				}
				if(find(tokens.begin(), tokens.end(), "print") != tokens.end()){
					if(tokens[0]!="print") printError();		
					if(tokens[1]!="(") printError();
					if(tokens[tokens.size()-1]!=")") printError();
					if(!isExpr(2,tokens.size()-1)) printError();
					exprCalc();
					if(tokens.size()==4)
						initialize(tokens[2]);
					file += "\tcall i32 (i8*, ...)* @printf(i8* getelementptr ([4 x i8]* @print.str, i32 0, i32 0), i32 %veriveritemporari" + to_string(tcount-1) + " )\n";
				}
				else if(find(tokens.begin(), tokens.end(), "=") != tokens.end()){
					if(tokens[1]!="=") printError();
					if(usedVar.find(tokens[0]) == usedVar.end()){
						file = "\t%" + tokens[0] + " = alloca i32\n" + "\tstore i32 0, i32* %" + tokens[0] + "\n" + file;
						usedVar.insert(tokens[0]);
					}
					if(!isExpr(2,tokens.size())) printError();
					exprCalc();
					if(tokens.size()==3){
						initialize(tokens[2]);
					}
					file += "\tstore i32 %veriveritemporari" + to_string(tcount-1) + ", i32* %"+ tokens[0] + "\n";
				}
				else printError();		
			}
			file += "\tbr label %ifend" + to_string(icount) + "\n";
			file += "\nifend" + to_string(icount) + ":\n\n";
			icount++;
		} 
		else if(find(tokens.begin(),tokens.end(), "=") != tokens.end()){ //line may be a assignment line
			if(tokens.size()<=2) printError();
			string s1 = tokens[0];
			if(tokens[1] != "=") printError();
			if(usedVar.find(s1) == usedVar.end()){ 	//if variable is not initialized before, we initialized it to zero
				file = "\t%" + s1 + " = alloca i32\n" + "\tstore i32 0, i32* %" + s1 + "\n" + file;
				usedVar.insert(s1);
			}
			if(!isExpr(2,tokens.size())) printError();
			exprCalc();
			if(tokens.size()==3){
				initialize(tokens[2]);
			}
			file += "\tstore i32 %veriveritemporari" +to_string(tcount-1) + ", i32* %" + s1 + "\n";
		}
		else if(find(tokens.begin(),tokens.end(), "print") != tokens.end()){ //line may be a print line
			if(tokens[0]!="print") printError();		
			if(tokens[1]!="(") printError();
			if(tokens[tokens.size()-1]!=")") printError();
			if(!isExpr(2,tokens.size()-1)) printError(); 	//check the inside of the print is an expression or not
			exprCalc();	//calculate the expression
			if(tokens.size()==4)
				initialize(tokens[2]);
			file += "\tcall i32 (i8*, ...)* @printf(i8* getelementptr ([4 x i8]* @print.str, i32 0, i32 0), i32 %veriveritemporari" + to_string(tcount-1) + " )\n";
		}
		else {
			printError();
		}
		if(openbracket != -1){	//if there is an unclosed paranthesis, gives error
			printError();
			return 0; 
		}
		tokens.clear();
		postTokens.clear();
	}
	file += "ret i32 0\n}\n";
	outfile<<header<< file;
	return 0;
}
