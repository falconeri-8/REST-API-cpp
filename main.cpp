#include <wx/wx.h>
#include <wx/combobox.h>
#include <sstream>
#include <string>

// Temporary, Set CMakeList to get these libraries
#include "Modules/DataManager.h"
#include "Modules/httplib.h"
#include "Modules/json.hpp"

using json = nlohmann::json;
using namespace std;

auto API_BASE_URL = "https://api.pirateweather.net";
auto API_KEY      = "YOUR API KEY";

class WeatherFrame : public wxFrame {
public:
    WeatherFrame()
        : wxFrame(nullptr, wxID_ANY, "Weather App", wxDefaultPosition, wxSize(800, 600))
    {
        // Main vertical sizer
        wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

        // Top bar panel
        wxPanel* topPanel = new wxPanel(this, wxID_ANY);
        wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);

        // Latitude input
        latInput = new wxTextCtrl(topPanel, wxID_ANY, "45.42", wxDefaultPosition, wxSize(100, -1));
        lonInput = new wxTextCtrl(topPanel, wxID_ANY, "-74.30", wxDefaultPosition, wxSize(100, -1));

        // Filter dropdown
        wxArrayString filterChoices;
        filterChoices.Add("currently");
        filterChoices.Add("minutely");
        filterChoices.Add("hourly");
        filterChoices.Add("daily");
        filterChoices.Add("alerts");

        filterDropdown = new wxComboBox(topPanel, wxID_ANY, "currently", wxDefaultPosition, wxSize(150, -1), filterChoices, wxCB_READONLY);

        // View button
        wxButton* viewBtn = new wxButton(topPanel, wxID_ANY, "View");

        // Add to top bar
        topSizer->Add(new wxStaticText(topPanel, wxID_ANY, "Latitude:"), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
        topSizer->Add(latInput, 0, wxALL, 5);
        topSizer->Add(new wxStaticText(topPanel, wxID_ANY, "Longitude:"), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
        topSizer->Add(lonInput, 0, wxALL, 5);
        topSizer->Add(new wxStaticText(topPanel, wxID_ANY, "Filter:"), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
        topSizer->Add(filterDropdown, 0, wxALL, 5);
        topSizer->Add(viewBtn, 0, wxALL, 5);

        topPanel->SetSizer(topSizer);

        // Display box for results
        displayBox = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);

        // Add panels to main sizer
        mainSizer->Add(topPanel, 0, wxEXPAND | wxALL, 5);
        mainSizer->Add(displayBox, 1, wxEXPAND | wxALL, 5);

        SetSizer(mainSizer);

        // Event binding
        viewBtn->Bind(wxEVT_BUTTON, &WeatherFrame::OnFetchWeather, this);
    }

private:
    wxTextCtrl* latInput;
    wxTextCtrl* lonInput;
    wxComboBox* filterDropdown;
    wxTextCtrl* displayBox;

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

    void OnFetchWeather(wxCommandEvent&) {
        string lat = latInput->GetValue().ToStdString();
        string lon = lonInput->GetValue().ToStdString();
        string filter = filterDropdown->GetValue().ToStdString();

        string excluded = buildExcludedFilters("currently,minutely,hourly,daily,alerts", filter);

        try {
            httplib::Client apiClient(API_BASE_URL);
            auto result = apiClient.Get(
                "/forecast/" + string(API_KEY) + "/" + lat + "," + lon + "?exclude=" + excluded
            );

            if (!result) {
                displayBox->SetValue("Error: No response from API.");
                return;
            }

            if (result->status != 200) {
                displayBox->SetValue("Error: HTTP " + to_string(result->status));
                return;
            }

            json parsed = json::parse(result->body);

            // Pretty-print the selected filter
            if (parsed.contains(filter)) {
                std::ostringstream oss;
                if (filter == "currently") {
                    oss << "Weather (Currently):\n";
                    for (auto& [k, v] : parsed[filter].items()) {
                        oss << k << ": " << v << "\n";
                    }
                } else if (filter == "hourly" || filter == "daily") {
                    oss << "Weather (" << filter << "):\n";
                    int count = 0;
                    for (auto& entry : parsed[filter]["data"]) {
                        oss << "Entry " << ++count << ":\n";
                        for (auto& [k, v] : entry.items()) {
                            oss << "  " << k << ": " << v << "\n";
                        }
                        oss << "\n";
                        if (count >= 5) break; // Limit preview
                    }
                } else {
                    oss << parsed.dump(4);
                }
                displayBox->SetValue(oss.str());
            } else {
                displayBox->SetValue("No data for filter: " + filter);
            }

        } catch (const exception& e) {
            displayBox->SetValue("Error: " + string(e.what()));
        }
    }
};

class WeatherApp : public wxApp {
public:
    bool OnInit() override {
        WeatherFrame* frame = new WeatherFrame();
        frame->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(WeatherApp);
