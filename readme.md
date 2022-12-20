pqc-vpn
===

## Build

### clone
git clone https://gitlab.com/gilgil/g.git  
git clone https://gitlab.com/gilgil/pqc-vpn.git or git clone https://github.com/pqcy/pqc-vpn.git

### Buid G library
cd g  
cd lib  
make libg  
cd ../..  

### Buid pqc-vpn library and apps
cd pqc-vpn  
cd lib  
qmake  
make -j$(NPROC)  
cd ../app  
qmake  
make -j$(NPROC)  
cd ../..  

## Run

### Server
sudo ./vpnserver-test 12345 crt/rootCA.pem eth0  

### Client
sudo ./start.sh  
sudo ./vpnclient-test dum0 00:00:00:11:11:11 wlan0 127.0.0.1 12345  

이더넷 주소는 임의로 주세요   

종료시   
sudo pkill vpnclient-test
sudo ./stop.sh  
