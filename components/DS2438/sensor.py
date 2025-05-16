import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import one_wire, sensor
from esphome.const import (
    CONF_HUMIDITY,
    CONF_ID,
    CONF_TEMPERATURE,
    CONF_VOLTAGE,
    CONF_VARIANT,
    DEVICE_CLASS_HUMIDITY,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_VOLTAGE,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
    UNIT_PERCENT,
    UNIT_VOLT,
)

DEPENDENCIES = ["one_wire"]

ds2438_ns = cg.esphome_ns.namespace("ds2438")

DS2438Sensor = ds2438_ns.class_(
    "DS2438Sensor",
    cg.PollingComponent,
    one_wire.OneWireDevice,
    cg.EntityBase
    )

# Configuration keys for DS2438 device entries.
CONF_DS2438 = "ds2438"
CONF_VAD = "voltage_vad"
CONF_VDD = "voltage_vdd"

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(DS2438Sensor),
            cv.Optional(CONF_TEMPERATURE): sensor.sensor_schema(
                unit_of_measurement=UNIT_CELSIUS,
                accuracy_decimals=2,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_HUMIDITY): sensor.sensor_schema(
                unit_of_measurement=UNIT_PERCENT,
                accuracy_decimals=2,
                device_class=DEVICE_CLASS_HUMIDITY,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_VAD): sensor.sensor_schema(
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=2,
                device_class=DEVICE_CLASS_VOLTAGE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_VDD): sensor.sensor_schema(
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=2,
                device_class=DEVICE_CLASS_VOLTAGE,
                state_class=STATE_CLASS_MEASUREMENT,
            )
        }
    )
    .extend(one_wire.one_wire_device_schema())
    .extend(cv.polling_component_schema("60s"))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await one_wire.register_one_wire_device(var, config)
    
    if temp_sensor_config  := config.get(CONF_TEMPERATURE):
        temp_sensor = await sensor.new_sensor(temp_sensor_config)
        cg.add(var.set_temperature_sensor(temp_sensor))
    if vad_sensor_config := config.get(CONF_VAD):
        vad_sensor = await sensor.new_sensor(vad_sensor_config)
        cg.add(var.set_vad_sensor(vad_sensor))
    if vdd_sensor_config := config.get(CONF_VDD):
        vdd_sensor = await sensor.new_sensor(vdd_sensor_config)
        cg.add(var.set_vdd_sensor(vdd_sensor))
    if hum_sensor_config := config.get(CONF_HUMIDITY):
        hum_sensor = await sensor.new_sensor(hum_sensor_config)
        cg.add(var.set_humidity_sensor(hum_sensor))
