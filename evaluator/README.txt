1) reentrant pure-parser (Y)
2) read from string(Y), or file(Y) or stdin(Y)
	delete buffer(Y)
3) evaluate expression on user data(Y)
4) make it a library free from any business context(Y)
5) benchmark (Y)
	raw cost 10^-5 ms per evaluation
	ast cost 10^-4 ms per evaluation

ref: https://www.techtalk7.com/string-input-to-flex-lexer/
