import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import display, i2c
from esphome.const import CONF_ID, CONF_LAMBDA

DEPENDENCIES = ['i2c']

CONF_ANIMATION = "animation"
CONF_COLON_BLINK = "colon_blink"

duplex_display_ns = cg.esphome_ns.namespace('duplex_display')
DuplexDisplay = duplex_display_ns.class_('DuplexDisplay', cg.PollingComponent, i2c.I2CDevice)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(i2c.I2CDevice)
}).extend(i2c.i2c_device_schema(None))

CONFIG_SCHEMA = display.BASIC_DISPLAY_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(DuplexDisplay),
    cv.Optional(CONF_ANIMATION, default=False): cv.boolean,
    cv.Optional(CONF_COLON_BLINK, default=False): cv.boolean,
}).extend(cv.polling_component_schema('1s')).extend(i2c.i2c_device_schema(0x09))

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    yield display.register_display(var, config)
    yield i2c.register_i2c_device(var, config)

    if CONF_LAMBDA in config:
        lambda_ = yield cg.process_lambda(config[CONF_LAMBDA],
                                          [(DuplexDisplay.operator('ref'), 'it')],
                                          return_type=cg.void)
        cg.add(var.set_writer(lambda_))

    if config[CONF_ANIMATION]:
        cg.add(var.set_animation(True))

    if config[CONF_COLON_BLINK]:
        cg.add(var.set_colon_blink(True))