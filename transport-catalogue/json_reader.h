#pragma once

#include "transport_catalogue.h"
#include "json.h"

namespace transport {
    namespace catalogue {

        class JsonReader {
        public:
            JsonReader(TransportCatalogue& tc) : catalogue_(tc) {}
            void LoadData(const json::Document& doc);

            void ProcessRequests(const json::Document& doc, std::ostream& output);

        private:
            TransportCatalogue& catalogue_;
        };

    } // namespace catalogue
}  // namespace transport