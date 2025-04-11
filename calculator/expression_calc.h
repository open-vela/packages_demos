/*
 * expression_calc.h
 * Created by W-Mai on 2022/10/2.
 *
 * MIT License
 * Copyright (c) 2022 XCLZ STUDIO@W-Mai
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef EXPRESSIONCALC_EXPRESSION_CALC_H
#define EXPRESSIONCALC_EXPRESSION_CALC_H

#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <stack>
#include <string>
#include <vector>

#define LAMBDA_EXPR(expr) [](const double* params, const int num) -> double { return expr; }

namespace XCLZ {

enum class ErrorType {
    Well,
    FunctionNotFound,
    EvalError,
    SyntaxError,
    BracketNotMatched
};

typedef struct Error {
    ErrorType   type = ErrorType::Well;
    std::string msg;
} Error_t;

typedef std::map<char, int>              TokenLevel_t;
typedef std::vector<std::string>         Notation_t;
typedef std::string                      Expression_t;
typedef std::string                      String_t;
typedef std::string::const_iterator      ExpressionIt_t;
typedef std::map<ErrorType, std::string> ErrorType2Name_t;

typedef double (*MyFunc_t)(const double* params, const int num);
typedef std::map<std::string, std::pair<int, MyFunc_t>> NoteTable_t;

// Class Declaration
class eXpressionCalc {

private:
    ErrorType2Name_t ErrorType2Name;
    TokenLevel_t     TokenLevel;
    NoteTable_t      NoteTable;
    Error_t          Error;

    Expression_t   Expr;
    ExpressionIt_t ExprIt;

    void resetError();

    bool     eatWhitespace();
    bool     checkNumber();
    String_t readNumber();
    bool     checkFunc();
    String_t readFunc();
    bool     isNumber(const Expression_t& expr);

public:
    eXpressionCalc();
    Notation_t reversePolishNotation();
    double     evalNotation(const Notation_t& notation);

    void           setExpression(Expression_t expression);
    const Error_t& getError();
    String_t       errorToString();

    void addFunc(const String_t& note, int pNum, MyFunc_t func);
    void addToken(char token, int level);
};
}

#endif // EXPRESSIONCALC_EXPRESSION_CALC_H
