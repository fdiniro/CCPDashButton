/*
 * ***************************************************************
 *
 *  CCPDashButton
 *
 *  2019 Fabio Di Niro
 *
 * ***************************************************************
 * This file is part of CCPDashButton.

    CCPDashButton is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Nome-Programma is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Nome-Programma.  If not, see <http://www.gnu.org/licenses/>.

 */


/*
 * Configuration, modify accordingly to your environment
 */

// Replace with your SSID and Password
const char* ssid     = "MySSID";
const char* password = "MyPassword";

// Replace with your preferred Mosquitto server
const char* mqtt_server = "1.2.3.4";
const int mqtt_port = 1883;
const char* mqtt_user = "mqttuser";
const char* mqtt_password = "mqttpassword";
const char* mqtt_topic = "DashButton";

// Replace with your preferred NTP server and time offset
// GMT +1 = 3600
// GMT +8 = 28800
// GMT -1 = -3600
// GMT 0 = 0
const char* myNTPserver = "pool.ntp.org";
const long timeOffset = 7200;

// Replace with your preferred name, avoid spaces and remember to authorize the dash button in the handler and the handler here
const char* dashButtonID = "Dash01";
const char* handlerID = "MyHandler";

// Specify the average cluster provisiomning time in your infrastructure: the progress bar will reach 99% in that time and then
// 100% when it really gets the cluster created confirmation
const int averageProvisioningTime = 160;
/*
 * Configuration end
 */
