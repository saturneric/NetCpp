//
// Created by Eric Saturn on 2019/12/14.
//

#ifndef NET_OPTS_PROCESSOR_H
#define NET_OPTS_PROCESSOR_H

#include "type.h"

#include <boost/program_options.hpp>

namespace po = boost::program_options;

class OptsProcessor {
public:
    OptsProcessor(int argc, char *argv[]){
        desc.add_options()
                ("help", "help list")
                ("init", "set up the environment")
                ("construct", "construct new project")
                ("update", "update changes done to a project")
                ("server", "start a server daemon")
                ("client", "start a client daemon")
                ("set", "change an option");
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            cout << desc << "\n";
            return 1;
        }

        if (vm.count("compression")) {
            cout << "Compression level was set to "
                 << vm["compression"].as<int>() << ".\n";
        } else {
            cout << "Compression level was not set.\n";
        }
    }
private:
    po::options_description desc("General Net Tools (0.0.1) By Saturn&Eric");
};


#endif //NET_OPTS_PROCESSOR_H
