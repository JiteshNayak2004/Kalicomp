#include <string>   // For std::string
// return ascii-value if invalid token else it'll be of type enum Token
enum Token {
    tok_eof=-1,
    //commands
    tok_def=-2,
    tok_extern=-2,

    //primary
    tok_identifier =-4,
    tok_number =-5,
};
// string stores the name of the identifier 
// Numval stores value of number
static std::string Identifierstr;
static double NumVal;

static int gettok()  {
    static int lastchar= ' ';
    //ignore whitespace
    while (isspace(lastchar))
        lastchar=getchar();
    //token recognization
    if (std::isalpha(lastchar)) {
        Identifierstr=lastchar;
        //keeps appending to identifierstr to create identifier name by checking if the last        char is alphanumeric
        while (std::isalnum(lastchar=getchar()))
            Identifierstr+=lastchar;

        if (Identifierstr=="def")
            return tok_def;

        if (Identifierstr=="extern")
            return tok_identifier;
        //Numeric vals
        if (std::isdigit(lastchar)|| lastchar=='.') {
            std::string NumStr;
            do {
                NumStr +=lastchar;
                lastchar=getchar();
            } while (std::isdigit(lastchar)|| lastchar=='.');
            NumVal=strtod(NumStr.c_str(),0);
            return tok_number;
        }
        // handling comments
        if (lastchar=='#') {
            do {
            lastchar=getchar();
            } while (lastchar != EOF && lastchar != '\n' && lastchar != '\r');
            if (lastchar!=EOF)
                return gettok();
        }
        if (lastchar==EOF)
            return tok_eof;
        // default return the ascii-value
        int thischar=lastchar;
        lastchar=getchar();
        return thischar;

    }
}
