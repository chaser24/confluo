#ifndef DIALOG_FILTER_EXPRESSION_H_
#define DIALOG_FILTER_EXPRESSION_H_

#include <cstdio>
#include <cstdlib>

#include <string>
#include <sstream>

#include "exceptions.h"
#include "relational_ops.h"

// TODO: Improve documentation

namespace dialog {

/**
 * Expression types.
 */
enum expression_id
  : uint8_t {
    AND,     //!< AND (&&)
  OR,      //!< OR  (|)
  NOT,     //!< NOT (!)
  PREDICATE     //!< PREDICATE
};

/**
 * Generic expression.
 */
struct expression_t {
  expression_t(expression_id eid)
      : id(eid) {
  }

  expression_id id;
};

/**
 * Predicate expression.
 */
struct predicate_t : public expression_t {
  predicate_t()
      : expression_t(expression_id::PREDICATE),
        op() {
  }

  predicate_t(const predicate_t& other)
      : expression_t(other.id),
        attr(other.attr),
        op(other.op),
        value(other.value) {
  }

  predicate_t& operator=(const predicate_t& other) {
    id = other.id;
    attr = other.attr;
    op = other.op;
    value = other.value;
    return *this;
  }

  bool operator<(const predicate_t &right) const {
    return to_string() < right.to_string();
  }

  std::string to_string() const {
    return attr + relop_utils::op_to_str(op) + value;
  }

  std::string attr;   // Attribute name
  relop_id op;     // Operation
  std::string value;  // Attribute value
};

/**
 * Conjunction expression.
 */
struct conjunction_t : public expression_t {
  conjunction_t(expression_t* l, expression_t* r)
      : expression_t(expression_id::AND),
        left(l),
        right(r) {
  }

  expression_t* left;
  expression_t* right;
};

/**
 * Disjunction expression.
 */
struct disjunction_t : public expression_t {
  disjunction_t(expression_t* l, expression_t* r)
      : expression_t(expression_id::OR),
        left(l),
        right(r) {
  }

  expression_t* left;
  expression_t* right;
};

/**
 * Negation expression.
 */
struct negation : public expression_t {
  negation()
      : expression_t(expression_id::NOT) {
    child = nullptr;
  }
  expression_t* child;
};

/**
 * Expression utilities.
 */
class expression_utils {
 public:

  /**
   * Prints out a given expression tree.
   *
   * @param exp The expression tree to print.
   */
  static void print_expression(expression_t* exp) {
    switch (exp->id) {
      case expression_id::PREDICATE: {
        predicate_t *p = (predicate_t*) exp;
        fprintf(stderr, "[%s %s %s]", p->attr.c_str(),
                relop_utils::op_to_str(p->op).c_str(), p->value.c_str());
        break;
      }
      case expression_id::NOT: {
        negation *n = (negation*) exp;
        fprintf(stderr, "NOT(");
        print_expression(n->child);
        fprintf(stderr, ")");
        break;
      }
      case expression_id::AND: {
        conjunction_t *c = (conjunction_t*) exp;
        fprintf(stderr, "AND(");
        print_expression(c->left);
        fprintf(stderr, ", ");
        print_expression(c->right);
        fprintf(stderr, ")");
        break;
      }
      case expression_id::OR: {
        disjunction_t *d = (disjunction_t*) exp;
        fprintf(stderr, "OR(");
        print_expression(d->left);
        fprintf(stderr, ", ");
        print_expression(d->right);
        fprintf(stderr, ")");
        break;
      }
    }
  }

  /**
   * Frees a given expression tree.
   *
   * @param exp Expression tree to free.
   */
  static void free_expression(expression_t* exp) {
    switch (exp->id) {
      case expression_id::PREDICATE: {
        predicate_t *p = (predicate_t*) exp;
        delete p;
        break;
      }
      case expression_id::NOT: {
        negation *n = (negation*) exp;
        free_expression(n->child);
        delete n;
        break;
      }
      case expression_id::AND: {
        conjunction_t *c = (conjunction_t*) exp;
        free_expression(c->left);
        free_expression(c->right);
        delete c;
        break;
      }
      case expression_id::OR: {
        disjunction_t *d = (disjunction_t*) exp;
        free_expression(d->left);
        free_expression(d->right);
        delete d;
        break;
      }
    }
  }
};

/**
 * Token generated by lexer.
 */
struct expression_lex_token {
  expression_lex_token(const int i, const std::string& val)
      : id(i),
        value(val) {
  }

  const int id;
  const std::string value;
};

/**
 * Lexer for the expression.
 */
class expression_lexer {
 public:
  static const int INVALID = -2;
  static const int END = -1;
  // Boolean
  static const int OR = 0;
  static const int AND = 1;
  static const int NOT = 2;
  // Parentheses
  static const int LEFT = 3;
  static const int RIGHT = 4;
  // Operators
  static const int OPERATOR = 5;
  // Operands
  static const int OPERAND = 6;

  expression_lexer() {
  }

  expression_lexer(const std::string& exp) {
    str(exp);
  }

  void str(const std::string& exp) {
    stream_.str(exp);
  }

  size_t pos() {
    return stream_.tellg();
  }

  std::string str() {
    return stream_.str();
  }

  const expression_lex_token next_token() {
    while (iswspace(stream_.peek()))
      stream_.get();

    char c = stream_.get();
    switch (c) {
      case EOF:
        return expression_lex_token(END, "");
      case '|': {
        if (stream_.get() != '|')
          THROW(parse_exception,
              "Invalid token starting with |; did you mean ||?");
        return expression_lex_token(OR, "||");
      }
      case '&': {
        if (stream_.get() != '&')
          THROW(parse_exception,
              "Invalid token starting with &; did you mean &&?");
        return expression_lex_token(AND, "&&");
      }
      case '!': {
        char c1 = stream_.get();
        if (c1 == '=')
          return expression_lex_token(OPERATOR, "!=");
        if (c1 == 'i') {
          if (stream_.get() == 'n' && iswspace(stream_.peek()))
            return expression_lex_token(OPERATOR, "!in");
          stream_.unget();
        }
        stream_.unget();
        return expression_lex_token(NOT, "!");
      }
      case '(':
        return expression_lex_token(LEFT, "(");
      case ')':
        return expression_lex_token(RIGHT, ")");
      case '=': {
        if (stream_.get() != '=')
          THROW(parse_exception,
              "Invalid token starting with =; did you mean ==?");
        return expression_lex_token(OPERATOR, "==");
      }
      case '<': {
        if (stream_.get() == '=')
          return expression_lex_token(OPERATOR, "<=");
        stream_.unget();
        return expression_lex_token(OPERATOR, "<");
      }
      case '>': {
        if (stream_.get() == '=')
          return expression_lex_token(OPERATOR, ">=");
        stream_.unget();
        return expression_lex_token(OPERATOR, ">");
      }
      default: {
        if (!opvalid(c))
          THROW(parse_exception, "All operands must conform to [a-zA-Z0-9_.]+");

        stream_.unget();
        std::string operand = "";
        while (opvalid(stream_.peek()))
          operand += (char) stream_.get();
        return expression_lex_token(OPERAND, operand);
      }
    }
    return expression_lex_token(INVALID, "");
  }

  const expression_lex_token peek_token() {
    const expression_lex_token tok = next_token();
    put_back(tok);
    return tok;
  }

  void put_back(const expression_lex_token& tok) {
    for (size_t i = 0; i < tok.value.size(); i++) {
      stream_.unget();
    }
  }

 private:
  bool opvalid(int c) {
    return isalnum(c) || c == '.' || c == '_' || c == '-';
  }

  std::stringstream stream_;
};

/**
 * Parser for the expression.
 */
class expression_parser {
 public:
  expression_parser(const std::string& exp) {
    lex_.str(exp);
  }

  expression_t* parse() {
    expression_t* e = exp();
    if (lex_.next_token().id != expression_lexer::END)
      THROW(parse_exception, "Parsing ended prematurely");
    return e;
  }

 private:
  expression_t* exp() {
    expression_t* t = term();
    if (lex_.peek_token().id != expression_lexer::OR)
      return t;
    lex_.next_token();
    return new disjunction_t(t, exp());
  }

  expression_t* term() {
    expression_t* f = factor();
    if (lex_.peek_token().id != expression_lexer::AND)
      return f;
    lex_.next_token();
    return new conjunction_t(f, term());
  }

  expression_t* factor() {
    expression_lex_token tok = lex_.next_token();
    if (tok.id == expression_lexer::NOT) {
      return negate(factor());
    } else if (tok.id == expression_lexer::LEFT) {
      expression_t *e = exp();
      expression_lex_token right = lex_.next_token();
      if (right.id != expression_lexer::RIGHT)
        THROW(parse_exception, "Could not find matching right parenthesis");
      return e;
    } else if (tok.id == expression_lexer::OPERAND) {
      predicate_t *p = new predicate_t;
      p->attr = tok.value;
      expression_lex_token op = lex_.next_token();
      if (op.id != expression_lexer::OPERATOR)
        THROW(parse_exception,
            "First operand must be followed by operator in all predicates");
      p->op = relop_utils::str_to_op(op.value);
      expression_lex_token operand = lex_.next_token();
      if (operand.id != expression_lexer::OPERAND)
        THROW(parse_exception,
            "Operand must be followed by operator in all predicates");
      p->value = operand.value;
      return p;
    } else {
      THROW(parse_exception, "Unexpected token " + tok.value);
    }
  }

  expression_t* negate(expression_t* exp) {
    switch (exp->id) {
      case expression_id::AND: {
        conjunction_t* c = (conjunction_t*) exp;
        disjunction_t* d = new disjunction_t(negate(c->left), negate(c->right));
        delete c;
        return d;
      }
      case expression_id::OR: {
        disjunction_t* d = (disjunction_t*) exp;
        conjunction_t* c = new conjunction_t(negate(d->left), negate(d->right));
        delete d;
        return c;
      }
      case expression_id::NOT: {
        negation* n = (negation*) exp;
        expression_t* e = n->child;
        delete n;
        return e;
      }
      case expression_id::PREDICATE: {
        predicate_t* p = (predicate_t *) exp;
        p->op = negate(p->op);
        return p;
      }
      default:
        return nullptr;
    }
  }

  relop_id negate(const relop_id& op) {
    switch (op) {
      case relop_id::EQ:
        return relop_id::NEQ;
      case relop_id::NEQ:
        return relop_id::EQ;
      case relop_id::LT:
        return relop_id::GE;
      case relop_id::GT:
        return relop_id::LE;
      case relop_id::LE:
        return relop_id::GT;
      case relop_id::GE:
        return relop_id::LT;
      default:
        THROW(parse_exception, "Could not negate: invalid operator");
    }
  }

  expression_lexer lex_;
};

}

#endif /* DIALOG_FILTER_EXPRESSION_H_ */