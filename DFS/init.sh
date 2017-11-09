#!/bin/bash

#Preparing documents

echo -e "Preparing documents\n"

mkdir S1 S2 S3 C1 C2 S1/storage S2/storage S3/storage C1/storage C2/storage
cp server.c S1/server.c 
cp server.c S2/server.c 
cp server.c S3/server.c
cp client.c C1/client.c 
cp client.c C2/client.c
cp client_cron.c C1/client_cron.c 
cp client_cron.c C2/client_cron.c
touch C1/config C2/config C1/accessor C2/accessor C1/Id C2/Id S1/Id S2/Id S3/Id C1/cron_instr C2/cron_instr
echo 1 > S1/Id
echo 2 > S2/Id
echo 3 > S3/Id
echo 4 > C1/Id
echo 5 > C2/Id
cat cron_instr C1/cron_instr
cat cron_instr C2/cron_instr

#Compilation

echo -e "Compiling Documents.\n"

gcc -o S1/server S1/server.c maekawa_svc.c maekawa_xdr.c -lrpcsvc -lnsl
gcc -o S2/server S2/server.c maekawa_svc.c maekawa_xdr.c -lrpcsvc -lnsl
gcc -o S3/server S3/server.c maekawa_svc.c maekawa_xdr.c -lrpcsvc -lnsl

gcc -o C1/client C1/client.c maekawa_clnt.c maekawa_xdr.c -lnsl
gcc -o C2/client C2/client.c maekawa_clnt.c maekawa_xdr.c -lnsl

gcc -o C1/client_cron C1/client_cron.c maekawa_clnt.c maekawa_xdr.c -lnsl
gcc -o C2/client_cron C2/client_cron.c maekawa_clnt.c maekawa_xdr.c -lnsl

echo -e "Compilation terminated.\n"

#Display MANUAL File to the USER

cat MANUAL

