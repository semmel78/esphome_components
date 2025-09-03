# esphome/components/ws2811_channels/__init__.py
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import light
from esphome.const import CONF_ID, CONF_NAME, CONF_DISABLED_BY_DEFAULT, CONF_RESTORE_MODE
from esphome.components.light import LightRestoreMode

# Diese Komponente kann in YAML direkt als Top-Level-Key "ws2811_channels:" benutzt werden.
AUTO_LOAD = ["light"]
DEPENDENCIES = ["light"]
MULTI_CONF = True  # erlaubt mehrere Instanzen in einer YAML

CONF_STRIP_ID   = "strip_id"
CONF_CHANNELS   = "channels"
CONF_PIXEL      = "pixel"
CONF_COLOR      = "color"
CONF_OUTPUT_ID  = "output_id"   # ID für unser LightOutput-Objekt

# C++-Typen registrieren
ns = cg.esphome_ns.namespace("ws2811_channels")
Controller   = ns.class_("WS2811ChannelsController", cg.Component)
ChannelLight = ns.class_("WS2811ChannelLight", light.LightOutput)

# Mapping der Farbangabe im YAML
COLOR_MAP = {"R": 0, "G": 1, "B": 2}

# Schema für EINEN Kanal (ein Monolight für R/G/B eines Pixels)
CHANNEL_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(ChannelLight),      # Output-ID (C++ LightOutput)
        cv.GenerateID(): cv.declare_id(light.LightState),                 # LightState-ID (für register_light)
        cv.Required(CONF_PIXEL): cv.int_range(min=0),                     # 0-basiert
        cv.Required(CONF_COLOR): cv.one_of(*COLOR_MAP.keys(), upper=True),
        cv.Optional(CONF_NAME): cv.string,
    }
)

# Top-Level YAML-Schema: ws2811_channels:
CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(Controller),                       # Controller-ID
        cv.Required(CONF_STRIP_ID): cv.use_id(light.AddressableLightState),
        cv.Required(CONF_CHANNELS): cv.ensure_list(CHANNEL_SCHEMA),
    }
).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    # Strip-State vom Addressable-Light (neopixelbus) holen
    strip_state = await cg.get_variable(config[CONF_STRIP_ID])  # AddressableLightState*

    # Controller erzeugen & registrieren
    ctrl = cg.new_Pvariable(config[CONF_ID], Controller, strip_state, 0)  # 0 = (optional) num_pixels, hier unbenutzt
    await cg.register_component(ctrl, config)

    # Für jeden YAML-Channel einen LightOutput + LightState registrieren
    for ch in config[CONF_CHANNELS]:
        pixel = ch[CONF_PIXEL]
        color = COLOR_MAP[ch[CONF_COLOR]]

        # C++-LightOutput mit eigener Output-ID
        out_var = cg.new_Pvariable(ch[CONF_OUTPUT_ID], ChannelLight, ctrl, pixel, color)

        # Name: entweder aus YAML oder automatisch
        name = ch.get(CONF_NAME)
        if name is None:
            name = f"Pixel {pixel+1} - {'RGB'[color]}"

        # LightState registrieren (verwendet ch[CONF_ID] als LightState-ID)
        await light.register_light(out_var, {
            CONF_ID: ch[CONF_ID],
            CONF_NAME: name,
            CONF_DISABLED_BY_DEFAULT: False,
            CONF_RESTORE_MODE: LightRestoreMode.RESTORE_DEFAULT_OFF,  # <<< neu
        })