#include "lex.h"
#include "var.h"

std::ostream& operator<<(std::ostream& os, token const& tok)
{
    return os << "<tok: " << tok.desc() << ' ' << int(tok.type()) << ' '
              << tok.pos().line << ':' << tok.pos().chr
              << " \"" << tok.str() << '"' << ">";
}

tokstream::~tokstream()
{
}

char
tokstream::getchar()
{
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
tokstream::print(std::ostream& os) const
{
    os << '[' << toks.size() << ':';
    for (int i = 0; i < (int) toks.size(); i++) {
        os << (i - 1) << " " << toks[i] << ' ';
        if (i == 0) os << "| ";
    }
    os << ']';
}

token
tokstream::operator[](int i)
{
   unsigned index = i + 1; // we allow -1 to mean previous tok.
   while (index >= toks.size()) {
       load();
   }
   return toks[index];
}

void
tokstream::consume()
{
   toks.pop_front();
}

bool
validnumber(std::string s)
{
    char *ep;
    (void) std::strtoul(s.c_str(), &ep, 0);
    return (*ep == 0);
}

number
readnumber(std::string s)
{
    return std::strtoul(s.c_str(), 0, 0);
}

token
lexer::fetchnext()
{
    s_ = "";
    for (;;) {
        char ch = getchar();
            
        if (eof()) break;
        if (ch == '\n' || ch == '\r') {
            continue;
        }
        if (std::isspace(ch))
            continue;
        if (ch == '"') {
            ch = getchar();
            while (!eof() && ch != '"') {
                s_.push_back(ch);
                ch = getchar();
            }
            if (eof()) return token(eoftok, "", pos(), "eof");
            return token(str, s_, pos(), "str");
        }
        switch (ch) {
        case ';':
        case '=':
        case ',':
        case '[': case ']':
        case '(': case ')':
            return token(ch, s_, pos(), ch);
        }
        if (ch == '.') {
            ch = getchar();
            if (ch == '.') return token(dotdot, s_, pos(), "..");
            putback();
            return token('.', s_, pos(), '.');
        }
        if (std::isalnum(ch)) {
            do {
                s_.push_back(ch);
                ch = getchar();
            } while (isalnum(ch));
            putback();
            if (s_ == "crc") return token(crc16, s_, pos(), "crc16");
            if (s_ == "write") return token(write, s_, pos(), "write");

            if (validnumber(s_)) return token(num, s_, pos(), "num");

            return token(name, s_, pos(), "name");
        }
        s_.push_back(ch);
        return token(err, s_, pos(), "err");
    }
    return token(eoftok, s_, pos(), "tok");
}
