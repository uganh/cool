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
    case TOKID(END):      return "EOF";
    case '(':             return "'('";
    case ')':             return "')'";
    case '*':             return "'*'";
    case '+':             return "'+'";
    case ',':             return "','";
    case '-':             return "'-'";
    case '.':             return "'.'";
    case '/':             return "'/'";
    case ':':             return "':'";
    case ';':             return "';'";
    case '<':             return "'<'";
    case '=':             return "'='";
    case '@':             return "'@'";
    case '{':             return "'{'";
    case '}':             return "'}'";
    case '~':             return "'~'";
    case TOKID(CASE):     return "CASE";
    case TOKID(CLASS):    return "CLASS";
    case TOKID(ELSE):     return "ELSE";
    case TOKID(ESAC):     return "ESAC";
    case TOKID(FALSE):    return "BOOL_CONST";
    case TOKID(FI):       return "FI";
    case TOKID(IF):       return "IF";
    case TOKID(IN):       return "IN";
    case TOKID(INHERITS): return "INHERITS";
    case TOKID(ISVOID):   return "ISVOID";
    case TOKID(LET):      return "LET";
    case TOKID(LOOP):     return "LOOP";
    case TOKID(NEW):      return "NEW";
    case TOKID(NOT):      return "NOT";
    case TOKID(OF):       return "OF";
    case TOKID(POOL):     return "POOL";
    case TOKID(THEN):     return "THEN";
    case TOKID(TRUE):     return "BOOL_CONST";
    case TOKID(WHILE):    return "WHILE";
    case TOKID(NUMBER):   return "INT_CONST";
    case TOKID(STRING):   return "STR_CONST";
    case TOKID(OBJECTID): return "OBJECTID";
    case TOKID(TYPEID):   return "TYPEID";
    case TOKID(ASSIGN):   return "ASSIGN";
    case TOKID(DARROW):   return "DARROW";
    case TOKID(LE):       return "LE";
    case TOKID(ERROR):    return "ERROR";
    default:              return "<Invalid Token>";
  }
}

void dump_token(std::ostream &out, int line, int token, yy::parser::value_type *yylval_ptr) {
  out << "#" << line << " " << token_to_string(token);

  switch (token) {
    case TOKID(TRUE):
      out << " true";
      break;
    case TOKID(FALSE):
      out << " false";
      break;
    case TOKID(NUMBER):
      out << " " << yylval_ptr->as<long>();
      break;
    case TOKID(STRING):
      out << " \"";
      print_escaped_string(out, yylval_ptr->as<std::string>().c_str());
      out << "\"";
      break;
    case TOKID(OBJECTID):
    case TOKID(TYPEID):
      out << " " << yylval_ptr->as<Symbol *>()->to_string();
      break;
    case TOKID(ERROR):
      out << " \"" << yylval_ptr->as<const char *>() << "\"";
      break;
  }

  out << std::endl;
}
