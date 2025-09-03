# esphome/components/ws2811_channels/__init__.py
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import light
from esphome.const import CONF_ID, CONF_NAME

AUTO_LOAD = ["light"]
DEPENDENCIES = ["light"]

CONF_STRIP_ID = "strip_id"
CONF_CHANNELS = "channels"
CONF_PIXEL = "pixel"
CONF_COLOR = "color"

ns = cg.esphome_ns.namespace("ws2811_channels")
Controller   = ns.class_("WS2811ChannelsController", cg.Component)
ChannelLight = ns.class_("WS2811ChannelLight", light.LightOutput)

# erlaubt: color = "R"|"G"|"B"
COLOR_MAP = {"R": 0, "G": 1, "B": 2}

CHANNEL_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(ChannelLight),         # <- jede Channel-Light bekommt eine eigene ID
        cv.Required(CONF_PIXEL): cv.int_range(min=0),         # 0-basiert
        cv.Required(CONF_COLOR): cv.one_of(*COLOR_MAP.keys(), upper=True),
        cv.Optional(CONF_NAME): cv.string,
    }
)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(Controller),           # Controller-ID
        cv.Required(CONF_STRIP_ID): cv.use_id(light.AddressableLightState),
        cv.Required(CONF_CHANNELS): cv.ensure_list(CHANNEL_SCHEMA),
    }
).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    strip_state = await cg.get_variable(config[CONF_STRIP_ID])  # AddressableLightState*
    # num_pixels nicht mehr nötig im Schema; Controller kann optional Größe prüfen, falls gewünscht
    ctrl = cg.new_Pvariable(config[CONF_ID], Controller, strip_state, 0)  # 0 = optional/ignoriert
    await cg.register_component(ctrl, config)

    for ch in config[CONF_CHANNELS]:
        pixel = ch[CONF_PIXEL]
        color = COLOR_MAP[ch[CONF_COLOR]]
        var   = cg.new_Pvariable(ch[CONF_ID], ChannelLight, ctrl, pixel, color)
        name  = ch.get(CONF_NAME)
        if name is None:
            name = f"Pixel {pixel+1} - {'RGB'[color]}"
        await light.register_light(var, {CONF_NAME: name})
