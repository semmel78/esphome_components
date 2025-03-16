import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import one_wire, sensor

from esphome.const import (
    CONF_RESOLUTION,
    DEVICE_CLASS_TEMPERATURE,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
)

empty_onewire_sensor_ns = cg.esphome_ns.namespace("empty_onewire_sensor")

EmptyOnewireSensor = empty_onewire_sensor_ns.class_(
    "EmptyOnewireSensor",
    cg.PollingComponent,
    sensor.Sensor,
    one_wire.OneWireDevice,
)

CONFIG_SCHEMA = (
    sensor.sensor_schema(
        EmptyOnewireSensor,
        unit_of_measurement=UNIT_CELSIUS,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_TEMPERATURE,
        state_class=STATE_CLASS_MEASUREMENT,
    )
    .extend(one_wire.one_wire_device_schema())
    .extend(cv.polling_component_schema("60s"))
)


async def to_code(config):
    var = await sensor.new_sensor(config)
    await cg.register_component(var, config)
    await one_wire.register_one_wire_device(var, config)