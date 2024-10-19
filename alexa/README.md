
Alexa console:
https://developer.amazon.com/alexa/console/ask

* New Skill
* Name: LED-Flasher
* "Custom" model
* "Provision your own" hosting
* Start from scratch
* Invocation name "led flasher"
* Create a lambda function (python, role from template "Simple microservice permissions")
* Copy function ARN
* In Alexa console, select the skill, then "Custom" -> "Endpoint" in LHS
* Copy the skill ID, then back in the lambda function add a 'Trigger', select Alexa and add the skill ID