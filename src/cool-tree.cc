#include "cool-tree.h"
#include "utilities.h"

static void dump_indents(std::ostream &stream, const std::vector<bool> &indents) {
  if (indents.size() > 1) {
    size_t limit = indents.size() - 1;
    for (size_t index = 0; index < limit; index++) {
      if (indents[index]) {
        stream << "  ";
      } else {
        stream << "| ";
      }
    }
  }

  if (indents.size() > 0) {
    if (indents.back()) {
      stream << "`-";
    } else {
      stream << "|-";
    }
  }
}

void Assign::dump(std::ostream &stream, std::vector<bool> &indents) const {

}

void Dispatch::dump(std::ostream &stream, std::vector<bool> &indents) const {

}

void Conditional::dump(std::ostream &stream, std::vector<bool> &indents) const {

}

void Loop::dump(std::ostream &stream, std::vector<bool> &indents) const {

}

void Block::dump(std::ostream &stream, std::vector<bool> &indents) const {

}

void Definition::dump(std::ostream &stream, std::vector<bool> &indents) const {

}

void Let::dump(std::ostream &stream, std::vector<bool> &indents) const {

}

void Branch::dump(std::ostream &stream, std::vector<bool> &indents) const {

}

void Case::dump(std::ostream &stream, std::vector<bool> &indents) const {

}

void New::dump(std::ostream &stream, std::vector<bool> &indents) const {

}

void IsVoid::dump(std::ostream &stream, std::vector<bool> &indents) const {

}

void Arithmetic::dump(std::ostream &stream, std::vector<bool> &indents) const {

}

void Complement::dump(std::ostream &stream, std::vector<bool> &indents) const {

}

void Comparison::dump(std::ostream &stream, std::vector<bool> &indents) const {

}

void Not::dump(std::ostream &stream, std::vector<bool> &indents) const {

}

void Object::dump(std::ostream &stream, std::vector<bool> &indents) const {

}

void Integer::dump(std::ostream &stream, std::vector<bool> &indents) const {

}

void String::dump(std::ostream &stream, std::vector<bool> &indents) const {

}

void Boolean::dump(std::ostream &stream, std::vector<bool> &indents) const {

}

void Attribute::dump(std::ostream &stream, std::vector<bool> &indents) const {

}

void Formal::dump(std::ostream &stream, std::vector<bool> &indents) const {

}

void Method::dump(std::ostream &stream, std::vector<bool> &indents) const {

}

void Class::dump(std::ostream &stream, std::vector<bool> &indents) const {

}

void Program::dump(std::ostream &stream) const {

}
