# led-flasher

Alexa skill that flashes LEDs connected to an ESP32 via AWS IoT, triggered by voice commands.

## Support

If you find this useful, please consider buying me a coffee:

[![Donate with PayPal](https://www.paypalobjects.com/en_GB/i/btn/btn_donate_SM.gif)](https://www.paypal.com/donate?hosted_button_id=Q3BESC73EWVNN)

## Architecture Diagram
![Architecture](architecture/led-flasher%20architecture.drawio.svg)

Eventually this will all be [CDK/ASK](https://aws.amazon.com/blogs/devops/deploying-alexa-skills-with-aws-cdk/) but for now I simply created
the skill, lambda function, and IoT devices in their respective web consoles.

## PCB
![PCB](pcb/pcb.svg)

## Demo
[![LED Flasher Demo](https://img.youtube.com/vi/og0j6WclKgI/0.jpg)](https://www.youtube.com/watch?v=og0j6WclKgI)
