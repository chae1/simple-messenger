## simple-messenger
Simple messenger server and client that can be executed on linux shell. 

### Usage
Run a server, then run a client.

$ make<br/>
$ ./server ip_address:port_num<br/>
$ ./client ip_address:port_num<br/>

ex) $ ./server xxx.xxx.xxx.xxx:20741


Check pdf file for a brief description of commands. 

### Issue
I have a trouble running a client to connect to the server in the following settings:
* Server runs on WSL (Windows Subsystems for Linux) on a machine in a home network.<br/>
* Client runs on WSL on a machine in a remote school network.<br/>
<br/>

Series of action I did in the server machine:
* Find local_ip_address_in_WSL by running ifconfig on WSL.<br/>
* Run a PowerShell script to forward port_num from Windows to WSL (following https://sungyong.medium.com/wsl2-port-forwarding-2f984a26c1fd)<br/>
* Make an inbound rule for port_num in Windows Defender Firewall with Advanced Security (following step 4 in https://www.williamjbowman.com/blog/2020/04/25/running-a-public-server-from-wsl-2/)<br/>
* $ ./server local_ip_address_in_WSL:port_num<br/>
<br/>

What I did in the client machine:
* $ ./client global_ip_address_of_a_server_machine:port_num<br/>

