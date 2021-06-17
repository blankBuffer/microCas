#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <math.h>


//micro cas
/*
Created By Ben Currie;
*/
bool CLEAR_TERM = false;

#pragma pack(push, 1)
namespace microCas{
	const bool ERRORS = false;// errors shows illegal memory accesses and memory leaks also show important debug info
	const bool FANCY_CHARS = true;
	typedef char byte;

	byte CHARS = 0,EXPR_POINTER = 1,STR = 2,EXPR = 3;
	
	char fileExtension[8]=".blogic";
	int32_t objCount = 0;
	
	int32_t bytesUsed = 0;
	int32_t mallocActivePointers = 0;
	
	struct MemTracker{
		void *pointer = nullptr;
		int32_t bytes = 0;
		byte objType;
	};
	
	const int32_t totalTrackedBytesSize = 0x10000;
	MemTracker trackedBytes[totalTrackedBytesSize];
	
	int32_t findFreeSpace(){
		for(int32_t i = 0;i<totalTrackedBytesSize;i++){//
			if(!trackedBytes[i].pointer){
				return i;
			}
		}
		return -1;
	}
	
	int32_t findPointer(void *pointer){
		for(int32_t i = 0;i<totalTrackedBytesSize;i++){//
			if(trackedBytes[i].pointer == pointer){
				return i;
			}
		}
		return -1;
	}
	
	//this is to keep track of allocated memory
	void freeO(void *p){
		
		int32_t index = findPointer(p);
		
		bytesUsed-=trackedBytes[index].bytes;
		trackedBytes[index].pointer = nullptr;
		trackedBytes[index].bytes = 0;
		trackedBytes[index].objType = (byte)-1;
		
		mallocActivePointers--;
		
		objCount--;
		free(p);
	}
	void *mallocO(int32_t bytes,byte objType){
		objCount++;
		int32_t index = findFreeSpace();
		
		bytesUsed+=bytes;
		trackedBytes[index].bytes = bytes;
		void *pointer = (byte*)malloc(bytes);
		trackedBytes[index].pointer = pointer;
		trackedBytes[index].objType = objType;
		
		mallocActivePointers++;
		if(mallocActivePointers>=totalTrackedBytesSize){
			printf("active pointer limit of %d reached! please terminate the program\n",totalTrackedBytesSize);
			while(true) {}
		}
		
		return pointer;
	}
	void *reallocO(void *oldPointer,int32_t bytes){
		
		int32_t index = findPointer(oldPointer);
		
		byte objType = trackedBytes[index].objType;
		
		int32_t difference = bytes-trackedBytes[index].bytes;
		bytesUsed+=difference;
		
		void *newPointer = (byte*)realloc((byte*)oldPointer,bytes);
		trackedBytes[index].pointer = newPointer;
		trackedBytes[index].bytes = bytes;
		trackedBytes[index].objType = objType;
		
		
		return newPointer;
		
	}
	//
	void printObjCount(){//prints all objects in heap created. not full proof.
		printf("objects in Memory: %d\n",objCount);
		printf("bytes used: ");
		if(bytesUsed < 1000) printf("%d B",bytesUsed);
		else printf("%.2f KB",((float)bytesUsed)/1000.0);
		printf("\n");
		printf("malloc active pointers: %d\n",mallocActivePointers);
	}

	struct Str{
		char *chars = nullptr;
		int16_t len = 0;
		int16_t mem = 2;
		
		void memInit(){
			objCount++;
			int32_t index = findFreeSpace();
			trackedBytes[index].bytes = sizeof(Str);
			trackedBytes[index].objType = STR;
			trackedBytes[index].pointer = this;
			bytesUsed+=sizeof(Str);
			
		}
		
		Str(){
			chars = (char*)mallocO(mem,CHARS);
			chars[0] = 0;
			
			memInit();
		}
		
		Str(const char *text){
			len = strlen(text);
			chars = (char*)mallocO(len+1,CHARS);
			mem = len+1;
			strcpy(chars, text);
			
			memInit();
		}
		
		void push(Str *other){
			int16_t totalLen = len+other->len;
			if(totalLen>=mem){
				mem = totalLen+64;
				chars = (char*)reallocO(chars, mem);
			}
			for(int32_t i = 0;i<other->len;i++) chars[i+len] = other->chars[i];
			len = totalLen;
			chars[len] = 0;
		}
		
		void push(const char *other){
			Str o(other);
			push(&o);
		}
		
		void push(int64_t n){
			char temp[64];
			sprintf(temp, "%lld",n);
			push(temp);
		}
		void push(double n){
			char temp[64];
			if(n>(double)999999 || n<(double)-999999) sprintf(temp, "%E",n);
			else sprintf(temp, "%lf",n);
			push(temp);
		}
		
		void push(char c){
			char temp[2];
			sprintf(temp, "%c",c);
			push(temp);
		}
		
		void tPrint(){
			printf("%s",chars);
		}
		void printBackwards(Str *out){
			int32_t j = out->len;
			out->len+=len;
			out->mem+=len;
			out->chars = (char*)reallocO(out->chars, out->mem);
			for(int32_t i = len-1;i>=0;i--){
				out->chars[j] = chars[i];
				j++;
			}
			out->chars[j] = 0;
		}
		void tPrintln(){
			tPrint();
			printf("\n");
		}
		
		Str *copy(){
			Str *newStr = new Str();
			newStr->push(this);
			return newStr;
		}
		
		void save(Str* name){
			FILE *file = fopen(name->chars, "w");
			fprintf(file, "%s",chars);
			fclose(file);
		}
		
		bool load(Str* name){
			FILE *file = fopen(name->chars, "r");
			if(file == NULL){
				printf("-file not found!\n");
				return true;
			}
			char data[4096];
			fscanf(file, "%s",data);
			push(data);
			fclose(file);
			return false;
		}
		
		void tScan(){
			char data[4096];
			scanf("%s",data);
			push(data);
		}
		
		bool equals(Str *other){
			return strcmp(chars, other->chars) == 0;
		}
		
		~Str(){
			freeO(chars);
			objCount--;
			int32_t index = findPointer(this);
			trackedBytes[index].bytes = 0;
			trackedBytes[index].objType = (byte)-1;
			trackedBytes[index].pointer = nullptr;
			bytesUsed-=sizeof(Str);
		}
	};

	void perfectPower(int64_t num,int64_t *retBase,int64_t *retExpo){//this turns a int64_t into a perfect power
		
		bool found = false;
		int32_t max = log2(num);//this calculates the max exponent for a given number. Since the lowest base is going to be 2 just take the log base 2.
		
		if(max > 31){//if its too many bits use long double to avoid losing precision
			long double ldNum = (long double)num;
			for(int32_t i = 2;i<=max;i++){
				int64_t n = lroundl(powl(ldNum,1.0L/i));
				if(powl(n,i) == ldNum){//check if the rounded root to a power is equal to original number
					(*retExpo) = i;
					(*retBase) = n;
					found = true;
				}
			}
		}else{//otherwise use regular double becuase its faster
			double dNum = (double)num;
			for(int32_t i = 2;i<=max;i++){
				int64_t n = lround(pow(dNum,1.0/i));
				if(pow(n,i) == dNum){
					(*retExpo) = i;
					(*retBase) = n;
					found = true;
				}
			}
		}
		if(!found){//if nothing is found then the exponent is one
			(*retBase) = num;
			(*retExpo) = 1;
		}
	}

	void generateHexString(Str *out,int64_t num,int32_t numberOfBytes){
		for(int32_t i = 0;i<2*numberOfBytes;i++){//put the number of elements
			char b = 0x0F&(num>>(numberOfBytes*8-4*(i+1)));
			if(b<10) b+='0';
			else b+='A'-10;
			out->push(b);
		}
	}

	int64_t readHexString(Str *str,int32_t numberOfBytes,int32_t offset){
		int64_t out = 0;
		for(int32_t i = 0;i<numberOfBytes*2;i++){
			out<<=4;
			char b = str->chars[i+offset];
			if(b>='A') b=(b-'A')+10;
			else b-='0';
			out+=b;
		}
		return out;
	}

	const byte INT = 0,FLOAT = 1, PIV = 2,EV = 3,INF = 4,NEGINF = 5,UNDEF = 6,BIGINT = 7,BOOL = 8;//all types of numbers
	
	//exprSign
	byte NEGATIVE = 1,POSITIVE = 0,UNKNOWN = 2;
	
	struct Num{//meant to be used on stack
		int64_t valueI;//only used when in integer mode
		double valueF;//only used when in float mode
		byte rep;//what type of number is it
		
		//bigInteger
		Str *digits = nullptr; // digits are stored backwards
		byte signum = POSITIVE;
		
		void print(Str *out){//print number
			if(rep == INT){
				out->push(valueI);
				return;
			}else if(rep == FLOAT){
				out->push(valueF);
				return;
			}else if(rep == PIV){
				if(FANCY_CHARS) out->push("π");
				else out->push("pi_");
				return;
			}else if(rep == EV){
				if(FANCY_CHARS) out->push("𝑒");
				else out->push("e_");
				return;
			}else if(rep == INF){
				out->push("inf_");
				return;
			}else if(rep == NEGINF){
				out->push("minf_");
				return;
			}else if(rep == UNDEF){
				out->push("undef");
				return;
			}else if(rep == BOOL){
				if(getState()) out->push("true");
				else out->push("false");
			}else if(rep == BIGINT){
				if(digits->len == 0){
					out->push("0");
				}else{
					if(signum == NEGATIVE) out->push("-");
					digits->printBackwards(out);
				}
			}
			if( ERRORS) out->push("error while printing num\n");//
		}
		void println(Str *out){
			print(out);
			out->push("\n");
		}
		void tPrint(){
			Str out;
			print(&out);
			out.tPrint();
		}
		void tPrintln(){
			Str out;
			println(&out);
			out.tPrint();
		}
		
		void setValueI(int64_t v){
			rep = INT;
			valueI = v;
		}
		void setValueF(double v){
			rep = FLOAT;
			valueF = v;
		}
		void setValue(Num *other){//become like another number
			rep = other->rep;
			if(rep == FLOAT) valueF = other->valueF;
			else if(rep == INT || rep == BOOL) valueI = other->valueI;
			else if(rep == BIGINT){
				
				if(!digits) digits = other->digits->copy();
				else{
					digits->len = 0;
					digits->chars[0] = 0;
					digits->push(other->digits);
				}
				signum = other->signum;
			}
		}
		
		Num(byte r){//set the rep/number type
			rep = r;
			objCount++;
		}
		Num(int64_t v){//int64_t constructor
			setValueI(v);
			objCount++;
		}
		Num(double v){//double constructor
			setValueF(v);
			objCount++;
		}
		Num(Num *num){//constructor for copying other Number
			setValue(num);
			objCount++;
		}
		Num(bool b){
			setState(b);
			objCount++;
		}
		
		Num(Str *dig){
			rep = BIGINT;
			digits = new Str();
			for(int32_t i = 0;i<dig->len;i++){
				if(dig->chars[dig->len-i-1] == '-'){
					signum = NEGATIVE;
					continue;
				}
				digits->push(dig->chars[dig->len-i-1]);
			}
			cleanBigInt();
			objCount++;
		}
		
		Num(){//the default mode is an integer of value 0
			rep = 0;
			valueI = 0;
			objCount++;
		}
		
		void setTrue(){
			rep = BOOL;
			valueI = 1;
		}
		
		void setFalse(){
			rep = BOOL;
			valueI = 0;
		}
		
		void setState(bool s){
			rep = BOOL;
			valueI = s;
		}
		
		bool getState(){
			return valueI!=0;
		}
		
		bool plain(){//not a transedental number
			return rep == INT || rep == FLOAT || rep == BIGINT;
		}
		void convertToFloat(){
			if(rep == INT) valueF = valueI;
			else if(rep == PIV) valueF = M_PI;
			else if(rep == EV) valueF = M_E;
			else if(rep == INF) valueF = INFINITY;
			else if(rep == INF) valueF = -INFINITY;
			else if(rep == BIGINT){
				float n = 1;
				valueF = 0;
				for(int32_t i = 0;i<digits->len;i++){
					valueF+= ((float)(digits->chars[i]-'0'))*n;
					n*=10;
				}
				if(signum == NEGATIVE) valueF = -valueF;
				delete digits;
				digits = nullptr;
			}
			rep = FLOAT;
		}
		void convertToInt(){
			if(rep == BIGINT){
				int64_t n = 1;
				valueI = 0;
				for(int32_t i = 0;i<digits->len;i++){
					valueI+= (digits->chars[i]-'0')*n;
					n*=10;
				}
				if(signum == NEGATIVE) valueI = -valueI;
				delete digits;
				digits = nullptr;
				rep = INT;
			}else if(ERRORS) printf("can't convert to int\n");
		}
		void convertToBigInt(){
			if(rep == INT) {
				if(valueI<0){
					signum = NEGATIVE;
					valueI = -valueI;
				}
				digits = new Str();
				rep = BIGINT;
				int16_t i = 0;
				while (valueI != 0) {
					int16_t dig = (int16_t)(valueI-(valueI/10)*10);
					digits->push((char)(dig+'0'));
					i++;
					valueI/=10;
				}
			}
			else if(ERRORS) printf("can't convert to bigInt\n");
		}
		void cleanBigInt(){//removes leading zeros
			for(int32_t i = digits->len-1; i>0;i--){
				if(digits->chars[i] == '0'){
					digits->chars[i] = 0;
					digits->len--;
				}
				else break;
			}
		}
		void addN(Num *other){//add another number. This automaticly becomes a float if int64_t is not sufficient. Also if you add anything with float the result will become float
			if(rep == INT && other->rep == INT){
				double backup = (double)valueI+(double)other->valueI;//checking for over or under flow
				int64_t test = valueI+other->valueI;
				if(backup>(double)INT64_MAX||backup<(double)INT64_MIN || ((test < 0) != (backup < 0))){
					rep = FLOAT;
					valueF = backup;
				}else valueI=test;
			}else if((other->rep == BIGINT || rep == BIGINT) && plain() && other->plain()){
				if(rep == INT) convertToBigInt();
				bool createdObj = false;
				if(other->rep == INT){
					Num *cpy = new Num(other);
					cpy->convertToBigInt();
					other = cpy;
					createdObj = true;
				}
				int32_t maxDig;
				if(digits->len > other->digits->len) maxDig = digits->len;
				else maxDig = other->digits->len;
				if(signum == other->signum){
					
					int16_t carry = 0;
					for(int32_t i = 0;(i<maxDig || carry != 0);i++){
						int16_t f = 0;
						if(i < digits->len) f = digits->chars[i]-'0';
						int16_t s = 0;
						if(i < other->digits->len) s = other->digits->chars[i]-'0';
						
						int16_t total = f+s+carry;
						carry = 0;
						if(total>9){
							carry++;
							total -=10;
						}
						if(i >= digits->len) digits->push((char)(total+'0'));
						else digits->chars[i] = total+'0';
						
					}
					
				}else{
					bool normalSubtract = false;
					if(digits->len > other->digits->len) normalSubtract = true;
					else if(digits->len == other->digits->len){
						for(int32_t i = maxDig-1;i>-1;i--){
							int16_t f = 0;
							if(i < digits->len) f = digits->chars[i]-'0';
							int16_t s = 0;
							if(i < other->digits->len) s = other->digits->chars[i]-'0';
							if(f>s){
								normalSubtract = true;
								break;
							}
						}
					}
					
					if(!normalSubtract){
						Num *n = new Num(this);
						setValue(other);
						other = n;
						createdObj = true;
					}
					
					int16_t carry = 0;
					
					for(int32_t i = 0;i<maxDig;i++){
						int16_t f = 0;
						if(i < digits->len) f = digits->chars[i]-'0';
						int16_t s = 0;
						if(i < other->digits->len) s = other->digits->chars[i]-'0';
						
						int16_t total = f-s-carry;
						carry = 0;
						if(total<0){
							carry++;
							total +=10;
						}
						
						digits->chars[i] = total+'0';
						
					}
					
				}
				if(createdObj) delete other;
				cleanBigInt();
			}
			else{
				convertToFloat();
				Num cpy(other);
				cpy.convertToFloat();
				valueF+=cpy.valueF;
			}
		}
		void pow10(int64_t n){
			digits->len = 0;
			digits->chars[0] = 0;
			signum = POSITIVE;
			for(int32_t i = 0;i<n;i++){
				digits->push("0");
			}
			digits->push("1");
		}
		void multN(Num *other){//multiply another number
			if(rep == INT && other->rep == INT){
				double backup = (double)valueI*(double)other->valueI;//checking for over or under flow
				int64_t test = valueI*other->valueI;
				if(backup>(double)INT64_MAX||backup<(double)INT64_MIN || (test < 0) != (backup < 0)){
					rep = FLOAT;
					valueF = backup;
				}else valueI=test;
			}else if((other->rep == BIGINT || rep == BIGINT) && plain() && other->plain()){
				if(rep == INT) convertToBigInt();
				bool createdObj = false;
				if(other->rep == INT){
					Num *cpy = new Num(other);
					cpy->convertToBigInt();
					other = cpy;
					createdObj = true;
				}
				
				pow10(3);
				if(createdObj) delete other;
			}else{
				convertToFloat();
				Num cpy(other);
				cpy.convertToFloat();
				valueF*=cpy.valueF;
			}
		}
		void modN(Num *other){
			if(rep == INT && other->rep == INT){
				valueI%=other->valueI;
			}
		}
		void divN(Num *other){//divide number. Be careful
			if(rep == INT && other->rep == INT){
				valueI/=other->valueI;
			}else{
				convertToFloat();
				Num cpy(other);
				cpy.convertToFloat();
				valueF/=cpy.valueF;
			}
		}
		void powN(Num *other){//to the power of other number
			if(rep == INT && other->rep == INT){
				double backup = pow((double)valueI,(double)other->valueI);//checking for over or under flow
				if(backup>(double)INT64_MAX||backup<(double)INT64_MIN) rep = FLOAT;
				valueF = backup;
				valueI = pow(valueI,other->valueI);
			}else{
				convertToFloat();
				Num cpy(other);
				cpy.convertToFloat();
				valueF=pow(valueF,cpy.valueF);
			}
		}
		bool neg(){//return if the number is negative
			if(rep == FLOAT) return valueF < 0.0;
			else if(rep == INT) return valueI < 0;
			else if(rep == NEGINF) return true;
			else if(rep == BIGINT) return signum == NEGATIVE;
			return false;
			
		}
		void flipSign(){//flips the sign of number
			valueI = -valueI;
			valueF = -valueF;
		}
		void absN(){//take the absolute value
			if(valueI < 0) valueI=-valueI;
			valueF = fabs(valueF);
			if(rep == NEGINF) rep = INF;
			if(rep == BIGINT) signum = POSITIVE;
		}
		bool equals(Num *other){//chekcs if other number is equal
			if(rep == INT && other->rep == INT){
				return valueI == other->valueI;
			}else if(other->rep == FLOAT){
				Num cpy(this);
				cpy.convertToFloat();
				return cpy.valueF == other->valueF;
			}else if(rep == BIGINT && other->rep == BIGINT){
				return digits->equals(other->digits) && signum == other->signum;
			}
			if(other->rep == rep) return true;
			return false;
		}
		
		bool equalsI(int64_t n){//check if number is equal
			if(rep == INT) return valueI == n;
			else if(rep == FLOAT) return valueF == (double)n;
			else if(rep == BIGINT) {
				Num cpy(this);
				cpy.convertToInt();
				return cpy.valueI == n;
			}
			return false;
		}
		bool integer(){
			if(rep == INT) return true;
			else if(rep == FLOAT){
				if(round(valueF) == valueF) return true;
			}
			return false;
		}
		bool isGreaterThanI(int64_t l){
			if(rep == INT){
				return valueI>l;
			}else if(rep == FLOAT){
				return valueF>l;
			}else if(rep == PIV){
				return M_PI>l;
			}else if(rep == EV){
				return M_E>l;
			}
			return false;
		}
		bool isLessThanI(int64_t l){
			if(rep == INT){
				return valueI<l;
			}else if(rep == FLOAT){
				return valueF<l;
			}else if(rep == PIV){
				return M_PI<l;
			}else if(rep == EV){
				return M_E<l;
			}
			return false;
		}
		~Num(){
			delete digits;
			objCount--;
		}
	};
	void gcd(Num *a, Num *b,Num *out){//greatest common divisor. Used for factoring
		Num acpy(a),bcpy(b);
		Num c;
		while (!bcpy.equalsI(0)) {
			c.setValue(&acpy);
			c.modN(&bcpy);
			acpy.setValue(&bcpy);
			bcpy.setValue(&c);
		}
		out->setValue(&acpy);
	}
	//expr mean expression
	const byte SUM = 0,PROD = 1,POW = 2,NUM = 3,VAR = 4,LOG = 5,DERI = 6,EQU = 7,ABS = 8,LIST = 9,SOLVE = 10,LIMIT = 11,SUBST = 12,INTEG = 13,SIN = 14,COS = 15,INDET = 16,ASIN = 17,ACOS = 18,TAN = 19,ATAN = 20,CI = 21,SI = 22,NOT = 23, AND = 24, OR = 25;//expr types
	
	const char *exprNames[26] = {"sum","product","power","number","variable","logarithm","derivitive","equation","absolute","list","solve","limit","substitute","integral","sin","cos","indeterrminate","arcsin","arccos","tan","arctan","cosine_integral","sin_integral","not","and","or"};
	
	
	const byte MIDDLE = 0,LEFT = 1,RIGHT = 2;//direction (this is very expirimental)
	
	const int32_t maxBluePrintLength = 2048;
	
	struct Expr{//meant to be used on heap
		byte exprType = NUM;
		int16_t numOfContExpr = 0;//how many expressions in contExpr. Note its memory size may not match
		int16_t memory = 0;
		Str *name = nullptr;//name if its a variable
		Expr **contExpr = nullptr;
		Num value;//value if its a number
		bool simple = false;//is its simplified. This makes its so it repeats less work
		byte direction = MIDDLE;//expirimental
		
		//init
		void init(byte type){//initialize with type
			exprType = type;
			objCount++;
			int32_t index = findFreeSpace();
			trackedBytes[index].bytes+=sizeof(Expr);
			trackedBytes[index].objType = EXPR;
			trackedBytes[index].pointer = this;
			bytesUsed+=sizeof(Expr);
			
		}
		Expr(byte type){//constuct expr with provided type
			init(type);
		}
		
		void generateBluePrint(Str *out){//returns length of string
			generateHexString(out, exprType, sizeof(exprType));//expression type
			if(exprType == VAR){
				generateHexString(out, name->len, sizeof(name->len));//namelength
				out->push(name);
			}else if(exprType == NUM){
				generateHexString(out, value.rep, sizeof(value.rep));
				if(value.rep == INT || value.rep == FLOAT){
					int64_t temp = 0;
					if(value.rep == INT) temp = value.valueI;
					else temp = *((int64_t *)(&value.valueF));//some voodoo magic
					int32_t small = 0;
					if(temp<SHRT_MAX && temp>SHRT_MIN){
						small = 1;
						out->push('V');//very small
					}else if(temp<INT32_MAX && temp>INT32_MIN){
						small = 2;
						out->push('S');//small
					}else out->push('B');//big
					if(small==1) generateHexString(out, temp, sizeof(int16_t));
					else if(small==2) generateHexString(out, temp, sizeof(int32_t));
					else generateHexString(out, temp, sizeof(int64_t));
				}
			}else{
				if(numOfContExpr<0xF){
					generateHexString(out, numOfContExpr, sizeof(char));
					out->chars[out->len-2] = 'V';
				}else{
					generateHexString(out, numOfContExpr, sizeof(numOfContExpr));
				}
				for(int32_t i = 0;i<numOfContExpr;i++) contExpr[i]->generateBluePrint(out);
			}
		}
		
		Expr(Num *val){//make a numberic expression
			init(NUM);
			value.setValue(val);
		}
		Expr(double val){//make a floating point based expression
			init(NUM);
			value.setValueF(val);
		}
		Expr(int64_t val){//make a integer based expression
			init(NUM);
			value.setValueI(val);
		}
		Expr(bool b){
			init(NUM);
			value.setState(b);
		}
		void setName(const char *name){//set the name of expression. Used for variables
			if(!name) return;
			if(!this->name) this->name = new Str();//make name string if not initialized
			this->name->len = 0;
			this->name->push(name);
		}
		void setName(Str *name){
			if(!name) return;
			if(!this->name) this->name = new Str();
			this->name->len = 0;
			this->name->push(name);
		}
		Expr(const char *name){//make variable
			init(VAR);
			setName(name);
		}
		Expr(Str *name){//make variable
			init(VAR);
			setName(name);
		}
		Expr(byte type,Expr *first,Expr *second){//fast way to constuct expression with 2 sub expression
			init(type);
			contExpr = (Expr**)mallocO(2*sizeof(Expr*),EXPR_POINTER);
			contExpr[0] = first;
			contExpr[1] = second;
			numOfContExpr = 2;
			memory = 2;
		}
		Expr(byte type,Expr *expr){////fast way to constuct expression with 1 sub expression
			init(type);
			contExpr = (Expr**)mallocO(sizeof(Expr*),EXPR_POINTER);
			contExpr[0] = expr;
			numOfContExpr = 1;
			memory = 1;
		}
		//shortcuts
		Expr *getBase(){//get base of object if its a power
			if(ERRORS){
				if(exprType != POW) printf("not correct object type\n");
				if(!contExpr[0]) printf("Base does not exist!? READ\n");
				
			}
			return contExpr[0];
		}
		Expr *getExpo(){
			if(ERRORS){
				if(exprType != POW) printf("not correct object type\n");
				if(!contExpr[1]) printf("Exponent does not exist!? READ\n");
			}
			return contExpr[1];
		}
		void setBase(Expr* base){
			simple = false;
			if(ERRORS){
				if(exprType != POW) printf("not correct object type\n");
				if(!contExpr[0]) printf("Base does not exist!? WRITE\n");
			}
			contExpr[0] = base;
		}
		void setExpo(Expr* expo){
			simple = false;
			if(ERRORS){
				if(exprType != POW) printf("not correct object type\n");
				if(!contExpr[1]) printf("Exponent does not exist!? WRITE\n");
			}
			contExpr[1] = expo;
		}
		//
		
		//check if exponet is -1. Very useful for inverses
		bool expoIsMinusOne(){
			if(ERRORS) if(exprType != POW) printf("not correct object type\n");
			Expr *expo = contExpr[1];
			if(expo->exprType == NUM){
				return expo->value.equalsI(-1L);
			}
			return false;
		}
		//
		bool inverseInt(){//returns weather something is an inverse integer also known as the denominator of numercal fraction
			if(exprType == POW){
				Expr *base = contExpr[0];
				if(expoIsMinusOne() && base->exprType == NUM && base->value.rep == INT) return true;
			}
			return false;
		}
		
		
		void basicPrint(Str *out){//used for debug (a primitive printing of the expression)
			if(exprType == INDET){
				out->push("indet!");
				return;
			}
			if(direction != MIDDLE) out->push("(");
			if(exprType == INTEG){
				if(FANCY_CHARS) out->push("∫(");
				else out->push("integral(");
				contExpr[0]->basicPrint(out);
				out->push(")");
			}else if(exprType == COS){
				out->push("cos(");
				contExpr[0]->basicPrint(out);
				out->push(")");
			}else if(exprType == SIN){
				out->push("sin(");
				contExpr[0]->basicPrint(out);
				out->push(")");
			}else if(exprType == SOLVE){
				out->push("solve(");
				contExpr[0]->basicPrint(out);
				out->push(",");
				contExpr[1]->basicPrint(out);
				out->push(")");
			}else if(exprType == EQU){
				contExpr[0]->basicPrint(out);
				out->push("=");
				contExpr[1]->basicPrint(out);
			}else if(exprType == NUM){
				value.print(out);
			}else if(exprType == VAR){
				out->push(name);
			}else if(exprType == POW){
				out->push("(");
				getBase()->basicPrint(out);
				out->push(")^(");
				getExpo()->basicPrint(out);
				out->push(")");
			}else if(exprType == SUM){
				out->push("{");
				for(int32_t i = 0;i<numOfContExpr;i++){
					contExpr[i]->basicPrint(out);
					if(i!=numOfContExpr-1) out->push("+");
				
				}
				out->push("}");
			}else if(exprType == OR){
				out->push("{");
				for(int32_t i = 0;i<numOfContExpr;i++){
					contExpr[i]->basicPrint(out);
					if(i!=numOfContExpr-1) out->push("||");
				}
				out->push("}");
			}else if(exprType == AND){
				out->push("{");
				for(int32_t i = 0;i<numOfContExpr;i++){
					contExpr[i]->basicPrint(out);
					if(i!=numOfContExpr-1) out->push("&&");
				}
				out->push("}");
			}else if(exprType == NOT){
				out->push("!(");
				contExpr[0]->basicPrint(out);
				out->push(")");
			}else if(exprType == PROD){
				out->push("{");
				for(int32_t i = 0;i<numOfContExpr;i++){
					contExpr[i]->basicPrint(out);
					if(i!=numOfContExpr-1) {
						if(FANCY_CHARS) out->push("·");
						else out->push("*");
					}
				
				}
				out->push("}");
			}else if(exprType == LOG){
				out->push("ln(");
				if(contExpr[0]) contExpr[0]->basicPrint(out);
				out->push(")");
			}else if(exprType == DERI){
				if(FANCY_CHARS) out->push("𝕕(");
				else out->push("d(");
				if(contExpr[0]) contExpr[0]->basicPrint(out);
				out->push(")");
			}else if(exprType == ABS){
				out->push("|");
				if(contExpr[0]) contExpr[0]->basicPrint(out);
				out->push("|");
			}else if(exprType == LIST){
				out->push("[");
				for(int32_t i = 0;i<numOfContExpr;i++){
					if(contExpr[i]) contExpr[i]->basicPrint(out);
					if(i != numOfContExpr-1) out->push(",");
				}
				out->push("]");
			}else if(exprType == SUBST){
				out->push("subst(");
				if(contExpr[0]) contExpr[0]->basicPrint(out);
				out->push(",");
				if(contExpr[1]) contExpr[1]->basicPrint(out);
				out->push(")");
			}else if(exprType == ASIN){
				out->push("asin(");
				if(contExpr[0]) contExpr[0]->basicPrint(out);
				out->push(")");
			}else if(exprType == ACOS){
				out->push("acos(");
				if(contExpr[0]) contExpr[0]->basicPrint(out);
				out->push(")");
			}else if(exprType == TAN){
				out->push("tan(");
				if(contExpr[0]) contExpr[0]->basicPrint(out);
				out->push(")");
			}else if(exprType == ATAN){
				out->push("atan(");
				if(contExpr[0]) contExpr[0]->basicPrint(out);
				out->push(")");
			}else if(exprType == CI){
				out->push("Ci(");
				if(contExpr[0]) contExpr[0]->basicPrint(out);
				out->push(")");
			}else if(exprType == SI){
				out->push("Si(");
				if(contExpr[0]) contExpr[0]->basicPrint(out);
				out->push(")");
			}
			
			if(direction == LEFT){
				out->push("⁻)");
			}else if(direction == RIGHT){
				out->push("⁺)");
			}
		}
		
		void print(Str *out){//fancy printing mode
			if(exprType == INDET){
				out->push("indet!");
				return;
			}
			if(direction != MIDDLE) out->push("(");
			if(exprType == INTEG){
				if(FANCY_CHARS) out->push("∫(");
				else out->push("integral(");
				contExpr[0]->print(out);
				out->push(")");
			}else if(exprType == COS){
				out->push("cos(");
				contExpr[0]->print(out);
				out->push(")");
			}else if(exprType == SIN){
				out->push("sin(");
				contExpr[0]->print(out);
				out->push(")");
			}else if(exprType == SOLVE){
				out->push("solve(");
				contExpr[0]->print(out);
				out->push(",");
				contExpr[1]->print(out);
				out->push(")");
			}else if(exprType == EQU){
				contExpr[0]->print(out);
				out->push("=");
				contExpr[1]->print(out);
			}else if(exprType == NUM){
				value.print(out);
			}else if(exprType == VAR){
			
				out->push(name);
				
			}else if(exprType == POW){
				bool special = false;
				if(expoIsMinusOne()){//special syntax for inverses
					if(getBase()->exprType == SIN){//note that csc is not an actual expression type
						out->push("csc(");
						getBase()->contExpr[0]->print(out);
						out->push(")");
						special = true;
					}else if(getBase()->exprType == COS){
						out->push("sec(");
						getBase()->contExpr[0]->print(out);
						out->push(")");
						special = true;
					}else if(getBase()->exprType == TAN){
						out->push("cot(");
						getBase()->contExpr[0]->print(out);
						out->push(")");
						special = true;
					}else{
						bool peren = false;
						if(getBase()->exprType == SUM || getBase()->exprType == PROD || getBase()->exprType == POW)  peren = true;
						out->push("1/");
						if(peren) out->push("(");
						if(getBase()) getBase()->print(out);
						if(peren) out->push(")");
						special = true;
					}
				}
				if(!special && getExpo()->exprType == POW){
					Expr *pw = getExpo();
					if(pw->getBase()->exprType == NUM && pw->expoIsMinusOne()){
						
						if(pw->getBase()->value.equalsI(2L)){
							Expr *base = getBase();
							if(base->exprType == NUM && base->value.equalsI(-1)){
								out->push("i");
							}else if(base->exprType == NUM && base->value.neg()){//oooh fancy using i XD
								Num cpy = Num(&base->value);
								cpy.absN();
								out->push("i*sqrt(");
								cpy.print(out);
								out->push(")");
							}else{
								out->push("sqrt(");
								if(getBase()) getBase()->print(out);
								out->push(")");
							}
							special = true;
						}else if(pw->getBase()->value.equalsI(3L)){
							out->push("cbrt(");
							if(getBase()) getBase()->print(out);
							out->push(")");
							special = true;
						}
						
					}
				}
				if(!special){
					bool usePInBase = false,usePInExpo = false;
					Expr *b = getBase();
					byte tyB = b->exprType,tyE = getExpo()->exprType;
					if((tyB == PROD || tyB == SUM || tyB == POW || tyB == EQU) ||  (tyB == NUM && b->value.neg()) ) usePInBase = true;
					if(!usePInBase){//very special case where the base contains the sqrt of negative number sqrt(-5) is i*sqrt(5) and needs parenthesis
						if(b->exprType == POW){
							Expr *pw = b->getExpo();
							if(pw->getBase()->exprType == NUM && pw->expoIsMinusOne()){
								
								if(pw->getBase()->value.equalsI(2L)){
									Expr *base = b->getBase();
									if(base->exprType == NUM && base->value.neg()) usePInBase = true;
								}
							}
						}
					}
					
					if(usePInBase) out->push("(");
					if(getBase()) getBase()->print(out);
					if(usePInBase) out->push(")");
					out->push("^");
					if(tyE == SUM || tyE == PROD || tyE == POW || tyE == EQU) usePInExpo = true;
					if(usePInExpo) out->push("(");
					if(getExpo()) getExpo()->print(out);
					if(usePInExpo) out->push(")");
				}
				
			}else if(exprType == SUM){
				if(numOfContExpr < 2) out->push("alone sum:");
				for(int32_t i = 0;i < numOfContExpr;i++){
					bool pr = contExpr[i]->exprType == SUM || contExpr[i]->exprType == EQU;
					if(pr) out->push("(");
					if(contExpr[i]) contExpr[i]->print(out);
					if(pr) out->push(")");
					
					bool nextNeg = false;
					if(i!=numOfContExpr-1){
						Expr *next = contExpr[i+1];
						if(next->exprType == PROD){
							for(int32_t i = 0;i<next->numOfContExpr;i++){
								if(next->contExpr[i]->exprType == NUM && next->contExpr[i]->value.neg()){
									nextNeg = true;
									break;
								}
							
							}
						}else if(next->exprType == NUM && next->value.neg()){
							nextNeg = true;
						}
						
					}
					
					if(i!=numOfContExpr-1 && !nextNeg) out->push("+");
				}
				
			}else if(exprType == OR){
				if(numOfContExpr < 2) out->push("alone or:");
				for(int32_t i = 0;i<numOfContExpr;i++){
					contExpr[i]->print(out);
					if(i != numOfContExpr-1) out->push(" || ");
				}
				
			}else if(exprType == AND){
				if(numOfContExpr < 2) out->push("alone and:");
				for(int32_t i = 0;i<numOfContExpr;i++){
					bool paren = false;
					if(contExpr[i]->exprType == OR) paren = true;
					
					if(paren) out->push("(");
					contExpr[i]->print(out);
					if(paren) out->push(")");
					if(i != numOfContExpr-1) out->push(" && ");
				}
			}else if(exprType == NOT){
				bool paren = false;
				if(contExpr[0]->exprType == OR || contExpr[0]->exprType == AND || contExpr[0]->exprType == SUM || contExpr[0]->exprType == PROD) paren = true;
				out->push("!");
				if(paren) out->push("(");
				contExpr[0]->print(out);
				if(paren) out->push(")");
			}else if(exprType == PROD){
				if(numOfContExpr < 2) out->push("alone product:");
				
				int32_t indexOfNeg = -1;
				bool neg = false;
				bool negOne = false;
				for(int32_t i = 0;i < numOfContExpr;i++){
					if(contExpr[i]->exprType == NUM && contExpr[i]->value.neg()){
						if(contExpr[i]->value.equalsI(-1L)) negOne = true;
						neg = !neg;
						indexOfNeg = i;
						break;
					}
				}
				
				if(neg){
					bool nextIsDiv = false;
					if(numOfContExpr>1){
						if(indexOfNeg == 0){
							if(contExpr[1]->exprType == POW){
								if(contExpr[1]->expoIsMinusOne()) nextIsDiv = true;
							}
						}else{
							if(contExpr[0]->exprType == POW){
								if(contExpr[0]->expoIsMinusOne()) nextIsDiv = true;
							}
						}
					}
					
					if(negOne && !nextIsDiv) out->push("-");
					else{
						contExpr[indexOfNeg]->print(out);
						if(!nextIsDiv){
							if(FANCY_CHARS) out->push("·");
							else out->push("*");
						}
					}
				}
				for(int32_t i = 0;i < numOfContExpr;i++){
					
					if(i == indexOfNeg) continue;
					bool pr = false;
					
					if(contExpr[i]->exprType == PROD || contExpr[i]->exprType == SUM || contExpr[i]->exprType == EQU) pr = true;
					
					if(pr) out->push("(");
					if(contExpr[i]){
						if(contExpr[i]->exprType == POW){
							if(contExpr[i]->expoIsMinusOne()){
								if(i == 0 && !neg) out->push("1/");
								else out->push("/");
								bool paren = false;
								if(contExpr[i]->getBase()->exprType == PROD || contExpr[i]->getBase()->exprType == SUM || contExpr[i]->getBase()->exprType == POW) paren = true;
								if(paren) out->push("(");
								contExpr[i]->getBase()->print(out);
								if(paren) out->push(")");
						
							}else contExpr[i]->print(out);
						}else contExpr[i]->print(out);
					}
					if(pr) out->push(")");
					
					bool div = false;
					if(i!=numOfContExpr-1){
						Expr *next = contExpr[i+1];
						if(next->exprType == POW){
							if(next->expoIsMinusOne()) div = true;
						}
					}
					
					if(! (i==numOfContExpr-1 || (indexOfNeg == numOfContExpr-1 && i+1==indexOfNeg))){
						if(!div) {
							if(FANCY_CHARS) out->push("·");
							else out->push("*");
						}
					}
					
				}
			}else if(exprType == LOG){
				out->push("ln(");
				if(contExpr[0]) contExpr[0]->print(out);
				out->push(")");
			}else if(exprType == DERI){
				if(FANCY_CHARS) out->push("𝕕(");
				else out->push("d(");
				if(contExpr[0]) contExpr[0]->print(out);
				out->push(")");
			}else if(exprType == ABS){
				out->push("|");
				if(contExpr[0]) contExpr[0]->print(out);
				out->push("|");
			}else if(exprType == LIST){
				out->push("[");
				for(int32_t i = 0;i<numOfContExpr;i++){
					if(contExpr[i]) contExpr[i]->print(out);
					if(i != numOfContExpr-1) out->push(",");
				}
				out->push("]");
			}else if(exprType == SUBST){
				out->push("subst(");
				if(contExpr[0]) contExpr[0]->print(out);
				out->push(",");
				if(contExpr[1]) contExpr[1]->print(out);
				out->push(")");
			}else if(exprType == ASIN){
				out->push("asin(");
				if(contExpr[0]) contExpr[0]->print(out);
				out->push(")");
			}else if(exprType == ACOS){
				out->push("acos(");
				if(contExpr[0]) contExpr[0]->print(out);
				out->push(")");
			}else if(exprType == TAN){
				out->push("tan(");
				if(contExpr[0]) contExpr[0]->print(out);
				out->push(")");
			}else if(exprType == ATAN){
				out->push("atan(");
				if(contExpr[0]) contExpr[0]->print(out);
				out->push(")");
			}else if(exprType == CI){
				out->push("Ci(");
				if(contExpr[0]) contExpr[0]->print(out);
				out->push(")");
			}else if(exprType == SI){
				out->push("Si(");
				if(contExpr[0]) contExpr[0]->print(out);
				out->push(")");
			}
			
			if(direction == LEFT){
				out->push("_L)");
			}else if(direction == RIGHT){
				out->push("_R)");
			}
		}
		void println(Str *out){
			print(out);
			out->push("\n");
		}
		void tPrint(){
			Str out;
			print(&out);
			out.tPrint();
		}
		void tPrintln(){
			Str out;
			println(&out);
			out.tPrint();
		}
		
		void basicPrintln(Str *out){
			basicPrint(out);
			out->push("\n");
		}
		
		const bool TEST_ADD_ELEMENT = false;
		const int32_t MEMORY_JUMP = 3;
		
		void addElement(Expr *expr){//add element to expression
			simple = false;
			numOfContExpr++;
			if(numOfContExpr>memory){
				if(TEST_ADD_ELEMENT) memory++;
				else memory+=MEMORY_JUMP;//memory increase when going over memory limit
				
				if(TEST_ADD_ELEMENT){
					if(memory != numOfContExpr){
						printf("ERROR memory is not equal to numOfContExpr\n");
						printf("fixing...\n");
						memory = numOfContExpr;
						while(true) {}
					}
				}
				if(numOfContExpr == 1) contExpr = (Expr**)mallocO(sizeof(Expr*)*memory,EXPR_POINTER);
				else contExpr = (Expr**)reallocO(contExpr,sizeof(Expr*)*memory);
			}
			
			contExpr[numOfContExpr-1] = expr;//last element pushed at end
		}
		
		void removeElement(int32_t index){//remove element from expression
			simple = false;
			if(contExpr[index]){
				delete contExpr[index];
				contExpr[index] = nullptr;
			}
			int32_t index2 = 0;
			for(int32_t i = 0;i<numOfContExpr;i++){
				if(i!=index){
					contExpr[index2] = contExpr[i];
					index2++;
				}
			}
			numOfContExpr--;
			
			if(numOfContExpr == 0){
				freeO(contExpr);
				memory = 0;
				contExpr = nullptr;
			}
		}
		
		void clearElements(){//delete all elements
			simple = false;
			if(!contExpr) return;
			for(int32_t i = 0;i<numOfContExpr;i++){
				if(contExpr[i]) delete contExpr[i];
			}
			freeO(contExpr);
			contExpr = nullptr;
			numOfContExpr = 0;
			memory = 0;
		}
		
		Expr *copy(){//return a copy of this expression
			if(exprType == VAR){
				Expr *cpy = new Expr(name);
				cpy->direction = direction;
				cpy->simple = simple;
				return cpy;
			}else if(exprType == NUM){
				Expr *cpy = new Expr(&value);
				cpy->simple = simple;
				cpy->direction = direction;
				return cpy;
			}else{
				
				Expr *expr = new Expr(exprType);
				expr->numOfContExpr = numOfContExpr;
				expr->contExpr = (Expr**)mallocO(sizeof(Expr*)*numOfContExpr,EXPR_POINTER);
				expr->memory = numOfContExpr;
				for(int32_t i = 0;i<numOfContExpr;i++){
					expr->contExpr[i] = contExpr[i]->copy();
				}
				expr->simple = simple;
				expr->direction = direction;
				return expr;
			}
		}
		double hash(){//generate hash from expression , used for sorting objects in consistant order. This is very bad and slow
			//warning: incomplete :list
			double multiplier = 1.0;
			if(direction == LEFT) multiplier=0.99;
			else if(direction == RIGHT) multiplier = 0.98;
			if(exprType == NUM){
				
				Num cpy(&value);
				cpy.convertToFloat();
				double v = cpy.valueF;
				return (v/(fabs(4.0*v)+4.0)+0.75)*multiplier;
				
			}else if(exprType == VAR){
				double v = 0.5;
				int32_t i = 0;
				while(name->chars[i] != 0){
					v = pow(1.0/name->chars[i],v);
					i++;
				}
				return v*multiplier*0.5;
			}else if(exprType == SUM || exprType == OR){
				double v = 0.0;
				for(int32_t i = 0; i < numOfContExpr;i++){
					v+=contExpr[i]->hash();
				}
				return (v-floor(v))*multiplier*0.5;
			}else if(exprType == PROD || exprType == AND){
				double v = 0.0;
				for(int32_t i = 0; i < numOfContExpr;i++){
					v+=contExpr[i]->hash();
				}
				return pow((v-floor(v)),0.7324)*multiplier*0.5;
			}else if(exprType == LIST){
				double v = 0.0;
				for(int32_t i = 0; i < numOfContExpr;i++){
					v+=contExpr[i]->hash();
				}
				return pow((v-floor(v)),0.987423)*multiplier*0.5;
			}else{
				double v = 0.7423;
				for(int32_t i = 0;i< numOfContExpr;i++) v = pow(contExpr[i]->hash()/exprType,v);
				return v*multiplier*0.5;
			}
		}
		bool equalStruct(Expr *other){//compares if two expressions are equal in structure
			if(other->exprType != exprType) return false;
			
			if(exprType == VAR){
				if( strcmp(name->chars, other->name->chars) == 0) return true;
				else return false;
			}else if(exprType == NUM){
				return value.equals(&other->value);
			}else if(exprType == POW ||exprType == SOLVE){
				return contExpr[0]->equalStruct(other->contExpr[0]) && contExpr[1]->equalStruct(other->contExpr[1]);
			}else if(exprType == LOG || exprType == ABS || exprType == DERI || exprType == SIN || exprType == COS || exprType == INTEG || exprType == ASIN || exprType == ACOS || exprType == TAN || exprType == ATAN || exprType == NOT){
				return contExpr[0]->equalStruct(other->contExpr[0]);
			}else if(exprType == SUM || exprType == PROD || exprType == LIST || exprType == EQU || exprType == OR || exprType == AND){
				if(numOfContExpr != other->numOfContExpr) return false;
				bool *used = (bool*)mallocO(numOfContExpr*sizeof(bool),EXPR_POINTER);
				for(int32_t m = 0;m < numOfContExpr;m++) used[m] = false;
				for(int32_t i = 0;i < numOfContExpr;i++){
					bool found = false;
					
					for(int32_t j = 0;j < numOfContExpr;j++){
						if(used[j]) continue;
						if(contExpr[i]->equalStruct(other->contExpr[j])){
							used[j] = true;
							found = true;
							break;
						}
					}
					
					if(!found){
						freeO(used);
						return false;
					}
				}
				freeO(used);
				return true;
				
			}
			
			if( ERRORS) printf("can't compare objects: missing function %d\n",exprType);
			return false;
		}
		
		bool constant(){//contains no variables, although if a variable ends with a "." it is treated as constant
			if(exprType == NUM) return true;
			else if(exprType == VAR){
				
				if(name->chars[name->len-1] == '.'){
					return true;
				}
			}
			if(contExpr){
				for(int32_t i = 0;i< numOfContExpr;i++){
					if(!contExpr[i]->constant()) return false;
				}
				return true;
			}
			return false;
		}
		
		bool isI(){
			if(exprType == POW){
				if(getExpo()->exprType == POW && getBase()->exprType == NUM && getBase()->value.equalsI(-1L)){
					Expr *expoPow = getExpo();
					if(expoPow->expoIsMinusOne() && expoPow->getBase()->exprType == NUM && expoPow->getBase()->value.equalsI(2L)){
						return true;
					}
				}
			}
			return false;
		}
		
		bool contains(Expr *var){//check if an expression contains a variable or expr. Not full proof because if var is a product it may not work
			if(equalStruct(var)) return true;
			else for(int32_t i = 0;i < numOfContExpr;i++) if(contExpr[i]->contains(var)) return true;
			
			return false;
		}
		bool containsType(int32_t exprT){//checks for a expression type in expression
			if(exprType == exprT) return true;
			else{
				for(int32_t i = 0;i<numOfContExpr;i++){
					if(contExpr[i]->containsType(exprT)){
						return true;
					}
				}
			}
			return false;
		}
		bool containsVars(){//like of constant() except also detects constant variables. Do not get this confused
			if(exprType == VAR) return true;
			else for(int32_t i = 0;i < numOfContExpr;i++) if(contExpr[i]->containsVars()) return true;
			
			return false;
		}
		void getAllExprInBool(Expr *list){
			if(exprType == AND || exprType == OR || exprType == NOT){
				for(int32_t i = 0;i<numOfContExpr;i++) contExpr[i]->getAllExprInBool(list);
			}else{
				list->addElement(copy());
			}
		}
		void nullify(){//set all contained expressions to nullptr
			for (int32_t i = 0;i<numOfContExpr;i++) contExpr[i] = nullptr;
		}
		void becomeInternal(Expr *other){//become an object within this object
			Expr *copyOfOther = other->copy();
			clearElements();
			simple = copyOfOther->simple;
			setName(copyOfOther->name);
			contExpr = copyOfOther->contExpr;
			if(direction == MIDDLE) direction = copyOfOther->direction;
			numOfContExpr = copyOfOther->numOfContExpr;
			memory = copyOfOther->memory;
			exprType = copyOfOther->exprType;
			value.setValue(&copyOfOther->value);
			copyOfOther->contExpr = nullptr;
			copyOfOther->numOfContExpr = 0;
			delete copyOfOther;
		}
		void become(Expr *other){//become another object
			clearElements();
			exprType = other->exprType;
			numOfContExpr = other->numOfContExpr;
			other->numOfContExpr = 0;
			memory = other->memory;
			simple = other->simple;
			direction = other->direction;
			setName(other->name);
			contExpr = other->contExpr;
			other->contExpr = nullptr;
			value.setValue(&other->value);
			delete other;
		}
		int32_t nestDepth(){//how many expression within expressions, used in integration
			if(numOfContExpr == 0){
				return 1;
			}
			int32_t max = 0;
			for(int32_t i = 0;i < numOfContExpr;i++){
				int32_t depth = contExpr[i]->nestDepth();
				if(depth>max){
					max = depth;
				}
			}
			return max+1;
		}
		//deletion of expression
		~Expr(){
			clearElements();
			delete name;
			objCount--;
			int32_t index = findPointer(this);
			trackedBytes[index].bytes-=sizeof(Expr);
			trackedBytes[index].objType = (byte)-1;
			trackedBytes[index].pointer = nullptr;
			bytesUsed-=sizeof(Expr);
		}
		
		//special moves
		void distr();//districution
		void factor();//factoring
		void factorQuad();
		void partFrac(Expr *var);//partial fraction
		
		//simplify algarithms
		void simplify();
		void simplify(bool addFractions);//simplify with flag for adding fractions, used for distribution
		void derivSimp();
		void powSimp();
		void sumSimp(bool addFractions);//flag for adding fractions, used for distribution
		void prodSimp();
		void logSimp();
		void solverSimp();
		void absSimp();
		void listSimp();
		void equSimp();//equation simplify
		void sinSimp();
		void cosSimp();
		void asinSimp();
		void acosSimp();
		void integSimp();//integral simplify
		void andSimp();
		void orSimp();
		void notSimp();
		
		byte exprSign();//get the sign of expression (asumes variables to be positive)
		int64_t getDegree(Expr *var);
		Expr *polyExtract(Expr *var);//returns a list of polynomial coefficients
		Expr *complexExtract();//returns a+b*i to [a,b]
		
		//substitution
		void replace(Expr* old,Expr *repl){//replace a variable/substitution, not full proof
			if(contains(old)){
				simple = false;
				if(equalStruct(old)){
					
					become(repl->copy());
				}else{
					for(int32_t i = 0;i<numOfContExpr;i++){
						contExpr[i]->replace(old,repl);
					}
				}
			}
		}
	};
	
	void printObjectsInMemory(){//prints all the pointers in memory
		if(objCount==0 && bytesUsed == 0 && mallocActivePointers == 0){
			printf("NO MEMORY USED\n");
			return;
		}
		printObjCount();
		for(int32_t i = 0;i<totalTrackedBytesSize;i++){
			if(trackedBytes[i].pointer){
				printf("address: %p, size: %d bytes,",trackedBytes[i].pointer,trackedBytes[i].bytes);
				printf(" object type: ");
				if(trackedBytes[i].objType == CHARS){
					printf("CHARS");
					printf(", str: \"%s\"",(char*)trackedBytes[i].pointer);
				}
				else if(trackedBytes[i].objType == EXPR_POINTER) printf("EXPR_POINTER");
				else if(trackedBytes[i].objType == STR) printf("STR");
				else if(trackedBytes[i].objType == EXPR){
					printf("EXPR");
					printf(", exprType: %s",exprNames[ ((Expr*)(trackedBytes[i].pointer))->exprType ]);
				}
				else printf("?");
				
				printf("\n");
			}
		}
	}
	
	int32_t readBluePrint(Str *in,Expr *parent,int32_t total){
		int32_t ogTotal = total;
		byte exprType = readHexString(in, sizeof(Expr::exprType),total);
		total+=sizeof(Expr::exprType)*2;
		
		parent->exprType = exprType;
		if(exprType == NUM){
			byte rep = readHexString(in, sizeof(Num::rep),total);
			total+=sizeof(Num::rep)*2;
			parent->value.rep = rep;
			if(rep == INT || rep == FLOAT){
				int64_t temp;
				int32_t small = 0;
				if(in->chars[total] == 'V') small = 1;
				else if(in->chars[total] == 'S') small = 2;
				total++;
				if(small == 1){
					temp = readHexString(in, sizeof(int16_t),total);
					temp = (int64_t)(*((int16_t*)(&temp)));//yeah I know this is a strange way to cast
					total+=sizeof(int16_t)*2;
				}else if(small == 2){
					temp = readHexString(in, sizeof(int32_t),total);
					temp = (int64_t)(*((int32_t*)(&temp)));
					total+=sizeof(int32_t)*2;
				}else{
					temp = readHexString(in, sizeof(int64_t),total);
					total+=sizeof(int64_t)*2;
				}
				if(rep == INT) parent->value.valueI = temp;
				else parent->value.valueF = *((double*)(&temp));
			}
		}else if(exprType == VAR){
			int16_t nameLength = (int32_t)readHexString(in, sizeof(Str::len),total);
			total+=sizeof(Str::len)*2;
			parent->name = new Str();
			for(int32_t i = 0;i<nameLength;i++) parent->name->push(in->chars[i+total]);
			total+=nameLength;
		}else{
			int16_t numOfContExpr = 0;
			if(in->chars[total] == 'V'){
				char temp[2];
				temp[0] = '0';
				temp[1] = in->chars[total+1];
				Str tmp(temp);
				numOfContExpr = (int16_t)readHexString(&tmp, 1,0);
				total+=2;
			}else{
				numOfContExpr = (int16_t)readHexString(in, sizeof(Expr::numOfContExpr),total);
				total+=sizeof(Expr::numOfContExpr)*2;
			}
			for(int32_t i = 0;i<numOfContExpr;i++){
				Expr *child = new Expr('\0');
				total+=readBluePrint(in, child,total);
				parent->addElement(child);
			}
		}
		
		return total-ogTotal;
	}

	Expr *readBluePrint(Str *in){
		Expr *out = new Expr('\0');
		readBluePrint(in, out,0);
		return out;
	}
	
	//easy programming
	Expr *varC(const char *name){
		return new Expr(name);
	}
	Expr *numC(int64_t val){
		return new Expr(val);
	}
	Expr *numC(Str *str){
		
		Num n(str);
		Expr *expr = new Expr(&n);
		return expr;
	}
	Expr *numFC(double val){
		return new Expr(val);
	}
	Expr *boolC(bool b){
		return new Expr(b);
	}
	Expr *orC(Expr *a,Expr *b){
		return new Expr(OR,a,b);
	}
	Expr *andC(Expr *a,Expr *b){
		return new Expr(AND,a,b);
	}
	Expr *orC(Expr *a,Expr *b,Expr *c){
		Expr *out = new Expr(OR,a,b);
		out->addElement(c);
		return out;
	}
	Expr *andC(Expr *a,Expr *b,Expr *c){
		Expr *out = new Expr(AND,a,b);
		out->addElement(c);
		return out;
	}
	Expr *notC(Expr *e){
		return new Expr(NOT,e);
	}
	Expr *invC(Expr *expr){
		return new Expr(POW,expr,new Expr((int64_t)-1L));
	}
	Expr *logC(Expr *expr){
		return new Expr(LOG,expr);
	}
	
	//2
	Expr *sumC(Expr *expr1,Expr *expr2){
		return new Expr(SUM,expr1,expr2);
	}
	Expr *prodC(Expr *expr1,Expr *expr2){
		return new Expr(PROD,expr1,expr2);
	}
	//3
	Expr *sumC(Expr *expr1,Expr *expr2,Expr *expr3){
		Expr *sm = new Expr(SUM,expr1,expr2);
		sm->addElement(expr3);
		return sm;
	}
	Expr *prodC(Expr *expr1,Expr *expr2,Expr *expr3){
		Expr *pr = new Expr(PROD,expr1,expr2);
		pr->addElement(expr3);
		return pr;
	}
	
	
	Expr *divC(Expr *expr1,Expr *expr2){
		return prodC(expr1,invC(expr2));
	}
	
	Expr *powC(Expr *expr1,Expr *expr2){
		return new Expr(POW,expr1,expr2);
	}
	Expr *diffC(Expr *expr){
		return new Expr(DERI,expr);
	}
	Expr *equC(Expr *left,Expr *right){
		return new Expr(EQU,left,right);
	}
	Expr *absC(Expr *expr){
		return new Expr(ABS,expr);
	}
	Expr *eC(){
		Num v(EV);
		return new Expr(&v);
	}
	Expr *piC(){
		Num v(PIV);
		return new Expr(&v);
	}
	Expr *integC(Expr *expr){
		return new Expr(INTEG,expr);
	}
	Expr *sinC(Expr *expr){
		return new Expr(SIN,expr);
	}
	Expr *cosC(Expr *expr){
		return new Expr(COS,expr);
	}
	Expr *tanC(Expr *expr){
		return new Expr(TAN,expr);
	}
	Expr *asinC(Expr *expr){
		return new Expr(ASIN,expr);
	}
	Expr *acosC(Expr *expr){
		return new Expr(ACOS,expr);
	}
	Expr *atanC(Expr *expr){
		return new Expr(ATAN,expr);
	}
	Expr *SiC(Expr *expr){
		return new Expr(SI,expr);
	}
	Expr *CiC(Expr *expr){
		return new Expr(CI,expr);
	}
	Expr *iC(){
		return powC(numC(-1L),invC(numC(2L)));
	}
	
	Expr *sqrtC(Expr *expr){
		return powC(expr,invC(numC(2L)));
	}
	Expr *sqC(Expr *expr){
		return powC(expr,numC(2L));
	}
	Expr *negC(Expr *expr){
		return prodC(numC(-1L),expr);
	}
	Expr *subC(Expr *expr1,Expr *expr2){
		return sumC(expr1,negC(expr2));
	}
	Expr *expC(Expr *expr){
		return powC(eC(),expr);
	}
	
	
	Expr *primeFactor(int64_t num){//returns a product of powers
		Expr *pr = new Expr(PROD);
		
		int64_t max = sqrt(num)+1;//makes it more efficient
		for(int64_t i = 2;i < max;i++){
			Expr *pw = nullptr;
			while(num%i==0){
				if(pw) pw->getExpo()->value.valueI++;
				else pw = powC(numC(i),numC(1L));
				num/=i;
			}
			if(pw){
				pr->addElement(pw);
				max = sqrt(num)+2;
			}
		}
		if(num != 1) pr->addElement(powC(numC(num),numC(1L)));
		
		return pr;
	}
	void Expr::derivSimp(){//derivative, very simple
		if(exprType == DERI){
			//
			if(contExpr[0]->constant()){
				clearElements();
				exprType = NUM;
				value.setValueI(0L);
			}else if(contExpr[0]->exprType == POW){
				Expr *pw = contExpr[0];
				if(pw->getExpo()->constant()){
					Expr *bs = pw->getBase();
					Expr *ex = pw->getExpo();
					pw->nullify();
					
					Expr *repl = prodC(diffC(bs),powC(bs->copy(),sumC(ex,numC(-1L))));
					repl->addElement(ex->copy());
					repl->simplify();
					become(repl);
				}else{
					addElement(copy());
					exprType = PROD;
					contExpr[1]->contExpr[0]->exprType = PROD;
					contExpr[1]->contExpr[0]->contExpr[0] = logC(contExpr[1]->contExpr[0]->contExpr[0]);
					simplify();
				}
			}else if(contExpr[0]->exprType == LOG){
				addElement( diffC(contExpr[0]->contExpr[0]->copy()));
				contExpr[0]->exprType = POW;
				contExpr[0]->addElement(numC(-1L));
				exprType = PROD;
				simple = false;
				simplify();
			}else if(contExpr[0]->exprType == SUM){
				becomeInternal(contExpr[0]);
				for(int32_t i = 0;i < numOfContExpr;i++) contExpr[i] = diffC(contExpr[i]);
				simple = false;
				simplify();
			}else if(contExpr[0]->exprType == PROD){
				Expr *prod = contExpr[0];
				
				Expr *sum = new Expr(SUM);
				for(int32_t i = 0;i<prod->numOfContExpr;i++){
					Expr *prodCopy = prod->copy();
					prodCopy->simple = false;
					prodCopy->contExpr[i] = diffC(prodCopy->contExpr[i]);
					sum->addElement(prodCopy);
				}
				
				become(sum);
				simplify();
			}else if(contExpr[0]->exprType == ABS){
				Expr *pr = new Expr(PROD);
				pr->addElement(contExpr[0]->contExpr[0]->copy());
				pr->addElement(invC(contExpr[0]));
				pr->addElement(diffC(contExpr[0]->contExpr[0]->copy()));
				nullify();
				
				become(pr);
				simplify();
			}else if(contExpr[0]->exprType == EQU){
				contExpr[0]->contExpr[0] = diffC(contExpr[0]->contExpr[0]);
				contExpr[0]->contExpr[1] = diffC(contExpr[0]->contExpr[1]);
				becomeInternal(contExpr[0]);
				simple = false;
				simplify();
			}else if(contExpr[0]->exprType == LIST){
				becomeInternal(contExpr[0]);
				for(int32_t i = 0;i<numOfContExpr;i++){
					contExpr[i] = diffC(contExpr[i]);
				}
				simple = false;
				simplify();
			}else if(contExpr[0]->exprType == SIN){
				Expr *repl = prodC(cosC(contExpr[0]->contExpr[0]->copy()),diffC(contExpr[0]->contExpr[0]->copy()));
				
				become(repl);
				simplify();
			}else if(contExpr[0]->exprType == COS){
				Expr *repl = prodC(sinC(contExpr[0]->contExpr[0]->copy()),diffC(contExpr[0]->contExpr[0]->copy()));
				repl->addElement(numC(-1L));
				
				become(repl);
				simplify();
			}else if(contExpr[0]->exprType == TAN){
				Expr *repl = prodC(sumC(powC(tanC(contExpr[0]->contExpr[0]->copy()),numC(2L)),numC(1L)),diffC(contExpr[0]->contExpr[0]->copy()));
				
				become(repl);
				simplify();
			}else if(contExpr[0]->exprType == ATAN){
				Expr *repl = prodC(invC(sumC(powC(contExpr[0]->contExpr[0]->copy(),numC(2L)),numC(1L))),diffC(contExpr[0]->contExpr[0]->copy()));
				
				become(repl);
				simplify();
			}else if(contExpr[0]->exprType == ASIN){
				Expr *repl = prodC(powC(sumC(numC(1L),prodC(numC(-1L),powC(contExpr[0]->contExpr[0]->copy(),numC(2L)))),invC(numC(-2L))),diffC(contExpr[0]->contExpr[0]->copy()));
				
				become(repl);
				simplify();
			}else if(contExpr[0]->exprType == ACOS){
				Expr *repl = prodC(powC(sumC(numC(1L),prodC(numC(-1L),powC(contExpr[0]->contExpr[0]->copy(),numC(2L)))),invC(numC(-2L))),diffC(contExpr[0]->contExpr[0]->copy()));
				repl->addElement(numC(-1L));
				
				become(repl);
				simplify();
			}else if(contExpr[0]->exprType == SI){
				Expr *v = contExpr[0]->contExpr[0];
				contExpr[0]->contExpr[0] = nullptr;
				Expr *repl = prodC(sinC(v),invC(v->copy()));
				repl->addElement(diffC(v->copy()));
				become(repl);
				simplify();
			}else if(contExpr[0]->exprType == CI){
				Expr *v = contExpr[0]->contExpr[0];
				contExpr[0]->contExpr[0] = nullptr;
				Expr *repl = prodC(cosC(v),invC(v->copy()));
				repl->addElement(diffC(v->copy()));
				become(repl);
				simplify();
			}else if(contExpr[0]->exprType == INTEG){
				becomeInternal(contExpr[0]->contExpr[0]);
			}
		}
	}
	
	void Expr::powSimp(){//simplify power
		if(exprType == POW){
			//println();
			{//can't tell with variables if epsilon increases or decrases
				if(getBase()->containsVars()) getBase()->direction = MIDDLE;
				if(getExpo()->containsVars()) getExpo()->direction = MIDDLE;
			}
			
			if(getBase()->exprType == EQU){//equation in base
				Expr *ex = getExpo()->copy();
				becomeInternal(getBase());
				contExpr[0] = powC(contExpr[0],ex);
				contExpr[1] = powC(contExpr[1],ex->copy());
				simple = false;
				simplify();
				return;
			}
			if(getExpo()->exprType == EQU){//equation in expo
				Expr *bs = getBase()->copy();
				becomeInternal(getExpo());
				contExpr[0] = powC(bs,contExpr[0]);
				contExpr[1] = powC(bs->copy(),contExpr[1]);
				simple = false;
				simplify();
				return;
			}
			
			{//list in base or expo
				if(getBase()->exprType == LIST){
					Expr *list = getBase();
					for(int32_t i = 0;i<list->numOfContExpr;i++){
						list->contExpr[i] = powC(list->contExpr[i],getExpo()->copy());
					}
					
					becomeInternal(list);
					simple = false;
					simplify();
					return;
				}else if(getExpo()->exprType == LIST){
					Expr *list = getExpo();
					for(int32_t i = 0;i<list->numOfContExpr;i++){
						list->contExpr[i] = powC(getBase()->copy(),list->contExpr[i]);
					}
					
					becomeInternal(list);
					simple = false;
					simplify();
					return;
				}
				
			}
			
			if(constant()){//(a+b)^n where n is integer -> (a+b)*(a+b)... n times, base must be sum and constant
				if(getBase()->exprType == SUM && getExpo()->exprType == NUM && getExpo()->value.rep == INT){
					int64_t n = getExpo()->value.valueI;
					if(n > 1 && n < 16){
						removeElement(1);
						exprType = PROD;
						for(int64_t i = 0;i < n-1;i++){
							addElement(contExpr[0]->copy());
						}
						simplify();
						return;
					}
				}
			}
			
			if(getBase()->exprType == POW){//(a^b)^c -> a^(b*c)
				
				Expr *a = getBase()->getBase();
				Expr *c = getExpo();
				Expr *t = getBase();
				t->exprType = PROD;
				setExpo(t);
				setBase(a);
				getExpo()->contExpr[0] = c;
				getExpo()->simple = false;
				bool evenInverseExpo = false;
				if(c->inverseInt()){
					if(c->getBase()->value.valueI % 2L == 0){
						evenInverseExpo = true;
					}
				}else if(c->exprType == PROD){
					for(int32_t i = 0;i<c->numOfContExpr;i++){
						if(c->contExpr[i]->exprType == POW){
							Expr *pw = c->contExpr[i];
							if(pw->inverseInt()){
								if(pw->getBase()->value.valueI %2L == 0){
									evenInverseExpo = true;
									break;
								}
							}
						}
					}
				}
				bool evenExpo = false;
				if(getExpo()->contExpr[1] ->exprType == NUM && getExpo()->contExpr[1]->value.rep == INT){
					if(getExpo()->contExpr[1]->value.valueI %2L == 0){
						evenExpo = true;
					}
				}else if(getExpo()->contExpr[1] ->exprType == PROD){
					Expr *pr = getExpo()->contExpr[1];
					for(int32_t i = 0; i < pr->numOfContExpr;i++){
						if(pr->contExpr[i]->exprType == NUM && pr->contExpr[i]->value.rep == INT){
							if(pr->contExpr[i]->value.valueI %2L == 0){
								evenExpo = true;
								break;
							}
						}
					}
				}
				if(evenExpo && evenInverseExpo){
					setBase(absC(getBase()));
					getBase()->simple = false;
					getBase()->simplify();
				}
				getExpo()->simplify();
			}
			if(getBase()->exprType == ABS){//|x|^(2*a) -> x^(2*a) && |x|^3 -> |x|*x^2
				getExpo()->factor();
				if(getExpo()->exprType == PROD){
					Expr *pr = getExpo();
					for(int32_t i = 0;i<pr->numOfContExpr;i++){
						if(pr->contExpr[i]->exprType == NUM && pr->contExpr[i]->value.rep == INT){
							if(labs(pr->contExpr[i]->value.valueI) %2L == 0){
								getBase()->becomeInternal(getBase()->contExpr[0]);
								break;
							}
						}
					}
				}else if(getExpo()->exprType == NUM && getExpo()->value.rep == INT){
					if(labs(getExpo()->value.valueI) %2L == 0){
						getBase()->becomeInternal(getBase()->contExpr[0]);
					}else if(labs(getExpo()->value.valueI) %2L == 1){
						getExpo()->value.valueI--;
						Expr *repl = prodC(getBase(),powC(getBase()->contExpr[0]->copy(),getExpo()));
						nullify();
						
						become(repl);
						simplify();
						return;
					}
				}
			}
			
			{//factor base and expo
				getBase()->factor();
				getExpo()->factor();
			}
			if(getBase()->exprType == NUM && getExpo()->exprType == NUM){//-1^-1-> -1
				if(getBase()->value.equalsI(-1L) && getExpo()->value.equalsI(-1L)){
					clearElements();
					exprType = NUM;
					value.setValueI(-1L);
					return;
				}
			}
			
			if(getBase()->exprType == NUM && getExpo()->exprType != NUM){//4^x->2^(x*2) && 2.0^(x*2.0) -> 4.0^x
			
				if(getBase()->value.rep == INT){
					int64_t b,e;
					perfectPower(getBase()->value.valueI,&b,&e);
					if(e != 1){
						getBase()->value.setValueI(b);
						Expr *repl = prodC(getExpo()->copy(),numC(e));
						delete getExpo();
						repl->simple = false;
						repl->simplify();
						setExpo(repl);
					}
				}else if(getBase()->value.rep == FLOAT && getExpo()->exprType == PROD){
					bool changed = false;
					for(int32_t i = 0;i < getExpo()->numOfContExpr;i++){
						if(getExpo()->contExpr[i]->exprType == NUM){
							getBase()->value.powN(&getExpo()->contExpr[i]->value);
							getExpo()->removeElement(i);
							i--;
							changed = true;
						}
					}
					if(changed){
						getExpo()->simple = false;
						getExpo()->simplify();
					}
				}
				
			}
			
			if(getExpo()->exprType == NUM && getExpo()->value.rep == INT){//(-infinity)^(-even number) -> 0+epsilon && (-infinity)^(-odd number) -> 0-epsilon
				if(getBase()->exprType == NUM && getBase()->value.rep == NEGINF){
					if(getExpo()->value.valueI%2L == 0){
						
						clearElements();
						exprType = NUM;
						value.rep = INF;
						return;
					}else{
						clearElements();
						exprType = NUM;
						value.rep = NEGINF;
						return;
					}
				}
			}
			
			if(getBase()->value.rep != EV) {//a^b -> e^(ln(a)*b)
				bool change = false;
				if((getBase()->exprType == NUM && (getBase()->value.rep == INF || getBase()->value.rep == NEGINF) ) || (getExpo()->exprType == NUM && (getExpo()->value.rep == INF || getExpo()->value.rep == NEGINF))){
					change = true;
				}
				if(getExpo()->exprType == POW){//x^(ln(x)^2) -> e^ln(x)^3
					Expr *pw = getExpo();
					if(pw->getBase()->exprType == LOG) change = true;
				}else if(getExpo()->exprType == PROD){
					Expr *pr = getExpo();
					for(int32_t i = 0;i < pr->numOfContExpr;i++){
						if(pr->contExpr[i]->exprType == POW){
							Expr *pw = pr->contExpr[i];
							if(pw->getBase()->exprType == LOG) change = true;
						}else if(pr->contExpr[i]->exprType == LOG) change = true;
						else if(pr->contExpr[i]->exprType == NUM && (pr->contExpr[i]->value.rep == INF || pr->contExpr[i]->value.rep == NEGINF)) change = true;
					}
				}else if(getExpo()->exprType == LOG) change = true;
				if(change){
					setExpo(prodC(getExpo(),logC(getBase())));
					Expr *repl = new Expr(NUM);
					repl->value.rep = EV;
					setBase(repl);
					getExpo()->simplify();
				}
			}
			
			if(getBase()->exprType == NUM && !getBase()->value.neg() && getExpo()->exprType == NUM){//c^(infinity) -> infinity && //c^(-infinity) -> 0+epsilon
				if(!getBase()->value.equalsI(1L)){
					if(getExpo()->value.rep == INF){
						clearElements();
						exprType = NUM;
						value.rep = INF;
						return;
					}else if(getExpo()->value.rep == NEGINF){
						clearElements();
						exprType = NUM;
						value.setValueI(0L);
						direction = RIGHT;
						return;
					}
				}
			}
			
			{//e^(complex) -> eulers identity
				Expr *cn = getExpo()->complexExtract();
				
				if(!(cn->contExpr[1]->exprType == NUM && cn->contExpr[1]->value.equalsI(0L))){
					bool baseIsE = getBase()->exprType == NUM && getBase()->value.rep == EV;
					if(!baseIsE) {//if base is not e add log ceof
						for(int32_t i = 0;i< 2;i++) cn->contExpr[i] = prodC(cn->contExpr[i],logC(getBase()->copy()));
					}
					Expr *repl = prodC(powC(eC(),cn->contExpr[0]),sumC(cosC(cn->contExpr[1]),prodC(iC(),sinC(cn->contExpr[1]->copy()))));
					cn->nullify();
					delete cn;
					become(repl);
					simplify();
					return;
				}else delete cn;
				
			}
			
			if(getBase()->exprType == NUM){//e^(ln(a)*b) -> a^b && e^ln(x) -> x && e^(ln(a)+b) -> a*e^b && e^(ln(a)*b+c) -> a^b*e^c
				if(getBase()->value.rep == EV){
					if(getExpo()->exprType == LOG){
						becomeInternal(getExpo()->contExpr[0]);
						return;
					}else if(getExpo()->exprType == PROD){
						Expr *pr = getExpo();
						int32_t count = 0;
						int32_t indexOfLog = -1;
						for(int32_t i = 0;i < pr->numOfContExpr;i++){
							if(pr->contExpr[i]->exprType == LOG && !pr->contExpr[i]->constant()){//2^ln(x) -> x^ln(2) makes integrals easier
								count++;
								indexOfLog = i;
							}
						}
						if(count == 0){
							for(int32_t i = 0;i < pr->numOfContExpr;i++){
								if(pr->contExpr[i]->exprType == LOG){
									count++;
									indexOfLog = i;
								}
							}
						}
						if(count == 1){
							delete getBase();
							setBase(pr->contExpr[indexOfLog]->contExpr[0]);
							pr->contExpr[indexOfLog]->contExpr[0] = nullptr;
							pr->removeElement(indexOfLog);
							getExpo()->simplify();

						}
						
					}
					
					else if(getExpo()->exprType == SUM){
						Expr *sm = getExpo();
						//find log
						Expr *prOut = new Expr(PROD);
						for(int32_t i = 0; i< sm->numOfContExpr;i++){
							if(sm->contExpr[i]->exprType == LOG){
								prOut->addElement(sm->contExpr[i]->contExpr[0]);
								sm->contExpr[i]->contExpr[0] = nullptr;
								sm->removeElement(i);
								i--;
							}else if(sm->contExpr[i]->exprType == PROD){
								int32_t count = 0;
								Expr *pr = sm->contExpr[i];
								int32_t indexOfLog = -1;
								for(int32_t j = 0;j < pr->numOfContExpr;j++){
									if(pr->contExpr[j]->exprType == LOG){
										count++;
										indexOfLog = j;
									}
								}
								if(count == 1){
									Expr *baseRepl = pr->contExpr[indexOfLog]->contExpr[0];
									pr->contExpr[indexOfLog]->contExpr[0] = nullptr;
									pr->removeElement(indexOfLog);
									Expr *pw = powC(baseRepl,pr);
									sm->contExpr[i] = nullptr;
									sm->removeElement(i);
									i--;
									prOut->addElement(pw);
								}
							}
						}
						
						if(prOut->numOfContExpr > 0){
							simple = false;
							prOut->addElement(copy());
							
							prOut->simplify();
							become(prOut);
							return;
						}else{
							delete prOut;
						}
						
					}
					
				}
			}
			
			{//(x*y)^a -> x^a*y^a
				if(getBase()->exprType == PROD){
					Expr *pr = getBase();
					setBase(nullptr);
					for(int32_t i = 0;i<pr->numOfContExpr;i++){
						pr->contExpr[i] = powC(pr->contExpr[i],getExpo()->copy());
					}
					become(pr);
					simple = false;
					simplify();
					return;
				}
			}
			
			if(getBase()->exprType == NUM && getBase()->value.rep == INT && (getExpo()->exprType == PROD||getExpo()->exprType == POW)){//c^(5/2) -> c^(2+1/2) convert to mixed fraction
				
				Expr *num = nullptr;
				Expr *den = nullptr;
				if(getExpo()->exprType == PROD){
					Expr *pr = getExpo();
					if(pr->numOfContExpr == 2){
						for(int32_t i = 0;i < 2;i++){
							if(pr->contExpr[i]->exprType == NUM && pr->contExpr[i]->value.rep == INT){
								num = pr->contExpr[i];
							}else if(pr->contExpr[i]->inverseInt()){
								den = pr->contExpr[i];
							}else {//does not fit
								den = nullptr;
								break;
							}
						}
					}
				}else if(getExpo()->exprType == POW){
					Expr *pw = getExpo();
					if(pw->inverseInt()){
						den = pw;
					}
				}
				if(den){
					int64_t n,d,v;
					
					
					if(num)n = num->value.valueI;
					else n = 1;
					d = den->getBase()->value.valueI;
					
					if(n>d){
					
						
						if(d>0 && n>0){
							v = n/d;
							n = n-v*d;
						}else{
							n = labs(n);
							d = labs(d);
							v = n/d+1;
							n = v*d-n;
							v=-v;
						}
						
						delete getExpo();
						Expr *repl = sumC(numC(v),prodC(numC(n),invC(numC(d))));
						
						setExpo(repl);
					}
				}
				
			}
			
			if(getExpo()->exprType == NUM){//x^1 -> x && x^0 -> 1
				
				if(getExpo()->value.equalsI(0L)){//0^0 indeterminate form
					if(getBase()->exprType == NUM && getBase()->value.equalsI(0L)){
						clearElements();
						exprType = INDET;
						return;
					}else{
						clearElements();
						exprType = NUM;
						value.setValueI(1L);
						return;
					}
				}else if(getExpo()->value.equalsI(1L)){
					becomeInternal(getBase());
					return;
				}
			}
			
			
			if(getBase()->exprType == NUM && getBase()->value.plain() && getExpo()->exprType == SUM){//2^(x+3) -> 8*2^x && 2^(x-3) -> 2^x*inv(8)
				Num n;
				Expr *sm = getExpo();
				bool found = false;
				for(int32_t i = 0;i<sm->numOfContExpr;i++){
					if(sm->contExpr[i]->exprType == NUM && sm->contExpr[i]->value.plain()){
						n.setValue(&sm->contExpr[i]->value);
						sm->removeElement(i);
						found = true;
						break;
					}
				}
				if(found){
					if(!n.neg()){
						Num bs(&getBase()->value);
						bs.powN(&n);
						Expr *pr = prodC(copy(),new Expr(&bs));
						
						become(pr);
					}else{
						n.absN();
						
						Num bs(&getBase()->value);
						bs.powN(&n);
						Expr *pr = prodC(copy(),invC(new Expr(&bs)));
						
						become(pr);
					}
					
					simplify();
					return;
				}
			}
			
			
			if(getBase()->exprType == NUM){//0^x -> 0 && 1^x -> 1
				
				if(getBase()->value.equalsI(0L)){
					
					if(expoIsMinusOne()){
						
						if(getBase()->direction == LEFT){//1/(0-epsilon)
							clearElements();
							exprType = NUM;
							value.rep = NEGINF;
						}else if(getBase()->direction == RIGHT){
							clearElements();
							exprType = NUM;
							value.rep = INF;
						}
						else{
							clearElements();
							exprType = NUM;
							value.rep = UNDEF;
						}
					}else{
						clearElements();
						exprType = NUM;
						value.setValueI(0L);
					}
					return;
				}else if(getBase()->value.equalsI(1L)){
					clearElements();
					exprType = NUM;
					value.setValueI(1L);
					return;
				}
			}
			
			if(getBase()->exprType == NUM){//12^(x/2) -> (2^2*3)^(x/2) -> 2^x*3^(x/2)
				if(getBase()->value.rep == INT){
					bool factorBase = false;
					int64_t invVal = 0;
					if(getExpo()->exprType == POW){
						Expr *pw = getExpo();
						if(pw->inverseInt()){
							invVal = pw->getBase()->value.valueI;
							factorBase = true;
						}
					}else if(getExpo()->exprType == PROD){
						Expr *pr = getExpo();
						for(int32_t i = 0;i < pr->numOfContExpr;i++){
							if(pr->contExpr[i]->exprType == POW){
								Expr *pw = pr->contExpr[i];
								if(pw->inverseInt()){
									invVal = pw->getBase()->value.valueI;
									factorBase = true;
									break;
								}
							}
						}
					}
					if(factorBase){
						Expr *pr = primeFactor(getBase()->value.valueI);
						bool worthIt = false;
						if(pr->numOfContExpr>1){
							for(int32_t i = 0;i < pr->numOfContExpr;i++){
								Expr *pw = pr->contExpr[i];
								if(pw->getExpo()->value.valueI >= invVal){
									worthIt = true;
									break;
								}
							}
						}
						if(worthIt){
							delete getBase();
							pr->simple = true;
							setBase(pr);
							simplify();
							return;
						}else delete pr;
					}
				}
			}
			
			if(getBase()->exprType == NUM && getExpo()->exprType == NUM){//basic numeric power
				if(getBase()->value.plain() && getExpo()->value.plain()){
					if(getExpo()->value.neg() && getExpo()->value.rep == INT && getBase()->value.rep == INT){
						getExpo()->value.absN();
						getBase()->value.powN(&getExpo()->value);
						getExpo()->value.setValueI(-1L);
					}else{
						getBase()->value.powN(&getExpo()->value);
						becomeInternal(getBase());
						return;
					}
				}
			}
			
		}
	}
	
	void Expr::distr(){//distribution
		if(exprType == PROD){
			Expr *sm = nullptr;
			for(int32_t i = 0; i < numOfContExpr;i++){
				if(contExpr[i]->exprType == SUM) {
					sm = contExpr[i];
					contExpr[i] = nullptr;
					removeElement(i);
					break;
				}
			}
			if(sm != nullptr){
				
				for(int32_t i = 0;i < sm->numOfContExpr;i++){
					sm->contExpr[i] = prodC(sm->contExpr[i],copy());
				}
				become(sm);
				simple = false;
				simplify(false);
			}
		}
	}
	
	void Expr::factorQuad(){
		if(exprType == SUM){
			Expr *var = nullptr;//find the (something)^2
			
			for(int32_t i = 0;i<numOfContExpr;i++){//this tries to find the variable
				
				if(contExpr[i]->exprType == POW){
					Expr *pw = contExpr[i];
					if(pw->getExpo()->exprType == NUM && pw->getExpo()->value.equalsI(2L)){
						var = pw->getBase();
						break;
					}
				}else if(contExpr[i]->exprType == PROD){
					Expr *pr = contExpr[i];
					for(int32_t j = 0;j < pr->numOfContExpr;j++){
						if(pr->contExpr[j]->exprType == POW){
							Expr *pw = pr->contExpr[j];
							if(pw->getExpo()->exprType == NUM && pw->getExpo()->value.equalsI(2L)){
								var = pw->getBase();
								break;
							}
							
						}
					}
					if(var) break;
				}
			}
			
			if(var){
				
				Expr *polyList = polyExtract(var);
				
				if(polyList){
					
					for(int32_t i = 0;i<polyList->numOfContExpr;i++){
						if(polyList->contExpr[i]->exprType != NUM){
							delete polyList;
							return;
						}
					}
					
					
					Expr *a = polyList->contExpr[2];
					Expr *b = polyList->contExpr[1];
					Expr *c = polyList->contExpr[0];
					
					
					polyList->nullify();
					delete polyList;
					
					Expr *pr = new Expr(PROD);
					
					
					Expr *twoAX = new Expr(PROD);//2ax
					twoAX->addElement(numC(2L));
					twoAX->addElement(a->copy());
					twoAX->addElement(var->copy());
					
					Expr *twoAXpB = sumC(twoAX,b);//2ax+b
					
					Expr *fourAC = new Expr(PROD);//-4ac
					fourAC->addElement(numC(-4L));
					fourAC->addElement(a->copy());
					fourAC->addElement(c);
					
					Expr *sqBSqm4AC = powC(sumC(powC(b->copy(),numC(2L)),fourAC),invC(numC(2L)));//sqrt(b^2-4ac)
					//
					
					Expr *rFact = twoAXpB->copy();
					rFact->addElement(prodC(numC(-1L),sqBSqm4AC->copy()));
					
					Expr *lFact = twoAXpB;
					lFact->addElement(sqBSqm4AC);
					
					pr->addElement(lFact);
					pr->addElement(rFact);
					
					pr->addElement(invC(a));
					pr->addElement(invC(numC(4L)));
					
					become(pr);
					simplify();
				}
				
			}
			
		}
	}
	
	void Expr::factor(){//factors common variables and numbers, does not factor polynomials
		if(exprType == SUM){
			simplify();
			//printf("factoring\n");
			
			if(exprType != SUM) return;
			
			
			{//move leading hash to front
				double max = 0.0;
				int32_t indexOfLeader = -1;
				for(int32_t i = 0;i < numOfContExpr;i++){
					if(contExpr[i]->exprType == PROD){
						Expr *pr = contExpr[i];
						Expr *prCpy = pr->copy();
						for(int32_t j = 0;j < pr->numOfContExpr;j++){
							if(pr->contExpr[j]->exprType == NUM && pr->contExpr[j]->value.plain()){
								prCpy->removeElement(j);
								break;
							}
						}
						prCpy->simplify();
						double v = prCpy->hash();
						delete prCpy;
						if(v>max){
							indexOfLeader = i;
							max = v;
						}
					}else if(contExpr[i]->exprType == NUM){
						
						Expr *cpy = contExpr[i]->copy();
						cpy->value.absN();
						cpy->value.convertToFloat();
						double v = cpy->hash();
						if(v>max){
							max = v;
							indexOfLeader = i;
						}
						delete cpy;
						
					}else{
						
						double v = contExpr[i]->hash();
						if(v>max){
							max = v;
							indexOfLeader = i;
						}
						
					}
				}
				Expr *ogFirst = contExpr[0];
				contExpr[0] = contExpr[indexOfLeader];
				contExpr[indexOfLeader] = ogFirst;
			}
			bool leaderIsNeg = false;
			if(contExpr[0]->exprType == NUM){
				if(contExpr[0]->value.neg()) leaderIsNeg = true;
			}else if(contExpr[0]->exprType == PROD){
				Expr *pr = contExpr[0];
				for(int32_t i = 0;i < pr->numOfContExpr;i++){
					if(pr->contExpr[i]->exprType == NUM){
						if(pr->contExpr[i]->value.neg()) leaderIsNeg = true;
					}
				}
			}
			
			if(leaderIsNeg) {
				for(int32_t i = 0;i< numOfContExpr;i++){
					contExpr[i] = prodC(contExpr[i],numC(-1L));
					contExpr[i]->simplify();
				}
			}
			
			Expr *factors = new Expr(PROD);
			if(leaderIsNeg){
				simple = false;
				factors->addElement(copy());
				factors->addElement(numC(-1L));
				
				become(factors);
				simplify();
				return;
			}
			//
			Expr *firstEle;
			
			if(contExpr[0]->exprType == PROD){
				firstEle = contExpr[0]->copy();
			}else{
				firstEle = new Expr(PROD);
				firstEle->addElement(contExpr[0]->copy());
			}
			for(int32_t i = 0;i < firstEle->numOfContExpr;i++){
				Expr *ele = firstEle->contExpr[i];
				if(ele->exprType == NUM && ele->value.rep == INT){
					
					Num gc(ele->value);
					for(int32_t j = 1; j<numOfContExpr;j++){
						if(contExpr[j]->exprType == NUM && contExpr[j]->value.rep == INT){
							gcd(&gc,&contExpr[j]->value,&gc);
						}else if(contExpr[j]->exprType == PROD){
							bool foundNum = false;
							Expr *pr = contExpr[j];
							for(int32_t k = 0;k < pr->numOfContExpr;k++){
								if(pr->contExpr[k]->exprType == NUM && pr->contExpr[k]->value.rep == INT){
									gcd(&gc,&pr->contExpr[k]->value,&gc);
									foundNum = true;
								}
							}
							if(!foundNum){
								gc.setValueI(1);
								break;
							}
						}else{
							gc.setValueI(1);
							break;
						}
					}
					if(gc.isLessThanI(0)) gc.flipSign();
					if(!gc.equalsI(1)){
					
						factors->addElement(new Expr(&gc));
						
						for(int32_t j = 0;j < numOfContExpr;j++){
							if(contExpr[j]->exprType == NUM){
								contExpr[j]->value.divN(&gc);
							}else if(contExpr[j]->exprType == PROD){
								Expr *pr = contExpr[j];
								for(int32_t k = 0;k < pr->numOfContExpr;k++){
									if(pr->contExpr[k]->exprType == NUM){
										pr->contExpr[k]->value.divN(&gc);
										break;
									}
								}
								pr->simple = false;
								pr->simplify();
							}
						}
						
					}
					delete ele;
					firstEle->contExpr[i] = nullptr;
				}else{
					Num maxExpo((int64_t)1);
					if(ele->exprType == POW){
						if(ele->getExpo()->exprType == NUM){
							if(ele->getExpo()->value.plain() && !ele->getExpo()->value.neg()){
								maxExpo.setValue(&ele->getExpo()->value);
								ele->becomeInternal(ele->getBase());
							}
						}
					}
					bool allHaveIt = true;
					for(int32_t j = 1; j< numOfContExpr;j++){
						if(contExpr[j]->exprType == PROD){
							Expr *pr = contExpr[j];
							bool found = false;
							for(int32_t k = 0; k < pr->numOfContExpr;k++){
								if(pr->contExpr[k]->equalStruct(ele)){
									found = true;
									maxExpo.setValueI(1L);
									break;
								}else if(pr->contExpr[k]->exprType == POW){
									Expr *pw = pr->contExpr[k];
									if(pw->getBase()->equalStruct(ele) && pw->getExpo()->exprType == NUM){
										if(pw->getExpo()->value.plain() && !pw->getExpo()->value.neg()){
											if(pw->getExpo()->value.rep == FLOAT) maxExpo.convertToFloat();
											if(maxExpo.rep == FLOAT){
												if(maxExpo.valueF > pw->getExpo()->value.valueF){
													maxExpo.setValue(&pw->getExpo()->value);
												}
											}else if(maxExpo.valueI > pw->getExpo()->value.valueI){
												maxExpo.setValue(&pw->getExpo()->value);
											}
											found = true;
											break;
										}
									}
								}
							
							}
							if(found) continue;
						}
						
						if(contExpr[j]->equalStruct(ele)){
							maxExpo.setValueI(1L);
							continue;
						}
						
						if(contExpr[j]->exprType == POW){
							Expr *pw = contExpr[j];
							if(pw->getBase()->equalStruct(ele) && pw->getExpo()->exprType == NUM){
								if(pw->getExpo()->value.plain() && !pw->getExpo()->value.neg()){
									if(pw->getExpo()->value.rep == FLOAT) maxExpo.convertToFloat();
									if(maxExpo.rep == FLOAT){
										if(maxExpo.valueF > pw->getExpo()->value.valueF){
											maxExpo.setValue(&pw->getExpo()->value);
										}
									}else if(maxExpo.valueI > pw->getExpo()->value.valueI){
										maxExpo.setValue(&pw->getExpo()->value);
									}
									continue;
								}
							}
						}
						
						allHaveIt = false;
						break;
						
					}
					if(allHaveIt){
						
						factors->addElement(powC(ele,new Expr(&maxExpo)));
						maxExpo.valueI = -maxExpo.valueI;
						maxExpo.valueF = -maxExpo.valueF;
						
						for(int32_t j = 0; j< numOfContExpr;j++){
							if(contExpr[j]->equalStruct(ele)){
								delete contExpr[j];
								contExpr[j] = numC(1L);
								continue;
							}
							if(contExpr[j]->exprType == POW){
								Expr *pw = contExpr[j];
								if(pw->getBase()->equalStruct(ele) && pw->getExpo()->exprType == NUM){
									if(pw->getExpo()->value.plain() && !pw->getExpo()->value.neg()){
										pw->getExpo()->value.addN(&maxExpo);
										pw->simple = false;
										pw->simplify();
										
										continue;
									}
								}
							}
							if(contExpr[j]->exprType == PROD){
								Expr *pr = contExpr[j];
								for(int32_t k = 0; k < pr->numOfContExpr;k++){
									if(pr->contExpr[k]->equalStruct(ele)){
										pr->removeElement(k);
										pr->simplify();
										
										
										break;
									}else if(pr->contExpr[k]->exprType == POW){
										Expr *pw = pr->contExpr[k];
										if(pw->getBase()->equalStruct(ele) && pw->getExpo()->exprType == NUM){
											if(pw->getExpo()->value.plain() && !pw->getExpo()->value.neg()){
												pw->getExpo()->value.addN(&maxExpo);
												pw->simple = false;
												pw->simplify();
												if(pw->exprType == NUM){
													pr->simple = false;
													pr->simplify();
												}
												break;
											}
										}
									}
								
								}
							}
						}
					}else{
						delete firstEle->contExpr[i];
						firstEle->contExpr[i] = nullptr;
					}
				}
			}
			if(factors->numOfContExpr != 0){
				simple = false;
				factors->addElement(copy());
				become(factors);
				firstEle->nullify();
				delete firstEle;
				simplify();
			}else{
				delete firstEle;
				delete factors;
			}
		}
	}
	
	void Expr::sumSimp(bool addFractions){
		if(exprType == SUM){
			
			for(int32_t i = 0;i < numOfContExpr;i++){//get epsilon
				if(contExpr[i]->direction != MIDDLE){
					direction = contExpr[i]->direction;
					contExpr[i]->direction = MIDDLE;
					break;
				}
			}
			{//infinty + x -> infinity && -infinty + x -> -infinity && infinity-infinity -> indeterminate
				int32_t dir = 0;
				for(int32_t i = 0; i< numOfContExpr;i++){
					if(contExpr[i]->exprType == NUM){
						if(contExpr[i]->value.rep == INF){
							if(dir == 0 || dir == 1){
								dir = 1;
							}else{
								clearElements();
								exprType = INDET;
								return;
							}
						}else if(contExpr[i]->value.rep == NEGINF){
							if(dir == 0 || dir == -1){
								dir = -1;
							}else{
								clearElements();
								exprType = INDET;
								return;
							}
						}
					}
				}
				
				if(dir != 0) {
					clearElements();
					exprType = NUM;
				}
				if(dir == 1){
					value.rep = INF;
					return;
				}else if(dir == -1){
					value.rep = NEGINF;
					return;
				}
				
			}
			
			
			{//sum contains a product with a sum a+b*(c+d) -> a+b*c+b*d
				for(int32_t i = 0;i < numOfContExpr;i++) if(contExpr[i]->exprType == PROD) contExpr[i]->distr();
			}
		
			{//sum contains sum
				for(int32_t i = 0;i<numOfContExpr;i++){
					if(contExpr[i]->exprType == SUM){
					
						for(int32_t j = 0;j<contExpr[i]->numOfContExpr;j++) addElement(contExpr[i]->contExpr[j]);
						contExpr[i]->nullify();
						removeElement(i);
						i--;
						
					}
				}
			}
			
			{//if sum with equation apply to both sides
				Expr *eq = nullptr;
				for(int32_t i = 0;i<numOfContExpr;i++){
					if(contExpr[i]->exprType == EQU){
						eq = contExpr[i];
						contExpr[i] = nullptr;
						removeElement(i);
						break;
					}
				}
				if(eq){
					eq->contExpr[0] = sumC(eq->contExpr[0],copy());
					eq->contExpr[1] = sumC(eq->contExpr[1],copy());
					
					become(eq);
					simple = false;
					simplify();
					return;
				}
			}
			
			{//if list in product apply to all
				Expr *list = nullptr;
				for(int32_t i = 0;i < numOfContExpr;i++){
					if(contExpr[i]->exprType == LIST){
						list = contExpr[i];
						contExpr[i] = nullptr;
						removeElement(i);
						break;
					}
				}
				if(list){
					for(int32_t i = 0;i< list->numOfContExpr;i++) list->contExpr[i] = sumC(copy(),list->contExpr[i]);
					become(list);
					simple = false;
					simplify();
					return;
				}
			}
			
			{//sin(x)^2+cos(x)^2 -> 1 || a*sin(x)^2+a*cos(x)^2 -> a || 1+tan(x)^2
				for(int32_t i = 0;i<numOfContExpr;i++){
					Expr *inner = nullptr;
					byte type = (byte)-1;
					bool plain = false;
					Expr *coef = nullptr;
					
					if(contExpr[i]->exprType == POW){
						Expr *pw = contExpr[i];
						if(pw->getExpo()->exprType == NUM && pw->getExpo()->value.equalsI(2L)){
							if(pw->getBase()->exprType == SIN || pw->getBase()->exprType == COS || pw->getBase()->exprType == TAN){
								inner = pw->getBase()->contExpr[0];
								type = pw->getBase()->exprType;
								plain = true;
							}else continue;
						}
					}else if(contExpr[i]->exprType == PROD){
						
						Expr *pr = contExpr[i];
						bool continueOuter = false;
						for(int32_t j = 0;j < pr->numOfContExpr;j++){
							if(pr->contExpr[j]->exprType == POW){
								Expr *pw = pr->contExpr[j];
								if(pw->getExpo()->exprType == NUM && pw->getExpo()->value.equalsI(2L)){
									if(pw->getBase()->exprType == SIN || pw->getBase()->exprType == COS){
										inner = pw->getBase()->contExpr[0];
										type = pw->getBase()->exprType;
										coef = new Expr(PROD);
										for(int32_t k = 0;k<pr->numOfContExpr;k++){
											if(k == j) continue;
											coef->addElement(pr->contExpr[k]);
										}
										break;
									}else{
										continueOuter = true;
										break;
									}
								}
							}
						}
						if(continueOuter) continue;
						
					}

					if(type != (byte)-1){
						
						if(type == TAN){
							for(int32_t j = 0;j<numOfContExpr;j++){
								if(contExpr[j]->exprType == NUM && contExpr[j]->value.valueI >= 1){
									contExpr[j]->value.valueI-=1;
									inner = inner->copy();
									delete contExpr[i];
									contExpr[i] = powC(cosC(inner),numC(-2L));
									break;
								}
							}
							continue;
						}
						
						bool breakOuter = false;
						for(int32_t j = i+1;j<numOfContExpr;j++){
							
							if(contExpr[j]->exprType == POW){
								Expr *pw = contExpr[j];
								if(pw->getExpo()->exprType == NUM && pw->getExpo()->value.equalsI(2L)){
									if(pw->getBase()->exprType == SIN || pw->getBase()->exprType == COS){
										if(inner->equalStruct(pw->getBase()->contExpr[0])){
											if(!(type == SIN ^ pw->getBase()->exprType == COS)){
												removeElement(j);
												delete contExpr[i];
												contExpr[i] = numC(1L);
												break;
											}
										}
									}else continue;
								}
							}else if(contExpr[j]->exprType == PROD){
								coef->tPrintln();
								Expr *pr = contExpr[j];
								Expr *otherCoef = nullptr;
								for(int32_t k = 0;k < pr->numOfContExpr;k++){
									if(pr->contExpr[k]->exprType == POW){
										Expr *pw = pr->contExpr[k];
										if(pw->getExpo()->exprType == NUM && pw->getExpo()->value.equalsI(2L)){
											if(pw->getBase()->exprType == SIN || pw->getBase()->exprType == COS){
												inner = pw->getBase()->contExpr[0];
												if(inner->equalStruct(pw->getBase()->contExpr[0])){
													otherCoef = new Expr(PROD);
													for(int32_t l = 0;l<pr->numOfContExpr;l++){
														if(l == k) continue;
														otherCoef->addElement(pr->contExpr[l]);
													}
													otherCoef->tPrintln();
													if(otherCoef->equalStruct(coef)){
														if(!(type == SIN ^ pw->getBase()->exprType == COS)){
															delete contExpr[i];
															contExpr[i] = otherCoef->copy();
															contExpr[i]->simplify();
															removeElement(j);
														}
													}
													otherCoef->nullify();
													delete otherCoef;
													breakOuter = true;
													break;
												}
											}
										}
									}
								}
								
							}
							if(breakOuter) break;
						}
					}
					if(coef){
						coef->nullify();
						delete coef;
					}
				}
			}
			
			if(addFractions){//3*ln(4)+4*ln(7) -> 2*ln(392) && // merging logs
				int32_t countOuter = 0;
				for(int32_t i = 0;i<numOfContExpr;i++){
					if(contExpr[i]->exprType == LOG){
						countOuter++;
					}else if(contExpr[i]->exprType == PROD){
						Expr *pr = contExpr[i];
						int32_t count = 0;
						for(int32_t j = 0;j<pr->numOfContExpr;j++){
							if(pr->contExpr[j]->exprType == LOG){
								count++;
							}
						}
						if(count == 1){
							countOuter++;
						}
					}
				}
				if(countOuter > 1){
					Expr *lg = new Expr(LOG,new Expr(PROD));
					for(int32_t i = 0;i < numOfContExpr;i++){
						
						if(contExpr[i]->exprType == LOG){
							
							lg->contExpr[0]->addElement(contExpr[i]->contExpr[0]);
							contExpr[i]->contExpr[0] = nullptr;
							removeElement(i);
							i--;
						}else if(contExpr[i]->exprType == PROD){
							
							Expr *pr = contExpr[i];
							int32_t indexOfLog = -1;
							int32_t count = 0;
							for(int32_t j = 0;j<pr->numOfContExpr;j++){
								if(pr->contExpr[j]->exprType == LOG){
									indexOfLog = j;
									count++;
								}
							}
							if(indexOfLog != -1 && count == 1){
								contExpr[i]=nullptr;
								removeElement(i);
								i--;
								Expr *nwbs = pr->contExpr[indexOfLog]->contExpr[0];
								pr->contExpr[indexOfLog]->contExpr[0] = nullptr;
								pr->removeElement(indexOfLog);
								Expr *nwpw = powC(nwbs,pr);
								lg->contExpr[0]->addElement(nwpw);
								
							}
						}
					
					}
					lg->simplify();
					addElement(lg);
				}
			}
			
			{//x+5*x+2*x -> 8*x
				for(int32_t i = 0;i<numOfContExpr;i++){
					if(contExpr[i]->exprType == NUM && contExpr[i]->value.plain()) continue;
					bool replace = false;
					Num sm((int64_t)1);
					Expr *current = contExpr[i]->copy();
					//remove num from current
					if(current->exprType == PROD){
						for(int32_t j = 0;j<current->numOfContExpr;j++){
							if(current->contExpr[j]->exprType == NUM){
								if(current->contExpr[j]->value.plain()){
									sm.setValue(&current->contExpr[j]->value);
									current->removeElement(j);
									break;
								}
							}
						}
						if(current->numOfContExpr == 1) current->becomeInternal(current->contExpr[0]);
					}
					
					for(int32_t j = i+1;j<numOfContExpr;j++){
						if(contExpr[j]->exprType == NUM && contExpr[j]->value.plain()) continue;
						Num sm2((int64_t)1);
						Expr *comp = contExpr[j]->copy();
						//remove num from comp
						if(comp->exprType == PROD){
							for(int32_t k = 0;k<comp->numOfContExpr;k++){
								if(comp->contExpr[k]->exprType == NUM){
									if(comp->contExpr[k]->value.plain()){
										sm2.setValue(&comp->contExpr[k]->value);
										comp->removeElement(k);
										break;
									}
								}
							}
							if(comp->numOfContExpr == 1) comp->becomeInternal(comp->contExpr[0]);
						}
						
						if(comp->equalStruct(current)){
							sm.addN(&sm2);
							removeElement(j);
							replace = true;
							j--;
						}
						delete comp;
						
					}
					if(replace){
						delete contExpr[i];
						Expr *toBe = prodC(current,new Expr(&sm));
						contExpr[i] = toBe;
						contExpr[i]->simple = false;
						contExpr[i]->simplify();
					}else{
						delete current;
					}
				
				}
			}
			
			
			{//adding numbers
				Num sm((int64_t)0);
				for(int32_t i = 0;i < numOfContExpr;i++){
					if(contExpr[i]->exprType == NUM){
						if(contExpr[i]->value.plain()){
							sm.addN(&(contExpr[i]->value));
							removeElement(i);
							i--;
						}
					
					}
				}
				if(!sm.equalsI(0L)) addElement(new Expr(&sm));
				
			}
			
			if(addFractions){//add fractions
				//check if a fraction exists
				
				bool found = false;
				
				for(int32_t i = 0;i < numOfContExpr;i++){
					if(contExpr[i]->exprType == POW){
						Expr *pw = contExpr[i];
						if(pw->getExpo()->exprType == NUM){
							if(pw->getExpo()->value.neg()){
								found = true;
								break;
							}
						}
					}else if(contExpr[i]->exprType == PROD){
						Expr *pr = contExpr[i];
						for(int32_t j = 0;j < pr->numOfContExpr;j++){
							if(pr->contExpr[j]->exprType == POW){
								Expr *pw = pr->contExpr[j];
								if(pw->getExpo()->exprType == NUM){
									if(pw->getExpo()->value.neg()){
										found = true;
										break;
									}
								}
							
							}
						
						}
					}
					if(found) break;
				}
				
				if(found){
					Expr *numer = numC((int64_t)0);
					Expr *den = new Expr(PROD);
					for(int32_t i = 0;i < numOfContExpr;i++){
						Expr *ele = contExpr[i];
						// a/b + c/d -> (a*d+c*b)/(d*b)
						if(ele->exprType == PROD){
							Expr *newNum = new Expr(PROD);
							Expr *newDen = new Expr(PROD);
							for(int32_t j = 0;j < ele->numOfContExpr;j++){
								bool isden = false;
								if(ele->contExpr[j]->exprType == POW){
									Expr *pw = ele->contExpr[j];
									if(pw->getExpo()->exprType == NUM){
										if(pw->getExpo()->value.neg()){
											isden = true;
											pw->getExpo()->value.absN();
											pw->simple = false;
											pw->simplify();
											newDen->addElement(pw);
										}
									}
								}
								
								if(!isden) newNum->addElement(ele->contExpr[j]);
							
							}
							ele->nullify();
							delete ele;
							
							if(den->equalStruct(newDen)){
								numer = sumC(numer,newNum);
								delete newDen;
							}else{
								newDen->simplify();
								numer = sumC(prodC(numer,newDen),prodC(newNum,den->copy()));
								den->addElement(newDen->copy());
							}
						}else{
						
							if(ele->exprType == POW){
								if(ele->getExpo()->exprType == NUM){
									if(ele->getExpo()->value.neg()){
										ele->getExpo()->value.absN();
										ele->simple = false;
										numer = sumC(prodC(numer,ele),den->copy());
										den->addElement(ele->copy());
										continue;
									}
								}
							}
							
							numer = sumC(numer,prodC(den->copy(),ele));
							
						}
						
					
					}
					nullify();
					for(int32_t i = 0;i<den->numOfContExpr;i++) den->contExpr[i] = invC(den->contExpr[i]);
					Expr *frac = prodC(numer,den);
					frac->simplify();
					become(frac);
					return;
					
				}
				
			}
			
			{//sum is alone
				if(numOfContExpr == 1){
					becomeInternal(contExpr[0]);
					return;
				}
				else if(numOfContExpr == 0){
					exprType = NUM;
					value.setValueI(0L);
					return;
				}
				
			}
			
		}
	}
	
	void Expr::prodSimp(){
		if(exprType == PROD){
			
			if(constant()){
				distr();
				if(exprType != PROD) return;
			}
			
			
			{//product contains a sum
				for(int32_t i = 0;i < numOfContExpr;i++) if(contExpr[i]->exprType == SUM) contExpr[i]->factor();
			}
			
			
			{//product contains product
				for(int32_t i = 0;i<numOfContExpr;i++){
					if(contExpr[i]->exprType == PROD){
					
						for(int32_t j = 0;j<contExpr[i]->numOfContExpr;j++) addElement(contExpr[i]->contExpr[j]);
						contExpr[i]->nullify();
						removeElement(i);
						i--;
						
					}
				}
			}
			
			{//if product with equation apply to both sides
				Expr *eq = nullptr;
				for(int32_t i = 0;i<numOfContExpr;i++){
					if(contExpr[i]->exprType == EQU){
						eq = contExpr[i];
						contExpr[i] = nullptr;
						removeElement(i);
						break;
					}
				}
				if(eq){
					eq->contExpr[0] = prodC(eq->contExpr[0],copy());
					eq->contExpr[1] = prodC(eq->contExpr[1],copy());
					become(eq);
					simple = false;
					simplify();
					return;
				}
			}
			
			{//if list in product apply to all
				Expr *list = nullptr;
				for(int32_t i = 0;i < numOfContExpr;i++){
					if(contExpr[i]->exprType == LIST){
						list = contExpr[i];
						contExpr[i] = nullptr;
						removeElement(i);
						break;
					}
				}
				if(list){
					for(int32_t i = 0;i< list->numOfContExpr;i++) list->contExpr[i] = prodC(copy(),list->contExpr[i]);
					become(list);
					simple = false;
					simplify();
					return;
				}
			}
			
			{//x*0 -> 0
				bool foundInf = false;
				bool foundZero = false;
				for(int32_t i = 0;i< numOfContExpr;i++){
					if(contExpr[i]->exprType == NUM){
						if(contExpr[i]->value.equalsI(0L)){
							foundZero = true;
						}else if(contExpr[i]->value.rep == INF || contExpr[i]->value.rep == NEGINF || contExpr[i]->value.rep == UNDEF){
							foundInf = true;
						}
					}
				}
				if(foundZero && !foundInf){
					clearElements();
					exprType = NUM;
					value.setValueI(0L);
					return;
				}else if(foundZero && foundInf){
					clearElements();
					exprType = INDET;
					return;
				}
			}
			
			{//sin(x)/cos(x) -> tan(x) or sin(x)^2/cos(x) -> tan(x)*sin(x) or sin(x)/cos(x)^2 -> tan(x)/cos(x)
				Expr *listToBeAdded = nullptr;//a temperary list to keep new trig parts in
				for(int32_t i = 0;i<numOfContExpr;i++){
					int64_t sinCount = 0;
					int64_t cosCount = 0;
					if(contExpr[i]->exprType == SIN || contExpr[i]->exprType == COS || contExpr[i]->exprType == TAN || contExpr[i]->exprType == POW){//this section looks for a trig function
						Expr *toComp = contExpr[i];
						Expr *inner = nullptr;
						if(toComp->exprType == SIN){
							sinCount++;
							inner = toComp->contExpr[0];
							
						}else if(toComp->exprType == COS){
							cosCount++;
							inner = toComp->contExpr[0];
							
						}else if(toComp->exprType == TAN){
							sinCount++;
							cosCount--;
							inner = toComp->contExpr[0];
							
						}else if(toComp->exprType == POW){
							if(toComp->getExpo()->exprType == NUM && toComp->getExpo()->value.rep == INT){
							
								if(toComp->getBase()->exprType == SIN){
									
									sinCount+=toComp->getExpo()->value.valueI;
									inner = toComp->getBase()->contExpr[0];
									
									
								}else if(toComp->getBase()->exprType == COS){
									
									cosCount+=toComp->getExpo()->value.valueI;
									inner = toComp->getBase()->contExpr[0];
									
									
								}else if(toComp->getBase()->exprType == TAN){
									
									sinCount+=toComp->getExpo()->value.valueI;
									cosCount-=toComp->getExpo()->value.valueI;
									inner = toComp->getBase()->contExpr[0];
									
									
								}else continue;
							}else continue;
						}else continue;
						
						
						bool moreThanOneTrig = false;
						
						for(int32_t j = i+1;j<numOfContExpr;j++){//looks for another trig to compare too and add to sin and cos count
						
							if(contExpr[j]->exprType == SIN){
								if(contExpr[j]->contExpr[0]->equalStruct(inner)){
									sinCount++;
									removeElement(j);
									moreThanOneTrig = true;
									j--;
								}
								
							}else if(contExpr[j]->exprType == COS){
								if(contExpr[j]->contExpr[0]->equalStruct(inner)){
									cosCount++;
									removeElement(j);
									moreThanOneTrig = true;
									j--;
								}
							}else if(contExpr[j]->exprType == TAN){
								if(contExpr[j]->contExpr[0]->equalStruct(inner)){
									sinCount++;
									cosCount--;
									removeElement(j);
									moreThanOneTrig = true;
									j--;
								}
							}else if(contExpr[j]->exprType == POW){
								Expr *pw = contExpr[j];
								if(pw->getExpo()->exprType == NUM && pw->getExpo()->value.rep == INT){
									if(pw->getBase()->exprType == SIN){
										if(pw->getBase()->contExpr[0]->equalStruct(inner)){
											sinCount+=pw->getExpo()->value.valueI;
											removeElement(j);
											moreThanOneTrig = true;
											j--;
										}
									}else if(pw->getBase()->exprType == COS){
										if(pw->getBase()->contExpr[0]->equalStruct(inner)){
											cosCount+=pw->getExpo()->value.valueI;
											removeElement(j);
											moreThanOneTrig = true;
											j--;
										}
									}else if(pw->getBase()->exprType == TAN){
										if(pw->getBase()->contExpr[0]->equalStruct(inner)){
											sinCount+=pw->getExpo()->value.valueI;
											cosCount-=pw->getExpo()->value.valueI;
											removeElement(j);
											moreThanOneTrig = true;
											j--;
										}
									}
								}
								
							}
							
						}
						
						if(!moreThanOneTrig) continue;
						
						if(!listToBeAdded) listToBeAdded = new Expr(LIST);
						inner = inner->copy();
						removeElement(i);
						i--;
						int64_t tanCount;
						if(sinCount>0 && cosCount<0){//all cases for introducing tan
							if(labs(sinCount)>labs(cosCount)){
								tanCount = -cosCount;
								sinCount += cosCount;
								
								Expr *sinObj = powC(sinC(inner->copy()),numC(sinCount));
								sinObj->simplify();
								listToBeAdded->addElement(sinObj);
							}else{
								tanCount = sinCount;
								cosCount += sinCount;
								
								Expr *cosObj = powC(cosC(inner->copy()),numC(cosCount));
								cosObj->simplify();
								listToBeAdded->addElement(cosObj);
							}
							
							Expr *tanObj = powC(tanC(inner->copy()),numC(tanCount));
							tanObj->simplify();
							listToBeAdded->addElement(tanObj);
							
						}else if(sinCount<0 && cosCount>0){
							if(labs(cosCount)>labs(sinCount)){
								tanCount = -sinCount;
								cosCount += sinCount;
								
								Expr *cosObj = powC(cosC(inner->copy()),numC(cosCount));
								cosObj->simplify();
								listToBeAdded->addElement(cosObj);
							}else{
								tanCount = cosCount;
								sinCount += cosCount;
								
								Expr *sinObj = powC(sinC(inner->copy()),numC(sinCount));
								sinObj->simplify();
								listToBeAdded->addElement(sinObj);
							}
							
							Expr *tanObj = powC(tanC(inner->copy()),numC(-tanCount));
							tanObj->simplify();
							listToBeAdded->addElement(tanObj);
							
						}else{
							Expr *sinObj = powC(sinC(inner->copy()),numC(sinCount));
							sinObj->simplify();
							listToBeAdded->addElement(sinObj);
							Expr *cosObj = powC(cosC(inner->copy()),numC(cosCount));
							cosObj->simplify();
							listToBeAdded->addElement(cosObj);
						}
						delete inner;
					}
					
					
					
				}
				
				if(listToBeAdded){
					for(int32_t i = 0;i < listToBeAdded->numOfContExpr;i++){
						addElement(listToBeAdded->contExpr[i]);
						listToBeAdded->contExpr[i] = nullptr;
					}
					delete listToBeAdded;
				}
			}
			
			
			bool absInProd = false;//do not delete this line
			{//check if there are absolute values becuase next step may need to be done twice example: |x|*|x|*x -> |x|^2*x -> x^3
				for(int32_t i = 0;i<numOfContExpr;i++){
					if(contExpr[i]->exprType == ABS){
						absInProd = true;
						break;
					}else if(contExpr[i]->exprType == POW){
						Expr *pw = contExpr[i];
						if(pw->getBase()->exprType == ABS){
							absInProd = true;
							break;
						}
					}
				}
			}
			
			
			int32_t cycles = 1;
			if(absInProd) cycles++;
			
			for(int32_t c = 0;c<cycles;c++){//x*x^2*x^a*5*y -> x^(3+a)*5*y
				for(int32_t i = 0;i<numOfContExpr;i++){
					if(contExpr[i]->exprType == NUM && contExpr[i]->value.plain()) continue;
					if(contExpr[i]->inverseInt()) continue;
					
					bool replace = false;
					Expr *current;
					
					if(contExpr[i]->exprType == POW) current = contExpr[i]->copy();
					else{
						current = new Expr(POW);
						current->addElement(contExpr[i]->copy());
						current->addElement(numC(1L));
					}
					Expr *expoSum = new Expr(SUM);
					expoSum->addElement(current->getExpo());
					
					
					for(int32_t j = i+1;j < numOfContExpr;j++){
						if(contExpr[j]->exprType == NUM && contExpr[j]->value.plain()) continue;
						if(contExpr[j]->inverseInt()) continue;
						Expr *comp = contExpr[j];
						if(comp->exprType == POW){
							if(comp->getBase()->equalStruct(current->getBase())){
								replace= true;
								expoSum->addElement(comp->getExpo());
								comp->contExpr[1] = nullptr;
								removeElement(j);
								j--;
							}
						}else if(comp->equalStruct(current->getBase())){
							replace= true;
							expoSum->addElement(numC(1L));
							removeElement(j);
							j--;
						}
						
						
					}
					if(replace){
						delete contExpr[i];
						current->setExpo(expoSum);
						expoSum->simplify();
						contExpr[i] = current;
						contExpr[i]->simple = false;
						contExpr[i]->powSimp();
					}else{
						delete current;
						expoSum->nullify();
						delete expoSum;
					}
					
				}
			}
			
			{//sqrt(2)*sqrt(3) -> sqrt(6)
				for(int32_t i = 0;i < numOfContExpr;i++){
					if(contExpr[i]->exprType == POW){
						Expr *current = contExpr[i];
						if(!(current->getBase()->exprType == NUM && current->value.plain())) continue;
						for(int32_t j = i+1;j < numOfContExpr;j++){
							if(contExpr[j]->exprType == POW){
								Expr *other = contExpr[j];
								if(!(other->getBase()->exprType == NUM && other->value.plain())) continue;
								if(current->getExpo()->equalStruct(other->getExpo())){
									current->getBase()->value.multN(&other->getBase()->value);
									removeElement(j);
									j--;
								}
							}
						}
					}
				}
			}
			
			{//multiplying numbers and inverse numbers
				Num pr((int64_t)1);
				Num ipr((int64_t)1);
				
				bool neg = false;
				bool inf = false;
				for(int32_t i = 0;i < numOfContExpr;i++){
					if(contExpr[i]->exprType == NUM){
						if(contExpr[i]->value.plain()){
							pr.multN(&(contExpr[i]->value));
							removeElement(i);
							i--;
						}else if(contExpr[i]->value.rep == INF){
							removeElement(i);
							i--;
							inf = true;
						}else if(contExpr[i]->value.rep == NEGINF){
							removeElement(i);
							i--;
							neg = !neg;
							inf = true;
						}
					
					}else if(contExpr[i]->exprType == POW){
						if(contExpr[i]->getBase()->exprType == NUM && contExpr[i]->expoIsMinusOne()){
							if(contExpr[i]->getBase()->value.plain()){
								ipr.multN(&(contExpr[i]->getBase()->value));
								removeElement(i);
								i--;
							}
						}
					}
				}
				
				neg = (pr.neg() != ipr.neg()) != neg;
				
				if(inf){
					
					Expr *posInf = new Expr(NUM);
					if(!neg) posInf->value.rep = INF;
					else posInf->value.rep = NEGINF;
					addElement(posInf);
				}else{
					Num g;
					gcd(&pr,&ipr,&g);
					
					pr.divN(&g);
					ipr.divN(&g);
					pr.absN();
					ipr.absN();
					
					
					if(!pr.equalsI(1L)){
						if(neg){
							pr.flipSign();
							neg = false;
						}
						addElement(new Expr(&pr));
					}
					if(!ipr.equalsI(1L)){
						if(neg){
							ipr.flipSign();
							neg = false;
						}
						addElement(invC(new Expr(&ipr)));
					}
					if(neg) addElement(numC(-1));
				}
				
			}
			
			{//infinty*(log(c)) -> infinity
				bool becomeInf = true;
				int32_t hasInf = 0;
				for(int32_t i = 0;i < numOfContExpr;i++){
					if(contExpr[i]->exprType == NUM && contExpr[i]->value.rep == INF){
						hasInf = 1;
						continue;
					}else if(contExpr[i]->exprType == NUM && contExpr[i]->value.rep == NEGINF){
						hasInf = -1;
						continue;
					}else if(contExpr[i]->exprType == LOG){
						continue;
					}else if(contExpr[i]->exprType == NUM && !contExpr[i]->value.neg()){
						continue;
					}else if(contExpr[i]->exprType == ABS){
						continue;
					}else if(contExpr[i]->exprType == POW){
						Expr *pw = contExpr[i];
						if(pw->getBase()->exprType == NUM && !pw->getBase()->value.neg() && pw->getExpo()->constant()){
							continue;
						}
					}
					becomeInf = false;
				}
				if(hasInf != 0 && becomeInf){
					clearElements();
					exprType = NUM;
					if(hasInf == 1) value.rep = INF;
					if(hasInf == -1) value.rep = NEGINF;
					return;
				}
			}
			
			{//product is alone
				if(numOfContExpr == 1) becomeInternal(contExpr[0]);
				else if(numOfContExpr == 0){
					exprType = NUM;
					value.setValueI(1L);
				}
				
			}
		}
	}
	void Expr::logSimp(){
		if(exprType == LOG){
			
			direction = contExpr[0]->direction;
			contExpr[0]->direction = MIDDLE;
			
			contExpr[0]->factor();
			
			if(contExpr[0]->exprType == EQU){//log of equation , log of both sides
				becomeInternal(contExpr[0]);
				contExpr[0] = logC(contExpr[0]);
				contExpr[1] = logC(contExpr[1]);
				simple = false;
				simplify();
				return;
			}
			
			if(contExpr[0]->exprType == LIST){//log of list turns into list
				Expr *list = contExpr[0];
				for(int32_t i = 0;i<list->numOfContExpr;i++){
					list->contExpr[i] = logC(list->contExpr[i]);
				}
				
				becomeInternal(list);
				simple = false;
				simplify();
				return;
			}
			if(contExpr[0]->exprType == NUM){//log(1) -> 0 && log(e) -> 1 && log(0) -> -infinity && log(infinity) -> infinity  && log(-num) -> pi*i + log(num)
				Expr *cont = contExpr[0];
				if(cont->value.rep == EV){
					clearElements();
					exprType = NUM;
					value.setValueI(1L);
					return;
				}else if(cont->value.equalsI(1L)){
					clearElements();
					exprType = NUM;
					value.setValueI(0L);
					return;
				}else if(cont->value.equalsI(0L)){
					clearElements();
					exprType = NUM;
					value.rep = NEGINF;
					return;
				}else if(cont->value.rep == INF){
					clearElements();
					exprType = NUM;
					value.rep = INF;
					return;
				}else if(cont->value.neg()){
					cont->value.absN();
					Expr *repl = sumC(prodC(piC(),iC()),copy());
					become(repl);
					simplify();
					return;
				}
			
			}
			
			{//log(a+b*i) -> (ln(a^2+b^2)+2*atan(b/a)*i)/2
				Expr *cn = contExpr[0]->complexExtract();
				
				if(!(cn->contExpr[1]->exprType == NUM && cn->contExpr[1]->value.equalsI(0L))){
				   Expr *repl = logC(sumC(powC(cn->contExpr[0],numC(2L)),powC(cn->contExpr[1],numC(2L))));
				   Expr *pr = prodC(iC(),atanC(prodC(cn->contExpr[1]->copy(),invC(cn->contExpr[0]->copy()))));
				   pr->addElement(numC(2L));
				   repl = prodC(sumC(pr,repl),invC(numC(2L)));
				   
				   cn->nullify();
				   delete cn;
				   become(repl);
				   simplify();
				   return;
				}else delete cn;
			}
			
			if(contExpr[0]->exprType == PROD){//remove denominator in exponents ex: ln(4^(x/y)*z) -> ln(4^x*z^y)/y
				Expr *prodOfDen = new Expr(PROD);
				Expr *pr = contExpr[0];
				for(int32_t i = 0;i<pr->numOfContExpr;i++){
					if(pr->contExpr[i]->exprType == POW){
						if(pr->contExpr[i]->getExpo()->exprType == PROD){
							Expr *innerProd = pr->contExpr[i]->getExpo();
							for(int32_t j = 0;j<innerProd->numOfContExpr;j++){
								if(innerProd->contExpr[j]->exprType == POW){
									Expr *pw = innerProd->contExpr[j];
									if(pw->getExpo()->exprType == NUM && pw->getExpo()->value.rep == INT && pw->getExpo()->value.neg()){
										prodOfDen->addElement(powC(pw->getBase()->copy(),numC(-pw->getExpo()->value.valueI)));
									}
								}
							}
						}else if(pr->contExpr[i]->getExpo()->exprType == POW){
							Expr *pw = pr->contExpr[i]->getExpo();
							if(pw->getExpo()->exprType == NUM && pw->getExpo()->value.rep == INT && pw->getExpo()->value.neg()){
								
								prodOfDen->addElement(powC(pw->getBase()->copy(),numC(-pw->getExpo()->value.valueI)));
							}
						}
					}
				}
				
				if(prodOfDen->numOfContExpr > 0){
					contExpr[0] = powC(contExpr[0],prodOfDen);
					
					Expr *repl = prodC(copy(),invC(prodOfDen->copy()));
					contExpr[0]->powSimp();
					
					become(repl);
					simplify();
					return;
				}else{
					delete prodOfDen;
				}
			}
			
			if(contExpr[0]->exprType == PROD){//need factoring common exponents ex: ln(a^x*b^x) -> x*ln(a*b)
				Expr *pr = contExpr[0];
				for(int32_t i = 0;i< pr->numOfContExpr;i++) if(pr->contExpr[i]->exprType == POW) pr->contExpr[i]->getExpo()->factor();
				if(pr->contExpr[0]->exprType == POW){
					Expr *list = pr->contExpr[0]->getExpo()->copy();
					if(list->exprType != PROD) list = new Expr(PROD,list);//list for first one
					for(int32_t i = 0;i<list->numOfContExpr;i++){
						bool allHaveIt = true;
						
						for(int32_t j = 1;j<pr->numOfContExpr;j++){
							if(pr->contExpr[j]->exprType == NUM && pr->contExpr[j]->value.rep == INT){//4->2^2
								int64_t b,e;
								perfectPower(pr->contExpr[j]->value.valueI,&b,&e);
								if(e!=1){
									delete pr->contExpr[j];
									pr->contExpr[j] = powC(numC(b),numC(e));
								}
							}
							if(pr->contExpr[j]->exprType == POW){
								if(pr->contExpr[j]->getExpo()->equalStruct(list->contExpr[i])){
									continue;
								}else if(pr->contExpr[j]->getExpo()->exprType == PROD){
									Expr *pwpr = pr->contExpr[j]->getExpo();
									for(int32_t k = 0;k<pwpr->numOfContExpr;k++){
										if(pwpr->contExpr[k]->equalStruct(list->contExpr[i])) continue;
									}
								}
								allHaveIt = false;
							}else{
								allHaveIt = false;
							}
						}
						if(!allHaveIt){
							list->removeElement(i);
							i--;
						}
						
					}
					
					if(list->numOfContExpr > 0){
						contExpr[0] = powC(contExpr[0],invC(list));
						Expr *repl = prodC(list->copy(),copy());
						become(repl);
						simplify();
						return;
					}else{
						for(int32_t i = 0;i<pr->numOfContExpr;i++){
							if(pr->contExpr[i]->constant() && pr->contExpr[i]->exprType == POW){
								pr->contExpr[i]->simplify();
							}
						}
						delete list;
					}
				}
			}
			
			if(contExpr[0]->exprType == POW){//log(e^x) -> x && log(a^b) -> b*log(a)
				Expr *repl = prodC(contExpr[0]->getExpo()->copy(),logC(powC(contExpr[0]->copy(),invC(contExpr[0]->getExpo()->copy()))));
				
				become(repl);
				simplify();
				return;
			}
			
			
			if(contExpr[0]->exprType == NUM){//log(8) -> 3*log(2)
				if(contExpr[0]->value.rep == INT){
					int64_t b,e;
					perfectPower(contExpr[0]->value.valueI,&b,&e);
					if(e != 1){
						exprType = PROD;
						contExpr[0]->value.setValueI(e);
						addElement(logC(numC(b)));
						return;
					}
				}else if(contExpr[0]->value.rep == FLOAT){
					double n = contExpr[0]->value.valueF;
					clearElements();
					exprType = NUM;
					value.setValueF(log(n));
					return;
				}
			
			}
			
		}
	}
	void Expr::solverSimp(){
		if(exprType == SOLVE){
			
			if(contExpr[0]->exprType == LIST){
				Expr *list = contExpr[0];
				for(int32_t i = 0;i< list->numOfContExpr;i++){
					list->contExpr[i] = new Expr(SOLVE,list->contExpr[i],contExpr[1]->copy());
				}
				becomeInternal(list);
				simple = false;
				simplify();
				return;
			}else if(contExpr[1]->exprType == LIST){
				Expr *list = contExpr[1];
				for(int32_t i = 0;i< list->numOfContExpr;i++){
					list->contExpr[i] = new Expr(SOLVE,contExpr[0]->copy(),list->contExpr[i]);
				}
				becomeInternal(list);
				simple = false;
				simplify();
				return;
			}
			
			if(contExpr[0]->exprType != EQU){
				contExpr[0] = equC(contExpr[0],numC((int64_t)0));
			}
			
			Expr *eq = contExpr[0];
			Expr *v = contExpr[1];
			
			if(eq->contExpr[0]->exprType == NUM && eq->contExpr[0]->value.equalsI(0L)){
				Expr *temp = eq->contExpr[0];
				eq->contExpr[0] = eq->contExpr[1];
				eq->contExpr[1] = temp;
			}
			
			if( eq->contExpr[1]->contains(v) ){
				eq->contExpr[0] = sumC(eq->contExpr[0],prodC(eq->contExpr[1],numC(-1L)));
				eq->contExpr[1] = numC((int64_t)0);
				eq->contExpr[0]->simplify();
			}
			
			while(true){
				
				bool changed = false;
				eq->contExpr[0]->factor();
				
				if(eq->contExpr[0]->exprType == SUM){
					Expr *sm = eq->contExpr[0];
					Expr *nvarSm = new Expr(SUM);
					for(int32_t i = 0;i < sm->numOfContExpr; i++){
						if(!sm->contExpr[i]->contains(v)){
							nvarSm->addElement(sm->contExpr[i]);
							sm->contExpr[i] = nullptr;
							sm->removeElement(i);
							i--;
							changed = true;
						}
					}
					if(changed){
						eq->contExpr[1] = sumC(prodC(nvarSm,numC(-1L)),eq->contExpr[1]);
						eq->simple = false;
						eq->simplify();
					}else{
						delete nvarSm;
					}
				}else if(eq->contExpr[0]->exprType == PROD){//isolate based on product
					Expr *pr = eq->contExpr[0];
					
					
					{
						Expr *nvarPr = new Expr(PROD);
						for(int32_t i = 0;i < pr->numOfContExpr; i++){
							if(!pr->contExpr[i]->contains(v)){
								nvarPr->addElement(pr->contExpr[i]);
								pr->contExpr[i] = nullptr;
								pr->removeElement(i);
								i--;
								changed = true;
							}
						}
						if(changed){
							eq->contExpr[1] = prodC(invC(nvarPr),eq->contExpr[1]);
							eq->simple = false;
						}else{
							delete nvarPr;
						}
					}
					if(!changed && eq->contExpr[1]->exprType == NUM && eq->contExpr[1]->value.equalsI(0L)){
						
						eq->contExpr[0]->exprType = LIST;
						Expr *list = eq->contExpr[0];
						for(int32_t i = 0;i < list->numOfContExpr;i++){//remove variable-less parts
							if(!list->contExpr[i]->contains(v)){
								list->removeElement(i);
								i--;
							}
						}
						eq->simple = false;
						simplify();
						return;
					}
					//quadratic
					if(!changed){//f(x)*(a*f(x)+b)=c -> a*((f(x)+b/2/a)^2-(b/2/a)^2)=c  (not useing polyExtract because its less robust)
						if(pr->numOfContExpr == 2){
							//find sum && other
							Expr *otherPart = nullptr,*sumPart = nullptr;
							for(int32_t k = 0;k<2;k++){
								if(pr->contExpr[k]->exprType == SUM) sumPart = pr->contExpr[k];
								else otherPart = pr->contExpr[k];
							}
							if(sumPart && otherPart){
								//find f(x) in sum
								int32_t indexOfFX = -1;
								for(int32_t k = 0;k<sumPart->numOfContExpr;k++){
									if(sumPart->contExpr[k]->contains(v)){
										indexOfFX = k;
										break;
									}
								}
								Expr *a = nullptr;
								if(sumPart->contExpr[indexOfFX]->exprType == PROD){
									Expr *FXpr = sumPart->contExpr[indexOfFX];
									
									for(int32_t k = 0;k<FXpr->numOfContExpr;k++){
										if(FXpr->contExpr[k]->equalStruct(otherPart)){
											a = new Expr(PROD);
											for(int32_t j = 0;j<FXpr->numOfContExpr;j++){
												if(j!=k){
													a->addElement(FXpr->contExpr[j]->copy());
												}
											}
											break;
										}
									}
								}else if(sumPart->contExpr[indexOfFX]->equalStruct(otherPart)){
									a = numC(1L);
								}
								
								if(a){
									sumPart->removeElement(indexOfFX);
									
									Expr *bO2a = new Expr(PROD);
									bO2a->addElement(sumPart);
									bO2a->addElement(invC(numC(2L)));
									bO2a->addElement(invC(a));
									Expr *nbO2aSq = new Expr(PROD);
									nbO2aSq->addElement(powC(sumPart->copy(),numC(2L)));
									nbO2aSq->addElement(invC(numC(-4L)));
									nbO2aSq->addElement(powC(a->copy(),numC(-2L)));
									
									
									Expr *nwSm = prodC(sumC(powC(sumC(otherPart,bO2a),numC(2L)), nbO2aSq  ),a->copy());
									nwSm->simplify();
									pr->nullify();
									delete pr;
									eq->contExpr[0] = nwSm;
									changed = true;
								}
								
								
							}
							
						}
					}
					
					if(changed) eq->simplify();
				}else if(eq->contExpr[0]->exprType == POW){
					Expr *pw = eq->contExpr[0];
					if(pw->getExpo()->contains(v) && !pw->getBase()->contains(v)){
						Expr *bs = pw->getBase();
						pw->setBase(nullptr);
						pw->becomeInternal(pw->getExpo());
						changed = true;
						eq->contExpr[1] = prodC(logC(eq->contExpr[1]),invC(logC(bs)));
						eq->contExpr[1]->simplify();
					}else if(!pw->getExpo()->contains(v) && pw->getBase()->contains(v)){
						Expr *ex = pw->getExpo();
						bool plusOrMinus = false;
						if(ex->exprType == NUM && ex->value.rep == INT){
							if(ex->value.valueI %2L == 0) plusOrMinus = true;
						}else if(ex->exprType == PROD){
							for(int32_t i = 0;i < ex->numOfContExpr;i++){
								if(ex->contExpr[i]->exprType == NUM && ex->contExpr[i]->value.rep == INT){
									if(ex->contExpr[i]->value.valueI %2L == 0){
										plusOrMinus = true;
										break;
									}
								}
							}
						}
						pw->setExpo(nullptr);
						pw->becomeInternal(pw->getBase());
						changed = true;
						if(!plusOrMinus){
							eq->contExpr[1] = powC(eq->contExpr[1],invC(ex));
							eq->contExpr[1]->simplify();
						}else{
							Expr *list = new Expr(LIST);
							list->addElement(powC(eq->contExpr[1],invC(ex)));
							list->addElement(prodC(powC(eq->contExpr[1]->copy(),invC(ex->copy())),numC(-1L)));
							eq->contExpr[1] = list;
							eq->simple = false;
							simplify();
							return;
						}
					}
				}else if(eq->contExpr[0]->exprType == LOG){
					eq->contExpr[0]->becomeInternal(eq->contExpr[0]->contExpr[0]);
					eq->contExpr[1] = powC(eC(),eq->contExpr[1]);
					eq->contExpr[1]->simplify();
					changed = true;
				}else if(eq->contExpr[0]->exprType == SIN){
					eq->contExpr[0]->becomeInternal(eq->contExpr[0]->contExpr[0]);
					eq->contExpr[1] = asinC(eq->contExpr[1]);
					eq->contExpr[1]->simplify();
					changed = true;
				}else if(eq->contExpr[0]->exprType == COS){
					eq->contExpr[0]->becomeInternal(eq->contExpr[0]->contExpr[0]);
					eq->contExpr[1] = acosC(eq->contExpr[1]);
					eq->contExpr[1]->simplify();
					changed = true;
				}else if(eq->contExpr[0]->exprType == TAN){
					eq->contExpr[0]->becomeInternal(eq->contExpr[0]->contExpr[0]);
					eq->contExpr[1] = atanC(eq->contExpr[1]);
					eq->contExpr[1]->simplify();
					changed = true;
				}else if(eq->contExpr[0]->exprType == ABS){
					changed = true;
					Expr *list = new Expr(LIST);
					list->addElement(eq->contExpr[1]);
					list->addElement(prodC(eq->contExpr[1]->copy(),numC(-1L)));
					eq->contExpr[1] = list;
					eq->contExpr[0]->becomeInternal(eq->contExpr[0]->contExpr[0]);
					eq->simple = false;
					simplify();
					return;
				}
				if(eq->contExpr[0]->equalStruct(v)){
					becomeInternal(eq);
					return;
				}
				if(!changed){
					if(ERRORS) printf("-unable to isolate variable\n");
					break;
				}
			}
		}
	}
	
	Expr *Expr::complexExtract(){//input is asumed to be simplified , a+b*i -> [a,b]
		Expr *out = new Expr(LIST);
		if(isI()){
			out->addElement(numC((int64_t)0));
			out->addElement(numC(1L));
			return out;
		}else if(exprType == PROD){
			int32_t indexOfI = -1;
			for(int32_t i = 0;i<numOfContExpr;i++){
				if(contExpr[i]->isI()){
					indexOfI = i;
					break;
				}
			}
			if(indexOfI != -1){
				Expr *cpy = copy();
				cpy->removeElement(indexOfI);
				out->addElement(numC((int64_t)0));
				cpy->simplify();
				out->addElement(cpy);
				return out;
			}
		}else if(exprType == SUM){
			out->addElement(new Expr(SUM));
			out->addElement(new Expr(SUM));
			for(int32_t i = 0;i < numOfContExpr;i++){
				if(contExpr[i]->isI()){
					out->contExpr[1]->addElement(numC(1L));
				}else if(contExpr[i]->exprType == PROD){
					Expr *pr = contExpr[i];
					int32_t indexOfI = -1;
					for(int32_t j = 0;j<pr->numOfContExpr;j++){
						if(pr->contExpr[j]->isI()){
							indexOfI = j;
							break;
						}
					}
					
					if(indexOfI != -1){
						Expr *cpy = pr->copy();
						cpy->removeElement(indexOfI);
						out->contExpr[1]->addElement(cpy);
					}else{
						out->contExpr[0]->addElement(pr->copy());
					}
					
				}else{
					out->contExpr[0]->addElement(contExpr[i]->copy());
				}
			}
			for(int32_t i = 0;i<2;i++) out->contExpr[i]->simplify();
			return out;
		}
		
		out->addElement(copy());
		out->addElement(numC((int64_t)0));
		return out;
	}
	
	void Expr::absSimp(){
		if(exprType == ABS){
			contExpr[0]->factor();
			if(contExpr[0]->exprType == LIST){
				Expr *list = contExpr[0];
				for(int32_t i = 0;i<list->numOfContExpr;i++){
					list->contExpr[i] = absC(list->contExpr[i]);
				}
				
				becomeInternal(list);
				simple = false;
				simplify();
				return;
			}else if(contExpr[0]->exprType == ABS){
				becomeInternal(contExpr[0]);
				return;
			}else if(contExpr[0]->exprType == NUM){
				contExpr[0]->value.absN();
				becomeInternal(contExpr[0]);
				return;
			}else if(contExpr[0]->exprType == POW){//|a^b| -> |a|^b
				Expr *pw = contExpr[0];
				pw->simple = false;
				pw->setBase(absC(pw->getBase()));
				becomeInternal(pw);
				simplify();
				return;
			}else if(contExpr[0]->exprType == PROD){//|a*b| -> |a|*|b|
				Expr *pr = contExpr[0];
				for(int32_t i = 0;i < pr->numOfContExpr;i++){
					pr->contExpr[i] = absC(pr->contExpr[i]);
				}
				pr->simple = false;
				becomeInternal(pr);
				simplify();
				return;
			}
			
			{//complex number |a+b*i| -> sqrt(a^2+b^2)
				Expr *cn = contExpr[0]->complexExtract();
				if(!(cn->contExpr[1]->exprType == NUM && cn->contExpr[1]->value.equalsI(0L))){
					Expr *repl = powC(sumC(powC(cn->contExpr[0],numC(2L)),powC(cn->contExpr[1],numC(2L))),invC(numC(2L)));
					cn->nullify();
					delete cn;
					become(repl);
					simplify();
					return;
				}else delete cn;
			}
			
			contExpr[0]->simplify();
			if(contExpr[0]->exprType == POW){// |x^(2*a)| -> x^(2*a) basicly anything even
				Expr *pw = contExpr[0];
				
				if(pw->getExpo()->exprType == PROD){
					Expr *pr = pw->getExpo();
					for(int32_t i = 0;i< pr->numOfContExpr;i++){
						if(pr->contExpr[i]->exprType == NUM && pr->contExpr[i]->value.rep == INT){
							if(pr->contExpr[i]->value.valueI %2L == 0){
								becomeInternal(contExpr[0]);
								return;
							}
						}
					}
				}else if(pw->getExpo()->exprType == NUM && pw->getExpo()->value.rep == INT){
					if(pw->getExpo()->value.valueI %2L == 0){
						becomeInternal(contExpr[0]);
						return;
					}
				}
				
			}
		}
	}
	
	void Expr::listSimp(){
		if(exprType == LIST){
			{//merge
				for(int32_t i = 0;i<numOfContExpr;i++){
					if(contExpr[i]->exprType == LIST){
					
						for(int32_t j = 0;j<contExpr[i]->numOfContExpr;j++) addElement(contExpr[i]->contExpr[j]);
						contExpr[i]->nullify();
						removeElement(i);
						i--;
						
					}
				}
			
			}
			
			{//factor for consistency
				for(int32_t i = 0;i<numOfContExpr;i++) contExpr[i]->factor();
			}
			
			{//remove duplicates
				for(int32_t i = 0; i < numOfContExpr;i++){
					for(int32_t j = i+1;j < numOfContExpr;j++){
						if(contExpr[i]->equalStruct(contExpr[j])){
							removeElement(j);
							j--;
						}
					}
				}
			}
			
			if(numOfContExpr == 1){//alone
				becomeInternal(contExpr[0]);
				return;
			}
		
		}
	}
	void Expr::equSimp(){
		if(exprType == EQU){
			if(contExpr[0]->exprType == LIST){//[x,y] = z -> [x=z,y=z]
				Expr *list = contExpr[0];
				for(int32_t i = 0;i < list->numOfContExpr;i++){
					list->contExpr[i] = equC(list->contExpr[i],contExpr[1]->copy());
				}
				becomeInternal(list);
				return;
			}
			if(contExpr[1]->exprType == LIST){//z = [x,y] -> [z=x,z=y]
				Expr *list = contExpr[1];
				for(int32_t i = 0;i < list->numOfContExpr;i++){
					list->contExpr[i] = equC(contExpr[0]->copy(),list->contExpr[i]);
				}
				becomeInternal(list);
				return;
			}
			while(true){
				bool changed = false;
				if(contExpr[0]->exprType == contExpr[1]->exprType && (contExpr[0]->exprType == PROD || contExpr[0]->exprType == SUM)){//a*b=a*c -> b=c && a+b=a+c -> b=c
					Expr *left = contExpr[0];
					Expr *right = contExpr[1];
					
					for(int32_t i = 0; i< left->numOfContExpr;i++){
					
						for(int32_t j = 0; j < right->numOfContExpr;j++){
							if(left->contExpr[i]->equalStruct(right->contExpr[j])){
								left->removeElement(i);
								right->removeElement(j);
								i--;
								changed = true;
								break;
							}
						
						}
						
					}
					contExpr[0]->simplify();
					contExpr[1]->simplify();
					
				}else if(contExpr[0]->exprType == POW && contExpr[1]->exprType == POW){//a^2=b^2 -> |a| = |b| && 2^x=2^y -> x=y
					if(contExpr[0]->getExpo()->equalStruct(contExpr[1]->getExpo())){
						contExpr[0] = powC(contExpr[0],invC(contExpr[0]->getExpo()->copy()));
						contExpr[1] = powC(contExpr[1],invC(contExpr[1]->getExpo()->copy()));
						contExpr[0]->simplify();
						contExpr[1]->simplify();
						changed = true;
					}else if(contExpr[0]->getBase()->equalStruct(contExpr[1]->getBase())){
						contExpr[0]->becomeInternal(contExpr[0]->getExpo());
						contExpr[1]->becomeInternal(contExpr[1]->getExpo());
						changed = true;
					}
				}
				else if(contExpr[0]->exprType == LOG && contExpr[1]->exprType == LOG){// ln(a)=ln(b) -> a = b
					contExpr[0]->becomeInternal(contExpr[0]->contExpr[0]);
					contExpr[1]->becomeInternal(contExpr[1]->contExpr[0]);
					changed = true;
				}
				if(!changed) break;
			}
			
			
		}
	}
	void Expr::sinSimp(){
		if(exprType == SIN){
			if(contExpr[0]->exprType == ASIN){
				becomeInternal(contExpr[0]->contExpr[0]);
				return;
			}
			contExpr[0]->factor();
			if(contExpr[0]->exprSign() == NEGATIVE){//sin(-x)-> -sin(x)
				contExpr[0] = prodC(numC(-1L),contExpr[0]);
				contExpr[0]->simplify();
				
				Expr *repl = prodC(numC(-1L),copy());
				become(repl);
				simplify();
				return;
			}
			
			{//sin(complex)
				Expr *cn = contExpr[0]->complexExtract();
				
				if(!(cn->contExpr[1]->exprType == NUM && cn->contExpr[1]->value.equalsI(0L))){
				   Expr *repl = prodC(cosC(cn->contExpr[0]),sumC(powC(eC(),cn->contExpr[1]),prodC(numC(-1L),powC( eC(),prodC(numC(-1L),cn->contExpr[1]->copy()) ))));
				   repl->addElement(iC());
				   repl = sumC(repl,prodC(sinC(cn->contExpr[0]->copy()), sumC( powC(eC(), cn->contExpr[1]->copy() ),powC(eC(), prodC(numC(-1L),cn->contExpr[1]->copy()) ) ) ));
				   repl = prodC(repl,invC(numC(2L)));
				   
				   cn->nullify();
				   delete cn;
				   become(repl);
				   simplify();
				   return;
				}else delete cn;
			}
			
			if(contExpr[0]->exprType == PROD && constant()){//unit circle
				Expr *pr = contExpr[0];
				//
				bool foundPi = false;
				int64_t num = 1,den = 1;
				bool foundOther = false;
				for(int32_t i = 0;i<pr->numOfContExpr;i++){
					bool integer = pr->contExpr[i]->exprType == NUM && pr->contExpr[i]->value.rep == INT;
					bool inverseInt = pr->contExpr[i]->inverseInt();
					bool isPi = pr->contExpr[i]->exprType == NUM && pr->contExpr[i]->value.rep == PIV;
					if(integer){
						num = pr->contExpr[i]->value.valueI;
					}else if(inverseInt){
						den = pr->contExpr[i]->getBase()->value.valueI;
					}else if(isPi){
						foundPi = true;
					}else{
						foundOther = true;
					}
					
				}
				if(foundPi && !foundOther){
					bool neg = false;
					if(den < 0){
						den = -den;
						neg = !neg;
					}
					if(num < 0){
						num = -num;
						neg = !neg;
					}
					num = num%(den*2);
					if(num > den){
						num = num%den;
						neg = !neg;
					}
					if(num>den/2){
						num = den-num;
					}
					if(den == 3){//sin(pi/3) -> sqrt(3)/2
						
						if(neg) become(prodC(powC(numC(3L),invC(numC(2L))),invC(numC(-2L))));
						else become(prodC(powC(numC(3L),invC(numC(2L))),invC(numC(2L))));
						return;
					}else if(den == 4){//sin(pi/4) -> sqrt(2)/2
						if(neg) become(prodC(powC(numC(2L),invC(numC(2L))),invC(numC(-2L))));
						else become(prodC(powC(numC(2L),invC(numC(2L))),invC(numC(2L))));
						return;
					}
					else if(den == 6){//sin(pi/6) -> 1/2
						
						if(neg) become(invC(numC(-2L)));
						else become(invC(numC(2L)));
						return;
					}else if(den == 2){
						clearElements();
						exprType = NUM;
						if(neg) value.setValueI(-1L);
						else value.setValueI(1L);
						return;
					}else{
						contExpr[0]->clearElements();
						contExpr[0]->addElement(numC(num));
						contExpr[0]->addElement(piC());
						contExpr[0]->addElement(invC(numC(den)));
						contExpr[0]->simplify();
						if(neg){
							Expr *repl = prodC(numC(-1L),copy());
							become(repl);
							return;
						}
					}
					
				}
			}
			if(contExpr[0]->exprType == NUM && (contExpr[0]->value.equalsI(0L) || contExpr[0]->value.rep == PIV)){//sin(0) -> 0
				clearElements();
				exprType = NUM;
				value.setValueI(0L);
				return;
			}
			
		}
	}
	void Expr::cosSimp(){
		if(exprType == COS){
			if(contExpr[0]->exprType == ACOS){
				becomeInternal(contExpr[0]->contExpr[0]);
				return;
			}
			if(contExpr[0]->exprType == ABS){//cos(|x|) -> cos(x)
				contExpr[0]->becomeInternal(contExpr[0]->contExpr[0]);
			}
			
			contExpr[0]->factor();
			
			if(contExpr[0]->exprSign()==NEGATIVE){//cos(-x)-> cos(x)
				contExpr[0] = prodC(numC(-1L),contExpr[0]);
				contExpr[0]->simplify();
			}
			
			
			{//cos(complex)
				Expr *cn = contExpr[0]->complexExtract();
				
				if(!(cn->contExpr[1]->exprType == NUM && cn->contExpr[1]->value.equalsI(0L))){
				   Expr *repl = prodC(sinC(cn->contExpr[0]),sumC(powC(eC(),cn->contExpr[1]),prodC(numC(-1L),powC(eC(),prodC(numC(-1L),cn->contExpr[1]->copy())))));
				   repl->addElement(iC());
				   repl = sumC(repl,prodC(cosC(cn->contExpr[0]->copy()), sumC(powC(eC(),cn->contExpr[1]->copy()),powC(eC(),prodC(numC(-1L),cn->contExpr[1]->copy()))) ));
				   repl = prodC(repl,invC(numC(2L)));
				   
				   cn->nullify();
				   delete cn;
				   become(repl);
				   simplify();
				   return;
				}else delete cn;
			}
			
			if(contExpr[0]->exprType == PROD && constant()){//unit circle
				Expr *pr = contExpr[0];
				//
				bool foundPi = false;
				int64_t num = 1,den = 1;
				bool foundOther = false;
				for(int32_t i = 0;i<pr->numOfContExpr;i++){
					bool integer = pr->contExpr[i]->exprType == NUM && pr->contExpr[i]->value.rep == INT;
					bool inverseInt = pr->contExpr[i]->inverseInt();
					bool isPi = pr->contExpr[i]->exprType == NUM && pr->contExpr[i]->value.rep == PIV;
					if(integer){
						num = pr->contExpr[i]->value.valueI;
					}else if(inverseInt){
						den = pr->contExpr[i]->getBase()->value.valueI;
					}else if(isPi){
						foundPi = true;
					}else{
						foundOther = true;
					}
					
				}
				if(foundPi && !foundOther){
					contExpr[0] = sumC(contExpr[0],prodC(piC(),invC(numC(2L))));
					exprType = SIN;
					simplify();
					return;
				}
			}
			
			
			if(contExpr[0]->exprType == NUM && contExpr[0]->value.equalsI(0L)){//cos(0) -> 1
				clearElements();
				exprType = NUM;
				value.setValueI(1L);
				return;
			}else if(contExpr[0]->exprType == NUM && contExpr[0]->value.rep == PIV){
				clearElements();
				exprType = NUM;
				value.setValueI(-1L);
				return;
			}
		
		}
	}
	void Expr::asinSimp(){
		if(exprType == ASIN){
			contExpr[0]->factor();
			if(contExpr[0]->exprType == PROD){//asin(-x)-> -asin(x)
				Expr *pr = contExpr[0];
				bool sig = false;
				for(int32_t i = 0;i < pr->numOfContExpr;i++){
					if(pr->contExpr[i]->exprType == NUM){
						if(pr->contExpr[i]->value.neg()){
							pr->contExpr[i]->value.absN();
							sig = !sig;
							pr->simple = false;
						}
					}else if(pr->contExpr[i]->exprType == POW){
						Expr *pw = pr->contExpr[i];
						if(pw->expoIsMinusOne() && pw->getBase()->exprType == NUM){
							if(pw->getBase()->value.neg()){
								pw->getBase()->value.absN();
								sig = !sig;
								pr->simple = false;
							}
						}
					}
				}
				if(sig){
					Expr *repl = prodC(numC(-1L),copy());
					become(repl);
					simplify();
					return;
				}
			}
		
		
		}
	}
	void Expr::acosSimp(){
		if(exprType == ACOS){
			contExpr[0]->factor();
			if(contExpr[0]->exprType == PROD){//acos(-x)-> asin(x)+pi/2
				Expr *pr = contExpr[0];
				bool sig = false;
				for(int32_t i = 0;i < pr->numOfContExpr;i++){
					if(pr->contExpr[i]->exprType == NUM){
						if(pr->contExpr[i]->value.neg()){
							pr->contExpr[i]->value.absN();
							sig = !sig;
							pr->simple = false;
						}
					}else if(pr->contExpr[i]->exprType == POW){
						Expr *pw = pr->contExpr[i];
						if(pw->expoIsMinusOne() && pw->getBase()->exprType == NUM){
							if(pw->getBase()->value.neg()){
								pw->getBase()->value.absN();
								sig = !sig;
								pr->simple = false;
							}
						}
					}
				}
				if(sig){
					Expr *repl = prodC(sumC(prodC(asinC(contExpr[0]->copy()),numC(2L)),piC()),invC(numC(2L)));
					become(repl);
					simplify();
					return;
				}
			}
		}
	}
	
	byte Expr::exprSign(){//returns 1 if statement is negative 0 if positive and 2 if unknown. Note variables are asumed to be positive
		if(exprType == NUM){
			return (byte)value.neg();
		}else if(exprType == PROD){
			bool sig = false;
			for(int32_t i = 0;i<numOfContExpr;i++){
				if(contExpr[i]->exprType == NUM){
					if(contExpr[i]->value.neg()) sig = !sig;
				}else if(contExpr[i]->exprType == POW){
					Expr *pw = contExpr[i];
					if(pw->expoIsMinusOne() && pw->getBase()->exprType == NUM){
						if(pw->getBase()->value.neg()) sig = !sig;
					}
				}
			}
			return (byte)sig;
		}else if(exprType == POW){
			if(expoIsMinusOne() && getBase()->exprType == NUM){
				return (byte)getBase()->value.neg();
			}
		}
		return 2;
	}
	int64_t Expr::getDegree(Expr *var){//return -1 if invalid (not a polynomial), input is asumed to be simplified
		if(!contains(var)){
			return 0;
		}if(equalStruct(var)){
			return 1;
		}else if(exprType == POW){
			Expr *expo = getExpo();
			if(getBase()->equalStruct(var) && expo->exprType == NUM && expo->value.rep == INT && !expo->value.neg()) return expo->value.valueI;
		}else if(exprType == PROD){
			
			int64_t maxExpo = -1;
			for(int32_t i = 0;i<numOfContExpr;i++){
				if(contExpr[i]->contains(var)){
					if(contExpr[i]->equalStruct(var)){
						if(1>maxExpo) maxExpo = 1;
					}else if(contExpr[i]->exprType == POW){
						Expr *pw = contExpr[i];
						Expr *expo = pw->getExpo();
						
						if(pw->getBase()->equalStruct(var) && expo->exprType == NUM && expo->value.rep == INT && !expo->value.neg()){
							if(expo->value.valueI>maxExpo) maxExpo = expo->value.valueI;
						}
					}else return -1;
				}
			}
			
			return maxExpo;
		}else if(exprType == SUM){
			int64_t maxExpo = -1L;
			for(int32_t i = 0;i<numOfContExpr;i++){
				if(contExpr[i]->equalStruct(var)){
					if(1>maxExpo)maxExpo = 1L;
					
				}else if(contExpr[i]->exprType == POW){
					Expr *pw = contExpr[i];
					Expr *expo = pw->getExpo();
					
					if(pw->getBase()->equalStruct(var) && expo->exprType == NUM && expo->value.rep == INT && !expo->value.neg()){
						if(expo->value.valueI>maxExpo) maxExpo = expo->value.valueI;
					}
				}else if(contExpr[i]->exprType == PROD){
					Expr *pr = contExpr[i];
					
					for(int32_t j = 0;j < pr->numOfContExpr;j++){
						if(pr->contExpr[j]->contains(var)){
							if(pr->contExpr[j]->equalStruct(var)){
								if(1>maxExpo) maxExpo = 1;
							}else if(pr->contExpr[j]->exprType == POW){
								Expr *pw = pr->contExpr[j];
								Expr *expo = pw->getExpo();
								
								if(pw->getBase()->equalStruct(var) && expo->exprType == NUM && expo->value.rep == INT && !expo->value.neg()){
									if(expo->value.valueI>maxExpo) maxExpo = expo->value.valueI;
								}
							}else return -1;
						}
					}
					
				}
				
			}
			return maxExpo;
		}
		return -1;
	}
	//poly extract does coefficients from low to high example x^2-2 -> [-2,0,1]
	Expr *Expr::polyExtract(Expr *var){//return the coefficients of the polynomial if it complies. DO NOT simplify output list
		Expr *coef = new Expr(LIST);
		if(!contains(var)){
			coef->addElement(copy());
			return coef;
		}else if(equalStruct(var)){
			coef->addElement(numC((int64_t)0));
			coef->addElement(numC((int64_t)1));
			return coef;
		}else if(exprType == POW){
			if(getBase()->equalStruct(var)){
				if(getExpo()->exprType == NUM && getExpo()->value.rep == INT && !getExpo()->value.neg()){
					int64_t val = getExpo()->value.valueI;
					for(int32_t i = 0;i<val+1;i++){
						if(i==val) coef->addElement(numC((int64_t)1));
						else coef->addElement(numC((int64_t)0));
					}
					return coef;
				}else{
					delete coef;
					return nullptr;
				}
			}else{
				delete coef;
				return nullptr;
			}
		}else if(exprType == PROD){
			int32_t xtermcount = 0;
			Expr *exCopy = copy();
			int64_t deg = 0;
			for(int32_t i = 0;i<exCopy->numOfContExpr;i++){
				if(exCopy->contExpr[i]->contains(var)){
					xtermcount++;
					Expr *cex = exCopy->contExpr[i];
					if(cex->equalStruct(var)){
						deg = 1;
					}else if(cex->exprType == POW){
						if(cex->getBase()->equalStruct(var) && cex->getExpo()->exprType == NUM && cex->getExpo()->value.rep == INT && !cex->getExpo()->value.neg()){
							deg = cex->getExpo()->value.valueI;
						}
					}else break;
					exCopy->removeElement(i);
					i--;
				}
			}
			if(xtermcount == 1 && deg != 0){
				exCopy->simplify();
				for(int32_t i = 0;i<deg+1;i++){
					if(i==deg) coef->addElement(exCopy);
					else coef->addElement(numC((int64_t)0));
				}
				return coef;
			}else{
				delete exCopy;
				delete coef;
				return nullptr;
			}
		}else if(exprType == SUM){
			for(int32_t i = 0;i < numOfContExpr;i++){
				if(!contExpr[i]->contains(var)){
					if(coef->numOfContExpr == 0) coef->addElement(contExpr[i]->copy());
					else coef->contExpr[0] = sumC(coef->contExpr[0],contExpr[i]->copy());
				}else if(contExpr[i]->equalStruct(var)){
					if(coef->numOfContExpr<1)coef->addElement(numC((int64_t)0));
					if(coef->numOfContExpr==1) coef->addElement(numC(1L));
					else coef->contExpr[1] = sumC(coef->contExpr[1],numC(1L));
				}else if(contExpr[i]->exprType == POW){
					Expr *pw = contExpr[i];
					if(pw->getBase()->equalStruct(var) && pw->getExpo()->exprType == NUM && pw->getExpo()->value.rep == INT && !pw->getExpo()->value.neg()){
						int64_t deg = pw->getExpo()->value.valueI;
						while(coef->numOfContExpr < deg) coef->addElement(numC((int64_t)0));
						if(coef->numOfContExpr == deg) coef->addElement(numC((int64_t)1));
						else coef->contExpr[deg] = sumC(coef->contExpr[deg],numC((int64_t)1));
					}else{
						delete coef;
						return nullptr;
					}
				}else if(contExpr[i]->exprType == PROD){
					Expr *pr = contExpr[i]->copy();
					int32_t xtermcount = 0;
					int64_t deg = 0;
					
					for(int32_t j = 0;j < pr->numOfContExpr;j++){
						
						if(pr->contExpr[j]->contains(var)){
							xtermcount++;
							Expr *cex = pr->contExpr[j];
							if(cex->equalStruct(var)){
								deg = 1;
							}else if(cex->exprType == POW){
								if(cex->getBase()->equalStruct(var) && cex->getExpo()->exprType == NUM && cex->getExpo()->value.rep == INT && !cex->getExpo()->value.neg()){
									deg = cex->getExpo()->value.valueI;
								}
							}else break;
							pr->removeElement(j);
							j--;
						}
						
						
					}
					if(xtermcount == 1 && deg != 0){
						while(coef->numOfContExpr < deg) coef->addElement(numC((int64_t)0));
						if(coef->numOfContExpr == deg) coef->addElement(pr);
						else coef->contExpr[deg] = sumC(coef->contExpr[deg],pr);
					}else{
						delete pr;
						delete coef;
						return nullptr;
					}
					
					
				}else {
					delete coef;
					return nullptr;
				}
				
			}
			for(int32_t i = 0;i<coef->numOfContExpr;i++) coef->contExpr[i]->simplify();
			return coef;
		}
		delete coef;
		return nullptr;
	}
	
	Expr *dividePoly(Expr *polyList,Expr *polyDiv){
		Expr *remain = polyList->copy();
		Expr *result = new Expr(LIST);
		for(int32_t i = 0;i<polyList->numOfContExpr-polyDiv->numOfContExpr+1;i++) result->addElement(nullptr);
		int32_t currentIndex = result->numOfContExpr-1;
		while(remain->numOfContExpr >= polyDiv->numOfContExpr){
			Expr *newCo = prodC(remain->contExpr[remain->numOfContExpr-1]->copy(),invC(polyDiv->contExpr[polyDiv->numOfContExpr-1]->copy()));
			newCo->simplify();
			
			for(int32_t i = 0;i<remain->numOfContExpr-1;i++){
				if(!(i<remain->numOfContExpr-polyDiv->numOfContExpr)){
					Expr *pr = new Expr(PROD);
					pr->addElement(polyDiv->contExpr[i-(remain->numOfContExpr-polyDiv->numOfContExpr)]->copy());
					pr->addElement(newCo->copy());
					pr->addElement(numC(-1L));
					Expr *sm = sumC(remain->contExpr[i], pr);
					sm->simplify();
					remain->contExpr[i] = sm;
				}
			}
			
			
			remain->removeElement(remain->numOfContExpr-1);
			result->contExpr[currentIndex] = newCo;
			currentIndex--;
		}
		return new Expr(LIST,result,remain);
	}
	
	void Expr::partFrac(Expr *var){//polynomial division and partial fraction decomp
		
		if(!contains(var)) return;
		printf("trying part frac\n");
		int64_t degDiff = 0;
		
		Expr *num = new Expr(PROD);
		Expr *den = new Expr(PROD);
		
		if(exprType == PROD){
			for(int32_t i = 0;i<numOfContExpr;i++){
				if(!contExpr[i]->contains(var)){
					num->addElement(contExpr[i]->copy());
				}else if(contExpr[i]->exprType == SUM){
					degDiff+=contExpr[i]->getDegree(var);
					num->addElement(contExpr[i]->copy());
				}else if(contExpr[i]->exprType == POW){
					Expr *pw = contExpr[i];
					
					if(pw->getExpo()->exprType == NUM && pw->getExpo()->value.rep == INT){
						Expr *ex = pw->getBase();
						if(ex->equalStruct(var)){
							degDiff+=pw->getExpo()->value.valueI;
						}else if(ex->exprType == SUM){
							degDiff+=(ex->getDegree(var))*pw->getExpo()->value.valueI;
						}else{
							delete num;
							delete den;
							return;
						}
						if(pw->getExpo()->value.valueI>0) num->addElement(pw->copy());
						else den->addElement(powC(ex->copy(), numC(-pw->getExpo()->value.valueI)));
					}else{
						delete num;
						delete den;
						return;
					}
				}else if(contExpr[i]->equalStruct(var)){
					degDiff++;
					num->addElement(contExpr[i]->copy());
				}else{
					delete num;
					delete den;
					return;
				}
			}
		}else if(exprType == POW){
			Expr *pw = this;
			if(pw->getExpo()->exprType == NUM && pw->getExpo()->value.rep == INT){
				Expr *ex = pw->getBase();
				if(ex->equalStruct(var)){
					degDiff+=pw->getExpo()->value.valueI;
				}else{
					delete num;
					delete den;
					return;
				}
				if(pw->getExpo()->value.valueI>0) num->addElement(pw->copy());
				else den->addElement(powC(ex->copy(), numC(-pw->getExpo()->value.valueI)));
			}else{
				delete num;
				delete den;
				return;
			}
		}else{
			delete num;
			delete den;
			return;
		}
		
		num->distr();
		den->simplify();
		
		num->tPrintln();
		den->tPrintln();
		
		if(degDiff >= 0){//polynomial division required
			printf("need polynomial division\n");
			
			Expr *denCopy = den->copy();
			denCopy->distr();
			
			Expr *numPoly = num->polyExtract(var);
			Expr *denPoly = denCopy->polyExtract(var);
			
			if(numPoly) numPoly->tPrintln();
			if(denPoly) denPoly->tPrintln();
			
			
			
			Expr *result = nullptr,*remain = nullptr;
			
			Expr *ans = dividePoly(numPoly, denPoly);
			result = ans->contExpr[0];
			remain = ans->contExpr[1];
			ans->nullify();
			delete ans;
			
			delete denCopy;
			delete denPoly;
			
			printf("result\n");
			if(result){
				printf("res:\n");
				result->tPrintln();
			}
			
			if(remain){
				printf("rem:\n");
				remain->tPrintln();
			}
			
			//Expr *result = new Expr(LIST);
			//Expr *remain;
			
			
			
		}
		
		{
			printf("need partial fraction\n");
			//zeros
		}
		printf("degreee difference:%lld\n",degDiff);
		
		delete num;
		delete den;
	}
	
	const bool SHOW_INTEGRATION_STEPS = false;
	
	void Expr::integSimp(){
		if(exprType == INTEG){
			
			if(contExpr[0]->exprType == DERI){//Int(d(x)) -> x
				becomeInternal(contExpr[0]->contExpr[0]);
				return;
			}
			if(contExpr[0]->value.equalsI(0L) && contExpr[0]->exprType == NUM){//integral 0 -> 0
				clearElements();
				exprType = NUM;
				value.setValueI(0L);
				return;
			}
			if(contExpr[0]->exprType == PROD){//integral(5*x) -> 5*integral(x)
				Expr *pr = contExpr[0];
				Expr *replProd = nullptr;
				for(int32_t i = 0;i<pr->numOfContExpr;i++){
					if(pr->contExpr[i]->constant()){
						if(!replProd){
							replProd = new Expr(PROD);
						}
						replProd->addElement(pr->contExpr[i]);
						pr->contExpr[i] = nullptr;
						pr->removeElement(i);
						i--;
					}
				}
				if(replProd){
					replProd->addElement(copy());
					become(replProd);
					simplify();
					return;
				}
			}
			
			contExpr[0]->factor();
			
			if(contExpr[0]->exprType == PROD){ //common forms integral(form*d(x))
				Expr *pr = contExpr[0];
				if(pr->numOfContExpr == 2){//simple integrals
					Expr *var = nullptr;
					Expr *otherPart = nullptr;
					for(int32_t i = 0;i<2;i++){
						if(pr->contExpr[i]->exprType == DERI){
							var = pr->contExpr[i]->contExpr[0];
						}else{
							otherPart = pr->contExpr[i];
						}
					}
					
					if(var){
						if(var->equalStruct(otherPart)){//integral(x*d(x)) -> x^2/2
							Expr *repl = prodC(powC(var->copy(),numC(2L)),invC(numC(2L)));
							become(repl);
							simple = true;
							return;
						}
						if(otherPart->exprType == POW){//integral((linear)^c.*d(x)) -> inv(a*(c+1))*(linear)^(c+1)
							Expr *pw = otherPart;
							if(pw->getExpo()->constant()){
								if(pw->getBase()->equalStruct(var)){
									if(pw->expoIsMinusOne()){//1/x -> ln(x)
										Expr *repl = logC(var->copy());
										become(repl);
										simple = true;
										return;
									}else{
										Expr *expoPlusOne = sumC(pw->getExpo()->copy(),numC(1L));
										Expr *repl = prodC(powC(var->copy(),expoPlusOne),invC(expoPlusOne->copy()));
										become(repl);
										simplify();
										return;
									}
								}
								
								else if(pw->getBase()->exprType == SUM && pw->expoIsMinusOne()){//full inverse quadratic
									Expr *sm = pw->getBase();
									Expr *cf = sm->polyExtract(var);
									if(cf){
										if(cf->numOfContExpr == 3 && cf->constant()){
											Expr *repl;
											
											Expr *a = cf->contExpr[2];
											Expr *b = cf->contExpr[1];
											Expr *c = cf->contExpr[0];
											cf->nullify();
											delete cf;
											Expr *inner = prodC(numC(4L),a->copy());//4ac-b^2
											inner->addElement(c);
											inner = sumC(inner,prodC(numC(-1L),powC(b->copy(),numC(2L))));
											inner->simplify();
											
											byte sig = inner->exprSign();
											if(sig == POSITIVE || sig == UNKNOWN){
												Expr *common = powC(inner,invC(numC(-2)));
												repl = prodC(numC(2L),a);
												repl->addElement(var->copy());
												repl = prodC(atanC(prodC(sumC(b,repl),common)),numC(2L));
												repl->addElement(common->copy());
											}else{
												inner = prodC(inner,numC(-1L));
												Expr *common = prodC(numC(2L),a);
												
												common->addElement(var->copy());
												common = sumC(common,b);
												
												common->addElement(powC(inner,invC(numC(2L))));
												
												repl = new Expr(PROD,invC(common->copy()));
												
												common->contExpr[2] = prodC(numC(-1L),common->contExpr[2]);
												
												repl->addElement(common);
												repl = prodC(logC(repl),powC(inner->copy(),invC(numC(-2L))));
											}
											become(repl);
											simplify();
											return;
										}else delete cf;
									}
								}
								else if(pw->getBase()->exprType == SUM && pw->getExpo()->exprType == POW){//arcsin variant
									Expr *ex = pw->getExpo();
									if(ex->getBase()->exprType == NUM && ex->getBase()->value.equalsI(-2) && ex->expoIsMinusOne()){
										Expr *sm = pw->getBase();
										if(sm->numOfContExpr == 2){
											Expr *a = nullptr,*b = nullptr;
											for(int32_t k = 0;k<sm->numOfContExpr;k++){
												if(sm->contExpr[k]->contains(var)){
													b = sm->contExpr[k];
												}else{
													a = sm->contExpr[k];
												}
											}
											if(a && b && a->constant()){
												a = a->copy();
												b = b->copy();
												if(b->exprType == POW){
													if(b->getBase()->equalStruct(var) && b->getExpo()->exprType == NUM && b->getExpo()->value.equalsI(2L)){
														delete b;
														b = numC(1L);
													}
												}else if(b->exprType == PROD){
													for(int32_t k = 0;k<b->numOfContExpr;k++){
														if(b->contExpr[k]->contains(var) && b->contExpr[k]->exprType == POW){
															Expr *pw = b->contExpr[k];
															if(pw->getBase()->equalStruct(var) && pw->getExpo()->exprType == NUM && pw->getExpo()->value.equalsI(2L)){
																b->removeElement(k);
																break;
															}
														}
													}
												}
												if(b->constant()){
													Expr *innerProd = prodC(var->copy(),powC(a,invC(numC(-2))));
													innerProd->addElement(powC(prodC(numC(-1L),b->copy()),invC(numC(2L))));
													Expr *repl = prodC(asinC(innerProd),powC(prodC(numC(-1L),b),invC(numC(-2L))));
													become(repl);
													simplify();
													return;
												}else{
													delete a;
													delete b;
												}
											}
										}
									}
									
								}
								else if(pw->getBase()->exprType == SIN || pw->getBase()->exprType == COS || pw->getBase()->exprType == TAN || pw->getBase()->exprType == ATAN){//common trig
									if(pw->getBase()->contExpr[0]->equalStruct(var)){
										if(pw->getBase()->exprType == COS && pw->getExpo()->exprType == NUM && pw->getExpo()->value.equalsI(2L)){
											Expr *repl = prodC( sumC( var->copy(),prodC(sinC(var->copy()),cosC(var->copy())) ) ,invC(numC(2)));
											become(repl);
											simple = true;
											return;
										}else if(pw->getBase()->exprType == SIN && pw->expoIsMinusOne()){
											Expr *repl = prodC(logC(prodC(sumC(numC(1L),prodC(numC(-1L),cosC(var->copy()))),invC(sumC(numC(1L),cosC(var->copy()))))),invC(numC(2L)));
											become(repl);
											simple = true;
											return;
										}else if(pw->getBase()->exprType == COS && pw->expoIsMinusOne()){
											Expr *repl = prodC(logC(prodC(sumC(numC(1L),sinC(var->copy())),invC(sumC(numC(1L),prodC(numC(-1L),sinC(var->copy())))))),invC(numC(2L)));
											become(repl);
											simple = true;
											return;
										}else if(pw->getBase()->exprType == TAN && pw->expoIsMinusOne()){
											Expr *tansq = powC(tanC(var->copy()),numC(2L));
											Expr *repl = prodC(logC(prodC(tansq,invC(sumC(tansq->copy(),numC(1L))))),invC(numC(2L)));
											become(repl);
											simple = true;
											return;
										}
									}
								}
							}else if(pw->getBase()->constant()){//c^x -> c^x/ln(c)
								if(pw->getExpo()->equalStruct(var)){
									Expr *repl = prodC(invC(logC(pw->getBase()->copy())),pw->copy());
									become(repl);
									simplify();
									return;
								}
							}
						}else if(otherPart->exprType == COS){//integral of cos
							if(otherPart->contExpr[0]->equalStruct(var)){
								Expr *repl = sinC(var->copy());
								become(repl);
								simple = true;
								return;
							}
						}else if(otherPart->exprType == SIN){//integral of sin
							if(otherPart->contExpr[0]->equalStruct(var)){
								Expr *repl =  prodC(cosC(var->copy()),numC(-1L));
								become(repl);
								simple = true;
								return;
							}
						}else if(otherPart->exprType == ASIN){//integral of arcsin
							if(otherPart->contExpr[0]->equalStruct(var)){
								Expr *repl =  sumC(powC(sumC(numC(1L),prodC(numC(-1L),powC(var->copy(),numC(2L)))),invC(numC(2L))),prodC(var->copy(),asinC(var->copy())));
								become(repl);
								simple = true;
								return;
							}
						}else if(otherPart->exprType == ACOS){//integral of arccos
							if(otherPart->contExpr[0]->equalStruct(var)){
								Expr *repl =  sumC(prodC(numC(-1L),powC(sumC(numC(1L),prodC(numC(-1L),powC(var->copy(),numC(2L)))),invC(numC(2L)))),prodC(var->copy(),acosC(var->copy())));
								become(repl);
								simple = true;
								return;
							}
						}else if(otherPart->exprType == ATAN){//integral of arctan
							if(otherPart->contExpr[0]->equalStruct(var)){
								Expr *repl =  prodC(var->copy(),atanC(var->copy()));
								repl->addElement(numC(2L));
								repl = sumC(repl,logC(sumC(numC(1L),powC(var->copy(),numC(2L)))));
								repl = prodC(repl,invC(numC(2)));
								become(repl);
								simple = true;
								return;
							}
						}else if(otherPart->exprType == TAN){//integral of arctan
							if(otherPart->contExpr[0]->equalStruct(var)){
								Expr *repl =  prodC(numC(-1L),logC(cosC(var->copy())));
								
								become(repl);
								simple = true;
								return;
							}
						}
						
						
						
					}
				}
				
				int32_t expCount = 0;
				Expr *var = nullptr;
				int32_t indexToSkip = 0;
				
				for(int32_t i = 0;i<pr->numOfContExpr;i++){//extract the variable
					if(pr->contExpr[i]->exprType == DERI){
						var = pr->contExpr[i]->contExpr[0];
						indexToSkip = i;
					}else if(pr->contExpr[i]->exprType == POW){//a^b -> e^(ln(a)*b) if b is non constant helpful for u sub
						Expr *pw = pr->contExpr[i];
						if(!pw->getExpo()->constant()){
							pw->setExpo(prodC(logC(pw->getBase()),pw->getExpo()));
							pw->setBase(eC());
							pw->getExpo()->simplify();
							expCount++;
						}
					}
				}
				
				if(!var) return;
				
				if(pr->numOfContExpr == 3){
					bool hasInv = false;
					byte hasTrig = 0;
					for(int32_t i = 0;i<pr->numOfContExpr;i++){
						if(i == indexToSkip) continue;
						if(pr->contExpr[i]->exprType == POW){
							if(pr->contExpr[i]->expoIsMinusOne() && pr->contExpr[i]->getBase()->equalStruct(var)){
								hasInv = true;
							}
						}else if(pr->contExpr[i]->exprType == SIN){
							Expr *sin = pr->contExpr[i];
							if(sin->contExpr[0]->equalStruct(var)){
								hasTrig = 1;
							}
						}else if(pr->contExpr[i]->exprType == COS){
							Expr *cos = pr->contExpr[i];
							if(cos->contExpr[0]->equalStruct(var)){
								hasTrig = 2;
							}
						}
					}
					if(hasTrig && hasInv){
						Expr *repl = nullptr;
						if(hasTrig == 1) repl = SiC(var->copy());
						else if(hasTrig == 2) repl = CiC(var->copy());
						
						become(repl);
						return;
					}
				}
				
				//combine e^x
				if(expCount > 1){
					Expr *expSum = new Expr(SUM);
					for(int32_t i = 0;i<pr->numOfContExpr;i++){
						if(pr->contExpr[i]->exprType == POW){
							Expr *pw = pr->contExpr[i];
							if(pw->getBase()->exprType == NUM && pw->getBase()->value.rep == EV){
								expSum->addElement(pw->getExpo());
								pw->setExpo(nullptr);
								pr->removeElement(i);
								i--;
							}
						}
					}
					Expr *tba = powC(eC(),expSum);
					
					pr->addElement(tba);
					
				}
				
				
				
				if(var && pr->numOfContExpr > 2){//u sub special case: example integral(sin(x)*cos(x)*d(x)) or integral(ln(x)*inv(x)*d(x))
					
					for(int32_t i = 0;i< pr->numOfContExpr;i++){
						if(i==indexToSkip) continue;
						Expr *possiblyU = diffC(pr->contExpr[i]->copy());
						possiblyU->simplify();
						Expr *constants = new Expr(PROD);
						if(possiblyU->exprType == PROD){//extract out the contants
							for(int32_t k = 0;k<possiblyU->numOfContExpr;k++){
								if(possiblyU->contExpr[k]->constant()){
									constants->addElement(possiblyU->contExpr[k]);
									possiblyU->contExpr[k] = nullptr;
									possiblyU->removeElement(k);
									k--;
								}
							}
						}
						possiblyU = prodC(pr->copy(),invC(possiblyU));//if dividing by derivative gives itself then its needs u sub
						possiblyU->simplify();
						if(possiblyU->equalStruct(pr->contExpr[i])){
							if(SHOW_INTEGRATION_STEPS){
								printf("u=");
								possiblyU->tPrintln();
								printf("special u sub\n");
							}
							become(prodC(powC(possiblyU,numC(2L)),invC(numC(2L))));
							addElement(invC(constants));//sort of a double u sub handler
							simplify();
							return;
						}
						//remove trash
						delete constants;
						delete possiblyU;
					}
				}
				
				
				
				if(var && strcmp(var->name->chars,"0u") != 0){//general u sub
					//find most nested part that is not dx and not a sum
					int32_t max = 0;
					int32_t indexOfHighestDepth = 0;
					for(int32_t i = 0;i < pr->numOfContExpr;i++){
						if(indexToSkip == i) continue;
						
						int32_t depth = pr->contExpr[i]->nestDepth();
						if(depth>max && pr->contExpr[i]->exprType != SUM){//u should not be a sum because thats usually what d(u) is
							max = depth;
							indexOfHighestDepth = i;
						}
					}
					Expr *mostComplicated = pr->contExpr[indexOfHighestDepth];
					if(mostComplicated->numOfContExpr > 0){
						Expr *inner = nullptr;
						
						max = 0;
						int32_t indexOfHighestDepth2 = 0;
						for(int32_t k = 0;k<mostComplicated->numOfContExpr;k++){
							if(!mostComplicated->contExpr[k]->constant()){
								int32_t depth = mostComplicated->contExpr[k]->nestDepth();
								if(depth>max){
									max = depth;
									indexOfHighestDepth2 = k;
								}
							}
						}
						
						inner = mostComplicated->contExpr[indexOfHighestDepth2];
						if(mostComplicated->exprType == POW){
							if(mostComplicated->getExpo()->exprType == NUM && mostComplicated->getExpo()->value.neg()){
								if(inner->exprType == SIN || inner->exprType == COS || inner->exprType == TAN){//u should not be a trig function if in the denominator as it makes things more complicated
									if(!inner->contExpr[0]->equalStruct(var)) inner = inner->contExpr[0];
								}
							}
						}
						
						if(!inner->equalStruct(var)){
							Expr *u = varC("0u");
							
							Expr *mcu = pr->copy();
							mcu->contExpr[indexOfHighestDepth]->replace(inner,u);
							
							mcu->addElement(invC(diffC(inner->copy())));
							mcu->addElement(diffC(u->copy()));
							mcu->simplify();
							
							if(mcu->contains(var)){
							
								if(SHOW_INTEGRATION_STEPS) {
									printf("u=");
									inner->tPrintln();
									printf("normal u sub with solve\n");
								
								}
								
								Expr *inTermsOfU = new Expr(SOLVE,equC(u->copy(),inner->copy()),var->copy());
								inTermsOfU->simplify();
								
								
								if(inTermsOfU->exprType == EQU){
									
									Expr *temp = inTermsOfU->contExpr[1];
									inTermsOfU->contExpr[1] = nullptr;
									delete inTermsOfU;
									inTermsOfU = temp;
									mcu->replace(var,inTermsOfU);
									delete inTermsOfU;
									mcu = integC(mcu);
									mcu->simplify();
									
									if(!mcu->containsType(INTEG)){
										mcu->replace(u,inner);
										delete u;
										become(mcu);
										simple = false;
										simplify();
										return;
									}else{
										delete u;
										delete mcu;
									}
									
								}else{
									delete inTermsOfU;
									delete u;
									delete mcu;
								}
							}else{
								if(SHOW_INTEGRATION_STEPS){
									printf("u=");
									inner->tPrintln();
									printf("normal u sub\n");
								}
								mcu = integC(mcu);
								mcu->simplify();
								mcu->replace(u,inner);
								delete u;
								become(mcu);
								simple = false;
								simplify();
								return;
							}
							
							
						}
					}
				}
				
				//try most nested u sub inegral of cos(x)*sqrt(1+sin(x)^2)*d(x)
				{
					//printf("try this\n");
					
				}
				
				//
				
				
				if(var){//(x+1)^2 gets expanded for integration by parts easier
					for(int32_t i = 0;i < pr->numOfContExpr;i++){
						if(pr->contExpr[i]->exprType == POW){
							Expr *pw = pr->contExpr[i];
							if(pw->getExpo()->exprType == NUM && pw->getBase()->exprType == SUM && pw->getExpo()->value.rep == INT && pw->getExpo()->value.valueI > 0 && pw->getExpo()->value.valueI < 3){
								printf("test!!!!!!\n");
								Expr *u = varC("0u.");
								pw->replace(var,u);
								
								pw->simplify();
								
								pw->replace(u,var);
								delete u;
							}
						}
					}
				}
				
				if(var){//IBP integral(fraction*d(x)*expr ) -> expr*integral(fraction*d(x))-integral(d(expr)*integral(fraction*d(x))) fraction must have expo < -1
					//example integral(x*e^(2*x)/(2*x+1)^2*d(x))
					int32_t indexOfFraction = -1;
					for(int32_t i = 0;i<pr->numOfContExpr;i++){
						if(pr->contExpr[i]->exprType == POW){
							Expr *pw = pr->contExpr[i];
							if(pw->getExpo()->exprType == NUM && pw->getExpo()->value.rep == INT && pw->getExpo()->value.valueI < -1 && pw->contains(var) && pw->getBase()->equalStruct(var)){
								indexOfFraction = i;
								break;
							}else if(pw->getExpo()->exprType == PROD && pw->getBase()->equalStruct(var)){
								Expr *pwPr = pw->getExpo();
								if(pwPr->numOfContExpr == 2){
									Expr *num = nullptr,*den = nullptr;
									for(int32_t j = 0;j<pwPr->numOfContExpr;j++){
										if(pwPr->contExpr[j]->exprType == NUM && pwPr->contExpr[j]->value.rep == INT){
											num = pwPr->contExpr[j];
										}else if(pwPr->contExpr[j]->exprType == POW){
											Expr *pw = pwPr->contExpr[j];
											if(pw->inverseInt()){
												den = pw->getBase();
											}
										}else break;
									}
									if(num && den){
										if(-num->value.valueI > den->value.valueI){
											indexOfFraction = i;
											break;
										}
										
									}
								}
							}
						}
					}
					if(indexOfFraction != -1){
						if(SHOW_INTEGRATION_STEPS){
							printf("integrate :");
							pr->contExpr[indexOfFraction]->tPrintln();
							printf("special IBP\n");
						}
						Expr *fraction = integC(prodC(pr->contExpr[indexOfFraction],diffC(var->copy())));
						pr->contExpr[indexOfFraction] = nullptr;
						pr->removeElement(indexOfFraction);
						pr->addElement(invC(diffC(var->copy())));
						fraction->simplify();
						pr->simplify();
						
						Expr *repl = sumC(prodC(pr->copy(),fraction),prodC(numC(-1L),integC(prodC(diffC(pr->copy()),fraction->copy()))));
						become(repl);
						simplify();
						return;
					}
				}
				
				if(var){//IBP  integral((polynomial)*(otherPart)) -> polynomial*integral(otherPart)-integral(d(polynomial)*integral(otherPart))
					int32_t indexOfEasy = -1;
					int32_t priority = 1;
					for(int32_t i = 0;i<pr->numOfContExpr;i++){
						if(pr->contExpr[i]->equalStruct(var)){
							if(priority == 1) indexOfEasy = i;
						
						}else if(pr->contExpr[i]->exprType == SUM){//no sums should exist
							indexOfEasy = -1;
							break;
						}else if((pr->contExpr[i]->exprType == LOG || pr->contExpr[i]->exprType == ATAN || pr->contExpr[i]->exprType == ASIN || pr->contExpr[i]->exprType == ACOS) && pr->contExpr[i]->contExpr[0]->equalStruct(var)){
							
							indexOfEasy = i;
							priority = 2;
							
						}else if(pr->contExpr[i]->exprType == POW){
							Expr *pw = pr->contExpr[i];
							if(pw->expoIsMinusOne()){
								indexOfEasy = -1;
								break;
							}
							if(pw->getExpo()->exprSign() != NEGATIVE && pw->getExpo()->constant() && pw->getBase()->equalStruct(var)){//fractional power like x^(1/2) should be diffrenciated only if it is positive
									if(priority == 1) indexOfEasy = i;
							}else if(pw->getExpo()->exprType == NUM && pw->getExpo()->value.rep == INT && pw->getExpo()->value.valueI>0){//x^2 or ln(x)^2
								if(pw->getBase()->exprType == LOG || pw->getBase()->exprType == ATAN || pw->getBase()->exprType == ACOS || pw->getBase()->exprType == ASIN){
									indexOfEasy = i;
									priority = 2;
								}
							}
							
						}
					}
					
					if(indexOfEasy!=-1){
						Expr *easy = pr->contExpr[indexOfEasy];
						if(SHOW_INTEGRATION_STEPS){
							printf("diffrenciate:");
							easy->tPrintln();
							printf("normal IBP\n");
						}
						pr->contExpr[indexOfEasy] = nullptr;
						pr->removeElement(indexOfEasy);
						
						Expr *otherPartIntegral = integC(pr->copy());
						otherPartIntegral->simplify();
						Expr *repl = sumC(prodC(easy,otherPartIntegral),prodC(numC(-1L),integC(prodC(diffC(easy->copy()),otherPartIntegral->copy()))));
						become(repl);
						simplify();
						return;
					}
					
					
				}
				
				
			}
			contExpr[0]->distr();
			if(contExpr[0]->exprType == SUM){//integral(x+y) -> integral(x)+integral(y)
				//check to see how many variables are involved
				bool oneVar = false;
				Expr *var = nullptr;
				if(contExpr[0]->contExpr[0]->exprType == PROD){
					Expr *pr = contExpr[0]->contExpr[0];
					for(int32_t i = 0;i < pr->numOfContExpr;i++){
						if(pr->contExpr[i]->exprType == DERI){
							var = pr->contExpr[i]->contExpr[0]->copy();
							break;
						}
					}
				}else if(contExpr[0]->contExpr[0]->exprType == DERI){
					var = contExpr[0]->contExpr[0]->contExpr[0]->copy();
				}
				if(var){
					Expr *temp = varC("0x.");
					replace(var,temp);
					if(constant()) oneVar = true;
					replace(temp,var);
					delete var;
					delete temp;
					if(oneVar){
						Expr *sm = contExpr[0];
						Expr *repl = new Expr(SUM);
						for(int32_t i = 0;i<sm->numOfContExpr;i++){
							Expr *peice = integC(sm->contExpr[i]->copy());
							peice->simplify();
							repl->addElement(peice);
							sm->removeElement(i);
							i--;
						}
						
						repl->addElement(copy());
						become(repl);
						simplify();
						return;
					}
				}
			}
			
		}
	}
	void Expr::orSimp(){
		if(exprType == OR){
			{//or contains or
				for(int32_t i = 0;i<numOfContExpr;i++){
					if(contExpr[i]->exprType == exprType){
						Expr *temp = contExpr[i];
						contExpr[i] = nullptr;
						removeElement(i);
						i--;
						for(int32_t j = 0;j<temp->numOfContExpr;j++) addElement(temp->contExpr[j]);
						temp->nullify();
						delete temp;
					}
				}
			}
			
			{// x && false -> false and x && true -> x
				bool hasTrue = false;
				for(int32_t i = 0;i<numOfContExpr;i++){
					if(contExpr[i]->exprType == NUM && contExpr[i]->value.rep == BOOL){
						bool state = contExpr[i]->value.getState();
						if(!state){
							removeElement(i);
							i--;
						}
						else hasTrue = true;
						break;
					}
				}
				if(hasTrue){
					become(boolC(true));
					return;
				}
			}
			int32_t numOfVars = 0;
			{//factor technique which allows simplifications like a && b && c || a && c || d to become a && c || d
				Expr *list = new Expr(LIST);
				getAllExprInBool(list);
				numOfVars = list->numOfContExpr;
				for(int32_t i = 0;i<numOfVars;i++) list->addElement(notC(list->contExpr[i]->copy()));
				
				for(int32_t i = 0;i<list->numOfContExpr;i++){
					Expr *v = list->contExpr[i];
					
					Expr *orEx = new Expr(OR);
					for(int32_t j = 0;j<numOfContExpr;j++){
						if(contExpr[j]->exprType == AND){
							bool hasVar = false;
							Expr *andEx = contExpr[j];
							for(int32_t k = 0;k<andEx->numOfContExpr;k++){
								if(andEx->contExpr[k]->equalStruct(v)){
									hasVar = true;
									andEx->removeElement(k);
									break;
								}
							}
							if(hasVar){
								orEx->addElement(contExpr[j]);
								contExpr[j] = nullptr;
								removeElement(j);
								j--;
							}
						}
						
					}
					
					orEx->simplify();
					
					if(orEx->exprType == OR){
						if(orEx->numOfContExpr == 0) {
							delete orEx;
							continue;
						}
						for(int32_t j = 0;j<orEx->numOfContExpr;j++){
							if(orEx->contExpr[j]->exprType == AND) orEx->contExpr[j]->addElement(v->copy());
							orEx->contExpr[j] = andC(orEx->contExpr[j],v->copy());
							addElement(orEx->contExpr[j]);
							orEx->contExpr[j] = nullptr;
						}
					}else if(orEx->exprType == AND){
						orEx->addElement(v->copy());
						addElement(orEx->copy());
					}else if(orEx->exprType == NUM && orEx->value.rep == BOOL && orEx->value.getState()==false){
						delete orEx;
						continue;
					}else{
						addElement(andC(orEx->copy(),v->copy()));
					}
					
					delete orEx;
					
					
				}
				
				delete list;
			}
			
			for(int32_t i = 0;i<numOfContExpr;i++){//remove duplicates and a || a && b -> a
				for(int32_t j = i+1;j<numOfContExpr;j++){
					if(contExpr[i]->equalStruct(contExpr[j])){
						removeElement(j);
						j--;
					}else if(contExpr[j]->exprType == AND && contExpr[i]->exprType != AND){
						Expr *andEx = contExpr[j];
						for(int32_t k = 0;k<andEx->numOfContExpr;k++){ if(andEx->contExpr[k]->equalStruct(contExpr[i])) {
								removeElement(j);
								j--;
							}
						}
					}else if (contExpr[j]->exprType != AND && contExpr[i]->exprType == AND){
						Expr *andEx = contExpr[i];
						for(int32_t k = 0;k<andEx->numOfContExpr;k++){ if(andEx->contExpr[k]->equalStruct(contExpr[j])) {
								removeElement(i);
								i--;
								break;
							}
						}
					}
				}
			}
			
			if(numOfVars >= 3 && numOfContExpr >= 3){//consensus
				for(int32_t i = 0;i< numOfContExpr;i++){//sort elements for faster processing
					if(contExpr[i]->numOfContExpr == 2 && contExpr[i]->exprType == AND){
						Expr *pair = contExpr[i];
						double left,right;
						if(pair->contExpr[0]->exprType == NOT) left = pair->contExpr[0]->contExpr[0]->hash();
						else left = pair->contExpr[0]->hash();
						
						if(pair->contExpr[1]->exprType == NOT) right = pair->contExpr[1]->contExpr[0]->hash();
						else right = pair->contExpr[1]->hash();
						
						if(right>left) {
							Expr *temp = pair->contExpr[0];
							pair->contExpr[0] = pair->contExpr[1];
							pair->contExpr[1] = temp;
						}
						
					}
				}
				for(int32_t i = 0;i< numOfContExpr;i++){
					for(int32_t j = i+1;j< numOfContExpr;j++){
						if(contExpr[i]->numOfContExpr == 2 && contExpr[i]->exprType == AND && contExpr[j]->numOfContExpr == 2 && contExpr[j]->exprType == AND){
						
							Expr *fpair = contExpr[i],*spair = contExpr[j];
							Expr *fpairV[2], *spairV[2];
							bool fpairN[2] = {false},spairN[2] = {false};
							
							for(int32_t k = 0;k<2;k++){
								if(fpair->contExpr[k]->exprType == NOT){
									fpairV[k] = fpair->contExpr[k]->contExpr[0];
									fpairN[k] = true;
								}else fpairV[k] = fpair->contExpr[k];
								
								if(spair->contExpr[k]->exprType == NOT){
									spairV[k] = spair->contExpr[k]->contExpr[0];
									spairN[k] = true;
								}else spairV[k] = spair->contExpr[k];
							}
							
							bool foundPair = false;
							if(fpairV[0]->equalStruct(spairV[0])){
								foundPair = true;
								fpairV[1] = fpair->contExpr[1];
								spairV[1] = spair->contExpr[1];
							}
							else if(fpairV[1]->equalStruct(spairV[1])){//swap
								foundPair = true;
								
								fpairN[0] = fpairN[1];
								fpairV[1] = fpair->contExpr[0];
								
								spairN[0] = spairN[1];
								spairV[1] = spair->contExpr[0];
								
							}
							
							if(foundPair && !fpairV[1]->equalStruct(spairV[1]) && (fpairN[0] ^ spairN[0] )){//ready
								Expr *look = andC(fpairV[1], spairV[1]);
								
								for(int32_t k = 0;k<numOfContExpr;k++){
									if(contExpr[k]->equalStruct(look)){
										removeElement(k);
										k--;
									}
								}
								
								look->nullify();
								delete look;
							}
							
						}
					}
				}
				
			}
			
			{//x || !x -> true
				bool foundOpposite = false;
				for(int32_t i = 0;i<numOfContExpr;i++){//remove duplicates
					bool isNot = contExpr[i]->exprType == NOT;
					for(int32_t j = i+1;j<numOfContExpr;j++){
						if(isNot){
							if(contExpr[i]->contExpr[0]->equalStruct(contExpr[j])){
								foundOpposite = true;
								break;
							}
						}else if(contExpr[j]->exprType == NOT && contExpr[i]->equalStruct(contExpr[j]->contExpr[0])){
							foundOpposite = true;
							break;
						}
					}
					if(foundOpposite) break;
				}
				if(foundOpposite){
					become(boolC(true));
					return;
				}
			}
			
			{//x || !x -> true, complicated case with demorgan , example !(x && y) || x && y, this becomes x && y || !x || !y which -> true
				for(int32_t i = 0;i<numOfContExpr;i++){
					if(contExpr[i]->exprType == AND){
						Expr *list = contExpr[i]->copy();
						
						bool becomeTrue = true;
						for(int32_t j = 0;j<list->numOfContExpr;j++){
							if(list->contExpr[j]->exprType == NOT) list->contExpr[j]->becomeInternal(list->contExpr[j]->contExpr[0]);
							else list->contExpr[j] = notC(list->contExpr[j]);
							
							bool found = false;
							for(int32_t k = 0;k<numOfContExpr;k++){
								if(contExpr[k]->equalStruct(list->contExpr[j])){
									found = true;
									break;
								}
							}
							if(!found){
								becomeTrue = false;
								break;
							}
						}
						
						if(becomeTrue){
							become(boolC(true));
							delete list;
							return;
						}
						delete list;
					}
				}
			}
			
			{//alone
				if(numOfContExpr == 1){
					becomeInternal(contExpr[0]);
					return;
				}else if(numOfContExpr == 0){
					become(boolC(false));
					return;
				}
			}
		}
	}

	void Expr::andSimp(){
		if(exprType == AND){
			
			
			{//and contains and
				for(int32_t i = 0;i<numOfContExpr;i++){
					if(contExpr[i]->exprType == exprType){
						Expr *temp = contExpr[i];
						contExpr[i] = nullptr;
						removeElement(i);
						i--;
						for(int32_t j = 0;j<temp->numOfContExpr;j++) addElement(temp->contExpr[j]);
						temp->nullify();
						delete temp;
					}
				}
			}
			{// x && false -> false and x && true -> x
				bool hasFalse = false;
				for(int32_t i = 0;i<numOfContExpr;i++){
					if(contExpr[i]->exprType == NUM && contExpr[i]->value.rep == BOOL){
						bool state = contExpr[i]->value.getState();
						if(state){
							removeElement(i);
							i--;
						}
						else hasFalse = true;
						break;
					}
				}
				if(hasFalse){
					become(boolC(false));
					return;
				}
			}
			for(int32_t i = 0;i<numOfContExpr;i++){//remove duplicates
				for(int32_t j = i+1;j<numOfContExpr;j++){
					if(contExpr[i]->equalStruct(contExpr[j])){
						removeElement(j);
						j--;
					}
				}
			}
			{//distr
				for(int32_t i = 0;i<numOfContExpr;i++){
					if(contExpr[i]->exprType == OR){
						Expr *outer = copy();
						outer->removeElement(i);
						outer->simplify();
						becomeInternal(contExpr[i]);
						for(int32_t j = 0;j<numOfContExpr;j++) contExpr[j] = andC(outer->copy(),contExpr[j]);
						simple = false;
						delete outer;
						simplify();
						return;
					}
				}
			}
			{//x && !x -> false
				bool foundOpposite = false;
				for(int32_t i = 0;i<numOfContExpr;i++){//remove duplicates
					bool isNot = contExpr[i]->exprType == NOT;
					for(int32_t j = i+1;j<numOfContExpr;j++){
						if(isNot){
							if(contExpr[i]->contExpr[0]->equalStruct(contExpr[j])){
								foundOpposite = true;
								break;
							}
						}else if(contExpr[j]->exprType == NOT && contExpr[i]->equalStruct(contExpr[j]->contExpr[0])){
							foundOpposite = true;
							break;
							
						}
					}
					if(foundOpposite) break;
				}
				if(foundOpposite){
					become(boolC(false));
					return;
				}
			}
			{//alone
				if(numOfContExpr == 1){
					becomeInternal(contExpr[0]);
					return;
				}else if(numOfContExpr == 0){
					become(boolC(true));
					return;
				}
			}
			 
		}
	}
		
	void Expr::notSimp(){
		if(exprType == NOT){
			
			if(contExpr[0]->exprType == NOT){// !!x -> x
				becomeInternal(contExpr[0]->contExpr[0]);
				simplify();
				return;
			}
			if(contExpr[0]->exprType == NUM && contExpr[0]->value.rep == BOOL){// !true -> false  and !false -> true
				contExpr[0]->value.setState(!contExpr[0]->value.getState());
				becomeInternal(contExpr[0]);
				return;
			}
			if(contExpr[0]->exprType == OR || contExpr[0]->exprType == AND){//demorgan law
				becomeInternal(contExpr[0]);
				if(exprType == OR) exprType = AND;
				else exprType = OR;
				
				for(int32_t i = 0;i<numOfContExpr;i++){
					contExpr[i] = notC(contExpr[i]);
					contExpr[i]->simple = false;
				}
				simple = false;
				simplify();
				return;
			}
		}
	}

	const bool SHOW_RECURSION = false;
	bool SHOW_ALL_STEPS = false;
	const bool LIMIT_RECURSION = false;
	int32_t recursiveCounter = 0;
	
	void Expr::simplify(){
		simplify(true);
	}
	
	void Expr::simplify(bool addFractions){
		recursiveCounter++;
		if(SHOW_RECURSION) printf("recursiveCounter: %d\n",recursiveCounter);
		if(LIMIT_RECURSION){
			if(recursiveCounter>16){
				printf("recursiveError!!!! terminate the program.\n");
				while (true) {}
			}
		}
		if(simple){
			recursiveCounter--;
			return;
		}
		if(SHOW_ALL_STEPS) tPrintln();
		if(exprType == SUBST){
			if(contExpr[1]->exprType == EQU){
				replace(contExpr[1]->contExpr[0],contExpr[1]->contExpr[1]);
				becomeInternal(contExpr[0]);
			}else if(contExpr[1]->exprType == LIST){
				Expr *list = contExpr[1];
				for(int32_t i = 0;i<contExpr[1]->numOfContExpr;i++){
					list->contExpr[i] = new Expr(SUBST,contExpr[0]->copy(),list->contExpr[i]);
				}
				becomeInternal(contExpr[1]);
				simple = false;
			}
		}
		if(exprType == VAR || exprType == NUM){
			simple = true;
			recursiveCounter--;
			return;
		}
		
		for(int32_t i = 0;i<numOfContExpr;i++) contExpr[i]->simplify();
		
		if(exprType == POW) powSimp();
		else if(exprType == DERI) derivSimp();
		
		else if(exprType == SUM) sumSimp(addFractions);
		else if(exprType == PROD) prodSimp();
		else if(exprType == LOG) logSimp();
		else if(exprType == SOLVE) solverSimp();
		else if(exprType == ABS) absSimp();
		else if(exprType == LIST) listSimp();
		else if(exprType == EQU) equSimp();
		else if(exprType == SIN) sinSimp();
		else if(exprType == COS) cosSimp();
		else if(exprType == ASIN) asinSimp();
		else if(exprType == ACOS) acosSimp();
		else if(exprType == INTEG) integSimp();
		else if(exprType == OR) orSimp();
		else if(exprType == AND) andSimp();
		else if(exprType == NOT) notSimp();
		
		if(containsType(INDET)) {
			clearElements();
			exprType = INDET;
		}
		simple = true;
		recursiveCounter--;
	}
	//RPN scanner
	
	const int32_t STACK_MAX = 64;
	Expr *stack[STACK_MAX];
	int32_t height = 0;
	
	void printStack(bool clear){
		if(CLEAR_TERM && clear) system("clear");
		printf("________________STACK_START\n");
		for(int32_t i = 0;i<height;i++){
			printf("%d: ",i);
			stack[i]->tPrintln();
		}
		printf("________________STACK_END\n");
	
	}
	
	void printStack(){
		printStack(true);
	}
	
	void clearStack(){
		if(CLEAR_TERM) system("clear");
		for(int32_t i = 0;i<height;i++) delete stack[i];
		height = 0;
	}
	
	Expr* rpnCas(){
		height = 0;
		printf("bytes used per expression: %ld\n",sizeof(Expr));
		printf("RPN Calculator\nType 'help' for help and 'quit' to quit\n");
		printf("Max elements in stack: %d\n",STACK_MAX);
		printf("pointer limit: %d\n",totalTrackedBytesSize);
		
		while(true){
			char op[128];
			scanf("\n%s",op);
			if(strcmp(op,"help")==0){
				printf("all commands are one character\nlist of all commands:\n");
				printf("	help : help\n");
				printf("	clear : clear stack\n");
				printf("	s : print stack\n");
				printf("	r : calculate result of last element on stack\n");
				printf("	obj : tells how many objects are in heap\n");
				printf("	leak : print all leaked objects\n");
				printf("	type a number to add integer to stack\n");
				printf("	f : add float to stack\n");
				printf("	p/pop : pop last element from stack\n");
				printf("	quit : quit cas\n");
				printf("	d/diff : differentiate in terms of last element on stack\n");
				printf("	pi : add pi to stack\n");
				printf("	e : add eulers number to stack\n");
				printf("	v : add variable to stack, ending with . makes the variable constant\n");
				printf("	+ : adds last two stack elements\n");
				printf("	* : multiplies last two stack elements\n");
				printf("	^ : exponentiates last element on stack\n");
				printf("	- : multiplies last element by -1\n");
				printf("	inv : takes the inverse of last element\n");
				printf("	log/ln : takes the natural logarithm of last element\n");
				printf("	swap/> : swaps last two elements\n");
				printf("	dup : duplicates last element\n");
				printf("	roll : rolls stack\n");
				printf("	/ : divide by last element on the stack\n");
				printf("	sqrt : take the square root of last element\n");
				printf("	= : set last two elements on stack equal to each other\n");
				printf("	solve : solve in terms of last element on stack\n");
				printf("	hash : get hash of last element\n");
				printf("	| : take the absolute value of last element\n");
				printf("	] : add last element on stack to list\n");
				printf("	list : make list\n");
				printf("	left : make direction from left\n");
				printf("	right : make direction from right\n");
				printf("	comp : compare if two expressions are equal in structure\n");
				printf("	-inf : add negative infinity to stack\n");
				printf("	inf : add infinity to stack\n");
				printf("	subst : substitute, note that the substitution function is always done first\n");
				printf("	sin : sine of last element on stack\n");
				printf("	cos : cosine of last element on stack\n");
				printf("	int/integrate : integrate last element on stack\n");
				printf("	factor : factor last element on stack\n");
				printf("	distr : distribute last element on stack\n");
				printf("	limit : take the limit of expression\n");
				printf("	i : add imaginary constant to stack\n");
				printf("	asin/arcsin : take the arcsin of last item on stack\n");
				printf("	acos/arccos : take the arccos of last item on stack\n");
				printf("	tan : take the tan of last item on stack\n");
				printf("	atan/arctan : take the tan of last item on stack\n");
				printf("	cbrt : take the cbrt of last item on stack\n");
				printf("	primef : finds the prime factors of an integer\n");
				printf("	Si : take the Sin Integral of last item on stack\n");
				printf("	Ci : take the Cos Integral of last item on stack\n");
				printf("	rbp : read blue print string\n");
				printf("	gbp : generate blue print string\n");
				printf("	break : break apart expression into its parts\n");
				printf("	true : adds true to stack\n");
				printf("	false : adds false to stack\n");
				printf("	and/&& : and last two items on stack\n");
				printf("	or/|| : or last two items on stack\n");
				printf("	not : not last item on stack\n");
				printf("	save : save last element on stack to file\n");
				printf("	save_all : saves all elements on the stack to file\n");
				printf("	load : load last save to stack\n");
				printf("	load_all : loads a stack\n");
				printf("	b : add big integer\n");
				printf("	x : add variable x to stack\n");
				printf("	y : add variable y to stack\n");
			}
			else if(strcmp(op,"s")==0){
				printStack(false);
			}
			else if(strcmp(op,"clear")==0){
				clearStack();
			}
			else if(strcmp(op,"r")==0){
				if(height == 0)	printf("-nothing on stack\n");
				else{
					int64_t start,end;
					start = clock();
					//
					
					{
						
					}
					
					//
					stack[height-1]->simplify();
					
					end = clock();
					printStack();
					printf("-took %lf milli secs to compute\n",(double)(end-start)/1000.0);
				}
			}
			else if(strcmp(op,"obj")==0 || strcmp(op,"o")==0){
				printObjCount();
			}
			else if(strcmp(op,"leak")==0){
				printObjectsInMemory();
			}
			else if((op[0]>='0' && op[0]<='9') || (op[0]=='-'&& op[1]!='\0')){
				int64_t val;
				val = strtoll(op,NULL,0);
				stack[height] = numC(val);
				height++;
				printStack();
			}
			else if(strcmp(op,"f")==0){
				printf("type in floating value\n");
				double val;
				scanf("%lf",&val);
				stack[height] = numFC(val);
				height++;
				printStack();
			}
			else if(strcmp(op,"pop")==0 || strcmp(op,"p")==0){
				if(height!=0){
					delete stack[height-1];
					height--;
					printStack();
				}else{
					printf("-can't delete\n");
				}
			}
			else if(strcmp(op,"quit")==0){
				for(int32_t i = 1;i<height;i++) delete stack[i];
				if(height == 0){
					stack[0] = numC((int64_t)0);
					height++;
				}
				if(objCount != 2 && ERRORS){
					printf("memory leak detected\n");
					printObjCount();
				}
				printf("done. Returning to main Menu\n");
				
				return stack[height-1];
			}
			else if(strcmp(op,"diff")==0 || strcmp(op,"d")==0){
				if(height<1) printf("-nothing on stack\n");
				else{
					stack[height-1] = diffC(stack[height-1]);
					printStack();
				}
			}
			else if(strcmp(op,"pi")==0){
				Expr *out = new Expr(NUM);
				out->value.rep = PIV;
				stack[height] = out;
				height++;
				printStack();
			}
			else if(strcmp(op,"e")==0){
				Expr *out = new Expr(NUM);
				out->value.rep = EV;
				stack[height] = out;
				height++;
				printStack();
			}
			else if(strcmp(op,"v")==0){
				printf("-type in name\n");
				char name[256];
				scanf("%s",name);
				bool startsWithNum = (name[0] > 47 && name[0] < 58) || (name[0] == '.') || (name[0] == '-');
				if(startsWithNum){
					printf("-variable name can't start with number (reserved for system)\n");
				}else{
					stack[height] = varC(name);
					height++;
					printStack();
				}
			}
			else if(strcmp(op,"+")==0){
				if(height<2) printf("-need more elements\n");
				else{
					if(stack[height-2]->exprType == SUM){
						stack[height-2]->addElement(stack[height-1]);
					}else if(stack[height-1]->exprType == SUM){
						stack[height-1]->addElement(stack[height-2]);
						stack[height-2]  = stack[height-1];
					}else{
						stack[height-2] = sumC(stack[height-2],stack[height-1]);
					}
					height--;
					printStack();
				}
			}
			else if(strcmp(op,"*")==0){
				if(height<2) printf("-need more elements\n");
				else{
					if(stack[height-2]->exprType == PROD){
						stack[height-2]->addElement(stack[height-1]);
					}else if(stack[height-1]->exprType == PROD){
						stack[height-1]->addElement(stack[height-2]);
						stack[height-2]  = stack[height-1];
					}else{
						stack[height-2] = prodC(stack[height-2],stack[height-1]);
					}
					height--;
					printStack();
				}
			}
			else if(strcmp(op,"^")==0){
				if(height<2) printf("-need more elements\n");
				else{
					stack[height-2] = powC(stack[height-2],stack[height-1]);
					height--;
					printStack();
				}
			}
			else if(strcmp(op,"-")==0){
				if(height<1) printf("-nothing on stack\n");
				else{
					stack[height-1] = prodC(numC(-1),stack[height-1]);
					printStack();
				}
			}
			else if(strcmp(op,"inv")==0){
				if(height<1) printf("-nothing on stack\n");
				else{
					stack[height-1] = powC(stack[height-1],numC(-1));
					printStack();
				}
			}
			else if(strcmp(op,"log")==0 || strcmp(op,"ln")==0){
				if(height<1) printf("-nothing on stack\n");
				else{
					stack[height-1] = logC(stack[height-1]);
					printStack();
				}
			}else if(strcmp(op,"swap")==0 || strcmp(op,">")==0){
				if(height<2) printf("-need more elements\n");
				else{
					Expr *temp = stack[height-1];
					stack[height-1] = stack[height-2];
					stack[height-2] = temp;
					printStack();
				}
			}else if(strcmp(op,"dup")==0){
				if(height<1) printf("-nothing on stack\n");
				else{
					stack[height] = stack[height-1]->copy();
					height++;
					printStack();
				}
			}
			else if(strcmp(op,"roll")==0){
				if(height<2) printf("-need more elements\n");
				else{
					Expr *og = stack[height-1];
					for(int32_t i = height-1;i >-1 ;i--){
						if(i == 0) stack[i] = og;
						else stack[i] = stack[i-1];
					}
					printStack();
				}
			}else if(strcmp(op,"/")==0){
				if(height<2) printf("-need more elements\n");
				else{
					if(stack[height-2]->exprType == PROD) stack[height-2]->addElement(invC(stack[height-1]));
					else stack[height-2] = prodC(stack[height-2],invC(stack[height-1]));
					height--;
					printStack();
				}
			}else if(strcmp(op,"sqrt")==0){
				if(height<1) printf("-need more elements\n");
				else {
					stack[height-1] = powC(stack[height-1],powC(numC(2L),numC(-1L)));
					printStack();
				}
			}else if(strcmp(op,"=")==0){
				if(height < 2) printf("-need more elements\n");
				else{
					stack[height-2] = equC(stack[height-2],stack[height-1]);
					height--;
					printStack();
				}
			}else if(strcmp(op,"solve")==0){
				if(height < 2) printf("-need more elements\n");
				else{
					stack[height-2] = new Expr(SOLVE,stack[height-2],stack[height-1]);
					height--;
					printStack();
				}
			}else if(strcmp(op,"hash")==0){
				if(height==0) printf("-nothing on stack\n");
				else printf("%lf\n",stack[height-1]->hash());
			}else if(strcmp(op,"|")==0){
				if(height<1) printf("-nothing on stack\n");
				else{
					stack[height-1] = absC(stack[height-1]);
					printStack();
				}
			}else if(strcmp(op,"]")==0){
				if(stack[height-2]){
					if(stack[height-2]->exprType == LIST){
						stack[height-2]->addElement(stack[height-1]);
						height--;
						printStack();
					}else{
						printf("-no list to add to\n");
					}
				}else{
					printf("-need more elements\n");
				}
			}else if(strcmp(op,"list")==0){
				stack[height] = new Expr(LIST);
				height++;
				printStack();
			}else if(strcmp(op,"left")==0){
				if(height<1) printf("-nothing on stack\n");
				else{
					stack[height-1]->direction = LEFT;
					printStack();
				}
			}else if(strcmp(op,"right")==0){
				if(height<1) printf("-nothing on stack\n");
				else{
					stack[height-1]->direction = RIGHT;
					printStack();
				}
			}else if(strcmp(op,"comp")==0){
				if(height < 2) printf("-need more elements\n");
				else {
					if(stack[height-1]->equalStruct(stack[height-2])) printf("true\n");
					else printf("false\n");
				}
			}else if(strcmp(op,"-inf")==0){
				stack[height] = new Expr(NUM);
				stack[height]->value.rep = NEGINF;
				height++;
				printStack();
			}else if(strcmp(op,"inf")==0){
				stack[height] = new Expr(NUM);
				stack[height]->value.rep = INF;
				height++;
				printStack();
			}else if(strcmp(op,"subst")==0){
				if(height < 2) printf("-need more elements\n");
				else{
					stack[height-2] = new Expr(SUBST,stack[height-2],stack[height-1]);
					height--;
					printStack();
				}
			}else if(strcmp(op,"sin")==0){
				if(height < 1) printf("-nothing on stack\n");
				else{
					stack[height-1] = sinC(stack[height-1]);
					printStack();
				}
			}else if(strcmp(op,"cos")==0){
				if(height < 1) printf("-nothing on stack\n");
				else{
					stack[height-1] = cosC(stack[height-1]);
					printStack();
				}
			}else if(strcmp(op,"int")==0 || strcmp(op,"integrate")==0){
				if(height < 1) printf("-nothing on stack\n");
				else{
					stack[height-1] = integC(stack[height-1]);
					printStack();
				}
				
			}else if(strcmp(op,"factor")==0){
				if(height < 1) printf("-nothing on stack\n");
				else{
					if(CLEAR_TERM) system("clear");
					int64_t start,end;
					start = clock();
					stack[height-1]->factor();
					end = clock();
					printStack();
					printf("-took %lf milli secs to compute\n",(double)(end-start)/1000.0);
				}
			}else if(strcmp(op,"distr")==0){
				if(height < 1) printf("-nothing on stack\n");
				else{
					if(CLEAR_TERM) system("clear");
					int64_t start,end;
					start = clock();
					stack[height-1]->distr();
					end = clock();
					printStack();
					printf("-took %lf milli secs to compute\n",(double)(end-start)/1000.0);
				}
			}else if(strcmp(op,"lim")==0){
				
			
			}else if(strcmp(op,"i")==0){
				stack[height] = iC();
				height++;
				printStack();
			}else if(strcmp(op,"asin")==0 || strcmp(op,"arcsin")==0){
				if(height < 1) printf("-nothing on stack\n");
				else{
					stack[height-1] = asinC(stack[height-1]);
					printStack();
				}
			}else if(strcmp(op,"acos")==0 || strcmp(op,"arccos")==0){
				if(height < 1) printf("-nothing on stack\n");
				else{
					stack[height-1] = acosC(stack[height-1]);
					printStack();
				}
			}else if(strcmp(op,"tan")==0){
				if(height < 1) printf("-nothing on stack\n");
				else{
					stack[height-1] = tanC(stack[height-1]);
					printStack();
				}
			}else if(strcmp(op,"atan")==0 || strcmp(op,"arctan")==0){
				if(height < 1) printf("-nothing on stack\n");
				else{
					stack[height-1] = atanC(stack[height-1]);
					printStack();
				}
			}else if(strcmp(op,"cbrt")==0){
				if(height < 1) printf("-nothing on stack\n");
				else{
					stack[height-1] = powC(stack[height-1],invC(numC(3L)));
					printStack();
				}
			}else if(strcmp(op,"primef")==0){
				if(height < 1) printf("-nothing on stack\n");
				else if(!(stack[height-1]->exprType == NUM && stack[height-1]->value.rep == INT && !stack[height-1]->value.neg())){
					printf("-not a positive integer\n");
				}else{
					Expr *prod = primeFactor(stack[height-1]->value.valueI);
					height++;
					stack[height-1] = prod;
					printStack();
				}
			}else if(strcmp(op,"Si")==0){
				if(height < 1) printf("-nothing on stack\n");
				else{
					stack[height-1] = SiC(stack[height-1]);
					printStack();
				}
			}else if(strcmp(op,"Ci")==0){
				if(height < 1) printf("-nothing on stack\n");
				else{
					stack[height-1] = CiC(stack[height-1]);
					printStack();
				}
			}else if(strcmp(op,"rbp")==0){
				char read[maxBluePrintLength] = {0};
				printf("-type in the encoded blue print\n");
				scanf("\n%s",read);
				Str readStr(read);
				Expr *readExpr = readBluePrint(&readStr);
				stack[height] = readExpr;
				height++;
				printStack();
			}else if(strcmp(op,"gbp")==0){
				Str bluePrint;
				stack[height-1]->generateBluePrint(&bluePrint);
				bluePrint.tPrintln();
			}else if(strcmp(op,"break")==0){
				if(height < 1) printf("-nothing on stack\n");
				else{
					Expr *expr = stack[height-1];
					height--;
					for(int32_t i = 0;i<expr->numOfContExpr;i++){
						stack[height] = expr->contExpr[i];
						height++;
					}
					expr->nullify();
					delete expr;
					printStack();
				}
			}else if(strcmp(op,"true")==0){
				stack[height] = boolC(true);
				height++;
				printStack();
			}else if(strcmp(op,"false")==0){
				stack[height] = boolC(false);
				height++;
				printStack();
			}else if(strcmp(op,"and")==0 || strcmp(op,"&&")==0){
				if(height<2) printf("-need more elements\n");
				else{
					if(stack[height-2]->exprType == AND){
						stack[height-2]->addElement(stack[height-1]);
					}else if(stack[height-1]->exprType == AND){
						stack[height-1]->addElement(stack[height-2]);
						stack[height-2]  = stack[height-1];
					}else{
						stack[height-2] = andC(stack[height-2],stack[height-1]);
					}
					height--;
					printStack();
				}
			}else if(strcmp(op,"or")==0 || strcmp(op,"||")==0){
				if(height<2) printf("-need more elements\n");
				else{
					if(stack[height-2]->exprType == OR){
						stack[height-2]->addElement(stack[height-1]);
					}else if(stack[height-1]->exprType == OR){
						stack[height-1]->addElement(stack[height-2]);
						stack[height-2]  = stack[height-1];
					}else{
						stack[height-2] = orC(stack[height-2],stack[height-1]);
					}
					height--;
					printStack();
				}
			}else if(strcmp(op,"!")==0 || strcmp(op,"not")==0){
				if(height<1) printf("-nothing on stack\n");
				else{
					stack[height-1] = notC(stack[height-1]);
					printStack();
				}
			}else if(strcmp(op,"save")==0){
				Str bluePrint;
				stack[height-1]->generateBluePrint(&bluePrint);
				Str fileName;
				printf("type file name\n");
				fileName.tScan();
				fileName.push(fileExtension);
				bluePrint.save(&fileName);
				printStack();
			}else if(strcmp(op,"save_all")==0){
				Str bluePrint;
				Expr *list = new Expr(LIST);
				for(int32_t i = 0;i<height;i++) list->addElement(stack[i]);
				list->generateBluePrint(&bluePrint);
				Str fileName;
				printf("type file name\n");
				fileName.tScan();
				fileName.push(fileExtension);
				bluePrint.save(&fileName);
				printStack();
			}else if(strcmp(op,"load")==0){
				Str bluePrint;
				Str fileName;
				printf("type file name\n");
				fileName.tScan();
				fileName.push(fileExtension);
				bluePrint.load(&fileName);
				if(bluePrint.len != 0){
					stack[height] = readBluePrint(&bluePrint);
					height++;
					printStack();
				}
			}else if(strcmp(op,"load_all")==0){
				Str bluePrint;
				Str fileName;
				printf("type file name\n");
				fileName.tScan();
				fileName.push(fileExtension);
				bluePrint.load(&fileName);
				if(bluePrint.len != 0){
					Expr *list = readBluePrint(&bluePrint);
					for(int32_t i = 0;i<list->numOfContExpr;i++){
						stack[height] = list->contExpr[i];
						height++;
						list->contExpr[i] = nullptr;
					}
					delete list;
					
					printStack();
				}
			}else if(strcmp(op,"b")==0){
				
				Str numStr;
				numStr.tScan();
				
				stack[height] = numC(&numStr);
				height++;
				printStack();
			}else if(strcmp(op,"x")==0 || strcmp(op,"y")==0){
				stack[height] = varC(op);
				height++;
				printStack();
			}
			else{
				printf("-not a command\n");
			}
		}
		
	}
}

namespace simpleTools{
	int32_t iter = 64;
	struct List{
		int32_t n = 0;
		double *e;
		void print(){
			printf("[");
			for(int32_t i = 0;i<n;i++){
				if(e[i] != (int32_t)e[i]) printf("%lf",e[i]);
				else printf("%d",(int32_t)e[i]);
				if(i != n-1) printf(",");
			}
			printf("]\n");
		}
		~List(){
			delete[] e;
		}
	};
	struct Poly{
		List c;
		double out(double x){
			double sum = 0;
			for(int32_t i = 0; i<c.n;i++){
				double prod = 1;
				for(int32_t j = 0;j<i;j++) prod*=x;
				prod*=c.e[i];
				sum+=prod;
			}
			return sum;
		}
		double bisection(double left,double right){
			bool increases = out(left) < 0.0;
			for(int32_t i = 0;i<iter;i++){
				double midPoint = (left+right)/2.0;
				if((out(midPoint) < 0.0)==increases) left = midPoint;
				else right = midPoint;
			}
			return (left+right)/2.0;
		}
		void diff(Poly *p){//get the derivative
			for(int32_t i = 0;i < p->c.n;i++) p->c.e[i] = c.e[i+1]*(i+1);
		}
		void solve(List *res){
			if(c.n == 2) res->e[0] = -c.e[0]/c.e[1];//linear solution
			else{
				//get solution of derivative
				Poly derv;
				derv.c.n = c.n-1;
				derv.c.e = new double[derv.c.n];
				diff(&derv);
				List derSol;
				derSol.n = derv.c.n-1;
				derSol.e = new double[derSol.n];
				derv.solve(&derSol);
				
				int32_t solutionCount = 0;
				//if derivative Solutions count == 0 use newtons method. Example 1/3*x^3+x+2 is special case
				if(derSol.n == 0){
					double guess = 1.0;
					for(int32_t i = 0;i<iter;i++) guess = guess-out(guess)/derv.out(guess);
					res->e[solutionCount] = guess;
					solutionCount++;
				}else{
					//left and rightMost Solutions
					if(out(derSol.e[0]) < 0.0 == (((c.n-1)%2 == 0) == c.e[c.n-1]>0)){//if left solution exists
						double step = 1;
						double leftBound = derSol.e[0]-1;
						while((out(leftBound) < 0.0) == (out(derSol.e[0]) < 0.0)){//finding crossing point
							step*=2;
							leftBound-=step;
						}
						res->e[solutionCount] = bisection(leftBound,derSol.e[0]);
						solutionCount++;
					}
					for(int32_t i = 0; i < derSol.n-1;i++){//middle solutions
						if(( out(derSol.e[i]) < 0.0 ) != ( out(derSol.e[i+1]) < 0.0 )){
							res->e[solutionCount] = bisection(derSol.e[i],derSol.e[i+1]);
							solutionCount++;
						}
					}
					if(out(derSol.e[derSol.n-1]) < 0.0 == (c.e[c.n-1]>0)){//if right solution exists
						double step = 1;
						double rightBound = derSol.e[derSol.n-1]+1;
						while((out(rightBound) < 0.0) == (out(derSol.e[derSol.n-1])< 0.0)){//finding crossing point
							step*=2;
							rightBound+=step;
						}
						res->e[solutionCount] = bisection(derSol.e[derSol.n-1],rightBound);
						solutionCount++;
					}
				}
				if(solutionCount != res->n){
					double *newList = new double[solutionCount];
					for(int32_t i = 0;i<solutionCount;i++) newList[i] = res->e[i];
					delete[] res->e;
					res->e = newList;
					res->n = solutionCount;
				}
			}
		}
		void print(){
			bool printedSomething = false;
			for(int32_t i = 0;i<c.n;i++){
				if(c.e[i] != 0.0){
					if(printedSomething) printf("+");
					if(c.e[i] != 1.0){
						if((int32_t)c.e[i] == c.e[i]) printf("%d",(int32_t)c.e[i]);
						else printf("%lf",c.e[i]);
						if(i>1) printf("*x^%d",i);
						else if(i>0) printf("*x");
					}else{
						if(i>1) printf("x^%d",i);
						else if(i>0) printf("x");
						else if(i==0) printf("1");
					}
					printedSomething = true;
				}
			}
			printf("\n");
		}
	};
	
	void polySolver(){
		printf("type the degree of the polynomial\n");
		int32_t deg;
		while(true){
			scanf("\n%d",&deg);
			if(deg>0) break;
			else printf("please enter valid degree\n");
		}
		Poly p;
		p.c.n = deg+1;
		p.c.e = new double[p.c.n];
		printf("type in all the coefficients from lowest degree to highest\n");
		for(int32_t i = 0;i<p.c.n;i++){
			double d;
			scanf("\n%lf",&d);
			p.c.e[i] = d;
		}
		p.print();
		List l;
		l.n = deg;
		l.e = new double[deg];
		p.solve(&l);
		printf("x=");
		l.print();
		printf("\n");
	}
}

#pragma pack(pop)

void topicHelp(){
	if(CLEAR_TERM) system("clear");
	printf("topics ...\n");
	printf("algebra\n");
	
	char topicName[16];
	scanf("\n%s",topicName);
	if(strcmp(topicName, "algebra")==0){
		
	}
}

void toolSelect(){
	
	while(true){
		printf("to start computer algebra system type \"cas\"\n");
		printf("to start polynomial solver type \"poly\"\n");
		printf("general math topic help \"helpme\"\n");
		printf("to get information on the project type \"about\"\n");
		printf("to quit type \"quit\"\n");
		char op[16];
		scanf("\n%s",op);
		if(CLEAR_TERM) system("clear");
		if(strcmp(op,"cas")==0){
			using namespace microCas;
			Expr *expr = rpnCas();
			expr->tPrintln();
			delete expr;
		}else if(strcmp(op,"poly")==0){
			simpleTools::polySolver();
		}else if(strcmp(op,"quit")==0){
			break;
		}else if(strcmp(op,"about")==0){
			printf("\n");
			printf("Project designed by Benjamin Robert Currie\n");
			printf("Started in 2020 during COVID-19 pandemic\n");
			printf("Math people that gave me the information...\n");
			printf("Ms.Llorin - Calculus teacher\n");
			printf("Mr.Fannon - Pre Calculus teacher\n");
			printf("Mr.Bigsby - Algebra Teacher\n");
			printf("Black Pen Red Pen - great calculus and algebra example problems\n");
			printf("My email: seegold123a@gmail.com\n");
			printf("\n");
		}else if(strcmp(op,"helpme")==0){
			topicHelp();
		}else{
			printf("try again ...\n");
		}
	}
	printf("closing...\n");
}


int main(){
	microCas::Str *settings = new microCas::Str();
	microCas::Str *fileName = new microCas::Str("settings.blogic");
	bool error = settings->load(fileName);
	if(error){//if the file does not exist, generate a file
		settings->push("clear_terminal:Y,show_steps:N");
		settings->save(fileName);
	}
	
	if(settings->chars[15] == 'Y') CLEAR_TERM = true;
	if(settings->chars[28] == 'Y') microCas::SHOW_ALL_STEPS = true;
	
	delete settings;
	delete fileName;
	
	if(CLEAR_TERM) system("clear");
	if(error) printf("generated settings file\n");
	printf("Created By Benjamin Currie @2020-2021 version 0.2\n");
	printf("running on a %ld bit system\n",sizeof(void*)*8);
	toolSelect();
	return 0;
	
}


