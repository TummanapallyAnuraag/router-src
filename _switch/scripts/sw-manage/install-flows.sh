#!/bin/bash

sudo ovs-ofctl add-flow sw0 hard_timeout=0,dl_type=0x0800,nw_dst=192.168.1.0/24,actions=mod_dl_src:78:31:1b:71:81:b8,mod_dl_dst:ff:ff:ff:ff:ff:ff,output:1
sudo ovs-ofctl add-flow sw0 hard_timeout=0,dl_type=0x0800,nw_dst=192.168.3.0/24,actions=mod_dl_src:78:31:1b:71:81:bd,mod_dl_dst:ff:ff:ff:ff:ff:ff,output:2
