#include<bits/stdc++.h>
#include"connection.h"
using namespace std;

int main(){
    Client client;
    client.create_socket();
    client.relay_io();
    return 0;
}