CppLox
======

A tree-walk interpreter for the lox language as described in
the book "Crafting Interpreters" written in C++.

It can execute lox scripts from a given file or can be launched in REPL mode:
```bash
lox <file-name> # Run from a file
lox             # Start the REPL
```

Additional features
-------------------
 - Strings can be compared lexicographically using the comparison operators
 - Ternary operator (`:?`)
 - `break` and `continue` statements
 - `assert` statement
 - Built-in functions `instance_of`, `sleep` and `string`.


Built-in Functions
------------------
`clock`: Returns the time since **January 1 1970, 00:00:00** (UNIX-epoch) in seconds  
`instance_of(<instance>, <class>)`: Check whether an instance is of a specific class.  
`sleep(<time-in-seconds>)`: Pause the execution of the script.  
`string(<expression>)`: Convert a Lox object to its string representation  


Examples
--------
### Fibonacci numbers
```
fun fib(n) {
	if (n <= 1) return 1;
	return fib(n - 1) + fib(n - 2);
}

for (var i = 0; i < 20; i = i + 1) {
	print fib(i);
}
```

### Inheritance example
```
class Metal {
	material() { print "Made of Iron"; }
}

class Box < Metal{
	shape() { print "Looks like a cube"; }
}

var metal_box = Box();
metal_box.material();
metal_box.shape();
```

### Up-counter (using closures)
```
fun make_counter(start) {
	var current = start;
	fun counter() {
		var ret = current;
		current = current + 1;
		return ret;
	}

	return counter;
}

var from_100 = make_counter(100);
print from_100();
print from_100();
```


Building
--------
From the project root directory run:

```bash
mkdir build && cd build
cmake ../
cmake --build .
```

It will output a file named `lox` (or `lox.exe` for windows) in the build directory.

You need a C++20 compiler and a build-system, both supported by CMake.
