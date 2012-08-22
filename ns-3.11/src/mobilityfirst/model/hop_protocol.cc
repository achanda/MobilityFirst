//functions for handling HOP packets and tables

#include "hop_protocol.h"
#include "ns3/socket-factory.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/names.h"
#include "ns3/random-variable.h"
#include "ns3/inet-socket-address.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-routing-table-entry.h"
#include "ns3/ipv4-route.h"
#include "ns3/boolean.h"
#include "ns3/uinteger.h"
#include "ns3/enum.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/ipv4-header.h"
#include "ns3/nstime.h"
#include <algorithm>
#include <limits>
#include <iostream>
#include <cstdio>

#define DELAY(time) (((time) < (Simulator::Now ())) ? Seconds (0.000001) : \
                     (time - Simulator::Now () + Seconds (0.000001)))



using namespace std;

namespace ns3 
{

	HOPProtocol::HOPProtocol()
        {
		//fp_1 = fopen("/home/nehal/Desktop/test_read_1.txt", "w"); 
		//fp_2 = fopen("/home/nehal/Desktop/test_read_2.txt", "w"); 
		//fp_3 = fopen("/home/nehal/Desktop/test_read_3.txt", "w"); 
		total_chunck = 0;
		chkID = 0;
		
	}

	//reads in from a file and creates data packets
	/*Ptr<Packet> HOPProtocol::FromFile (Ipv4Address m_mainAddress, Ipv4Address sender, Ipv4Address receiver)
	{
		FILE *f;
		if (sender == "10.1.1.6")
			f = fopen ("/home/nehal/Desktop/nehals_1.txt" , "r" );
		if (sender == "10.1.1.1")
			f = fopen ("/home/nehal/Desktop/nehals_2.txt" , "r" );
		if (sender == "10.1.1.7")
			f = fopen ("/home/nehal/Desktop/nehals_3.txt" , "r" );

		NS_ASSERT (f);
		cout << "File opened" << endl;
		int counter = 0;
		int chk_ID, seq;
		int cur_bytes_read = 0;
		int size;
		char *payload;
		DataTuple tuple;
		//cout << "Reading" << endl;

		while(!feof(f)) 
		{
			//cout << "Reading" << endl;
			counter++;	
			payload = (char*) malloc (sizeof(char)*PAYLOAD_SIZE);	
			int cur_seq_num = counter % CHK_PKT_COUNT;
			seq = cur_seq_num==0 ? CHK_PKT_COUNT : cur_seq_num; // packet sequence number	
			//cout << "Packet Number" << seq << endl;
			chk_ID = cur_seq_num==0 ? counter/CHK_PKT_COUNT : counter/CHK_PKT_COUNT + 1; //chunk ID
			//cout << "Chunck Number" << chk_ID << endl;
			
			cur_bytes_read = fread(payload, 1,PAYLOAD_SIZE, f);//read in bytes from file
			//cout << "Reading" << endl;
			//cout << cur_bytes_read << endl;
			//cout << payload << endl;

			Ptr<Packet> packet = Create<Packet> ();
        		mf::MessageHeader msg;
	       		msg.SetMessageType (9); //type = 9 for data packets
	       		msg.SetSequenceNumber (chk_ID); // sets chunck ID
               		msg.SetMessageAddr(m_mainAddress); //sets the IP address
			
			mf::MessageHeader::DATA &data = msg.GetDATA ();
			data.src = sender; //source address
			data.dest = receiver; //destination address
			data.pkt_no = seq; //packet sequence number
			data.payload_size = cur_bytes_read; //data
			for (int i=0; i<cur_bytes_read; i++)
				data.payload[i] = payload[i];
			packet->AddHeader(msg);

			//cout << "Pkt Size" << packet->GetSize() << endl;

			// saves the packet in a data set
			if (seq == 1)
			{
				tuple.source = sender;
				//tuple.packet_count = seq;
				tuple.chunck_id = chk_ID;
				//cout << "1" << endl;
				tuple.data_pkts.push_back (packet);
			}
			else
				tuple.data_pkts.push_back (packet);

			if (chk_ID == 1)
			{
				size = tuple.data_pkts.size ();
				//cout << size << endl;
			}

			if (seq == CHK_PKT_COUNT)
			{
				//tuple.packet_count = seq;				
				m_dataSet.push_back (tuple);
				tuple.data_pkts.clear ();
				PacketCountTuple pkt_count_tuple; 
				pkt_count_tuple.source = sender;
				pkt_count_tuple.chunck_id = chk_ID;
				pkt_count_tuple.packet_count = seq;
				m_PktCountSet.push_back (pkt_count_tuple);
				//cout << "inserted chunck: " << chk_ID << endl;
			}

		}
		
		if (seq < CHK_PKT_COUNT)
		{
			//tuple.packet_count = seq;
			m_dataSet.push_back (tuple);
			tuple.data_pkts.clear ();
			PacketCountTuple pkt_count_tuple; 
			pkt_count_tuple.source = sender;
			pkt_count_tuple.chunck_id = chk_ID;
			pkt_count_tuple.packet_count = seq;
			m_PktCountSet.push_back (pkt_count_tuple);
			//cout << "inserted chunck: " << chk_ID << endl;
		}
		
		// initilizes file transfer by sending CSYN packet for chunck 1
		Ptr<Packet> packet = SendCSYNC (size, 1, m_mainAddress, sender, receiver);		
		return packet;
	}*/

	Ptr<Packet> HOPProtocol::FromFile (Ipv4Address m_mainAddress, Ipv4Address sender, Ipv4Address receiver)
	{
		//int counter = 0;
		//uint32_t chk_ID = 1;

		chkID++;
		cout << "Chunck ID at source: " << chkID << endl;
		uint32_t seq;
		//int cur_bytes_read = 0;
		int size;
		char *payload;
		DataTuple tuple;
		//cout << "Reading" << endl;

		//while(chk_ID <= 10000) 
		//{
			//cout << "Reading" << endl;
			//counter++;	
		for (int j = 1; j<= CHK_PKT_COUNT; j++)
		{
			seq = j;
			payload = (char*) malloc (sizeof(char)*PAYLOAD_SIZE);	
			/*int cur_seq_num = counter % CHK_PKT_COUNT;
			seq = cur_seq_num==0 ? CHK_PKT_COUNT : cur_seq_num; // packet sequence number	
			//cout << "Packet Number" << seq << endl;
			chk_ID = cur_seq_num==0 ? counter/CHK_PKT_COUNT : counter/CHK_PKT_COUNT + 1; //chunk ID
			cout << "Chunck Number" << chk_ID << endl;*/
			
			//cout << "Reading" << endl;
			//cout << cur_bytes_read << endl;
			//cout << payload << endl;

			Ptr<Packet> packet = Create<Packet> ();
        		mf::MessageHeader msg;
	       		msg.SetMessageType (9); //type = 9 for data packets
	       		msg.SetSequenceNumber (chkID); // sets chunck ID
               		msg.SetMessageAddr(m_mainAddress); //sets the IP address
			
			mf::MessageHeader::DATA &data = msg.GetDATA ();
			data.src = sender; //source address
			data.dest = receiver; //destination address
			data.pkt_no = seq; //packet sequence number
			data.payload_size = PAYLOAD_SIZE; //data
			for (int i=0; i<PAYLOAD_SIZE; i++)
				data.payload[i] = '0';
			packet->AddHeader(msg);

			//cout << "Pkt Size" << packet->GetSize() << endl;

			// saves the packet in a data set
			if (seq == 1)
			{
				tuple.source = sender;
				//tuple.packet_count = seq;
				tuple.chunck_id = chkID;
				//cout << "1" << endl;
				tuple.data_pkts.push_back (packet);
			}
			else
				tuple.data_pkts.push_back (packet);

			//if (chk_ID == 1)
			//{
			size = tuple.data_pkts.size ();
				//cout << size << endl;
			//}

			if (seq == CHK_PKT_COUNT)
			{
				//tuple.packet_count = seq;				
				m_dataSet.push_back (tuple);
				tuple.data_pkts.clear ();
				PacketCountTuple pkt_count_tuple; 
				pkt_count_tuple.source = sender;
				pkt_count_tuple.chunck_id = chkID;
				pkt_count_tuple.packet_count = seq;
				m_PktCountSet.push_back (pkt_count_tuple);
				//cout << "inserted chunck: " << chk_ID << endl;
			}
		}
			
		

		//}
		
		// initilizes file transfer by sending CSYN packet for chunck 1
		if (chkID == 1)
		{
			Ptr<Packet> packet = SendCSYNC (size, 1, m_mainAddress, sender, receiver);		
			return packet;
		}
		else
		{
			int resume_size = resume.size ();
			for (int i=0; i<resume_size; i++)
			{
				if(resume[i] == sender)
				{
					cout << "in resume" << endl;
					Ptr<Packet> packet = SendCSYNC (size, chkID, m_mainAddress, sender, receiver);
					resume[i] = "0.0.0.0";
					return packet;
				}
			}
		}
					
		return NULL;
	}


	// receives data packets
	void HOPProtocol::RcvdDataPkt (mf::MessageHeader data_pkt, Ipv4Address m_mainAddress)
	{
		int bitmap_flag = 0;

		const mf::MessageHeader::DATA &data = data_pkt.GetDATA ();

		uint32_t chk_id = data_pkt.GetSequenceNumber (); //gets the chunk ID

		cout << " Data Packet rcvd of seq no: " << data.pkt_no << " of chunck " << chk_id << "of source " << data.src << endl;	
		
		// sets the bitmap for CSYN ACK

		for (BitmapSet::iterator it = m_bitmapSet.begin ();it != m_bitmapSet.end (); it++)
    		{
      			if (it->chunck_id == chk_id && it->source == data.src)
			{
				//cout << "bitmap set" << endl;
				it->bitmap[data.pkt_no-1] = '1';
				bitmap_flag = 1;
			}
		}
				
		if (bitmap_flag == 0)
		{
			BitmapTuple tuple;
			tuple.source = data.src;
			tuple.chunck_id = chk_id;
			for (int i=0; i<CHK_PKT_COUNT; i++)
				tuple.bitmap[i] = '0';
			tuple.bitmap[data.pkt_no - 1] = '1';
			m_bitmapSet.push_back (tuple);
		}

		//if destination address = My IP address, saves the packet in a packet array
		if (data.dest ==  m_mainAddress)
		{	
			pkt_array[data.pkt_no - 1] =  data_pkt;
		}
		// else, saves it in data set by setting the address equal to the main address
		else
		{
			data_pkt.SetMessageAddr (m_mainAddress);
			Ptr<Packet> packet = Create<Packet> ();
			packet->AddHeader(data_pkt);
			
			/*if (data.pkt_no != 1)
			{			
				for (DataSet::iterator it = m_dataSet.begin ();it != m_dataSet.end (); it++)
				{
					if (it->chunck_id == chk_id && it->source == data.src)
					{
						cout <<  "size before inserted" << it->data_pkts.size () << endl;
						it->data_pkts.push_back(packet);
						int size = it->data_pkts.size ();
						cout <<  "size when inserted" << size << endl;
						break;
					}
				}
			}
			else
			{
				cout << "started inserting" << endl;
				DataTuple tuple;
				tuple.source = data.src;
				tuple.chunck_id = chk_id;
				//cout << "1" << endl;
				tuple.data_pkts.push_back (packet);
				m_dataSet.push_back (tuple);
				cout << "inserted" << endl;
				//tuple.data_pkts.clear ();
				for (DataSet::iterator it = m_dataSet.begin ();it != m_dataSet.end (); it++)
				{
					if (it->chunck_id == chk_id && it->source == data.src)
					{
						cout <<  "size before inserted" << it->data_pkts.size () << endl;
					}
					break;
				}
			}*/
			
			int insert_flag = 0;

			for (DataSet::iterator it = m_dataSet.begin ();it != m_dataSet.end (); it++)
			{
					if (it->source == data.src && it->chunck_id == chk_id)
					{
						cout <<  "size before inserted" << it->data_pkts.size () << endl;
						it->data_pkts.push_back(packet);
						int size = it->data_pkts.size ();
						cout <<  "size when inserted" << size << endl;
					
						insert_flag = 1;
						break;
					}
			}

			cout << "insert Flag " << insert_flag << endl;
			if(insert_flag == 0)
			{
				cout << "started inserting" << endl;
				DataTuple tuple;
				tuple.source = data.src;
				tuple.chunck_id = chk_id;
				//cout << "1" << endl;
				tuple.data_pkts.push_back (packet);
				m_dataSet.push_back (tuple);
				cout << "inserted" << endl;
				//tuple.data_pkts.clear ();
				for (DataSet::iterator it = m_dataSet.begin ();it != m_dataSet.end (); it++)
				{
					if (it->chunck_id == chk_id && it->source == data.src)
					{
						cout <<  "size before inserted" << it->data_pkts.size () << endl;
					}
					break;
				}
			}
		
				
		}
			
	}

	// writes data in the file
	/*void HOPProtocol::ToFile (int pkt_count, Ipv4Address sender)
	{
		cout << "in  to file" << endl;
		for (int i=0; i<pkt_count; i++)
		{
			mf::MessageHeader data_pkt = pkt_array[i];
			const mf::MessageHeader::DATA &data = data_pkt.GetDATA ();

			int bytes_written;
			char *payload;
			payload = (char*) malloc (sizeof(char)*data.payload_size);
			
			for (int i=0; i<data.payload_size; i++)
			{
				payload[i] = data.payload[i];
			}
			
			if (sender == "10.1.1.1")
				bytes_written = fwrite(payload, 1, data.payload_size, fp_1);
		
			if (sender == "10.1.1.6")
				bytes_written = fwrite(payload, 1, data.payload_size, fp_2);
		
			if (sender == "10.1.1.7")
				bytes_written = fwrite(payload, 1, data.payload_size, fp_3);
		
		}
	}*/

	void HOPProtocol::ToFile (int pkt_count, Ipv4Address sender)
	{
		cout << "Finally received Packet" << endl;
	}

	// creates CSYNC packet
	Ptr<Packet> HOPProtocol::SendCSYNC (int size, uint32_t seq_no, Ipv4Address m_mainAddress, Ipv4Address sender, Ipv4Address receiver)
	{
		cout << m_mainAddress << " in sendcsync for " << size << endl;
		Ptr<Packet> packet = Create<Packet> ();
        	mf::MessageHeader msg;
	       	msg.SetMessageType (7); // type = 7 for CSYNC packet
	       	msg.SetSequenceNumber (seq_no); //sets message sequence number = chunck ID
               	msg.SetMessageAddr(m_mainAddress); //sets IP address - main address

		mf::MessageHeader::CSYNC &csync = msg.GetCSYNC ();
		csync.src = sender;
		csync.dest = receiver;
		csync.chunck_pkt_count = size; // number of data packets in chunck
		cout << size << endl;
		packet->AddHeader(msg);	
		
		return packet;
	}

	// creates CSYNC ACK packet
	CSYNCTuple HOPProtocol::SendCSYNCACK (mf::MessageHeader csync_pkt, Ipv4Address m_mainAddress)
	{
		int file_flag = 1;
		
		CSYNCTuple csync_tuple;
		int inserted_flag = 0;
		uint32_t chk_id = csync_pkt.GetSequenceNumber (); // chunck ID from CSYNC packet
		//cout << "chk id" << chk_id << endl;

		const mf::MessageHeader::CSYNC &csync = csync_pkt.GetCSYNC ();

		Ipv4Address sender = csync.src;
		Ipv4Address receiver = csync.dest;
		
		int pkt_count = csync.chunck_pkt_count; // number of packets in chunck from CSYNC packet
		//cout << "pkt count" << pkt_count << endl;

		cout << " CSYNC Rcvd for chunck id: " << chk_id << "for source " << csync.src << endl;

		// saves the number of packets in the chunck in a Packet Count Tuple
		for (PktCountSet::iterator it = m_PktCountSet.begin ();it != m_PktCountSet.end (); it++)
		{
			if (it->chunck_id == chk_id && it->source == csync.src)
			{
				inserted_flag = 1;
				break;
			}
		}
		
		cout << "inserted flag " << inserted_flag << endl;
		if(inserted_flag == 0)
		{	
			PacketCountTuple pkt_count_tuple; 
			pkt_count_tuple.source = sender;
			pkt_count_tuple.chunck_id = chk_id;
			pkt_count_tuple.packet_count = pkt_count;
			m_PktCountSet.push_back (pkt_count_tuple);
		}

		//creates a CSYNC ACK packet
		Ptr<Packet> packet = Create<Packet> ();
        	mf::MessageHeader msg;
	       	msg.SetMessageType (8); // type = 8 for CSYNC ACK packet
	       	msg.SetSequenceNumber (chk_id); // set chunck ID
               	msg.SetMessageAddr(m_mainAddress); // sets main address

		mf::MessageHeader::CSYNC_ACK &csync_ack = msg.GetCSYNC_ACK ();
		csync_ack.src = sender; // source address
		csync_ack.dest = receiver; // destination address
		csync_ack.chunck_pkt_count = pkt_count; // sets packet count

		for (int i=0; i<CHK_PKT_COUNT; i++)
			csync_ack.bitmap[i] = '0';

		// gets the Bitmap
		BitmapTuple* rcvd_pkt_bitmap = GetBitmap(chk_id, sender);
		
		//sets the bitmap in CSYNC ACK packet
		if (rcvd_pkt_bitmap != NULL)
		{
			cout << "in rcvd bitmap" << endl;
			for (int i=0; i<pkt_count; i++)
				csync_ack.bitmap[i] = rcvd_pkt_bitmap->bitmap[i];
		}

		packet->AddHeader(msg);	
		
		csync_tuple.csync_ack_pkt = packet;
		csync_tuple.flag = 0;	
		// if all the packets of this chunck are received, 
		//then sends the packets to the ToFile function (if destination = main address)
		// if not, then sends a CSYNC for the first chunck to start packet transfer
		if (rcvd_pkt_bitmap != NULL)
		{
			for (int i=0; i<pkt_count; i++)
			{
				//cout << "bitmap" << rcvd_pkt_bitmap->bitmap[i] << endl;
				if (rcvd_pkt_bitmap->bitmap[i] == '0')
				{
					file_flag = 0;
					break;
				}
			}	
		}

		if (rcvd_pkt_bitmap != NULL && file_flag == 1)
		{
			//cout << "in ToFile for chunck id: " << chk_id << endl;
			if (receiver == m_mainAddress)
			{
				
					//if (chunck_in_file != chk_id)	
					//{
					//Time rcvd = Simulator::Now ();
					//int64_t SETT_int = rcvd.GetNanoSeconds();	 			
					//cout << "nehal" << m_mainAddress << " " << chk_id << " " << SETT_int << endl;
					total_chunck++;
					cout << "nehal" << sender << ":" << m_mainAddress << " " << total_chunck << endl;
					ToFile (pkt_count, sender);
					//}
					//chunck_in_file = chk_id;
			}
			else
			{
				//initilizes data transfer to the next hop
				if (chk_id == 1)
				{
					csync_packet = SendCSYNC (pkt_count, chk_id, m_mainAddress, sender,receiver);
					csync_tuple.source = sender;
					csync_tuple.destination = receiver;
					csync_tuple.csync_pkt = csync_packet;
					csync_tuple.flag = 1;					
				}
				// resumes if the data transfer was stopped because all the data packets of the chunck were not received
				else
				{
					int resume_size = resume.size ();
					for (int i=0; i<resume_size; i++)
					{
						if(resume[i] == sender)
						{
							cout << "in resume" << endl;
							csync_packet = SendCSYNC (pkt_count, chk_id, m_mainAddress, sender,receiver);
							csync_tuple.source = sender;
							csync_tuple.destination = receiver;
							csync_tuple.csync_pkt = csync_packet;
							csync_tuple.flag = 1;
							resume[i] = "0.0.0.0";
							break;
						}
					}
				}	
			}
		}
		//cout << "CSYNC Flag" << endl;
		return csync_tuple;
		
	}

	// processes CSYNC ACK packets and returns a vector data packets and the corresponding CSYNC for it
	std::vector< Ptr<Packet> > HOPProtocol::ProcessCSYNCACK (mf::MessageHeader csync_ack_pkt, Ipv4Address m_mainAddress)	
	{
		int resend_flag = 0;	
		std::vector< Ptr<Packet> > data_pkts;
		data_pkts.clear();	
		
		uint32_t chk_id = csync_ack_pkt.GetSequenceNumber ();
		//cout << "chk id" << chk_id << endl;

		const mf::MessageHeader::CSYNC_ACK &csync_ack = csync_ack_pkt.GetCSYNC_ACK ();

		Ipv4Address sender = csync_ack.src;
		Ipv4Address receiver = csync_ack.dest;
		int pkt_count = csync_ack.chunck_pkt_count;
		//cout << "pkt count" << pkt_count << endl;

		cout << " CSYNC ACK Rcvd for chunck id: " << chk_id << "for source " << sender << endl;

		cout << pkt_count << endl;
		// checks if some packets fron the previous chunck is missing
		for (int i=0; i<pkt_count; i++)
		{
			cout << csync_ack.bitmap[i] << endl;
			if (csync_ack.bitmap[i] == '0')
				resend_flag = 1;
		}

		cout << "Resend Flag " << resend_flag << endl; 

		//cout << "resend flag" << resend_flag << endl;
		//if yes, then sends the missing packet
		if (resend_flag == 1)
		{
			vector< Ptr<Packet> > data_pkt_vector = GetDataPkts (chk_id, sender);
			cout << data_pkt_vector.size() << " " << pkt_count << endl;
			
			Time rcvd = Simulator::Now ();
			int64_t SETT_int = rcvd.GetNanoSeconds();	 			
			cout << "nehal" << m_mainAddress << " " << chk_id << " " << SETT_int << endl;

			for (int i=0; i<pkt_count; i++)
			{
				if (csync_ack.bitmap[i] == '0')
				{
					Ptr<Packet> packet = data_pkt_vector[i];
					data_pkts.push_back (packet);
				}
			}
			Ptr<Packet> packet = SendCSYNC (pkt_count, chk_id, m_mainAddress, sender, receiver);
			data_pkts.push_back (packet);
		}
		//if not, then sends the data packets for the next chunck
		if (resend_flag == 0)
		{
			int count = 0;			
			for (PktCountSet::iterator it = m_PktCountSet.begin ();it != m_PktCountSet.end (); it++)
			{
				if ((it->chunck_id == (chk_id + 1)) && it->source == sender)
				{
					count = it->packet_count;
					break;
				}
			}
			cout << "Count" << count << endl;
			if(count != 0)
			{
				vector< Ptr<Packet> > data_pkt_vector = GetDataPkts (chk_id + 1, sender);
				cout << "0" << endl;
				int data_size = data_pkt_vector.size ();
				cout << "Data Size" << data_size << endl;
				if (data_size == count)
				{
					Ptr<Packet> packet = SendCSYNC (data_pkt_vector.size() , chk_id+1, m_mainAddress, sender, receiver);
					cout << "inserted CYSNC for chunck id" << chk_id + 1 << endl;
					data_pkts.push_back (packet);
				}
			}
		}

		if (data_pkts.size () == 0)
		{
			resume.push_back(sender);
			cout << "inserted in resume" << endl;
		}

		return data_pkts;
	}

	// returns a Bitmap given the chunck ID
	BitmapTuple* HOPProtocol::GetBitmap (uint32_t &id, Ipv4Address source)
	{
  		for (BitmapSet::iterator it = m_bitmapSet.begin ();it != m_bitmapSet.end (); it++)
    		{
      			if (it->chunck_id == id && it->source == source)
        			return &(*it);
    		}
  		return NULL;
	}

	// returns data packets for a given chunck
	std::vector< Ptr<Packet> > HOPProtocol::GetDataPkts (uint32_t chunck_id, Ipv4Address source)
	{
		vector< Ptr<Packet> > data_pkt_vector;
		for (DataSet::iterator it = m_dataSet.begin ();it != m_dataSet.end (); it++)
		{
			if (it->chunck_id == chunck_id && it->source == source)
			{
				int size = it->data_pkts.size ();
				cout << "size in GetDataPackets" << size << endl;
				for (int i=0; i<size; i++)
				{
					Ptr<Packet> pkt = it->data_pkts[i];
					data_pkt_vector.push_back (pkt);
					cout << "in for" << endl;
				}
				break;
			}
		}
		cout << "out of data" << endl;
		return data_pkt_vector;
	}

	

	// erase Bitmap tuple given the chunck ID
	void HOPProtocol::EraseBitmapTuple (uint32_t &id, Ipv4Address source)
	{
  		for (BitmapSet::iterator it = m_bitmapSet.begin ();it != m_bitmapSet.end (); it++)
    		{
      			if (it->chunck_id == id && it->source == source)
        		{
        	  		it = m_bitmapSet.erase (it);
        	  		break;
        		}
    		}
	}	
	
	
	
		
		
}
