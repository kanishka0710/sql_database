//
//  TestManually.hpp
//  Database
//
//  Created by rick gessner on 3/30/23.
//  Copyright © 2018-2023 rick gessner. All rights reserved.
//


#ifndef TestManually_h
#define TestManually_h

#include <sstream>
#include "controller/AppController.hpp"

using namespace ECE141;

bool doManualTesting() {
  ECE141::AppController   theApp(std::cout);
  ECE141::StatusResult    theResult{};

  std::string theUserInput;

  while(theApp.isRunning()) {
    std::cout << "\n> ";
    if(std::getline(std::cin, theUserInput)) {
      if(theUserInput.length()) {
        auto theTimer=Config::getTimer();
        auto theStart=theTimer.now();
        std::stringstream theStream(theUserInput);
        theResult=theApp.handleInput(theStream, [&](View &aView) {
          aView.show(std::cout);
          std::cout << "\nElapsed: " << std::fixed
            << theTimer.elapsed(theStart);
          std::cout << std::endl;
        });
      }
    }
  }
  return theResult;
}

#endif /* TestManually_h */
