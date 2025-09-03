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
WS2811ChannelLight = ns.class_("WS2811ChannelLight", light.LightOutput)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_STRIP_ID): cv.use_id(light.AddressableLight),
        cv.Required(CONF_NUM_PIXELS): cv.int_range(min=1),
        cv.Optional(CONF_NAME_PREFIX, default="Pixel"): cv.string,
    }
)

async def to_code(config):
    strip = await cg.get_variable(config[CONF_STRIP_ID])
    num_pixels = config[CONF_NUM_PIXELS]
    name_prefix = config[CONF_NAME_PREFIX]
    colors = ["Red", "Green", "Blue"]

    for p in range(num_pixels):
        for c, cname in enumerate(colors):
            var = cg.new_Pvariable(WS2811ChannelLight, strip, p, c)
            await light.register_light(var, {CONF_NAME: f"{name_prefix} {p+1} - {cname}"})
