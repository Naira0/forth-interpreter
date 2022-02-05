a crappy forth interpreter.

this implementation does not follow any specific standard because I wanted to experiment with it a bit.

some unique things with this implementation is how arrays and strings work.

for example if you want to print a string you can just do `"hello forth" .`

the interpreter will create a temporary string which can be printed or put in a variable like `"hello" variable str`

for numbers, you could do `1 2 3 3 composite variable nums`