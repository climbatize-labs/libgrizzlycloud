import boto3
import psutil
from keys import *

def post_message(client, message_body, url):
    response = client.send_message(QueueUrl = url, MessageBody= message_body)
    return response

def isstarted(client):
    running = 0
    for pid in psutil.pids():
        p = psutil.Process(pid)
        if(p.name().find("grizzlycloud") != -1):
            running = 1
            break

    if(running == 1):
        post_message(client, "isstartedYes", queue_replies_url)
    else:
        post_message(client, "isstartedNo", queue_replies_url)
    print 'Isstarted: ', running

def pop_message(client, url):
    response = client.receive_message(QueueUrl = url, MaxNumberOfMessages = 10)
    if not 'Messages' in response:
        print 'No messages to handle'
        return

    messages = response['Messages']
    for msg in messages:
        if(msg['Body'] == "isstarted"):
            isstarted(client)
        receipt = msg['ReceiptHandle']
        client.delete_message(QueueUrl = url, ReceiptHandle = receipt)

client = boto3.client('sqs', aws_access_key_id = access_key, aws_secret_access_key = access_secret, region_name = region)
pop_message(client, queue_url)
