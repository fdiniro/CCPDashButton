# CCP Quick Cluster Creator
### Requirements
- Python 3 interpreter<br>
- A sample POST payload in JSON format to create clusters on CCP (provided for ACI CNI on CCP 3.2)<br>
<br>
To generate the JSON payload, open the browser developer tools and create a cluster from the CCP UI.<br>
Once you submit the cluster, you should find a POST with a JSON payload. Copy it and put it in a file in the root directory of this repo, you will need to reference the file name whan you run the cluster creator script (instructions below)<br>

### Installation
```
pip3 install -r requirements.txt
```

### Create a template with your typical settings
#### The cluster_creator.py script will use this template and replace cluster name, description and cidr with your inputs

The cluster creator script requires a template to deploy your clusters.<br>
Access the Container Platform UI and create a cluster with your favourite settings:
![alt text](https://raw.githubusercontent.com/fdiniro/CCPDashButton/master/ccp-provisioner/screenshots/new_cluster.png "New Cluster Creation")<br>
Fill in with your settings like Datacenter, networks, number of worker nodes, memory, etc but do <b>NOT</b> click on the 'Finish' button:
![alt text](https://raw.githubusercontent.com/fdiniro/CCPDashButton/master/ccp-provisioner/screenshots/template_def.png "Fill in with your typical settings to create a template")<br>
Open the inspect tool on your browser:
![alt text](https://raw.githubusercontent.com/fdiniro/CCPDashButton/master/ccp-provisioner/screenshots/inspect_tool.png "Inspect tool")<br>
Clean up the existing data and submit the cluster creation by clicking the 'Finish' button:
![alt text](https://raw.githubusercontent.com/fdiniro/CCPDashButton/master/ccp-provisioner/screenshots/create_cluster.png "Clean up and submit cluster creation")<br>
Find and copy to the clipboard the submitted payload:
![alt text](https://raw.githubusercontent.com/fdiniro/CCPDashButton/master/ccp-provisioner/screenshots/view_source.png "Find Submitted payload")<br>
![alt text](https://raw.githubusercontent.com/fdiniro/CCPDashButton/master/ccp-provisioner/screenshots/copy.png "Select and Copy to Clipboard")<br>
Create a file in the root directory of this repository with the payload content you just copied, for instance 'creation_payload.json'<br>
You will pass this file as an argument to the cluster_creator.py script.<br> 


### cluster_creator.py Usage:<br>

```
python3 cluster_creator.py ccp_hostname ccp_username ccp_password tenant_cluster_name cluster_subnet cluster_skeleton_json_file cni
```
        
Example:<br>

```
python3 cluster_creator.py ccp.rmlab.local admin password dashcluster1 192.168.165.0/24 creation_payload.json calico
```
<br>
<b>Note:</b><br>
If you create a cluster with calico CNI, the 'cni' keyword must be 'calico'. In case you use ACI-CNI, you will specify 'aci'.<br>
If you create a cluster with calico CNI, 'cluster\_subnet' keyword is the pod\_cidr, for ACI CNI is the node\_vlan\_cidr.

