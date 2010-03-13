#include "lex.h"
#include "var.h"

namespace memory {

using std::string;

std::ostream& operator<<(std::ostream& os, token const& tok) {
    return os << "<tok: " << tok.desc() << ' ' << int(tok.type()) << ' '
              << tok.pos().line << ':' << tok.pos().chr
              << " \"" << tok.str() << '"' << ">";
}

tokstream::~tokstream() {
}

int
tokstream::getchar() {
    if (!is_) return -1;
    is_ >> std::noskipws;
    is_ >> ch_;
    pos_.chr++;
    if (ch_ == '\n') {
        pos_.line++;
        pos_.chr = 0;
    }
    return ch_;
}

void
tokstream::print(std::ostream& os) const {
    os << '[' << toks.size() << ':';
    for (int i = 0; i < (int) toks.size(); i++) {
        os << std::dec << (i - 1) << " " << toks[i] << ' ';
        if (i == 0) os << "| ";
    }
    os << ']';
}

token
tokstream::operator[](int i) {
   unsigned index = i + 1; // we allow -1 to mean previous tok.
   while (index >= toks.size()) {
       load();
   }
   return toks[index];
}

void
tokstream::consume() {
   toks.pop_front();
}

bool
validnumber(string s) {
    char *ep;
    (void) std::strtoul(s.c_str(), &ep, 0);
    return (*ep == 0);
}

number
readnumber(string s) {
    return std::strtoul(s.c_str(), 0, 0);
}

token
lexer::fetchnext() {
    for (;;) {
        char ch = getchar();
            
        if (eof()) break;
        if (ch == '\n' || ch == '\r') {
            continue;
        }
        if (std::isspace(ch))
            continue;
        if (ch == '#') {
            do
                ch = getchar();
            while (!eof() && ch != '\n');
            continue;
        }
        if (ch == '"') {
            string s = "";
            ch = getchar();
            while (!eof() && ch != '"') {
                s.push_back(ch);
                ch = getchar();
            }
            if (eof()) return token(eoftok, "", pos(), "eof");
            return token(str, s, pos(), "str");
        }
        switch (ch) {
        case ';':
        case '=':
        case ',':
        case '[': case ']':
        case '(': case ')':
        case '{': case '}':
            return token(ch, "", pos(), ch);
        }
        if (ch == '.') {
            ch = getchar();
            if (ch == '.') return token(dotdot, "", pos(), "..");
            putback();
            return token('.', "", pos(), '.');
        }
        string s;
        if (std::isalnum(ch) || ch == '_') {
            do {
                s.push_back(ch);
                ch = getchar();
            } while ((isalnum(ch) || ch == '_') && !eof());
            if (!eof()) putback();

            if (validnumber(s)) return token(num, s, pos(), "num");

            return token(name, s, pos(), "name");
        }
        s.push_back(ch);
        throw lexer_error(string("didn't expect: ") + s);
    }
    return token(eoftok, "", pos(), "eof");
}

} // namespace
