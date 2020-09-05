//
// Created by Eric Saturn on 2019/12/12.
//

#ifndef NET_PRINT_TOOLS_H
#define NET_PRINT_TOOLS_H

#include <string>
#include <map>

using std::string;
using std::initializer_list;
using std::pair;

//提示信息打印类函数
namespace Net {

    namespace PrintTools {

        using FormalItem = pair<string, string>;

        void debugPrintError(const string &error_info);

        void debugPrintWarning(const string &warning_info);

        void debugPrintSuccess(const string &success_info);

        void printInfo(const string& info, const string& tag);

        void printInfoBuffer(const string& info, const string& tag);

        void printInfoFormal(const string& title, initializer_list<FormalItem> body);
    }
}


#endif //NET_PRINT_TOOLS_H
