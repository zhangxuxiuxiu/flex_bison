/*
 * helper functions for purecalc
 */
# include <stdio.h>
# include <stdlib.h>
# include <stdarg.h>
# include <string.h>
# include <math.h>
# include "grammar.tab.h"
# include "grammar.lex.h"
# include "ast.h"

/* symbol table */
/* hash a symbol */
	static unsigned
symhash(const char *sym)
{
	unsigned int hash = 0;
	unsigned c;
	while( !!(c = *sym++) ) hash = hash*9 ^ c;
	return hash;
}

	struct symbol *
lookup(struct pcdata *pp, const char* sym)
{
	struct symbol *sp = &(pp->symtab)[symhash(sym)%NHASH];
	int scount = NHASH; /* how many have we looked at */
	while(--scount >= 0) {
		if(sp->name && !strcmp(sp->name, sym)) { return sp; }
		if(!sp->name) { /* new entry */
			sp->name = strdup(sym);
			sp->value = 0;
			sp->func = NULL;
			sp->syms = NULL;
			return sp;
		}
		if(++sp >= pp->symtab+NHASH) sp = pp->symtab; /* try the next entry */
	}
	yyerror(pp, "symbol table overflow\n");
	abort(); /* tried them all, table is full */
}

struct symbol * addsym(struct pcdata *pp,const char* sym, void *fp)
{
	struct symbol* s = lookup(pp, sym);
	s -> func = newfptr(pp, fp);
	return s;
}

	struct ast *
newast(struct pcdata *pp, int nodetype, struct ast *l, struct ast *r)
{
	struct ast *a = malloc(sizeof(struct ast));
	if(!a) {
		yyerror(pp, "out of space");
		exit(0);
	}
	a->nodetype = nodetype;
	a->l = l;
	a->r = r;
	return a;
}

	struct ast *
newnum(struct pcdata *pp, double d)
{
	struct numval *a = malloc(sizeof(struct numval));
	if(!a) {
		yyerror(pp, "out of space");
		exit(0);
	}
	a->nodetype = 'K';
	a->number = d;
	return (struct ast *)a;
}

	struct ast *
newfptr(struct pcdata *pp, void*p)
{
	struct fnptr *a = malloc(sizeof(struct fnptr));
	if(!a) {
		yyerror(pp, "out of space");
		exit(0);
	}
	a->nodetype = 'P';
	a->fp = p;
	return (struct ast *)a;
}

	struct ast *
newcmp(struct pcdata *pp, int cmptype, struct ast *l, struct ast *r)
{
	struct ast *a = malloc(sizeof(struct ast));
	if(!a) {
		yyerror(pp, "out of space");
		exit(0);
	}
	a->nodetype = '0' + cmptype;
	a->l = l;
	a->r = r;
	return a;
}

	struct ast *
newfunc(struct pcdata *pp, int functype, struct ast *l)
{
	struct fncall *a = malloc(sizeof(struct fncall));
	if(!a) {
		yyerror(pp, "out of space");
		exit(0);
	}
	a->nodetype = 'F';
	a->l = l;
	a->functype = functype;
	return (struct ast *)a;
}

	struct ast *
newcall(struct pcdata *pp, struct symbol *s, struct ast *l)
{
	struct ufncall *a = malloc(sizeof(struct ufncall));
	if(!a) {
		yyerror(pp, "out of space");
		exit(0);
	}
	a->nodetype = 'C';
	a->l = l;
	a->s = s;
	return (struct ast *)a;
}

	struct ast *
newref(struct pcdata *pp, struct symbol *s)
{
	struct symref *a = malloc(sizeof(struct symref));
	if(!a) {
		yyerror(pp, "out of space");
		exit(0);
	}
	a->nodetype = 'N';
	a->s = s;
	return (struct ast *)a;
}

	struct ast *
newasgn(struct pcdata *pp, struct symbol *s, struct ast *v)
{
	struct symasgn *a = malloc(sizeof(struct symasgn));
	if(!a) {
		yyerror(pp, "out of space");
		exit(0);
	}
	a->nodetype = '=';
	a->s = s;
	a->v = v;
	return (struct ast *)a;
}

	struct ast *
newflow(struct pcdata *pp, int nodetype, struct ast *cond, struct ast *tl, struct ast *el)
{
	struct flow *a = malloc(sizeof(struct flow));
	if(!a) {
		yyerror(pp, "out of space");
		exit(0);
	}
	a->nodetype = nodetype;
	a->cond = cond;
	a->tl = tl;
	a->el = el;
	return (struct ast *)a;
}

	struct symlist *
newsymlist(struct pcdata *pp, struct symbol *sym, struct symlist *next)
{
	struct symlist *sl = malloc(sizeof(struct symlist));
	if(!sl) {
		yyerror(pp, "out of space");
		exit(0);
	}
	sl->sym = sym;
	sl->next = next;
	return sl;
}

	void
free_symlist(struct pcdata *pp, struct symlist *sl)
{
	struct symlist *nsl;
	while(sl) {
		nsl = sl->next;
		free(sl);
		sl = nsl;
	}
}

/* define a function */
	void
dodef(struct pcdata *pp, struct symbol *name, struct symlist *syms, struct ast *func)
{
	if(name->syms) free_symlist(pp, name->syms);
	if(name->func) free_ast(pp, name->func);
	name->syms = syms;
	name->func = func;
}

static double callbuiltin(struct pcdata *pp, struct fncall *, void* u, double(*cvt)(void* fn, void* u));
static double calluser(struct pcdata *pp, struct ufncall *, void* u, double(*cvt)(void* fn, void* u));
double eval(struct pcdata *pp, struct ast *a, void* u, double(*cvt)(void* fn, void* u))
{
	double v;
	if(!a) {
		yyerror(pp, "internal error, null eval");
		return 0.0;
	}
	switch(a->nodetype) {
		/* constant */
		case 'K': v = ((struct numval *)a)->number; break;
		case 'P': v = (*cvt)(((struct fnptr *)a)->fp, u); break;
		case 'N': v = eval(pp, ((struct symref *)a)->s->func, u, cvt); break;
			  /* assignment */
		case '=': v = ((struct symasgn *)a)->s->value =
			  eval(pp, ((struct symasgn *)a)->v, u, cvt); break;
			  /* expressions */
		case '+': v = eval(pp, a->l, u, cvt) + eval(pp, a->r, u, cvt); break;
		case '-': v = eval(pp, a->l, u, cvt) - eval(pp, a->r, u, cvt); break;
		case '*': v = eval(pp, a->l, u, cvt) * eval(pp, a->r, u, cvt); break;
		case '/': v = eval(pp, a->l, u, cvt) / eval(pp, a->r, u, cvt); break;
		case '|': v = fabs(eval(pp, a->l, u, cvt)); break;
		case 'M': v = -eval(pp, a->l, u, cvt); break;
			  /* comparisons */
		case '1': v = (eval(pp, a->l, u, cvt) > eval(pp, a->r, u, cvt))? 1 : 0; break;
		case '2': v = (eval(pp, a->l, u, cvt) < eval(pp, a->r, u, cvt))? 1 : 0; break;
		case '3': v = (eval(pp, a->l, u, cvt) != eval(pp, a->r, u, cvt))? 1 : 0; break;
		case '4': v = (eval(pp, a->l, u, cvt) == eval(pp, a->r, u, cvt))? 1 : 0; break;
		case '5': v = (eval(pp, a->l, u, cvt) >= eval(pp, a->r, u, cvt))? 1 : 0; break;
		case '6': v = (eval(pp, a->l, u, cvt) <= eval(pp, a->r, u, cvt))? 1 : 0; break;
			  /* control flow */
			  /* null if/else/do expressions allowed in the grammar, so check for them */
		case 'I':
			  if( eval(pp, ((struct flow *)a)->cond, u, cvt) != 0) {
				  if( ((struct flow *)a)->tl) {
					  v = eval(pp, ((struct flow *)a)->tl, u, cvt);
				  } else
					  v = 0.0; /* a default value */
			  } else {
				  if( ((struct flow *)a)->el) {
					  v = eval(pp, ((struct flow *)a)->el, u, cvt);
				  } else
					  v = 0.0; /* a default value */
			  }
			  break;
		case 'W':
			  v = 0.0; /* a default value */
			  if( ((struct flow *)a)->tl) {
				  while( eval(pp, ((struct flow *)a)->cond, u, cvt) != 0)
					  v = eval(pp, ((struct flow *)a)->tl, u, cvt);
			  }
			  break; /* last value is value */
		case 'L': eval(pp, a->l, u, cvt); v = eval(pp, a->r, u, cvt); break;
		case 'F': v = callbuiltin(pp, (struct fncall *)a, u, cvt); break;
		case 'C': v = calluser(pp, (struct ufncall *)a, u, cvt); break;
		default: printf("internal error: bad node %c\n", a->nodetype);
	}
	return v;
}

	static double
callbuiltin(struct pcdata *pp, struct fncall *f, void* u, double(*cvt)(void* fn, void* u))
{
	enum bifs functype = f->functype;
	double v = eval(pp, f->l, u, cvt);
	switch(functype) {
		case B_sqrt:
			return sqrt(v);
		case B_exp:
			return exp(v);
		case B_log:
			return log(v);
		case B_print:
			printf("= %4.4g\n", v);
			return v;
		default:
			yyerror(pp, "Unknown built-in function %d", functype);
			return 0.0;
	}
}

	static double
calluser(struct pcdata *pp, struct ufncall *f,void* u, double(*cvt)(void* fn, void* u))
{
	struct symbol *fn = f->s; /* function name */
	struct symlist *sl; /* dummy arguments */
	struct ast *args = f->l; /* actual arguments */
	double *oldval, *newval; /* saved arg values */
	double v;
	int nargs;
	int i;
	if(!fn->func) {
		yyerror(pp, "call to undefined function", fn->name);
		return 0;
	}
	/* count the arguments */
	sl = fn->syms;
	for(nargs = 0; sl; sl = sl->next)
		nargs++;
	/* prepare to save them */
	oldval = (double *)malloc(nargs * sizeof(double));
	newval = (double *)malloc(nargs * sizeof(double));
	if(!oldval || !newval) {
		yyerror(pp, "Out of space in %s", fn->name); return 0.0;
	}
	/* evaluate the arguments */
	for(i = 0; i < nargs; i++) {
		if(!args) {
			yyerror(pp, "too few args in call to %s", fn->name);
			free(oldval); free(newval);
			return 0;
		}
		if(args->nodetype == 'L') { /* if this is a list node */
			newval[i] = eval(pp, args->l, u, cvt);
			args = args->r;
		} else { /* if it's the end of the list */
			newval[i] = eval(pp, args, u, cvt);
			args = NULL;
		}
	}
	/* save old values of dummies, assign new ones */
	sl = fn->syms;
	for(i = 0; i < nargs; i++) {
		struct symbol *s = sl->sym;
		oldval[i] = s->value;
		s->value = newval[i];
		sl = sl->next;
	}
	free(newval);
	/* evaluate the function */
	v = eval(pp, fn->func, u, cvt);
	/* put the dummies back */
	sl = fn->syms;
	for(i = 0; i < nargs; i++) {
		struct symbol *s = sl->sym;
		s->value = oldval[i];
		sl = sl->next;
	}
	free(oldval);
	return v;
}

void free_symbol(struct pcdata* p, struct symbol* s){
	if(!s) return;	
	if(s->name) free(s->name);
	if(s->func) free_ast(p, s->func);
	if(s->syms) free_symlist(p, s->syms);
}

	void
free_ast(struct pcdata *pp, struct ast *a)
{
	switch(a->nodetype) {
		/* two subtrees */
		case '+':
		case '-':
		case '*':
		case '/':
		case '1': case '2': case '3': case '4': case '5': case '6':
		case 'L':
			free_ast(pp, a->r);
			/* one subtree */
		case '|':
		case 'M': case 'C': case 'F':
			free_ast(pp, a->l);
			/* no subtree */
		case 'K': case 'N': case 'P':
			break;
		case '=':
			free( ((struct symasgn *)a)->v);
			break;
		case 'I': case 'W':
			free( ((struct flow *)a)->cond);
			if( ((struct flow *)a)->tl) free( ((struct flow *)a)->tl);
			if( ((struct flow *)a)->el) free( ((struct flow *)a)->el);
			break;
		default: printf("internal error: free bad node %c\n", a->nodetype);
	}
	free(a); /* always free the node itself */
}

