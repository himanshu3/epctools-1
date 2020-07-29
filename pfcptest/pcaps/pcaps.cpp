#include <stdlib.h>
#include <vector>
#include <signal.h>

#include "Packet.h"
#include "UdpLayer.h"
#include "PcapFileDevice.h"
#include "PayloadLayer.h"

#include "pfcpr15.h"
#include "epctools.h"
#include "epfcp.h"

#include "test.h"

#define LOG_SYSTEM 1
#define LOG_PFCP 2
#define LOG_TEST 3

namespace PFCPTest
{
    bool PcapTest(void *args)
    {
        EString &name = *reinterpret_cast<EString *>(args);
        EString original_pcap = "./pcaps/originals/" + name + ".pcap";
        EString baseline_pcap = "./pcaps/baselines/" + name + ".pcap";
        EString result_pcap = "./pcaps/results/" + name + ".pcap";

        // use the IFileReaderDevice interface to automatically identify file type (pcap/pcap-ng)
        // and create an interface instance that both readers implement
        pcpp::IFileReaderDevice *reader = pcpp::IFileReaderDevice::getReader(original_pcap.c_str());

        // verify that a reader interface was indeed created
        if (reader == NULL)
            throw EError(EError::Error, errno, "Cannot determine reader for file type");

        // open the reader for reading
        if (!reader->open())
            throw EError(EError::Error, errno, "Cannot open input pcap for reading");

        bool result = true;

        // read the first (and only) packet from the file
        pcpp::RawPacket rawPacket;
        pcpp::RawPacketVector result_packets;
        while(reader->getNextPacket(rawPacket))
        {
            // parse the raw packet into a parsed packet
            pcpp::Packet parsedPacket(&rawPacket);

            pcpp::UdpLayer *udpLayer = parsedPacket.getLayerOfType<pcpp::UdpLayer>();
            if (udpLayer == nullptr)
                throw EError(EError::Error, errno, "Something went wrong, couldn't find UDP layer");

            std::vector<uint8_t> payload;
            payload.assign(udpLayer->getLayerPayload(), udpLayer->getLayerPayload() + udpLayer->getLayerPayloadSize());

            PFCP_R15::Translator trans;

            PFCP::TranslatorMsgInfo info;
            trans.getMsgInfo(info, payload.data(), payload.size());

            PFCP::ReqInPtr msg_in = new PFCP::ReqIn();
            msg_in->setSeid(info.seid());
            msg_in->setSeqNbr(info.seqNbr());
            msg_in->setMsgType(info.msgType());
            msg_in->setIsReq(info.isReq());
            msg_in->setVersion(info.version());
            msg_in->assign(payload.data(), payload.size());

            PFCP::AppMsgReqPtr app_msg = trans.decodeReq(msg_in);

            switch(app_msg->msgType()) {
                case PFCP_PFD_MGMT_REQ: {
                    PFCP_R15::PfdMgmtReq &pfd_mgmt_req = *(static_cast<PFCP_R15::PfdMgmtReq *>(app_msg));
                    // // TEMP fixup
                    // pfd_mgmt_req.data().header.message_len += 1;
                    // pfd_mgmt_req.data().app_ids_pfds[0].header.len += 1;
                    // pfd_mgmt_req.data().app_ids_pfds[0].pfd_context[0].header.len += 1;
                    // pfd_mgmt_req.data().app_ids_pfds[0].pfd_context[0].pfd_contents[0].header.len += 1;
                    break;
                }
                default:
                    ELogger::log(LOG_PFCP).major("Unhandled PFCP message type {}", app_msg->msgType());
            }

            PFCP::ReqOutPtr msg_out = trans.encodeReq(app_msg);
            
            std::vector<uint8_t> payload_out;
            payload_out.assign(msg_out->data(), msg_out->data() + msg_out->len());
            payload_out.back() = 67;

            if(payload.size() != payload_out.size() || 
               memcmp(payload.data(), payload_out.data(), payload.size()) != 0)
                result = false;

            parsedPacket.removeAllLayersAfter(udpLayer);

            pcpp::PayloadLayer *new_layer = new pcpp::PayloadLayer(payload_out.data(), payload_out.size(), false);
            parsedPacket.addLayer(new_layer, true);

            // compute all calculated fields
            parsedPacket.computeCalculateFields();
            result_packets.pushBack(new pcpp::RawPacket(*parsedPacket.getRawPacket()));
        }

        // close the file reader, we don't need it anymore
        reader->close();

        // write the modified packets to a pcap file
        pcpp::PcapFileWriterDevice writer(result_pcap.c_str(), result_packets.front()->getLinkLayerType());
        writer.open();
        for(auto packet : result_packets)
            writer.writePacket(*packet);
        writer.close();

        return result;
    }
}