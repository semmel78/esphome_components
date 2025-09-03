# esphome/components/ws2811_channels/__init__.py
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import light
from esphome.const import (
    CONF_ID,
    CONF_NAME,
    CONF_DISABLED_BY_DEFAULT,
    CONF_RESTORE_MODE,
)

from esphome.components.light import LightRestoreMode as LRM

AUTO_LOAD = ["light"]
DEPENDENCIES = ["light"]
MULTI_CONF = True  # erlaubt mehrere Instanzen in einer YAML

CONF_STRIP_ID = "strip_id"
CONF_CHANNELS = "channels"
CONF_PIXEL = "pixel"
CONF_COLOR = "color"
CONF_OUTPUT_ID = "output_id"   # ID für unser LightOutput-Objekt

# C++-Typen
ns = cg.esphome_ns.namespace("ws2811_channels")
Controller = ns.class_("WS2811ChannelsController", cg.Component)
ChannelLight = ns.class_("WS2811ChannelLight", light.LightOutput)

# Mapping für Farbangaben
COLOR_MAP = {"R": 0, "G": 1, "B": 2}

# Schema für EINEN Kanal
CHANNEL_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(ChannelLight),   # Output-ID (LightOutput)
        cv.GenerateID(): cv.declare_id(light.LightState),             # LightState-ID
        cv.Required(CONF_PIXEL): cv.int_range(min=0),
        cv.Required(CONF_COLOR): cv.one_of(*COLOR_MAP.keys(), upper=True),
        cv.Optional(CONF_NAME): cv.string,
    }
)

# Top-Level-Schema
CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(Controller),                   # Controller-ID
        cv.Required(CONF_STRIP_ID): cv.use_id(light.AddressableLightState),
        cv.Required(CONF_CHANNELS): cv.ensure_list(CHANNEL_SCHEMA),
    }
).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    strip_state = await cg.get_variable(config[CONF_STRIP_ID])  # AddressableLightState*

    # Controller erzeugen (kein Klassen-Argument übergeben!)
    ctrl = cg.new_Pvariable(config[CONF_ID], strip_state, 0)
    await cg.register_component(ctrl, config)

    for ch in config[CONF_CHANNELS]:
        pixel = ch[CONF_PIXEL]
        color = COLOR_MAP[ch[CONF_COLOR]]

        # Channel-LightOutput erzeugen
        out_var = cg.new_Pvariable(ch[CONF_OUTPUT_ID], ctrl, pixel, color)

        # Name aus YAML oder automatisch
        name = ch.get(CONF_NAME) or f"Pixel {pixel+1} - {'RGB'[color]}"

        # LightState registrieren mit allen Pflichtfeldern
        await light.register_light(out_var, {
            CONF_ID: ch[CONF_ID],
            CONF_NAME: name,
            CONF_DISABLED_BY_DEFAULT: False,
            CONF_RESTORE_MODE: LRM.LIGHT_RESTORE_DEFAULT_OFF,
        })
