//
// Created by Eric Saturn on 2019/12/12.
//

#include "error.h"

using std::string;

namespace Net {
    namespace error {
        void printError(string error_info) {
            printf("\033[31mError: %s\033[0m\n", error_info.data());
        }

        void printWarning(string warning_info) {
            printf("\033[33mWarning: %s\033[0m\n", warning_info.data());
        }

        void printSuccess(string succes_info) {
            printf("\033[32m%s\033[0m\n", succes_info.data());
        }

        void printRed(string red_info) {
            printf("\033[31m%s\n\033[0m", red_info.data());
        }

        void printInfo(const string& info, string tag) {
            printf("[DEBUG INFO] %s ", info.data());
            if(tag.size())
                printf("{ %s }\n",tag.data());
        }

        void printInfoFormal(const string& title, initializer_list<FormalItem> body) {
            printf("\n>>>\n {%s}\n",title.data());
            printf(">-------------------------------------\n");
            for(auto item : body){
                printf("[%s] : \"%s\"; \n", item.first.data(), item.second.data());
            }
            printf("----------------------------------<\n<<<\n\n");
        }
    }
}