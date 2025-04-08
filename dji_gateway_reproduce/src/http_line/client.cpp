#include <iostream>
#include <fstream>
#include "../../3rdParty/cpp-httplib-master/httplib.h"
#include "../utility/include/dlog.h"

int main() {
    using namespace dji;
    httplib::Client cli("172.21.204.118", 9191);

    std::ifstream file("/home/nakanomiku/wxx/intra-train/Intra_training/dji_gateway_reproduce/upload_test/testfile.conf");
    if (!file) {
        std::cerr << "Failed to open file." << std::endl;
        dlog::LogWarn(__func__, "Failed to open file");
        return 1;
    }

    std::string fileContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    
    auto res = cli.Post("/upload", fileContent, "text/plain");

    if (res) {
        std::cout << "Response Status: " << res->status << std::endl;
        //dlog::LogInfo(__func__, "")
        std::cout << "Response Body: " << res->body << std::endl;
    } else {
        std::cerr << "Request failed: " << res.error() << std::endl;
    }

    return 0;
}