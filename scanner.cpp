#include <iostream>
#include <string>
#include <vector>
#include <fstream>

using namespace std;

enum class TokenType {
    IDENTIFIER,
    INT,
    STRING,
    FLOAT,
    BOOLV,
    BOOLF,
    CREATE,
    PAPER, 
    IF, 
    ELSE, 
    THEN, 
    FROM, 
    TO, 
    WHILE, 
    IS,
    RETURN, 
    IN, 
    CALCULATE,
    SQRT, 
    QBIC,

    ASSIGN,        
    PLUS,          
    MINUS,         
    MULTI,         
    DIVISION,      
    IN_OP,         
    OUT_OP,        
    IN_LV,         
    OUT_LV,        
    SIMILAR,       
    LESS_THAN,     
    GREATER_THAN,
    LESS_EQUAL,
    GREATER_EQUAL,
    NOT_EQUAL,  
    POSITION,      
    NOM,           
    INCREMENT,     
    DECREMENT,     
    POWER,         
    QUOTE,
    END_OF_FILE,         
    UNKNOWN
};

struct Token {
    TokenType type;
    string valor;
    int linea;
    int columna;
};

ifstream archivo;
int linea_actual = 1;
int columna_actual = 0;
char letra_actual = ' ';

char get_char() {
    if (archivo.get(letra_actual)) {
        columna_actual++;
        if (letra_actual == '\n') {
            linea_actual++;
            columna_actual = 0;
        }
        return letra_actual;
    } else {
        letra_actual = EOF;
        return EOF;
    }
}

char peek_char() {
    return archivo.peek();
}

void blanco() {
    while (isspace(letra_actual) || (letra_actual == '/')) {
        if (isspace(letra_actual)) {
            get_char();
        } else if (letra_actual == '/') {
            char next_char = peek_char();
            if (next_char == '/') {
                get_char();
                while (letra_actual != '\n' && letra_actual != EOF) {
                    get_char();
                }
            } else if (next_char == '*') {
                get_char();
                while (true) {
                    get_char();
                    if (letra_actual == '*' && peek_char() == '/') {
                        get_char();
                        break;
                    } else if (letra_actual == EOF) {
                        cerr << "Error: Comentario no cerrado" << endl;
                        return; 
                    }
                }
            } else {
                break; 
            }
        }
    }
}

Token get_Token() {
    blanco();
    if(letra_actual == EOF) {
        return {TokenType::END_OF_FILE, "", linea_actual, columna_actual};
    }

    if(isalpha(letra_actual)) {
        string valor;
        int ini_col = columna_actual;
        while(isalnum(letra_actual)) {
            valor += letra_actual;
            get_char();
        }
        if(valor == "int") {
            return {TokenType::INT, valor, linea_actual, ini_col};
        } else if(valor == "str") {
            return {TokenType::STRING, valor, linea_actual, ini_col};
        } else if(valor == "float") {
            return {TokenType::FLOAT, valor, linea_actual, ini_col};
        } else if(valor == "boolv") {
            return {TokenType::BOOLV, valor, linea_actual, ini_col};
        } else if(valor == "boolf") {
            return {TokenType::BOOLF, valor, linea_actual, ini_col};
        } else if(valor == "create") {
            return {TokenType::CREATE, valor, linea_actual, ini_col};
        } else if(valor == "paper") {
            return {TokenType::PAPER, valor, linea_actual, ini_col};
        } else if(valor == "if") {
            return {TokenType::IF, valor, linea_actual, ini_col};
        } else if(valor == "else") {
            return {TokenType::ELSE, valor, linea_actual, ini_col};
        } else if(valor == "then") {
            return {TokenType::THEN, valor, linea_actual, ini_col};
        } else if(valor == "from") {
            return {TokenType::FROM, valor, linea_actual, ini_col};
        } else if(valor == "to") {
            return {TokenType::TO, valor, linea_actual, ini_col};
        } else if(valor == "while") {
            return {TokenType::WHILE, valor, linea_actual, ini_col};
        } else if(valor == "is") {
            return {TokenType::IS, valor, linea_actual, ini_col};
        } else if(valor == "return") {
            return {TokenType::RETURN, valor, linea_actual, ini_col};
        } else if(valor == "in") {
            return {TokenType::IN, valor, linea_actual, ini_col};
        } else if(valor == "calculate") {
            return {TokenType::CALCULATE, valor, linea_actual, ini_col};
        } else if(valor == "sqrt") {
            return {TokenType::SQRT, valor, linea_actual, ini_col};
        } else if(valor == "qbic") {
            return {TokenType::QBIC, valor, linea_actual, ini_col};
        } else {
            return {TokenType::IDENTIFIER, valor, linea_actual, ini_col};
        }
    }

    if(isdigit(letra_actual)) {
        string valor;
        int ini_col = columna_actual;
        bool es_float = false;

        while(isdigit(letra_actual) || letra_actual == '.') {
            if(letra_actual == '.') {
                if(es_float) {
                    cerr << "Error: Numero flotante con más de un punto decimal" << linea_actual << "," << ini_col << endl;
                    return {TokenType::UNKNOWN, "", linea_actual, ini_col};
                }
                es_float = true;
            }
            valor += letra_actual;
            get_char();
        }

        if(es_float) {
            return {TokenType::FLOAT, valor, linea_actual, ini_col};
        } else {
            return {TokenType::INT, valor, linea_actual, ini_col};
        }
    }

    if (letra_actual == '=') {
        get_char(); 
        if (letra_actual == '=') {
            get_char(); 
            return {TokenType::SIMILAR, "==", linea_actual, columna_actual - 2};
        } else {
            return {TokenType::ASSIGN, "=", linea_actual, columna_actual - 1};
        }
    } else if (letra_actual == '>') {
        get_char();
        if (letra_actual == '=') {
            get_char();
            return {TokenType::GREATER_EQUAL, ">=", linea_actual, columna_actual - 2};
        } else {
            return {TokenType::GREATER_THAN, ">", linea_actual, columna_actual - 1};
        }
    } else if (letra_actual == '<') {
        get_char();
        if (letra_actual == '=') {
            get_char();
            return {TokenType::LESS_EQUAL, "<=", linea_actual, columna_actual - 2};
        } else {
            return {TokenType::LESS_THAN, "<", linea_actual, columna_actual - 1};
        }
    } else if (letra_actual == '!') {
        get_char();
        if (letra_actual == '=') {
            get_char();
            return {TokenType::NOT_EQUAL, "!=", linea_actual, columna_actual - 2};
        } else {
            cerr << "Error léxico: Carácter no válido '!' en línea " << linea_actual << ", columna " << columna_actual - 1 << endl;
            return {TokenType::UNKNOWN, "!", linea_actual, columna_actual - 1};
        }
    } else if (letra_actual == '-') {
        get_char();
        if (letra_actual == '>') {
            get_char();
            return {TokenType::NOM, "->", linea_actual, columna_actual - 2};
        } else {
            return {TokenType::MINUS, "-", linea_actual, columna_actual - 1};
        }
    } else if (letra_actual == '+') {
        get_char();
        if (letra_actual == '+') {
            get_char();
            return {TokenType::INCREMENT, "++", linea_actual, columna_actual - 2};
        } else {
            return {TokenType::PLUS, "+", linea_actual, columna_actual - 1};
        }
    } else if (letra_actual == '-') {
        get_char();
        if (letra_actual == '-') {
            get_char();
            return {TokenType::DECREMENT, "--", linea_actual, columna_actual - 2};
        } else {
            return {TokenType::MINUS, "-", linea_actual, columna_actual - 1};
        }
    } else {
        switch (letra_actual) {
            case '*':
                get_char();
                return {TokenType::MULTI, "*", linea_actual, columna_actual - 1};
            case '/':
                get_char();
                return {TokenType::DIVISION, "/", linea_actual, columna_actual - 1};
            case '{':
                get_char();
                return {TokenType::IN_OP, "{", linea_actual, columna_actual - 1};
            case '}':
                get_char();
                return {TokenType::OUT_OP, "}", linea_actual, columna_actual - 1};
            case '[':
                get_char();
                return {TokenType::IN_LV, "[", linea_actual, columna_actual - 1};
            case ']':
                get_char();
                return {TokenType::OUT_LV, "]", linea_actual, columna_actual - 1};
            case ',':
                get_char();
                return {TokenType::POSITION, ",", linea_actual, columna_actual - 1};
            case '^':
                get_char();
                return {TokenType::POWER, "^", linea_actual, columna_actual - 1};
            case '"':
                get_char();
                return {TokenType::QUOTE, "\"", linea_actual, columna_actual - 1};
            default:
                std::cerr << "Error: Caracter invalido '" << letra_actual << "' en línea " << linea_actual << ", columna " << columna_actual << std::endl;
                get_char();
                return {TokenType::UNKNOWN, "", linea_actual, columna_actual};
        }
    }
}

string Token_type(TokenType type) {
    switch(type) {
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::INT: return "INT";
        case TokenType::STRING: return "STRING";
        case TokenType::FLOAT: return "FLOAT";
        case TokenType::BOOLV: return "BOOLV";
        case TokenType::BOOLF: return "BOOLF";
        case TokenType::CREATE: return "CREATE";
        case TokenType::PAPER: return "PAPER";
        case TokenType::IF: return "IF";
        case TokenType::ELSE: return "ELSE";
        case TokenType::THEN: return "THEN";
        case TokenType::FROM: return "FROM";
        case TokenType::TO: return "TO";
        case TokenType::WHILE: return "WHILE";
        case TokenType::IS: return "IS";
        case TokenType::RETURN: return "RETURN";
        case TokenType::IN: return "IN";
        case TokenType::CALCULATE: return "CALCULATE";
        case TokenType::SQRT: return "SQRT";
        case TokenType::QBIC: return "QBIC";
        case TokenType::ASSIGN: return "ASSIGN";
        case TokenType::PLUS: return "PLUS";
        case TokenType::MINUS: return "MINUS";
        case TokenType::MULTI: return "MULTI";
        case TokenType::DIVISION: return "DIVISION";
        case TokenType::IN_OP: return "IN_OP";
        case TokenType::OUT_OP: return "OUT_OP";
        case TokenType::IN_LV: return "IN_LV";
        case TokenType::OUT_LV: return "OUT_LV";
        case TokenType::SIMILAR: return "SIMILAR";
        case TokenType::LESS_THAN: return "LESS_THAN";
        case TokenType::GREATER_THAN: return "GREATER_THAN";
        case TokenType::LESS_EQUAL: return "LESS_EQUAL";
        case TokenType::GREATER_EQUAL: return "GREATER_EQUAL";
        case TokenType::NOT_EQUAL: return "NOT_EQUAL";
        case TokenType::POSITION: return "POSITION";
        case TokenType::NOM: return "NOM";
        case TokenType::INCREMENT: return "INCREMENT";
        case TokenType::DECREMENT: return "DECREMENT";
        case TokenType::POWER: return "POWER";
        case TokenType::QUOTE: return "QUOTE";
        case TokenType::UNKNOWN: return "UNKNOWN";
        default: return "UNKNOWN";
    }
}   

int main() {
    string file;
    cout << "Ingrese la ruta del archivo: ";
    cin >> file;

    archivo.open(file);
    if(!archivo.is_open()) {
        cerr << "Error: no se puede abrir este archivo." << endl;
        return 1;
    }

    get_char();
    Token token_actual;
    do {
        token_actual = get_Token();
        if(token_actual.type != TokenType::UNKNOWN) {
            cout << "Token: " << Token_type(token_actual.type) << "| Valor: '" << token_actual.valor << "'| Linea: " << token_actual.linea << "| Columna: " << token_actual.columna << endl;
        } 
    } while(token_actual.type != TokenType::END_OF_FILE);
    archivo.close();
    cout << "Análisis completado." << endl;
    return 0;
}
