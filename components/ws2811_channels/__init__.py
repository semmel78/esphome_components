# esphome/components/ws2811_channels/__init__.py
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import light
from esphome.const import CONF_ID, CONF_NAME

AUTO_LOAD = ["light"]
DEPENDENCIES = ["light"]

CONF_STRIP_ID = "strip_id"
CONF_NUM_PIXELS = "num_pixels"
CONF_NAME_PREFIX = "name_prefix"

ns = cg.esphome_ns.namespace("ws2811_channels")
Controller   = ns.class_("WS2811ChannelsController", cg.Component)
ChannelLight = ns.class_("WS2811ChannelLight", light.LightOutput)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(Controller),
        cv.Required(CONF_STRIP_ID): cv.use_id(light.AddressableLightState),
        cv.Required(CONF_NUM_PIXELS): cv.int_range(min=1),
        cv.Optional(CONF_NAME_PREFIX, default="Pixel"): cv.string,
    }
).extend(cv.COMPONENT_SCHEMA)   # <<< WICHTIG

async def to_code(config):
    strip_state = await cg.get_variable(config[CONF_STRIP_ID])  # AddressableLightState*
    num_pixels  = config[CONF_NUM_PIXELS]
    name_prefix = config[CONF_NAME_PREFIX]
    colors = ["Red", "Green", "Blue"]

    # Controller mit ID erzeugen
    ctrl = cg.new_Pvariable(config[CONF_ID], Controller, strip_state, num_pixels)

    # <<< WICHTIG: register_component mit config aufrufen
    await cg.register_component(ctrl, config)

    # Channel-Lights anlegen
    for p in range(num_pixels):
        for c, cname in enumerate(colors):
            light_id = cg.new_id()
            var = cg.new_Pvariable(light_id, ChannelLight, ctrl, p, c)
            await light.register_light(var, {CONF_NAME: f"{name_prefix} {p+1} - {cname}"})
