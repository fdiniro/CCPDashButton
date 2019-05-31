# Introduction
CCPDashButton is a simple project to demonstrate the capabilities of [Cisco Container Platform](https://www.cisco.com/c/en/us/products/cloud-systems-management/container-platform/index.html) APIs.
Cisco Container Platform is a solution capable of provision new, production grade, secure Kubernetes clusters both on-prem or in the cloud with a few clicks.
CCPDashButton leverages those capabilities through CCP APIs to showcase how a new Kubernetes cluster may be created with just one click... like ordering soap for your washing machine.
CCPDashButton has been designed to work with mobile data connectivity, allowing the dash button to connect to your smartphone WiFi. It can also overcome communication constraints of Cisco Container Platform installations behind a proxy (socks5 is required).

# Components
A working CCPDashButton requires some components, shown in the following picture:
 ![alt text](https://github.com/fdiniro/CCPDashButton/blob/master/CCPDashButton_architecture.png "CCPDashButton architecture")<br>
-	Cisco Container Platform installation
-	MQTT service
-	WiFi connectivity 
-	The dash button itself
-	Linux box to run the scripts on

# How it works
The way it works is pretty easy.
The dash button and the handler exchange some messages with the following syntax:

TimeStamp (HH:mm:ss) 		SenderID	 	Verb		Argument

The messages that the dash button may send are:
-	Timestamp	DashID		GetStatus
-	Timestamp	DashID		ProvisionCluster	ClusterName
The message that the MQTTHandler may send in response to the above ones are:
-	Timestamp	HandlerID	StatusUnavailable
-	Timestamp	HandlerID  	StatusAvailable
-	Timestamp	HandlerID	StatusProvisioning	ClusterName
-	Timestamp	HandlerID	StatusProvisioned	MasterIPAddress

The dash button sketch has just the setup() routine, then it sleeps to save energy. There is no loop() routine.
The ESP32 is keept in deep-sleeping and as soon as the enclosure press the “Reset” button so thtat the ESP32 will be woken up and the setup() routine will be executed.
The button will perform the following actions upon button pressing:
-	Connect to the WiFi, if unsuccessful show the error and go back to sleep
-	Connect to the MQTT service, if unsuccessful show the error and go back to sleep
-	Query the status of the handler, if unavailable show the error and go back to sleep
-	If the handler status is “available” then it request a cluster provisioning supplying its name derived from the timestamp and the button ID
-	Query the handler to have confirmation of provisioning, if it gets a “StatusProvisioning” answer it show a progress bar and the indication of the cluster name as it was received from the handle sgkr. If by mistake you press again the button while the Handler is provisioning, the GetStatus request wiil be follogwd bya “StatusProvisioning” and the button wil just follow the provisioning with the progress bar
-	When the handler sends the “StatusProvisioned” message, the dash button will show the argument of this message, that is the IP address of the management for 30 seconds, and then fall again to deep sleep

# Cisco Container Platform 
The communication with CCP thought its API is currently done invoking the “cluster-creator.py” Python script from “DashButton-Backend” by Riccardo Tortorici, that has been merged into this project under the ccp-provisioner folder. At the current date that script supports both Calico and ACI CNI networking flavors of CCP, allowing you to deploy K8s cluster on any kind of network, or with an automatic integration with Cisco ACI that allows you per-cluster/namespace/deployment/pod network visibility and segmentation and wire-rate HW offloaded K8s load-balanced services. 
Often, the control plane of solutions installed in lab or datacenter environments are behind firewalls and proxies, so we will use a public MQTT service to connect to, and to bridge the communication between the dash button and the CCP install. If you plan to use the dash button in the same network where the CCP has been installed you may get rid of the MQTT, but you should perform a major rewriting of the CCPDashButton code.

# MQTT Service
The MQTT service is used to bridge the dash button with the CCP install when a proxy is sitting in between. Thre are some MQTT services available on the internet, some of them are free at least for the lower consumption tier. You may opt also to host the MQTT service on one of your devices.
For my dash button I opted for a Raspberry Pi running in my home, where I forwarded the required TCP ports to receive MQTT communications.
To setup a Raspberry Pi for such job you can follow these short instructions:
-	Download the most recent Raspbian image and write it to an SD card
-	Boot your Raspberry Pi and change your network configuration to have static IP addresses:
o	Edit /etc/dhcdcp.conf
-	Install MQTT apt-get install mqtt
-	Set mqtt password
-	Edit /etc/mqtt.conf 
o	Listener 1080
-	Configure your network devices to enable the Raspberry Pi to receive incoming calls on ports: 1883 (MQTT) 8080 Socks5. You can change those ports if you need to. 

# WiFi Connectivity
This is required by the dash button to connect to the network and reach the MQTT service.
At the current state no proxy is supported nor implemented in the communications between the dash button and the MQTT server.

# The Dash Button
Although different types of microcontroller boards may be used, for this project I chose an ESP 32 with integrated WiFi and OLED display, containing the size of the whole button with a single device with all the desired features embedded. In this particular example I used a WiFi32 produced by Heltec, which includes a Tensilica LX6 dual core 240Mhz and a 128x64 monochrome OLED display. I attached also a 3.7V 370 mah lithium battery through the 1.25 mm connector on the bottom of the board.
The button is completed with a 3d printed case I designed for it, remixing the "WIFI Kit 32 Case" by snwilson58. You can find 3d models ready to print here: https://www.thingiverse.com/thing:3639502

To program the ESP32 you need to install the following libraries in your Arduino IDE:
-	NTPClient
-	PubSubClient
-	Heltec ESP32 dev-Boards (if, like me, you’re using that specific kind of board)

Once your IDE is ready you can load the file “CCPDashButton.ino” and customize the settings:

-	ssid: specify the SSID of the WiFi network you want your dash button to connect to
-	password: provide the password of the above network
-	mqtt_server: provide the IP address of the MQTT server you want to use
-	mqtt_port = the default MQTT port is 1883, you can eventually use a different port if needed
-	mqtt_user: the username to authenticate on the MQTT service
-	mqtt_password: the password for the above username
-	mqtt_topic: the topic that the dash button have to use for the messages that it exchange with the MQTTHandler. The default topic is DashButton but of course you can change it.
-	myNTPserver: your preferred NTP service, remember that timestamps are used for communications and to generate the cluster name
-	timeOffset: here you can set your timezone as a difference in seconds from Greenwich
-	dashButtonID: the unique name you want to give to your dash button. Remember to set the same name (case sensitive) also in the MQTTHandler in order to authorize the dash button. 
-	handlerID: the unique name you will give to your MQTTHandler, this way the dash button knows whose message to listen to
-	averageProvisioningTime: when provisioning a new cluster, the dash button will show a progress bar while waiting for provision completion. As CCP does not provide an indication of the current provision progress we don’t have a mean to understand the real progress of that operation. For this reason the progress bar on the dash button will advance regardless of any event and will reach 99% in the number of seconds specified in this field. So you should measure how much time it takes to provision a cluster in your environment and set a similar length in order to have a rough estimation of remaining time. As soon as the cluster is ready the dash button will complete the progress bar and show the completion informations.;


# The Linux box
The last component you will need is a Linux box to run the two scripts:
-	MQTTHandler – handles the communication with the dash button and eventually launches the cluster-creator.py script
-	cluster-creator.py (from the DashButton-Backend project from Riccardo Tortorici) trigger a new cluster provisioning supplying the right information in a json schema.
To setup the Linux box you can follow these steps:
-	Install mosquito (pub/sub)
-	Follow install guidelines for cluster-creator.py
-	Copy the MQTTHandler, make it executable and launch it
