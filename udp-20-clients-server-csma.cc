// Network topology
//
//       n0    n1   n2   n3       n21
//       |     |    |    |        |
//       ================= ... ===
//              LAN
//
// - Servidor em n0, clientes em n1 ... n21
// - Todos os clientes enviam mensagens simultaneamente ao servidor (comecar com 2 clientes)

#include <fstream>
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"

#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Udp20ClientsServerCSMA");

int 
main (int argc, char *argv[]){
    uint16_t numClients = 3;
    Address serverAddress;

    // --- LOGGING --- //
    LogComponentEnable ("Udp20ClientsServerCSMA", LOG_LEVEL_ALL);
    LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

    // --- CRIACAO DOS NODES --- //    
    NS_LOG_INFO ("Create nodes.");
    NodeContainer serverNode;
    NodeContainer clientNodes;
    serverNode.Create(1);
    clientNodes.Create(numClients);
    NodeContainer allNodes(serverNode, clientNodes);

    // --- INTERNET --- // 
    NS_LOG_INFO("Install internet protocols stack.");
    InternetStackHelper internet;
    internet.Install (allNodes);

    // --- SET E INSTALL CANAL --- //
    NS_LOG_INFO("Set channel attributes.");
    CsmaHelper csma;
    csma.SetChannelAttribute ("DataRate", DataRateValue (DataRate (5000000)));
    csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));
    csma.SetDeviceAttribute ("Mtu", UintegerValue (1400));

    NS_LOG_INFO("Install channel.");
    NetDeviceContainer csmaDevices = csma.Install (allNodes);

    // --- SET E INSTALL IP ADRESSES --- //
    Ipv4AddressHelper address;
    address.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interface = address.Assign (csmaDevices);
    serverAddress = Address (interface.GetAddress (0)); // salva endereco do servidor

    // --- APLICACOES --- //
    uint16_t serverPort = 9;
    NS_LOG_INFO ("Create UdpServerEcho application on node 1.");
    UdpEchoServerHelper server (serverPort);
    ApplicationContainer serverApp = server.Install (serverNode.Get(0));

    NS_LOG_INFO ("Create UdpClientEcho applications.");
    uint32_t packetSize = 1024;
    uint32_t maxPacketCount = 1;
    Time interPacketInterval = Seconds (1.);

    //Uma aplicacao por cliente
    std::vector<ApplicationContainer> clientApp(numClients);
    for(uint32_t i=0; i<clientApp.size (); ++i){
        UdpEchoClientHelper clientHelper(serverAddress, serverPort); 
        clientHelper.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
        clientHelper.SetAttribute ("Interval", TimeValue (interPacketInterval));
        clientHelper.SetAttribute ("PacketSize", UintegerValue (packetSize));
        clientApp[i] = clientHelper.Install(clientNodes.Get(i)); //instala app no cliente i
        clientApp[i].Start(Seconds(2.0));
        clientApp[i].Stop(Seconds(10.0));
    }

    // --- NETANIM --- //
    NS_LOG_INFO("Set animation.");
    AnimationInterface anim ("udp-20-clients-server-csma-anim.xml");

    anim.SetConstantPosition(serverNode.Get(0), 300, 300); //server = node 0
    uint32_t x = 0, y = 0;
    for(uint32_t i=0; i<numClients; ++i){
        x = 100 + 40*i;
        y = (i <= 9) ? (400) : (200);
        anim.SetConstantPosition(clientNodes.Get(i), x, y); 
    }

    // --- EXECUCAO --- //
    NS_LOG_INFO("Run simulation.");
    Simulator::Run ();
    Simulator::Destroy ();
    NS_LOG_INFO ("Done.");

    return 0;
}