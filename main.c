#include "ast.h"

int yyparse(void);
extern FILE *yyin;
extern Node *root;

Node *mk(char *kind, Node *a, Node *b, Node *c) {
    Node *n = calloc(1, sizeof(Node));
    n->kind = kind; n->a = a; n->b = b; n->c = c;
    return n;
}
Node *num(int v) { Node *n = mk("num", 0, 0, 0); n->val = v; return n; }
Node *var(char *kind, char *name, Node *a) { Node *n = mk(kind, a, 0, 0); n->name = name; return n; }

/* ---------- interpreter ---------- */
char *names[100]; int vals[100]; int nvars = 0;

int *lookup(char *name, int create) {
    for (int i = 0; i < nvars; i++) if (!strcmp(names[i], name)) return &vals[i];
    if (!create) { fprintf(stderr, "Undeclared variable %s\n", name); exit(1); }
    names[nvars] = name; vals[nvars] = 0;
    return &vals[nvars++];
}

int eval(Node *n) {
    char *k = n->kind;
    if (!strcmp(k, "num")) return n->val;
    if (!strcmp(k, "var")) return *lookup(n->name, 0);
    if (!strcmp(k, "neg")) return -eval(n->a);
    if (!strcmp(k, "!")) return !eval(n->a);
    if (!strcmp(k, "&&")) return eval(n->a) && eval(n->b);
    if (!strcmp(k, "||")) return eval(n->a) || eval(n->b);
    int x = eval(n->a), y = eval(n->b);
    if (!strcmp(k, "+")) return x + y;
    if (!strcmp(k, "-")) return x - y;
    if (!strcmp(k, "*")) return x * y;
    if (!strcmp(k, "/")) { if (!y) { fprintf(stderr, "Division by zero\n"); exit(1); } return x / y; }
    if (!strcmp(k, "%")) return x % y;
    if (!strcmp(k, "<")) return x < y;
    if (!strcmp(k, ">")) return x > y;
    if (!strcmp(k, "<=")) return x <= y;
    if (!strcmp(k, ">=")) return x >= y;
    if (!strcmp(k, "==")) return x == y;
    if (!strcmp(k, "!=")) return x != y;
    return 0;
}

void exec(Node *n) {
    for (; n; n = n->next) {
        char *k = n->kind;
        if (!strcmp(k, "program") || !strcmp(k, "block")) exec(n->a);
        else if (!strcmp(k, "decl")) *lookup(n->name, 1) = n->a ? eval(n->a) : 0;
        else if (!strcmp(k, "=")) *lookup(n->name, 0) = eval(n->a);
        else if (!strcmp(k, "print")) printf("%d\n", eval(n->a));
        else if (!strcmp(k, "if")) { if (eval(n->a)) exec(n->b); else if (n->c) exec(n->c); }
        else if (!strcmp(k, "while")) while (eval(n->a)) exec(n->b);
    }
}

/* ---------- code generation (Python) ---------- */
void gen_expr(FILE *f, Node *n) {
    char *k = n->kind;
    if (!strcmp(k, "num")) fprintf(f, "%d", n->val);
    else if (!strcmp(k, "var")) fprintf(f, "%s", n->name);
    else if (!strcmp(k, "neg")) { fprintf(f, "(-"); gen_expr(f, n->a); fprintf(f, ")"); }
    else if (!strcmp(k, "!")) { fprintf(f, "int(not "); gen_expr(f, n->a); fprintf(f, ")"); }
    else if (!strcmp(k, "/")) { fprintf(f, "int("); gen_expr(f, n->a); fprintf(f, " / "); gen_expr(f, n->b); fprintf(f, ")"); }
    else {
        char *op = !strcmp(k, "&&") ? "and" : !strcmp(k, "||") ? "or" : k;
        int cmp = strchr("<>=!&|", k[0]) != 0;
        fprintf(f, cmp ? "int(" : "(");
        gen_expr(f, n->a); fprintf(f, " %s ", op); gen_expr(f, n->b);
        fprintf(f, ")");
    }
}

void gen(FILE *f, Node *n, int ind);

void gen_body(FILE *f, Node *n, int ind) {
    if (!strcmp(n->kind, "block") && n->a) gen(f, n->a, ind);
    else if (!strcmp(n->kind, "block")) fprintf(f, "%*spass\n", ind * 4, "");
    else { Node *nx = n->next; n->next = 0; gen(f, n, ind); n->next = nx; }
}

void gen(FILE *f, Node *n, int ind) {
    for (; n; n = n->next) {
        char *k = n->kind;
        if (!strcmp(k, "program")) { gen(f, n->a, ind); continue; }
        if (!strcmp(k, "block")) { gen_body(f, n, ind); continue; }
        fprintf(f, "%*s", ind * 4, "");
        if (!strcmp(k, "decl") || !strcmp(k, "=")) {
            fprintf(f, "%s = ", n->name);
            if (n->a) gen_expr(f, n->a); else fprintf(f, "0");
            fprintf(f, "\n");
        } else if (!strcmp(k, "print")) {
            fprintf(f, "print("); gen_expr(f, n->a); fprintf(f, ")\n");
        } else if (!strcmp(k, "if")) {
            fprintf(f, "if "); gen_expr(f, n->a); fprintf(f, ":\n");
            gen_body(f, n->b, ind + 1);
            if (n->c) { fprintf(f, "%*selse:\n", ind * 4, ""); gen_body(f, n->c, ind + 1); }
        } else if (!strcmp(k, "while")) {
            fprintf(f, "while "); gen_expr(f, n->a); fprintf(f, ":\n");
            gen_body(f, n->b, ind + 1);
        }
    }
}

/* ---------- AST -> JSON + HTML ---------- */
void json(FILE *f, Node *n) {
    if (!strcmp(n->kind, "num")) fprintf(f, "{\"name\":\"%d\"", n->val);
    else if (n->name) fprintf(f, "{\"name\":\"%s %s\"", n->kind, n->name);
    else fprintf(f, "{\"name\":\"%s\"", n->kind);
    fprintf(f, ",\"children\":[");
    int first = 1;
    Node *kids[3] = {n->a, n->b, n->c};
    for (int i = 0; i < 3; i++) {
        Node *list = kids[i];
        int isList = (i == 0 && (!strcmp(n->kind, "program") || !strcmp(n->kind, "block")));
        for (Node *k = list; k; k = isList ? k->next : 0) {
            if (!first) fprintf(f, ",");
            json(f, k); first = 0;
        }
    }
    fprintf(f, "]}");
}

void html(Node *n) {
    FILE *t = fopen("template.html", "r"), *f = fopen("ast.html", "w");
    char line[1000];
    while (fgets(line, sizeof line, t)) {
        if (strstr(line, "AST_DATA")) { fprintf(f, "const data = "); json(f, n); fprintf(f, ";\n"); }
        else fputs(line, f);
    }
    fclose(t); fclose(f);
}

int main(int argc, char **argv) {
    if (argc > 1) yyin = fopen(argv[1], "r");
    yyparse();

    printf("=== Result ===\n");
    exec(root);

    FILE *py = fopen("out.py", "w");
    gen(py, root, 0);
    fclose(py);
    printf("=== Python code written to out.py ===\n");

    html(root);
    printf("=== AST written to ast.html ===\n");
    return 0;
}
