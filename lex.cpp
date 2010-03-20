#include "lex.h"
#include "var.h"

using std::string;

namespace memory {
    std::ostream& operator<<(std::ostream& os, token const& tok) {
        return os << "<tok: " << tok.desc() << ' ' << int(tok.type()) << ' '
                  << tok.pos()
                  << " \"" << tok.str() << '"' << ">";
    }

    tokstream::~tokstream() {
    }

    void
    tokstream::print(std::ostream& os) const {
        os << '[' << toks_.size() << ':';
        for (int i = 0; i < (int) toks_.size(); i++) {
            os << std::dec << (i - 1) << " " << toks_[i] << ' ';
            if (i == 0) os << "| ";
        }
        os << ']';
    }

    token
    tokstream::operator[](int i) {
       unsigned index = i + 1; // we allow -1 to mean previous tok.
       while (index >= toks_.size()) {
           load();
       }
       return toks_[index];
    }

    void
    tokstream::consume() {
       toks_.pop_front();
    }

    void
    tokstream::consumeuntil(int t) {
        while (toks_[0].type() != t && toks_[0].type() != eof)
            toks_.pop_front();
    }

    int
    lexer::getchar() {
        char ch = 0;
        is_ >> std::noskipws >> ch;
        pos_.chr++;
        if (ch == '\n') {
            pos_.line++;
            pos_.chr = 0;
        }
        return is_ ? ch : -1;
    }

    token
    lexer::next() {
        for (;;) {
            char ch = getchar();
            if (!is_) break;
            if (ch == '\n' || ch == '\r') {
                continue;
            }
            if (std::isspace(ch))
                continue;
            if (ch == '#') {
                do
                    ch = getchar();
                while (is_ && ch != '\n');
                continue;
            }
            if (ch == '"') {
                string s = "";
                ch = getchar();
                while (is_ && ch != '"') {
                    s.push_back(ch);
                    ch = getchar();
                }
                if (!is_) return token(tokstream::eof, "", pos_, "eof");
                return token(str, s, pos_, "str");
            }
            switch (ch) {
            case ';':
            case '=':
            case ',':
            case '+':
            case '-':
            case '*':
            case '/':
            case '[': case ']':
            case '(': case ')':
            case '{': case '}':
                return token(ch, "", pos_, ch);
            }
            if (ch == '.') {
                ch = getchar();
                if (ch == '.') return token(dotdot, "", pos_, "..");
                is_.putback(ch);
                return token('.', "", pos_, '.');
            }
            string s;
            if (std::isalnum(ch) || ch == '_') {
                do {
                    s.push_back(ch);
                    ch = getchar();
                } while (is_ && (isalnum(ch) || ch == '_'));
                if (is_) is_.putback(ch);

                if (validnumber(s)) return token(num, s, pos_, "num");
                if (s == "fn") return token(fn, s, pos_, "fn");
                if (s == "if") return token(iftok, s, pos_, "if");
                if (s == "else") return token(elsetok, s, pos_, "else");
                if (s == "while") return token(whiletok, s, pos_, "if");

                return token(name, s, pos_, "name");
            }
            s.push_back(ch);
            throw lexer_error(string("didn't expect: \"") + s + "\"");
        }
        return token(tokstream::eof, "", pos_, "eof");
    }

    bool
    validnumber(string const& s) {
        char *ep;
        (void) std::strtoul(s.c_str(), &ep, 0);
        return (*ep == 0);
    }

    number
    readnumber(string const& s) {
        return std::strtoul(s.c_str(), 0, 0);
    }
}
