//
//  AppProcessor.hpp
//  Database5
//
//  Created by rick gessner on 3/30/23.
//  Copyright © 2018-2023 rick gessner. All rights reserved.
//

#ifndef AppController_hpp
#define AppController_hpp

#include <cstdio>
#include "misc/Config.hpp"
#include "misc/Errors.hpp"
#include "view/View.hpp"
#include "CommandProcessor.hpp"

namespace ECE141 {

    class AppController {
    public:

        AppController();
        AppController(std::ostream &anOutput);
        virtual ~AppController();

        //app api...
        virtual StatusResult  handleInput(std::istream &anInput,
                                          ViewListener aViewer);
        bool          isRunning() const {return running;}

        OptString     getError(StatusResult &aResult) const;

        bool running;


    };

}

#endif /* AppController_hpp */