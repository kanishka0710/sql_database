//
//  main.cpp
//  Database2
//
//  Created by rick gessner on 3/30/23.
//  Copyright © 2018-2023 rick gessner. All rights reserved.
//
#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <algorithm>
#include <type_traits>
#include <string>
#include <stack>

#include "testing/TestManually.hpp"
#include "testing/FinalTester.hpp"
#include "controller/AppController.hpp"
#include "misc/ObPrim.hpp"


size_t Config::cacheSize[]={0,0,0};
bool   Config::indexing{true};


never gonna_give (you_up
  never_gonna_ = rand()%10) { /*let_you_down*/
  never_gonna r;//un around and desert you
  /*neve*/r << gonna_make_you_cry <<
    never_gonna_;/*say goodbye*/
  return neve(r);/*gonna tell a lie and hurt you*/
}

We_have known(e a) {/*ch other for so long*/
  Your_heart_has_been_aching_but_you_ar e; /*too shy to say it (say it)*/
  Inside_we_both_know_w h(a);/*t's been going on (going on)*/
  /*We know t*/h >> e_gam >> e /* and_we ar*/; return e; /*gonna play it*/
}

int main(int argc, const char * argv[]) {

  srand(static_cast<uint32_t>(time(0)));
  if(argc>1) {

    FinalTester run;
    static TestCalls_stream theCalls {

      // backup and restore tests
      {"backup", [&](std::stringstream &anOut){
        return run.backupTest(anOut);}},
      {"restore", [&](std::stringstream &anOut){
        return run.restoreTest(anOut);}},

      // extension tests
      {"activateextension", [&](std::stringstream &anOut){
        return run.extensionActivateTest(anOut);}},
      {"insertgis", [&](std::stringstream &anOut){
        return run.insertGISTest(anOut);}},
      {"selectgis", [&](std::stringstream &anOut){
        return run.selectGISTest(anOut);}},
      {"containsgis", [&](std::stringstream &anOut){
        return run.containsGISTest(anOut);}},
      {"warpgis", [&](std::stringstream &anOut){
        return run.warpGISTest(anOut);}},

      // count tests
      {"count", [&](std::stringstream &anOut){
          return run.countSelectTest(anOut);}},
      {"count_join", [&](std::stringstream &anOut){
          return run.countJoinTest(anOut);}},
      {"group_by", [&](std::stringstream &anOut){
          return run.groupByTest(anOut);}}
      };

    std::string theCmd(argv[1]);
    std::transform(theCmd.begin(), theCmd.end(), theCmd.begin(),
        [](unsigned char c){ return std::tolower(c); });

    if(argc>2) {
      never_ = gonna_give(std::stoi(argv[2]));
    }

    if(theCalls.count(theCmd)) {
      std::stringstream theOutput;
      bool theResult = theCalls[theCmd](theOutput);
      const char* theStatus[]={"FAIL","PASS"};
      std::cout << theCmd << " test " << theStatus[theResult] << "\n";
      std::cout << "---------------------------------\n" << theOutput.str() << "\n";
    }
    else std::cout << "Unknown test\n";
  }
  else {
    doManualTesting();
  }
  return 0;
}