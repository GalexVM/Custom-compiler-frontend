//#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <climits>
#include <cctype>
#include <cstring>
#include <utility>
#include <vector>
#include <deque>
#include <unordered_map>
#include <stack>
#include <set>
#include <list>
#include <vector>
#include "Tree.h"

using namespace std;

list<pair<string, string>> tokens;



unordered_map<string, vector<vector<token>>> parseGrammarFromFile(const string& filename) {
    unordered_map<string, vector<vector<token>>> productions;
    ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "No se pudo abrir el archivo: " << filename << std::endl;
        return productions;  // Devolver un mapa vacío en caso de error.
    }

    std::string line;

    while (std::getline(file, line)) {
        std::istringstream lineStream(line);
        std::string nonTerminal;
        lineStream >> nonTerminal;

        if (nonTerminal.empty()) {
            continue;  // Línea vacía o sin no terminal, omitir.
        }

        token production;
        std::vector<token> productionSymbols;

        while (lineStream >> production.parseValue) {
            if (production.parseValue == "::=") {
                continue;  // Ignorar la flecha ::= en las producciones.
            }
            productionSymbols.push_back(production);
        }

        productions[nonTerminal].push_back(productionSymbols);
    }

    return productions;
}

class Scanner {
private:
    char c = 0;
    string identifier;
    unsigned int numLine = 1, numCol = 1, tempCol = numCol;
    bool stringOn = false;
    bool numberIsFloat = false;
    string number;
    char prevChar = 0;
public:
    char tokenchar = 0;
    string tokentype, tokenval;
    unsigned int numErrors = 0;
    bool scan = true, error = false;
    void Scan(string code) {
        vector<pair<int, int>> errores;

        ifstream archivo(code);
        if (archivo.is_open()) {
            while (archivo.get(c)) {
                //cout << c;
                tempCol = numCol;
                scan = true;
                //TODO: incluir ';' como fin
                if (c == ' ' || c == '\t')
                {
                    if (!identifier.empty() || !number.empty()) {
                        if (!identifier.empty())
                        {
                            scan = true;
                            tokentype = "ID";
                            tokenval = identifier;
                            tempCol -= identifier.size() + numErrors;
                            identifier.clear();
                        }
                        if (!number.empty())
                        {
                            scan = true;
                            if (numberIsFloat)
                                tokentype = "FLOAT";
                            else
                                tokentype = "NUMBER";
                            tokenval = number;
                            tempCol -= number.size() + numErrors;
                            number.clear();
                        }
                    }
                    else {
                        scan = false;
                    }


                }
                else if (c == '.') {
                    if (!number.empty()) {
                        number += c;
                        numberIsFloat = true;
                        scan = false;
                    }
                }
                else if (c == '\n')
                {
                    numLine++;
                    numCol = 0;
                    scan = false;
                    numErrors = 0;
                }
                else if (c == '(' || c == ')' || c == '{' || c == '}' || c == '+' ||
                         c == '-' || c == '/' || c == '%' || c == ';' || c == '&' || c == '|')//Single-character
                {
                    tokenchar = c;
                    switch (tokenchar)
                    {
                        case '(':tokentype = "OPEN_BRACES"; break;
                        case ')':tokentype = "CLOSE_BRACES"; break;
                        case '{':tokentype = "OPEN_BLOCK"; break;
                        case '}':tokentype = "CLOSE_BLOCK"; break;
                        case '+':tokentype = "SUM_OPERATOR"; break;
                        case '-':tokentype = "SUBS_OPERATOR"; break;
                        case '/':tokentype = "DIV_OPERATOR"; break;
                        case '%':tokentype = "MOD_OPERATOR"; break;
                        case '&':tokentype = "AND_OPERATOR"; break;
                        case '|':tokentype = "OR_OPERATOR"; break;
                        case ';': {
                            tokentype = "SEMICOLON";
                            if (!identifier.empty())
                            {
                                cout << "DEBUG SCAN - " << "ID" << "[" << identifier << "]"
                                     << " encontrado en (" << numLine << ":" << tempCol - identifier.size() << ")" << endl;
                                identifier.clear();
                            }
                            break; }
                    }
                    tokenval = string(1, c);
                }
                else if (c == '=' || c == '<' || c == '>' || c == '!')//Two-char operators
                {
                    char temp;
                    tokenchar = c;
                    switch (tokenchar)
                    {
                        case '=': {
                            if ((temp = archivo.peek()) == '=') {
                                tokentype = "EQUAL";
                                archivo.ignore(1);
                                tokenval = string(1, tokenchar) + string(1, temp);
                                numCol++;
                            }
                            else {
                                tokentype = "ASSIGN";
                                tokenval = string(1, tokenchar);
                            } break;
                        }
                        case '<': {
                            if ((temp = archivo.peek()) == '=') {
                                tokentype = "LESS_EQUAL_THAN";
                                archivo.ignore(1);
                                tokenval = string(1, tokenchar) + string(1, temp);
                                numCol++;
                            }
                            else {
                                tokentype = "LESS_THAN";
                                tokenval = string(1, tokenchar);
                            } break;
                        }
                        case '>': {
                            if ((temp = archivo.peek()) == '=') {
                                tokentype = "GREATER_EQUAL_THAN";
                                archivo.ignore(1);
                                tokenval = string(1, tokenchar) + string(1, temp);
                                numCol++;
                            }
                            else {
                                tokentype = "GREATER_THAN";
                                tokenval = string(1, tokenchar);
                            }  break;
                        }
                        case '!': {
                            if ((temp = archivo.peek()) == '=') {
                                tokentype = "DIFFERENT_FROM";
                                archivo.ignore(1);
                                tokenval = string(1, tokenchar) + string(1, temp);
                                numCol++;
                            }
                            else {
                                tokentype = "NOT_OPERATOR";
                                tokenval = string(1, tokenchar);
                            }break;
                        }

                    }
                }

                else if (c == '#')//Comment
                {
                    tokenchar = c;
                    tokentype = "COMMENTARY";
                    tokenval = string(1, tokenchar) + "...";
                    archivo.ignore(INT_MAX, '\n');
                    scan = false;
                    cout << "DEBUG SCAN - " << tokentype << "[" << tokenval << "]"
                         << " encontrado en (" << numLine << ":" << numCol << ")" << endl;
                    numLine++;
                    numCol = 0;
                }

                else if (c == '"')//String literal
                {
                    if (!stringOn) {
                        tokenchar = c;
                        tokentype = "STRING";
                        tokenval = string(1, tokenchar) + "...";
                        archivo.ignore(INT_MAX, '"');
                        scan = false;
                        cout << "DEBUG SCAN - " << tokentype << "[" << tokenval << "]"
                             << " encontrado en (" << numLine << ":" << numCol << ")" << endl;
                        tokens.emplace_back(tokentype, tokenval);
                        stringOn = false;
                    }

                }

                else if (isdigit(c))//Digit
                {
                    tokenchar = c;
                    scan = false;
                    number += c;
                }

                else if (isalpha(c))//id, reserved words
                {
                    //if
                    if (c == 'i')
                    {

                        if (archivo.peek() == 'f')
                        {
                            tokenchar = c;
                            tokentype = "IF";
                            tokenval = string(1, tokenchar) + 'f';
                            archivo.ignore(1);
                            numCol++;
                        }
                        else
                        {
                            identifier += c;
                            scan = false;
                        }

                    }
                        //else, end
                    else if (c == 'e')
                    {
                        char b[4];
                        std::streampos originalPos = archivo.tellg();
                        for (auto i = 0; i < 4 - 1; i++)
                        {
                            b[i] = archivo.peek();
                            archivo.seekg(1, std::ios::cur);
                        }
                        b[3] = '\0';
                        if (strcmp(b, "lse") == 0)
                        {
                            tokenchar = c;
                            tokentype = "ELSE";
                            tokenval = string(1, tokenchar) + "lse";
                            numCol += 3;
                        }
                        else {
                            archivo.seekg(originalPos);
                        }

                        char b2[3];
                        originalPos = archivo.tellg();
                        for (auto i = 0; i < 3 - 1; i++)
                        {
                            b2[i] = archivo.peek();;
                            archivo.seekg(1, std::ios::cur);
                        }
                        b2[2] = '\0';
                        if (strcmp(b2, "nd") == 0)
                        {
                            tokenchar = c;
                            tokentype = "END";
                            tokenval = string(1, tokenchar) + "nd";
                            numCol += 2;
                        }
                        else {
                            archivo.seekg(originalPos);
                            identifier += c;
                            scan = false;
                        }
                    }
                        //while
                    else if (c == 'w')
                    {
                        char b[5];
                        std::streampos originalPos = archivo.tellg();
                        for (auto i = 0; i < 5 - 1; i++)
                        {
                            b[i] = archivo.peek();;
                            archivo.seekg(1, std::ios::cur);
                        }
                        b[4] = '\0';
                        if (strcmp(b, "hile") == 0)
                        {
                            tokenchar = c;
                            tokentype = "WHILE";
                            tokenval = string(1, tokenchar) + "hile";
                            numCol += 4;
                        }
                        else {
                            //archivo.seekg(-4, ios::cur);
                            archivo.seekg(originalPos);
                            identifier += c;
                            scan = false;
                        }
                    }
                        //begin
                    else if (c == 'b')
                    {
                        char b[5];
                        std::streampos originalPos = archivo.tellg();
                        for (auto i = 0; i < 5 - 1; i++)
                        {
                            b[i] = archivo.peek();;
                            archivo.seekg(1, std::ios::cur);
                        }
                        b[4] = '\0';
                        if (strcmp(b, "egin") == 0)
                        {
                            tokenchar = c;
                            tokentype = "BEGIN";
                            tokenval = string(1, tokenchar) + "egin";
                            numCol += 4;
                        }
                        else {
                            //archivo.seekg(-4, ios::cur);
                            archivo.seekg(originalPos);
                            identifier += c;
                            scan = false;
                        }
                    }
                        //Int
                    else if (c == 'I')
                    {
                        char b[3];
                        std::streampos originalPos = archivo.tellg();
                        for (auto i = 0; i < 3 - 1; i++)
                        {
                            b[i] = archivo.peek();;
                            archivo.seekg(1, std::ios::cur);
                        }
                        b[2] = '\0';
                        if (strcmp(b, "nt") == 0)
                        {
                            tokenchar = c;
                            tokentype = "DATA_INT";
                            tokenval = string(1, tokenchar) + "nt";
                            numCol += 2;
                        }
                        else {
                            archivo.seekg(originalPos);
                            identifier += c;
                            scan = false;
                        }
                    }
                        //String
                    else if (c == 'S')
                    {
                        char b[6];
                        std::streampos originalPos = archivo.tellg();
                        for (auto i = 0; i < 6 - 1; i++)
                        {
                            b[i] = archivo.peek();;
                            archivo.seekg(1, std::ios::cur);
                        }
                        b[5] = '\0';
                        if (strcmp(b, "tring") == 0)
                        {
                            tokenchar = c;
                            tokentype = "DATA_STRING";
                            tokenval = string(1, tokenchar) + "tring";
                            numCol += 5;
                        }
                        else {
                            archivo.seekg(originalPos);
                            identifier += c;
                            scan = false;
                        }
                    }
                        //Bool
                    else if (c == 'B')
                    {
                        char b[4];
                        std::streampos originalPos = archivo.tellg();
                        for (auto i = 0; i < 4 - 1; i++)
                        {
                            b[i] = archivo.peek();;
                            archivo.seekg(1, std::ios::cur);
                        }
                        b[3] = '\0';
                        if (strcmp(b, "ool") == 0)
                        {
                            tokenchar = c;
                            tokentype = "DATA_BOOL";
                            tokenval = string(1, tokenchar) + "ool";
                            numCol += 3;
                        }
                        else {
                            archivo.seekg(originalPos);
                            identifier += c;
                            scan = false;
                        }
                    }
                        //true
                    else if (c == 't')
                    {
                        char b[4];
                        std::streampos originalPos = archivo.tellg();
                        for (auto i = 0; i < 4 - 1; i++)
                        {
                            b[i] = archivo.peek();;
                            archivo.seekg(1, std::ios::cur);
                        }
                        b[3] = '\0';
                        if (strcmp(b, "rue") == 0)
                        {
                            tokenchar = c;
                            tokentype = "BOOLEAN_TRUE";
                            tokenval = string(1, tokenchar) + "rue";
                            numCol += 3;
                        }
                        else {
                            archivo.seekg(originalPos);
                            identifier += c;
                            scan = false;
                        }
                    }
                        //false
                    else if (c == 'f')
                    {
                        char b[5];
                        std::streampos originalPos = archivo.tellg();
                        for (auto i = 0; i < 5 - 1; i++)
                        {
                            b[i] = archivo.peek();;
                            archivo.seekg(1, std::ios::cur);
                        }
                        b[4] = '\0';
                        if (strcmp(b, "alse") == 0)
                        {
                            tokenchar = c;
                            tokentype = "BOOLEAN_FALSE";
                            tokenval = string(1, tokenchar) + "alse";
                            numCol += 4;
                        }
                        else {
                            archivo.seekg(originalPos);
                            identifier += c;
                            scan = false;
                        }
                    }
                        //loadPdf
                    else if (c == 'l')
                    {
                        char b[7];
                        std::streampos originalPos = archivo.tellg();
                        for (auto i = 0; i < 7 - 1; i++)
                        {
                            b[i] = archivo.peek();;
                            archivo.seekg(1, std::ios::cur);
                        }
                        b[6] = '\0';
                        if (strcmp(b, "oadPdf") == 0)
                        {
                            tokenchar = c;
                            tokentype = "FUNCTION_LOADPDF";
                            tokenval = string(1, tokenchar) + "oadPdf";
                            numCol += 6;
                        }
                        else {
                            archivo.seekg(originalPos);
                            identifier += c;
                            scan = false;
                        }
                    }
                        //mergePdf
                    else if (c == 'm')
                    {
                        char b[8];
                        std::streampos originalPos = archivo.tellg();
                        for (auto i = 0; i < 8 - 1; i++)
                        {
                            b[i] = archivo.peek();;
                            archivo.seekg(1, std::ios::cur);
                        }
                        b[7] = '\0';
                        if (strcmp(b, "ergePdf") == 0)
                        {
                            tokenchar = c;
                            tokentype = "FUNCTION_MERGEPDF";
                            tokenval = string(1, tokenchar) + "ergePdf";
                            numCol += 7;
                        }
                        else {
                            archivo.seekg(originalPos);
                            identifier += c;
                            scan = false;
                        }
                    }
                        //splitPdf
                    else if (c == 's')
                    {
                        char b[8];
                        std::streampos originalPos = archivo.tellg();
                        for (auto i = 0; i < 8 - 1; i++)
                        {
                            b[i] = archivo.peek();;
                            archivo.seekg(1, std::ios::cur);
                        }
                        b[7] = '\0';
                        if (strcmp(b, "plitPdf") == 0)
                        {
                            tokenchar = c;
                            tokentype = "FUNCTION_SPLITPDF";
                            tokenval = string(1, tokenchar) + "plitPdf";
                            numCol += 7;
                        }
                        else {
                            archivo.seekg(originalPos);
                            identifier += c;
                            scan = false;
                        }
                    }
                    else {
                        identifier += c;
                        scan = false;
                    }
                }
                else {
                    errores.emplace_back(numLine, numCol);
                    scan = false;
                    numErrors++;
                }

                if (scan) {
                    tokens.emplace_back(tokentype, tokenval);
                    cout << "DEBUG SCAN - " << tokentype << "[" << tokenval << "]" << " encontrado en (" << numLine << ":" << tempCol << ")" << endl;
                }
                numCol++;
            }
            archivo.close();
        }
        else {
            cout << "No se pudo abrir el archivo";
        }

        if (!errores.empty())
        {
            cout << "ERRORES DETECTADOS\n";
            for (auto i : errores)
                cout << "-> fila: " << i.first << ", columna: " << i.second << endl;
        }
    }
};

class Parser {
    //TODO: documentar
private:
    vector<string> nonTerminals;
    int numNonTerminals = 1;
    //vector<string> terminals;
    vector<token> terminals;
    int numTerminals = 1;

    unordered_map<string, set<string>> firsts;
    unordered_map<string, set<string>> follows;
    unordered_map<unsigned int, vector<string>> firstpluses;

    bool isTerminal(string tok);
    bool isNullable(string tok);
    vector<int> typesTable;
    int numTipos = 3;

public:
    //unordered_map<string, vector<vector<string>>> productions;
    unordered_map<string, vector<vector<token>>> productions;
    //vector<vector<string>> table;
    vector<vector<token>> table;
    unordered_map<string, int>mf;
    unordered_map<string, int>mc;
    stack<string> parseStack;
    //list<string> tk;
    list<token> tk;
    set<string> get_first(string non_term);
    set<string> get_follow(string non_term, string prev = "");
    set<string> get_single_first(vector<token> prod);
    void printTableToFile(string filename);
    void printFirstsAndFollows();
    void readGrammar();
    void buildTable();
    bool parseTokens(Tree& tree);
    void buildSemanticTable();
    bool testSemanticOperations(string a, string b);
    void build_AST_tree(NodeTree* ini, NodeTree* ast_ini, Tree& tree, Tree& ast);
    void build_AST_if(NodeTree* piv, NodeTree* astPiv, NodeTree* sts, Tree& tree, Tree& ast);
    void build_AST_expr(NodeTree* exp, NodeTree* astPiv, Tree& tree, Tree& ast);
    void build_AST_aritmethic(NodeTree* term, NodeTree* astPiv, Tree&  tree, Tree& ast);
    void index_operator_AST(NodeTree* astPiv, NodeTree* aritOp, Tree& ast);
    void build_AST_factor(NodeTree* factor, NodeTree* astPiv, Tree& tree, Tree& ast);

};
void Parser::readGrammar()
{
    productions = parseGrammarFromFile("gramatica.txt");

    //Llenar no terminales
    for (const auto& pair : productions) {
        string key = pair.first;
        nonTerminals.push_back(key);
    }
    numNonTerminals = nonTerminals.size();

    set<token,TokenComparator> temp;

    //Llenar terminales
    for (const auto& pair : productions) {
        for (auto prod : pair.second) {
            for (auto tok : prod) {
                if (isTerminal(tok.parseValue)) {
                    temp.insert(tok);
                }
            }
        }
    }
    for (auto t : temp)
        terminals.push_back(t);
    numTerminals = terminals.size();
}

bool Parser::isTerminal(string tok)
{
    for (auto i : nonTerminals)
    {
        if (tok == i) {
            return false;
        }
    }
    return true;
}

bool Parser::isNullable(string tok)
{
    for (auto subprod : productions[tok])
    {
        if (subprod[0].parseValue == "''")
            return true;
    }
    return false;
}

set<string> Parser::get_first(string non_term)
{
    set<string> r;
    for (int i = 0; i < productions[non_term].size(); i++) //Each production
    {
        if (isTerminal(productions[non_term][i][0].parseValue))
        {
            if (productions[non_term][i][0].parseValue != "''")
                r.insert(productions[non_term][i][0].parseValue);
        }
        else
        {
            set<string> t = get_first(productions[non_term][i][0].parseValue);
            r.insert(t.begin(), t.end());
        }

    }
    return r;
}

set<string> Parser::get_single_first(vector<token> prod)
{
    set<string> r;
    if (isTerminal(prod[0].parseValue))
    {
        if (prod[0].parseValue != "''")
            r.insert(prod[0].parseValue);
    }
    else
    {
        set<string> t = get_first(prod[0].parseValue);
        r.insert(t.begin(), t.end());
    }
    return r;
}

set<string> Parser::get_follow(string non_term, string prev)
{
    set<string> r;
    for (const auto& prod : productions) //All productions
    {
        for (auto subprod : prod.second) // productions with same left operand
        {
            for (auto t = 0; t < subprod.size(); t++) // all tokens in each production
            {
                if (subprod[t].parseValue == non_term) // find non_term
                {
                    if (t == subprod.size() - 1) // last token
                    {
                        if (prev != prod.first) //evitar bucles de follow
                        {
                            set<string> temp = get_follow(prod.first, non_term);
                            r.insert(temp.begin(), temp.end());
                        }

                    }
                    else if (!isTerminal(subprod[t + 1].parseValue)) //the follow is non terminal
                    {
                        set<string> temp = get_first(subprod[t + 1].parseValue);
                        r.insert(temp.begin(), temp.end());
                        if (isNullable(subprod[t + 1].parseValue))
                        {
                            temp.clear();
                            temp = get_follow(prod.first, non_term);
                            r.insert(temp.begin(), temp.end());
                        }

                    }
                    else if (isTerminal(subprod[t + 1].parseValue)) //the follow is terminal
                    {
                        if (subprod[t + 1].parseValue != "''") {
                            r.insert(subprod[t + 1].parseValue);
                        }
                    }
                }
            }
        }
    }
    return r;
}

void Parser::buildTable()
{
    //First, follow
    for (auto term : nonTerminals)
    {
        firsts[term] = get_first(term);
        follows[term] = get_follow(term);
    }
    //Initialize table
    table = vector<vector<token>>((numNonTerminals) * (numTerminals), vector<token>());
    int f = numTerminals;

    int c = 0;
    for (auto i : nonTerminals)
    {
        mf[i] = c;
        c++;
    }
    c = 0;
    for (auto i : terminals)
    {
        mc[i.parseValue] = c;
        c++;
    }
    //printTableToFile("table.txt");

    //Fill table
    int counter = 0;
    for (const auto& pair : productions)
    {
        for (auto prod : pair.second)
        {
            if (prod[0].parseValue == "''") {
                //fill follow
                for (auto follow : follows[pair.first])
                {
                    table[f * mf[pair.first] + mc[follow]] = vector<token>(prod); //table[A][follow(A)] = production
                    //printTableToFile("table.txt");
                }
            }
            else {
                //fill first
                set<string> temp = get_single_first(prod);
                for (auto first : temp)
                {
                    table[f * mf[pair.first] + mc[first]] = vector<token>(prod); //table[A][first(A)] = production
                    //printTableToFile("table.txt");
                }
            }
        }
    }
}

bool Parser::parseTokens(Tree& tree)
{
    //init tree
    NodeTree* piv = tree.root;
    //Tokens are input

    for (auto i : tokens)
    {
        if (i.first == "ID" || i.first == "NUMBER" || i.first == "STRING" || i.first == "FLOAT")
            tk.emplace_back(i.first,i.second); //Token id, value
        else
            tk.emplace_back(i.second,i.second); //value, value
    }
    cout << "Tokens: \n";
    for (auto i : tk) cout << i.parseValue << endl;

    parseStack.push("$");
    tk.emplace_back("$","$");
    parseStack.push(nonTerminals[0]); //push Program
    bool success = true;

    vector<token>* production;

    while (parseStack.top() != "$" && tk.front().parseValue != "$")
    {
        if (tk.front().parseValue == parseStack.top()) //POP
        {
            cout << "POP: " << parseStack.top() << "\t";
            cout << "[" << parseStack.top() << "->" << tk.front().parseValue << "]" << endl;
            tk.pop_front();
            parseStack.pop();

            bool termino = false;
            auto temp = (*piv->padre->hijos)[0];
            if (piv != temp) // no esta todo a la izq
            {
                //Mover uno a la izq
                for (int i = 0; i < piv->padre->hijos->size(); i++)
                    if (((*piv->padre->hijos)[i]) == piv)
                        piv = ((*piv->padre->hijos)[i - 1]);
            }
            else //sí está todo a la izq
            {
                while (piv == temp) {
                    if (piv != tree.root) {
                        piv = piv->padre;
                        if (piv != tree.root) temp = (*piv->padre->hijos)[0];
                        else termino = true;
                    }
                }
                if (!termino) {
                    //Mover a la izq
                    for (int i = 0; i < piv->padre->hijos->size(); i++)
                        if (((*piv->padre->hijos)[i]) == piv)
                            piv = ((*piv->padre->hijos)[i - 1]);
                }
            }
        }
        else //Move
        {
            production = &(table[numTerminals * mf[parseStack.top()] + mc[tk.front().parseValue]]); //table[f*i + j]
            for( auto& i: *production) //Guardar valores originales
            {
                if (i.parseValue == "ID" || i.parseValue == "NUMBER" || i.parseValue == "STRING" || i.parseValue == "FLOAT"){
                    i.originalValue = tk.front().originalValue;
                }
                else{
                    i.originalValue = "";
                }
            }

            cout << "Move: ";
            for (auto i : *production)
            {
                cout << i.parseValue << " ";
            }cout << "\t[" << parseStack.top() << "->" << tk.front().parseValue << "]" << endl;

            if (production->empty())
            {
                cout << "Parse error, invalid grammar: " << parseStack.top() << "->" << tk.front().parseValue << endl;
                //Buscar en el input el follow del token  del stack
                bool foundFollow = false;
                while (!foundFollow && !tk.empty()) {
                    for (auto i : follows[parseStack.top()])
                    {
                        if (tk.front().parseValue == i) {
                            foundFollow = true;
                            cout << "Found follow: " << i << endl;
                            parseStack.pop();
                            break;
                        }
                    }
                    if (!foundFollow) {
                        tk.pop_front();
                    }
                }
                //tk.pop_front();
                success = false;
                continue;
            }
            parseStack.pop();
            for (int j = (*production).size() - 1; j >= 0; j--)
            {
                parseStack.push((*production)[j].parseValue);
                piv->addChild((*production)[j]); //Añadir hijos
            }

            if (parseStack.top() == "''")//Hacer pseudo pop en el arbol
            {
                parseStack.pop();
                //Mismo código que POP, pero sin comprobar si está en la izq
                bool termino = false;
                auto temp = (*piv->padre->hijos)[0];
                while (piv == temp) {
                    if (piv != tree.root) {
                        piv = piv->padre;
                        if (piv != tree.root) temp = (*piv->padre->hijos)[0];
                        else termino = true;
                    }
                }
                if (!termino) {
                    //Mover a la izq
                    for (int i = 0; i < piv->padre->hijos->size(); i++)
                        if (((*piv->padre->hijos)[i]) == piv)
                            piv = ((*piv->padre->hijos)[i - 1]);
                }
            }
            else
            {
                piv = piv->hijos->back();//Ir al final de los hijos recién añadidos
            }
        }
    }

    if (parseStack.top() != "$" || tk.front().parseValue != "$" || !success)
    {
        cout << "Parse Error\n";
    }
    else
    {
        cout << "Success!\n";
    }

    for (auto it = tokens.begin(); it != tokens.end(); ++it)
    {
        string o1, o2;
        if (it->first == "LESS_THAN") {
            if (it != tokens.begin()) {
                auto prev = std::prev(it);
                o1 = prev->first;
            }
            if (std::next(it) != tokens.end()) {
                auto next = std::next(it);
                o2 = next->first;
            }
            if (!testSemanticOperations(o1, o2)) {
                cout << "SEMANTIC ERROR: " << o1 << " does not use operator " << it->second << " with " << o2 << endl;
            }
        }
    }


    return false;
}

void Parser::buildSemanticTable()
{
    /*
    * NUMBER = 0
    * FLOAT = 1
    * STRING = 2
    */
    typesTable = vector<int>(numTipos * numTipos);
    typesTable[numTipos * 0 + 0] = 0;
    typesTable[numTipos * 0 + 1] = 1;
    typesTable[numTipos * 0 + 2] = -1;
    typesTable[numTipos * 1 + 0] = 1;
    typesTable[numTipos * 1 + 1] = 1;
    typesTable[numTipos * 1 + 2] = -1;
    typesTable[numTipos * 2 + 0] = -1;
    typesTable[numTipos * 2 + 1] = -1;
    typesTable[numTipos * 2 + 2] = 2;
}

bool Parser::testSemanticOperations(string a, string b)
{
    /*
    * NUMBER = 0
    * FLOAT = 1
    * STRING = 2
    */
    int ia = 0, ib = 0;
    if (a == "NUMBER")ia = 0;
    else if (a == "FLOAT")ia = 1;
    else if (a == "STRING")ia = 2;
    if (b == "NUMBER")ib = 0;
    else if (b == "FLOAT")ib = 1;
    else if (b == "STRING")ib = 2;
    else return false;
    if (typesTable[numTipos * ia + ib] == -1)
        return false;
    else
        return true;
}

void Parser::printTableToFile(string filename)
{
    std::ofstream outputFile(filename); // Abre un archivo para escritura

    if (!outputFile.is_open())
    {
        std::cerr << "No se pudo abrir el archivo " << filename << " para escritura." << std::endl;
        return;
    }

    for (int i = 0; i < numNonTerminals; i++)
    {
        for (int j = 0; j < numTerminals; j++) {
            outputFile << "(" << i << ": " << nonTerminals[i] << " , " << j << ": " << terminals[j].parseValue << " )";
            for (auto p : table[numTerminals * i + j])
            {
                outputFile << p.parseValue;
            }
            outputFile << endl;
        }
    }

    outputFile.close(); // Cierra el archivo
}

void Parser::printFirstsAndFollows()
{
    for (const auto& pair : firsts)
    {
        cout << pair.first << "\t";
    }cout << endl;
    for (const auto& pair : firsts)
    {
        for (auto first : pair.second)
        {
            cout << first << ',';
        }
        cout << "\t";
    }
    cout << endl << endl;

    for (const auto& pair : follows)
    {
        cout << pair.first << "\t";
    }cout << endl;
    for (const auto& pair : follows)
    {
        for (auto follow : pair.second)
        {
            cout << follow << ',';
        }
        cout << "\t";
    }
    cout << endl << endl;
}

void Parser::build_AST_factor(NodeTree* factor, NodeTree* astPiv, Tree& tree, Tree& ast)
{
    //Factor: Name[0] o Literal[0]
    token value;
    if((*factor->hijos)[0]->name.parseValue == "Name" ){
        value = (*(*factor->hijos)[0]->hijos)[1]->name;
    }else{
        value = (*(*factor->hijos)[0]->hijos)[0]->name;
    }
    //auto value = (*(*factor->hijos)[0]->hijos)[0]->name;
    astPiv->addChild(value);

}

void Parser::index_operator_AST(NodeTree* astPiv,NodeTree* aritOp, Tree& ast)
{
    astPiv->addChild(aritOp->name);
    auto op = astPiv->hijos->back();
    while ( op->padre &&  op->padre->priority > 0 &&  op->priority < op->padre->priority){
        //Swap
        auto temp = op->name;
        op->name = op->padre->name;
        op->padre->name = temp;
        op = op->padre;
    }
}

void Parser::build_AST_aritmethic(NodeTree* term, NodeTree* astPiv, Tree&  tree, Tree& ast)
{
  /*Jerarquía:
   %
   *, /
   +, -
  */
  auto tempTerm = term;
  auto tempAstPiv = astPiv;
  //vector<NodeTree*> terms;
  vector<NodeTree*> astPivs;
  unordered_map<string, NodeTree*> terms;
  //Term: TermP[0] Factor[1]

  if((*tempTerm->hijos)[0]->numHijos>1) //Hay operadores
  {
      while((*tempTerm->hijos)[0]->numHijos>1){ //TermP no es "
          //TermP: TermP[0] Factor[1] Operator[2] o "
          auto aritOp = (*(*(*tempTerm->hijos)[0]->hijos)[2]->hijos)[0]; //+,-,*,/,%
          //Guardar ubicación en el parse tree de cada operador
          terms[aritOp->name.parseValue] = (*tempTerm->hijos)[0]; //TermP más cercano a un operador
          index_operator_AST(tempAstPiv,aritOp,ast);
          tempAstPiv = tempAstPiv->hijos->back();
          tempTerm = (*tempTerm->hijos)[0];
      }
      tempAstPiv = astPiv->hijos->back();
      while(tempAstPiv->numHijos > 0 && tempAstPiv->type != "variable"){
        auto op = tempAstPiv->name.parseValue;
        auto tempTerm = terms[op];
        build_AST_factor((*tempTerm->hijos)[1],tempAstPiv, tree, ast);
          tempAstPiv = tempAstPiv->hijos->back();
      }
      if(tempAstPiv->type != "variable"){
          auto op = tempAstPiv->name.parseValue;
          auto tempTerm2 = terms[op];
          build_AST_factor((*tempTerm2->hijos)[1],tempAstPiv, tree, ast);
      }

  }
  else //No hay operadores
  {
      build_AST_factor((*tempTerm->hijos)[1],tempAstPiv,tree,ast);
  }




}

void Parser::build_AST_expr(NodeTree* exp, NodeTree* astPiv, Tree& tree, Tree& ast)
{
    //Expresión: ExpressionP[0] CompExpr[1]
    //1. Verificar si hay AND, OR
    bool hasLogic = false;
    unsigned int childPos;
    if ((*exp->hijos)[0]->hijos->size() > 1) { //ExpressionP no es null
        hasLogic = true;
        //ExpressionP: Expression[0] LogicOP[1] o "
        auto logicOp = (*(*(*exp->hijos)[0]->hijos)[1]->hijos)[0]; //& o |
        astPiv->addChild(logicOp->name);
        build_AST_expr((*(*exp->hijos)[0]->hijos)[0], (*astPiv->hijos)[0], tree, ast); //& hijo de otro &
    }
    //2. Verificar si hay comparadores <, >, etc.
    if(astPiv->numHijos > 0) astPiv = (*astPiv->hijos)[0];
    auto compExpr = (*exp->hijos)[1];
    //CompExpr: CompExprP[0] Term[1]
    if((*compExpr->hijos)[0]->numHijos > 1) // CompExprP no null
    {
        //ComExprP: Term[0] CompOp[1] o " <
        auto compOp = (*(*(*compExpr->hijos)[0]->hijos)[1]->hijos)[0]; // <, >, <=, etc.
        astPiv->addChild(compOp->name);
        childPos = astPiv->hijos->size()-1;
    }
    //else {... Sería si admitimos expresiones tipo (a & b < c). No lo haremos de momento.

    //3. Verificar si hay artimética +, -, /, etc.
    build_AST_aritmethic((*compExpr->hijos)[1], (*astPiv->hijos)[childPos], tree, ast);
    build_AST_aritmethic((*(*compExpr->hijos)[0]->hijos)[0], (*astPiv->hijos)[childPos], tree, ast);


}

void Parser::build_AST_if(NodeTree* piv, NodeTree* astPiv, NodeTree* sts, Tree& tree, Tree& ast)
{
    NodeTree* st = (*sts->hijos)[(*sts->hijos).size() - 1]; //Statement
    /*Hijos If: Else } Statements { )Expression( if*/
    //Agregar If como hijo de Program
    astPiv->addChild((*st->hijos)[0]->name); //astPiv inicia en root
    unsigned int childPos = astPiv->hijos->size() - 1; //Posición de ese if
    //Colocar recursivamente otros statements si es que hay.
    //En Statements buscar otro statement pero agregar en el mismo nivel de ast.
    build_AST_tree(sts, astPiv, tree, ast);
    //Poner hijos a if
    NodeTree* if_st = (*st->hijos)[0];
    NodeTree* Else = (*if_st->hijos)[0], * Statements = (*if_st->hijos)[2], * Expression = (*if_st->hijos)[5];
    astPiv = (*astPiv->hijos)[childPos]; //Apuntar al if del ast
    astPiv->addChild(Else->name);
    astPiv->addChild("Block");
    astPiv->addChild(Expression->name); //Condición
    build_AST_expr(Expression, (*astPiv->hijos)[2], tree, ast);


}

void Parser::build_AST_tree(NodeTree* ini, NodeTree* ast_ini, Tree& tree, Tree& ast)
{
    NodeTree* piv = ini;
    NodeTree* astPiv = ast_ini;
    //Recorrer arbol semántico buscando statements
    NodeTree* sts = nullptr;
    bool hasStatements = false;
    for (int i = 0; i < piv->hijos->size(); i++)
    {
        if ((*piv->hijos)[i]->name.parseValue == "Statements") {
            sts = (*piv->hijos)[i];
            hasStatements = true;
        }
    }
    if (!hasStatements)return;

    /*Hijos de sts: Statements ; Statement o " */
    //Analizar qué tipo de statement es Statement
    if (sts->numHijos > 1) //Sí tiene statement
    {
        NodeTree* st = (*sts->hijos)[sts->hijos->size() - 1]; //Statement
        if ((*st->hijos)[0]->name.parseValue == "If") {
            build_AST_if(piv, astPiv, sts, tree, ast);
        }
        //else if()...
    }
    else //Statements vacío
    {
        return;
    }

}

int main(int argc, char* argv[]) {

    Scanner sc;
    Tree tree("Program");
    sc.Scan("code.txt");

    Parser parse;
    parse.readGrammar();

    parse.buildTable();
    parse.buildSemanticTable();
    parse.printTableToFile("table.txt");
    parse.parseTokens(tree);

    Tree ast_tree("Program");

    parse.build_AST_tree(tree.root, ast_tree.root, tree, ast_tree);


    tree.printTree();
    cout << endl;
    ast_tree.printTree();
    //tree.preOrderTraversal();


    return 0;
}