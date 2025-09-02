#include <iostream>
#include <sstream>
#include <string>

// Temporary, Set CMakeList to get these libraries
#include "Modules/httplib.h"
#include "Modules/DataManager.h"
#include "Modules/json.hpp"

using json = nlohmann::json;
using namespace std;

auto API_BASE_URL = "https://api.pirateweather.net";
auto API_KEY      = "YOUR_API_KEY";

string buildExcludedFilters(const string& excluded, const string& filterToRemove) {
    stringstream ss(excluded);
    string item, cleaned;

    while (getline(ss, item, ',')) {
        if (item != filterToRemove && !item.empty()) {
            if (!cleaned.empty()) cleaned += ",";
            cleaned += item;
        }
    }
    return cleaned;
}

pair<string, string> getCoordinates() {
    string lat, lon;
    cout << "Enter Latitude: ";
    cin >> lat;
    cout << "Enter Longitude: ";
    cin >> lon;
    return {lat, lon};
}

string getFilterChoice() {
    cout << "\nFilter by:\n"
              << "[1] Current\n"
              << "[2] Minutely\n"
              << "[3] Hourly\n"
              << "[4] Daily\n";

    int choice = 0;
    cout << "\nEnter Choice: ";
    cin >> choice;

    switch (choice) {
        case 1: return "currently";
        case 2: return "minutely";
        case 3: return "hourly";
        case 4: return "daily";
        default:
            throw invalid_argument("Invalid input. Please choose 1–4.");
    }
}

int main() {
    try {
        cout << "RESTful API C++ Prototype: Weather Logging\n\n";

        auto [lat, lon] = getCoordinates();
        string filter = getFilterChoice();
        string excluded = buildExcludedFilters(
            "currently,minutely,hourly,daily,alerts", filter
        );

        httplib::Client apiClient(API_BASE_URL);
        httplib::Headers authHeaders = {
            {"Content-Type", "application/json"}
        };

        auto result = apiClient.Get(
            "/forecast/" + string(API_KEY) + "/" + lat + "," + lon + "?exclude=" + excluded,
            authHeaders
        );

        if (!result) {
            cerr << "Error: No response from API.\n";
            return 1;
        }

        if (result->status != 200) {
            cerr << "Error: HTTP " << result->status << "\n";
            return 1;
        }

        try {
            json parsed = json::parse(result->body);
            cout << parsed.dump(4) << "\n";
            Write("Islamabad.txt", parsed.dump(4));
        } catch (const exception& e) {
            cerr << "Error parsing JSON: " << e.what() << "\n";
            return 1;
        }

    } catch (const exception& e) {
        cerr << "Fatal Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
