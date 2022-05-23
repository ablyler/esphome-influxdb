#include <string>
#include "esphome/core/log.h"
#include "esphome/core/application.h"
#include "influxdb_writer.h"

#ifdef USE_LOGGER
#include "esphome/components/logger/logger.h"
#endif

namespace esphome
{
    namespace influxdb
    {
        static const char *TAG = "influxdb_jab";

        void InfluxDBWriter::setup()
        {
            ESP_LOGCONFIG(TAG, "Setting up InfluxDB Writer...");
            std::vector<EntityBase *> objs;
            for (auto fun : setup_callbacks)
                objs.push_back(fun());

            this->service_url = "http://"+this->host+":"+to_string(this->port)+"/api/v2/write?precision=ns&bucket="+this->database+"&org="+this->org;

            this->request_ = new http_request::HttpRequestComponent();
            this->request_->setup();

            this->request_->set_headers(this->get_http_headers());
            this->request_->set_method("GET");
            this->request_->set_useragent("ESPHome InfluxDB Bot");
            this->request_->set_timeout(this->send_timeout);

            if (publish_all)
            {
#ifdef USE_BINARY_SENSOR
                for (auto *obj : App.get_binary_sensors())
                {
                    if (!obj->is_internal() && std::none_of(objs.begin(), objs.end(), [&obj](EntityBase *o) { return o == obj; }))
                        obj->add_on_state_callback([this, obj](bool state) { this->on_sensor_update(obj, obj->get_object_id(), tags, state); });
                }
#endif
#ifdef USE_SENSOR
                for (auto *obj : App.get_sensors())
                {
                    if (!obj->is_internal() && std::none_of(objs.begin(), objs.end(), [&obj](EntityBase *o) { return o == obj; }))
                        obj->add_on_state_callback([this, obj](float state) { this->on_sensor_update(obj, obj->get_object_id(), tags, state); });
                }
#endif
#ifdef USE_TEXT_SENSOR
                for (auto *obj : App.get_text_sensors())
                {
                    if (!obj->is_internal() && std::none_of(objs.begin(), objs.end(), [&obj](EntityBase *o) { return o == obj; }))
                        obj->add_on_state_callback([this, obj](std::string state) { this->on_sensor_update(obj, obj->get_object_id(), tags, state); });
                }
#endif
            }
        }

        float InfluxDBWriter::get_setup_priority() const { return esphome::setup_priority::AFTER_CONNECTION; }

        std::list<http_request::Header> InfluxDBWriter::get_http_headers()
        {
            std::list<http_request::Header> headers;
            http_request::Header header;

            // accept
            header.name = "Accept";
            header.value = "application/json";
            headers.push_back(header);

            // content type
            header.name = "Content-Type";
            header.value = "text/plain; charset=utf-8";
            headers.push_back(header);

            if ((this->token_header.length() > 0))
            {
                header.name = "Authorization";
                header.value = this->token_header.c_str();
                headers.push_back(header);
            }
            else if ((this->username.length() > 0) && (this->password.length() > 0))
            {
                header.name = "u";
                header.value = this->username.c_str();
                headers.push_back(header);
                header.name = "p";
                header.value = this->password.c_str();
                headers.push_back(header);
            }

            return headers;
        }

        void InfluxDBWriter::loop()
        {
        }

        void InfluxDBWriter::write(std::string measurement, std::string tags, const std::string value, bool is_string)
        {
            std::string line = measurement + tags + " value=" + (is_string ? ("\"" + value + "\"") : value);
            this->request_->set_url(this->service_url);
            this->request_->set_headers(this->get_http_headers());
            this->request_->set_body(line.c_str());
            this->request_->set_method("POST");
            const std::vector<esphome::http_request::HttpRequestResponseTrigger *> triggers;
            this->request_->send(triggers);
            this->request_->close();

            ESP_LOGD(TAG, "InfluxDB packet: %s", line.c_str());
        }

        void InfluxDBWriter::dump_config()
        {
            ESP_LOGCONFIG(TAG, "InfluxDB Writer:");
            ESP_LOGCONFIG(TAG, "  Address: %s:%u", host.c_str(), port);
            ESP_LOGCONFIG(TAG, "  Database: %s", database.c_str());
            ESP_LOGCONFIG(TAG, "  Org: %s", org.c_str());
        }

#ifdef USE_BINARY_SENSOR
        void InfluxDBWriter::on_sensor_update(binary_sensor::BinarySensor *obj, std::string measurement, std::string tags, bool state)
        {
            write(measurement, tags, state ? "t" : "f", false);
        }
#endif

#ifdef USE_SENSOR
        void InfluxDBWriter::on_sensor_update(sensor::Sensor *obj, std::string measurement, std::string tags, float state)
        {
            if(!isnan(state))
                write(measurement, tags, to_string(state), false);
        }
#endif

#ifdef USE_TEXT_SENSOR
        void InfluxDBWriter::on_sensor_update(text_sensor::TextSensor *obj, std::string measurement, std::string tags, std::string state)
        {
            write(measurement, tags, state, true);
        }
#endif

    } // namespace influxdb
} // namespace esphome
