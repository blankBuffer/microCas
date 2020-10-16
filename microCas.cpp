#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <limits.h>
#include <string.h>

//micro cas
/*
Created By Ben Currie;
My goal was to make a cas in under 5000 lines of code
to show that all of highschools math can be decribed in ONE file

*/
#pragma pack(push, 1)
namespace microCas{
	const bool ERRORS = false;// errors shows illegal memory accesses and memory leaks
	
	int objCount = 0;
	const short maxNameLength = 8;
	
	void freeO(void *p){
		objCount--;
		free(p);
	}
	void *mallocO(int bytes){
		objCount++;
		return malloc(bytes);
	}
	void *reallocO(void *p,int bytes){
		return realloc(p,bytes);
	}
	
	void printObjCount(){
		printf("objects in Memory: %d\n",objCount);//prints all objects in heap created. not full proof.
	}
	
	long int gcd(long int a, long int b){
    	long int c;
		while (b != 0) {
	 		c = a % b;
	 		a = b;
	 		b = c;
	 	}
		return a;
	}
	
	void perfectPower(long int num,long int *retBase,long int *retExpo){
		
		bool found = false;
		int max = log2(num)+1;
		
		if(max > 31){
			long double ldNum = (long double)num;
			for(int i = 2;i<=max;i++){
				long int n = lroundl(powl(ldNum,1.0L/i));
				if(powl(n,i) == ldNum){
					(*retExpo) = i;
					(*retBase) = n;
					found = true;
				}
			}
		}else{
			double dNum = (double)num;
			for(int i = 2;i<=max;i++){
				long int n = lround(pow(dNum,1.0/i));
				if(pow(n,i) == dNum){
					(*retExpo) = i;
					(*retBase) = n;
					found = true;
				}
			}
		}
		if(!found){
			(*retBase) = num;
			(*retExpo) = 1;
		}
	}
	const char INT = 0,FLOAT = 1, PIV = 2,EV = 3,INF = 4,NEGINF = 5,UNDEF = 6;//rep
	
	struct Num{//meant to be used on stack
		long int valueI;
		double valueF;
		char rep;
		
		void setValueI(long int v){
			rep = INT;
			valueI = v;
		}
		void setValueF(double v){
			rep = FLOAT;
			valueF = v;
		}
		void setValue(Num *other){
			rep = other->rep;
			if(rep == FLOAT) valueF = other->valueF;
			else if(rep == INT) valueI = other->valueI;
		}
		
		Num(int r){
			rep = r;
		}
		Num(long int v){
			setValueI(v);
		}
		Num(double v){
			setValueF(v);
		}
		Num(Num *num){
			setValue(num);
		}
		Num(){
		
		}
		bool plain(){//not a transedental number
			return rep == INT || rep == FLOAT;
		}
		void convertToFloat(){
			if(rep == INT){
				rep = FLOAT;
				valueF = valueI;
			}else if(rep == PIV){
				rep = FLOAT;
				valueF = M_PI;
			}else if(rep == EV){
				rep = FLOAT;
				valueF = M_E;
			}
		}
		void addN(Num *other){
			if(rep == INT && other->rep == INT){
				double backup = (double)valueI+(double)other->valueI;//checking for over or under flow
				if(backup>LONG_MAX||backup<LONG_MIN) rep = FLOAT;
				valueF = backup;
				valueI+=other->valueI;
				if((valueI < 0) != (backup < 0)) rep = FLOAT;
			}else{
				convertToFloat();
				Num cpy(other);
				cpy.convertToFloat();
				valueF+=cpy.valueF;
			}
		}
		void multN(Num *other){
			if(rep == INT && other->rep == INT){
				double backup = (double)valueI*(double)other->valueI;//checking for over or under flow
				if(backup>LONG_MAX||backup<LONG_MIN) rep = FLOAT;
				valueF = backup;
				valueI*=other->valueI;
			}else{
				convertToFloat();
				Num cpy(other);
				cpy.convertToFloat();
				valueF*=cpy.valueF;
			}
		}
		void divN(Num *other){
			if(rep == INT && other->rep == INT){
				valueI/=other->valueI;
			}else{
				convertToFloat();
				Num cpy(other);
				cpy.convertToFloat();
				valueF/=cpy.valueF;
			}
		}
		void powN(Num *other){
			if(rep == INT && other->rep == INT){
				double backup = pow((double)valueI,(double)other->valueI);//checking for over or under flow
				if(backup>LONG_MAX||backup<LONG_MIN) rep = FLOAT;
				valueF = backup;
				valueI = pow(valueI,other->valueI);
			}else{
				convertToFloat();
				Num cpy(other);
				cpy.convertToFloat();
				valueF=pow(valueF,cpy.valueF);
			}
		}
		bool neg(){
			if(rep == FLOAT) return valueF < 0.0;
			else if(rep == INT) return valueI < 0;
			else if(rep == NEGINF) return true;
			return false;
		}
		void flipSign(){
			valueI = -valueI;
			valueF = -valueF;
		}
		void absN(){
			valueI = labs(valueI);
			valueF = fabs(valueF);
			if(rep == NEGINF) rep = INF;
		}
		bool equals(Num *other){
			if(rep == INT && other->rep == INT){
				return valueI == other->valueI;
			}else if(other->rep == FLOAT){
				Num cpy(this);
				cpy.convertToFloat();
				return cpy.valueF == other->valueF;
			}
			if(other->rep == rep) return true;
			return false;
		}
		
		bool equalsI(long int n){
			if(rep == INT) return valueI == n;
			else if(rep == FLOAT) return valueF == (double)n;
			return false;
		}
		void print(){
			if(rep == INT){
				printf("%ld",valueI);
				return;
			}else if(rep == FLOAT){
				printf("%lf",valueF);
				return;
			}else if(rep == PIV){
				printf("π");
				return;
			}else if(rep == EV){
				printf("e");
				return;
			}else if(rep == INF){
				printf("∞");
				return;
			}else if(rep == NEGINF){
				printf("-∞");
				return;
			}else if(rep == UNDEF){
				printf("undef");
				return;
			}
			if( ERRORS) printf("error while printing num");
		}
		void println(){
			print();
			printf("\n");
		}
	};
	const char SUM = 0,PROD = 1,POW = 2,NUM = 3,VAR = 4,LOG = 5,DERI = 6,EQU = 7,ABS = 8,LIST = 9,SOLVE = 10,LIMIT = 11,SUBST = 12,INTEG = 13,SIN = 14,COS = 15;//expr types
	const char MIDDLE = 0,LEFT = 1,RIGHT = 2,INDET = 3;//direction
	//WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW"Expr"W
	struct Expr{//meant to be used on heap
		char exprType;
		short numOfContExpr = 0;
		char name[maxNameLength];
		Expr **contExpr = nullptr;
		Num value;
		bool simple = false;
		char direction = MIDDLE;
		
		//init
		void init(int type){
			exprType = type;
			objCount++;
		}
		Expr(int type){
			init(type);
		}
		Expr(Num *val){
			init(NUM);
			value.setValue(val);
		}
		Expr(double val){
			init(NUM);
			value.setValueF(val);
		}
		Expr(long int val){
			init(NUM);
			value.setValueI(val);
		}
		void setName(const char *name){
			for(int i = 0;i<maxNameLength;i++) this->name[i] = name[i];
		}
		Expr(const char *name){
			init(VAR);
			setName(name);
		}
		Expr(int type,Expr *first,Expr *second){
			init(type);
			contExpr = (Expr**)mallocO(2*sizeof(Expr));
			contExpr[0] = first;
			contExpr[1] = second;
			numOfContExpr = 2;
		}
		Expr(int type,Expr *expr){
			init(type);
			contExpr = (Expr**)mallocO(sizeof(Expr));
			contExpr[0] = expr;
			numOfContExpr = 1;
		}
		//easy programming
		Expr *getBase(){
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
		bool expoIsMinusOne(){
			if(ERRORS) if(exprType != POW) printf("not correct object type\n");
			if(getExpo()->exprType == NUM){
				return getExpo()->value.equalsI(-1L);
			}
			return false;
		}
		//
		void print(){
			if(direction == INDET){
				printf("indet!");
				return;
			}
			if(direction != MIDDLE) printf("(");
			if(exprType == INTEG){
				printf("integral(");
				contExpr[0]->print();
				printf(")");
			}else if(exprType == COS){
				printf("cos(");
				contExpr[0]->print();
				printf(")");
			}else if(exprType == SIN){
				printf("sin(");
				contExpr[0]->print();
				printf(")");
			}else if(exprType == SOLVE){
				printf("solve(");
				contExpr[0]->print();
				printf(",");
				contExpr[1]->print();
				printf(")");
			}else if(exprType == EQU){
				contExpr[0]->print();
				printf("=");
				contExpr[1]->print();
			}else if(exprType == NUM){
				value.print();
			}else if(exprType == VAR){
			
				printf("%s",name);
				
			}else if(exprType == POW){
				bool special = false;
				if(expoIsMinusOne()){
					printf("inv(");
					if(getBase()) getBase()->print();
					printf(")");
					special = true;
				}
				if(!special && getExpo()->exprType == POW){
					Expr *pw = getExpo();
					if(pw->getBase()->exprType == NUM && pw->getExpo()->exprType == NUM){
						if(pw->expoIsMinusOne()){
							if(pw->getBase()->value.equalsI(2L)){
								printf("sqrt(");
								if(getBase()) getBase()->print();
								printf(")");
								special = true;
							}else if(pw->getBase()->value.equalsI(3L)){
								printf("cbrt(");
								if(getBase()) getBase()->print();
								printf(")");
								special = true;
							}
						}
					}
				}
				if(!special){
					bool usePInBase = false,usePInExpo = false;
					Expr *b = getBase();
					int tyB = b->exprType,tyE = getExpo()->exprType;
					if((tyB == PROD || tyB == SUM || tyB == POW || tyB == EQU) ||  (tyB == NUM && b->value.neg()) ) usePInBase = true;
					if(usePInBase) printf("(");
					if(getBase()) getBase()->print();
					if(usePInBase) printf(")");
					printf("^");
					if(tyE == SUM || tyE == PROD || tyE == POW || tyE == EQU) usePInExpo = true;
					if(usePInExpo) printf("(");
					if(getExpo()) getExpo()->print();
					if(usePInExpo) printf(")");
				}
				
			}else if(exprType == SUM){
				if(numOfContExpr < 2) printf("alone sum:");
				for(int i = 0;i < numOfContExpr;i++){
					bool pr = contExpr[i]->exprType == SUM || contExpr[i]->exprType == EQU;
					if(pr) printf("(");
					if(contExpr[i]) contExpr[i]->print();
					if(pr) printf(")");
					if(i!=numOfContExpr-1) printf("+");
				}
				
			}else if(exprType == PROD){
				if(numOfContExpr < 2) printf("alone product:");
				for(int i = 0;i < numOfContExpr;i++){
					bool pr = false;
					
					if(contExpr[i]->exprType == PROD || contExpr[i]->exprType == SUM || contExpr[i]->exprType == EQU) pr = true;
					
					if(pr) printf("(");
					if(contExpr[i]) contExpr[i]->print();
					if(pr) printf(")");
					if(i!=numOfContExpr-1) printf("*");
				}
			}else if(exprType == LOG){
				printf("log(");
				if(contExpr[0]) contExpr[0]->print();
				printf(")");
			}else if(exprType == DERI){
				printf("d(");
				if(contExpr[0]) contExpr[0]->print();
				printf(")");
			}else if(exprType == ABS){
				printf("abs(");
				if(contExpr[0]) contExpr[0]->print();
				printf(")");
			}else if(exprType == LIST){
				printf("[");
				for(int i = 0;i<numOfContExpr;i++){
					if(contExpr[i]) contExpr[i]->print();
					if(i != numOfContExpr-1) printf(",");
				}
				printf("]");
			}else if(exprType == SUBST){
				printf("subst(");
				if(contExpr[0]) contExpr[0]->print();
				printf(",");
				if(contExpr[1]) contExpr[1]->print();
				printf(")");
			}
			
			if(direction == LEFT){
				printf(":-ε)");
			}else if(direction == RIGHT){
				printf(":+ε)");
			}
		}
		void println(){
			print();
			printf("\n");
		}
		
		void addElement(Expr *expr){
			simple = false;
			numOfContExpr++;
			if(numOfContExpr == 1) contExpr = (Expr**)mallocO(sizeof(Expr));
			else contExpr = (Expr**)reallocO(contExpr,sizeof(Expr)*numOfContExpr);
			contExpr[numOfContExpr-1] = expr;
		}
		
		void removeElement(int index){
			simple = false;
			if(contExpr[index]){
				delete contExpr[index];
				contExpr[index] = nullptr;
			}
			int index2 = 0;
			for(int i = 0;i<numOfContExpr;i++){
				if(i!=index){
					contExpr[index2] = contExpr[i];
					index2++;
				}
			}
			numOfContExpr--;
			if(numOfContExpr == 0){
				freeO(contExpr);
				contExpr = nullptr;
			}else contExpr = (Expr**)reallocO(contExpr,sizeof(Expr)*numOfContExpr);
		}
		
		void clearElements(){
			simple = false;
			if(!contExpr) return;
			for(int i = 0;i<numOfContExpr;i++){
				if(contExpr[i]) delete contExpr[i];
			}
			freeO(contExpr);
			contExpr = nullptr;
			numOfContExpr = 0;
		}
		
		Expr *copy(){
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
			}else if(exprType == SUM || exprType == PROD || exprType == POW || exprType == LOG || DERI){
				
				Expr *expr = new Expr(exprType);
				expr->numOfContExpr = numOfContExpr;
				expr->contExpr = (Expr**)mallocO(sizeof(Expr)*numOfContExpr);
				for(int i = 0;i<numOfContExpr;i++){
					expr->contExpr[i] = contExpr[i]->copy();
				}
				expr->simple = simple;
				expr->direction = direction;
				return expr;
			}
			
			if(ERRORS) printf("can't copy object: missing function\n");
			return nullptr;
		}
		double hash(){
			//warning: incomplete :list	
			double multiplier = 1.0;
			if(direction == LEFT) multiplier=0.99;
			else if(direction == RIGHT) multiplier = 0.98;
			if(exprType == NUM){
				Num cpy(value);
				cpy.convertToFloat();
				double v = cpy.valueF;
				double a = 1+fabs(v);
				return (0.5*((a+v)/a))*multiplier;
			}else if(exprType == VAR){
				double v = 0.5;
				int i = 0;
				while(name[i] != '\0'){
					v = pow(1.0/name[i],v);
					i++;
				}
				return v*multiplier;
			}else if(exprType == SUM){
				double v = 0.0;
				for(int i = 0; i < numOfContExpr;i++){
					v+=contExpr[i]->hash();
				}
				return (v-floor(v))*multiplier;
			}else if(exprType == PROD){
				double v = 0.0;
				for(int i = 0; i < numOfContExpr;i++){
					v+=contExpr[i]->hash();
				}
				return pow((v-floor(v)),0.7324)*multiplier;
			}else if(exprType == LIST){
				double v = 0.0;
				for(int i = 0; i < numOfContExpr;i++){
					v+=contExpr[i]->hash();
				}
				return pow((v-floor(v)),0.987423)*multiplier;
			}else{
				double v = 0.7423;
				for(int i = 0;i< numOfContExpr;i++) v = pow(contExpr[i]->hash()/exprType,v);
				return v*multiplier;
			}
		}
		bool equalStruct(Expr *other){
			if(other->exprType != exprType) return false;
			
			if(exprType == VAR){
				if( strcmp(name, other->name) == 0) return true;
				else return false;
			}else if(exprType == NUM){
				return value.equals(&other->value);
			}else if(exprType == POW ||exprType == SOLVE){
				return contExpr[0]->equalStruct(other->contExpr[0]) && contExpr[1]->equalStruct(other->contExpr[1]);
			}else if(exprType == LOG || exprType == ABS || exprType == DERI || exprType == SIN || exprType == COS || exprType == INTEG){
				return contExpr[0]->equalStruct(other->contExpr[0]);
			}else if(exprType == SUM || exprType == PROD || exprType == LIST || exprType == EQU){
				if(numOfContExpr != other->numOfContExpr) return false;
				bool *used = (bool*)mallocO(numOfContExpr*sizeof(bool));
				for(int m = 0;m < numOfContExpr;m++) used[m] = false;
				for(int i = 0;i < numOfContExpr;i++){
					bool found = false;
					
					for(int j = 0;j < numOfContExpr;j++){
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
		
		bool constant(){
			if(exprType == NUM) return true;
			else if(exprType == VAR){
				int indexOfEnd;
				for(int i = 0;i<maxNameLength;i++){
					if(name[i] == '\0'){
						indexOfEnd = i;
						break;
					}
				}
				if(name[indexOfEnd-1] == '.'){
					return true;
				}
			}
			if(contExpr){
				for(int i = 0;i< numOfContExpr;i++){
					if(!contExpr[i]->constant()) return false;
				}
				return true;
			}
			return false;
		}
		bool contains(Expr *var){
			if(equalStruct(var)) return true;
			else for(int i = 0;i < numOfContExpr;i++) if(contExpr[i]->contains(var)) return true;
			
			return false;
		}
		bool containsType(int exprT){
			if(exprType == exprT) return true;
			else{
				for(int i = 0;i<numOfContExpr;i++){
					if(contExpr[i]->containsType(exprT)){
						return true;
					}
				}
			}
			return false;
		}
		bool containsVars(){
			if(exprType == VAR) return true;
			else for(int i = 0;i < numOfContExpr;i++) if(contExpr[i]->containsVars()) return true;
			
			return false;
		}
		void nullify(){
			for (int i = 0;i<numOfContExpr;i++) contExpr[i] = nullptr;
		}
		void becomeInternal(Expr *other){//become an object within this object
			Expr *copyOfOther = other->copy();
			clearElements();
			simple = copyOfOther->simple;
			setName(copyOfOther->name);
			contExpr = copyOfOther->contExpr;
			if(direction == MIDDLE) direction = copyOfOther->direction;
			numOfContExpr = copyOfOther->numOfContExpr;
			exprType = copyOfOther->exprType;
			value.setValue(&copyOfOther->value);
			copyOfOther->contExpr = nullptr;
			delete copyOfOther;
		}
		void become(Expr *other){
			exprType = other->exprType;
			numOfContExpr = other->numOfContExpr;
			other->numOfContExpr = 0;
			simple = other->simple;
			direction = other->direction;
			setName(other->name);
			contExpr = other->contExpr;
			other->contExpr = nullptr;
			value.setValue(&other->value);
			delete other;
		}
		int nestDepth(){
			if(numOfContExpr == 0){
				return 1;
			}
			int max = 0;
			for(int i = 0;i < numOfContExpr;i++){
				int depth = contExpr[i]->nestDepth();
				if(depth>max){
					max = depth;
				}
			}
			return max+1;
		}
		//deletion
		~Expr(){
			clearElements();
			objCount--;
		}
		
		void distr();
		void factor();
		
		//simplify algarithms
		void simplify();
		void simplify(bool addFractions);//simplify with flags
		void derivSimp();
		void powSimp();
		void sumSimp(bool addFractions);
		void prodSimp();
		void logSimp();
		void solverSimp();
		void absSimp();
		void listSimp();
		void equSimp();
		void sinSimp();
		void cosSimp();
		void integSimp(bool usub);
		void integSimp(){
			integSimp(true);
		}
		//substitution
		void replace(Expr* old,Expr *repl){
			if(contains(old)){
				simple = false;
				if(equalStruct(old)){
					clearElements();
					become(repl->copy());
				}else{
					for(int i = 0;i<numOfContExpr;i++){
						contExpr[i]->replace(old,repl);
					}
				}
			}
		}
	};
	
	//easy programming
	Expr *varC(const char *name){
		return new Expr(name);
	}
	Expr *numC(long int val){
		return new Expr(val);
	}
	Expr *numFC(double val){
		return new Expr(val);
	}
	Expr *invC(Expr *expr){
		return new Expr(POW,expr,new Expr(-1L));
	}
	Expr *logC(Expr *expr){
		return new Expr(LOG,expr);
	}
	Expr *sumC(Expr *expr1,Expr *expr2){
		return new Expr(SUM,expr1,expr2);
	}
	Expr *prodC(Expr *expr1,Expr *expr2){
		return new Expr(PROD,expr1,expr2);
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
	Expr *primeFactor(long int num){
		Expr *pr = new Expr(PROD);
		
		long int max = sqrt(num)+1;
		for(long int i = 2;i < max;i++){
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
	
	//WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW"derivSimp"W
	void Expr::derivSimp(){
		if(exprType == DERI){
			//
			if(contExpr[0]->constant()){
				clearElements();
				exprType = NUM;
				value.setValueI(0L);
			}else if(contExpr[0]->exprType == POW){
				addElement(copy());
				exprType = PROD;
				contExpr[1]->contExpr[0]->exprType = PROD;
				contExpr[1]->contExpr[0]->contExpr[0] = logC(contExpr[1]->contExpr[0]->contExpr[0]);
				simplify();
			}else if(contExpr[0]->exprType == LOG){
				addElement( diffC(contExpr[0]->contExpr[0]->copy()));
				contExpr[0]->exprType = POW;
				contExpr[0]->addElement(numC(-1L));
				exprType = PROD;
				simple = false;
				simplify();
			}else if(contExpr[0]->exprType == SUM){
				becomeInternal(contExpr[0]);
				for(int i = 0;i < numOfContExpr;i++) contExpr[i] = diffC(contExpr[i]);
				simple = false;
				simplify();
			}else if(contExpr[0]->exprType == PROD){
				Expr *prod = contExpr[0];
				
				Expr *sum = new Expr(SUM);
				for(int i = 0;i<prod->numOfContExpr;i++){
					Expr *prodCopy = prod->copy();
					prodCopy->simple = false;
					prodCopy->contExpr[i] = diffC(prodCopy->contExpr[i]);
					sum->addElement(prodCopy);
				}
				clearElements();
				become(sum);
				simplify();
			}else if(contExpr[0]->exprType == ABS){
				Expr *pr = new Expr(PROD);
				pr->addElement(contExpr[0]->contExpr[0]->copy());
				pr->addElement(invC(contExpr[0]));
				pr->addElement(diffC(contExpr[0]->contExpr[0]->copy()));
				nullify();
				clearElements();
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
				for(int i = 0;i<numOfContExpr;i++){
					contExpr[i] = diffC(contExpr[i]);
				}
				simple = false;
				simplify();
			}else if(contExpr[0]->exprType == SIN){
				Expr *repl = prodC(cosC(contExpr[0]->contExpr[0]->copy()),diffC(contExpr[0]->contExpr[0]->copy()));
				clearElements();
				become(repl);
				simplify();
				return;
			}else if(contExpr[0]->exprType == COS){
				Expr *repl = prodC(sinC(contExpr[0]->contExpr[0]->copy()),diffC(contExpr[0]->contExpr[0]->copy()));
				repl->addElement(numC(-1L));
				clearElements();
				become(repl);
				simplify();
				return;
			}else if(contExpr[0]->exprType == INTEG){
				becomeInternal(contExpr[0]->contExpr[0]);
				return;
			}
		}
	}
	
	void Expr::powSimp(){
		if(exprType == POW){
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
					for(int i = 0;i<list->numOfContExpr;i++){
						list->contExpr[i] = powC(list->contExpr[i],getExpo()->copy());
					}
					
					becomeInternal(list);
					simple = false;
					simplify();
					return;
				}else if(getExpo()->exprType == LIST){
					Expr *list = getExpo();
					for(int i = 0;i<list->numOfContExpr;i++){
						list->contExpr[i] = powC(getBase()->copy(),list->contExpr[i]);
					}
					
					becomeInternal(list);
					simple = false;
					simplify();
					return;
				}
				
			}
			
			if(getExpo()->exprType == NUM && getExpo()->value.equalsI(2L) && getBase()->exprType == SIN){//sin(x)^2 = 1-cos(x)^2
				getBase()->exprType = COS;
				Expr *repl = sumC(numC(1L),prodC(numC(-1L),copy()));
				clearElements();
				become(repl);
				simplify();
				return;
			}
			
			if(constant()){//(a+b)^n where n is integer -> (a+b)*(a+b)... n times, base must be sum and constant
				if(getBase()->exprType == SUM && getExpo()->exprType == NUM && getExpo()->value.rep == INT){
					long int n = getExpo()->value.valueI;
					if(n > 1 && n < 16){
						removeElement(1);
						exprType = PROD;
						for(long i = 0;i < n-1;i++){
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
				if(c->exprType == POW){
					if(c->expoIsMinusOne() && c->getBase()->exprType == NUM && c->getBase()->value.rep == INT){
						if(c->getBase()->value.valueI % 2L == 0){
							evenInverseExpo = true;
						}
					}
				}else if(c->exprType == PROD){
					for(int i = 0;i<c->numOfContExpr;i++){
						if(c->contExpr[i]->exprType == POW){
							Expr *pw = c->contExpr[i];
							if(pw->expoIsMinusOne() && pw->getBase()->exprType == NUM && pw->getBase()->value.rep == INT){
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
					for(int i = 0; i < pr->numOfContExpr;i++){
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
					for(int i = 0;i<pr->numOfContExpr;i++){
						if(pr->contExpr[i]->exprType == NUM && pr->contExpr[i]->value.rep == INT){
							if(abs(pr->contExpr[i]->value.valueI) %2L == 0){
								getBase()->becomeInternal(getBase()->contExpr[0]);
								break;
							}else{
								Expr *num = pr->contExpr[i];
								pr->contExpr[i] = nullptr;
								pr->removeElement(i);
								
								Expr *repl = powC(powC(getBase(),num),pr);
								nullify();
								clearElements();
								become(repl);
								simplify();
								return;
							}
						}
					}
				}else if(getExpo()->exprType == NUM && getExpo()->value.rep == INT){
					if(abs(getExpo()->value.valueI) %2L == 0){
						getBase()->becomeInternal(getBase()->contExpr[0]);
					}else if(abs(getExpo()->value.valueI) %2L == 1){
						getExpo()->value.valueI--;
						Expr *repl = prodC(getBase(),powC(getBase()->contExpr[0]->copy(),getExpo()));
						nullify();
						clearElements();
						become(repl);
						simplify();
						return;
					}
				}
			}
			
			{//factor base and distr expo
				getBase()->factor();
				getExpo()->distr();
			}
			
			{//(x*y)^a -> x^a*y^a
				if(getBase()->exprType == PROD){
					Expr *replace = new Expr(PROD);
					Expr *pr = getBase();
					for(int i = 0;i<pr->numOfContExpr;i++){
						Expr *pow = powC(pr->contExpr[i],getExpo()->copy());
						replace->addElement(pow);
					}
					pr->nullify();
					clearElements();
					exprType = PROD;
					contExpr = replace->contExpr;
					replace->contExpr = nullptr;
					numOfContExpr = replace->numOfContExpr;
					replace->numOfContExpr = 0;
					delete replace;
					simple = false;
					simplify();
					return;
				}
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
					long b,e;
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
					for(int i = 0;i < getExpo()->numOfContExpr;i++){
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
			
			if(getExpo()->exprType == NUM && getExpo()->value.rep == INT){//-infinity^(-even number) -> 0+epsilon && -infinity^(-odd number) -> 0-epsilon
				if(getBase()->exprType == NUM && getBase()->value.rep == NEGINF){
					if(getExpo()->value.valueI%2L == 0){
						direction = RIGHT;
					}else{
						direction = LEFT;
					}
					exprType = NUM;
					value.setValueI(0L);
					clearElements();
					return;
				}
			}
			
			if(getBase()->value.rep != EV) {//a^b -> e^(ln(a)*b)
				bool change = false;
				if((getBase()->exprType == NUM && getBase()->value.rep == INF) || (getExpo()->exprType == NUM && getExpo()->value.rep == INF)){
					change = true;
				}
				if(getExpo()->exprType == POW){
					Expr *pw = getExpo();
					if(pw->getBase()->exprType == LOG) change = true;
				}else if(getExpo()->exprType == PROD){
					Expr *pr = getExpo();
					for(int i = 0;i < pr->numOfContExpr;i++){
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
			
			if(getBase()->exprType == NUM){//e^(ln(a)*b) -> a^b && e^ln(x) -> x && e^(ln(a)+b) -> a*e^b && e^(ln(a)*b+c) -> a^b*e^c
				if(getBase()->value.rep == EV){
					if(getExpo()->exprType == LOG){
						becomeInternal(getExpo()->contExpr[0]);
						return;
					}else if(getExpo()->exprType == PROD){
						Expr *pr = getExpo();
						int count = 0;
						int indexOfLog;
						for(int i = 0;i < pr->numOfContExpr;i++){
							if(pr->contExpr[i]->exprType == LOG){
								count++;
								indexOfLog = i;
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
						for(int i = 0; i< sm->numOfContExpr;i++){
							if(sm->contExpr[i]->exprType == LOG){
								prOut->addElement(sm->contExpr[i]->contExpr[0]);
								sm->contExpr[i]->contExpr[0] = nullptr;
								sm->removeElement(i);
								i--;
							}else if(sm->contExpr[i]->exprType == PROD){
								int count = 0;
								Expr *pr = sm->contExpr[i];
								int indexOfLog;
								for(int j = 0;j < pr->numOfContExpr;j++){
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
							clearElements();
							prOut->simplify();
							become(prOut);
							return;
						}else{
							delete prOut;
						}
						
					}
					
				}
			}
			if(getBase()->exprType == NUM && getBase()->value.rep == INT && getExpo()->exprType == PROD){//c^(5/2) -> c^(2+1/2) convert to mixed fraction
				Expr *pr = getExpo();
				if(pr->numOfContExpr == 2){
					Expr *num = nullptr;
					Expr *den = nullptr;
					for(int i = 0;i < 2;i++){
						if(pr->contExpr[i]->exprType == NUM && pr->contExpr[i]->value.rep == INT){
							num = pr->contExpr[i];
						}else if(pr->contExpr[i]->exprType == POW){
							Expr *pw = pr->contExpr[i];
							if(pw->expoIsMinusOne() && pw->getBase()->exprType == NUM && pw->getBase()->value.rep == INT){
								den = pr->contExpr[i];
							}
						}
					}
					if(num && den){
						long int n,d,v;
						bool neg = false;
						
						n = num->value.valueI;
						d = den->getBase()->value.valueI;
						if(abs(n)>abs(d)){
							if(n<0) {
								neg = true;
								n = -n;
							}
							v = n/d;
							n = n-v*d;
							
							if(neg){
								n = -n;
								v = -v;
							}
							delete getExpo();
							setExpo(sumC(numC(v),prodC(numC(n),invC(numC(d)))));
						}
					}
				}
			}
			if(getExpo()->exprType == NUM){//x^1 -> x && x^0 -> 1 
				
				if(getExpo()->value.equalsI(0L)){//0^0 indeterminate form
					if(getBase()->exprType == NUM && getBase()->value.equalsI(0L)){
						direction = INDET;
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
				for(int i = 0;i<sm->numOfContExpr;i++){
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
						
						clearElements();
						become(pr);
					}else{
						n.absN();
						
						Num bs(&getBase()->value);
						bs.powN(&n);
						Expr *pr = prodC(copy(),invC(new Expr(&bs)));
						
						clearElements();
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
					long int invVal;
					if(getExpo()->exprType == POW){
						Expr *pw = getExpo();
						if(pw->getBase()->exprType == NUM && pw->getBase()->value.rep == INT && pw->expoIsMinusOne()){
							invVal = pw->getBase()->value.valueI;
							factorBase = true;
						}
					}else if(getExpo()->exprType == PROD){
						Expr *pr = getExpo();
						for(int i = 0;i < pr->numOfContExpr;i++){
							if(pr->contExpr[i]->exprType == POW){
								Expr *pw = pr->contExpr[i];
								if(pw->getBase()->exprType == NUM && pw->getBase()->value.rep == INT && pw->expoIsMinusOne()){
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
							for(int i = 0;i < pr->numOfContExpr;i++){
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
			{//rationalize the denominator x^(-y) -> x^(1-y)/x where x and y are constants and y is fraction
				if(getBase()->exprType == NUM && getBase()->value.rep == INT){
					bool rat = false;
					if(getExpo()->exprType == POW){
						Expr *pw = getExpo();
						if(pw->expoIsMinusOne() && pw->getBase()->exprType == NUM && pw->getBase()->value.rep == INT && pw->getBase()->value.neg()){
							rat = true;
						}
					}
					else if(getExpo()->exprType == PROD){
						Expr *pr = getExpo();
						if(pr->numOfContExpr == 2){
							Expr *num = nullptr;
							Expr *den = nullptr;
							for(int i = 0;i < 2;i++){
								if(pr->contExpr[i]->exprType == NUM && pr->contExpr[i]->value.rep == INT){
									num = pr->contExpr[i];
								}else if(pr->contExpr[i]->exprType == POW){
									Expr *pw = pr->contExpr[i];
									if(pw->expoIsMinusOne() && pw->getBase()->exprType == NUM && pw->getBase()->value.rep == INT){
										den = pr->contExpr[i];
									}
								}
							}
							if(num && den){
								if(num->value.neg()){
									rat = true;
								}
							}
						}
					}
					if(rat){
						setExpo(sumC(getExpo(),numC(1L)));
						Expr *repl = prodC(copy(),invC(getBase()->copy()));
						clearElements();
						become(repl);
						simplify();
						return;
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
	
	void Expr::distr(){
		if(exprType == PROD){
			Expr *sm = nullptr;
			for(int i = 0; i < numOfContExpr;i++){
				if(contExpr[i]->exprType == SUM) {
					sm = contExpr[i];
					contExpr[i] = nullptr;
					removeElement(i);
					break;
				}
			}
			if(sm != nullptr){
				
				for(int i = 0;i < sm->numOfContExpr;i++){
					sm->contExpr[i] = prodC(sm->contExpr[i],copy());
				}
				clearElements();
				become(sm);
				simple = false;
				simplify(false);
			}
		}
	}
	
	void Expr::factor(){
		if(exprType == SUM){
			
			{//move leading hash to front
				double max = 0.0;
				int indexOfLeader;
				for(int i = 0;i < numOfContExpr;i++){
					if(contExpr[i]->exprType == PROD){
						Expr *pr = contExpr[i];
						Expr *prCpy = pr->copy();
						for(int j = 0;j < pr->numOfContExpr;j++){
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
				for(int i = 0;i < pr->numOfContExpr;i++){
					if(pr->contExpr[i]->exprType == NUM){
						if(pr->contExpr[i]->value.neg()) leaderIsNeg = true;
					}
				}
			}
			
			if(leaderIsNeg) {
				for(int i = 0;i< numOfContExpr;i++){
					contExpr[i] = prodC(contExpr[i],numC(-1L));
					contExpr[i]->simplify();
				}
			}
			
			Expr *factors = new Expr(PROD);
			if(leaderIsNeg){
				simple = false;
				factors->addElement(copy());
				factors->addElement(numC(-1L));
				clearElements();
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
			for(int i = 0;i < firstEle->numOfContExpr;i++){
				Expr *ele = firstEle->contExpr[i];
				if(ele->exprType == NUM && ele->value.rep == INT){
					
					long int gc = ele->value.valueI;
					for(int j = 1; j<numOfContExpr;j++){
						if(contExpr[j]->exprType == NUM && contExpr[j]->value.rep == INT){
							gc = gcd(gc,contExpr[j]->value.valueI);
						}else if(contExpr[j]->exprType == PROD){
							bool foundNum = false;
							Expr *pr = contExpr[j];
							for(int k = 0;k < pr->numOfContExpr;k++){
								if(pr->contExpr[k]->exprType == NUM && pr->contExpr[k]->value.rep == INT){
									gc = gcd(gc,pr->contExpr[k]->value.valueI);
									foundNum = true;
								}
							}
							if(!foundNum){
								gc = 1;
								break;
							}
						}else{
							gc = 1;
							break;
						}
					}
					if(gc < 0) gc=-gc;
					if(gc != 1){
					
						factors->addElement(numC(gc));
						
						for(int j = 0;j < numOfContExpr;j++){
							if(contExpr[j]->exprType == NUM){
								contExpr[j]->value.valueI/=gc;
							}else if(contExpr[j]->exprType == PROD){
								Expr *pr = contExpr[j];
								for(int k = 0;k < pr->numOfContExpr;k++){
									if(pr->contExpr[k]->exprType == NUM){
										pr->contExpr[k]->value.valueI/=gc;
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
					if(ele->exprType == POW){
						if(ele->getExpo()->exprType == NUM){
							if(ele->getExpo()->value.plain() && !ele->getExpo()->value.neg()){
								ele->becomeInternal(ele->getBase());
							}
						}
					}
					bool allHaveIt = true;
					for(int j = 1; j< numOfContExpr;j++){
						if(contExpr[j]->exprType == PROD){
							Expr *pr = contExpr[j];
							bool found = false;
							for(int k = 0; k < pr->numOfContExpr;k++){
								if(pr->contExpr[k]->equalStruct(ele)){
									found = true;
									break;
								}else if(pr->contExpr[k]->exprType == POW){
									Expr *pw = pr->contExpr[k];
									if(pw->getBase()->equalStruct(ele) && pw->getExpo()->exprType == NUM){
										if(pw->getExpo()->value.plain() && !pw->getExpo()->value.neg()){
											found = true;
											break;
										}
									}
								}
							
							}
							if(found) continue;
						}
						
						if(contExpr[j]->equalStruct(ele)) continue;
						
						if(contExpr[j]->exprType == POW){
							Expr *pw = contExpr[j];
							if(pw->getBase()->equalStruct(ele) && pw->getExpo()->exprType == NUM){
								if(pw->getExpo()->value.plain() && !pw->getExpo()->value.neg()){
									continue;
								}
							}
						}
						
						allHaveIt = false;
						break;
						
					}
					if(allHaveIt){
						factors->addElement(ele);
						for(int j = 0; j< numOfContExpr;j++){
							if(contExpr[j]->equalStruct(ele)){
								delete contExpr[j];
								contExpr[j] = numC(1L);
								continue;
							}
							if(contExpr[j]->exprType == POW){
								Expr *pw = contExpr[j];
								if(pw->getBase()->equalStruct(ele) && pw->getExpo()->exprType == NUM){
									if(pw->getExpo()->value.plain() && !pw->getExpo()->value.neg()){
										Num negOne(-1L);
										pw->getExpo()->value.addN(&negOne);
										pw->simple = false;
										pw->simplify();
										continue;
									}
								}
							}
							if(contExpr[j]->exprType == PROD){
								Expr *pr = contExpr[j];
								bool found = false;
								for(int k = 0; k < pr->numOfContExpr;k++){
									if(pr->contExpr[k]->equalStruct(ele)){
										pr->removeElement(k);
										pr->simplify();
										break;
									}else if(pr->contExpr[k]->exprType == POW){
										Expr *pw = pr->contExpr[k];
										if(pw->getBase()->equalStruct(ele) && pw->getExpo()->exprType == NUM){
											if(pw->getExpo()->value.plain() && !pw->getExpo()->value.neg()){
												Num negOne(-1L);
												pw->getExpo()->value.addN(&negOne);
												pw->simple = false;
												pw->simplify();
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
				clearElements();
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
			for(int i = 0;i < numOfContExpr;i++){//get epsilon
				if(contExpr[i]->direction != MIDDLE){
					direction = contExpr[i]->direction;
					contExpr[i]->direction = MIDDLE;
					break;
				}
			}
			{//infinty + x -> infinity && -infinty + x -> -infinity && infinity-infinity -> indeterminate
				int dir = 0;
				for(int i = 0; i< numOfContExpr;i++){
					if(contExpr[i]->exprType == NUM){
						if(contExpr[i]->value.rep == INF){
							if(dir == 0 || dir == 1){
								dir = 1;
							}else{
								direction = INDET;
							}
						}else if(contExpr[i]->value.rep == NEGINF){
							if(dir == 0 || dir == -1){
								dir = -1;
							}else{
								direction = INDET;
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
				for(int i = 0;i < numOfContExpr;i++) if(contExpr[i]->exprType == PROD) contExpr[i]->distr();
			}
		
			{//sum contains sum
				for(int i = 0;i<numOfContExpr;i++){
					if(contExpr[i]->exprType == SUM){
					
						for(int j = 0;j<contExpr[i]->numOfContExpr;j++) addElement(contExpr[i]->contExpr[j]);
						contExpr[i]->nullify();
						removeElement(i);
						i--;
						
					}
				}
			}
			
			{//if sum with equation apply to both sides
				Expr *eq = nullptr;
				for(int i = 0;i<numOfContExpr;i++){
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
					clearElements();
					become(eq);
					simple = false;
					simplify();
					return;
				}
			}
			
			{//if list in product apply to all
				Expr *list = nullptr;
				for(int i = 0;i < numOfContExpr;i++){
					if(contExpr[i]->exprType == LIST){
						list = contExpr[i];
						contExpr[i] = nullptr;
						removeElement(i);
						break;
					}
				}
				if(list){
					for(int i = 0;i< list->numOfContExpr;i++) list->contExpr[i] = sumC(copy(),list->contExpr[i]);
					clearElements();
					become(list);
					simple = false;
					simplify();
					return;
				}
			}
			
			
			{//3*ln(4)+4*ln(7) -> 2*ln(392) && // merging logs
				int countOuter = 0;
				for(int i = 0;i<numOfContExpr;i++){
					if(contExpr[i]->exprType == LOG){
						countOuter++;
					}else if(contExpr[i]->exprType == PROD){
						Expr *pr = contExpr[i];
						int count = 0;
						for(int j = 0;j<pr->numOfContExpr;j++){
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
					for(int i = 0;i < numOfContExpr;i++){
						
						if(contExpr[i]->exprType == LOG){
							
							lg->contExpr[0]->addElement(contExpr[i]->contExpr[0]);
							contExpr[i]->contExpr[0] = nullptr;
							removeElement(i);
							i--;
						}else if(contExpr[i]->exprType == PROD){
							
							Expr *pr = contExpr[i];
							int indexOfLog = -1;
							int count = 0;
							for(int j = 0;j<pr->numOfContExpr;j++){
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
				for(int i = 0;i<numOfContExpr;i++){
					if(contExpr[i]->exprType == NUM && contExpr[i]->value.plain()) continue;
					bool replace = false;
					Num sm(1L); 
					Expr *current = contExpr[i]->copy();
					//remove num from current
					if(current->exprType == PROD){
						for(int j = 0;j<current->numOfContExpr;j++){
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
					
					for(int j = i+1;j<numOfContExpr;j++){
						if(contExpr[j]->exprType == NUM && contExpr[j]->value.plain()) continue;
						Num sm2(1L);
						Expr *comp = contExpr[j]->copy();
						//remove num from comp
						if(comp->exprType == PROD){
							for(int k = 0;k<comp->numOfContExpr;k++){
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
						contExpr[i]->prodSimp();
					}else{
						delete current;
					}
				
				}
			}
			{//adding numbers
				Num sm(0L);
				for(int i = 0;i < numOfContExpr;i++){
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
				
				for(int i = 0;i < numOfContExpr;i++){
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
						for(int j = 0;j < pr->numOfContExpr;j++){
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
					Expr *numer = numC(0L);
					Expr *den = new Expr(PROD);
					for(int i = 0;i < numOfContExpr;i++){
						Expr *ele = contExpr[i];
						// a/b + c/d -> (a*d+c*b)/(d*b)
						if(ele->exprType == PROD){
							Expr *newNum = new Expr(PROD);
							Expr *newDen = new Expr(PROD);
							for(int j = 0;j < ele->numOfContExpr;j++){
								bool isden = false;
								if(ele->contExpr[j]->exprType == POW){
									Expr *pw = ele->contExpr[j];
									if(pw->getExpo()->exprType == NUM){
										if(pw->getExpo()->value.neg()){
											isden = true;
											pw->getExpo()->value.absN();
											pw->simple = false;
											newDen->addElement(pw);
										}
									}
								}
								
								if(!isden) newNum->addElement(ele->contExpr[j]);
							
							}
							ele->nullify();
							delete ele;
							
							numer = sumC(prodC(numer,newDen),prodC(newNum,den->copy()));
							den->addElement(newDen->copy());
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
					clearElements();
					for(int i = 0;i<den->numOfContExpr;i++) den->contExpr[i] = invC(den->contExpr[i]);
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
				for(int i = 0;i < numOfContExpr;i++) if(contExpr[i]->exprType == SUM) contExpr[i]->factor();
			}
		
			{//product contains product
				for(int i = 0;i<numOfContExpr;i++){
					if(contExpr[i]->exprType == PROD){
					
						for(int j = 0;j<contExpr[i]->numOfContExpr;j++) addElement(contExpr[i]->contExpr[j]);
						contExpr[i]->nullify();
						removeElement(i);
						i--;
						
					}
				}
			}
			
			{//if product with equation apply to both sides
				Expr *eq = nullptr;
				for(int i = 0;i<numOfContExpr;i++){
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
					clearElements();
					become(eq);
					simple = false;
					simplify();
					return;
				}
			}
			
			{//if list in product apply to all
				Expr *list = nullptr;
				for(int i = 0;i < numOfContExpr;i++){
					if(contExpr[i]->exprType == LIST){
						list = contExpr[i];
						contExpr[i] = nullptr;
						removeElement(i);
						break;
					}
				}
				if(list){
					for(int i = 0;i< list->numOfContExpr;i++) list->contExpr[i] = prodC(copy(),list->contExpr[i]);
					clearElements();
					become(list);
					simple = false;
					simplify();
					return;
				}
			}
			
			{//x*0 -> 0
				bool foundInf = false;
				bool foundZero = false;
				for(int i = 0;i< numOfContExpr;i++){
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
					direction = INDET;
				}
			}
			
			{//4^(3*x) -> 64^x this step prepares for next step then reversed
				for(int i = 0;i<numOfContExpr;i++){
					if(contExpr[i]->exprType == POW){
						Expr *pw = contExpr[i];
						if(pw->getExpo()->exprType == PROD && pw->getBase()->exprType == NUM && pw->getBase()->value.rep == INT){
							Expr *pr = pw->getExpo();
							
							bool changed = false;
							for(int j = 0;j < pr->numOfContExpr; j++){
								if(pr->contExpr[j]->exprType == NUM && pr->contExpr[j]->value.plain() && !pr->contExpr[j]->value.neg()){
									pw->getBase()->value.powN(&pr->contExpr[j]->value);
									pr->removeElement(j);
									j--;
									changed = true;
								}
							}
							if(changed && pr->numOfContExpr == 1) pr->becomeInternal(pr->contExpr[0]);
							
						}
					}
				}
			}
			
			bool absInProd = false;
			{//check if there are absolute values becuase next step may need to be done twice
				for(int i = 0;i<numOfContExpr;i++){
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
			int cycles = 1;
			if(absInProd) cycles++;
			for(int c = 0;c<cycles;c++){//x*x^2*x^a*5*y -> x^(3+a)*5*y && 5^x*3^x -> 15^x
				for(int i = 0;i<numOfContExpr;i++){
					if(contExpr[i]->exprType == NUM && contExpr[i]->value.plain()) continue;
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
					bool baseIsNum = current->getBase()->exprType == NUM && current->getBase()->value.plain();
					
					for(int j = i+1;j < numOfContExpr;j++){
						if(contExpr[j]->exprType == NUM && contExpr[j]->value.plain()) continue;
						Expr *comp = contExpr[j];
						if(comp->exprType == POW){
							if(baseIsNum){
								if(comp->getBase()->exprType == NUM && comp->getExpo()->equalStruct(current->getExpo()) && comp->getBase()->value.plain()){
									contExpr[i]->getBase()->value.multN(&comp->getBase()->value);
									removeElement(j);
									j--;
								}
							}else if(comp->getBase()->equalStruct(current->getBase())){
								replace= true;
								expoSum->addElement(comp->getExpo());
								comp->contExpr[1] = nullptr;
								removeElement(j);
								j--;
							}
						}else if(!baseIsNum){
							if(comp->equalStruct(current->getBase())){
								replace= true;
								expoSum->addElement(numC(1L));
								removeElement(j);
								j--;
							}
						}
						
					}
					if(replace){
						delete contExpr[i];
						current->setExpo(expoSum);
						contExpr[i] = current;
						contExpr[i]->simple = false;
						contExpr[i]->simplify();
					}else{
						delete current;
						expoSum->nullify();
						delete expoSum;
					}
					
				}
			}
			
			for(int i = 0;i<numOfContExpr;i++){//bring back
				if(contExpr[i]->exprType == POW){
					if(contExpr[i]->getBase()->exprType == NUM && contExpr[i]->getBase()->value.rep == INT){
						contExpr[i]->simple = false;
						contExpr[i]->simplify();
					}
				}
			}
			
			{//multiplying numbers and inverse numbers
				Num pr(1L);
				Num ipr(1L);
				
				bool neg = false;
				bool inf = false;
				for(int i = 0;i < numOfContExpr;i++){
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
				
					Num g(gcd(pr.valueI,ipr.valueI));
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
				int hasInf = 0;
				for(int i = 0;i < numOfContExpr;i++){
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
				for(int i = 0;i<list->numOfContExpr;i++){
					list->contExpr[i] = logC(list->contExpr[i]);
				}
				
				becomeInternal(list);
				simple = false;
				simplify();
				return;
			}
			if(contExpr[0]->exprType == NUM){//log(1) -> 0 && log(e) -> 1 && log(0) -> -infinity && log(infinity) -> infinity
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
				}
			
			}
			
			{//need factoring common exponents
			}
			
			if(contExpr[0]->exprType == POW){//log(e^x) -> x && log(a^b) -> b*log(a)
				if(contExpr[0]->getBase()->exprType == NUM){
					if(contExpr[0]->getBase()->value.rep == EV){
						becomeInternal(contExpr[0]->getExpo());
						return;
					}
				}
				Expr *b = contExpr[0]->getExpo();
				contExpr[0]->contExpr[1] = nullptr;
				contExpr[0]->removeElement(1);
				contExpr[0]->exprType = LOG;
				exprType = PROD;
				addElement(b);
				simple = false;
				simplify();
				return;
			}
			
			
			if(contExpr[0]->exprType == NUM){//log(8) -> 3*log(2)
				if(contExpr[0]->value.rep == INT){
					long int b,e;
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
				for(int i = 0;i< list->numOfContExpr;i++){
					list->contExpr[i] = new Expr(SOLVE,list->contExpr[i],contExpr[1]->copy());
				}
				becomeInternal(list);
				simple = false;
				simplify();
				return;
			}else if(contExpr[1]->exprType == LIST){
				Expr *list = contExpr[1];
				for(int i = 0;i< list->numOfContExpr;i++){
					list->contExpr[i] = new Expr(SOLVE,contExpr[0]->copy(),list->contExpr[i]);
				}
				becomeInternal(list);
				simple = false;
				simplify();
				return;
			}
			
			if(contExpr[0]->exprType != EQU){
				contExpr[0] = equC(contExpr[0],numC(0L));
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
				eq->contExpr[1] = numC(0L);
				eq->contExpr[0]->simplify();
			}
			
			while(true){
				
				bool changed = false;
				eq->contExpr[0]->factor();
				
				if(eq->contExpr[0]->exprType == SUM){
					Expr *sm = eq->contExpr[0];
					Expr *nvarSm = new Expr(SUM);
					for(int i = 0;i < sm->numOfContExpr; i++){
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
						for(int i = 0;i < pr->numOfContExpr; i++){
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
						for(int i = 0;i < list->numOfContExpr;i++){//remove variable-less parts
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
					if(!changed){//f(x)*(a*f(x)+b)=c -> a*((f(x)+b/2/a)^2-(b/2/a)^2)=c
						if(pr->numOfContExpr == 2){
							//find sum && other
							Expr *otherPart = nullptr,*sumPart = nullptr;
							for(int k = 0;k<2;k++){
								if(pr->contExpr[k]->exprType == SUM) sumPart = pr->contExpr[k];
								else otherPart = pr->contExpr[k];
							}
							if(sumPart && otherPart){
								//find f(x) in sum
								int indexOfFX = -1;
								for(int k = 0;k<sumPart->numOfContExpr;k++){
									if(sumPart->contExpr[k]->contains(v)){
										indexOfFX = k;
										break;
									}
								}
								Expr *a = nullptr;
								if(sumPart->contExpr[indexOfFX]->exprType == PROD){
									Expr *FXpr = sumPart->contExpr[indexOfFX];
									
									for(int k = 0;k<FXpr->numOfContExpr;k++){
										if(FXpr->contExpr[k]->equalStruct(otherPart)){
											a = new Expr(PROD);
											for(int j = 0;j<FXpr->numOfContExpr;j++){
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
							for(int i = 0;i < ex->numOfContExpr;i++){
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
					printf("-unable to isolate variable\n");
					break;
				}	
			}
		}
	}
	void Expr::absSimp(){
		if(exprType == ABS){
			contExpr[0]->factor();
			if(contExpr[0]->exprType == LIST){
				Expr *list = contExpr[0];
				for(int i = 0;i<list->numOfContExpr;i++){
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
			}else if(contExpr[0]->exprType == POW){
				Expr *pw = contExpr[0];
				pw->simple = false;
				pw->setBase(absC(pw->getBase()));
				becomeInternal(pw);
				simplify();
				return;
			}if(contExpr[0]->exprType == PROD){//|a*b| -> |a|*|b|
				Expr *pr = contExpr[0];
				for(int i = 0;i < pr->numOfContExpr;i++){
					pr->contExpr[i] = absC(pr->contExpr[i]);
				}
				pr->simple = false;
				becomeInternal(pr);
				simplify();
				return;
			}
			
			contExpr[0]->simplify();
			if(contExpr[0]->exprType == POW){// |x^(2*a)| -> x^(2*a) basicly anything even
				Expr *pw = contExpr[0];
				pw->getExpo()->factor();
				if(pw->getExpo()->exprType == PROD){
					Expr *pr = pw->getExpo();
					for(int i = 0;i< pr->numOfContExpr;i++){
						if(pr->contExpr[i]->exprType == NUM && pr->contExpr[i]->value.rep == INT){
							if(pr->contExpr[i]->value.valueI %2L == 0){
								becomeInternal(contExpr[0]);
								getExpo()->distr();
								return;
							}
						}
					}
				}else if(pw->getExpo()->exprType == NUM && pw->getExpo()->value.rep == INT){
					if(pw->getExpo()->value.valueI %2L == 0){
						becomeInternal(contExpr[0]);
						getExpo()->distr();
						return;
					}
				}
				pw->getExpo()->distr();
			}
		}
	}
	
	void Expr::listSimp(){
		if(exprType == LIST){
			{//merge
				for(int i = 0;i<numOfContExpr;i++){
					if(contExpr[i]->exprType == LIST){
					
						for(int j = 0;j<contExpr[i]->numOfContExpr;j++) addElement(contExpr[i]->contExpr[j]);
						contExpr[i]->nullify();
						removeElement(i);
						i--;
						
					}
				}
			
			}
			
			{//factor for consistency
				for(int i = 0;i<numOfContExpr;i++) contExpr[i]->factor();
			}
			
			{//remove duplicates
				for(int i = 0; i < numOfContExpr;i++){
					for(int j = i+1;j < numOfContExpr;j++){
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
				for(int i = 0;i < list->numOfContExpr;i++){
					list->contExpr[i] = equC(list->contExpr[i],contExpr[1]->copy());
				}
				becomeInternal(list);
				return;
			}
			if(contExpr[1]->exprType == LIST){//z = [x,y] -> [z=x,z=y] 
				Expr *list = contExpr[1];
				for(int i = 0;i < list->numOfContExpr;i++){
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
					
					for(int i = 0; i< left->numOfContExpr;i++){
					
						for(int j = 0; j < right->numOfContExpr;j++){
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
			contExpr[0]->factor();
			if(contExpr[0]->exprType == PROD){//sin(-x)-> -sin(x)
				Expr *pr = contExpr[0];
				bool sig = false;
				for(int i = 0;i < pr->numOfContExpr;i++){
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
					clearElements();
					become(repl);
					simplify();
					return;
				}
			}
			if(contExpr[0]->exprType == PROD && constant()){//unit circle
				Expr *pr = contExpr[0];
				//
				bool foundPi = false;
				long int num = 1,den = 1;
				bool foundOther = false;
				for(int i = 0;i<pr->numOfContExpr;i++){
					bool integer = pr->contExpr[i]->exprType == NUM && pr->contExpr[i]->value.rep == INT;
					bool inverseInt = false;
					if(pr->contExpr[i]->exprType == POW){
						if(pr->contExpr[i]->expoIsMinusOne() && pr->contExpr[i]->getBase()->exprType == NUM && pr->contExpr[i]->getBase()->value.rep == INT){
							inverseInt = true;
						}
					}
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
						clearElements();
						if(neg) become(prodC(powC(numC(3L),invC(numC(2L))),invC(numC(-2L))));
						else become(prodC(powC(numC(3L),invC(numC(2L))),invC(numC(2L))));
						return;
					}else if(den == 4){//sin(pi/4) -> sqrt(2)/2
						clearElements();
						if(neg) become(prodC(powC(numC(2L),invC(numC(2L))),invC(numC(-2L))));
						else become(prodC(powC(numC(2L),invC(numC(2L))),invC(numC(2L))));
						return;
					}
					else if(den == 6){//sin(pi/6) -> 1/2
						clearElements();
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
							clearElements();
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
		
			if(contExpr[0]->exprType == ABS){//cos(|x|) -> cos(x)
				contExpr[0]->becomeInternal(contExpr[0]->contExpr[0]);
			}
			
			contExpr[0]->factor();
			
			if(contExpr[0]->exprType == PROD){//cos(-x)-> -cos(x)
				Expr *pr = contExpr[0];
				for(int i = 0;i < pr->numOfContExpr;i++){
					if(pr->contExpr[i]->exprType == NUM){
						if(pr->contExpr[i]->value.neg()){
							pr->contExpr[i]->value.absN();
						}
					}else if(pr->contExpr[i]->exprType == POW){
						Expr *pw = pr->contExpr[i];
						if(pw->expoIsMinusOne() && pw->getBase()->exprType == NUM){
							if(pw->getBase()->value.neg()){
								pw->getBase()->value.absN();
								pr->simple = false;
							}
						}
					}else if(pr->contExpr[i]->exprType == ABS){
						pr->simple = false;
						pr->contExpr[i]->becomeInternal(pr->contExpr[i]->contExpr[0]);
					}
				}
				pr->simple = false;
				pr->simplify();
			
			}
			if(contExpr[0]->exprType == PROD && constant()){//unit circle
				Expr *pr = contExpr[0];
				//
				bool foundPi = false;
				long int num = 1,den = 1;
				bool foundOther = false;
				for(int i = 0;i<pr->numOfContExpr;i++){
					bool integer = pr->contExpr[i]->exprType == NUM && pr->contExpr[i]->value.rep == INT;
					bool inverseInt = false;
					if(pr->contExpr[i]->exprType == POW){
						if(pr->contExpr[i]->expoIsMinusOne() && pr->contExpr[i]->getBase()->exprType == NUM && pr->contExpr[i]->getBase()->value.rep == INT){
							inverseInt = true;
						}
					}
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
	
	Expr *linear(Expr *expr,Expr *var){//returns the a in a*x+b .used for integral
		Expr *v = varC("0tmp.");
		expr->replace(var,v);
		bool c = expr->constant();
		expr->replace(v,var);
		delete v;
		if(!c) {
			return nullptr;
		}
		int count = 0;
		Expr *og = expr;
		if(expr->exprType == SUM){
			for(int i = 0;i<og->numOfContExpr;i++){
				if(og->contExpr[i]->contains(var)){
					count++;
					expr = expr->contExpr[i];
				}
			}
		}else{
			count = 1;
		}
		if(count != 1) return nullptr;
		if(expr->equalStruct(var)){
			return numC(1L);
		}else if(expr->exprType == PROD){
			Expr *cpy = expr->copy();
			for(int i = 0;i<cpy->numOfContExpr;i++){
				if(cpy->contExpr[i]->equalStruct(var)){
					cpy->removeElement(i);
					break;
				}
			}
			if(cpy->constant()){
				return cpy;
			}else{
				delete cpy;
				return nullptr;
			}
		}
		return nullptr;
	}
	
	void Expr::integSimp(bool usub){
		if(exprType == INTEG){
			
			if(contExpr[0]->exprType == DERI){//Int(d(x)) -> x
				becomeInternal(contExpr[0]->contExpr[0]);
				return;
			}
			if(contExpr[0]->value.equalsI(0L) && contExpr[0]->exprType == NUM){//int 0 -> 0
				clearElements();
				exprType = NUM;
				value.setValueI(0L);
				return;
			}
			if(contExpr[0]->exprType == PROD){//integral(5*x) -> 5*integral(x)
				Expr *pr = contExpr[0];
				Expr *replProd = nullptr;
				for(int i = 0;i<pr->numOfContExpr;i++){
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
					clearElements();
					become(replProd);
					simplify();
					return;
				}
			}
			contExpr[0]->factor();
			
			if(contExpr[0]->exprType == PROD){//common forms integral(form*d(x))
				Expr *pr = contExpr[0];
				if(pr->numOfContExpr == 2){//simple integrals
					Expr *var = nullptr;
					Expr *otherPart = nullptr;
					for(int i = 0;i<2;i++){
						if(pr->contExpr[i]->exprType == DERI){
							var = pr->contExpr[i]->contExpr[0];
						}else{
							otherPart = pr->contExpr[i];
						}
					}
					if(var){
						if(var->equalStruct(otherPart)){//integral(x*d(x)) -> x^2/2
							Expr *repl = prodC(powC(var->copy(),numC(2L)),invC(numC(2L)));
							clearElements();
							become(repl);
							simple = true;
							return;
						}
						if(otherPart->exprType == POW){//integral((linear)^c.*d(x)) -> inv(a*(c+1))*(linear)^(c+1)
							if(otherPart->getExpo()->constant()){
								Expr *a = linear(otherPart->getBase(),var);
								if(a){
									if(otherPart->expoIsMinusOne()){
										Expr *repl = prodC(invC(a),logC(otherPart->getBase()->copy()));
										clearElements();
										become(repl);
										simplify();
										return;
									}else{
									
										Expr *cp1 = sumC(numC(1L),otherPart->getExpo()->copy());
									
										Expr *repl = prodC(invC(cp1),powC(otherPart->getBase()->copy(),cp1->copy()));
										repl->addElement(invC(a));
										clearElements();
										become(repl);
										simplify();
										return;
									}
								}else if(otherPart->getExpo()->exprType == NUM && otherPart->getExpo()->value.equalsI(2L)){
									
									if(otherPart->getBase()->exprType == COS){
										Expr *inner = otherPart->getBase()->contExpr[0];
										Expr *a = linear(inner,var);
										if(a){
											
											Expr *repl = sinC(prodC(inner->copy(),numC(2L)));
											Expr *twax = prodC(a,var->copy());
											twax->addElement(numC(2L));
											repl = prodC(sumC(twax,repl),invC(numC(4L)));
											repl->addElement(invC(a->copy()));
											clearElements();
											become(repl);
											simplify();
											return;
										}
									}
									
								}
							}else if(otherPart->getBase()->constant()){//integral(n.^(linear)*d(x)) -> n.^(linear)/(ln(n.)*a)
								Expr *a = nullptr;
								a = linear(otherPart->getExpo(),var);
								if(a){
									Expr *repl = prodC(otherPart->copy(),invC(logC(otherPart->getBase()->copy())));
									repl->addElement(invC(a));
									clearElements();
									become(repl);
									simplify();
									return;
								}
							}
						}else if(otherPart->exprType == LOG){//integral(ln(linear)*d(x))
							Expr *a = linear(otherPart->contExpr[0],var);
							if(a){
								Expr *repl = new Expr(PROD);
								repl->addElement(invC(a));
								repl->addElement(sumC(otherPart->copy(),numC(-1L)));
								repl->addElement(otherPart->contExpr[0]->copy());
								
								clearElements();
								become(repl);
								simplify();
								return;
							}
						}else if(otherPart->exprType == COS){
							Expr *a = linear(otherPart->contExpr[0],var);
							if(a){
								Expr *repl = prodC(sinC(otherPart->contExpr[0]->copy()),invC(a));
								clearElements();
								become(repl);
								simplify();
								return;
							}
						}else if(otherPart->exprType == SIN){
							Expr *a = linear(otherPart->contExpr[0],var);
							if(a){
								Expr *repl = prodC(cosC(otherPart->contExpr[0]->copy()),invC(a));
								repl->addElement(numC(-1L));
								clearElements();
								become(repl);
								simplify();
								return;
							}
						}
						
						
						
					}
				}
				Expr *var = nullptr;
				for(int i = 0;i<pr->numOfContExpr;i++){
					if(pr->contExpr[i]->exprType == DERI){
						var = pr->contExpr[i]->contExpr[0];
					}
				}
				
				if(var){//IBP integral(fraction*d(x)*expr ) -> expr*integral(fraction*d(x))-integral(d(expr)*integral(fraction*d(x))) fraction must have expo < -1
					//example integral(x*e^(2*x)/(2*x+1)^2*d(x))
					int indexOfFraction = -1;
					for(int i = 0;i<pr->numOfContExpr;i++){
						if(pr->contExpr[i]->exprType == POW){
							Expr *pw = pr->contExpr[i];
							if(pw->getExpo()->exprType == NUM && pw->getExpo()->value.rep == INT && pw->getExpo()->value.valueI < -1 && pw->contains(var)){
								indexOfFraction = i;
								break;
							}
						}
					}
					if(indexOfFraction != -1){
						Expr *fraction = integC(prodC(pr->contExpr[indexOfFraction],diffC(var->copy())));
						pr->contExpr[indexOfFraction] = nullptr;
						pr->removeElement(indexOfFraction);
						pr->addElement(invC(diffC(var->copy())));
						fraction->simplify();
						pr->simplify();
						
						Expr *repl = sumC(prodC(pr->copy(),fraction),prodC(numC(-1L),integC(prodC(diffC(pr->copy()),fraction->copy()))));
						clearElements();
						become(repl);
						simplify();
						return;
					}
				}
				
				if(var && usub){//u sub
					//find most nested part that is not dx
					int max = 0;
					int indexOfHighestDepth = 0;
					for(int i = 0;i < pr->numOfContExpr;i++){
						int depth = pr->contExpr[i]->nestDepth();
						if(depth>max){
							max = depth;
							indexOfHighestDepth = i;
						}
					}
					Expr *mostComplicated = pr->contExpr[indexOfHighestDepth];
					if(mostComplicated->numOfContExpr > 0){
						
						max = 0;
						int indexOfHighestDepth2 = 0;
						for(int k = 0;k<mostComplicated->numOfContExpr;k++){
							int depth = mostComplicated->contExpr[k]->nestDepth();
							if(depth>max){
								max = depth;
								indexOfHighestDepth2 = k;
							}
						}
						Expr *inner = mostComplicated->contExpr[indexOfHighestDepth2];
						
						Expr *u = varC("0u");
						
						Expr *mcu = pr->copy();
						mcu->contExpr[indexOfHighestDepth]->replace(inner,u);
						
						mcu->addElement(invC(diffC(inner->copy())));
						mcu->addElement(diffC(u->copy()));
						mcu->simplify();
						
						if(mcu->contains(var)){
							delete u;
							delete mcu;
						}else{
							mcu = integC(mcu);
							mcu->integSimp(false);
							mcu->replace(u,inner);
							delete u;
							clearElements();
							become(mcu);
							simplify();
							return;
						}
						
						
					}
					
				}
				
				if(var && pr->numOfContExpr > 2){//u sub special case: example integral(sin(x)*cos(x)*d(x)) or integral(ln(x)*inv(x)*d(x))
					
					for(int i = 0;i< pr->numOfContExpr;i++){
						Expr *leftOver = diffC(pr->contExpr[i]->copy());
						leftOver->simplify();
						Expr *a = new Expr(PROD);
						if(leftOver->exprType == PROD){
							for(int k = 0;k<leftOver->numOfContExpr;k++){
								if(leftOver->contExpr[k]->constant()){
									a->addElement(leftOver->contExpr[k]);
									leftOver->contExpr[k] = nullptr;
									leftOver->removeElement(k);
									k--;
								}
							}
						}
						leftOver = prodC(pr->copy(),invC(leftOver));
						leftOver->simplify();
						if(leftOver->equalStruct(pr->contExpr[i])){
							clearElements();
							become(prodC(powC(leftOver,numC(2L)),invC(numC(2L))));
							addElement(invC(a));
							simplify();
							return;
						}
						delete a;
						delete leftOver;
					}
				}
				
				if(var){//IBP  integral((polynomial)*(otherPart)) -> polynomial*integral(otherPart)-integral(d(polynomial)*integral(otherPart))	
					int indexOfEasy = -1;
					for(int i = 0;i<pr->numOfContExpr;i++){
						if(pr->contExpr[i]->equalStruct(var)){
							indexOfEasy = i;
						}else if(pr->contExpr[i]->exprType == LOG){
							indexOfEasy = i;
							break;//logs are best option
						}else if(pr->contExpr[i]->exprType == POW){
							Expr *pw = pr->contExpr[i];
							if(pw->getExpo()->exprType == NUM && pw->getExpo()->value.rep == INT && pw->getExpo()->value.valueI>0){
								if(pw->getBase()->equalStruct(var)){
									indexOfEasy = i;
								}else if(pw->getBase()->exprType == LOG){
									indexOfEasy = i;
									break;//logs are best option
								}else{
									Expr *a = linear(pw->getBase(),var);
									if(a){
										delete a;
										indexOfEasy = i;
									}
								}
							}
						}
					}
					if(indexOfEasy!=-1){
						Expr *easy = pr->contExpr[indexOfEasy];
						pr->contExpr[indexOfEasy] = nullptr;
						pr->removeElement(indexOfEasy);
						
						Expr *otherPartIntegral = integC(pr->copy());
						otherPartIntegral->simplify();
						Expr *repl = sumC(prodC(easy,otherPartIntegral),prodC(numC(-1L),integC(prodC(diffC(easy->copy()),otherPartIntegral->copy()))));
						clearElements();
						become(repl);
						simplify();
						return;
					}
					
					
				}
				
			}
			
			contExpr[0]->distr();
			if(contExpr[0]->exprType == SUM){//integral(x+y) -> integral(x)+integral(y)
				Expr *sm = contExpr[0];
				Expr *repl = nullptr;
				for(int i = 0;i<sm->numOfContExpr;i++){
					Expr *test = integC(sm->contExpr[i]->copy());
					test->simplify();
					if(!test->containsType(INTEG) || test->exprType == SUM){
						if(!repl){
							repl = new Expr(SUM);
						}
						repl->addElement(test);
						sm->removeElement(i);
						i--;
					}else delete test;
				}
				if(repl){
					repl->addElement(copy());
					clearElements();
					become(repl);
					simplify();
					return;
				}
			}
			
		}
	}
	//WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW"simplify"W
	void Expr::simplify(){
		simplify(true);
	}
	
	void Expr::simplify(bool addFractions){
		//println();
		if(exprType == SUBST){
			if(contExpr[1]->exprType == EQU){
				replace(contExpr[1]->contExpr[0],contExpr[1]->contExpr[1]);
				becomeInternal(contExpr[0]);
			}else if(contExpr[1]->exprType == LIST){
				Expr *list = contExpr[1];
				for(int i = 0;i<contExpr[1]->numOfContExpr;i++){
					list->contExpr[i] = new Expr(SUBST,contExpr[0]->copy(),list->contExpr[i]);
				}
				becomeInternal(contExpr[1]);
				simple = false;
			}
		}
		if(simple) return;
		if(exprType == VAR || exprType == NUM){
			simple = true;
			return;
		}
		
		for(int i = 0;i<numOfContExpr;i++) contExpr[i]->simplify();
		
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
		else if(exprType == INTEG) integSimp();
		simple = true;
	}
	//RPN scanner
	
	const int STACK_MAX = 64;
	Expr *stack[STACK_MAX];
	int height = 0;
	
	void printStack(){
		printf("****************\n");
		for(int i = 0;i<height;i++){
			printf(": ");
			stack[i]->println();
		}
		printf("****************\n");
	
	}
	
	void clearStack(){
		for(int i = 0;i<height;i++) delete stack[i];
		height = 0;
	}
	
	Expr* rpnCas(){
		height = 0;
		printf("Max variable name length: %d\n",maxNameLength-1);
		printf("RPN Calculator\nType 'h' for help and 'q' to quit\n");
		printf("Max elements in stack: %d\n",STACK_MAX);
		
		while(true){
			char op;
			scanf("\n%c",&op);
			if(op == 'h'){
				printf("all commands are one character\nlist of all commands:\n");
				printf("	h : help\n");
				printf("	s : print stack\n");
				printf("	c : clear stack\n");
				printf("	r : calculate result of last element on stack\n");
				printf("	o : tells how many objects are in heap\n");
				printf("	n : add integer to stack\n");
				printf("	f : add float to stack\n");
				printf("	p : pop last element from stack\n");
				printf("	q : quit cas\n");
				printf("	d : diffrenciate in terms of last element on stack\n");
				printf("	3 : add pi to stack\n");
				printf("	2 : add eulers number to stack\n");
				printf("	v : add variable to stack, ending with . makes the variable constant\n");
				printf("	+ : adds last two stack elements\n");
				printf("	* : multiplies last two stack elements\n");
				printf("	^ : exponentiates last element on stack\n");
				printf("	- : multiplies last element by -1\n");
				printf("	i : takes the inverse of last element\n");
				printf("	l : takes the natural logarithm of last element\n");
				printf("	> : swaps last two elements\n");
				printf("	; : duplicates last element\n");
				printf("	# : rolls stack\n");
				printf("	/ : divide by last element on the stack\n");
				printf("	w : take the square root of last element\n");
				printf("	= : set last two elements on stack equal to each other\n");
				printf("	u : solve in terms of last element on stack\n");
				printf("	0 : get hash of last element\n");
				printf("	| : take the absolute value of last element\n");
				printf("	] : add last element on stack to list\n");
				printf("	[ : make list\n");
				printf("	z : make direction from left\n");
				printf("	x : make direction from right\n");
				printf("	6 : compare if two expressions are equal in structure\n");
				printf("	4 : add negative infinity to stack\n");
				printf("	5 : add infinity to stack\n");
				printf("	t : substitute, note that the substitution function is always done first\n");
				printf("	S : sine of last element on stack\n");
				printf("	C : cosine of last element on stack\n");
				printf("	I : integrate last element on stack\n");
				printf("	F : factor last element on stack\n");
				printf("	D : distribute last element on stack\n");
			}
			else if(op == 's'){
				printStack();
			}
			else if(op == 'c'){
				clearStack();
			}
			else if(op == 'r'){
				if(height == 0)	printf("-nothing on stack\n");
				else{
					long int start,end;
					start = clock();
					stack[height-1]->simplify();
					end = clock();
					
					printf("-took %lf milli secs to compute\n",(double)(end-start)/1000.0);
					printf("result: ");
					stack[height-1]->println();
				}
			}
			else if(op == 'o'){
				printObjCount();
			}
			else if(op == 'n'){
				printf("type in value\n");
				long int val;
				scanf("%ld",&val);
				stack[height] = numC(val);
				height++;
			}
			else if(op == 'f'){
				printf("type in floating value\n");
				double val;
				scanf("%lf",&val);
				stack[height] = numFC(val);
				height++;
			}
			else if(op == 'p'){
				if(height!=0){
					delete stack[height-1];
					height--;
				}else{
					printf("-can't delete\n");
				}
			}
			else if(op == 'q'){
				for(int i = 1;i<height;i++) delete stack[i];
				if(height == 0){
					stack[0] = numC(0L);
					height++;
				}
				if(objCount != 1 && ERRORS) printf("memory leak detected\n");
				printf("done\n");
				return stack[height-1];
			}
			else if(op == 'd'){
				if(height<1) printf("-nothing on stack\n");
				else stack[height-1] = diffC(stack[height-1]);
			}
			else if(op == '3'){
				Expr *out = new Expr(NUM);
				out->value.rep = PIV;
				stack[height] = out;
				printf("-added pi to stack\n");
				height++;
			}
			else if(op == '2'){
				Expr *out = new Expr(NUM);
				out->value.rep = EV;
				stack[height] = out;
				printf("-added e to stack\n");
				height++;
			}
			else if(op == 'v'){
				printf("-type in name\n");
				char name[maxNameLength];
				scanf("%s",name);
				bool startsWithNum = (name[0] > 47 && name[0] < 58) || (name[0] == 46);
				if(startsWithNum){
					printf("-variable name can't start with number\n");
				}else if(name[0] == 'e' && name[1] == '\0'){
					printf("-reserved for eulers number\n");
				}else{
					stack[height] = varC(name);
					height++;
				}
			}
			else if(op == '+'){
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
				}
			}
			else if(op == '*'){
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
				}
			}
			else if(op == '^'){
				if(height<2) printf("-need more elements\n");
				else{
					stack[height-2] = powC(stack[height-2],stack[height-1]);
					height--;
				}
			}
			else if(op == '-'){
				if(height<1) printf("-nothing on stack\n");
				else stack[height-1] = prodC(numC(-1),stack[height-1]);
			}
			else if(op == 'i'){
				if(height<1) printf("-nothing on stack\n");
				else stack[height-1] = powC(stack[height-1],numC(-1));
			}
			else if(op == 'l'){
				if(height<1) printf("-nothing on stack\n");
				else stack[height-1] = logC(stack[height-1]);
			}else if(op == '>'){
				if(height<2) printf("-need more elements\n");
				else{
					Expr *temp = stack[height-1];
					stack[height-1] = stack[height-2];
					stack[height-2] = temp;
				}
			}else if(op == ';'){
				if(height<1) printf("-nothing on stack\n");
				else{
					stack[height] = stack[height-1]->copy();
					height++;
				}
			}
			else if(op == '#'){
				if(height<2) printf("-need more elements\n");
				else{
					Expr *og = stack[height-1];
					for(int i = height-1;i >-1 ;i--){
						if(i == 0) stack[i] = og;
						else stack[i] = stack[i-1];
					}
				}
			}else if(op == '/'){
				if(height<2) printf("-need more elements\n");
				else{
					if(stack[height-2]->exprType == PROD) stack[height-2]->addElement(invC(stack[height-1]));
					else stack[height-2] = prodC(stack[height-2],invC(stack[height-1]));
					height--;
				}
			}else if(op == 'w'){
				if(height<1) printf("-need more elements\n");
				else stack[height-1] = powC(stack[height-1],powC(numC(2L),numC(-1L)));
			}else if(op == '='){
				if(height < 2) printf("-need more elements\n");
				else{
					stack[height-2] = equC(stack[height-2],stack[height-1]);
					height--;
				}
			}else if(op == 'u'){
				if(height < 2) printf("-need more elements\n");
				else{
					stack[height-2] = new Expr(SOLVE,stack[height-2],stack[height-1]);
					height--;
				}
			}else if(op == '0'){
				if(height==0) printf("-nothing on stack\n");
				else printf("%lf\n",stack[height-1]->hash());
			}else if(op == '|'){
				if(height<1) printf("-nothing on stack\n");
				else stack[height-1] = absC(stack[height-1]);
			}else if(op == ']'){
				if(stack[height-2]){
					if(stack[height-2]->exprType == LIST){
						stack[height-2]->addElement(stack[height-1]);
						height--;
					}else{
						printf("-no list to add to\n");
					}
				}else{
					printf("-need more elements\n");
				}
			}else if(op == '['){
				stack[height] = new Expr(LIST);
				height++;
			}else if(op == 'z'){
				if(height<1) printf("-nothing on stack\n");
				else stack[height-1]->direction = LEFT;
			}else if(op == 'x'){
				if(height<1) printf("-nothing on stack\n");
				else stack[height-1]->direction = RIGHT;
			}else if(op == '6'){
				if(height < 2) printf("-need more elements\n");
				else {
					if(stack[height-1]->equalStruct(stack[height-2])) printf("true\n");
					else printf("false\n");
				}
			}else if(op == '4'){
				stack[height] = new Expr(NUM);
				stack[height]->value.rep = NEGINF;
				height++;
			}else if(op == '5'){
				stack[height] = new Expr(NUM);
				stack[height]->value.rep = INF;
				height++;
			}else if(op == 't'){
				if(height < 2) printf("-need more elements\n");
				else{
					stack[height-2] = new Expr(SUBST,stack[height-2],stack[height-1]);
					height--;
				}
			}else if(op == 'S'){
				if(height < 1) printf("-need more elements\n");
				else stack[height-1] = sinC(stack[height-1]);
			}else if(op == 'C'){
				if(height < 1) printf("-need more elements\n");
				else stack[height-1] = cosC(stack[height-1]);
			}else if(op == 'I'){
				if(height < 1) printf("-need more elements\n");
				else stack[height-1] = integC(stack[height-1]);
			}else if(op == 'F'){
				stack[height-1]->factor();
			}else if(op == 'D'){
				stack[height-1]->distr();
			}
			
		}
		
	}
}

namespace simpleTools{
	int iter = 64;
	struct List{
		int n = 0;
		double *e;
		void print(){
			printf("[");
			for(int i = 0;i<n;i++){
				if(e[i] != (int)e[i]) printf("%lf",e[i]);
				else printf("%d",(int)e[i]);
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
			for(int i = 0; i<c.n;i++){
				double prod = 1;
				for(int j = 0;j<i;j++) prod*=x;
				prod*=c.e[i];
				sum+=prod;
			}
			return sum;
		}
		double bisection(double left,double right){
			bool increases = out(left) < 0.0;
			for(int i = 0;i<iter;i++){
				double midPoint = (left+right)/2.0;
				if((out(midPoint) < 0.0)==increases) left = midPoint;
				else right = midPoint;
			}
			return (left+right)/2.0;
		}
		void diff(Poly *p){//get the derivative
			for(int i = 0;i < p->c.n;i++) p->c.e[i] = c.e[i+1]*(i+1);
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
				
				int solutionCount = 0;
				//if derivative Solutions count == 0 use newtons method. Example 1/3*x^3+x+2 is special case
				if(derSol.n == 0){
					double guess = 1.0;
					for(int i = 0;i<iter;i++) guess = guess-out(guess)/derv.out(guess);
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
					for(int i = 0; i < derSol.n-1;i++){//middle solutions
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
					for(int i = 0;i<solutionCount;i++) newList[i] = res->e[i];
					delete[] res->e;
					res->e = newList;
					res->n = solutionCount;
				}
			}
		}
		void print(){
			bool printedSomething = false;
			for(int i = 0;i<c.n;i++){
				if(c.e[i] != 0.0){
					if(printedSomething) printf("+");
					if(c.e[i] != 1.0){
						if((int)c.e[i] == c.e[i]) printf("%d",(int)c.e[i]);
						else printf("%lf",c.e[i]);
						if(i>1) printf("*x^%d",i);
						else if(i>0) printf("*x");
					}else{
						if(i>1) printf("x^%d",i);
						else if(i>0) printf("x");
					}
					printedSomething = true;
				}
			}
			printf("\n");
		}
	};
	
	void polySolver(){
		printf("type the degree of the polynomial\n");
		int deg;
		while(true){
			scanf("\n%d",&deg);
			if(deg>0) break;
			else printf("please enter valid degree\n");
		}
		Poly p;
		p.c.n = deg+1;
		p.c.e = new double[p.c.n];
		printf("type in all the coefficients from lowest degree to highest\n");
		for(int i = 0;i<p.c.n;i++){
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
		
	}
}

#pragma pack(pop)

void toolSelect(){
	printf("Created By Benjamin Currie @2020\n");
	while(true){
		printf("Tool Selecter:\nc : rpn cas\np : numeric polynomial solver\ng : grapher\n");
		char op;
		scanf("\n%c",&op);
		if(op == 'c'){
			using namespace microCas;
			delete rpnCas();
		}else if(op == 'p'){
			simpleTools::polySolver();
		}
	}
}

int main(){
	toolSelect();
}
