#include <iostream>
#include <fstream>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response) {
    size_t totalSize = size * nmemb;
    response->append((char*)contents, totalSize);
    return totalSize;
}


bool getLatLongFromAddress(const std::string& address, double& latitude, double& longitude) {
    CURL* curl;
    CURLcode res;
    std::string response;
    
    std::string encodedAddress = curl_easy_escape(curl, address.c_str(), address.size());
    
 
    std::string url = "https://msearch.gsi.go.jp/address-search/AddressSearch?q=" + encodedAddress;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

  
        res = curl_easy_perform(curl);
        
        if(res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return false;
        }
        
        curl_easy_cleanup(curl);
    }
    
    curl_global_cleanup();


    try {
        auto jsonResponse = json::parse(response);
        if (!jsonResponse.empty()) {
            latitude = jsonResponse[0]["geometry"]["coordinates"][1];
            longitude = jsonResponse[0]["geometry"]["coordinates"][0];
            return true;
        } else {
            std::cerr << "Address not found in the response" << std::endl;
            return false;
        }
    } catch (json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return false;
    }
}

int main() {
    std::ifstream inputFile("input.csv");
    std::ofstream outputFile("output.csv");
    std::string address;
    
    if (!inputFile.is_open()) {
        std::cerr << "Failed to open input.csv" << std::endl;
        return 1;
    }
    
    if (!outputFile.is_open()) {
        std::cerr << "Failed to open output.csv" << std::endl;
        return 1;
    }
    
    outputFile << "Latitude,Longitude,Address\n";  

 
     while (std::getline(inputFile, address)) {
        if (address.empty() || address == "住所") {
            continue;  
        }

        double latitude, longitude;

     
        if (getLatLongFromAddress(address, latitude, longitude)) {
        
            outputFile << latitude << "," << longitude << "," << address << "\n"; 
            std::cout << "Processed: " << address << " -> Latitude: " << latitude << ", Longitude: " << longitude << std::endl;
        } else {
            std::cerr << "Failed to get latitude and longitude for the address: " << address << std::endl;
        }
    }

    inputFile.close();
    outputFile.close();
    return 0;
}