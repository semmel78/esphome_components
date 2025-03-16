import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import one_wire, sensor
from esphome.const import (
    CONF_RESOLUTION,
    DEVICE_CLASS_TEMPERATURE,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
)

empty_onewire_sensor_ns = cg.esphome_ns.namespace('empty_onewire_sensor')
EmptyOnewireSensor = empty_onewire_sensor_ns.class_(
    'EmptyOnewireSensor',
    cg.PollingComponent,
    sensor.Sensor,
    one_wire.OneWireDevice,
)

CONFIG_SCHEMA = (
    sensor.sensor_schema(
        UNIT_EMPTY,
        ICON_EMPTY,
        1,
    )
    .extend(
        {
            cv.GenerateID(): cv.declare_id(EmptyI2CSensor),
        }
    )
    .extend(one_wire.one_wire_device_schema())
    .extend(cv.polling_component_schema('60s'))
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    yield sensor.register_sensor(var, config)
    yield one_wire.register_one_wire_device(var, config)
    