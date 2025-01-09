    //
    //  Tokenizer.hpp
    //  Database
    //
    //  Created by rick gessner on 3/19/19.
    //  Copyright © 2019 rick gessner. All rights reserved.
    //

    #ifndef Tokenizer_hpp
    #define Tokenizer_hpp

    #include <iostream>
    #include <vector>
    #include <cstring>
    #include <functional>
    #include "keywords.hpp"
    #include "Scanner.hpp"
    #include "misc/BasicTypes.hpp"

    namespace ECE141 {

      using parseCallback = bool(char aChar);


      //-----------------

      struct Token {

        Token& operator=(const Token &aCopy) {
          type=aCopy.type;
          keyword=aCopy.keyword;
          data=aCopy.data;
          return *this;
        }

          bool operator==(const Token &aCopy) const {
              return type==aCopy.type && keyword==aCopy.keyword && data==aCopy.data;
          }


          TokenType   type;
        Keywords    keyword;
        Operators   op;
        std::string data;
      };

      //-----------------

      using TokenVisitor = std::function<bool(const Token& aToken)>;

      class Tokenizer : public Scanner {
      public:
        Tokenizer(std::istream &anInputStream);

        StatusResult  tokenize();
        Token&        tokenAt(size_t anOffset);

        Token&        current();
        bool          more() {return index<size();}
        bool          next(int anOffset=1);
        Token&        peek(int anOffset=1);
        void          restart() {index=0;}
        size_t        size() {return tokens.size();}
        size_t        pos() {return index;}
        size_t        remaining() {return index<size() ? size()-index :0;}

                              //these might consume a token...
        bool          skipTo(Keywords aKeyword);
        bool          skipTo(TokenType aTokenType);

        bool          skipIf(Keywords aKeyword);
        bool          skipIf(Operators anOperator);
        bool          skipIf(TokenType aTokenType);
        bool          skipIf(char aChar);

        bool          each(const TokenVisitor aVisitor);
        void          dump(); //utility

          Token &peekStart(int anOffset);

      protected:

        std::vector<Token>    tokens;
        size_t                index;

          void goBackToStart();

      };

    }

    #endif /* Tokenizer_hpp */


