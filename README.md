## simple-messenger
Simple messenger server and client that can be executed on linux shell. 

### Usage
Run a server, then run a client.

$ make<br/>
$ ./server ip_address:port<br/>
$ ./client ip_address:port<br/>

ex) $ ./server xxx.xxx.xxx.xxx:20741


Check pdf file for a brief description of commands. 

### How to run a server on WSL
You may want to run a server on WSL (Windows Subsystems for Linux).<br/>
<br/>

After a series of action I searched on the internet, server worked fine:
* Find local_ip_address_in_WSL by running ifconfig on WSL.<br/>
* Run a PowerShell script to forward the port from Windows to WSL local ip address (following https://sungyong.medium.com/wsl2-port-forwarding-2f984a26c1fd).<br/>
* Make an inbound rule for the port in Windows Defender Firewall with Advanced Security (following step 4 in https://www.williamjbowman.com/blog/2020/04/25/running-a-public-server-from-wsl-2/).<br/>
* Check whether there's another firewall program and open the port in that program.
* $ ./server local_ip_address_in_WSL:port<br/>
