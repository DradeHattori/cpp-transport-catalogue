//main.cpp
#include "json.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "log_duration.h"

int main() {
    try {
        LOG_DURATION("Program");
        using namespace transport::catalogue;

        json::Document input_doc = json::Load(std::cin);

        TransportCatalogue catalogue;

        JsonReader json_reader(catalogue);

        json_reader.LoadData(input_doc);
        
        
        json_reader.ProcessRequests(input_doc, std::cout);

    

    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
