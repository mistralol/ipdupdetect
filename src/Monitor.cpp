
#include <main.h>

Monitor::Monitor(const std::string ifname) :
	m_ifname(ifname),
	m_exit(false)
{
	LogDebug("Started montor thread for interface '%s'", m_ifname.c_str());
	Start();
}

Monitor::~Monitor()
{
	LogDebug("Stopping montor thread for interface '%s'", m_ifname.c_str());
	m_exit = true;
	Stop();
	LogDebug("Stopped montor thread for interface '%s'", m_ifname.c_str());
}

void Monitor::Reset()
{
	ScopedLock lock(&m_ipsmutex);
	LogInfo("Reset cache for interface '%s'", m_ifname.c_str());
	m_ips.clear();
}

void Monitor::Dump()
{
	ScopedLock lock(&m_ipsmutex);
	LogInfo("Interface '%s' Info dump", m_ifname.c_str());
	for(auto it : m_ips)
	{
		LogInfo("IP %s has mac %s", it.first.c_str(), it.second.c_str());
	}
}

void Monitor::Run()
{
	pcap_t *pcap;
	
	char err[512];
	pcap = pcap_open_live(m_ifname.c_str(), sizeof(err), 0, 0, err);
	if (pcap == NULL)
	{
	
		LogError("pcap_open_live failed for interface '%s' error '%s'", m_ifname.c_str(), err);
		return;
	}
	
	if (pcap_setnonblock(pcap, 1, err) < 0)
	{
		LogCritical("pcap_setnonblock failed error: '%s'", err);
		abort();
	}
	
	int fd = pcap_get_selectable_fd(pcap);
	if (fd < 0)
	{
		LogCritical("invalid fd from pcap_get_selectalble_fd");
		abort();
	}
	
	while(m_exit == false)
	{
		struct pollfd fds = {fd, POLLIN, 0};
	
		int ret = poll(&fds, 1, 100);
		if (ret < 0)
		{
			LogCritical("Poll failed");
			abort();
		}
	
		if (ret == 1)
		{
			if ((fds.revents & POLLIN))
			{
				struct pcap_pkthdr hdr;
				const u_char *buf = pcap_next(pcap, &hdr);
				Process(&hdr, buf);
			}
		}
	}
	
	pcap_close(pcap);	
}

void Monitor::Process(const struct pcap_pkthdr *hdr, const u_char *buf)
{
	unsigned int len = hdr->len;
	unsigned int caplen = hdr->caplen;
	
	if (len < sizeof(struct ether_header))
		return;
	
	if (caplen < sizeof(struct ether_header))
		return;
		
	const struct ether_header *eth = (const struct ether_header *) buf;
	buf += sizeof(*eth);
	len -= sizeof(*eth);
	caplen -= sizeof(*eth);
	
	switch(ntohs(eth->ether_type))
	{
		case ETHERTYPE_ARP:
			//LogInfo("Got Arp");
			if (len < sizeof(struct ether_arp))
				return;
	
			if (caplen < sizeof(struct ether_arp))
				return;

			do {
				const struct ether_arp *arp = (const struct ether_arp *) buf;
				ProcessArp(arp);
			} while(0);
			break;
		case ETHERTYPE_IP:
			//Ignore ip packets for now
			break;
		case ETHERTYPE_IPV6:
			//Ignore IPv6 for now
			break;
		default:
			//LogDebug("Unknown Ethernet packet %d", ntohs(eth->ether_type));
			break;
	}
}



//    unsigned short int arphdr.ar_hrd;          /* Format of hardware address.  */
//    unsigned short int ar_pro;          /* Format of protocol address.  */
//    unsigned char ar_hln;               /* Length of hardware address.  */
//    unsigned char ar_pln;               /* Length of protocol address.  */
//    unsigned short int ar_op;           /* ARP opcode (command).  */
void Monitor::ProcessArp(const struct ether_arp *arp)
{
	//LogDebug("ProcessArp Command %d Hardware %d", ntohs(arp->ea_hdr.ar_op), ntohs(arp->ea_hdr.ar_hrd));
	unsigned short op = ntohs(arp->ea_hdr.ar_op);
	unsigned short hardware = ntohs(arp->ea_hdr.ar_hrd);
	
	if (hardware != ARPHRD_ETHER)
		return;

	//Sender ip / Reciver ip
	char sip[16], rip[16]; //255.255.255.255 + NULL
	sprintf(sip, "%d.%d.%d.%d", arp->arp_spa[0], arp->arp_spa[1], arp->arp_spa[2], arp->arp_spa[3]);
	sprintf(rip, "%d.%d.%d.%d", arp->arp_tpa[0], arp->arp_tpa[1], arp->arp_tpa[2], arp->arp_tpa[3]);
	
	//Sender mac / Reciver mac
	char smac[18], rmac[18]; //ff:ff:ff:ff:ff:ff + NULL
	sprintf(smac, "%02x:%02x:%02x:%02x:%02x:%02x",
		arp->arp_sha[0], arp->arp_sha[1], arp->arp_sha[2],
		arp->arp_sha[3], arp->arp_sha[4], arp->arp_sha[5]);
	sprintf(rmac, "%02x:%02x:%02x:%02x:%02x:%02x",
		arp->arp_tha[0], arp->arp_tha[1], arp->arp_tha[2],
		arp->arp_tha[3], arp->arp_tha[4], arp->arp_tha[5]);

	std::string src_ip(sip);
	std::string dest_ip(rip);
	std::string src_mac(smac);
	std::string dest_mac(rmac);
	
	switch(op)
	{
		case ARPOP_REQUEST:
			if (src_ip == "0.0.0.0")
				return;
			//LogDebug("ARP WHO HAS %s (%s) TELL %s (%s)", rip, rmac, sip, smac);

			//Track Based on sender ip address and mac
			do {
				ScopedLock lock(&m_ipsmutex);
				auto it = m_ips.find(src_ip);
				if (it == m_ips.end())
				{
					LogInfo("Interface %s Has IP Address %s Has HW: %s",
						m_ifname.c_str(), src_ip.c_str(), src_mac.c_str());
					m_ips[src_ip] = src_mac;
				}
				else
				{
					if (it->second != src_mac)
					{
						LogWarning("Interface %s IP Address %s Change from HW: %s to HW: %s",
							m_ifname.c_str(), src_ip.c_str(), it->second.c_str(), src_mac.c_str());
					}
				}
			} while(0);
			
			break;
		case ARPOP_REPLY:
			//We don't really se arp replies because they are not broadcast
			//So there is not much to do here.
			//LogDebug("ARP %s (%s) IS-AT %s (%s)", sip, smac, rip, rmac);
			break;
	}
}


