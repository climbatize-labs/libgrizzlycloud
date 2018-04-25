import boto3
from keys import *

TITLE             = "GrizzlyCloud"

def build_speechlet_response(title, output, reprompt_text, should_end_session):
    return {
        'outputSpeech': {
            'type': 'PlainText',
            'text': output
        },
        'card': {
            'type': 'Simple',
            'title': "SessionSpeechlet - " + title,
            'content': "SessionSpeechlet - " + output
        },
        'reprompt': {
            'outputSpeech': {
                'type': 'PlainText',
                'text': reprompt_text
            }
        },
        'shouldEndSession': should_end_session
    }

def build_response(session_attributes, speechlet_response):
    return {
        'version': '1.0',
        'sessionAttributes': session_attributes,
        'response': speechlet_response
    }

def post_message(client, message_body, url):
    response = client.send_message(QueueUrl = url, MessageBody= message_body)

def dfail(client, event, context, mtype):
    post_message(client, 'fail', queue_url)
    speechlet = build_speechlet_response(TITLE, mtype, "", "true")
    return build_response({}, speechlet)

def handleReply(client, url):
    response = client.receive_message(QueueUrl = url, MaxNumberOfMessages = 1)
    if not 'Messages' in response:
        speechlet = build_speechlet_response(TITLE, "No replies to handle", "", "true")
        return build_response({}, speechlet)

    messages = response['Messages']
    for msg in messages:
        receipt = msg['ReceiptHandle']
        client.delete_message(QueueUrl = url, ReceiptHandle = receipt)
        reply = msg['Body']
        speechlet = "{}"
        if(reply == "isstartedYes"):
            speechlet = build_speechlet_response(TITLE, "Grizzly cloud is running", "", "true")
        elif(reply == "isstartedNo"):
            speechlet = build_speechlet_response(TITLE, "Grizzly cloud is not running", "", "true")
        else:
            speechlet = build_speechlet_response(TITLE, "Unknown reply " + reply, "", "true")
        return build_response({}, speechlet)

def on_intent(client, request):
    intent = request['intent']['name']
    if(intent == "isstartedIntent"):
        post_message(client, 'isstarted', queue_url)
        speechlet = build_speechlet_response(TITLE, "Status check requested", "", "true")
        return build_response({}, speechlet)
    elif(intent == "repliesIntent"):
        return handleReply(client, queue_replies_url)

    speechlet = build_speechlet_response(TITLE, "Unsupported intent", "", "true")
    return build_response({}, speechlet)

def lambda_handler(event, context):
    client = boto3.client('sqs', aws_access_key_id = access_key, aws_secret_access_key = access_secret, region_name = region)
    if 'request' in event:
        if event['request']['type'] == "LaunchRequest":
            speechlet = build_speechlet_response(TITLE, "Grizzly cloud started", "", "true")
            return build_response({}, speechlet)
        elif event['request']['type'] == "IntentRequest":
            return on_intent(client, event['request'])
        elif event['request']['type'] == "SessionEndedRequest":
            speechlet = build_speechlet_response(TITLE, "Grizzly cloud stopped", "", "true")
            return build_response({}, speechlet)
