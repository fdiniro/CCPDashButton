import ipaddress
import sys
import requests
import urllib3
import json

# Disable SSL warnings
urllib3.disable_warnings()

try:
    ccp_host = sys.argv[1]
    username = sys.argv[2]
    password = sys.argv[3]
    cni = sys.argv[4]
except:
    print(
        '''
        Usage:
        python3 sub_in_use.py ccp_hostname ccp_username ccp_password cni
        
        Example:
        python3 sub_in_use.py ccp.rmlab.local admin password aci
        '''
    )
    sys.exit(1)

# CCP Headers
head_appjson = {'Content-Type': 'application/json'}

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

def fetch_clusters(ccp_host, username, password):
        try:
            session = authenticate(ccp_host, username, password)
            req = session.get(
                'https://{}:443/2/clusters'.format(ccp_host), verify=False, headers=head_appjson)
            response = req.json()

            # List of currently used subnets
            assigned_node_subnets = []
            if cni == 'aci':
                [assigned_node_subnets.append(ipaddress.ip_network(json.loads(
                    cluster['network_plugin']['details'])['node_vlan_cidr'])) for cluster in response]
            else:
                [assigned_node_subnets.append(ipaddress.ip_network(json.loads(
                    cluster['network_plugin']['details'])['pod_cidr'])) for cluster in response]
            return assigned_node_subnets
  
        except:
            return {
                "creation": "failed",
                "reason": "Failed to fetch running cluster list"
            }

subnets_in_use = fetch_clusters(ccp_host, username, password)
for subnet in subnets_in_use:
    print(subnet)