# CN-CA2🌐
## Computer Network CA2. 📡

### Amir-Hossein Shahgholi (SID: 810199441)🎓

### Erfan-Soleymani (SID: 810199439)🎓
</br>

In this assignment we build a wireless topology containing one `client` node, one `central(master)` node and 3 mapper nodes.
Client generate random numbers between 0-25 and send these data to master nodes and then master node send these data to all 3 mapper nodes and in the next step mapper that contains proper mapping for this data decodes this data to corresponding charecter and send this data back to client.
Client connects to central node using `UDP`, client connects to mappers using `TCP` and mappers connects to client using `UDP`.
</br></br>
![Topology](images/Topology.png)

## Run
To run this sample app you have to enter command below in your terminal:
```bash
./waf --run scratch/sample
```
</br>

We set bandwidth and error-rate in code and also expect `higher` throughput with higher `bandwidth` and `lower` throughput with more `error-rate`.

### Code Explanation
First we define our classes for nodes master, client, mapper
```bash
class master : public Application
{
  master(uint16_t port, Ipv4InterfaceContainer &ip, Ipv4InterfaceContainer &mappers_ip);
  ...
};

class client : public Application
{
  client(uint16_t port, Ipv4InterfaceContainer &ip, Ipv4InterfaceContainer &my_ip);
  ...
};

class mapper : public Application
{
  mapper(uint16_t id, uint16_t port, Ipv4InterfaceContainer &ip, Ipv4InterfaceContainer &client_ip, std::map<int, std::string> mapp);
  ...
};
```

Then we create specific map for each mapper to decode.
```bash
  std::map<int, std::string> Mapper1map;
  std::map<int, std::string> Mapper2map;
  std::map<int, std::string> Mapper3map;
  Mapper1map.insert(std::make_pair(0, "a"));
  Mapper1map.insert(std::make_pair(1, "b"));
  Mapper1map.insert(std::make_pair(2, "c"));
  Mapper1map.insert(std::make_pair(3, "d"));
  Mapper1map.insert(std::make_pair(4, "e"));
  Mapper1map.insert(std::make_pair(5, "f"));
  Mapper1map.insert(std::make_pair(6, "g"));
  Mapper1map.insert(std::make_pair(7, "h"));
  Mapper1map.insert(std::make_pair(8, "i"));
  Mapper2map.insert(std::make_pair(9, "j"));
  Mapper2map.insert(std::make_pair(10, "k"));
  Mapper2map.insert(std::make_pair(11, "l"));
  Mapper2map.insert(std::make_pair(12, "m"));
  Mapper2map.insert(std::make_pair(13, "n"));
  Mapper2map.insert(std::make_pair(14, "o"));
  Mapper2map.insert(std::make_pair(15, "p"));
  Mapper2map.insert(std::make_pair(16, "q"));
  Mapper2map.insert(std::make_pair(17, "r"));
  Mapper3map.insert(std::make_pair(18, "s"));
  Mapper3map.insert(std::make_pair(19, "t"));
  Mapper3map.insert(std::make_pair(20, "u"));
  Mapper3map.insert(std::make_pair(21, "v"));
  Mapper3map.insert(std::make_pair(22, "w"));
  Mapper3map.insert(std::make_pair(23, "x"));
  Mapper3map.insert(std::make_pair(24, "y"));
  Mapper3map.insert(std::make_pair(25, "z"));
```

In next steps we create nodes and assign port to them and the install them as we learned from the sample code. We create 3 Tcp sockets to connect master node to mappers.</br>
Client start generating random numbers between 0 to 25 and then send them to master client using udp connection. Master node just receives the packets and forward these packets to mapper nodes. Each mapper node receives the packaet and find the data and try to decode this data. If mapper have decode of this data then send this decoded data to client using UDP connection and if not just ignore this data.
Sending data through UDP connection is just creating a udp socket to destination ip:port address and send data and in other hand reciver create same socket and bind to same ip:port and recive the packet over UDP socket.
#### UDP Example
```bash
      Ptr<Socket> sock = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
      InetSocketAddress sockAddr(client_ip.GetAddress(0), port);
      sock->Connect(sockAddr);
      Ptr<Packet> new_packet = new Packet();
      MyHeader m;
      m.SetData(static_cast<int>(decoded_data[0]));
      new_packet->AddHeader(m);
      sock->Send(new_packet);
```

### Example Output

*** double error = 0.000001; ***
*** string bandwidth = "10Mbps"; ***
```bash
Flow ID                 : 1 ; 10.1.3.1 -----> 10.1.3.2
Tx Packets = 100
Rx Packets = 98
Duration                : 9.90008
Last Received Packet    : 9.90008 Seconds
Throughput: 0.00226568 Mbps
---------------------------------------------------------------------------
Flow ID                 : 2 ; 10.1.3.3 -----> 10.1.3.2
Tx Packets = 2
Rx Packets = 2
Duration                : 6.0005
Last Received Packet    : 9.0005 Seconds
Throughput: 0.000142404 Mbps
---------------------------------------------------------------------------
Flow ID                 : 3 ; 10.1.3.4 -----> 10.1.3.2
Tx Packets = 2
Rx Packets = 2
Duration                : 6.00155
Last Received Packet    : 9.00155 Seconds
Throughput: 0.000142379 Mbps
---------------------------------------------------------------------------
Flow ID                 : 4 ; 10.1.3.5 -----> 10.1.3.2
Tx Packets = 2
Rx Packets = 2
Duration                : 6.00098
Last Received Packet    : 9.00098 Seconds
Throughput: 0.000142392 Mbps
```
