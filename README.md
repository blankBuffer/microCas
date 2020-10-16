# microCas
c++ computer algebra system. 3rd attempt. Solves, diffreciates, simplifies and integrates a wide variety of equations.

when starting the program in a terminal it gives you a choice of which program to choose. These include a polynomial solver the CAS and a grapher. 
The polynomial solver can solve any polynomial of any degree and find all the real solutions. It uses the property that the derivative of polynomials
have less solutions. Those solutions to the derivative equasion can be used as the bounds for the solutions. Then you can use bisection and newtons method to 
converge to those solutions. 

The rpn (reverse polish notation) cas (computer algebra system) is a quick way to solve, diffrenciate and simplify equstions. It usees many algebraic properties 
that I found useful to allow things to cancel out as much as possible and reduce an expression to a consistant form. For example a simple property is that (a^b)^c 
can be reduced to a^(b*c). This is useful if you have the case x^6-(x^2)^3 becomeing zero becuase they are equal. Many many other properties useful for simplifying
expressions have been implimented and are constatly changing to allow for more complicated expressiosn.

Some example problems that the CAS can handle include:

x+x -> 2*x
a^((x-y)/(z-w)) - a^((y-x)/(w-z)) -> 0
|x|^3 -> abs(x)*x^2
2^x*4^x -> 2^(3*x)
sqrt(12) -> sqrt(3)*2
a^(b/ln(a)) -> e^b
4*ln(3)+3*ln(7) -> log(27783)
ln(1000)/ln(100) -> 3/2
solve(x^2-3*x+1,x) -> [x=sqrt(5)*inv(-2)+3*inv(2),x=sqrt(5)*inv(2)+3*inv(2)]
d(ln(x+2^x)) -> inv(2^x+x)*d(x)*(1+log(2)*2^x)
...

The way equations are entered is that every function is operations is a key on the keyboard.
Help menu...

h : help
	s : print stack
	c : clear stack
	r : calculate result of last element on stack
	o : tells how many objects are in heap
	n : add integer to stack
	f : add float to stack
	p : pop last element from stack
	q : quit cas
	d : diffrenciate in terms of last element on stack
	3 : add pi to stack
	2 : add eulers number to stack
	v : add variable to stack, ending with . makes the variable constant
	+ : adds last two stack elements
	* : multiplies last two stack elements
	^ : exponentiates last element on stack
	- : multiplies last element by -1
	i : takes the inverse of last element
	l : takes the natural logarithm of last element
	> : swaps last two elements
	; : duplicates last element
	# : rolls stack
	/ : divide by last element on the stack
	w : take the square root of last element
	= : set last two elements on stack equal to each other
	u : solve in terms of last element on stack
	0 : get hash of last element
	| : take the absolute value of last element
	] : add last element on stack to list
	[ : make list
	z : make direction from left
	x : make direction from right
	6 : compare if two expressions are equal in structure
	4 : add negative infinity to stack
	5 : add infinity to stack
	t : substitute, note that the substitution function is always done first
	S : sine of last element on stack
	C : cosine of last element on stack
	I : integrate last element on stack
  
  
 To type in x^2-6*x=2
 
 v  (create a variable)
 x  (name given to variable)
 n  (create an integer)
 2  (give it a value of two)
 ^  (exponenciate x to two. this makes x^2)
 n  (create an integer)
 -6 (set the integr to -6)
 v  (create a variable)
 x  (give it the name x)
 *  (muliply the -6 and x to get -6*x)
 +  (add x^2 and -6*x to get x^2-6x)
 n  (create an integer)
 2  (give it a value of two)
 =  (set them equal to each other and get x^2-6*x=2)
 
 With practice you get very quick at typing expressions.
 
 Try typeing solve(ln(x+a)=pi,x)
 solution:
 v
 x
 v
 a
 +
 l
 3
 =
 v
 x
 u
 
 
