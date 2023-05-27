#include "token.hxx"
#include "expr.hxx"
#include "resolver.hxx"
#include "interpreter.hxx"

void Resolver::resolve_local(const Expr &expr, const Token &name)
{
	for (auto iter = scopes.crbegin(); iter != scopes.crend(); ++iter) {
		if (iter->contains(name.lexeme))
			interpreter.resolve(expr, iter - scopes.crbegin());
	}
}
