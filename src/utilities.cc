#include "utilities.h"

#include <cctype>
#include <iomanip>

void print_escaped_string(std::ostream &out, const char *str) {
  while (char c = *str++) {
    switch (c) {
      case '\\': out << "\\\\"; break;
      case '\"': out << "\\\""; break;
      case '\a': out << "\\a";  break;
      case '\b': out << "\\b";  break;
      case '\f': out << "\\f";  break;
      case '\n': out << "\\n";  break;
      case '\r': out << "\\r";  break;
      case '\t': out << "\\t";  break;
      case '\v': out << "\\v";  break;
      default:
        if (std::isprint(c)) {
          out << c;
        }
        else {
          out << '\\' << std::ios::hex << std::setfill('0') << std::setw(2)
              << static_cast<int>(static_cast<unsigned char>(c))
              << std::ios::dec << std::setfill(' ');
        }
        break;
    }
  }
}

const char *token_to_string(int token) {
  switch (token) {
    case 0:           return "EOF";
    case '(':         return "'('";
    case ')':         return "')'";
    case '*':         return "'*'";
    case '+':         return "'+'";
    case ',':         return "','";
    case '-':         return "'-'";
    case '.':         return "'.'";
    case '/':         return "'/'";
    case ':':         return "':'";
    case ';':         return "';'";
    case '<':         return "'<'";
    case '=':         return "'='";
    case '@':         return "'@'";
    case '{':         return "'{'";
    case '}':         return "'}'";
    case '~':         return "'~'";
    case TK_CASE:     return "CASE";
    case TK_CLASS:    return "CLASS";
    case TK_ELSE:     return "ELSE";
    case TK_ESAC:     return "ESAC";
    case TK_FALSE:    return "BOOL_CONST";
    case TK_FI:       return "FI";
    case TK_IF:       return "IF";
    case TK_IN:       return "IN";
    case TK_INHERITS: return "INHERITS";
    case TK_ISVOID:   return "ISVOID";
    case TK_LET:      return "LET";
    case TK_LOOP:     return "LOOP";
    case TK_NEW:      return "NEW";
    case TK_NOT:      return "NOT";
    case TK_OF:       return "OF";
    case TK_POOL:     return "POOL";
    case TK_THEN:     return "THEN";
    case TK_TRUE:     return "BOOL_CONST";
    case TK_WHILE:    return "WHILE";
    case TK_NUMBER:   return "INT_CONST";
    case TK_STRING:   return "STR_CONST";
    case TK_OBJECTID: return "OBJECTID";
    case TK_TYPEID:   return "TYPEID";
    case TK_ASSIGN:   return "ASSIGN";
    case TK_DARROW:   return "DARROW";
    case TK_LE:       return "LE";
    case TK_ERROR:    return "ERROR";
    default:          return "<Invalid Token>";
  }
}

void dump_token(std::ostream &out, int lineno, int token, YYSTYPE *yylval_ptr) {
  out << "#" << lineno << " " << token_to_string(token);

  switch (token) {
    case TK_TRUE:
      out << " true";
      break;
    case TK_FALSE:
      out << " false";
      break;
    case TK_NUMBER:
      out << " " << yylval_ptr->ival;
    case TK_STRING:
      // TODO
      break;
    case TK_OBJECTID:
    case TK_TYPEID:
      // TODO
      break;
    case TK_ERROR:
      out << " \"" << yylval_ptr->errs << "\"";
      break;
  }

  out << std::endl;
}
