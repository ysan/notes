#!/bin/sh

#sudo mount -t cifs -o sec=ntlm //192.168.0.2/disk /mnt/Landisk
#sudo mount -t cifs -o sec=ntlm -o username=admin,password= //192.168.50.8/disk /mnt/Landisk
sudo mount -t cifs -o sec=ntlm,vers=1.0 -o username=admin,password= //192.168.50.7/disk /mnt/Landisk
