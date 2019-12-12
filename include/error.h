//
// Created by Eric Saturn on 2019/12/12.
//

#ifndef NET_ERROR_H
#define NET_ERROR_H

#include <string>
#include <map>

using std::string;
using std::initializer_list;
using std::pair;

//提示信息打印类函数
namespace Net {
    namespace error {

        using FormalItem = pair<string, string>;

        void printError(string error_info);

        void printWarning(string warning_info);

        void printSuccess(string succes_info);

        void printRed(string red_info);

        void printInfo(const string& info, string tag = "");

        void printInfoBuffer(const string& info, string tag = "");

        void printInfoFormal(const string& title, initializer_list<FormalItem> body);
    }
}


#endif //NET_ERROR_H
