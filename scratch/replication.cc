/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/multichannel-probe-module.h"
#include "ns3/testmodule-module.h"
//#include "ns3/ipv4-conga-routing-helper.h"
#include "ns3/ipv4-doppelganger-routing-helper.h"
#include "ns3/ipv4-doppelganger-routing.h"

#include <string>
#include <iostream>
#include <fstream>
#include <stdint.h>

#define CROSS_CORE 0
#define DISTRIBUTION_SIZE 1024

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("VarClients");

const int K = 4;
const int PODS = K;
const int EDGE = (K/2);
const int EDGES = PODS * EDGE;
const int AGG = (K/2);
const int AGGS = PODS * AGG;
const int CORE = (K/2)*(K/2);
const int NODE = K/2 ;
const int NODES = PODS * EDGE * NODE ;



RpcClient::selectionStrategy rpcSelectionStrategy = RpcClient::noReplica;
const char *RpcSelectionStrategyString = "RpcSelectionStrategy";
Ipv4DoppelgangerRouting::LoadBalencingStrategy networkSelectionStrategy = Ipv4DoppelgangerRouting::none;
const char *NetworkSelectionStrategyString = "NetworkSelectionStrategy";



bool debug = false;

std::string ManifestName = "manifest.config";
std::string ProbeName = "default.csv";

const char* WorkingDirectoryString = "WorkingDirectory";
std::string WorkingDirectory = "/tmp/replica-trash/";


//Transmission Iterval Distribution Patterns
const char *TransmissionDistributionUniformString = "TransmissionDistributionUniform";
bool TransmissionDistributionUniform = false;
const char *TransmissionDistributionUniformMinString = "TransmissionDistributionUniformMin";
uint32_t TransmissionDistributionUniformMin = 0;
const char *TransmissionDistributionUniformMaxString = "TransmissionDistributionUniformMax";
uint32_t TransmissionDistributionUniformMax = 0;

const char *TransmissionDistributionNormalString = "TransmissionDistributionNormal";
bool TransmissionDistributionNormal = false;
const char *TransmissionDistributionNormalMeanString = "TransmissionDistributionNormalMean";
double TransmissionDistributionNormalMean = 0.0;
const char *TransmissionDistributionNormalStdString = "TransmissionDistributionNormalStd";
double TransmissionDistributionNormalStd = 0.0;

const char *TransmissionDistributionExponentialString = "TransmissionDistributionExponential";
bool TransmissionDistributionExponential = false;
const char *TransmissionDistributionExponentialLambdaString = "TransmissionDistributionExponentialLambda";
double TransmissionDistributionExponentialLambda = 0.0;
const char *TransmissionDistributionExponentialMultiplierString = "TransmissionDistributionExponentialMultiplier";
double TransmissionDistributionExponentialMultiplier = 0.0;
const char *TransmissionDistributionExponentialMinString = "TransmissionDistributionExponentialMin";
double TransmissionDistributionExponentialMin = 0.0;

//Packet Size Distribution Parameters
const char *PacketSizeDistributionUniformString = "PacketSizeDistributionUniform";
bool PacketSizeDistributionUniform = false;
const char *PacketSizeDistributionUniformMinString = "PacketSizeDistributionUniformMin";
uint32_t PacketSizeDistributionUniformMin = 0;
const char *PacketSizeDistributionUniformMaxString = "PacketSizeDistributionUniformMax";
uint32_t PacketSizeDistributionUniformMax = 0;

const char *PacketSizeDistributionNormalString = "PacketSizeDistributionNormal";
bool PacketSizeDistributionNormal = false;
const char *PacketSizeDistributionNormalMeanString = "PacketSizeDistributionNormalMean";
double PacketSizeDistributionNormalMean = 0.0;
const char *PacketSizeDistributionNormalStdString = "PacketSizeDistributionNormalStd";
double PacketSizeDistributionNormalStd = 0.0;

const char *PacketSizeDistributionExponentialString = "PacketSizeDistributionExponential";
bool PacketSizeDistributionExponential = false;
const char *PacketSizeDistributionExponentialLambdaString = "PacketSizeDistributionExponentialLambda";
double PacketSizeDistributionExponentialLambda = 0.0;
const char *PacketSizeDistributionExponentialMultiplierString = "PacketSizeDistributionExponentialMultiplier";
double PacketSizeDistributionExponentialMultiplier = 0.0;
const char *PacketSizeDistributionExponentialMinString = "PacketSizeDistributionExponentialMin";
double PacketSizeDistributionExponentialMin = 0.0;

//Server Load Distribution Parameters
const char *ServerLoadDistributionUniformString = "ServerLoadDistributionUniform";
bool ServerLoadDistributionUniform = false;
const char *ServerLoadDistributionUniformMinString = "ServerLoadDistributionUniformMin";
uint32_t ServerLoadDistributionUniformMin = 0;
const char *ServerLoadDistributionUniformMaxString = "ServerLoadDistributionUniformMax";
uint32_t ServerLoadDistributionUniformMax = 0;

const char *ServerLoadDistributionNormalString = "ServerLoadDistributionNormal";
bool ServerLoadDistributionNormal = false;
const char *ServerLoadDistributionNormalMeanString = "ServerLoadDistributionNormalMean";
double ServerLoadDistributionNormalMean = 0.0;
const char *ServerLoadDistributionNormalStdString = "ServerLoadDistributionNormalStd";
double ServerLoadDistributionNormalStd = 0.0;

const char *ServerLoadDistributionExponentialString = "ServerLoadDistributionExponential";
bool ServerLoadDistributionExponential = false;
const char *ServerLoadDistributionExponentialLambdaString = "ServerLoadDistributionExponentialLambda";
double ServerLoadDistributionExponentialLambda = 0.0;
const char *ServerLoadDistributionExponentialMultiplierString = "ServerLoadDistributionExponentialMultiplier";
double ServerLoadDistributionExponentialMultiplier = 0.0;
const char *ServerLoadDistributionExponentialMinString = "ServerLoadDistributionExponentialMin";
double ServerLoadDistributionExponentialMin = 0.0;



const char *ManifestNameString = "ManifestName";
const char *ProbeNameString = "ProbeName";

const char *DebugString = "Debug";

const char *KString = "K";
const char *TopologyString = "Topology";
const char *Topology = "PFatTree";
const char *ParallelString = "Parallel";

#include <random>
//Distributions This should be moved to a seperate file
std::vector<uint32_t> uniform_distribution(uint32_t size, uint32_t min, uint32_t max) {
  if (min > max) {
    NS_LOG_WARN("Min is set to less than max, this has undefined behaviour. Setting both min an max to 0");
    min=0;
    max=0;
  }
  std::vector<uint32_t> uniform_sample;
  std::default_random_engine generator;
  std::uniform_int_distribution<uint32_t> distribution(min,max);

  for (uint32_t i=0; i< size;i++) {
    uniform_sample.push_back(distribution(generator));
    //printf("sample %d\n",(uniform_sample[i]));
  }
  return uniform_sample;
}

//This function returns an aproximate normal sample of integers. If the mean
//and the STD are too small the array will cluster to a small number of
//integers
std::vector<uint32_t> normal_distribution(uint32_t size, double mean, double std) {
  std::vector<uint32_t> normal_sample;
  std::default_random_engine generator;
  std::normal_distribution<double> distribution(mean,std);

  int tmp_value;
  for (uint32_t i=0; i< size;i++) {
    tmp_value = (int) distribution(generator);
    if (tmp_value < 0) {
      printf("Normal distribution should only generate positive numbers (setting value to 0 and breaking distribution). Value %d\n", tmp_value);
      tmp_value = 0;
    }
    normal_sample.push_back((uint32_t) tmp_value);
  }
  return normal_sample;
}

//The multiplier is used to shift the distribution from the range [0,1] out to
//further numbers. The value of the multiplier is also the max value
//further numbers. The min value is the min, caused by shifting all values up by the min
std::vector<uint32_t> exponential_distribution(uint32_t size, double lambda, double multiplier, double min) {
  std::vector<uint32_t> exponential_sample;
  std::default_random_engine generator;
  std::exponential_distribution<double> distribution(lambda);

  for (uint32_t i=0;i<size;i++) {
    exponential_sample.push_back(distribution(generator) * multiplier + min);
  }
  return exponential_sample;
}
//\Distributions
bool CheckUniformDistributionParameters(int min, int max) {
  //Uniform transmision conditions
    if (min == 0 || max == 0) {
      NS_LOG_WARN("Uniform distribution set but min max values left to default");
      return false;
    } else if (min > max) {
      NS_LOG_WARN("Uniform distributition min value greater than max value min: " << min <<" max: " << max <<" (check parameters)");
      return false;
    } 
  return true;
}

bool CheckNormalDistributionParameters(double mean, double std) {
    if (mean < 0) {
      NS_LOG_WARN("Select a normal mean greater than 0. Mean " << TransmissionDistributionNormalMean);
      return false;
    }
    if (std <= 0.0) {
      NS_LOG_WARN("Select a standard deviation greater than 0. Normal distributions with std <= 0 are undefined std:" << TransmissionDistributionNormalStd);
      return false;
    }
    return true;
}

bool CheckExponentialDistributionParameters(double lambda, double multiplier, double min) {
    if (lambda <= 0) {
      NS_LOG_WARN("Exponential lambda value should not be less than 0. lambda " << lambda);
      return false;
    }
    if (multiplier < 0) {
      NS_LOG_WARN("Exponential multiplier value should not be less than 0. Multiplier: " << multiplier);
      return false;
    }
    if (min < 0) {
      NS_LOG_WARN("Exponential Min value should not be less than 0. Min: " << min);
      return false;
    }
    return true;

}

void parseArgs(int argc, char* argv[]) {
  CommandLine cmd;
  //Command Line argument debugging code
  //Debug
  cmd.AddValue(DebugString, "Print all log level info statements for all clients", debug);
  //Selection Strategy
  int placeholderRpcSelectionStrategy;
  cmd.AddValue(RpcSelectionStrategyString, "RPC selection strategies - check RpcClient::selectionStratigies for valid values", placeholderRpcSelectionStrategy);

  int placeholderNetworkSelectionStrategy;
  cmd.AddValue(NetworkSelectionStrategyString, "Network selection strategy - check Ivp4Doppelganger::LoadBalencingStrategies for valid values", placeholderNetworkSelectionStrategy);

  //TransmissionDistributionUniform
  cmd.AddValue(TransmissionDistributionUniformString, "Set to true for uniform transmission distribution", TransmissionDistributionUniform);
  cmd.AddValue(TransmissionDistributionUniformMinString, "Set to the minimum value for a transmission interval (us)", TransmissionDistributionUniformMin);
  cmd.AddValue(TransmissionDistributionUniformMaxString, "Set the maximum value for the uniform transmission interval (us)", TransmissionDistributionUniformMax);

  //TransmissionDistributionNormal
  cmd.AddValue(TransmissionDistributionNormalString, "Set to true for normal transmission distribution", TransmissionDistributionNormal);
  cmd.AddValue(TransmissionDistributionNormalMeanString, "Set to the mean value for a transmission interval (us)", TransmissionDistributionNormalMean);
  cmd.AddValue(TransmissionDistributionNormalStdString, "Set the standard deviation value for the normal transmission interval (us)", TransmissionDistributionNormalStd);

  //TransmissionDistributionNormal
  cmd.AddValue(TransmissionDistributionExponentialString, "Set to true for normal transmission distribution", TransmissionDistributionExponential);
  cmd.AddValue(TransmissionDistributionExponentialLambdaString, "Lambda value for transmission distribution", TransmissionDistributionExponentialLambda);
  cmd.AddValue(TransmissionDistributionExponentialMultiplierString, "Value to multiply exponential distribution by", TransmissionDistributionExponentialMultiplier);
  cmd.AddValue(TransmissionDistributionExponentialMinString, "Value to shift exponential distribution by (post multiply)", TransmissionDistributionExponentialMin);
  
  //PacketSizeDistributionUniform
  cmd.AddValue(PacketSizeDistributionUniformString, "Set to true for uniform packet size distribution", PacketSizeDistributionUniform);
  cmd.AddValue(PacketSizeDistributionUniformMinString, "Set to the minimum value for a packet size interval (us)", PacketSizeDistributionUniformMin);
  cmd.AddValue(PacketSizeDistributionUniformMaxString, "Set the maximum value for the uniform packet size interval (us)", PacketSizeDistributionUniformMax);

  //PacketSizeDistributionNormal
  cmd.AddValue(PacketSizeDistributionNormalString, "Set to true for normal packet size distribution", PacketSizeDistributionNormal);
  cmd.AddValue(PacketSizeDistributionNormalMeanString, "Set to the mean value for a packet size interval (us)", PacketSizeDistributionNormalMean);
  cmd.AddValue(PacketSizeDistributionNormalStdString, "Set the standard deviation value for the normal packet size interval (us)", PacketSizeDistributionNormalStd);

  //PacketSizeDistributionNormal
  cmd.AddValue(PacketSizeDistributionExponentialString, "Set to true for normal packet size distribution", PacketSizeDistributionExponential);
  cmd.AddValue(PacketSizeDistributionExponentialLambdaString, "Lambda value for packet size distribution", PacketSizeDistributionExponentialLambda);
  cmd.AddValue(PacketSizeDistributionExponentialMultiplierString, "Value to multiply exponential distribution by", PacketSizeDistributionExponentialMultiplier);
  cmd.AddValue(PacketSizeDistributionExponentialMinString, "Value to shift exponential distribution by (post multiply)", PacketSizeDistributionExponentialMin);

  //ServerLoadDistributionUniform
  cmd.AddValue(ServerLoadDistributionUniformString, "Set to true for uniform server load distribution", ServerLoadDistributionUniform);
  cmd.AddValue(ServerLoadDistributionUniformMinString, "Set to the minimum value for a server load interval (us)", ServerLoadDistributionUniformMin);
  cmd.AddValue(ServerLoadDistributionUniformMaxString, "Set the maximum value for the uniform server load interval (us)", ServerLoadDistributionUniformMax);

  //ServerLoadDistributionNormal
  cmd.AddValue(ServerLoadDistributionNormalString, "Set to true for normal server load distribution", ServerLoadDistributionNormal);
  cmd.AddValue(ServerLoadDistributionNormalMeanString, "Set to the mean value for a server load interval (us)", ServerLoadDistributionNormalMean);
  cmd.AddValue(ServerLoadDistributionNormalStdString, "Set the standard deviation value for the normal server load interval (us)", ServerLoadDistributionNormalStd);

  //ServerLoadDistributionNormal
  cmd.AddValue(ServerLoadDistributionExponentialString, "Set to true for normal server load distribution", ServerLoadDistributionExponential);
  cmd.AddValue(ServerLoadDistributionExponentialLambdaString, "Lambda value for server load distribution", ServerLoadDistributionExponentialLambda);
  cmd.AddValue(ServerLoadDistributionExponentialMultiplierString, "Value to multiply exponential distribution by", ServerLoadDistributionExponentialMultiplier);
  cmd.AddValue(ServerLoadDistributionExponentialMinString, "Value to shift exponential distribution by (post multiply)", ServerLoadDistributionExponentialMin);


  //Set manifest and host name

  cmd.AddValue(WorkingDirectoryString,"The location of the current working directory", WorkingDirectory);
  cmd.AddValue(ManifestNameString, "Then name of the ouput manifest (includes all configurations)", ManifestName);
  cmd.AddValue(ProbeNameString, "Then name of the output probe CSV", ProbeName);
  cmd.Parse(argc, argv);

  rpcSelectionStrategy = (RpcClient::selectionStrategy) placeholderRpcSelectionStrategy;
  networkSelectionStrategy = (Ipv4DoppelgangerRouting::LoadBalencingStrategy) placeholderNetworkSelectionStrategy;

  //mode = DRED;
  //
  //Open a file to write out manifest
  std::string manifestFilename = ManifestName;
  std::ios_base::openmode openmode = std::ios_base::out | std::ios_base::trunc;
  //ofstream->open (manifestFilename.c_str (), openmode);
  OutputStreamWrapper StreamWrapper = OutputStreamWrapper(manifestFilename, openmode);
  //StreamWrapper->SetStream(ofstream);
  std::ostream *stream = StreamWrapper.GetStream();

  *stream << ManifestNameString << ":" << ManifestName << "\n";
  *stream << DebugString << ":" << debug << "\n";
  *stream << RpcSelectionStrategyString << ":" << rpcSelectionStrategy << "\n";
  *stream << KString << ":" << K << "\n";
  *stream << TopologyString << ":" << Topology << "\n";
  //TODO finish manifest and allow it to be an input argument
  *stream << "MANIFEST INCOMPLETE" << "\n";
}


//This function checks mutual exclusion across n_args bools. If booleans are
//not passes or n_args is incorrect the program might crash. Use carefully.
//This function returns true if exactly 1 of the boolean values is true.
bool CheckMutuallyExclusiveConditions(int n_args, ...) {
  int trueConditions = 0;
  va_list ap;
  va_start(ap, n_args);
  for (int i=0; i< n_args; i++) {
    bool condition = (bool) va_arg(ap, int);
    if (condition) {
      trueConditions++;
    }
  }
  if (trueConditions <= 0) {
    return false;
  } else if (trueConditions > 1) {
    return false;
  }
  return true;
}

bool ServerLoadArgsGood() {
  //Check that only one transmission scheme is being used
  if (! CheckMutuallyExclusiveConditions(3,ServerLoadDistributionUniform,ServerLoadDistributionNormal,ServerLoadDistributionExponential)){
    NS_LOG_WARN("Incorrect number of packet size parameters set, check command line arguments");
  }
  //Check conditions on individual distributions
  //Uniform transmision conditions
  if (ServerLoadDistributionUniform) {
    return CheckUniformDistributionParameters(ServerLoadDistributionUniformMin,ServerLoadDistributionUniformMax);
  }
  //Normal Distribution
  if (ServerLoadDistributionNormal) {
    return CheckNormalDistributionParameters(ServerLoadDistributionNormalMean, ServerLoadDistributionNormalStd);
  }
  //Exponential Distribution
  if (ServerLoadDistributionExponential) {
    return CheckExponentialDistributionParameters(ServerLoadDistributionExponentialLambda,ServerLoadDistributionExponentialMultiplier,ServerLoadDistributionExponentialMin);
  }
  return false;
}

bool ClientPacketSizeArgsGood() {
  //Check that only one transmission scheme is being used
  if (! CheckMutuallyExclusiveConditions(3,PacketSizeDistributionUniform,PacketSizeDistributionNormal,PacketSizeDistributionExponential)){
    NS_LOG_WARN("Incorrect number of packet size parameters set, check command line arguments");
  }
  //Check conditions on individual distributions
  //Uniform transmision conditions
  if (PacketSizeDistributionUniform) {
    return CheckUniformDistributionParameters(PacketSizeDistributionUniformMin,PacketSizeDistributionUniformMax);
  }
  //Normal Distribution
  if (PacketSizeDistributionNormal) {
    return CheckNormalDistributionParameters(PacketSizeDistributionNormalMean, PacketSizeDistributionNormalStd);
  }
  //Exponential Distribution
  if (PacketSizeDistributionExponential) {
    return CheckExponentialDistributionParameters(PacketSizeDistributionExponentialLambda,PacketSizeDistributionExponentialMultiplier,PacketSizeDistributionExponentialMin);
  }
  return false;
}

bool ClientTransmissionArgsGood() {
  //Check that only one transmission scheme is being used
  if (! CheckMutuallyExclusiveConditions(3,TransmissionDistributionUniform,TransmissionDistributionNormal,TransmissionDistributionExponential)){
    NS_LOG_WARN("Incorrect number of Client transmission parameters set, check command line arguments");
  }
  //Check conditions on individual distributions
  //Uniform transmision conditions
  if (TransmissionDistributionUniform) {
    return CheckUniformDistributionParameters(TransmissionDistributionUniformMin,TransmissionDistributionUniformMax);
  }
  //Normal Distribution
  if (TransmissionDistributionNormal) {
    return CheckNormalDistributionParameters(TransmissionDistributionNormalMean, TransmissionDistributionNormalStd);
  }
  //Exponential Distribution
  if (TransmissionDistributionExponential) {
    return CheckExponentialDistributionParameters(TransmissionDistributionExponentialLambda,TransmissionDistributionExponentialMultiplier,TransmissionDistributionExponentialMin);
  }
  return false;
}

std::vector<uint32_t>GetServerLoadDistribution() {
  if (!ServerLoadArgsGood()) {
    NS_LOG_WARN("Error Processing Client ServerLoad Arguments");
  }
  if (ServerLoadDistributionUniform) {
    return uniform_distribution(DISTRIBUTION_SIZE, ServerLoadDistributionUniformMin, ServerLoadDistributionUniformMax);
  } else if (ServerLoadDistributionNormal) {
    return normal_distribution(DISTRIBUTION_SIZE, ServerLoadDistributionNormalMean, ServerLoadDistributionNormalStd);
  } else if (ServerLoadDistributionExponential) {
    return exponential_distribution(DISTRIBUTION_SIZE, ServerLoadDistributionExponentialLambda, ServerLoadDistributionExponentialMultiplier, ServerLoadDistributionExponentialMin);
  } else {
    NS_LOG_WARN("No server load distribution selected. (should be unreachable) check - ClientServerLoadArgsGood()");
  }

  NS_LOG_WARN("Reach the end of server load distribution without hitting an argument defined distribution");
  //TODO make this the smartest default
  return uniform_distribution(1,10,10); //Static Interval
}

std::vector<uint32_t>GetClientTransmissionDistribution() {
  if (!ClientTransmissionArgsGood()) {
    NS_LOG_WARN("Error Processing Client Transmission Arguments");
  }
  if (TransmissionDistributionUniform) {
    return uniform_distribution(DISTRIBUTION_SIZE, TransmissionDistributionUniformMin, TransmissionDistributionUniformMax);
  } else if (TransmissionDistributionNormal) {
    return normal_distribution(DISTRIBUTION_SIZE, TransmissionDistributionNormalMean, TransmissionDistributionNormalStd);
  } else if (TransmissionDistributionExponential) {
    return exponential_distribution(DISTRIBUTION_SIZE, TransmissionDistributionExponentialLambda, TransmissionDistributionExponentialMultiplier, TransmissionDistributionExponentialMin);
  } else {
    NS_LOG_WARN("No client transmission distribution selected. (should be unreachable) check - ClientTransmissionArgsGood()");
  }

  NS_LOG_WARN("Reach the end of client transmission distribution without hitting an argument defined distribution");
  //TODO make this the smartest default
  return uniform_distribution(1,1000,1000); //Static Interval
}

std::vector<uint32_t>GetPacketSizeDistribution() {
  if (!ClientPacketSizeArgsGood()) {
    NS_LOG_WARN("Error Processing Client PacketSize Arguments");
  }
  if (PacketSizeDistributionUniform) {
    return uniform_distribution(DISTRIBUTION_SIZE, PacketSizeDistributionUniformMin, PacketSizeDistributionUniformMax);
  } else if (PacketSizeDistributionNormal) {
    return normal_distribution(DISTRIBUTION_SIZE, PacketSizeDistributionNormalMean, PacketSizeDistributionNormalStd);
  } else if (PacketSizeDistributionExponential) {
    return exponential_distribution(DISTRIBUTION_SIZE, PacketSizeDistributionExponentialLambda, PacketSizeDistributionExponentialMultiplier, PacketSizeDistributionExponentialMin);
  } else {
    NS_LOG_WARN("No packet distribution selected. (should be unreachable) check - ClientPacketSizeArgsGood()");
  }

  NS_LOG_WARN("Reach the end of packet size distribution without hitting an argument defined distribution");
  //TODO make this the smartest default
  //Set all packets to 128 bytes
  return uniform_distribution(1,128,128); //Static Interval
}
//\Globals




//// Custom Routing
void
CreateAndAggregateObjectFromTypeId (Ptr<Node> node, const std::string typeId)
{
  ObjectFactory factory;
  factory.SetTypeId (typeId);
  Ptr<Object> protocol = factory.Create <Object> ();
  node->AggregateObject (protocol);
}

Ptr<Ipv4DoppelgangerRouting> GetDoppelgangerRouter(Ptr<Node> node) {
      Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
      Ptr<Ipv4ListRouting> list = DynamicCast<Ipv4ListRouting>(ipv4->GetRoutingProtocol());
      int16_t valuePlaceholder=0;
      Ptr<Ipv4DoppelgangerRouting> router = DynamicCast<Ipv4DoppelgangerRouting>(list->GetRoutingProtocol(0,valuePlaceholder));
      return router;
}

//Derrived from Internet stack helper
static void AddInternetStack (Ptr <Node> node){

      ObjectFactory m_tcpFactory;
      Ipv4RoutingHelper *m_routing;

      m_tcpFactory.SetTypeId ("ns3::TcpL4Protocol");
      Ipv4StaticRoutingHelper staticRouting;
      Ipv4GlobalRoutingHelper globalRouting;
      Ipv4DoppelgangerRoutingHelper doppelgangerRouting;
      Ipv4ListRoutingHelper listRouting;
      //The entire point of this routine is to add this call
      listRouting.Add (doppelgangerRouting, 1);
      //Change complete
      listRouting.Add (staticRouting, 0);
      listRouting.Add (globalRouting, -10);
      m_routing = listRouting.Copy();



      if (node->GetObject<Ipv4> () != 0)
        {
          NS_FATAL_ERROR ("InternetStackHelper::Install (): Aggregating " 
                          "an InternetStack to a node with an existing Ipv4 object");
          return;
        }

      CreateAndAggregateObjectFromTypeId (node, "ns3::ArpL3Protocol");
      CreateAndAggregateObjectFromTypeId (node, "ns3::Ipv4L3Protocol");
      CreateAndAggregateObjectFromTypeId (node, "ns3::Icmpv4L4Protocol");

      //Ptr<ArpL3Protocol> arp = node->GetObject<ArpL3Protocol> ();
      //NS_ASSERT (arp);
      //arp->SetAttribute ("RequestJitter", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));

      // Set routing
      Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
      Ptr<Ipv4RoutingProtocol> ipv4Routing = m_routing->Create (node);
      ipv4->SetRoutingProtocol (ipv4Routing);

      CreateAndAggregateObjectFromTypeId (node, "ns3::TrafficControlLayer");
      CreateAndAggregateObjectFromTypeId (node, "ns3::UdpL4Protocol");
      node->AggregateObject (m_tcpFactory.Create<Object> ());
      Ptr<PacketSocketFactory> factory = CreateObject<PacketSocketFactory> ();
      node->AggregateObject (factory);

      Ptr<ArpL3Protocol> arp = node->GetObject<ArpL3Protocol> ();
      Ptr<TrafficControlLayer> tc = node->GetObject<TrafficControlLayer> ();
      NS_ASSERT (arp);
      NS_ASSERT (tc);
      arp->SetTrafficControl (tc);
}

Ipv4Address getNodeIP(Ptr<Node> node) {
  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
  Ipv4InterfaceAddress iaddr = ipv4->GetAddress (1,0); 
  Ipv4Address ipAddr = iaddr.GetLocal (); 
  return ipAddr;
}

//----------------------------------------------RPC Client----------------------------------------------------
void SetDoppelgangerRoutingParameters(NodeContainer nodes, Ipv4DoppelgangerRouting::FatTreeSwitchType switchType, Ipv4DoppelgangerRouting::LoadBalencingStrategy strat, std::vector<std::vector<int>> rpcServices, std::map<uint32_t, uint32_t> ipServerMap, uint64_t * serverLoad, Time * serverLoad_update) {
  NodeContainer::Iterator i;
  for (i = nodes.Begin(); i != nodes.End(); ++i) {
      NS_LOG_WARN("Installing parameters on" << (*i)->GetId());
      Ptr<Ipv4> ipv4 = (*i)->GetObject<Ipv4> ();
      Ptr<Ipv4RoutingProtocol> routingProtocol = ipv4->GetRoutingProtocol();

      Ptr<Ipv4ListRouting> list = DynamicCast<Ipv4ListRouting>(routingProtocol);
      int16_t routing_priority=0;
      Ptr<Ipv4DoppelgangerRouting> doppelRouter = DynamicCast<Ipv4DoppelgangerRouting>(list->GetRoutingProtocol(0,routing_priority));
      doppelRouter->SetRpcServices(rpcServices);
      doppelRouter->SetIPServerMap(ipServerMap);
      doppelRouter->SetGlobalServerLoad(serverLoad);
      doppelRouter->SetGlobalServerLoadUpdate(serverLoad_update);
      doppelRouter->SetLoadBalencingStrategy(strat);
      doppelRouter->SetFatTreeSwitchType(switchType);
      doppelRouter->SetAddress(getNodeIP((*i)));
      doppelRouter->SetFatTreeK(K);
      //Start here we need to get the list routing protocol
  }

}

void SetupRpcClient(
  float duration, uint16_t Ports[NODES], Address addresses[NODES], int trafficMatrix[NODES][NODES], NodeContainer nodes,
   int clientIndex, uint32_t * global_packets_sent,
  std::vector<uint32_t> ClientPacketSizeDistribution,   
  std::vector<uint32_t> ClientTransmissionDistribution,
  std::vector<uint32_t> RPCServiceDistribution,
   RpcClient::selectionStrategy rpcSelectionStrategy,
std::vector<std::vector<int>> rpcServices, uint64_t * serverLoad, Time* serverLoad_update)
{
  RpcClientHelper rpcClient(addresses[0], int(Ports[0]));
  ApplicationContainer clientApps = rpcClient.Install(nodes.Get(clientIndex));
  clientApps.Start(Seconds(0));
  clientApps.Stop(Seconds(duration));
  Ptr<RpcClient> uec = DynamicCast<RpcClient>(clientApps.Get(0));

  //Convert Addresses
  ////TODO Start here, trying to convert one set of pointers to another.
  Address *addrs = new Address [NODES];
  uint16_t *ports = new uint16_t [NODES];
  int **tm = new int *[NODES];
  for (int i = 0; i < NODES; i++)
  {
    addrs[i] = addresses[i];
    ports[i] = Ports[i];
    tm[i] = &trafficMatrix[i][0];
  }
  //uec->SetAllAddresses((Address **)(addresses),(uint16_t **)(Ports),PARALLEL,NODES);
  //uec->SetAllAddresses(addrs, ports, tm, PARALLEL, NODES);
  uec->SetAllAddresses(addrs, ports, tm, NODES);
  uec->SetGlobalPackets(global_packets_sent);
  uec->SetRpcServices(rpcServices);
  uec->SetGlobalSeverLoad(serverLoad);
  uec->SetGlobalSeverLoadUpdate(serverLoad_update);
  //Fix this should be checked prior to executing
  uec->SetReplicaSelectionStrategy(rpcSelectionStrategy);

  uec->SetPacketSizeDistribution(ClientPacketSizeDistribution);
  uec->SetTransmitionDistribution(ClientTransmissionDistribution);
  uec->SetRPCDistribution(RPCServiceDistribution);
}

void printTM(int tm[NODES][NODES])
{
  printf("\n");
  for (int i = 0; i < NODES; i++)
  {
    for (int j = 0; j < NODES; j++)
    {
      printf("[%2d]", tm[i][j]);
    }
    printf("\n");
  }
}

void zeroTM(int tm[NODES][NODES])
{
  for (int i = 0; i < NODES; i++)
  {
    for (int j = 0; j < NODES; j++)
    {
      tm[i][j] = 0;
    }
  }
}

void populateTrafficMatrix(int tm[NODES][NODES], int pattern)
{
  zeroTM(tm);
  switch (pattern)
  {
  case CROSS_CORE:

    //This relies on the fact that clients are even numbers and servers
    //are odd, ie every client should have a relative server beside
    //them accounting for the (i-1) as the first term of the server
    //index equasion. THe second term adds the index to halfway across
    //the fat tree, the last term mods the server index by the size of the fat-tree.

    for (int i = 0; i < NODES; i++)
    {
      int serverindex = ((i - 1) + ((K * K * K) / 8)) % ((K * K * K) / 4); //TODO Debug this might not be right for all fat trees
      tm[i][serverindex] = 1;
    }
  }
  printTM(tm);
}

void SetupTraffic(float duration,

int serverport, NodeContainer nodes, int numNodes, int tm[NODES][NODES], RpcClient::selectionStrategy rpcSelectionStrategy, Address rpcServerAddresses[NODES], uint16_t Ports[NODES],
uint32_t * global_packets_sent, 
std::vector<uint32_t> ClientPacketSizeDistribution,   
std::vector<uint32_t> ClientTransmissionDistribution,
std::vector<uint32_t> RPCServiceDistribution,
std::vector<std::vector<int>> rpcReplicas, 
std::vector<std::vector<int>> rpcServices, 
uint64_t * serverLoad,
Time * serverLoad_update,
std::vector<uint32_t> ServerLoadDistribution) {

  //Assign attributes to RPC Servers
  for (int i = 0; i < numNodes; i++) {
    ApplicationContainer serverApps;
    RpcServerHelper rpcServer(serverport);
    serverApps = rpcServer.Install(nodes.Get(i));
		serverApps.Start (Seconds (0));
		serverApps.Stop (Seconds (duration));

    //Each server gets an array of services to serve
    Ptr<RpcServer> server = DynamicCast<RpcServer>(serverApps.Get(0));
    server->AssignRPC(rpcServices[i]);
    server->SetGlobalLoad(serverLoad);
    server->SetGlobalLoadUpdate(serverLoad_update);
    server->SetID(i);
    server->SetLoadDistribution(ServerLoadDistribution);
  }

  //Setup clients on every node
  for (int i = 0; i < numNodes; i++) {
    SetupRpcClient(duration, Ports, rpcServerAddresses, tm, nodes, i, global_packets_sent, ClientPacketSizeDistribution, ClientTransmissionDistribution, RPCServiceDistribution, rpcSelectionStrategy, rpcReplicas, serverLoad, serverLoad_update);
    /*
    for (int j =0 ; j < 100; j++){
      NS_LOG_WARN("ONLY ALLOCATING A SINGLE CLIENT\n");
    }
     break;
     */
  }
}


void translateIp(int base, int *a, int *b, int *c, int *d)
{
  *d = base % 256;
  base = base / 256;
  *c = base % 256;
  base = base / 256;
  *b = base % 256;
  base = base / 256;
  *a = base % 256;
  return;
}

//Replication Placement Strategies
//No replication, each server performs exactly 1 RPC
void replicationStrategy_noReplication(std::vector<std::vector<int>> *replicas) {
  for(int i=0;i<NODES;i++) {
    //Each server serves their own ID's RPC
    (*replicas)[i].push_back(i);
  }
}

//Replicate each service so that there are 2 instances of the RPC, and they live on oposite halfs ove the fat tree
void replicationStrategy_crossCoreReplication(std::vector<std::vector<int>> *replicas) {
  for(int i=0;i<NODES;i++) {
    //Each server serves their own ID's RPC
    (*replicas)[i].push_back(i);
    (*replicas)[i].push_back((i + (NODES / 2)) % NODES);
  }
}

int toIP(int a, int b, int c, int d ) {
	int total = 0;
	total += a << 24;
	total += b << 16;
	total += c << 8;
	total += d;
	return total;
}

void printIP(int ip) {
	int a = (ip & (255 << 24)) >> 24;
	int b = (ip & (255 << 16)) >> 16;
	int c = (ip & (255 << 8)) >> 8;
	int d = (ip & (255 << 0)) >> 0;
	NS_LOG_WARN(a << "." << b << "." <<  c << "." << d);
}


void SetIpv4NodeDevices(Ptr<Node> node, uint32_t ip) {
  for ( uint devNum=0; devNum < node->GetNDevices(); ++devNum) {
    Ptr<NetDevice> device = node->GetDevice(devNum);

    //Stolen from helper
    Ptr<Node> nodeprime = device->GetNode ();
    NS_ASSERT_MSG (nodeprime, "Ipv4AddressHelper::Assign(): NetDevice is not not associated "
                  "with any node -> fail");

    Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
    NS_ASSERT_MSG (ipv4, "Ipv4AddressHelper::Assign(): NetDevice is associated"
                  " with a node without IPv4 stack installed -> fail "
                  "(maybe need to use InternetStackHelper?)");

    int32_t interface = ipv4->GetInterfaceForDevice (device);
    if (interface == -1)
      {
        interface = ipv4->AddInterface (device);
      }
    NS_ASSERT_MSG (interface >= 0, "Ipv4AddressHelper::Assign(): "
                  "Interface index not found");

    //Ipv4InterfaceAddress ipv4Addr = Ipv4InterfaceAddress (NewAddress (), toIP(255,0,0,0));
    Ipv4InterfaceAddress ipv4Addr = Ipv4InterfaceAddress (Ipv4Address (ip), toIP(255,0,0,0));
    ipv4->AddAddress (interface, ipv4Addr);
    ipv4->SetMetric (interface, 1);
    ipv4->SetUp (interface);
  }
}


uint64_t SumPacketRedirections(NodeContainer nc) {
  uint64_t sum = 0;
  for (NodeContainer::Iterator node = nc.Begin(); node != nc.End(); ++node) {
    Ptr<Ipv4DoppelgangerRouting> router = GetDoppelgangerRouter((*node));
    sum+=router->GetPacketRedirections();
    //printf("Router %lu -- sum %lu\n",router->GetPacketRedirections(), sum);
  }
  return sum;
}

uint64_t SumPackets(NodeContainer nc) {
  uint64_t sum = 0;
  for (NodeContainer::Iterator node = nc.Begin(); node != nc.End(); ++node) {
    Ptr<Ipv4DoppelgangerRouting> router = GetDoppelgangerRouter((*node));
    sum+=router->GetTotalPackets();
    //printf("Router %lu -- sum %lu\n",router->GetPacketRedirections(), sum);
  }
  return sum;
}

int main(int argc, char *argv[])
{
  parseArgs(argc, argv);
  //Default vlaues for command line arguments

  Config::SetDefault("ns3::Ipv4GlobalRouting::RandomEcmpRouting", BooleanValue(true));
  Config::SetDefault("ns3::Ipv4GlobalRouting::RespondToInterfaceEvents", BooleanValue(true));


  //printf("Client - NPackets %d, baseInterval %f packetSize %d \n",ClientProtocolNPackets,ClientProtocolInterval,ClientProtocolPacketSize);
  //printf("Cover - NPackets %d, baseInterval %f packetSize %d \n",CoverNPackets,CoverInterval,CoverPacketSize);

  Time::SetResolution(Time::NS);
  LogComponentEnable("RpcClientApplication", LOG_LEVEL_WARN);
  if (debug)
  {
    LogComponentEnable("RpcClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("RpcServerApplication", LOG_LEVEL_INFO);
    LogComponentEnable("VarClients", LOG_LEVEL_INFO);
    LogComponentEnable("VarClients", LOG_LEVEL_INFO);
    LogComponentEnable("Ipv4DoppelgangerRouting",LOG_LEVEL_INFO);
  }

  NS_LOG_INFO("Setting Random Seed");
  srand (time(NULL));


  NodeContainer nodes;
  nodes.Create (NODES);

  NodeContainer edge;
  edge.Create(EDGES);

  NodeContainer agg;
  agg.Create(AGGS);

  NodeContainer core;
  core.Create(CORE);
  NodeContainer nc_node2edge[NODES];
  NetDeviceContainer ndc_node2edge[NODES];

  NodeContainer nc_edge2agg[EDGE * AGG * PODS];
  NetDeviceContainer ndc_edge2agg[EDGE * AGG * PODS];

  NodeContainer nc_agg2core[CORE*PODS];
  NetDeviceContainer ndc_agg2core[CORE*PODS];
  

  //connect nodes to edges
  for (int n = 0; n < NODES; n++) {
      nc_node2edge[n] = NodeContainer(edge.Get(n/(K/2)), nodes.Get(n));
  }

  //connect edges to agg
  for (int pod = 0; pod < PODS; pod++) {
      for (int edgeS = 0; edgeS < EDGE; edgeS++) {
          for (int aggS = 0; aggS < AGG; aggS++) {
              int aggIndex = pod*AGG + aggS;
              int edgeIndex = pod*EDGE + edgeS;
              int link = (pod * (K/2)*(K/2)) + (edgeS * EDGE) + aggS;
              nc_edge2agg[link] = NodeContainer(agg.Get(aggIndex), edge.Get(edgeIndex));
          }
      }
  }

  //connect agg to core
  for (int coreS = 0; coreS < CORE;coreS++) {
      for (int pod = 0; pod < PODS; pod++) {
              nc_agg2core[(coreS * CORE) + pod] = NodeContainer(core.Get(coreS), agg.Get((pod*AGG) + (coreS/AGG)));
      }
  }

  NodeContainer::Iterator i;
  for (i = nodes.Begin(); i != nodes.End(); ++i) {
      NS_LOG_WARN("Installing " << (*i)->GetId());
      AddInternetStack(*i);
  }
  for (i = edge.Begin(); i != edge.End(); ++i) {
      NS_LOG_WARN("Installing " << (*i)->GetId());
      AddInternetStack(*i);
  }
  for (i = agg.Begin(); i != agg.End(); ++i) {
      NS_LOG_WARN("Installing " << (*i)->GetId());
      AddInternetStack(*i);
  }
  for (i = core.Begin(); i != core.End(); ++i) {
      NS_LOG_WARN("Installing " << (*i)->GetId());
      AddInternetStack(*i);
  }

  PointToPointHelper pointToPoint;
  PointToPointHelper pointToPoint2;

  TrafficControlHelper tch;
  //int linkrate = 1000;
  int linkrate = 1000;
  int queuedepth = 50;

  pointToPoint.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue(std::to_string(queuedepth) + "p"));
  pointToPoint.SetDeviceAttribute("DataRate", StringValue(std::to_string(linkrate) + "Mbps"));
  pointToPoint.SetChannelAttribute("Delay", StringValue("1.0us"));
  ////
  pointToPoint2.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue(std::to_string(queuedepth) + "p"));
  pointToPoint2.SetDeviceAttribute("DataRate", StringValue(std::to_string(linkrate) + "Mbps"));
  pointToPoint2.SetChannelAttribute("Delay", StringValue("1.0us"));

  uint16_t handle = tch.SetRootQueueDisc("ns3::FifoQueueDisc");
  tch.AddInternalQueues(handle, 1, "ns3::DropTailQueue", "MaxSize", StringValue(std::to_string(queuedepth) + "p"));


  //connect nodes to edges
  for (int n = 0; n < NODES; n++) {
      ndc_node2edge[n] = pointToPoint.Install(nc_node2edge[n]);
  }
  //connect edge to agg
  const int aggC = EDGE * AGG * PODS;
  //const int aggC = 1;
  for (int e=0;e<aggC;e++){
      //printf("edge %d total %d\n",e,aggC);
      //printf("c.GetN == %d\n",nc_edge2agg[e].GetN());
      ndc_edge2agg[e] = pointToPoint.Install(nc_edge2agg[e]);
  }
  //connect agg to cores
  for (int s = 0;s < CORE * PODS;s++) {
      ndc_agg2core[s] = pointToPoint.Install(nc_agg2core[s]);
  }

  //Assign queues AFTER the stack install (not sure why)

  //connect nodes to edges
  for (int n = 0; n < NODES; n++) {
      tch.Install(ndc_node2edge[n].Get(0));
      tch.Install(ndc_node2edge[n].Get(1));
  }

  printf("Connecting Topology");
  //connect edges to agg
  for (int pod = 0; pod < PODS; pod++) {
      for (int edgeS = 0; edgeS < EDGE; edgeS++) {
          for (int aggS = 0; aggS < AGG; aggS++) {
              //int aggIndex = pod*AGG + aggS;
              //int edgeIndex = pod*EDGE + edgeS;
              int link = (pod * (K/2)*(K/2)) + (edgeS * EDGE) + aggS;
              //int link = (pod * PODS) + (aggIndex) + edgeIndex;
              //printf("link %d, aggIndex %d, edgeIndex %d\n",link,aggIndex,edgeIndex);
              tch.Install(ndc_edge2agg[link].Get(0));
              tch.Install(ndc_edge2agg[link].Get(1));
          }
      }
  }

  //connect agg to core
  for (int coreS = 0; coreS < CORE;coreS++) {
      for (int pod = 0; pod < PODS; pod++) {
              tch.Install(ndc_agg2core[(coreS * CORE) + pod].Get(0));
              tch.Install(ndc_agg2core[(coreS * CORE) + pod].Get(1));
      }
  }


  //Ipv4AddressHelper address;
  //Ipv4InterfaceContainer node2edge[NODES];
  //Ipv4InterfaceContainer edge2agg[EDGE*AGG*PODS];
  //Ipv4InterfaceContainer agg2core[CORE*PODS];

  //TODO Assign address as described in the fat tree paper, code for doing so is prototyped in pfattree.c
  //address.SetBase("10.1.1.0", "255.255.255.255");

  //Generate Addresses for each of the end hosts and assign them


  /*
  for (int i=0;i<NODES;i++) {
  	node2edge[i] = address.Assign(ndc_node2edge[i]);
  }
  for (int i=0;i<EDGE*AGG*PODS;i++) {
  	edge2agg[i] = address.Assign(ndc_edge2agg[i]);
  }
  for (int i=0;i<CORE*PODS;i++) {
  	agg2core[i] = address.Assign(ndc_agg2core[i]);
  }*/

  int a,b,c,d;
  a = 10;
  int itterator=0;
  printf("Setting up IP addresses\n");
  for (int i=0; i<PODS; i++) {
    for (int j=0; j<EDGE; j++) {
      for (int k=2; k<( NODE + 2);  k++) {
        b=i;
        c=j;
        d=k;
        //printf("Printing IP\n");
        int ip=toIP(a,b,c,d);
        printIP(ip);

        Ptr<Node> node = nodes.Get(itterator);
        SetIpv4NodeDevices(node, ip);
        itterator++;
      }
    }
  }

  //Set up node routing tables

  for ( int n=0;n<NODES;n++ ) {
    Ptr<Node> node = nodes.Get(n);
    a = 10;
    for (int i=0; i<PODS; i++) {
      for (int j=0; j<EDGE; j++) {
        for (int k=2; k<( NODE + 2);  k++) {
          //skip assigning a route to yourself
          //if (itterator == n) {
          //  continue;
          //}
          b=i;
          c=j;
          d=k;
          int ip=toIP(a,b,c,d);
          printIP(ip);
          Ptr<Ipv4DoppelgangerRouting> router = GetDoppelgangerRouter(node);
          router->AddRoute(Ipv4Address(ip),Ipv4Mask(toIP(255,0,0,0)),1);
        }
      }
    }
  }


  a=10;
  d=1;
  itterator = 0;
  for (int i=0; i<PODS; i++) {
    for (int j=0; j<EDGE; j++) {
      b=i;
      c=j;
      //printf("Printing IP\n");
      int ip=toIP(a,b,c,d);
      printIP(ip);
      Ptr<Node> node = edge.Get(itterator);
      SetIpv4NodeDevices(node, ip);
      itterator++;
    }
  }

  int a2,b2,c2,d2;
  a2=10;

  for(int e=0; e<EDGES;e++) {
    Ptr<Node> node = edge.Get(e);
    Ipv4Address ipAddr = getNodeIP(node);
    Ptr<Ipv4DoppelgangerRouting> router = GetDoppelgangerRouter(node);
    int a1,b1,c1,d1;
    translateIp(ipAddr.Get(),&a1,&b1,&c1,&d1);

    for (int i=0; i<PODS; i++) {
      for (int j=0; j<EDGE; j++) {
        for (int k=2; k<( NODE + 2);  k++) {
          b2=i;
          c2=j;
          d2=k;
          int ip=toIP(a2,b2,c2,d2);
          printIP(ip);

          //Edge Up
          //IP is in another pod or belonging to a different edge
          if (b1 != b2 || c1 != c2) {
            //Add every IP on all upward ports
            for( int port = K/2 + 1; port <= K ; port++) {
              router->AddRoute(Ipv4Address(ip),Ipv4Mask(toIP(255,0,0,0)),port); //Up 2
            }
          //Edge Down
           } else if (b1 == b2 && c1 == c2) {
            //IP is in this pod
            //HACK the port id's on the router go from 0 - (K/2)^2, the flip
            //here is that the first indes is an incorrect address. I think
            //that it's the mac not the ipv4 or something like this. the D
            //index starts from 2 based on the fat tree topology, therefore any
            //down stream client can be indentified by gaking the last digit of
            //the IP subtracting one and setting that to the port index.
            router->AddRoute(Ipv4Address(ip),Ipv4Mask(toIP(255,0,0,0)),d2-1); //down
          }
        }
      }
    }
  }

  a=10;
  d=1;
  itterator=0;
  for (int i=0; i<PODS; i++) {
    for (int j=EDGE; j<EDGE + AGG; j++) {
      b=i;
      c=j;
      //printf("Printing IP\n");
      int ip=toIP(a,b,c,d);
      printIP(ip);
      Ptr<Node> node = agg.Get(itterator);
      SetIpv4NodeDevices(node, ip);
      itterator++;
    }
  }

  for(int a=0; a<AGGS;a++) {
    Ptr<Node> node = agg.Get(a);
    Ipv4Address ipAddr = getNodeIP(node);
    Ptr<Ipv4DoppelgangerRouting> router = GetDoppelgangerRouter(node);
    int a1,b1,c1,d1;
    translateIp(ipAddr.Get(),&a1,&b1,&c1,&d1);

    for (int i=0; i<PODS; i++) {
      for (int j=0; j<EDGE; j++) {
        for (int k=2; k<( NODE + 2);  k++) {
          //skip assigning a route to yourself
          b2=i;
          c2=j;
          d2=k;
          int ip=toIP(a2,b2,c2,d2);
          printIP(ip);

          //Edge Up
          //IP is in another pod // All nodes in this pod can be routed to from here
          if (b1 != b2) {
            //Add every IP on all upward ports
            for( int port = K/2 + 1; port <= K ; port++) {
              router->AddRoute(Ipv4Address(ip),Ipv4Mask(toIP(255,0,0,0)),port); //Up 2
            }

           } else if (b1 == b2) {
            //IP is in this pod

            //Another hack. The second to last digit in the IP space c a.b.c.d
            //denotes the edge router that the node is attached too. To route
            //to the correct port, the c (range [0,k-1]) can be routed to by
            //selecting port c + 1
            router->AddRoute(Ipv4Address(ip),Ipv4Mask(toIP(255,0,0,0)),c2+1); //down
          }
        }
      }
    }
  }


  a=10;
  b=PODS;
  d=1;
  itterator = 0;
  for (int i=1; i<AGG + 1; i++) {
    for (int j=1; j<AGG + 1; j++) {
      c=i;
      d=j;
      //printf("Printing IP\n");
      int ip=toIP(a,b,c,d);
      printIP(ip);
      Ptr<Node> node = core.Get(itterator);
      SetIpv4NodeDevices(node, ip);
      itterator++;
    }
  }

  for(int c=0; c<CORE;c++) {
    Ptr<Node> node = core.Get(c);
    Ipv4Address ipAddr = getNodeIP(node);
    Ptr<Ipv4DoppelgangerRouting> router = GetDoppelgangerRouter(node);
    int a1,b1,c1,d1;
    translateIp(ipAddr.Get(),&a1,&b1,&c1,&d1);

    for (int i=0; i<PODS; i++) {
      for (int j=0; j<EDGE; j++) {
        for (int k=2; k<( NODE + 2);  k++) {
          //skip assigning a route to yourself
          b2=i;
          c2=j;
          d2=k;
          int ip=toIP(a2,b2,c2,d2);
          printIP(ip);
          //All paths route down
          int port = b2 + 1; //Good ports start at 1
          router->AddRoute(Ipv4Address(ip),Ipv4Mask(toIP(255,0,0,0)),port); //down
        }
      }
    }
  }
  //return -1;

  ////////////////////////////////////////////////////////////////////////////////////
  //Setup Clients
  ///////////////////////////////////////////////////////////////////////////////////
  int RpcServerPort = 10;
  float duration = 0.01;
  //float duration = 0.003;

  uint32_t global_packets_sent = 0;

  Address IPS[NODES];
  uint16_t Ports[NODES];
  for (int i = 0; i < NODES; i++)
  {
      Ptr<Node> node = nodes.Get(i);
      //Ptr<NetDevice> nd = node->GetDevice(1);
      //IPS[i] = node2edge[i].GetAddress(1);
      //IPS[i] = nd->GetAddress();
      IPS[i] = Address(getNodeIP(node));
      Ports[i] = uint16_t(RpcServerPort);
  }

  //Create Traffic Matrix
  int trafficMatrix[NODES][NODES];
  //int pattern = CROSS_CORE;
  //populateTrafficMatrix(trafficMatrix, pattern);

  //Create Server RPC database
  std::vector<std::vector<int>> servicesPerServer(NODES, std::vector<int>(0));
  //replicationStrategy_noReplication(&servicesPerServer);
  replicationStrategy_crossCoreReplication(&servicesPerServer);

  //Create Client Database for servers - each index is an RPC. This is an
  //inverted index of the servers. If a client wants to find which servers can
  //service RPC x it can lookup rpcReplicas[x] and get back each of the
  //corresponding servers
  std::vector<std::vector<int>> rpcReplicas(NODES, std::vector<int>(0));
  for(int i=0;i<NODES;i++){
    for (uint j=0;j<servicesPerServer[i].size();j++) {
      rpcReplicas[servicesPerServer[i][j]].push_back(i);
    }
  }

  //Create Server Load Global
  uint64_t* serverLoad = (uint64_t*) malloc(NODES * sizeof(uint64_t));
  Time* serverLoad_update = (Time*) malloc(NODES * sizeof(Time));
  for(int i=0;i<NODES;i++) {
    serverLoad[i]=0;
    serverLoad_update[i]=Time(0);
  }
  //Create IP map to server index
  std::map<uint32_t, uint32_t> ipServerMap;
  for (int i=0; i<NODES; i++) {
    Ipv4Address addr = Ipv4Address::ConvertFrom(IPS[i]);
    uint32_t ip_address = addr.Get();
    ipServerMap.insert(std::pair<uint32_t,uint32_t>(ip_address,i));
  }


  Ptr<MultichannelProbe> mcp = CreateObject<MultichannelProbe>(ProbeName);
  mcp->SetAttribute("Interval", StringValue("1s"));
  mcp->AttachAll();
  mcp->Stop(Seconds(duration));

  ApplicationContainer clientApps;
  ApplicationContainer serverApps;

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  clientApps.Start(Seconds(0));
  clientApps.Stop(Seconds(duration));
  serverApps.Start(Seconds(0));
  serverApps.Stop(Seconds(duration));

  /*
  Ipv4InterfaceContainer *node2edgePtr = new Ipv4InterfaceContainer[NODES];
  for (int i = 0; i < NODES; i++)
  {
    node2edgePtr[i] = node2edge[i];
  }*/


  //Generate Distributions
  std::vector<uint32_t> ClientPacketSizeDistribution = GetPacketSizeDistribution();
  std::vector<uint32_t> ClientTransmissionDistribution = GetClientTransmissionDistribution();
  //for( uint i=0; i < ClientTransmissionDistribution.size();i++) {
  //  printf("d[%d]=%d\n",i,ClientTransmissionDistribution[i]);
  //}
  std::vector<uint32_t> RPCServiceDistribution = uniform_distribution(rpcReplicas.size() * 5,0,rpcReplicas.size()-1); //Uniform requests for RPC servers

  std::vector<uint32_t> ServerLoadDistribution = GetServerLoadDistribution();


  SetupTraffic(
      duration,
      RpcServerPort,
      nodes,
      NODES,         //total nodes
      trafficMatrix, //distance
      rpcSelectionStrategy,
      IPS,
      Ports,
      &global_packets_sent,
      ClientPacketSizeDistribution,
      ClientTransmissionDistribution,
      RPCServiceDistribution,
      //RPC Globals
      servicesPerServer,
      rpcReplicas,
      serverLoad,
      serverLoad_update,
      ServerLoadDistribution
  );


  printf("setting custom load balencing strats\n");
  SetDoppelgangerRoutingParameters(nodes,Ipv4DoppelgangerRouting::endhost,networkSelectionStrategy,servicesPerServer,ipServerMap,serverLoad,serverLoad_update);
  SetDoppelgangerRoutingParameters(edge,Ipv4DoppelgangerRouting::edge,networkSelectionStrategy,servicesPerServer,ipServerMap,serverLoad,serverLoad_update);
  SetDoppelgangerRoutingParameters(agg,Ipv4DoppelgangerRouting::agg,networkSelectionStrategy,servicesPerServer,ipServerMap,serverLoad,serverLoad_update);
  SetDoppelgangerRoutingParameters(core,Ipv4DoppelgangerRouting::core,networkSelectionStrategy,servicesPerServer,ipServerMap,serverLoad,serverLoad_update);
  printf("done setting custom load ballencing\n");


  printf("Running the simulator!!\n");
  Simulator::Run();


  printf("Collecting Post Run information\n");

  std::ofstream runsummary;
  runsummary.open(WorkingDirectory + "RouterSummary.dat");
  runsummary << "NodeRedirections,";
  runsummary << "NodeTotal,";
  runsummary << "EdgeRedirections,";
  runsummary << "EdgeTotal,";
  runsummary << "AggRedirections,";
  runsummary << "AggTotal,";
  runsummary << "CoreRedirections,";
  runsummary << "CoreTotal,";
  runsummary << "\n";

  runsummary << SumPacketRedirections(nodes) << ",";
  runsummary << SumPackets(nodes) << ",";
  runsummary << SumPacketRedirections(edge) << ",";
  runsummary << SumPackets(edge) << ",";
  runsummary << SumPacketRedirections(agg) << ",";
  runsummary << SumPackets(agg) << ",";
  runsummary << SumPacketRedirections(core) << ",";
  runsummary << SumPackets(core);
  runsummary << "\n";

  runsummary.close();

  

  //Post Execution Data Collection
  printf("Node Redirections (%lu/%lu)\n",SumPacketRedirections(nodes),SumPackets(nodes));
  printf("Edge Redirections (%lu/%lu)\n",SumPacketRedirections(edge),SumPackets(edge));
  printf("Agg Redirections (%lu/%lu)\n",SumPacketRedirections(agg),SumPackets(agg));
  printf("Core Redirections (%lu/%lu)\n",SumPacketRedirections(core),SumPackets(core));


  Simulator::Destroy();


  return 0;
}

//NOTES on how to set up experiments. Each one of the major experiments should
//have some degrees of freedom, and some degrees of configurability.
//
//Things which should be passed in from the command line include - NPackets -
//NormalPacketSize - Client Rate (constant)
//
// The issue of configurability comes in which determining which nodes are
// running which traffic patterns, and what kinds of workloads they are using.
// Composition should be handeled in script. I cannot think of a useful reason
// to configure this early on, at least untill the efficacy of the protocols
// has been demonstrated.
//
//

