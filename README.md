# InfluxDB custom component for ESPHome
Credit goes to la7dja @ https://github.com/la7dja for this great template. I used his existing UDP version to use the ESPHome http_request module.

This custom component allows you to send sensor updates from ESPHome to an InfluxDB database.


## Installation

Clone this repository into `custom_components/influxdb` from the directory
where your ESPHome configuration is stored.

```bash
git clone https://github.com/jabastien/esphome-influxdb custom_components/influxdb
```

## Usage

Add `influxdb` section to your ESPHome configuration file.

### Example configuration

```yaml
influxdb:
  host: "influxdb-host"
  database: "$(devicename)_db"
  username: !secret influxdb_user
  password: !secret influxdb_pass
  
  sensors:
    meter_id:
      ignore: True
    ams_temperature:
      measurement: 'temperature'
      tags: 
        room: kitchen
```

### Configuration variables

* **host** (Required, string): Hostname or IP for the InfluxDB server
* **port** (Optional, int): Port number the InfluxDB server is listening on. Defaults to <del>8089</del> 8086.
* **org** (Required, string): Org name for InfluxDB
* **username** (Optional, string): Username for InfluxDB Authentication
* **password** (Optional, string): Password for InfluxDB Authentication
* **api_token** (Optional, string): Token for InfluxDB Authentication
* **database** (Optional, default = 'esphome'): Auto create Database if does not exist.
* <del>**max\_packet_size** (Optional, int): Max size of UDP packets. Defaults to 500.</del>
* <del>**send_timeout** (Optional, time): Time to wait before sending UDP packets which have not been filled to max size. Defaults to 100ms.</del>
* **publish_all** (Optional, boolean): If true, publish updates from all sensors unless explicitly ignored in per sensor configuration. If false, only publish updates from sensors explicitly configured. Defaults to True.
* **tags** (Optional, mapping): Mapping of tag keys and values. Defaults to 'node: <esphome.name>'.
* **sensors** (Optional, mapping): Per sensor configuration. Keys are sensor IDs. All types of sensors are included in this mapping, there is no distinction between float, binary and text sensors.

#### Sensor configuration variables

* **ignore** (Optional, boolean): Whether or not to include updates for this sensor. Defaults to True.
* **measurement** (Optional, string): Name of measurements with update from this sensor. Defaults to the sanitized name of the sensor.
* **tags** (Optional, mapping): Additional tags added for this sensor.


