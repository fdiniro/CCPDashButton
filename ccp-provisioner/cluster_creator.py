import requests
import urllib3
import json
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
except:
    print(
        '''
        Usage:
        python3 cluster_creator.py ccp_hostname ccp_username ccp_password tenant_cluster_name cluster_subnet cluster_skeleton_json_file cni
        
        Example:
        python3 cluster_creator.py ccp.rmlab.local admin password dashcluster1 192.168.165.0/24 creation_payload.json aci
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

# Create CCP Cluster function


def create_cluster():
    session = authenticate(ccp_host, username, password)
    request = session.post('https://{}/2/clusters'.format(ccp_host), verify=False,
                           headers=head_appjson, json=payload)
    return request.json()


# Create cluster payload
payload = create_cluster_payload(
    cluster_name, description, cidr, cluster_skel_json, cni)

# Create cluster
try:
    print('Creating cluster {} on {}... please standby.'.format(
        cluster_name, ccp_host))
    response = create_cluster()
    if cni == 'calico':
        print('Master IP: {}'.format(
            response['master_vip']), sep=' ', end='\n', file=sys.stdout, flush=True)
    elif cni == 'aci':
        for node in response['nodes']:
            if node['is_master'] == True:
                print('Master IP: {}'.format(
                    node['public_ip']), sep=' ', end='\n', file=sys.stdout, flush=True)
    else:
        sys.exit(1)

except:
    print('Unable to create cluster {} on {}'.format(cluster_name, ccp_host))
    sys.exit(1)
