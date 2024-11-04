# -*- coding: utf-8 -*-

# This sample demonstrates handling intents from an Alexa skill using the Alexa Skills Kit SDK for Python.
# Please visit https://alexa.design/cookbook for additional examples on implementing slots, dialog management,
# session persistence, api calls, and more.
# This sample is built using the handler classes approach in skill builder.
import logging
import gettext
import json
import os
import boto3
from jose import jwk, jwt
from jose.utils import base64url_decode
from enum import Enum

from ask_sdk_core.skill_builder import SkillBuilder
from ask_sdk_core.dispatch_components import (
    AbstractRequestHandler, AbstractRequestInterceptor, AbstractExceptionHandler)
import ask_sdk_core.utils as ask_utils
from ask_sdk_core.handler_input import HandlerInput
from ask_sdk_model import Response
from alexa import data

logger = logging.getLogger(__name__)
if os.environ.get('LOG_LEVEL'):
    logger.setLevel(logging.getLevelName(os.environ.get('LOG_LEVEL')))
else:
    logger.setLevel(logging.INFO)

FLASH = "FLASH"
ON = "ON"
OFF = "OFF"


class Leds(str, Enum):
    RED = "RED"
    GREEN = "GREEN"
    BLUE = "BLUE"
    ALL = "ALL"
    NOTFOUND = "NOTFOUND"
    @staticmethod
    def fromString(label):
        label = label.upper()
        if label in iter(Leds):
            return Leds[label].value
        return Leds.ALL.value

class LaunchRequestHandler(AbstractRequestHandler):
    """Handler for Skill Launch."""

    def can_handle(self, handler_input):
        # type: (HandlerInput) -> bool
        logger.debug("Checking can_handle LaunchRequestHandler")
        return ask_utils.is_request_type("LaunchRequest")(handler_input)

    def handle(self, handler_input):
        # type: (HandlerInput) -> Response
        _ = handler_input.attributes_manager.request_attributes["_"]
        logger.debug("In Launch Request")

        token = ask_utils.get_account_linking_access_token(handler_input)

        # TODO: Verify the token with Cognito
        # TODO: See https://github.com/aws-samples/amazon-cognito-api-gateway/blob/main/custom-auth/lambda.py

        logger.debug("Token: " + token)
        # logger.debug(jwt.decode(token))
        message = str(token).rsplit('.')[1]
        logger.debug(message)
        logger.debug("Message ^^")
        decodedMessage = base64url_decode(message.encode()).decode('utf-8')
        logger.debug(decodedMessage)
        logger.debug("Decoded ^^")
        payload = json.loads(decodedMessage)
        username = payload["username"]
        logger.debug(username)
        logger.debug("Username ^^")

        speak_output = _(data.WELCOME_MESSAGE.format(username))

        return (
            handler_input.response_builder
            .speak(speak_output)
            .ask(_(data.HELP_MSG))
            .response
        )


class TestIntentHandler(AbstractRequestHandler):
    """Handler for Test Intent."""

    def can_handle(self, handler_input):
        # type: (HandlerInput) -> bool
        logger.debug("Checking can_handle TestIntentHandler")
        logger.debug(handler_input)
        logger.debug(ask_utils.get_request_type(handler_input))
        return ask_utils.is_intent_name("TestIntent")(handler_input)

    def handle(self, handler_input):
        # type: (HandlerInput) -> Response
        logger.debug("In TestIntentHandler")
        # logger.debug(handler_input)
        # slot = ask_utils.request_util.get_slot(handler_input, "colour")
        # logger.debug(slot)
        # logger.debug("XXXXXXXXXXXXX ^ handler_input")

        _ = handler_input.attributes_manager.request_attributes["_"]
        speak_output = _(data.TEST_MSG)

        return (
            handler_input.response_builder
            .speak(speak_output)
            # .ask("add a reprompt if you want to keep the session open for the user to respond")
            .response
        )


class OnIntentHandler(AbstractRequestHandler):
    """Handler for On Intent."""

    def can_handle(self, handler_input):
        # type: (HandlerInput) -> bool
        logger.debug("Checking can_handle OnIntent")
        logger.debug(handler_input)
        logger.debug(ask_utils.get_request_type(handler_input))
        return ask_utils.is_intent_name("OnIntent")(handler_input)

    def handle(self, handler_input):
        # type: (HandlerInput) -> Response
        logger.debug("In OnIntentHandler")
        slot_values = ask_utils.get_slot_value_v2(handler_input, "colour")
        colour = slot_values.value
        logger.info(("Switching on {}").format(colour))
        led = Leds.fromString(colour)
        _ = handler_input.attributes_manager.request_attributes["_"]
        if led == Leds.NOTFOUND.value:
            speak_output = _(data.UNKNOWN_COLOUR_MESSAGE.format(colour))
        else:
            speak_output = _(data.ON_MSG).format(colour)
            postMessage(ON, led)

        return (
            handler_input.response_builder
            .speak(speak_output)
            .ask(_(data.REPROMPT_MSG))
            .response
        )


class OffIntentHandler(AbstractRequestHandler):
    """Handler for On Intent."""

    def can_handle(self, handler_input):
        # type: (HandlerInput) -> bool
        logger.debug("Checking can_handle OffIntent")
        logger.debug(handler_input)
        logger.debug(ask_utils.get_request_type(handler_input))

        return ask_utils.is_intent_name("OffIntent")(handler_input)

    def handle(self, handler_input):
        # type: (HandlerInput) -> Response
        logger.debug("In OffIntentHandler")
        slot_values = ask_utils.get_slot_value_v2(handler_input, "colour")
        colour = slot_values.value
        logger.info(("Switching off {}").format(colour))

        led = Leds.fromString(colour)
        _ = handler_input.attributes_manager.request_attributes["_"]
        if led == Leds.NOTFOUND.value:
            speak_output = _(data.UNKNOWN_COLOUR_MESSAGE.format(colour))
        else:
            speak_output = _(data.OFF_MSG).format(colour)
            postMessage(OFF, led)

        return (
            handler_input.response_builder
            .speak(speak_output)
            .ask(_(data.REPROMPT_MSG))
            .response
        )


class FlashIntentHandler(AbstractRequestHandler):
    """Handler for Flash Intent."""

    def can_handle(self, handler_input):
        # type: (HandlerInput) -> bool
        logger.debug("Checking can_handle FlashIntent")
        logger.debug(handler_input)
        # logger.info(ask_utils.get_intent_name(handler_input))
        logger.debug(ask_utils.get_request_type(handler_input))
        return ask_utils.is_intent_name("FlashIntent")(handler_input)

    def handle(self, handler_input):
        # type: (HandlerInput) -> Response
        logger.debug("In FlashIntentHandler")
        logger.debug("user:" + handler_input.request_envelope.session.user.user_id)
        slot_values = ask_utils.get_slot_value_v2(handler_input, "colour")
        colour = slot_values.value
        logger.info(("Flashing {}").format(colour))
        logger.debug(colour)

        token = ask_utils.get_account_linking_access_token(handler_input)

        # TODO: Verify the token with Cognito
        # TODO: See https://github.com/aws-samples/amazon-cognito-api-gateway/blob/main/custom-auth/lambda.py

        logger.debug("Token: " + token)
        # logger.debug(jwt.decode(token))
        message = str(token).rsplit('.')[1]
        logger.debug(message)
        logger.debug("Message ^^")
        decodedMessage = base64url_decode(message.encode()).decode('utf-8')
        logger.debug(decodedMessage)
        logger.debug("Decoded ^^")
        payload = json.loads(decodedMessage)
        logger.debug(payload["username"])
        logger.debug("Username ^^")

        led = Leds.fromString(colour)
        _ = handler_input.attributes_manager.request_attributes["_"]
        if led == Leds.NOTFOUND.value:
            speak_output = _(data.UNKNOWN_COLOUR_MESSAGE.format(colour))
        else:
            speak_output = _(data.FLASH_MSG).format(colour)
            postMessage(FLASH, led)

        return (
            handler_input.response_builder
            .speak(speak_output)
            .ask(_(data.REPROMPT_MSG))
            .response
        )


class HelpIntentHandler(AbstractRequestHandler):
    """Handler for Help Intent."""

    def can_handle(self, handler_input):
        # type: (HandlerInput) -> bool
        logger.debug("Checking can_handle HelpIntentHandler")
        return ask_utils.is_intent_name("AMAZON.HelpIntent")(handler_input)

    def handle(self, handler_input):
        # type: (HandlerInput) -> Response
        logger.debug("In HelpIntentHandler")
        _ = handler_input.attributes_manager.request_attributes["_"]
        speak_output = _(data.HELP_MSG)

        return (
            handler_input.response_builder
            .speak(speak_output)
            .ask(speak_output)
            .response
        )


class CancelOrStopIntentHandler(AbstractRequestHandler):
    """Single handler for Cancel and Stop Intent."""

    def can_handle(self, handler_input):
        # type: (HandlerInput) -> bool
        logger.debug("Checking can_handle CancelOrStopIntentHandler")
        return (ask_utils.is_intent_name("AMAZON.CancelIntent")(handler_input) or
                ask_utils.is_intent_name("AMAZON.StopIntent")(handler_input))

    def handle(self, handler_input):
        # type: (HandlerInput) -> Response
        logger.debug("In CancelOrStopIntentHandler")
        _ = handler_input.attributes_manager.request_attributes["_"]
        speak_output = _(data.GOODBYE_MSG)

        return (
            handler_input.response_builder
            .speak(speak_output)
            .response
        )


class FallbackIntentHandler(AbstractRequestHandler):
    """Single handler for Fallback Intent."""

    def can_handle(self, handler_input):
        # type: (HandlerInput) -> bool
        logger.debug("Checking can_handle FallbackIntentHandler")
        return ask_utils.is_intent_name("AMAZON.FallbackIntent")(handler_input)

    def handle(self, handler_input):
        # type: (HandlerInput) -> Response
        logger.debug("In FallbackIntentHandler")
        speech = "Nope, that doesn't make sense"
        reprompt = "What was that?"

        return handler_input.response_builder.speak(speech).ask(reprompt).response


class SessionEndedRequestHandler(AbstractRequestHandler):
    """Handler for Session End."""

    def can_handle(self, handler_input):
        # type: (HandlerInput) -> bool
        logger.debug("Checking can_handle SessionEndedRequestHandler")
        return ask_utils.is_request_type("SessionEndedRequest")(handler_input)

    def handle(self, handler_input):
        # type: (HandlerInput) -> Response
        logger.debug("In SessionEndedRequestHandler")
        # Any cleanup logic goes here.

        return handler_input.response_builder.speak("Goodbye").response


class IntentReflectorHandler(AbstractRequestHandler):
    """The intent reflector is used for interaction model testing and debugging.
    It will simply repeat the intent the user said. You can create custom handlers
    for your intents by defining them above, then also adding them to the request
    handler chain below.
    """

    def can_handle(self, handler_input):
        # type: (HandlerInput) -> bool
        logger.debug("Checking can_handle IntentReflectorHandler")
        return ask_utils.is_request_type("IntentRequest")(handler_input)

    def handle(self, handler_input):
        # type: (HandlerInput) -> Response
        logger.debug("In IntentReflectorHandler")
        _ = handler_input.attributes_manager.request_attributes["_"]
        intent_name = ask_utils.get_intent_name(handler_input)
        speak_output = _(data.REFLECTOR_MSG).format(intent_name)

        return (
            handler_input.response_builder
            .speak(speak_output)
            # .ask("add a reprompt if you want to keep the session open for the user to respond")
            .response
        )


class CatchAllExceptionHandler(AbstractExceptionHandler):
    """Generic error handling to capture any syntax or routing errors. If you receive an error
    stating the request handler chain is not found, you have not implemented a handler for
    the intent being invoked or included it in the skill builder below.
    """

    def can_handle(self, handler_input, exception):
        # type: (HandlerInput, Exception) -> bool
        logger.debug("Checking can_handle CatchAllExceptionHandler")
        return True

    def handle(self, handler_input, exception):
        # type: (HandlerInput, Exception) -> Response
        logger.debug("In CatchAllExceptionHandler")
        logger.error(exception, exc_info=True)
        _ = handler_input.attributes_manager.request_attributes["_"]
        speak_output = _(data.ERROR)

        return (
            handler_input.response_builder
            .speak(speak_output)
            .ask(speak_output)
            .response
        )


class LocalizationInterceptor(AbstractRequestInterceptor):
    """
    Add function to request attributes, that can load locale specific data
    """

    def process(self, handler_input):
        logger.debug("In LocalizationInterceptor")
        locale = handler_input.request_envelope.request.locale
        i18n = gettext.translation(
            'data', localedir='locales', languages=[locale], fallback=True)
        handler_input.attributes_manager.request_attributes["_"] = i18n.gettext


def postMessage(action, led):
    client = boto3.client('iot-data', region_name='eu-west-2')
    client.publish(
        topic='esp32/sub',
        qos=0,
        payload=json.dumps(
            {
                "action": action,
                "led": led
            }
        )
    )


# The SkillBuilder object acts as the entry point for your skill, routing all request and response
# payloads to the handlers above. Make sure any new handlers or interceptors you've
# defined are included below. The order matters - they're processed top to bottom.


sb = SkillBuilder()

sb.add_request_handler(LaunchRequestHandler())
sb.add_request_handler(TestIntentHandler())
sb.add_request_handler(FlashIntentHandler())
sb.add_request_handler(OnIntentHandler())
sb.add_request_handler(OffIntentHandler())
sb.add_request_handler(HelpIntentHandler())
sb.add_request_handler(CancelOrStopIntentHandler())
sb.add_request_handler(FallbackIntentHandler())
sb.add_request_handler(SessionEndedRequestHandler())
# make sure IntentReflectorHandler is last so it doesn't override your custom intent handlers
sb.add_request_handler(IntentReflectorHandler())

sb.add_global_request_interceptor(LocalizationInterceptor())

sb.add_exception_handler(CatchAllExceptionHandler())

lambda_handler = sb.lambda_handler()
