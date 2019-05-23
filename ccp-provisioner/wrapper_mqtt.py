import requests
import urllib3
import json
import paho.mqtt.client as paho
import time
import sys


# Set arguments. Print usage if not enough arguments have been provided
try:
    ccp_host = sys.argv[1]
    username = sys.argv[2]
    password = sys.argv[3]
    cluster_name = sys.argv[4]
    cidr = sys.argv[5]
    cluster_skel_json = sys.argv[6]
    cni = sys.argv[7]
    # MQTT Settings
    broker = sys.argv[8]
    topic = sys.argv[9]
    trigger_message = sys.argv[10]

except:
    print(
        '''
        Usage:
        python3 wrapper_mqtt.py ccp_hostname ccp_username ccp_password tenant_cluster_name cluster_subnet cluster_skeleton_json_file cni mqtt_ip mqtt_topic trigger_message
        
        Example:
        python3 wrapper_mqtt.py ccp.rmlab.local admin password dashcluster1 192.168.165.0/24 creation_payload.json aci 127.0.0.1 ccp/dashbutton create
        '''
    )
    sys.exit(1)

# Set default tenant cluster description
description = 'Created by RMLAB Dash Button'

# Disable SSL warnings
urllib3.disable_warnings()

# CCP Headers
head_appjson = {'Content-Type': 'application/json'}

# Cluster Creation Payload Function
def create_cluster_payload(cluster_name, description, cidr, cluster_skel_json, cni):
    with open(cluster_skel_json) as skel:
        payload = json.load(skel)
    payload['name'] = cluster_name
    payload['description'] = description + ' - ' + cidr
    if cni == 'aci':
        payload['network_plugin']['details'] = '{"node_vlan_cidr":"' + cidr + '"}'
    else:
        payload['network_plugin']['details'] = '{"pod_cidr":"' + cidr + '"}'
    return payload

# Cisco Container Platform Authentication
def authenticate(ccp_host, username, password):
    try:
        # CCP login API requests headers and payload
        head_login = {'Content-Type': 'application/x-www-form-urlencoded'}
        auth_payload = "username={}&password={}".format(username, password)
        s = requests.session()
        s.post(
            'https://{}/2/system/login'.format(ccp_host), verify=False, headers=head_login, data=auth_payload)
        return s
    except:
        return {
            "Error": "Unable to authenticate to {}".format(ccp_host)
        }


# MQTT Message empty by default
msg = ''

# Callback for MQTT
def on_message(client, userdata, message):
    time.sleep(1)
    global msg
    msg = str(message.payload.decode("utf-8"))


# MQTT - Init client
client = paho.Client("ccp-wrapper")
client.on_message = on_message
print("Connecting to broker ", broker)
client.connect(broker)
client.loop_start()
print("Subscribing topic {}".format(topic))
client.subscribe("{}".format(topic))

# Create CCP Cluster
def create_cluster():
    session = authenticate(ccp_host, username, password)
    request = session.post('https://{}/2/clusters'.format(ccp_host), verify=False,
                           headers=head_appjson, json=payload)
    return request.json()


# Create cluster payload
payload = create_cluster_payload(
    cluster_name, description, cidr, cluster_skel_json, cni)

# Wait for cluster creation requests from MQTT
while True:
    if trigger_message in msg:
        print('Request received! Creating cluster...')
        response = create_cluster()
        print(response)
        # Clear msg back after cluster creation
        msg = ''